#include <jni.h>
#include <android/log.h>

#define LOG_TAG "CHAD_MENU"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

__attribute__((constructor))
void on_load() {
    // Если ты увидишь это в логах — значит плагин жив
    LOGI("=== ТЕСТ: ПЛАГИН ЗАГРУЖЕН И РАБОТАЕТ ===");
}
