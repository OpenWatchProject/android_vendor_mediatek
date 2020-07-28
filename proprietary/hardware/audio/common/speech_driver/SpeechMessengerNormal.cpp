#include <SpeechMessengerNormal.h>

#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <pthread.h>

#include <cutils/properties.h> /* for PROPERTY_KEY_MAX */

#include <log/log.h>

extern "C" {
#include <hardware/ccci_intf.h>
}


#include <audio_memory_control.h>

#include <AudioAssert.h>

#include <SpeechType.h>

#include <SpeechUtility.h>

#include <SpeechMessageID.h>

#include <SpeechShareMemBase.h>

#include <SpeechCCCIType.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechMessengerNormal"


namespace android {


/*
 * =============================================================================
 *                     utility for modem
 * =============================================================================
 */

static CCCI_MD get_ccci_md_number(const modem_index_t mModemIndex) {
    CCCI_MD ccci_md = MD_SYS1;

    switch (mModemIndex) {
    case MODEM_1:
        ccci_md = MD_SYS1;
        break;
    case MODEM_2:
        ccci_md = MD_SYS2;
        break;
    default:
        ccci_md = MD_SYS1;
    }
    return ccci_md;
}

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

typedef uint32_t ccci_channel_t;

enum { /* ccci_channel_t */
#if 0 /* 80020004 or 0x80030004 */
    CCCI_M2A_CHANNEL_NUMBER = 4,
#endif
    CCCI_A2M_CHANNEL_NUMBER = 5
};


typedef uint8_t ccci_msg_buffer_t;

enum { /* ccci_msg_buffer_t */
    CCCI_MSG_BUFFER_TYPE_MAILBOX       = 0,
    CCCI_MSG_BUFFER_TYPE_AP_PAYLOAD    = 1,
    CCCI_MSG_BUFFER_TYPE_MD_PAYLOAD    = 2
};


typedef struct ccci_mail_box_t {
    uint32_t magic;
    uint16_t param_16bit;
    uint16_t msg_id;        /* sph_msg_id_t */
    uint32_t ch;            /* ccci_channel_t */
    uint32_t param_32bit;
} ccci_mail_box_t;


typedef struct ccci_ap_payload_t {
    uint32_t offset;        /* always 0 */
    uint32_t payload_size;
    uint32_t ch;
    uint16_t param_16bit;
    uint16_t msg_id;        /* sph_msg_id_t */

    uint16_t data_sync;     /* CCCI_AP_PAYLOAD_SYNC */
    uint16_t data_type;     /* share_buff_data_type_t */
    uint16_t data_size;

    uint8_t  payload_data[CCCI_MAX_AP_PAYLOAD_DATA_SIZE];
} ccci_ap_payload_t;


typedef struct ccci_md_payload_t {
    uint32_t offset;
    uint32_t message_size;
    uint32_t ch;
    uint16_t param_16bit;
    uint16_t msg_id;        /* sph_msg_id_t */

    uint16_t data_sync;     /* CCCI_MD_PAYLOAD_SYNC */
    uint16_t data_type;     /* share_buff_data_type_t */
    uint16_t data_size;
    uint16_t idx;
    uint16_t total_idx;

    uint8_t  data[CCCI_MAX_MD_PAYLOAD_DATA_SIZE];
} ccci_md_payload_t;


typedef struct ccci_msg_t {
    union {
        uint8_t           buffer[CCCI_MAX_BUF_SIZE]; /* for ccci read/write */
        ccci_mail_box_t   mail_box;
        ccci_ap_payload_t ap_payload;
        ccci_md_payload_t md_payload;
    };

    uint8_t               buffer_type; /* ccci_msg_buffer_t */
    uint16_t              buffer_size;
} ccci_msg_t;

#define MAX_CCCI_TIMEOUT_MS (100)

/*
 * =============================================================================
 *                     class implementation
 * =============================================================================
 */

SpeechMessengerNormal::SpeechMessengerNormal(const modem_index_t modem_index) {
    int retval = 0;

    ALOGV("%s(), modem_index: %d", __FUNCTION__, modem_index);
    mModemIndex = modem_index;

    mCcciDeviceHandler = -1;
    mSpeechShareMem = NULL;
    mCcciShareMemoryHandler = -1;
    hFormatShareMemoryThread = 0;

    mCcciMsgSend = NULL;
    mCcciMsgRead = NULL;

    AUDIO_ALLOC_STRUCT(ccci_msg_t, mCcciMsgSend);
    AUDIO_ALLOC_STRUCT(ccci_msg_t, mCcciMsgRead);

#ifdef MTK_CCCI_SHARE_BUFFER_SUPPORT
    mSpeechShareMem =  SpeechShareMemBase::createInstance();
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
    }
#endif

    retval = checkCcciStatusAndRecovery();
    if (retval != 0) {
        ALOGW("%s(), ccci open fail!!", __FUNCTION__);
    }
}


SpeechMessengerNormal::~SpeechMessengerNormal() {
#ifdef MTK_CCCI_SHARE_BUFFER_SUPPORT
    AL_LOCK(mShareMemoryHandlerLock);
    closeShareMemory();
    AL_UNLOCK(mShareMemoryHandlerLock);
    SpeechShareMemBase::destroyInstance(mSpeechShareMem);
    mSpeechShareMem = NULL;
#endif

    AL_LOCK(mCcciHandlerLock);
    closeCcciDriver();
    AL_UNLOCK(mCcciHandlerLock);

    AUDIO_FREE_POINTER(mCcciMsgSend);
    AUDIO_FREE_POINTER(mCcciMsgRead);
}


int SpeechMessengerNormal::openCcciDriver() {
    CCCI_USER ccci_user = USR_AUDIO_RX;
    CCCI_MD   ccci_md   = get_ccci_md_number(mModemIndex);

    const uint32_t k_max_device_name_size = 32;
    char dev_name[k_max_device_name_size];
    memset(dev_name, 0, sizeof(dev_name));

    if (mCcciDeviceHandler >= 0) {
        ALOGD("%s(), ccci_md: %d, mCcciDeviceHandler: %d already open",
              __FUNCTION__, ccci_md, mCcciDeviceHandler);
        return 0;
    }

    // open ccci device driver
    strncpy(dev_name,
            ccci_get_node_name(ccci_user, ccci_md),
            sizeof(dev_name) - 1);

    mCcciDeviceHandler = open(dev_name, O_RDWR);
    if (mCcciDeviceHandler < 0) {
        ALOGE("%s(), open(%s) fail!! mCcciDeviceHandler: %d, errno: %d", __FUNCTION__,
              dev_name, (int32_t)mCcciDeviceHandler, errno);
        return -ENODEV;
    }

    ALOGD("%s(), ccci_md: %d, dev_name: \"%s\", mCcciDeviceHandler: %d",
          __FUNCTION__, ccci_md, dev_name, mCcciDeviceHandler);
    return 0;
}


int SpeechMessengerNormal::closeCcciDriver() {
    ALOGD("%s(), mCcciDeviceHandler: %d", __FUNCTION__, mCcciDeviceHandler);

    if (mCcciDeviceHandler >= 0) {
        close(mCcciDeviceHandler);
        mCcciDeviceHandler = -1;
    }
    return 0;
}


int SpeechMessengerNormal::openShareMemory() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif

    if (mCcciShareMemoryHandler >= 0) {
        ALOGD("%s(), mModemIndex: %d, mCcciShareMemoryHandler: %d already open",
              __FUNCTION__, mModemIndex, mCcciShareMemoryHandler);
        return 0;
    }

    // get share momoey address
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
        return -ENODEV;
    }

    mCcciShareMemoryHandler = mSpeechShareMem->openShareMemory(mModemIndex);
    if (mCcciShareMemoryHandler < 0) {
        ALOGE("%s(), mModemIndex(%d) fail!! mCcciShareMemoryHandler: %d, errno: %d", __FUNCTION__,
              mModemIndex, (int32_t)mCcciShareMemoryHandler, errno);
        return -ENODEV;
    }

    int retval = 0;
    if (checkModemReady() == true) {
        retval = formatShareMemory();
    } else {
        retval = pthread_create(&hFormatShareMemoryThread, NULL,
                                SpeechMessengerNormal::formatShareMemoryThread,
                                (void *)this);
        ASSERT(retval == 0);
    }

    return retval;
}


int SpeechMessengerNormal::closeShareMemory() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
        return -ENODEV;
    }
    mSpeechShareMem->closeShareMemory();
    mCcciShareMemoryHandler = -1;


    return 0;
}


int SpeechMessengerNormal::checkCcciStatusAndRecovery() {
    const uint8_t k_max_try_cnt = 200;
    uint8_t try_cnt = 0;

    int retval = 0;

    for (try_cnt = 0; try_cnt < k_max_try_cnt; try_cnt++) {
        AL_LOCK(mCcciHandlerLock);
        if (mCcciDeviceHandler >= 0) {
            retval = 0;
        } else {
            if (try_cnt == 0) {
                ALOGW("%s() mCcciDeviceHandler: %d", __FUNCTION__, mCcciDeviceHandler);
            }
            retval = openCcciDriver();
        }
        AL_UNLOCK(mCcciHandlerLock);

        if (retval == 0) {
            break;
        } else {
            ALOGW("%s(), #%d, sleep 10 ms & retry openCcciDriver", __FUNCTION__, try_cnt);
            usleep(10 * 1000); /* 10 ms */
        }
    }

    if (retval != 0) {
        ALOGW("%s(), ccci driver not init!!", __FUNCTION__);
        return retval;
    }

#ifdef MTK_CCCI_SHARE_BUFFER_SUPPORT
    for (try_cnt = 0; try_cnt < k_max_try_cnt; try_cnt++) {
        AL_LOCK(mShareMemoryHandlerLock);
        if (mCcciShareMemoryHandler >= 0) {
            retval = 0;
        } else {
            if (try_cnt == 0) {
                ALOGW("%s() mCcciShareMemoryHandler: %d", __FUNCTION__, mCcciShareMemoryHandler);
            }
            retval = openShareMemory();
        }
        AL_UNLOCK(mShareMemoryHandlerLock);

        if (retval == 0) {
            break;
        } else {
            ALOGW("%s(), #%d, sleep 10 ms & retry openShareMemory", __FUNCTION__, try_cnt);
            usleep(10 * 1000); /* 10 ms */
        }
    }

    if (retval != 0) {
        ALOGW("%s(), ccci share memory not init", __FUNCTION__);
        return retval;
    }
#endif

    return 0;
}


void *SpeechMessengerNormal::formatShareMemoryThread(void *arg) {
    SpeechMessengerNormal *pSpeechMessenger = NULL;

    uint32_t try_cnt = 0;
    const uint32_t k_max_try_cnt = 3000; /* (5min == 300sec) / 100 ms */

    /* detach pthread */
    pthread_detach(pthread_self());

    pSpeechMessenger = static_cast<SpeechMessengerNormal *>(arg);
    if (pSpeechMessenger == NULL) {
        ALOGE("%s(), NULL!! pSpeechMessenger %p", __FUNCTION__, pSpeechMessenger);
        WARNING("cast fail!!");
        goto FORMAT_SHM_THREAD_DONE;
    }

    /* wait until modem ready */
    for (try_cnt = 0; try_cnt < k_max_try_cnt; try_cnt++) {
        if (pSpeechMessenger->checkModemReady() == true) {
            break;
        }

        if (try_cnt == 0) { /* only print once at beginning */
            ALOGW("%s(), modem not ready...retry start", __FUNCTION__);
        }

        usleep(100 * 1000); /* 100 ms */
    }

    if (try_cnt == k_max_try_cnt) {
        ALOGW("%s(), #%u checkModemReady retry fail...", __FUNCTION__, try_cnt);
    } else if (try_cnt != 0) {
        ALOGW("%s(), #%u checkModemReady done", __FUNCTION__, try_cnt);
    }

    /* format share memory */
    pSpeechMessenger->formatShareMemory();



FORMAT_SHM_THREAD_DONE:
    ALOGD("%s(), terminated", __FUNCTION__);
    pthread_exit(NULL);
    return NULL;
}


int SpeechMessengerNormal::formatShareMemory() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return -ENODEV;
#endif
    if (checkModemReady() == false) {
        ALOGW("%s(), checkModemReady fail...", __FUNCTION__);
        return -EFAULT;
    }
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
        return -ENODEV;
    }
    return mSpeechShareMem->formatShareMemory();
}


bool SpeechMessengerNormal::checkModemReady() {
    modem_status_t modem_status = MODEM_STATUS_INVALID;
    unsigned int status_value = 0;
    int retval = 0;
    struct timespec ts_start;
    struct timespec ts_stop;
    uint64_t time_diff_msg = 0;

    if (mCcciDeviceHandler < 0) {
        ALOGW("%s(), ccci not init!!", __FUNCTION__);
        return false; // -ENODEV;
    }

    audio_get_timespec_monotonic(&ts_start);
    retval = ::ioctl(mCcciDeviceHandler, CCCI_IOC_GET_MD_STATE, &status_value);
    audio_get_timespec_monotonic(&ts_stop);
    time_diff_msg = get_time_diff_ms(&ts_start, &ts_stop);
    if (time_diff_msg >= MAX_CCCI_TIMEOUT_MS) {
        ALOGE("%s(), check MD ready ioctl time %ju ms is too long", __FUNCTION__,
              time_diff_msg);
    }

    if (retval < 0) {
        ALOGW("%s(), ioctl CCCI_IOC_GET_MD_STATE fail!! retval: %d, errno: %d",
              __FUNCTION__, retval, errno);
        return false; // retval;
    }

    if (status_value <= MODEM_STATUS_EXPT) {
        modem_status = (modem_status_t)(status_value & 0xFF);
    }

    static bool dump_modem_fail_log = false; /* avoid to dump too much error log */
    if (modem_status == MODEM_STATUS_READY) {
        dump_modem_fail_log = false;
    } else {
        if (dump_modem_fail_log == false) {
            ALOGW("%s(), modem_status %d != MODEM_STATUS_READY", __FUNCTION__, modem_status);
            dump_modem_fail_log = true;
        }
    }

    return (modem_status == MODEM_STATUS_READY);
}


bool SpeechMessengerNormal::checkModemAlive() {
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
        return false;
    }
    return mSpeechShareMem->checkModemAlive();
}


uint32_t SpeechMessengerNormal::getMaxApPayloadDataSize() {
    return CCCI_MAX_AP_PAYLOAD_DATA_SIZE;
}


uint32_t SpeechMessengerNormal::getMaxMdPayloadDataSize() {
    return CCCI_MAX_MD_PAYLOAD_DATA_SIZE;
}


int SpeechMessengerNormal::sendSpeechMessage(sph_msg_t *p_sph_msg) {
    AL_AUTOLOCK(mCcciMsgSendLock);

    struct timespec ts_start;
    struct timespec ts_stop;
    uint64_t time_diff_msg = 0;

    int length_write = 0;
    int retval = 0;
    int try_cnt = 0;
    const int k_max_try_cnt = 20;

    if (p_sph_msg == NULL) {
        ALOGE("%s(), p_sph_msg = NULL, return", __FUNCTION__);
        return -EFAULT;
    }

    audio_get_timespec_monotonic(&ts_start);
    retval = checkCcciStatusAndRecovery();
    audio_get_timespec_monotonic(&ts_stop);
    time_diff_msg = get_time_diff_ms(&ts_start, &ts_stop);
    if (time_diff_msg >= MAX_CCCI_TIMEOUT_MS) {
        ALOGE("%s(), msg 0x%x check ccci time %ju ms is too long", __FUNCTION__,
              p_sph_msg->msg_id, time_diff_msg);
    }

    if (retval != 0) {
        PRINT_SPH_MSG(ALOGE, "send msg failed!! ccci not ready", p_sph_msg);
        return retval;
    }

    if (checkModemReady() == false) {
        PRINT_SPH_MSG(ALOGE, "send msg failed!! modem not ready", p_sph_msg);
        return -EPIPE;
    }

    /* parsing speech message */
    memset(mCcciMsgSend, 0, sizeof(ccci_msg_t));
    retval = speechMessageToCcciMessage(p_sph_msg, mCcciMsgSend);

    if (retval != 0) {
        ALOGE("%s(), speechMessageToCcciMessage fail!! return", __FUNCTION__);
        return retval;
    }

    audio_get_timespec_monotonic(&ts_start);
    /* send message */
    for (try_cnt = 0; try_cnt < k_max_try_cnt; try_cnt++) {
        length_write = write(mCcciDeviceHandler,
                             mCcciMsgSend->buffer,
                             mCcciMsgSend->buffer_size);
        if (length_write == mCcciMsgSend->buffer_size) {
            retval = 0;
            break;
        }

        if (checkModemReady() == false) {
            PRINT_SPH_MSG(ALOGE, "write msg failed!! modem not ready", p_sph_msg);
            retval = -EPIPE;
            break;
        }

        retval = -EBADMSG;
        ALOGW("%s(), try_cnt: #%d, msg_id: 0x%x, length_write: %d, errno: %d",
              __FUNCTION__, try_cnt, p_sph_msg->msg_id, length_write, errno);
        usleep(2 * 1000);
    }
    audio_get_timespec_monotonic(&ts_stop);
    time_diff_msg = get_time_diff_ms(&ts_start, &ts_stop);
    if (time_diff_msg >= MAX_CCCI_TIMEOUT_MS) {
        ALOGE("%s(), msg 0x%x write through ccci time %ju ms is too long", __FUNCTION__,
              p_sph_msg->msg_id, time_diff_msg);
    }
    return retval;
}


int SpeechMessengerNormal::readSpeechMessage(sph_msg_t *p_sph_msg) {
    AL_AUTOLOCK(mCcciMsgReadLock);

    int length_read = 0;
    int retval = 0;

    if (p_sph_msg == NULL) {
        ALOGE("%s(), p_sph_msg = NULL, return", __FUNCTION__);
        return -EFAULT;
    }

    retval = checkCcciStatusAndRecovery();
    if (retval != 0) {
        PRINT_SPH_MSG(ALOGE, "read msg failed!! ccci not ready", p_sph_msg);
        return retval;
    }

    /* read message */
    memset(mCcciMsgRead->buffer, 0, sizeof(mCcciMsgRead->buffer));
    length_read = read(mCcciDeviceHandler, mCcciMsgRead->buffer, sizeof(mCcciMsgRead->buffer));
    if (length_read < CCCI_MAILBOX_SIZE) { /* at least one mailbox at once */
        if (checkModemReady() == true) {
            ALOGV("%s(), read ccci fail!! modem ready, length_read: %d, errno: %d",
                  __FUNCTION__, (int32_t)length_read, errno);
            return -ETIMEDOUT;
        } else {
            ALOGW("%s(), read ccci fail!! modem invalid, length_read: %d, errno: %d",
                  __FUNCTION__, (int32_t)length_read, errno);
            return -EPIPE;
        }
    }

    mCcciMsgRead->buffer_size = length_read;

    /* parsing ccci message */
    if (mCcciMsgRead->mail_box.magic == CCCI_MAILBOX_MAGIC) {
        mCcciMsgRead->buffer_type = CCCI_MSG_BUFFER_TYPE_MAILBOX;
    } else {
        mCcciMsgRead->buffer_type = CCCI_MSG_BUFFER_TYPE_MD_PAYLOAD;
    }
    retval = ccciMessageToSpeechMessage(mCcciMsgRead, p_sph_msg);

    return retval;
}


int SpeechMessengerNormal::speechMessageToCcciMessage(
    sph_msg_t *p_sph_msg, ccci_msg_t *p_ccci_msg) {

    int retval = 0;

    if (!p_ccci_msg || !p_sph_msg) {
        ALOGW("%s(), NULL!! return", __FUNCTION__);
        return -EFAULT;
    }

    ccci_mail_box_t   *p_mail_box   = &p_ccci_msg->mail_box;
    ccci_ap_payload_t *p_ap_payload = &p_ccci_msg->ap_payload;

    switch (p_sph_msg->buffer_type) {
    case SPH_MSG_BUFFER_TYPE_MAILBOX:
        p_mail_box->magic = CCCI_MAILBOX_MAGIC;
        p_mail_box->param_16bit = p_sph_msg->param_16bit;
        p_mail_box->msg_id = p_sph_msg->msg_id;
        p_mail_box->ch = CCCI_A2M_CHANNEL_NUMBER;
        p_mail_box->param_32bit = p_sph_msg->param_32bit;

        /* date size to be writed */
        p_ccci_msg->buffer_size = CCCI_MAILBOX_SIZE;
        retval = 0;
        break;
    case SPH_MSG_BUFFER_TYPE_PAYLOAD:
        p_ap_payload->offset = 0; /* always 0 */
        p_ap_payload->payload_size = CCCI_MAX_AP_PAYLOAD_HEADER_SIZE + p_sph_msg->payload_data_size;
        p_ap_payload->ch = CCCI_A2M_CHANNEL_NUMBER;
        p_ap_payload->param_16bit  = CCCI_MAX_AP_PAYLOAD_HEADER_SIZE + p_sph_msg->payload_data_size;
        p_ap_payload->msg_id = p_sph_msg->msg_id;


        p_ap_payload->data_sync = CCCI_AP_PAYLOAD_SYNC;
        p_ap_payload->data_type = p_sph_msg->payload_data_type;
        p_ap_payload->data_size = p_sph_msg->payload_data_size;

        if (p_sph_msg->payload_data_addr == NULL) {
            ALOGE("%s(), payload_data_addr == NULL!!", __FUNCTION__);
            retval = -ENODEV;
            break;
        }

        if (p_sph_msg->payload_data_size > CCCI_MAX_AP_PAYLOAD_DATA_SIZE) {
            ALOGE("%s(), payload_data_size %d > %d!!", __FUNCTION__,
                  p_sph_msg->payload_data_size,
                  CCCI_MAX_AP_PAYLOAD_DATA_SIZE);
            retval = -ENOMEM;
            break;
        }
        memcpy(p_ap_payload->payload_data,
               p_sph_msg->payload_data_addr,
               p_sph_msg->payload_data_size);

        /* date size to be writed */
        p_ccci_msg->buffer_size = CCCI_MAILBOX_SIZE + p_ap_payload->payload_size;
        retval = 0;
        break;
    default:
        ALOGW("%s(), not support type %d!!", __FUNCTION__, p_sph_msg->buffer_type);
        retval = -EINVAL;
    }

    return retval;
}


int SpeechMessengerNormal::ccciMessageToSpeechMessage(
    ccci_msg_t *p_ccci_msg, sph_msg_t *p_sph_msg) {

    int retval = 0;

    if (!p_ccci_msg || !p_sph_msg) {
        ALOGW("%s(), NULL!! return", __FUNCTION__);
        return -EFAULT;
    }

    ccci_mail_box_t   *p_mail_box   = &p_ccci_msg->mail_box;
    ccci_md_payload_t *p_md_payload = &p_ccci_msg->md_payload;

    switch (p_ccci_msg->buffer_type) {
    case CCCI_MSG_BUFFER_TYPE_MAILBOX:
        ALOGV("%s(), type: mailbox, buffer_size: %d, ch:0x%x", __FUNCTION__, p_ccci_msg->buffer_size, p_mail_box->ch);
        ASSERT(p_ccci_msg->buffer_size == CCCI_MAILBOX_SIZE);

        p_sph_msg->buffer_type = SPH_MSG_BUFFER_TYPE_MAILBOX;
        p_sph_msg->msg_id = p_mail_box->msg_id;
        p_sph_msg->param_16bit = p_mail_box->param_16bit;
        p_sph_msg->param_32bit = p_mail_box->param_32bit;
        retval = 0;
        break;
    case CCCI_MSG_BUFFER_TYPE_MD_PAYLOAD:
        ALOGV("%s(), type: payload, buffer_size: %d, ch:0x%x", __FUNCTION__, p_ccci_msg->buffer_size, p_md_payload->ch);
        ASSERT(p_ccci_msg->buffer_size == p_md_payload->message_size);
        ASSERT(p_md_payload->message_size == (CCCI_MAILBOX_SIZE + CCCI_MAX_MD_PAYLOAD_HEADER_SIZE + p_md_payload->data_size));
        ASSERT(p_md_payload->data_sync == CCCI_MD_PAYLOAD_SYNC);
        ASSERT(p_md_payload->data_size <= CCCI_MAX_MD_PAYLOAD_DATA_SIZE);

        p_sph_msg->buffer_type = SPH_MSG_BUFFER_TYPE_PAYLOAD;
        p_sph_msg->msg_id = p_md_payload->msg_id;

        p_sph_msg->payload_data_type        = p_md_payload->data_type;
        p_sph_msg->payload_data_size        = p_md_payload->data_size;
        p_sph_msg->payload_data_addr        = p_md_payload->data;
        p_sph_msg->payload_data_idx         = p_md_payload->idx;
        p_sph_msg->payload_data_total_idx   = p_md_payload->total_idx;

        retval = 0;
        break;

    default:
        ALOGW("%s(), not support type %d!!", __FUNCTION__, p_ccci_msg->buffer_type);
        retval = -EINVAL;
    }

    return retval;
}


int SpeechMessengerNormal::resetShareMemoryIndex() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
        return 0;
    }
    return mSpeechShareMem->resetShareMemoryIndex();
}


int SpeechMessengerNormal::writeSphParamToShareMemory(const void *p_sph_param,
                                                      uint32_t sph_param_length,
                                                      uint32_t *p_write_idx) {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return -ENODEV;
#endif
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
        return -ENODEV;
    }
    return mSpeechShareMem->writeSphParamToShareMemory(p_sph_param,
                                                       sph_param_length,
                                                       p_write_idx);
}


int SpeechMessengerNormal::writeApDataToShareMemory(const void *p_data_buf,
                                                    uint16_t data_type,
                                                    uint16_t data_size,
                                                    uint16_t *p_payload_length,
                                                    uint32_t *p_write_idx) {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return -ENODEV;
#endif
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
        return -ENODEV;
    }
    return mSpeechShareMem->writeApDataToShareMemory(p_data_buf,
                                                     data_type,
                                                     data_size,
                                                     p_payload_length,
                                                     p_write_idx);
}


int SpeechMessengerNormal::readMdDataFromShareMemory(void *p_data_buf,
                                                     uint16_t *p_data_type,
                                                     uint16_t *p_data_size,
                                                     uint16_t payload_length,
                                                     uint32_t read_idx) {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return -ENODEV;
#endif
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
        return -ENODEV;
    }
    return mSpeechShareMem->readMdDataFromShareMemory(p_data_buf,
                                                      p_data_type,
                                                      p_data_size,
                                                      payload_length,
                                                      read_idx);
}


int SpeechMessengerNormal::getShareMemoryType() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return -ENODEV;
#endif
    if (mSpeechShareMem == NULL) {
        ALOGE("%s(), mSpeechShareMem == NULL!!", __FUNCTION__);
        return -ENODEV;
    }
    return mSpeechShareMem->getShareMemoryType();
}


} // end of namespace android

