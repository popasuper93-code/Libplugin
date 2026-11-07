#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include "shadowhook.h" // Подключаем ShadowHook

#define LOG_TAG "CHAD_CHEAT"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void (*orig_abort)();

void hook_abort() {
    LOGI("BYPASS: Перехвачен abort()! Игра пыталась упасть.");
}

__attribute__((constructor))
void on_load() {
    LOGI("=== CHAD PLUGIN: ЗАГРУЖЕН (SHADOWHOOK) ===");
    
    // Инициализация хука
    shadowhook_init(SHADOWHOOK_MODE_UNIQUE, false);
    
    // Ставим хук на abort
    shadowhook_hook_sym_name("libc.so", "abort", (void*)hook_abort, (void**)&orig_abort);
    LOGI("Хук на abort установлен!");
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    return JNI_VERSION_1_6;
}
