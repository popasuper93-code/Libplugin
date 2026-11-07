#define IMGUI_DEFINE_MATH_OPERATORS
#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "chad_menu.h"

#define LOG_TAG "CHAD_MENU"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

bool g_Initialized = false;
int g_FrameCount = 0; // Счетчик кадров для логов

EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (g_FrameCount < 5) {
        LOGI("Хук отрисовки вызван! Кадр: %d", g_FrameCount);
    }

    if (!g_Initialized) {
        LOGI("Попытка инициализации ImGui...");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;

        // Отключаем шрифты для теста, чтобы исключить краш из-за них
        // chad::LoadFonts("fonts"); 

        if (!ImGui_ImplOpenGL3_Init("#version 300 es")) {
            LOGI("ОШИБКА: Не удалось инициализировать OpenGL3 backend!");
        } else {
            LOGI("OpenGL3 backend успешно инициализирован.");
        }
        
        g_Initialized = true;
    }

    if (g_Initialized) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Вызываем меню Клода
        chad::Render(); 

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        if (g_FrameCount < 5) {
            LOGI("Кадр %d успешно отрисован.", g_FrameCount);
        }
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
    LOGI("=== CHAD PLUGIN ЗАГРУЖЕН (DEBUG MODE) ===");

    void* egl = dlopen("libEGL.so", RTLD_LAZY);
    if (egl) {
        void* swap = dlsym(egl, "eglSwapBuffers");
        if (swap) {
            inline_hook(swap, (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("Хук на eglSwapBuffers установлен!");
        } else {
            LOGI("ОШИБКА: Не нашли eglSwapBuffers!");
        }
    } else {
         LOGI("ОШИБКА: Не смогли открыть libEGL.so!");
    }
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    return JNI_VERSION_1_6;
}
