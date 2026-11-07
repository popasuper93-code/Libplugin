#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <string.h>

#define LOG_TAG "CHAD_CHEAT"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Оригинальная функция
void (*orig_abort)();

// Наш хук
void hook_abort() {
    LOGI("BYPASS: Перехвачен abort()! Игра пыталась упасть.");
}

// Функция для установки хука (меняет первые инструкции функции на прыжок)
void inline_hook(void* target, void* replace, void** backup) {
    if (!target) return;
    
    // Сохраняем оригинальные байты (первые 16 байт)
    *backup = malloc(16);
    memcpy(*backup, target, 16);

    // Делаем память доступной для записи
    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = (uintptr_t)target & ~(page_size - 1);
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

    // Патчим под ARM64 (LDR X16, #8; BR X16; <target_address>)
    // Это прыжок на наш хук (replace)
    uint32_t* patch = (uint32_t*)target;
    patch[0] = 0x58000050; // LDR X16, #8
    patch[1] = 0xd61f0200; // BR X16
    patch[2] = ((uintptr_t)replace & 0xFFFFFFFF); // Адрес хука (нижние 32 бита)
    patch[3] = ((uintptr_t)replace >> 32);        // Адрес хука (верхние 32 бита)
    
    // Очищаем кэш инструкций
    __builtin___clear_cache((char*)target, (char*)target + 16);
    
    // Возвращаем защиту (опционально)
    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_EXEC);
}

__attribute__((constructor))
void on_load() {
    LOGI("========================================");
    LOGI("=== CHAD PLUGIN: ЗАГРУЖЕН (MANUAL HOOK) ===");
    LOGI("========================================");

    void* libc = dlopen("libc.so", RTLD_LAZY);
    if (libc) {
        void* abort_addr = dlsym(libc, "abort");
        if (abort_addr) {
            inline_hook(abort_addr, (void*)hook_abort, (void**)&orig_abort);
            LOGI("Ручной хук на abort установлен!");
        }
    }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    return JNI_VERSION_1_6;
}
