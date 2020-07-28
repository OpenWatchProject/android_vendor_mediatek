#include "devicenode.h"

#define DEBUG_LOG_TAG "DNO"
#include "debug.h"

class DeviceNodeOperator {
public:
    DeviceNodeOperator(const char *path, int flag);
    ~DeviceNodeOperator();
    ssize_t writeMsg(void *buf, size_t size);
    ssize_t readMsg(void *buf, size_t size);
private:
    int mFd;
};

DeviceNodeOperator::DeviceNodeOperator(const char *path, int flag)
{
    mFd = open(path, flag);
    if (mFd == -1)
    {
        HWC_LOGE("failed to open %s", path);
    }
}

DeviceNodeOperator::~DeviceNodeOperator()
{
    if (mFd != -1)
    {
        close(mFd);
    }
}

ssize_t DeviceNodeOperator::writeMsg(void *buf, size_t size)
{
    ssize_t res = -1;
    if (mFd != -1) {
        res = write(mFd, buf, size);
    }

    return res;
}

ssize_t DeviceNodeOperator::readMsg(void *buf, size_t size)
{
    ssize_t res = -1;
    if (mFd != -1) {
        res = read(mFd, buf, size);
    }

    return res;
}

int set4kMhlInfo2Ged(bool enable)
{
    int res = 0;
    char buf[16];
    DeviceNodeOperator gedEvent("/d/ged/hal/event_notify", O_WRONLY);
    ssize_t len = snprintf(buf, sizeof(buf), "mhl4k-vid %d", enable ? 1 : 0);
    res = gedEvent.writeMsg(buf, len);
    if (len != res)
    {
        HWC_LOGE("failed to write 4k MHL info to GED");
    }
    return res;
}

int setMergeMdInfo2Ged(bool enable)
{
    int res = 0;
    char buf[32];
    DeviceNodeOperator gedEvent("/d/ged/hal/event_notify", O_WRONLY);
    ssize_t len = snprintf(buf, sizeof(buf), "video-merge-md %d", enable ? 1 : 0);
    res = gedEvent.writeMsg(buf, len);
    if (len != res)
    {
        HWC_LOGE("failed to write mergeMD info to GED");
    }
    return res;
}
