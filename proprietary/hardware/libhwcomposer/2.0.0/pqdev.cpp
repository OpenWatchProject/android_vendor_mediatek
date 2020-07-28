#define DEBUG_LOG_TAG "PqDevice"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "pqdev.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include <ddp_pq.h>
#include <ddp_drv.h>

#include "platform.h"
#include "utils/debug.h"

#define MTK_PQ_DEVICE "/dev/mtk_disp_mgr"

#ifdef USES_PQSERVICE
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>
using android::hardware::hidl_array;
using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;
#endif

#ifdef USE_SWWATCHDOG
#include "utils/swwatchdog.h"
#define WDT_IOCTL(fd, CMD, ...)                                                             \
({                                                                                          \
    int err = 0;                                                                            \
    if (Platform::getInstance().m_config.wdt_trace)                                         \
    {                                                                                       \
        ATRACE_NAME(#CMD);                                                                  \
        SWWatchDog::AutoWDT _wdt(String8::format("[PqDev] ioctl(" #CMD "):%d", __LINE__), 500);   \
        err = ioctl(fd, CMD, ##__VA_ARGS__);                                                \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        SWWatchDog::AutoWDT _wdt(String8::format("[PqDev] ioctl(" #CMD "):%d", __LINE__), 500);   \
        err = ioctl(fd, CMD, ##__VA_ARGS__);                                                \
    }                                                                                       \
    err;                                                                                    \
})
#else
#define WDT_IOCTL(fd, CMD, ...)                                                             \
({                                                                                          \
    int err = 0;                                                                            \
    if (Platform::getInstance().m_config.wdt_trace)                                         \
    {                                                                                       \
        ATRACE_NAME(#CMD);                                                                  \
        err = ioctl(fd, CMD, ##__VA_ARGS__);                                                \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        err = ioctl(fd, CMD, ##__VA_ARGS__);                                                \
    }                                                                                       \
    err;                                                                                    \
})
#endif

#ifndef DISP_IOCTL_SUPPORT_COLOR_TRANSFORM
#define DISP_IOCTL_SUPPORT_COLOR_TRANSFORM 0x0
struct DISP_COLOR_TRANSFORM {
    int matrix[4][4];
};
#endif

ANDROID_SINGLETON_STATIC_INSTANCE(PqDevice);

PqDevice::PqDevice()
    : m_pq_fd(-1)
    , m_use_ioctl(false)
{
    if (DISP_IOCTL_SUPPORT_COLOR_TRANSFORM != 0)
    {
        m_use_ioctl = true;
        m_pq_fd = open(MTK_PQ_DEVICE, O_WRONLY);
        if (m_pq_fd == -1)
        {
            HWC_LOGW("failed to open pq device node(%s): %s", strerror(errno));
        }
    }
}

PqDevice::~PqDevice()
{
    if (m_pq_fd != -1)
    {
        protectedClose(m_pq_fd);
    }
}

bool PqDevice::setColorTransform(const float* matrix, const int32_t& hint)
{
    bool res = false;
    if (isColorTransformIoctl())
    {
        res = setColorTransformViaIoctl(matrix, hint);
    }
    else
    {
        res = setColorTransformViaService(matrix, hint);
    }
    return res;
}

bool PqDevice::setColorTransformViaIoctl(const float* matrix, const int32_t& /*hint*/)
{
    bool res = false;
    int32_t ret = 0;

    if (m_pq_fd != -1)
    {
        struct DISP_COLOR_TRANSFORM transform;
        const int32_t dimension = 4;
        for (int32_t i = 0; i < dimension; ++i)
        {
            DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D', "matrix ");
            for (int32_t j = 0; j < dimension; ++j)
            {
                transform.matrix[i][j] = matrix[i * dimension + j] * 1024.f + 0.5f;
                logger.printf("%d,", transform.matrix[i][j]);
            }
        }
        ret = WDT_IOCTL(m_pq_fd, DISP_IOCTL_SUPPORT_COLOR_TRANSFORM, &transform);
        if (ret == 0)
        {
            res = true;
        }
    }
    else
    {
        HWC_LOGW("failed to set color transform, because pq fd is invalid");
    }

    return res;
}

bool PqDevice::setColorTransformViaService(const float* matrix, const int32_t& hint)
{
    bool res = false;

#ifdef USES_PQSERVICE
    int retryCount = 0;
    sp<IPictureQuality> pq_service = IPictureQuality::tryGetService();

    while (pq_service == nullptr)
    {
        if (retryCount > 20)
        {
            HWC_LOGE("Can't get PQ service tried(%d) times", retryCount);
        }
        usleep(100000); //sleep 100 ms to wait for next get service
        pq_service = IPictureQuality::tryGetService();
        retryCount++;
    }

    if (pq_service == nullptr)
    {
        HWC_LOGE("cannot find PQ service!");
        res = false;
    }
    else
    {
        const int32_t dimension = 4;
        hidl_array<float, 4, 4> send_matrix;
        for (int32_t i = 0; i < dimension; ++i)
        {
            DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D', "matrix ");
            for (int32_t j = 0; j < dimension; ++j)
            {
                send_matrix[i][j] = matrix[i * dimension + j];
                logger.printf("%f,", send_matrix[i][j]);
            }
        }
        res = (pq_service->setColorTransform(send_matrix, hint, 1) == Result::OK);
    }
#endif

    return res;
}

bool PqDevice::isColorTransformIoctl()
{
    Mutex::Autolock lock(m_lock);
    return m_use_ioctl;
}

void PqDevice::useColorTransformIoctl(int32_t useIoctl)
{
    Mutex::Autolock lock(m_lock);
    m_use_ioctl = useIoctl;
}
