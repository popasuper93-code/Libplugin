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
#include <link.h>
#include <android/dlext.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "chad_menu.h"

#define LOG_TAG "CHAD_MENU"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

bool g_Initialized = false;
int g_FrameCount = 0;

// Оригиналы функций
EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
void* (*orig_android_dlopen_ext)(const char* filename, int flag, const android_dlextinfo* extinfo);

// Функция патча libloader.so
void patch_libloader(void* handle) {
    if (!handle) return;
    struct link_map* map;
    dlinfo(handle, RTLD_DI_LINKMAP, &map);
    uintptr_t base = map->l_addr;
    
    uintptr_t target_addr = base + 0x778480; // Из твоих логов
    
    // Разрешаем запись
    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = target_addr & ~(page_size - 1);
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

    // MOV X0, #0 ; RET (возврат null, чтобы не было JNI-краша)
    uint32_t* patch = (uint32_t*)target_addr;
    patch[0] = 0xd2800000; 
    patch[1] = 0xd65f03c0; 

    __builtin___clear_cache((char*)target_addr, (char*)target_addr + 8);
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_EXEC);
    
    LOGI("=== АНТИЧИТ В libloader.so (0x778480) ЗАПАТЧЕН! ===");
}

// Хук на загрузку библиотек
void* hook_android_dlopen_ext(const char* filename, int flag, const android_dlextinfo* extinfo) {
    // Вызываем оригинальную функцию загрузки
    void* handle = orig_android_dlopen_ext(filename, flag, extinfo);

    if (filename && strstr(filename, "libloader.so")) {
        LOGI("=== ПЕРЕХВАЧЕНА ЗАГРУЗКА libloader.so ===");
        patch_libloader(handle);
    }
    
    // Проверяем наш плагин, чтобы понять, работает ли хук
    if (filename && strstr(filename, "libplugin.so")) {
        LOGI("=== ПЕРЕХВАЧЕНА ЗАГРУЗКА НАШЕГО ПЛАГИНА ===");
    }

    return handle;
}

// Хук отрисовки
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

// Установка inline-хука
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
    
    // Пишем в файл для 100% проверки
    FILE* f = fopen("/sdcard/CHAD_PLUGIN_LOADED.txt", "w");
    if (f) {
        fprintf(f, "Plugin loaded successfully!\n");
        fclose(f);
    }

    // Хукаем загрузчик библиотек, чтобы поймать libloader.so
    void* dl = dlopen("libdl.so", RTLD_LAZY);
    if (dl) {
        void* dlopen_ext = dlsym(dl, "android_dlopen_ext");
        if (dlopen_ext) {
            inline_hook(dlopen_ext, (void*)hook_android_dlopen_ext, (void**)&orig_android_dlopen_ext);
            LOGI("Хук на android_dlopen_ext установлен!");
        }
    }

    // Хукаем отрисовку
    void* egl = dlopen("libEGL.so", RTLD_LAZY);
    if (egl) {
        void* swap = dlsym(egl, "eglSwapBuffers");
        if (swap) inline_hook(swap, (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    }
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("=== JNI_OnLoad ВЫЗВАН ===");
    return JNI_VERSION_1_6;
}
