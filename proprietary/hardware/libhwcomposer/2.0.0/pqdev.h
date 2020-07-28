#ifndef __HWC_PQ_DEV_H__
#define __HWC_PQ_DEV_H__

#include <utils/Singleton.h>
#include <utils/Mutex.h>

using namespace android;

class PqDevice : public Singleton<PqDevice>
{
public:
    PqDevice();
    ~PqDevice();

    bool setColorTransform(const float* matrix, const int32_t& hint);

    bool isColorTransformIoctl();
    void useColorTransformIoctl(int32_t useIoctl);

private:
    bool setColorTransformViaIoctl(const float* matrix, const int32_t& hint);
    bool setColorTransformViaService(const float* matrix, const int32_t& hint);

    int m_pq_fd;

    Mutex m_lock;
    bool m_use_ioctl;
};

#endif
