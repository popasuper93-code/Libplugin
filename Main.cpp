#define IMGUI_DEFINE_MATH_OPERATORS
#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <link.h> // <--- ВАЖНО ДЛЯ dl_iterate_phdr

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "chad_menu.h"

#define LOG_TAG "CHAD_MENU"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

bool g_Initialized = false;
int g_FrameCount = 0;
uintptr_t g_libloader_base = 0;

EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

// Функция поиска библиотеки в памяти
int find_libloader(struct dl_phdr_info *info, size_t size, void *data) {
    if (info->dlpi_name && strstr(info->dlpi_name, "libloader.so") != NULL) {
        g_libloader_base = info->dlpi_addr;
        LOGI("libloader.so найдена! База: 0x%lx", g_libloader_base);
        return 1; // Нашли, останавливаем перебор
    }
    return 0;
}

void* patch_thread(void*) {
    LOGI("=== ПОТОК ПАТЧЕРА ЗАПУЩЕН ===");
    // Ждем, пока игра загрузит libloader.so
    while (g_libloader_base == 0) {
        dl_iterate_phdr(find_libloader, nullptr);
        usleep(10000); // Ждем 10 мс
    }
    
    LOGI("Патчим античит по адресу: 0x%lx", g_libloader_base + 0x778480);
    
    uintptr_t target_addr = g_libloader_base + 0x778480;
    
    // Делаем память доступной для записи
    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = target_addr & ~(page_size - 1);
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

    // Патчим на MOV X0, #0 ; RET (возврат null)
    uint32_t* patch = (uint32_t*)target_addr;
    patch[0] = 0xd2800000; // MOV X0, #0
    patch[1] = 0xd65f03c0; // RET

    __builtin___clear_cache((char*)target_addr, (char*)target_addr + 8);
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_EXEC);
    
    LOGI("=== АНТИЧИТ В libloader.so (0x778480) ЗАПАТЧЕН НА NULL! ===");
    return nullptr;
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (g_FrameCount < 5) LOGI("Хук отрисовки вызван! Кадр: %d", g_FrameCount);

    if (!g_Initialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;
        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_Initialized = true;
        LOGI("ImGui инициализирован!");
    }

    if (g_Initialized) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        chad::Render(); 
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    
    g_FrameCount++;
    return orig_eglSwapBuffers(dpy, surface);
}

void inline_hook(void* target, void* replace, void** backup) {
    if (!target) return;
    *backup = malloc(16);
    memcpy(*backup, target, 16);

    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = (uintptr_t)target & ~(page_size - 1);
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

    uint32_t* patch = (uint32_t*)target;
    patch[0] = 0x58000050;
    patch[1] = 0xd61f0200;
    patch[2] = (uint32_t)((uintptr_t)replace & 0xFFFFFFFF);
    patch[3] = (uint32_t)((uintptr_t)replace >> 32);

    __builtin___clear_cache((char*)target, (char*)target + 16);
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_EXEC);
}

__attribute__((constructor))
void on_load() {
    LOGI("=== CHAD PLUGIN: ЗАГРУЖЕН ===");

    // Запускаем патчер
    pthread_t thread;
    pthread_create(&thread, nullptr, patch_thread, nullptr);

    // Хукаем отрисовку меню
    void* egl = dlopen("libEGL.so", RTLD_LAZY);
    if (egl) {
        void* swap = dlsym(egl, "eglSwapBuffers");
        if (swap) inline_hook(swap, (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    }
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    return JNI_VERSION_1_6;
}
