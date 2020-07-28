#include "VideoTelephony.h"

// for socket
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// for system property
#include <cutils/properties.h>
#include <cutils/sockets.h>
//for fd open
#include <errno.h>
#include <fcntl.h>
// for kill process
#include <sys/types.h>
#include <signal.h>
#include <limits.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace videotelephony {
namespace V1_0 {
namespace implementation {

VideoTelephony* VideoTelephony::sVideoTelephony = NULL;

// Methods from ::vendor::mediatek::hardware::videotelephony::V1_0::IVideoTelephony follow.
Return<uint16_t> VideoTelephony::configureFmqMode(uint16_t mode) {

    VT_HIDL_LOGD("[IVT] configureFmqMode");

    // skip operation if the config has been done before
    // it mean the VTS may restart, just use the same queue
    if (VT_HIDL_IS_SET(mInitStatus, VT_HIDL_INIT_STATUS_SET_MODE_DONE)) {

        VT_HIDL_LOGD("[IVT] configureFmqMode, already done, skip");
        return 0;
    }

    // 1 : ViLTE
    mMode = mode;

    VT_HIDL_SET(mInitStatus, VT_HIDL_INIT_STATUS_SET_MODE_DONE);

    if (VT_HIDL_INIT_STATUS_ALL_DONE == mInitStatus) {
        initialization();
    } else {
        VT_HIDL_LOGI("[IVT] configureFmqMode, getInstance wait client");
    }

    return 0;
}

Return<void> VideoTelephony::configureFmqSyncRead(configureFmqSyncRead_cb _hidl_cb) {

    VT_HIDL_LOGD("[IVT] configureFmqSyncRead");

    // skip operation if the config has been done before
    // it mean the VTS may restart, just use the same queue
    if (VT_HIDL_IS_SET(mInitStatus ,VT_HIDL_INIT_STATUS_SET_ReadQ_DONE)) {

        VT_HIDL_LOGD("[IVT] configureFmqSyncRead, already done, skip");

        _hidl_cb(true /* ret */, *mReadFmq->getDesc());

        return Void();
    }

    mReadFmq.reset(new (std::nothrow) MessageQueue<uint8_t, kSynchronizedReadWrite>(VT_HIDL_MAX_DATA_SIZE, true /* blocking */));

    if ((mReadFmq == nullptr) || (mReadFmq->isValid() == false)) {

        VT_HIDL_LOGD("[IVT] configureFmqSyncRead fail");

        _hidl_cb(false /* ret */, vtHiDLFmq::Descriptor());

    } else {

        VT_HIDL_LOGD("[IVT] configureFmqSyncRead success");

        auto evFlagWordPtr = mReadFmq->getEventFlagWord();
        if (evFlagWordPtr != nullptr) {
            std::atomic_init(evFlagWordPtr,
                             static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL));

            VT_HIDL_LOGD("[IVT] configureFmqSyncRead, evFlagWordPtr init done");

        } else {

            VT_HIDL_LOGD("[IVT] configureFmqSyncRead, evFlagWordPtr = null");
        }

        _hidl_cb(true /* ret */, *mReadFmq->getDesc());
    }

    VT_HIDL_SET(mInitStatus, VT_HIDL_INIT_STATUS_SET_ReadQ_DONE);

    if (VT_HIDL_INIT_STATUS_ALL_DONE == mInitStatus) {
        initialization();
    } else {
        VT_HIDL_LOGI("[IVT] configureFmqSyncRead, getInstance wait client");
    }

    return Void();
}

Return<void> VideoTelephony::configureFmqSyncWrite(configureFmqSyncWrite_cb _hidl_cb) {

    VT_HIDL_LOGD("[IVT] configureFmqSyncWrite");

    // skip operation if the config has been done before
    // it mean the VTS may restart, just use the same queue
    if (VT_HIDL_IS_SET(mInitStatus, VT_HIDL_INIT_STATUS_SET_WriteQ_DONE)) {

        VT_HIDL_LOGD("[IVT] configureFmqSyncRead, already done, skip");

        _hidl_cb(true /* ret */, *mWriteFmq->getDesc());

        return Void();
    }

    mWriteFmq.reset(new (std::nothrow) MessageQueue<uint8_t, kSynchronizedReadWrite>(VT_HIDL_MAX_DATA_SIZE, true /* blocking */));

    if ((mWriteFmq == nullptr) || (mWriteFmq->isValid() == false)) {

        VT_HIDL_LOGD("[IVT] configureFmqSyncWrite fail");

        _hidl_cb(false /* ret */, vtHiDLFmq::Descriptor());

    } else {

        VT_HIDL_LOGD("[IVT] configureFmqSyncWrite success");

        auto evFlagWordPtr = mWriteFmq->getEventFlagWord();
        if (evFlagWordPtr != nullptr) {
            std::atomic_init(evFlagWordPtr,
                             static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL));

            VT_HIDL_LOGD("[IVT] configureFmqSyncWrite, evFlagWordPtr init done");

        }  else {

            VT_HIDL_LOGD("[IVT] configureFmqSyncWrite, evFlagWordPtr = null");
        }

        _hidl_cb(true /* ret */, *mWriteFmq->getDesc());
    }

    VT_HIDL_SET(mInitStatus, VT_HIDL_INIT_STATUS_SET_WriteQ_DONE);

    if (VT_HIDL_INIT_STATUS_ALL_DONE == mInitStatus) {
        initialization();
    } else {
        VT_HIDL_LOGI("[IVT] configureFmqSyncWrite, getInstance wait client");
    }

    return Void();
}

int VideoTelephony::getMode() {

    return mMode;
}

int VideoTelephony::isImsVideoCallon() {
    char vilte_prop_val[100] = {0};
    char viwifi_prop_val[100] = {0};

    property_get("persist.vendor.vilte_support", vilte_prop_val, "0");
    property_get("persist.vendor.viwifi_support", viwifi_prop_val, "0");

    if (atoi(vilte_prop_val) == 1 || atoi(viwifi_prop_val) == 1) {
        VT_HIDL_LOGD("[IVT] isImsVideoCallon : True");
        return VT_HIDL_TRUE;
    } else {
        VT_HIDL_LOGD("[IVT] isImsVideoCallon : False");
        return VT_HIDL_FALSE;
    }
}

int VideoTelephony::isIMCBrun() {
    char prop_val[100] = {0}; // "stopped" or "running"

    property_get("init.svc.vendor.volte_imcb", prop_val, "running");

    if (!strcmp(prop_val, "running")) {
        VT_HIDL_LOGD("[IVT] isIMCBrun : True");
        return VT_HIDL_TRUE;
    } else {
        VT_HIDL_LOGD("[IVT] isIMCBrun : False");
        return VT_HIDL_FALSE;
    }
}

VideoTelephony* VideoTelephony::getInstance() {

    VT_HIDL_LOGD("[IVT] getInstance");

    if (sVideoTelephony == NULL) {

        VT_HIDL_LOGD("[IVT] getInstance : new instance");

        sVideoTelephony = new VideoTelephony();
        sVideoTelephony->mInitStatus = 0;
	sVideoTelephony->mMode = 0;
    }

    return sVideoTelephony;
}

int VideoTelephony::initialization() {

    VT_HIDL_LOGD("[IVT] initialization");

    // Thread for read message frome VTS
    if (VideoTelephony::isImsVideoCallon()) {

        pthread_attr_t  attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int ret = pthread_create(&g_vt_vts_thd, &(attr), VT_Reader_Thread, NULL);
        pthread_attr_destroy(&attr);
        if (ret < 0) {
            VT_HIDL_LOGE("[IVT] [VT THREAD] Fail to create thread");
            return false;
        }
    }

    // Thread for recv IMCB message
    if (VideoTelephony::isImsVideoCallon()) {

        pthread_attr_t  attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int ret = pthread_create(&g_vt_imcb_thd, &(attr), VT_IMCB_Thread, NULL);
        pthread_attr_destroy(&attr);
        if (ret < 0) {
            VT_HIDL_LOGE("[IVT] [VT THREAD] Fail to create imcb thread");
            return false;
        }
    }

    return true;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

IVideoTelephony* HIDL_FETCH_IVideoTelephony(const char* /* name */) {

    //return new VideoTelephony();
    return VideoTelephony::getInstance();
}

void VT_Bind(int *fd, const char *des) {

    if (fd[0] != 0)
        close(fd[0]); //won't close stdin

    VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_Bind] des = %s", des);

    fd[0] = socket_local_client(des, ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);

    VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_Bind] des = %s fd = %d", des, fd[0]);

    int retry = 0;
    while (fd[0] < 0) {
        retry++;

        if (!strcmp(des, "volte_imsvt1") && !VideoTelephony::isIMCBrun())
            usleep(3000 * 1000);
        else
            usleep(500 * 1000);

        VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_Bind] des = %s Fail to connect . retry count: %d", des, retry);
        fd[0] = socket_local_client(des, ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
    }
    VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_Bind] des = %s Success fd: %d", des, fd[0]);

    VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_Bind] des = %s initialize communication", des);
}

static void *VT_Reader_Thread(void *arg) {

    VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_READ_Thread] start");

    VideoTelephony* vt = VideoTelephony::getInstance();

    while (1) {

        // the view of read/write is from user
        // user will wrtie data to writer Q, so we read from writer Q

        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_READ_Thread] read from VTS");

        android::hardware::EventFlag* efGroup = nullptr;
        std::atomic<uint32_t> mFw;
        android::status_t status = android::hardware::EventFlag::createEventFlag(&mFw, &efGroup);

        int type = 0;
        vt->mWriteFmq->readBlocking(
                (uint8_t*) (&type),
                4,
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                efGroup*/);

        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_READ_Thread] type : %d", type);

        int len = 0;
        vt->mWriteFmq->readBlocking(
                (uint8_t*) (&len),
                4,
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                efGroup*/);

        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_READ_Thread] len : %d", len);

        unsigned char* outBuffer = (unsigned char* ) malloc((sizeof(char) * len) + 1);

        int ret = vt->mWriteFmq->readBlocking(
                (uint8_t*) (outBuffer),
                len,
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                efGroup*/);
        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_READ_Thread] readblock ret: %d", ret);

        status = android::hardware::EventFlag::deleteEventFlag(&efGroup);

        if (vt->getMode() == 1) {

            VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_READ_Thread] write to IMCB");

            if (send(g_vt_imcb_fd, (const void*)&type, sizeof(int), 0) != sizeof(int)) {

                VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_READ_Thread] [IMCB] send fail (type) / fd: %d, errCode: %d", g_vt_imcb_fd, errno);

                free(outBuffer);
                continue;
            }

            if (send(g_vt_imcb_fd, (const void*)&len, sizeof(int), 0) != sizeof(int)) {

                VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_READ_Thread] [IMCB] send fail (len) / fd: %d, errCode: %d", g_vt_imcb_fd, errno);

                free(outBuffer);
                continue;
            }


            int lenSent = 0;
            int retLen = send(g_vt_imcb_fd, (const void*)outBuffer, len, 0);

            while (retLen != (len - lenSent)) {
                lenSent += retLen;

                // send remaining data if errno = 0
                if (errno == 0) {
                    VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_READ_Thread] [IMCB] send remaining (data) / fd: %d, retLen: %d, errCode: %d", g_vt_imcb_fd, retLen, errno);
                    retLen = send(g_vt_imcb_fd, (const void*)(outBuffer + lenSent), (len - lenSent), 0);

                } else {
                    VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_READ_Thread] [IMCB] send fail (data) / fd: %d, retLen: %d, errCode: %d", g_vt_imcb_fd, retLen, errno);
                    break;
                }
            }

        }
        free(outBuffer);
    }
    return 0;
}

static void *VT_IMCB_Thread(void *arg) {

    VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_IMCB_Thread] start");

    VT_Bind(&g_vt_imcb_fd, "volte_imsvt1");

    VideoTelephony* vt = VideoTelephony::getInstance();

    while (1) {
        int msg_type;
        unsigned int recv_length = 0;
        unsigned char* outBuffer = NULL;

        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_IMCB_Thread] read from IMCB");

        int ret = recv(g_vt_imcb_fd, &msg_type, sizeof(msg_type), 0);
        if (ret != sizeof(msg_type)) {

            VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_IMCB_Thread] recv fail (type) / ret = %d / fd: %d / err: %s", ret, g_vt_imcb_fd, strerror(errno));

            restartProcess();
            VT_Bind(&g_vt_imcb_fd, "volte_imsvt1");
            continue;
        }

        ret = recv(g_vt_imcb_fd, &recv_length, sizeof(recv_length), 0);
        if (ret != sizeof(recv_length) || (UINT_MAX / sizeof(char) - 1) <= recv_length) {

            VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_IMCB_Thread] recv fail (len) / ret = %d / fd: %d / err: %s", ret, g_vt_imcb_fd, strerror(errno));

            restartProcess();
            VT_Bind(&g_vt_imcb_fd, "volte_imsvt1");
            continue;
        }

        outBuffer = (unsigned char* ) malloc((sizeof(char) * recv_length) + 1);
        if (NULL == outBuffer) {
            VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_IMCB_Thread] malloc fail, recv_length = %d", recv_length);

            restartProcess();
            VT_Bind(&g_vt_imcb_fd, "volte_imsvt1");
            continue;
        }

        ret = recv(g_vt_imcb_fd, reinterpret_cast<void *>(outBuffer), recv_length, 0);
        if (ret != recv_length) {

            VT_HIDL_LOGE("[IVT] [VT THREAD] [VT_IMCB_Thread] recv fail (data) / ret = %d / fd: %d / err: %s", ret, g_vt_imcb_fd, strerror(errno));

            free(outBuffer);

            restartProcess();
            VT_Bind(&g_vt_imcb_fd, "volte_imsvt1");
            continue;
        }

        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_IMCB_Thread] read from IMCB done");

        // the view of read/write is from user
        // user will read data from reader Q, so we write to reader Q

        android::hardware::EventFlag* efGroup = nullptr;
        std::atomic<uint32_t> mFw;
        android::status_t status = android::hardware::EventFlag::createEventFlag(&mFw, &efGroup);

        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_IMCB_Thread] write to VTS (status = %d)", status);
        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_IMCB_Thread] write to VTS (efGroup = %d)", efGroup);

        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_IMCB_Thread] write to VTS (type)");

        vt->mReadFmq->writeBlocking(
                (uint8_t*)&msg_type,
                4,
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                efGroup*/);

        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_IMCB_Thread] write to VTS (len)");

        vt->mReadFmq->writeBlocking(
                (uint8_t*)&recv_length,
                4,
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                efGroup*/);

        VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_IMCB_Thread] write to VTS (buffer)");

        vt->mReadFmq->writeBlocking(
                (uint8_t*)outBuffer,
                recv_length,
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_FULL),
                static_cast<uint32_t>(IVideoTelephony::EventFlagBits::FMQ_NOT_EMPTY),
                VT_HIDL_READ_WRITE_WAITTING_TIME /* timeOutNanos *//*,
                efGroup*/);

        free(outBuffer);

        status = android::hardware::EventFlag::deleteEventFlag(&efGroup);
    }

    close(g_vt_imcb_fd);

    VT_HIDL_LOGI("[IVT] [VT THREAD] [VT_IMCB_Thread] end");

    return 0;
}

static void restartProcess(void) {
    return;
}



}  // namespace implementation
}  // namespace V1_0
}  // namespace videotelephony
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
