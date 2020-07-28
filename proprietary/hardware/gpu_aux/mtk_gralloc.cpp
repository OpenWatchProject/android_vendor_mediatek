#include "mtk_gralloc.h"
#include "mtk_gralloc0.h"
#include "mtk_gralloc1.h"
#include "utils.h"

android::Mutex MTKGralloc::s_mutex;
MTKGralloc *MTKGralloc::s_pInst = 0;

MTKGralloc *MTKGralloc::getInstance()
{
    const hw_module_t *pModule;
    int err;
    uint8_t major;

    s_mutex.lock();
    if (0 == s_pInst) {
        err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &pModule);
        if (err) {
            ALOGE("[AUX] failed to get gralloc module");
            s_mutex.unlock();
            return NULL;
        }

        major = (pModule->module_api_version >> 8) & 0xff;
        switch (major) {
            case 1:
                s_pInst = new MTKGralloc1();
                break;
            case 0:
                s_pInst = new MTKGralloc0();
                break;
            default:
                ALOGE("[AUX] unknown gralloc module major version %d", major);
                s_mutex.unlock();
                return NULL;
        }
        atexit(destroyInstance);
    }
    s_mutex.unlock();
    return s_pInst;
}

void MTKGralloc::destroyInstance()
{
    s_mutex.lock();
    if (0 != s_pInst) {
        delete s_pInst;
        s_pInst = 0;
    }
    s_mutex.unlock();
}

uint64_t MTKGralloc::getUsage()
{
    return usage;
}
