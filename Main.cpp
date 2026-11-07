#include <jni.h>
#include <android/log.h>

#define LOG_TAG "CHAD_CHEAT"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Эта функция вызывается автоматически, когда игра загружает наш .so
__attribute__((constructor))
void on_load() {
    LOGI("========================================");
    LOGI("=== ПЛАГИН CHAD УСПЕШНО ЗАГРУЖЕН В ИГРУ! ===");
    LOGI("=======================================");
}

// Стандартная функция для JNI
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    return JNI_VERSION_1_6;
}
