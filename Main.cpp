#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/syscall.h>
#include <signal.h>
#include "dobby.h"

#define LOG_TAG "CHAD_CHEAT"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Оригинальные функции
void (*orig_abort)();
int (*orig_tgkill)(pid_t tgid, pid_t tid, int sig);
int (*orig_raise)(int sig);

// 1. Хук на abort (игра пытается сама себя убить)
void hook_abort() {
    LOGI("BYPASS: Перехвачен abort()! Игра пыталась упасть.");
}

// 2. Хук на tgkill (античит посылает сигнал убийства потоку)
int hook_tgkill(pid_t tgid, pid_t tid, int sig) {
    if (sig == SIGABRT || sig == SIGSEGV) {
        LOGI("BYPASS: Заблокирован сигнал %d для потока %d", sig, tid);
        return 0; // Игнорируем сигнал, процесс не умирает
    }
    return orig_tgkill(tgid, tid, sig);
}

// 3. Хук на raise
int hook_raise(int sig) {
    if (sig == SIGABRT || sig == SIGSEGV) {
        LOGI("BYPASS: Заблокирован raise(%d)", sig);
        return 0;
    }
    return orig_raise(sig);
}

__attribute__((constructor))
void on_load() {
    LOGI("========================================");
    LOGI("=== CHAD PLUGIN: ЗАГРУЖЕН С ОБХОДОМ ===");
    LOGI("========================================");
    
    void* libc = dlopen("libc.so", RTLD_LAZY);
    if (libc) {
        // Ставим хук на abort
        void* abort_addr = dlsym(libc, "abort");
        if (abort_addr) DobbyHook(abort_addr, (void*)hook_abort, (void**)&orig_abort);
        
        // Ставим хук на raise
        void* raise_addr = dlsym(libc, "raise");
        if (raise_addr) DobbyHook(raise_addr, (void*)hook_raise, (void**)&orig_raise);
        
        // Ставим хук на tgkill
        void* tgkill_addr = dlsym(libc, "tgkill");
        if (tgkill_addr) DobbyHook(tgkill_addr, (void*)hook_tgkill, (void**)&orig_tgkill);
        
        LOGI("Все хуки на защиту установлены!");
    }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    return JNI_VERSION_1_6;
}
