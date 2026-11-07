#define IMGUI_DEFINE_MATH_OPERATORS
#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/syscall.h>
#include <pthread.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "chad_menu.h"

#define LOG_TAG "CHAD_MENU"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

bool g_Initialized = false;
int g_FrameCount = 0;

EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
int (*orig_raise)(int sig);
int (*orig_pthread_kill)(pthread_t thread, int sig);

// 1. Блокируем raise (обычно используется в abort)
int hook_raise(int sig) {
    if (sig == SIGABRT || sig == SIGSEGV) {
        LOGI("BYPASS: Заблокирован raise(%d)!", sig);
        return 0;
    }
    return orig_raise(sig);
}

// 2. Блокируем pthread_kill (часто используется античитами)
int hook_pthread_kill(pthread_t thread, int sig) {
    if (sig == SIGABRT || sig == SIGSEGV) {
        LOGI("BYPASS: Заблокирован pthread_kill(%d)!", sig);
        return 0;
    }
    return orig_pthread_kill(thread, sig);
}

// 3. Хук отрисовки
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
    LOGI("=== CHAD PLUGIN: ХУК НА СИГНАЛЫ ===");

    void* libc = dlopen("libc.so", RTLD_LAZY);
    if (libc) {
        void* raise_addr = dlsym(libc, "raise");
        if (raise_addr) {
            inline_hook(raise_addr, (void*)hook_raise, (void**)&orig_raise);
            LOGI("Хук на raise установлен!");
        }

        void* pthread_kill_addr = dlsym(libc, "pthread_kill");
        if (pthread_kill_addr) {
            inline_hook(pthread_kill_addr, (void*)hook_pthread_kill, (void**)&orig_pthread_kill);
            LOGI("Хук на pthread_kill установлен!");
        }
    }

    void* egl = dlopen("libEGL.so", RTLD_LAZY);
    if (egl) {
        void* swap = dlsym(egl, "eglSwapBuffers");
        if (swap) inline_hook(swap, (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    }
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    return JNI_VERSION_1_6;
}
