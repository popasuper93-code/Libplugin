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
#include <link.h> // Для поиска базового адреса

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

// Ищем базовый адрес библиотеки libloader.so в памяти
int find_libloader(struct dl_phdr_info *info, size_t size, void *data) {
    if (strstr(info->dlpi_name, "libloader.so") != NULL) {
        g_libloader_base = info->dlpi_addr;
        LOGI("Нашли libloader.so! База: 0x%lx", g_libloader_base);
        return 1; // Останавливаем поиск
    }
    return 0;
}

// Функция-патчер
void patch_libloader() {
    while (g_libloader_base == 0) {
        dl_iterate_phdr(find_libloader, NULL);
        sleep(1); // Ждем, пока игра загрузит библиотеку
    }

    // Смещение из твоего лога
    uintptr_t target_addr = g_libloader_base + 0x778480;
    
    // Делаем память доступной для записи
    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = target_addr & ~(page_size - 1);
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

    // Патчим инструкцию на RET (возврат) 0xC0035FD6, чтобы функция просто вышла
    uint32_t* patch = (uint32_t*)target_addr;
    patch[0] = 0xd65f03c0; // RET в ARM64

    __builtin___clear_cache((char*)target_addr, (char*)target_addr + 4);
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_EXEC);
    
    LOGI("=== АНТИЧИТ В libloader.so ПО АДРЕСУ 0x778480 УСПЕШНО ЗАБЛОКИРОВАН! ===");
}

// Поток для патча
void* patch_thread(void*) {
    patch_libloader();
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
    LOGI("=== CHAD PLUGIN: ОХОТА НА АНТИЧИТ ===");

    // Запускаем поток для поиска и патча libloader.so
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
