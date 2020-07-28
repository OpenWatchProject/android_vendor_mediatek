#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechMessengerEVDO"
#include "SpeechMessengerEVDO.h"
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <sys/prctl.h>
#include <sys/resource.h>

#include <cutils/properties.h>

#include "SpeechDriverLAD.h"
#include <SpeechPcmMixerBGSPlayer.h>
#include "SpeechPcm2way.h"
#include "SpeechVMRecorder.h"
#include "AudioVolumeFactory.h"

#include "AudioALSAHardwareResourceManager.h"

#include "AudioALSACaptureDataProviderVoice.h"
#include "SpeechDataProcessingHandler.h"
#if defined(SPH_VCL_SUPPORT)
#include "SpeechVoiceCustomLogger.h"
#endif

#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
#include "SpeechDataEncrypter.h"
#endif

#include <SpeechUtility.h>

//#include "AudioMTKStreamInManager.h"
//#include "AudioResourceManager.h"
extern "C" {
#ifndef MTK_ECCCI_C2K
#include <c2kutils.h>
#endif
#include "hardware/ccci_intf.h"
}
#include <dlfcn.h>


#ifndef sph_msleep
#define sph_msleep(ms) usleep((ms)*1000)
#endif

namespace android {

/** CCCI driver & ioctl */


/** CCCI ioctl */
//static const char DEVICE_NAME[] = "/dev/ttySDIO4";

//const variable
#define BUFFER_SIZE 512
#define MAX_RETRY 5
#define READ_BUF_SIZE 4096


#define SPH_DUMP_STR_SIZE (500)

/** Property keys*/
static const char *kPropertyKeyModemStatus = "vendor.audiohal.modem_ext.status";
static const char* PROPERTY_KEY_NW_CODEC_INFO[2] = {"vendor.audiohal.ril.speech.codec.info", "vendor.audiohal.ril.hd.voice.status"};


/** CCCI channel No */
static const uint8_t    CCCI_M2A_CHANNEL = 4;
static const uint8_t    CCCI_A2M_CHANNEL = 5;

/** CCCI magic number */
static const uint32_t   CCCI_MAILBOX_MAGIC_NUMBER = 0xFFFFFFFF;

/** UART channel No */
static const uint32_t   CCCI_MAILBOX_COMMAND_NUMBER = 0x22224444;
static const uint32_t   CCCI_MAILBOX_DATA_NUMBER = 0x33336666;

#define EVDO_BUF_HEADER_FOOTER_SIZE (16)

static const uint32_t ap_working_buf_byte = 4096;

//static FILE *fout = NULL;
//static FILE *fout2 = NULL;

SpeechMessengerEVDO::SpeechMessengerEVDO(modem_index_t modem_index, SpeechDriverLAD *pLad) :
    mModemIndex(modem_index),
    mLad(pLad) {
    ALOGD("%s()", __FUNCTION__);
    CCCIEnable = false;
    mModemStatus = MODEM_STATUS_INVALID;

    fHdl = -1;

    mA2MShareBufLen = 0;
    mM2AShareBufLen = 0;

    mA2MShareBufBase = NULL;
    mM2AShareBufBase = NULL;

    mA2MShareBufEnd = NULL;
    mM2AShareBufEnd = NULL;
    mWasModemReset = false;

    memset(&mM2AShareBuf, 0, sizeof(mM2AShareBuf));

    //initial the message queue
    memset((void *)pQueue, 0, sizeof(pQueue));
    iQRead = 0;
    iQWrite = 0;

    mWaitAckMessageID = 0;
#ifdef USE_CCCI_SHARE_BUFFER
    CCCIBuf_Hdl = -1;
    mA2MParaShareBufBase = NULL;
    mA2MParaShareBufLen = 0;
#endif

    //initial modem side modem status
    mModemSideModemStatus = get_uint32_from_mixctrl(kPropertyKeyModemStatus);
    ALOGD("%s(), mModemIndex(%d), mModemSideModemStatus(0x%x)", __FUNCTION__, mModemIndex, mModemSideModemStatus);

    ResetSpeechParamAckCount();
    mECCCIShareBuf = NULL;
    mM2AShareBufRead = NULL;

    mModemStatus = MODEM_STATUS_READY;
    mWaitAckMessageID = 0;
    mIsModemResetDuringPhoneCall = false;
    mIsModemReset = false;
    mIsModemEPOF = false;
    mPcmRecordType.direction = RECORD_TYPE_UL;
    mCreatingSphThreadFlag = false;
    mWaitSphThreadFlag = false;
    hReadThread = 0;
    hSendSphThread = 0;
    hOpenMuxdDeviceThread = 0;
    mLogEnable = 0;

}

void SpeechMessengerEVDO::OpenMuxdDeviceUntilReady() {
    ALOGD("+%s()", __FUNCTION__);

    if (fHdl >= 0) {
        close(fHdl);
        fHdl = -1;
    }

    while (fHdl < 0) {
        sph_msleep(100);

        AL_LOCK(mfHdlLock);
        AL_LOCK(mGetModemCurrentStatusLock);
        char dev_node[32];

#ifdef MTK_ECCCI_C2K
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_AUDIO_RX, MD_SYS3));
        ALOGD("%s(), dev_node=%s", __FUNCTION__, dev_node);
        fHdl = open(dev_node, O_RDWR);
#else
        snprintf(dev_node, 32, "%s", "/dev/ttySDIO0");
        ALOGD("%s(), dev_node=%s", __FUNCTION__, dev_node);
        fHdl = open(dev_node, O_RDWR | O_NONBLOCK);
#endif

        if (fHdl < 0) {
            ALOGE("%s(), re-open(%s) fail, fHdl = %d, errno = %d", __FUNCTION__, dev_node, fHdl, errno);
        } else {
            ALOGD("%s(), re-open(%s) success, fHdl = %d", __FUNCTION__, dev_node, fHdl);
        }

        AL_UNLOCK(mfHdlLock);
        AL_UNLOCK(mGetModemCurrentStatusLock);
    }

    // Clean mWaitAckMessageID to avoid the queue receiving the wrong ack
    mWaitAckMessageID = 0;

    mIsModemResetDuringPhoneCall = false;
    mIsModemReset = false;
    mIsModemEPOF = false;

    ALOGD("-%s()", __FUNCTION__);
}


void *SpeechMessengerEVDO::OpenMuxdDeviceThread(void *arg) {
    pthread_detach(pthread_self());

    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    // Adjust thread priority
    int rtnPrio = setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    if (0 != rtnPrio) {
        ALOGE("[%s] failed, errno: %d, return=%d", __FUNCTION__, errno, rtnPrio);
    } else {
        ALOGD("%s setpriority ok, priority: %d", __FUNCTION__, ANDROID_PRIORITY_AUDIO);
    }
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    SpeechMessengerEVDO *pCCCI = (SpeechMessengerEVDO *)arg;

    pCCCI->OpenMuxdDeviceUntilReady();

    pthread_exit(NULL);
    return 0;
}


status_t SpeechMessengerEVDO::Initial() {
    ALOGD("%s()", __FUNCTION__);
    AL_AUTOLOCK(mfHdlLock);

    char dev_node[32];
#ifdef MTK_ECCCI_C2K
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_AUDIO_RX, MD_SYS3));
    ALOGD("%s(), dev_node=%s", __FUNCTION__, dev_node);
    fHdl = open(dev_node, O_RDWR);
#else
    snprintf(dev_node, 32, "%s", "/dev/ttySDIO0");
    ALOGD("%s(), dev_node=%s", __FUNCTION__, dev_node);
    fHdl = open(dev_node, O_RDWR | O_NONBLOCK);
#endif

    if (fHdl < 0) {
        ALOGE("%s(), open(%s) fail, fHdl = %d, errno=%d, return", __FUNCTION__, dev_node, fHdl, errno);
        mModemStatus = MODEM_STATUS_INIT;

        int ret = pthread_create(&hOpenMuxdDeviceThread, NULL, SpeechMessengerEVDO::OpenMuxdDeviceThread, (void *)this);
        if (ret != 0) {
            ALOGE("%s() create hOpenMuxdDeviceThread fail!!", __FUNCTION__);
            return UNKNOWN_ERROR;
        }
    } else {
        ALOGD("%s(), open(%s) success, fHdl = %d", __FUNCTION__, dev_node, fHdl);
        mModemStatus = MODEM_STATUS_READY;

        // Clean mWaitAckMessageID to avoid the queue receiving the wrong ack
        mWaitAckMessageID = 0;

        mIsModemResetDuringPhoneCall = false;
        mIsModemReset = false;
        mIsModemEPOF = false;
        mPcmRecordType.direction = RECORD_TYPE_UL;
        mCreatingSphThreadFlag = false;
        mWaitSphThreadFlag = false;

    }

#ifdef USE_CCCI_SHARE_BUFFER
    CCCI_MD md_id = MD_SYS3;
    unsigned char *share_addr = NULL;
    unsigned int length = 0;
    CCCIBuf_Hdl = ccci_smem_get(md_id, USR_SMEM_RAW_AUDIO, &share_addr, &length);
    if (CCCIBuf_Hdl < 0) {
        ALOGE("%s() fail to allocate share buffer for MD %d", __FUNCTION__, md_id);
        return UNKNOWN_ERROR;
    }
    ALOGD("%s(), share_addr %p, length %d", __FUNCTION__, share_addr, length);
    mA2MParaShareBufBase = (char *) share_addr;
    mA2MParaShareBufLen = length;
    InitA2MRawParaRingBuffer();
#endif

    // Get total share buffer length & base address
    const uint32_t share_buf_length = 32000;
    mECCCIShareBuf = new char[share_buf_length];

    char *share_buf_address =  mECCCIShareBuf;
    ALOGD("%s(), share_buf_address: %p, share_buf_length: %u", __FUNCTION__, share_buf_address, share_buf_length);

    mA2MShareBufLen = share_buf_length >> 1; // a2m buffer lengh should be half of share_buf_length
    mM2AShareBufLen = share_buf_length >> 1; // m2a buffer lengh should be half of share_buf_length

    mA2MShareBufBase = share_buf_address;
    mM2AShareBufBase = share_buf_address + mA2MShareBufLen;

    mA2MShareBufEnd = mA2MShareBufBase + mA2MShareBufLen;
    mM2AShareBufEnd = mM2AShareBufBase + mM2AShareBufLen;

    mM2AShareBufRead = mM2AShareBufBase;

    mM2AShareBuf.bufLen   = mM2AShareBufLen;
    mM2AShareBuf.pBufBase = mM2AShareBufBase;
    mM2AShareBuf.pRead    = mM2AShareBuf.pBufBase;
    mM2AShareBuf.pWrite   = mM2AShareBuf.pBufBase;

    mSphPCMBuf = new char[ap_working_buf_byte];
    mSphPCMBufEnd = mSphPCMBuf + ap_working_buf_byte;
    pcm_ring_buf.bufLen = ap_working_buf_byte;
    pcm_ring_buf.pBufBase = mSphPCMBuf;
    pcm_ring_buf.pRead  = pcm_ring_buf.pBufBase;
    pcm_ring_buf.pWrite = pcm_ring_buf.pRead;
    LastSentMessage = 0;
    LastNeedAckSentMessage = 0;
    /* create the CCCI event reading thread */
    CCCIEnable = true;
#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
    //trigger initialization
    SpeechDataEncrypter::GetInstance()->GetDumpStatus();
#endif
    mLogEnable = __android_log_is_loggable(ANDROID_LOG_DEBUG, LOG_TAG, ANDROID_LOG_INFO);

    return CreateReadingThread();
}

status_t SpeechMessengerEVDO::Deinitial() {
    ALOGD("%s()", __FUNCTION__);
    AL_AUTOLOCK(mfHdlLock);

    CCCIEnable = false;

    if (fHdl >= 0) {
        close(fHdl);
        fHdl = -1;
    }

    if (mECCCIShareBuf != NULL) {
        delete mECCCIShareBuf;
        mECCCIShareBuf = NULL;

        mA2MShareBufLen = 0;
        mM2AShareBufLen = 0;

        mA2MShareBufBase = NULL;
        mM2AShareBufBase = NULL;

        mA2MShareBufEnd = NULL;
        mM2AShareBufEnd = NULL;
    }

    if (mSphPCMBuf != NULL) {
        delete mSphPCMBuf;
        mSphPCMBuf = NULL;
    }
    mSphPCMBufEnd = NULL;

#if defined(USE_CCCI_SHARE_BUFFER)
    if (CCCIBuf_Hdl >= 0) {
        ccci_smem_put(CCCIBuf_Hdl, (unsigned char *) mA2MParaShareBufBase, mA2MParaShareBufLen);
        CCCIBuf_Hdl = -1;
    }
    mA2MParaShareBufBase = NULL;
    mA2MParaShareBufLen = 0;
#endif

    return NO_ERROR;
}


SpeechMessengerEVDO::~SpeechMessengerEVDO() {
    ALOGD("%s()", __FUNCTION__);
}

/** Create CCCI message */
ccci_buff_t SpeechMessengerEVDO::InitCcciMailbox(uint16_t id, uint16_t param_16bit, uint32_t param_32bit) {
    ccci_buff_t ccci_buff;
    memset(&ccci_buff, 0, sizeof(ccci_buff));

    ccci_buff.magic    = CCCI_MAILBOX_MAGIC_NUMBER;
    ccci_buff.message  = (id << 16) | param_16bit;
    ccci_buff.channel  = CCCI_A2M_CHANNEL;
    ccci_buff.reserved = param_32bit;

    //    ALOGD("JT:-InitCcciMailBox");
    return ccci_buff;
}

/** Get CCCI message's ID */
uint16_t SpeechMessengerEVDO::GetMessageID(const ccci_buff_t &ccci_buff) {
    return (ccci_buff.message) >> 16;
}

/** Get CCCI message's parameters */
uint16_t SpeechMessengerEVDO::GetMessageParam(const ccci_buff_t &ccci_buff) {
    return (ccci_buff.message) & 0xFFFF;
}

/** Get CCCI message's payload length */
uint16_t SpeechMessengerEVDO::GetMessageLength(const ccci_buff_t &ccci_buff) { // TODO(Harvey): rename payload length
    return (ccci_buff.message) & 0xFFFF;
}

uint16_t SpeechMessengerEVDO::GetMessageOffset(const ccci_buff_t &ccci_buff) {
    return (ccci_buff.reserved) & 0xFFFF;
}


char SpeechMessengerEVDO::GetModemCurrentStatus() {
    AL_AUTOLOCK(mGetModemCurrentStatusLock);

    if (mIsModemEPOF == true) {
        mModemStatus = MODEM_STATUS_INVALID;
        ALOGD("%s() MD EPOF!set mModemStatus=%d", __FUNCTION__, mModemStatus);
    } else {
        unsigned int  status = (unsigned int)MODEM_STATUS_INVALID;
        int retval = 0;
#ifdef MTK_ECCCI_C2K
        retval = ::ioctl(fHdl, CCCI_IOC_GET_MD_STATE, &status);
#else
        retval = ::ioctl(fHdl, CMDM_IOCTL_CHECK_MD_READY, &status);
#endif
        if (retval < 0) {
            mModemStatus = MODEM_STATUS_EXPT;
            ALOGE("%s() ioctl CCCI_IOC_GET_MD_STATE fail!! retval = %d, errno: %d, set mModemStatus=%d",
                  __FUNCTION__, retval, errno, mModemStatus);
        } else {
            mModemStatus = (char)status;
        }
    }
    return mModemStatus;
}

bool SpeechMessengerEVDO::CheckModemIsReady() {
    return (GetModemCurrentStatus() == MODEM_STATUS_READY);
}

status_t SpeechMessengerEVDO::WaitUntilModemReady() {
    char status = 0;
    uint32_t trycnt = 0;
    const uint32_t kMaxTryCnt = 10; // total 200 msec
    do {
        status = GetModemCurrentStatus();
        if (status == MODEM_STATUS_READY) {
            ALOGD("%s(): Modem ready", __FUNCTION__);
            break;
        } else {
            ALOGW("Wait CCCI open #%d times, modem current status = %d", ++trycnt, status);
            sph_msleep(20);
            if (trycnt == kMaxTryCnt) { break; }
        }
    } while (1);

    return (trycnt < kMaxTryCnt) ? NO_ERROR : TIMED_OUT;
}

#undef LOG_TAG
#define LOG_TAG "SpeechMessengerEVDO_Send"

int32_t SpeechMessengerEVDO::SendMessage(const ccci_buff_t &ccci_buff) {
    ALOGV("%s()", __FUNCTION__);
    AL_AUTOLOCK(mfHdlLock);

    // check if need ack
    uint16_t message_id = GetMessageID(ccci_buff);
    const bool b_need_ack = (JudgeAckOfMsg(message_id) == MESSAGE_NEED_ACK) ? true : false;

    // check modem status during phone call
    char modem_status = GetModemCurrentStatus();
    if (modem_status != MODEM_STATUS_READY) {
        ALOGE("%s(), modem_status(%d) != MODEM_STATUS_READY, mIsModemEPOF = %d", __FUNCTION__, modem_status, mIsModemEPOF);
        mIsModemResetDuringPhoneCall = true;
        mIsModemReset = true;
        ResetSpeechParamAckCount();
    }

    // Do not send any on/off message when mIsModemResetDuringPhoneCall is true
    if (mIsModemResetDuringPhoneCall == true && IsModemFunctionOnOffMessage(message_id) == true) {
        ALOGE("%s(), mIsModemResetDuringPhoneCall == true, drop on/off message: 0x%x", __FUNCTION__, ccci_buff.message);
        SendMsgFailErrorHandling(ccci_buff);

        // clean mIsModemResetDuringPhoneCall when phone call/loopback stop
        if (message_id == MSG_A2M_SPH_OFF) {
            ALOGD("%s(), Phone call stop. Set mIsModemResetDuringPhoneCall = false", __FUNCTION__);
            mIsModemResetDuringPhoneCall = false;
        } else if (message_id == MSG_A2M_SET_ACOUSTIC_LOOPBACK) {
            const bool loopback_on = GetMessageParam(ccci_buff) & 0x1;
            if (loopback_on == false) {
                ALOGD("%s(), loopback stop. Set mIsModemResetDuringPhoneCall = false", __FUNCTION__);
                mIsModemResetDuringPhoneCall = false;
            }
        }

        return UNKNOWN_ERROR;
    }

    // save ack info before write to avoid race condition with CCCIReadThread
    if (b_need_ack == true) {
        mWaitAckMessageID = message_id;
    }

    // init send msg
    ccci_buff_t uart_buf;
    memset((void *)&uart_buf, 0, sizeof(ccci_buff));

    uint16_t offset_buffer = GetMessageOffset(ccci_buff);
    uint16_t size_payload = 0;
    uint16_t size_message = 0;
    char *pPayload = NULL;
    switch (message_id) {
    case MSG_A2M_PNW_DL_DATA_NOTIFY:
    case MSG_A2M_BGSND_DATA_NOTIFY:
    case MSG_A2M_CTM_DATA_NOTIFY:
    case MSG_A2M_EM_NB:
    case MSG_A2M_EM_WB: // case AUDIO_HD_RECORD_PARAMETER:
    case MSG_A2M_EM_DMNR:
    case MSG_A2M_EM_MAGICON:
    case MSG_A2M_EM_HAC:
    case MSG_A2M_VIBSPK_PARAMETER:
    case MSG_A2M_SMARTPA_PARAMETER:
#ifdef USE_CCCI_SHARE_BUFFER
        //case MSG_A2M_EM_DYNAMIC_SPH:
#else
    case MSG_A2M_EM_DYNAMIC_SPH:
#endif
        size_payload = GetMessageLength(ccci_buff);
        size_message = size_payload + CCCI_BUF_HEADER_SIZE;

        uart_buf.magic = 0; // offset => 0 for modem side
        uart_buf.message = size_payload;
        uart_buf.channel = CCCI_MAILBOX_DATA_NUMBER;
        uart_buf.reserved = ccci_buff.message;

        pPayload = GetA2MShareBufBase() + offset_buffer;
        ASSERT(size_payload <= (CCCI_MAX_PAYLOAD_SIZE * sizeof(uint32_t)));
        memcpy(uart_buf.payload, pPayload, size_payload);
#ifdef CONFIG_MT_ENG_BUILD // eng load
        ALOGD("%s(), channel=0x%x, message=0x%x, length=0x%x, payload[0]=0x%x, payload[1]=0x%x, payload[2]=0x%x, payload[3]=0x%x",
              __FUNCTION__, uart_buf.channel, uart_buf.reserved, uart_buf.message, uart_buf.payload[0], uart_buf.payload[1], uart_buf.payload[2], uart_buf.payload[3]);
#else
        if (JudgeLogPrintOfMsg(message_id)) {
            ALOGD("%s(), channel=0x%x, message=0x%x, length=0x%x, payload[0]=0x%x, payload[1]=0x%x, payload[2]=0x%x, payload[3]=0x%x",
                  __FUNCTION__, uart_buf.channel, uart_buf.reserved, uart_buf.message, uart_buf.payload[0], uart_buf.payload[1], uart_buf.payload[2], uart_buf.payload[3]);
        } else {
            ALOGV("%s(), channel=0x%x, message=0x%x, length=0x%x, payload[0]=0x%x, payload[1]=0x%x, payload[2]=0x%x, payload[3]=0x%x",
                  __FUNCTION__, uart_buf.channel, uart_buf.reserved, uart_buf.message, uart_buf.payload[0], uart_buf.payload[1], uart_buf.payload[2], uart_buf.payload[3]);
        }
#endif
        break;
    case MSG_A2M_SPH_UL_ENCRYPTION:
    case MSG_A2M_SPH_DL_DECRYPTION:
        size_payload = GetMessageLength(ccci_buff);
        size_message = size_payload + CCCI_BUF_HEADER_SIZE;

        uart_buf.magic = 0; // offset => 0 for modem side
        uart_buf.message = size_payload;
        uart_buf.channel = CCCI_MAILBOX_DATA_NUMBER;
        uart_buf.reserved = ccci_buff.message;

        pPayload = mSphPCMBuf + offset_buffer;
        ASSERT(size_payload <= (CCCI_MAX_PAYLOAD_SIZE * sizeof(uint32_t)));
        memcpy(uart_buf.payload, pPayload, size_payload);
            ALOGD_IF(mLogEnable, "%s(), channel=0x%x, message=0x%x, length=0x%x, payload[0]=0x%x, payload[1]=0x%x, payload[2]=0x%x, payload[3]=0x%x",
                  __FUNCTION__, uart_buf.channel, uart_buf.reserved, uart_buf.message, uart_buf.payload[0], uart_buf.payload[1], uart_buf.payload[2], uart_buf.payload[3]);
        break;
    default: {
        size_message = CCCI_BUF_HEADER_SIZE;

        uart_buf.magic = ccci_buff.magic;
        uart_buf.message = ccci_buff.message;
        uart_buf.channel = CCCI_MAILBOX_COMMAND_NUMBER;
        uart_buf.reserved = ccci_buff.reserved;
#ifdef CONFIG_MT_ENG_BUILD// eng load
        ALOGD("%s(), size_message = %d, data[0] = 0x%x, data[1] = 0x%x, channel = 0x%x, reserved = 0x%x, b_need_ack %d",
              __FUNCTION__, size_message, uart_buf.magic, uart_buf.message, uart_buf.channel, uart_buf.reserved, b_need_ack);
#else  // use and use debug load
        if (ccci_buff.magic == CCCI_MAILBOX_MAGIC_NUMBER) { // message without data payload
            ALOGD("%s(), size_message = %d, data[0] = 0x%x, data[1] = 0x%x, channel = 0x%x, reserved = 0x%x, b_need_ack %d",
                  __FUNCTION__, size_message, uart_buf.magic, uart_buf.message, uart_buf.channel, uart_buf.reserved, b_need_ack);
        } else {
            ALOGV("%s(), size_message = %d, data[0] = 0x%x, data[1] = 0x%x, channel = 0x%x, reserved = 0x%x, b_need_ack %d",
                  __FUNCTION__, size_message, uart_buf.magic, uart_buf.message, uart_buf.channel, uart_buf.reserved, b_need_ack);
        }
#endif
        break;
    }
    }

    //if (b_need_ack == true)
    //{
    //    ALOGD("%s(), size_message = %d, data[0] = 0x%x, data[1] = 0x%x, channel = 0x%x, reserved = 0x%x",
    //          __FUNCTION__, size_message, uart_buf.magic, uart_buf.message, uart_buf.channel , uart_buf.reserved);
    //}

    // send message
    int i = 0;
    int write_length = 0;
    status_t ret = UNKNOWN_ERROR;

    if (fHdl < 0) {
        ALOGW("%s(), fHdl(%d) < 0", __FUNCTION__, fHdl);
    } else {
        for (i = 0; i < 150; i++) { // try 150 times for every 2 ms if sent message fail
            write_length = write(fHdl, (void *)&uart_buf, size_message);
            if (i != 0) {
                ALOGD("%s() write %d times, return write_length = %d, size_message = %d", __FUNCTION__, i, write_length, size_message);
            }
            if (write_length == size_message) {
                LastSentMessage = message_id;
                if (MESSAGE_NEED_ACK == JudgeAckOfMsg(message_id)) {
                    LastNeedAckSentMessage = message_id;
                }
                ret = NO_ERROR;
                break;
            } else if (write_length > 0) {
                int total_write_length = write_length;
                char *pWriteBuf = ((char *)&uart_buf) + write_length;
                do {
                    write_length = write(fHdl, pWriteBuf, size_message - total_write_length);
                    if (write_length < 0) {
                        ALOGE("%s() fail, write_length: %d, modem current status: %d, errno: %d", __FUNCTION__, write_length, GetModemCurrentStatus(), errno);
                        if (message_id != MSG_A2M_SPH_OFF && message_id != MSG_A2M_SET_ACOUSTIC_LOOPBACK) {
                            mIsModemResetDuringPhoneCall = true;
                        }
                        ResetSpeechParamAckCount();
                        break;
                    }

                    total_write_length += write_length;
                    pWriteBuf += write_length;
                } while (total_write_length == size_message);

                if (total_write_length == size_message) { ret = NO_ERROR; }
                break;
            } else {
                modem_status = GetModemCurrentStatus();
                ALOGW("%s(), message_id: 0x%x, try: #%d, write_length: %d, errno: %d, modem status: %d",
                      __FUNCTION__, message_id, i, write_length, errno, modem_status);

                if (errno == 3 || modem_status != MODEM_STATUS_READY) {
                    ALOGE("%s(), MD RESET SKIP MESSAGE: 0x%x", __FUNCTION__, ccci_buff.message);
                    // if modem reset during phone call, raise mIsModemResetDuringPhoneCall
                    if (message_id != MSG_A2M_SPH_OFF && message_id != MSG_A2M_SET_ACOUSTIC_LOOPBACK) {
                        mIsModemResetDuringPhoneCall = true;
                    }
                    mIsModemReset = true;
                    ResetSpeechParamAckCount();
                    break;
                }
                sph_msleep(2);
            }

        }
    }

    // error handling for ack message
    if (ret != NO_ERROR && b_need_ack == true) {
        mWaitAckMessageID = 0;
        SendMsgFailErrorHandling(ccci_buff);
    }

    return ret;

}

#undef LOG_TAG
#define LOG_TAG "SpeechMessengerEVDO_Read"

status_t SpeechMessengerEVDO::ReadMessage(ccci_buff_t &ccci_buff) {

    if (fHdl < 0) {
        ALOGD("%s(), fHdl(%d) < 0", __FUNCTION__, fHdl);
        return UNKNOWN_ERROR;
    }

    /* read message */
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(fHdl, &fds);
    int rc = select(fHdl + 1, &fds, NULL, NULL, NULL);
    ALOGV("%s(), rc: %d, errno: %d", __FUNCTION__, rc, errno);
    FD_CLR(fHdl, &fds);

    //ccci_buff_t uart_buf;
    evdo_buff_t uart_buf;

    int length_read = read(fHdl, (void *)&uart_buf, CCCI_BUF_HEADER_SIZE);
    if (length_read != CCCI_BUF_HEADER_SIZE) {
        mModemStatus = MODEM_STATUS_EXPT;
        ALOGE("%s() fail, read_length: %d, errno = %d, force set mModemStatus = %d", __FUNCTION__, length_read, errno, mModemStatus);
        return UNKNOWN_ERROR;
    }

#ifdef CONFIG_MT_ENG_BUILD // eng load
    ALOGD("%s(), length_read = %d, data[0] = 0x%x, data[1] = 0x%x, ch = 0x%x, reserved = 0x%x",
          __FUNCTION__, length_read, uart_buf.magic, uart_buf.message, uart_buf.channel, uart_buf.reserved);
#endif
    if (uart_buf.magic == CCCI_MAILBOX_MAGIC_NUMBER) { // message
        ASSERT(uart_buf.channel == CCCI_MAILBOX_COMMAND_NUMBER);

        ccci_buff.magic = uart_buf.magic;
        ccci_buff.message = uart_buf.message;
        ccci_buff.channel = CCCI_M2A_CHANNEL;
        ccci_buff.reserved = uart_buf.reserved;
#ifndef CONFIG_MT_ENG_BUILD // user or user debug load
        if (ccci_buff.magic == CCCI_MAILBOX_MAGIC_NUMBER && JudgeLogPrintOfMsg(GetMessageID(ccci_buff))) {
            ALOGD("%s(), length_read = %d, data[0] = 0x%x, data[1] = 0x%x, ch = 0x%x, reserved = 0x%x",
                  __FUNCTION__, length_read, uart_buf.magic, uart_buf.message, uart_buf.channel, uart_buf.reserved);
        } else {
            ALOGV("%s(), length_read = %d, data[0] = 0x%x, data[1] = 0x%x, ch = 0x%x, reserved = 0x%x",
                  __FUNCTION__, length_read, uart_buf.magic, uart_buf.message, uart_buf.channel, uart_buf.reserved);
        }
#endif

    } else { // data
        ASSERT(uart_buf.magic == 0);
        ASSERT(uart_buf.channel == CCCI_MAILBOX_DATA_NUMBER);

        ccci_buff.magic = CCCI_MAILBOX_MAGIC_NUMBER;
        ccci_buff.message = uart_buf.reserved;
        ccci_buff.channel = CCCI_M2A_CHANNEL;
        ccci_buff.reserved = mM2AShareBuf.pWrite - mM2AShareBuf.pBufBase; // offset, read to write

        const uint16_t payload_length = GetMessageLength(ccci_buff);
        ASSERT(payload_length == uart_buf.message); // TODO(Harvey): check only, remove later
        //ALOGD("%s(), payload_length = %d", __FUNCTION__, payload_length);

        int read_payload_length = 0;
        char *pPayload = (char *)uart_buf.payload;
        do {
            FD_ZERO(&fds);
            FD_SET(fHdl, &fds);
            rc = select(fHdl + 1, &fds, NULL, NULL, NULL);
            ALOGV("%s(), rc: %d, errno: %d", __FUNCTION__, rc, errno);
            FD_CLR(fHdl, &fds);

            length_read = read(fHdl, pPayload, payload_length - read_payload_length);
            char status = GetModemCurrentStatus();
            //          ALOGD("%s(), pPayload[0]=0x%x, pPayload[1]=0x%x, pPayload[2]=0x%x, pPayload[3]=0x%x, pPayload[4]=0x%x, pPayload[5]=0x%x, pPayload[6]=0x%x, pPayload[7]=0x%x, pPayload[8]=0x%x, pPayload[9]=0x%x, pPayload[10]=0x%x, pPayload[11]=0x%x, pPayload[12]=0x%x, pPayload[13]=0x%x",
            //         __FUNCTION__, pPayload[0], pPayload[1], pPayload[2], pPayload[3], pPayload[4], pPayload[5], pPayload[6], pPayload[7], pPayload[8], pPayload[9], pPayload[10], pPayload[11], pPayload[12], pPayload[13]);
            if (length_read <= 0) {
                ALOGE("%s() data fail, read_length: %d, errno = %d", __FUNCTION__, length_read, errno);

                mModemStatus = MODEM_STATUS_EXPT;
                OpenMuxdDeviceUntilReady();

                return UNKNOWN_ERROR;
            } else if (status != MODEM_STATUS_READY) {
                ALOGE("%s() modem status(%d) != ready", __FUNCTION__, status);
                return UNKNOWN_ERROR;
            } else {
                read_payload_length += length_read;
                pPayload += length_read;

#ifdef CONFIG_MT_ENG_BUILD // eng load
                ALOGD("%s(), read_payload_length = %d, payload_length = %d", __FUNCTION__, read_payload_length, payload_length);
#else
                ALOGV("%s(), read_payload_length = %d, payload_length = %d", __FUNCTION__, read_payload_length, payload_length);
#endif
            }
        } while (read_payload_length != payload_length);

        uint16_t *pHeader = (uint16_t *)uart_buf.payload;
        //ALOGD("%s(), sync = 0x%x, type = 0x%x, data_len = %d", __FUNCTION__ , pHeader[0], pHeader[1], pHeader[2]);

        //fwrite((void *)&pHeader[3], sizeof(char), payload_length - 6, fout);

        RingBuf_copyFromLinear(&mM2AShareBuf, (char *)uart_buf.payload, payload_length);

#ifdef CONFIG_MT_ENG_BUILD // eng load
        ALOGD("%s(), message=0x%x, length=0x%x, payload[0]=0x%x, payload[1]=0x%x, payload[2]=0x%x, payload[3]=0x%x",
              __FUNCTION__, uart_buf.reserved, uart_buf.message, uart_buf.payload[0], uart_buf.payload[1], uart_buf.payload[2], uart_buf.payload[3]);
#else
        ALOGV("%s(), message=0x%x, length=0x%x, payload[0]=0x%x, payload[1]=0x%x, payload[2]=0x%x, payload[3]=0x%x",
              __FUNCTION__, uart_buf.reserved, uart_buf.message, uart_buf.payload[0], uart_buf.payload[1], uart_buf.payload[2], uart_buf.payload[3]);
#endif
        mM2AShareBuf.pRead = mM2AShareBuf.pWrite; // pseudo read
    }

    /* check modem status */
    AL_LOCK(mfHdlLock);
    uint16_t message_id = GetMessageID(ccci_buff);
    if (message_id == MSG_M2A_EM_DATA_REQUEST) {
        mIsModemResetDuringPhoneCall = false;
        mIsModemReset = false;
        mIsModemEPOF = false;
    }

    char status = GetModemCurrentStatus();
    if (fHdl < 0) {
        ALOGD("%s(), fHdl(%d) < 0", __FUNCTION__, fHdl);
        sph_msleep(100);
        AL_UNLOCK(mfHdlLock);
        return UNKNOWN_ERROR;
    } else if (status != MODEM_STATUS_READY) {
        ALOGD("%s() modem current status = %d, return", __FUNCTION__, status);
        sph_msleep(100);
        AL_UNLOCK(mfHdlLock);
        return UNKNOWN_ERROR;
    }
    AL_UNLOCK(mfHdlLock);

    return NO_ERROR;
}

#undef LOG_TAG
#define LOG_TAG "SpeechMessengerEVDO"

uint32_t SpeechMessengerEVDO::GetQueueCount() const {
    int32_t count = (iQWrite - iQRead);
    if (count < 0) { count += CCCI_MAX_QUEUE_NUM; }
    return count;
}

bool SpeechMessengerEVDO::CheckOffsetAndLength(const ccci_buff_t &ccci_buff) {
    uint16_t message_id = GetMessageID(ccci_buff);
    uint16_t length     = GetMessageLength(ccci_buff);
    uint16_t offset     = GetMessageOffset(ccci_buff);

    if (offset > mM2AShareBufLen || length > mM2AShareBufLen) {
        ALOGE("%s(), message_id = 0x%x, length(0x%x), offset(0x%x), mM2AShareBufLen(0x%x)", __FUNCTION__, message_id, length, offset, mM2AShareBufLen);
        ASSERT(offset > mM2AShareBufLen || length > mM2AShareBufLen);
        return false;
    } else {
        return true;
    }
}


ccci_message_ack_t SpeechMessengerEVDO::JudgeAckOfMsg(const uint16_t message_id) {
    //    ALOGD("JT: +JudgeAckOfMsg()");
    ccci_message_ack_t ack;
    switch (message_id) {
    case MSG_A2M_SET_SPH_MODE:
    case MSG_A2M_SPH_ON:
    case MSG_A2M_SPH_OFF:
    case MSG_A2M_SPH_ROUTER_ON:
    case MSG_A2M_PCM_REC_ON:
    case MSG_A2M_VM_REC_ON:
    case MSG_A2M_PCM_REC_OFF:
    case MSG_A2M_VM_REC_OFF:
    case MSG_A2M_BGSND_ON:
    case MSG_A2M_BGSND_OFF:
    case MSG_A2M_PNW_ON:
    case MSG_A2M_PNW_OFF:
    case MSG_A2M_CTM_ON:
    case MSG_A2M_CTM_OFF:
    case MSG_A2M_SET_ACOUSTIC_LOOPBACK:
    case MSG_A2M_EM_NB:
    case MSG_A2M_EM_DMNR:
    case MSG_A2M_EM_WB:
    case MSG_A2M_EM_MAGICON:
    case MSG_A2M_EM_HAC:
    case MSG_A2M_VIBSPK_PARAMETER:
    case MSG_A2M_SMARTPA_PARAMETER:
    case MSG_A2M_RECORD_RAW_PCM_ON:
    case MSG_A2M_RECORD_RAW_PCM_OFF:
    case MSG_A2M_EM_DYNAMIC_SPH:
        ack = MESSAGE_NEED_ACK;
        break;
    default:
        ack = MESSAGE_BYPASS_ACK;
    }
    //    ALOGD("JT: -JudgeAckOfMsg(), ack=%d", ack);
    return ack;
}

bool SpeechMessengerEVDO::JudgeLogPrintOfMsg(const uint16_t message_id) {
    bool isLogPrint = true;
    switch (message_id) {
        case MSG_A2M_PNW_DL_DATA_NOTIFY:
        case MSG_A2M_BGSND_DATA_NOTIFY:
        case MSG_A2M_CTM_DATA_NOTIFY:
        case MSG_A2M_SPH_UL_ENCRYPTION:
        case MSG_A2M_SPH_DL_DECRYPTION:
        case MSG_A2M_PNW_UL_DATA_READ_ACK:
        case MSG_A2M_REC_DATA_READ_ACK:
        case MSG_A2M_CTM_DEBUG_DATA_READ_ACK:
        case MSG_A2M_PCM_REC_DATA_READ_ACK:
        case MSG_A2M_VM_REC_DATA_READ_ACK:
        case MSG_A2M_RAW_PCM_REC_DATA_READ_ACK:
        case MSG_A2M_CUST_DUMP_READ_ACK:
        case MSG_M2A_PNW_DL_DATA_REQUEST:
        case MSG_M2A_BGSND_DATA_REQUEST:
        case MSG_M2A_CTM_DATA_REQUEST:
        case MSG_M2A_SPH_UL_ENCRYPTION:
        case MSG_M2A_SPH_DL_DECRYPTION:
        case MSG_M2A_PNW_UL_DATA_NOTIFY:
        case MSG_M2A_CTM_DEBUG_DATA_NOTIFY:
        case MSG_M2A_PCM_REC_DATA_NOTIFY:
        case MSG_M2A_VM_REC_DATA_NOTIFY:
        case MSG_M2A_RAW_PCM_REC_DATA_NOTIFY:
        case MSG_M2A_CUST_DUMP_NOTIFY:
        isLogPrint = false;
        break;
    default:
        isLogPrint = true;
    }
    return isLogPrint;
}


bool SpeechMessengerEVDO::IsModemFunctionOnOffMessage(const uint16_t message_id) {
    bool bIsModemFunctionOnOffMessage = false;

    switch (message_id) {
    case MSG_A2M_SPH_ON:
    case MSG_A2M_SPH_OFF:
    case MSG_A2M_SPH_ROUTER_ON:
    case MSG_A2M_PCM_REC_ON:
    case MSG_A2M_VM_REC_ON:
    case MSG_A2M_PCM_REC_OFF:
    case MSG_A2M_VM_REC_OFF:
    case MSG_A2M_BGSND_ON:
    case MSG_A2M_BGSND_OFF:
    case MSG_A2M_PNW_ON:
    case MSG_A2M_PNW_OFF:
    case MSG_A2M_CTM_ON:
    case MSG_A2M_CTM_OFF:
    case MSG_A2M_SET_ACOUSTIC_LOOPBACK:
    case MSG_A2M_RECORD_RAW_PCM_ON:
    case MSG_A2M_RECORD_RAW_PCM_OFF:
        bIsModemFunctionOnOffMessage = true;
        break;
    default:
        bIsModemFunctionOnOffMessage = false;
    }

    return bIsModemFunctionOnOffMessage;
}

status_t SpeechMessengerEVDO::SendMessageInQueue(ccci_buff_t ccci_buff) {
    AL_LOCK(mCCCIMessageQueueMutex);
    status_t ret = NO_ERROR;

    // check modem status during phone call
    char modem_status = GetModemCurrentStatus();
    if (modem_status != MODEM_STATUS_READY) {
        ALOGE("%s(), modem_status(%d) != MODEM_STATUS_READY, errno: %d, mIsModemEPOF = %d", __FUNCTION__, modem_status, errno, mIsModemEPOF);
        mIsModemResetDuringPhoneCall = true;
        mIsModemReset = true;
        ResetSpeechParamAckCount();
        AL_UNLOCK(mCCCIMessageQueueMutex);
        uint16_t message_id = GetMessageID(ccci_buff);
        switch (message_id) {
        case MSG_A2M_EM_NB:
        case MSG_A2M_EM_WB:
        case MSG_A2M_EM_DMNR:
        case MSG_A2M_EM_MAGICON:
        case MSG_A2M_EM_HAC:
        case MSG_A2M_VIBSPK_PARAMETER:
        case MSG_A2M_SMARTPA_PARAMETER:
#ifndef USE_CCCI_SHARE_BUFFER
        case MSG_A2M_EM_DYNAMIC_SPH:
#endif
            {
                A2MBufUnLock();
                break;
            }
        }
        return ret;
    }

    uint32_t count = GetQueueCount();
    ALOGV("%s(), count of message in queue =%d", __FUNCTION__, count);

    ccci_message_ack_t ack_type = JudgeAckOfMsg(GetMessageID(ccci_buff));
#ifdef CONFIG_MT_ENG_BUILD
    if (ack_type == MESSAGE_NEED_ACK) {
        ALOGD("%s(), mModemIndex = %d, need ack message: 0x%x, reserved param: 0x%x",
              __FUNCTION__, mModemIndex, ccci_buff.message, ccci_buff.reserved);
    } else {
        ALOGV("%s(), mModemIndex = %d, no ack message: 0x%x, reserved param: 0x%x",
              __FUNCTION__, mModemIndex, ccci_buff.message, ccci_buff.reserved);
    }
#else
    if (ack_type == MESSAGE_NEED_ACK) {
        ALOGV("%s(), mModemIndex = %d, need ack message: 0x%x, reserved param: 0x%x",
              __FUNCTION__, mModemIndex, ccci_buff.message, ccci_buff.reserved);
    } else {
        ALOGV("%s(), mModemIndex = %d, no ack message: 0x%x, reserved param: 0x%x",
              __FUNCTION__, mModemIndex, ccci_buff.message, ccci_buff.reserved);
    }
#endif

    ASSERT(count < (CCCI_MAX_QUEUE_NUM - 1));  // check queue full

    if (mIsModemEPOF == true) {
        ALOGD("%s(), mIsModemEPOF=%d, Skip message(0x%x) to queue, now count(%u)", __FUNCTION__, mIsModemEPOF, ccci_buff.message, GetQueueCount());
        A2MBufUnLock();

    } else {


        if (count == 0) { // queue is empty
            if (ack_type == MESSAGE_BYPASS_ACK) { // no need ack, send directly, don't care ret value
                ret = SendMessage(ccci_buff);
            } else { // need ack, en-queue and send message
                pQueue[iQWrite].ccci_buff = ccci_buff;
                pQueue[iQWrite].ack_type  = ack_type;
                iQWrite++;
                if (iQWrite == CCCI_MAX_QUEUE_NUM) { iQWrite -= CCCI_MAX_QUEUE_NUM; }

                ret = SendMessage(ccci_buff);
                if (ret != NO_ERROR) { // skip this fail CCCI message
                    iQRead++;
                    if (iQRead == CCCI_MAX_QUEUE_NUM) { iQRead -= CCCI_MAX_QUEUE_NUM; }
                }
            }
        } else { // queue is not empty, must queue the element
            pQueue[iQWrite].ccci_buff = ccci_buff;
            pQueue[iQWrite].ack_type  = ack_type;
            iQWrite++;
            if (iQWrite == CCCI_MAX_QUEUE_NUM) { iQWrite -= CCCI_MAX_QUEUE_NUM; }

#ifdef CONFIG_MT_ENG_BUILD
            ALOGD("%s(), Send message(0x%x) to queue, reserve param(0x%x), mModemIndex (%d), count(%u), LastSentMessage(0x%x), LastNeedAckSentMessage(0x%x)", __FUNCTION__, ccci_buff.message, ccci_buff.reserved, mModemIndex, GetQueueCount(), LastSentMessage, LastNeedAckSentMessage);
#else
            ALOGV("%s(), Send message(0x%x) to queue, reserve param(0x%x), mModemIndex (%d), count(%u), LastSentMessage(0x%x), LastNeedAckSentMessage(0x%x)", __FUNCTION__, ccci_buff.message, ccci_buff.reserved, mModemIndex, GetQueueCount(), LastSentMessage, LastNeedAckSentMessage);
#endif
        }
    }
    AL_UNLOCK(mCCCIMessageQueueMutex);
    return ret;
}

status_t SpeechMessengerEVDO::ConsumeMessageInQueue() {
    AL_LOCK(mCCCIMessageQueueMutex);

    uint32_t count = GetQueueCount();
    if (count > 10) {
        ALOGW("%s(), queue count: %u", __FUNCTION__, count);
    }

    if (count == 0) {
        ALOGW("%s(), no message in queue", __FUNCTION__);
        AL_UNLOCK(mCCCIMessageQueueMutex);
        return UNKNOWN_ERROR;
    }

    status_t ret = NO_ERROR;
    ALOGD("%s,start count [%d]", __FUNCTION__, count);
    while (1) {
        // when entering this function, the first message in queue must be a message waiting for ack
        // so we increment index, consuming the first message in queue
        iQRead++;
        if (iQRead == CCCI_MAX_QUEUE_NUM) { iQRead -= CCCI_MAX_QUEUE_NUM; }

        // check if empty
        if (iQRead == iQWrite) {
            ret = NO_ERROR;
            break;
        }

        // update count
        count = GetQueueCount();

        // send message
        if (pQueue[iQRead].ack_type == MESSAGE_BYPASS_ACK) { // no need ack, send directly, don't care ret value
            ALOGV("%s(), no need ack message: 0x%x, count: %u", __FUNCTION__, pQueue[iQRead].ccci_buff.message, count);
            ret = SendMessage(pQueue[iQRead].ccci_buff);
        } else if (pQueue[iQRead].ack_type == MESSAGE_NEED_ACK) {
            ALOGV("%s(), need ack message: 0x%x, count: %u", __FUNCTION__, pQueue[iQRead].ccci_buff.message, count);
            ret = SendMessage(pQueue[iQRead].ccci_buff);
            if (ret == NO_ERROR) { // Send CCCI message success and wait for ack
                break;
            }
        } else if (pQueue[iQRead].ack_type == MESSAGE_CANCELED) { // the cancelled message, ignore it
            ALOGD("%s(), cancel on-off-on message: 0x%x, count: %u", __FUNCTION__, pQueue[iQRead].ccci_buff.message, count);
            ret = NO_ERROR;
        }
    }
    ALOGD("%s,end left Count %d", __FUNCTION__, count);
    AL_UNLOCK(mCCCIMessageQueueMutex);
    return ret;
}

bool SpeechMessengerEVDO::MDReset_CheckMessageInQueue() {
    AL_LOCK(mCCCIMessageQueueMutex);
    uint32_t count = GetQueueCount();
    ALOGD("%s(), queue count: %u", __FUNCTION__, count);

    bool ret = true;
    while (1) {
        // Modem already reset.
        // Check every CCCI message in queue.
        // These messages that are sent before modem reset, don't send to modem.
        // But AP side need to do related action to make AP side in the correct state.

        // check if empty
        if (iQRead == iQWrite) {
            ALOGD("%s(), check message done", __FUNCTION__);
            ret = true;
            break;
        }

        // Need ack message. But modem reset, so simulate that the modem send back ack msg.
        if (JudgeAckOfMsg(GetMessageID(pQueue[iQRead].ccci_buff)) == MESSAGE_NEED_ACK) {
            SendMsgFailErrorHandling(pQueue[iQRead].ccci_buff);
        }

        iQRead++;
        if (iQRead == CCCI_MAX_QUEUE_NUM) { iQRead -= CCCI_MAX_QUEUE_NUM; }
    }

    if (count != 0) {
        ALOGE("%s(), queue is not empty!!", __FUNCTION__);
        iQWrite = 0;
        iQRead = 0;
    }

    AL_UNLOCK(mCCCIMessageQueueMutex);
    return ret;
}

bool SpeechMessengerEVDO::GetMDResetFlag() {
    ALOGD("%s(), mIsModemReset=%d", __FUNCTION__, mIsModemReset);
    return mIsModemReset;
}

status_t SpeechMessengerEVDO::CreateReadingThread() {
    int ret = pthread_create(&hReadThread, NULL, SpeechMessengerEVDO::EVDOReadThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

void *SpeechMessengerEVDO::EVDOReadThread(void *arg) {
    pthread_detach(pthread_self());

    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);

    SpeechMessengerEVDO *pCCCI = (SpeechMessengerEVDO *)arg;
    // Adjust thread priority
    int rtnPrio = setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    if (0 != rtnPrio) {
        ALOGE("[%s] failed, errno: %d, return=%d", __FUNCTION__, errno, rtnPrio);
    } else {
        ALOGD("%s setpriority ok, priority: %d", __FUNCTION__, ANDROID_PRIORITY_AUDIO);
    }

    // Handle CCCI Message From Modems
    while (pCCCI->CCCIEnable) {
        /// read message
        ccci_buff_t ccci_buff;
        memset((void *)&ccci_buff, 0, sizeof(ccci_buff));
        status_t ret = pCCCI->ReadMessage(ccci_buff);
        if (ret != NO_ERROR) {
            ALOGD("%s(), ret(%d) != NO_ERROR", __FUNCTION__, ret);
            sph_msleep(10);
            continue;
        }

        /// handle message
        uint16_t m2a_message_id = pCCCI->GetMessageID(ccci_buff);

        switch (m2a_message_id) {
        case MSG_M2A_SET_SPH_MODE_ACK: { // ack of MSG_A2M_SET_SPH_MODE
            // Do nothing... just leave a log
            ALOGV("--SetSpeechMode Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* Speech */
        case MSG_M2A_SPH_ON_ACK: { // ack of MSG_A2M_SPH_ON
            phone_call_mode_t phone_call_mode = (phone_call_mode_t)pCCCI->GetMessageParam(ccci_buff);
            if (phone_call_mode == RAT_3G324M_MODE) {
                pCCCI->SetModemSideModemStatus(VT_STATUS_MASK);
            } else {
                pCCCI->SetModemSideModemStatus(SPEECH_STATUS_MASK);
            }
            ALOGD("--SpeechOn Ack done(0x%x)", ccci_buff.message);
#if defined(MTK_AUDIO_GAIN_TABLE)
            // update speech network type: CDMA, NB
            //bit 0~3: Network; bit 4,5: Voice Band, bit 15: 0 no network support, 1: network support
#if defined(MTK_AUDIO_GAIN_TABLE_SUPPORT_CDMA)
            uint32_t messageNetwork = 0x8004;
#else
            uint32_t messageNetwork = 0x0;
#endif
            ALOGD("%s(), apply gain with messageNetwork = %d", __FUNCTION__, messageNetwork);
            AudioVolumeFactory::CreateAudioVolumeController()->speechNetworkChange(messageNetwork);
#endif
            break;
        }
        case MSG_M2A_SPH_OFF_ACK: { // ack of MSG_A2M_SPH_OFF
            if (pCCCI->GetModemSideModemStatus(VT_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(VT_STATUS_MASK);
            } else if (pCCCI->GetModemSideModemStatus(SPEECH_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(SPEECH_STATUS_MASK);
            } else {
                ALOGE("--SpeechOff Ack is not paired!!");
            }
            property_set(PROPERTY_KEY_NW_CODEC_INFO[0], "");
            property_set(PROPERTY_KEY_NW_CODEC_INFO[1], "");

            pCCCI->mLad->Signal();

            ALOGD("--SpeechOff Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* Speech Router */
        case MSG_M2A_SPH_ROUTER_ON_ACK: { // ack of MSG_A2M_SPH_ROUTER_ON
            const bool pcm_route_on = pCCCI->GetMessageParam(ccci_buff) & 0x1;
            if (pcm_route_on == true) { // pcm route on
                pCCCI->SetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
            } else { // pcm route off
                if (pCCCI->GetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK) == true) {
                    pCCCI->ResetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
                } else {
                    ALOGE("--SpeechRouterOn Ack is not paired!!");
                }
            }

            ALOGV("--SpeechRouterOn Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* Record */
        case MSG_M2A_PCM_REC_ON_ACK: { // ack of MSG_A2M_PCM_REC_ON
            pCCCI->SetModemSideModemStatus(RECORD_STATUS_MASK);
            ALOGV("--recordOn Ack done(0x%x)", ccci_buff.message);
            //fout = fopen("/data/vendor/audiohal/audio_dump/record.bin", "wb");
            //fout2 = fopen("/data/vendor/audiohal/audio_dump/record2.bin", "wb");
            break;
        }

        case MSG_M2A_VM_REC_ON_ACK: { // ack of MSG_A2M_VM_REC_ON
            pCCCI->SetModemSideModemStatus(VM_RECORD_STATUS_MASK);
            ALOGV("--VMRecordOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_RECORD_RAW_PCM_ON_ACK: { // ack of MSG_A2M_RECORD_RAW_PCM_ON
            pCCCI->SetModemSideModemStatus(RAW_RECORD_STATUS_MASK);
            ALOGV("--RawRecordOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_PCM_REC_DATA_NOTIFY: { // meaning that we are recording, modem have some data
            ASSERT(pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x), data_length: %u", ccci_buff.message, (unsigned int)(pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN));

                AudioALSACaptureDataProviderVoice::getInstance()->provideModemRecordDataToProvider(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                if (pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PCM_REC_DATA_READ_ACK, 0, 0));
                } else {
                    ALOGW("%s() RECORD_STATUS_MASK(%d)", __FUNCTION__, pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK));
                }
            }

            break;
        }
        case MSG_M2A_RAW_PCM_REC_DATA_NOTIFY: { // meaning that we are recording, modem have some data
            ALOGV("%s() MSG_M2A_RAW_PCM_REC_DATA_NOTIFY(0x%x), data_length: %d", __FUNCTION__, ccci_buff.message, pCCCI->GetMessageLength(ccci_buff));
            ASSERT(pCCCI->GetModemSideModemStatus(RAW_RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(RAW_RECORD_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_RAW_PCM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_RAW_PCM_REC_DATA_NOTIFY(0x%x), data_length: %u", ccci_buff.message, (unsigned int)(pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN));

                // Phone record

#ifndef TEST_NEW_RECOED_FOR_OLD_CAPTURE
                SpeechDataProcessingHandler::getInstance()->provideModemRecordDataToProvider(pCCCI->GetM2ARawPcmRingBuffer(ccci_buff));
#else
                AudioALSACaptureDataProviderVoice::getInstance()->provideModemRecordDataToProvider(pCCCI->GetM2ARawPcmRingBuffer(ccci_buff));
#endif

                if (pCCCI->mLad->GetApSideModemStatus(RAW_RECORD_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_RAW_PCM_REC_DATA_READ_ACK, 0, 0));
                } else {
                    ALOGV("%s() RAW_RECORD_STATUS_MASK(%d)", __FUNCTION__, pCCCI->mLad->GetApSideModemStatus(RAW_RECORD_STATUS_MASK));
                }
            }

            break;
        }

        case MSG_M2A_VM_REC_DATA_NOTIFY: { // meaning that we are recording, modem have some data
            ASSERT(pCCCI->GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(VM_RECORD_STATUS_MASK) == false) {
                ALOGD("MSG_M2A_VM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                ALOGV("MSG_M2A_VM_REC_DATA_NOTIFY(0x%x), data_length: %u", ccci_buff.message, (unsigned int)(pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN));

                SpeechVMRecorder::getInstance()->getVmDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                if (pCCCI->mLad->GetApSideModemStatus(VM_RECORD_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_VM_REC_DATA_READ_ACK, 0, 0));
                } else {
                    ALOGD("%s() VM_RECORD_STATUS_MASK(%d)", __FUNCTION__, pCCCI->mLad->GetApSideModemStatus(VM_RECORD_STATUS_MASK));
                }
            }

            break;
        }
#ifdef SPH_VCL_SUPPORT
        case MSG_M2A_CUST_DUMP_NOTIFY: { // meaning that we are recording, modem have some data
            // check VCL need open
            SpeechVoiceCustomLogger *pSpeechVoiceCustomLogger = SpeechVoiceCustomLogger::GetInstance();
            if (pSpeechVoiceCustomLogger->GetVCLRecordStatus() == false) {
                ALOGW("MSG_M2A_CUST_DUMP_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                ALOGD("MSG_M2A_CUST_DUMP_NOTIFY(0x%x), data_length: %d", ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN);

                pSpeechVoiceCustomLogger->CopyBufferToVCL(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_CUST_DUMP_READ_ACK, 0, 0));
            }

            break;
        }
#endif
        case MSG_M2A_PCM_REC_OFF_ACK: { // ack of MSG_A2M_PCM_REC_OFF
            if (pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(RECORD_STATUS_MASK);
                //fclose(fout);
                //fclose(fout2);
            } else {
                ALOGE("--recordOff Ack is not paired!!");
            }
            ALOGV("--recordOff Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_VM_REC_OFF_ACK: { // ack of MSG_A2M_VM_REC_OFF
            if (pCCCI->GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(VM_RECORD_STATUS_MASK);
            } else {
                ALOGE("--VMRecordOff Ack is not paired!!");
            }
            ALOGV("--VMRecordOff Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_RECORD_RAW_PCM_OFF_ACK: { // ack of MSG_A2M_RECORD_RAW_PCM_OFF
            if (pCCCI->GetModemSideModemStatus(RAW_RECORD_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(RAW_RECORD_STATUS_MASK);
            } else {
                ALOGE("--RawRecordOff Ack is not paired!!");
            }
            ALOGV("--RawRecordOff Ack done(0x%x)", ccci_buff.message);
            break;
        }
        /* Background sound */
        case MSG_M2A_BGSND_ON_ACK: { // ack of MSG_A2M_BGSND_ON
            pCCCI->SetModemSideModemStatus(BGS_STATUS_MASK);
            ALOGV("--BGSoundOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_BGSND_DATA_REQUEST: { // modem request bgs data to play
            ASSERT(pCCCI->GetModemSideModemStatus(BGS_STATUS_MASK) == true);

            SpeechPcmMixerBGSPlayer *pBGSPlayer = SpeechPcmMixerBGSPlayer::GetInstance();
            if (pCCCI->mLad->GetApSideModemStatus(BGS_STATUS_MASK) == false) {
                ALOGV("MSG_M2A_BGSND_DATA_REQUEST(0x%x) after AP side trun off BGS!! Drop it.", ccci_buff.message);
            } else {
                ALOGV("MSG_M2A_BGSND_DATA_REQUEST(0x%x), num_data_request: %d", ccci_buff.message, pCCCI->GetMessageParam(ccci_buff));

                // parse size of data request
                uint16_t num_data_request = pCCCI->GetMessageParam(ccci_buff);
                uint32_t max_buf_length = A2M_SHARED_BUFFER_BGS_DATA_SIZE - CCCI_SHARE_BUFF_HEADER_LEN;
                if (num_data_request > max_buf_length) { num_data_request = max_buf_length; }

                // get bgs share buffer address
                uint16_t offset = A2M_SHARED_BUFFER_BGS_DATA_BASE;
                char *p_header_address = pCCCI->GetA2MShareBufBase() + offset;
                char *p_data_address = p_header_address + CCCI_SHARE_BUFF_HEADER_LEN;

                // fill playback data
                uint16_t data_length = pBGSPlayer->PutDataToSpeaker(p_data_address, num_data_request);

                // fill header info
                pCCCI->SetShareBufHeader((uint16_t *)p_header_address,
                                         CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                         SHARE_BUFF_DATA_TYPE_CCCI_BGS_TYPE,
                                         data_length);

                // send data notify to modem side
                const uint16_t payload_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
                if (pCCCI->mLad->GetApSideModemStatus(BGS_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_BGSND_DATA_NOTIFY, payload_length, offset));
                }
            }

            break;
        }
        case MSG_M2A_BGSND_OFF_ACK: { // ack of MSG_A2M_BGSND_OFF
            if (pCCCI->GetModemSideModemStatus(BGS_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(BGS_STATUS_MASK);
            } else {
                ALOGE("--BGSoundOff Ack is not paired!!");
            }
            ALOGV("--BGSoundOff Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* PCM2Way */
        case MSG_M2A_PNW_ON_ACK: { // ack of MSG_A2M_PNW_ON
            pCCCI->SetModemSideModemStatus(P2W_STATUS_MASK);
            ALOGV("--PCM2WayOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_PNW_UL_DATA_NOTIFY: { // Get Microphone data from Modem
            ASSERT(pCCCI->GetModemSideModemStatus(P2W_STATUS_MASK) == true);

            Record2Way *pRecord2Way = Record2Way::GetInstance();
            if (pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_PNW_UL_DATA_NOTIFY(0x%x) after AP side trun off PCM2Way!! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_PNW_UL_DATA_NOTIFY(0x%x), data_length: %u", ccci_buff.message, (unsigned int)(pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN));
                pRecord2Way->GetDataFromMicrophone(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));
                if (pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PNW_UL_DATA_READ_ACK, 0, 0));
                }
            }

#if 0 // PCM2WAY: UL -> DL Loopback
            // Used for debug and Speech DVT
            uint16_t size_bytes = 320;
            char buffer[320];
            pRecord2Way->Read(buffer, size_bytes);
            Play2Way::GetInstance()->Write(buffer, size_bytes);
#endif
            break;
        }
        case MSG_M2A_PNW_DL_DATA_REQUEST: { // Put Data to modem and play
            ASSERT(pCCCI->GetModemSideModemStatus(P2W_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_PNW_DL_DATA_REQUEST(0x%x) after AP side trun off PCM2Way!! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_PNW_DL_DATA_REQUEST(0x%x), num_data_request: %d", ccci_buff.message, pCCCI->GetMessageParam(ccci_buff));

                // parse size of data request
                uint16_t num_data_request = pCCCI->GetMessageParam(ccci_buff);
                uint32_t max_buf_length = A2M_SHARED_BUFFER_BGS_DATA_SIZE - CCCI_SHARE_BUFF_HEADER_LEN;
                if (num_data_request > max_buf_length) { num_data_request = max_buf_length; }

                // get pcm2way share buffer address
                uint16_t offset = A2M_SHARED_BUFFER_P2W_DL_DATA_BASE;
                char *p_header_address = pCCCI->GetA2MShareBufBase() + offset;
                char *p_data_address = p_header_address + CCCI_SHARE_BUFF_HEADER_LEN;

                // fill playback data
                Play2Way *pPlay2Way = Play2Way::GetInstance();
                const uint16_t data_length = pPlay2Way->PutDataToSpeaker(p_data_address, num_data_request);

                // fill header info
                pCCCI->SetShareBufHeader((uint16_t *)p_header_address,
                                         CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                         SHARE_BUFF_DATA_TYPE_PCM_FillSpk,
                                         data_length);

                // send data notify to modem side
                const uint16_t payload_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
                if (pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PNW_DL_DATA_NOTIFY, payload_length, offset));
                }
            }
            break;
        }
        case MSG_M2A_PNW_OFF_ACK: { // ack of MSG_A2M_PNW_OFF
            if (pCCCI->GetModemSideModemStatus(P2W_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(P2W_STATUS_MASK);
            } else {
                ALOGE("--PCM2WayOff Ack is not paired!!");
            }
            ALOGV("--PCM2WayOff Ack done(0x%x)", ccci_buff.message);
            break;
        }
        /* PCM Encryption */
        case MSG_M2A_SPH_ENCRYPTION_ACK: { // ack of MSG_A2M_SPH_ENCRYPTION
            ALOGD("--SetEncryption Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_SPH_UL_ENCRYPTION: {
#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
            if ((pCCCI->GetModemSideModemStatus(SPEECH_STATUS_MASK) == true) || (pCCCI->GetModemSideModemStatus(VT_STATUS_MASK) == true)) {
                struct timespec entertime;
                struct timespec leavetime;
                unsigned long long timediffns = 0;
                entertime = GetSystemTime();

                uint16_t OffsetTarget = A2M_SHARED_BUFFER_ULENC_TARGET_BASE;
                char *BufferSource = pCCCI->mSphPCMBuf + A2M_SHARED_BUFFER_ULENC_SOURCE_BASE;
                char *BufferTarget = pCCCI->mSphPCMBuf + OffsetTarget;
                char *BufferDataTarget = BufferTarget + CCCI_SHARE_BUFF_HEADER_LEN;

                const uint16_t ByteDataSource = pCCCI->GetM2AUplinkData(ccci_buff, BufferSource);
                ALOGD_IF(pCCCI->mLogEnable, "+%s(), MSG_M2A_SPH_UL_ENCRYPTION(0x%x), ByteDataSource=%d", __FUNCTION__, m2a_message_id, ByteDataSource);
                ALOGD_IF(pCCCI->mLogEnable, "+%s(), MSG_M2A_SPH_UL_ENCRYPTION(0x%x), BufferSource[0] = 0x%x, BufferSource[1]=0x%x, BufferSource[2] = 0x%x, BufferSource[3]=0x%x", __FUNCTION__, m2a_message_id, *((uint16_t *)BufferSource), *((uint16_t *)BufferSource + 1), *((uint16_t *)BufferSource + 2), *((uint16_t *)BufferSource + 3));

                uint16_t ByteDataTarget = SpeechDataEncrypter::GetInstance()->Encryption(BufferDataTarget, BufferSource, ByteDataSource);

                // fill header info
                pCCCI->SetShareBufHeader((uint16_t *)BufferTarget,
                                         CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                         SHARE_BUFF_DATA_TYPE_CCCI_UL_ENC,
                                         ByteDataTarget);

                ALOGD_IF(pCCCI->mLogEnable, "%s(), MSG_M2A_SPH_UL_ENCRYPTION(0x%x), BufferTarget = %p, ByteDataTarget = 0x%x", __FUNCTION__, m2a_message_id, BufferTarget, ByteDataTarget);
                ALOGD_IF(pCCCI->mLogEnable, "%s(), MSG_M2A_SPH_UL_ENCRYPTION(0x%x), BufferDataTarget[0] = 0x%x, BufferDataTarget[1]=0x%x, BufferDataTarget[2] = 0x%x, BufferDataTarget[3]=0x%x", __FUNCTION__, m2a_message_id, *((uint16_t *)BufferDataTarget), *((uint16_t *)BufferDataTarget + 1), *((uint16_t *)BufferDataTarget + 2), *((uint16_t *)BufferDataTarget + 3));

                // send data notify to modem side
                uint16_t ByteMessage = CCCI_SHARE_BUFF_HEADER_LEN + ByteDataTarget;
                pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_SPH_UL_ENCRYPTION, ByteMessage, OffsetTarget));
                leavetime = GetSystemTime();

                timediffns = TimeDifference(leavetime, entertime);
                ALOGD_IF(pCCCI->mLogEnable, "-%s(), MSG_M2A_SPH_UL_ENCRYPTION(0x%x), ByteDataTarget=%d, process time=%lld(ns)", __FUNCTION__, m2a_message_id, ByteDataTarget, timediffns);
            } else {
                ALOGD("-%s(), MSG_M2A_SPH_UL_ENCRYPTION(0x%x), not in call, skip ", __FUNCTION__, m2a_message_id);
            }
#else
            ALOGD("%s(), MSG_M2A_SPH_UL_ENCRYPTION(0x%x) not support, skip", __FUNCTION__, ccci_buff.message);
#endif
            break;
        }
        case MSG_M2A_SPH_DL_DECRYPTION: {
#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
            if ((pCCCI->GetModemSideModemStatus(SPEECH_STATUS_MASK) == true) || (pCCCI->GetModemSideModemStatus(VT_STATUS_MASK) == true)) {
                struct timespec entertime;
                struct timespec leavetime;
                unsigned long long timediffns = 0;
                entertime = GetSystemTime();
                uint16_t OffsetTarget = A2M_SHARED_BUFFER_DLDEC_TARGET_BASE;
                char *BufferSource = pCCCI->mSphPCMBuf + A2M_SHARED_BUFFER_DLDEC_SOURCE_BASE;
                char *BufferTarget = pCCCI->mSphPCMBuf + OffsetTarget;
                char *BufferDataTarget = BufferTarget + CCCI_SHARE_BUFF_HEADER_LEN;

                const uint16_t ByteDataSource = pCCCI->GetM2AUplinkData(ccci_buff, BufferSource);
                ALOGD_IF(pCCCI->mLogEnable, "+%s(), MSG_M2A_SPH_DL_DECRYPTION(0x%x), ByteDataSource=%d, BufferSource[0] = 0x%x, BufferSource[1]=0x%x, BufferSource[2] = 0x%x, BufferSource[3]=0x%x", __FUNCTION__, m2a_message_id, ByteDataSource, *((uint16_t *)BufferSource), *((uint16_t *)BufferSource + 1), *((uint16_t *)BufferSource + 2), *((uint16_t *)BufferSource + 3));

                uint16_t ByteDataTarget = SpeechDataEncrypter::GetInstance()->Decryption(BufferDataTarget, BufferSource, ByteDataSource);

                // fill header info
                pCCCI->SetShareBufHeader((uint16_t *)BufferTarget,
                                         CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                         SHARE_BUFF_DATA_TYPE_CCCI_DL_DEC,
                                         ByteDataTarget);

                ALOGD_IF(pCCCI->mLogEnable, "%s(), MSG_M2A_SPH_DL_DECRYPTION(0x%x), ByteDataSource=%d, BufferSource[0] = 0x%x, BufferSource[1]=0x%x, BufferSource[2] = 0x%x, BufferSource[3]=0x%x", __FUNCTION__, m2a_message_id, ByteDataSource, *((uint16_t *)BufferSource), *((uint16_t *)BufferSource + 1), *((uint16_t *)BufferSource + 2), *((uint16_t *)BufferSource + 3));
                // send data notify to modem side
                uint16_t ByteMessage = CCCI_SHARE_BUFF_HEADER_LEN + ByteDataTarget;
                pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_SPH_DL_DECRYPTION, ByteMessage, OffsetTarget));
                leavetime = GetSystemTime();

                timediffns = TimeDifference(leavetime, entertime);
                ALOGD_IF(pCCCI->mLogEnable, "-%s(), MSG_M2A_SPH_DL_DECRYPTION(0x%x), ByteDataTarget=%d, process time=%lld(ns)", __FUNCTION__, m2a_message_id, ByteDataTarget, timediffns);
            } else {
                ALOGD("-%s(), MSG_M2A_SPH_DL_DECRYPTION(0x%x), not in call, skip ", __FUNCTION__, m2a_message_id);
            }
#else
            ALOGD("%s(), MSG_M2A_SPH_DL_DECRYPTION(0x%x) not support, skip", __FUNCTION__, ccci_buff.message);
#endif
            break;
        }

        /* TTY */
        case MSG_M2A_CTM_ON_ACK: { // ack of MSG_A2M_CTM_ON
            pCCCI->SetModemSideModemStatus(TTY_STATUS_MASK);
            ALOGV("--TtyCtmOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_CTM_DEBUG_DATA_NOTIFY: {
            if (pCCCI->mLad->GetApSideModemStatus(TTY_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_CTM_DEBUG_DATA_NOTIFY(0x%x) after AP side trun off TTY!! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_CTM_DEBUG_DATA_NOTIFY(0x%x), data_length: %u", ccci_buff.message, (unsigned int)(pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN));
                SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();

                const uint16_t data_type = pCCCI->GetM2AShareBufDataType(ccci_buff);
                if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_IN) {
                    pSpeechVMRecorder->getCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff), pSpeechVMRecorder->pCtmDumpFileUlIn);
                } else if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_IN) {
                    pSpeechVMRecorder->getCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff), pSpeechVMRecorder->pCtmDumpFileDlIn);
                } else if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_OUT) {
                    pSpeechVMRecorder->getCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff), pSpeechVMRecorder->pCtmDumpFileUlOut);
                } else if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_OUT) {
                    pSpeechVMRecorder->getCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff), pSpeechVMRecorder->pCtmDumpFileDlOut);
                } else {
                    ALOGW("%s(), data_type(0x%x) error", __FUNCTION__, data_type);
                    ASSERT(0);
                }

                if (pCCCI->mLad->GetApSideModemStatus(TTY_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_CTM_DEBUG_DATA_READ_ACK, 0, 0));
                }
            }
            break;
        }
        case MSG_M2A_CTM_OFF_ACK: { // ack of MSG_A2M_CTM_OFF
            if (pCCCI->GetModemSideModemStatus(TTY_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(TTY_STATUS_MASK);
            } else {
                ALOGE("--TtyCtmOff Ack is not paired!!");
            }
            ALOGV("--TtyCtmOff Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* Loopback */
        case MSG_M2A_SET_ACOUSTIC_LOOPBACK_ACK: { // ack of MSG_A2M_SET_ACOUSTIC_LOOPBACK
            const bool loopback_on = pCCCI->GetMessageParam(ccci_buff) & 0x1;
            if (loopback_on == true) { // loopback on
                pCCCI->SetModemSideModemStatus(LOOPBACK_STATUS_MASK);
            } else { // loopback off
                if (pCCCI->GetModemSideModemStatus(LOOPBACK_STATUS_MASK) == true) {
                    pCCCI->ResetModemSideModemStatus(LOOPBACK_STATUS_MASK);
                } else {
                    ALOGE("--SetAcousticLoopback Ack is not paired!!");
                }
            }

            pCCCI->mLad->Signal();

            ALOGV("--SetAcousticLoopback Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* Speech Enhancement parameters */
        case MSG_M2A_EM_NB_ACK: { // ack of MSG_A2M_EM_NB
            pCCCI->AddSpeechParamAckCount(NB_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGV("--SetNBSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_DMNR_ACK: { // ack of MSG_A2M_EM_DMNR
            pCCCI->AddSpeechParamAckCount(DMNR_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGV("--SetDualMicSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_MAGICON_ACK: { // ack of MSG_A2M_EM_MAGICON
            pCCCI->AddSpeechParamAckCount(MAGICON_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGV("--SetMagiConSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_HAC_ACK: { // ack of MSG_A2M_EM_HAC
            pCCCI->AddSpeechParamAckCount(HAC_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGV("--SetHACSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }

        case MSG_M2A_EM_WB_ACK: { // ack of MSG_A2M_EM_WB
            pCCCI->AddSpeechParamAckCount(WB_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGV("--SetWBSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_DYNAMIC_SPH_ACK: { // ack of MSG_A2M_EM_DYNAMIC_SPH
            switch (ccci_buff.message & 0xFFFF) {
            case 0xAA01:
                pCCCI->AddSpeechParamAckCount(DYNAMIC_SPH_PARAM);
                break;
            case 0xAA03:
                pCCCI->AddSpeechParamAckCount(DMNR_SPEECH_PARAM);
                break;
            case 0:
                pCCCI->AddSpeechParamAckCount(DYNAMIC_SPH_PARAM);//some MD ack with 0
                break;
            }
            pCCCI->A2MBufUnLock();
            ALOGV("--SetDynamicSpeechParameters Ack done(0x%x)", ccci_buff.message);
#ifdef USE_CCCI_SHARE_BUFFER
            //release buffer when ack received
            int datalength = pCCCI->GetMessageLength(pCCCI->pQueue[pCCCI->iQRead].ccci_buff);
            pCCCI->AdvanceA2MRawParaRingBuffer(datalength);
#endif
            break;
        }
        case MSG_M2A_SPH_ENH_CORE_ACK: { // ack of MSG_A2M_SPH_ENH_CORE
            // Do nothing... just leave a log
            ALOGD("--MSG_M2A_SPH_ENH_CORE_ACK Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* Modem Reset */
        case MSG_M2A_EM_DATA_REQUEST: { // Modem reset. Requset for all EM data(NB/DMNR/WB)
            ALOGW("..[MD Reset Notify(MSG_M2A_EM_DATA_REQUEST: 0x%x)]..mCreatingSphThreadFlag =%d", ccci_buff.message, pCCCI->mCreatingSphThreadFlag);

            // Clean mWaitAckMessageID to avoid the queue receiving the wrong ack
            pCCCI->mWaitAckMessageID = 0;

            // close analog downlink path when modem reset during phone call
            if (pCCCI->mLad->GetApSideModemStatus(SPEECH_STATUS_MASK) == true ||
                pCCCI->mLad->GetApSideModemStatus(VT_STATUS_MASK)     == true) {
                //AudioResourceManager::getInstance()->StopOutputDevice();
            }
            pCCCI->mIsModemResetDuringPhoneCall = false;
            pCCCI->mIsModemReset = false;
            pCCCI->mIsModemEPOF = false;

            // Create Send Sph Para Thread
#if 0
            if (pCCCI->mCreatingSphThreadFlag == false) {
                pCCCI->mCreatingSphThreadFlag = true;
                pCCCI->CreateSendSphParaThread();
            } else {
                pCCCI->mWaitSphThreadFlag = true;
                while (pCCCI->mCreatingSphThreadFlag == true) {
                    sph_msleep(5);
                }
                ALOGD("Finaly, mCreatingSphThreadFlag==%d", pCCCI->mCreatingSphThreadFlag);
                pCCCI->mCreatingSphThreadFlag = true;
                pCCCI->mWaitSphThreadFlag = false;
                pCCCI->CreateSendSphParaThread();
            }
#endif
            // Check the first message in queue. If need ack, take action.
            // Modem reset, flush all CCCI queue first. Don't care for the CCCI queue.
            pCCCI->ResetSpeechParamAckCount();
            pCCCI->MDReset_CheckMessageInQueue();
            break;
        }
        case MSG_M2A_VIBSPK_PARAMETER_ACK: { // ack of MSG_M2A_VIBSPK_PARAMETER
            pCCCI->A2MBufUnLock();
            ALOGV("--SetVibSpkParam Ack done(0x%x)", ccci_buff.message);
            break;
        }

        case MSG_M2A_SMARTPA_PARAMETER_ACK: {
            pCCCI->A2MBufUnLock();
            ALOGV("--SetSmartpaParam Ack done(0x%x)", ccci_buff.message);
            break;
        }
        /* Modem EPOF */
        case MSG_M2A_EPOF_NOTIFY: {
            ALOGW("..[MD EPOF Notify(MSG_M2A_EPOF_NOTIFY: 0x%x)]..", ccci_buff.message);

            // Check the first message in queue. If need ack, take action.

            // Modem reset, flush all CCCI queue first. Don't care for the CCCI queue.
            pCCCI->ResetSpeechParamAckCount();
            pCCCI->MDReset_CheckMessageInQueue();

            pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_EPOF_ACK, 0, 0));
            pCCCI->mIsModemEPOF = true;

            break;
        }

        case MSG_M2A_NW_CODEC_INFO_NOTIFY: {
            ALOGV("--MSG_M2A_NW_CODEC_INFO_NOTIFY (0x%x)", ccci_buff.message);
            pCCCI->SetNWCodecInfo(ccci_buff);
            pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_NW_CODEC_INFO_READ_ACK, 0, 0));
            break;
        }

        default: {
            //TINA REMOVE TEMP
            ALOGD("Read modem message(0x%x), don't care. (or no such message)", ccci_buff.message);
            sph_msleep(10);
            break;
        }
        }

        /// If AP side is waiting for this ack, then consume message queue
        uint16_t a2m_message_id_of_m2a_ack = m2a_message_id & 0x7FFF; // 0xAF** -> 0x2F**
        if (pCCCI->JudgeAckOfMsg(a2m_message_id_of_m2a_ack) == MESSAGE_NEED_ACK) {
            if (a2m_message_id_of_m2a_ack == pCCCI->mWaitAckMessageID) {
                pCCCI->mWaitAckMessageID = 0; // reset
                pCCCI->ConsumeMessageInQueue();
            } else {
                ALOGD("Message(0x%x) might double ack!! The current mWaitAckMessageID is 0x%x", ccci_buff.message, pCCCI->mWaitAckMessageID);
            }
        }
    }
    pthread_exit(NULL);
    return 0;
}

status_t SpeechMessengerEVDO::CreateSendSphParaThread() {

    //    ALOGD("%s, mCreatingSphThreadFlag=%d", __FUNCTION__, mCreatingSphThreadFlag);
    //    if (mCreatingSphThreadFlag == false)
    //    {
    //        mCreatingSphThreadFlag = true;
    int ret = pthread_create(&hSendSphThread, NULL, SpeechMessengerEVDO::SendSphParaThread, (void *)this);

    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        //            mCreatingSphThreadFlag = false;
        return UNKNOWN_ERROR;
    }
    //    }
    return NO_ERROR;
}

void *SpeechMessengerEVDO::SendSphParaThread(void *arg) {
    pthread_detach(pthread_self());

    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    // Adjust thread priority
    int rtnPrio = setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    if (0 != rtnPrio) {
        ALOGE("[%s] failed, errno: %d, return=%d", __FUNCTION__, errno, rtnPrio);
    } else {
        ALOGD("%s setpriority ok, priority: %d", __FUNCTION__, ANDROID_PRIORITY_AUDIO);
    }
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    SpeechMessengerEVDO *pCCCI = (SpeechMessengerEVDO *)arg;

    // Check the first message in queue. If need ack, take action.
    // Modem reset, flush all CCCI queue first. Don't care for the CCCI queue.
    pCCCI->MDReset_CheckMessageInQueue();

    // Get SpeechParamLock
    if (pCCCI->SpeechParamLock() == false) {
        ALOGE("%s() fail to get SpeechParamLock!!", __FUNCTION__);
        return 0;
    }
    if (pCCCI->mWaitSphThreadFlag == true) {
        pCCCI->SpeechParamUnLock();
        pCCCI->mCreatingSphThreadFlag = false;
        pthread_exit(NULL);
        ALOGD("%s(), another MD reset is waiting for speech parameter thread", __FUNCTION__);
        return 0;
    }

    // Send speech parameters to modem side
    pCCCI->ResetSpeechParamAckCount();
    pCCCI->mLad->SetAllSpeechEnhancementInfoToModem();

    // Release SpeechParamLock
    pCCCI->SpeechParamUnLock();

    pCCCI->mCreatingSphThreadFlag = false;

    pthread_exit(NULL);
    return 0;
}

bool SpeechMessengerEVDO::A2MBufLock() {
    const uint32_t kA2MBufLockTimeout = 3000; // 3 sec

    int rc = AL_LOCK_MS(mA2MShareBufMutex, kA2MBufLockTimeout);
    ALOGD("%s()", __FUNCTION__);
    if (rc != 0) {
        ALOGE("%s(), Cannot get Lock!! Timeout : %u msec", __FUNCTION__, kA2MBufLockTimeout);
        return false;
    }
    return true;
}

void SpeechMessengerEVDO::A2MBufUnLock() {
    AL_UNLOCK(mA2MShareBufMutex);
    ALOGD("%s()", __FUNCTION__);
}

bool SpeechMessengerEVDO::SpeechParamLock() {
    const uint32_t kSphParamLockTimeout = 10000; // 10 sec

    ALOGD("%s()", __FUNCTION__);
    int rc = AL_LOCK_MS(mSetSpeechParamMutex, kSphParamLockTimeout);
    if (rc != 0) {
        ALOGE("%s(), Cannot get Lock!! Timeout : %u msec", __FUNCTION__, kSphParamLockTimeout);
        return false;
    }
    return true;
}

void SpeechMessengerEVDO::SpeechParamUnLock() {
    ALOGD("%s()", __FUNCTION__);
    AL_UNLOCK(mSetSpeechParamMutex);
}

void SpeechMessengerEVDO::ResetSpeechParamAckCount() {
    memset(&mSpeechParamAckCount, 0, sizeof(mSpeechParamAckCount));
    ALOGD("%s(), NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)/DynamicSPH(%u)", __FUNCTION__,
          mSpeechParamAckCount[NB_SPEECH_PARAM],
          mSpeechParamAckCount[DMNR_SPEECH_PARAM],
          mSpeechParamAckCount[WB_SPEECH_PARAM],
          mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
          mSpeechParamAckCount[HAC_SPEECH_PARAM],
          mSpeechParamAckCount[DYNAMIC_SPH_PARAM]);
}

void SpeechMessengerEVDO::AddSpeechParamAckCount(speech_param_ack_t type) {
    if (type >= NUM_SPEECH_PARAM_ACK_TYPE || type < 0) {
        ALOGE("%s(), no such type: %d", __FUNCTION__, type);
    } else {
        if (mSpeechParamAckCount[type] < 0xFFFFFFFF) { //prevent overflow
            mSpeechParamAckCount[type]++;
        }
        ALOGD("%s()(%d), NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)/DynamicSPH(%u)", __FUNCTION__, type,
              mSpeechParamAckCount[NB_SPEECH_PARAM],
              mSpeechParamAckCount[DMNR_SPEECH_PARAM],
              mSpeechParamAckCount[WB_SPEECH_PARAM],
              mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
              mSpeechParamAckCount[HAC_SPEECH_PARAM],
              mSpeechParamAckCount[DYNAMIC_SPH_PARAM]);
    }
}

bool SpeechMessengerEVDO::CheckSpeechParamAckAllArrival() {
    bool ret = true;

    // Get SpeechParamLock
    if (SpeechParamLock() == false) {
        ALOGE("%s() fail to get SpeechParamLock!!", __FUNCTION__);
        return false;
    }

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    if (mSpeechParamAckCount[DYNAMIC_SPH_PARAM] == 0) { ret = false; }
#else

    if (mSpeechParamAckCount[NB_SPEECH_PARAM] == 0) { ret = false; }

    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        if (mSpeechParamAckCount[DMNR_SPEECH_PARAM] == 0) { ret = false; }

#if defined(MTK_MAGICONFERENCE_SUPPORT) || defined(MTK_INCALL_HANDSFREE_DMNR)
        if (mSpeechParamAckCount[MAGICON_SPEECH_PARAM] == 0) { ret = false; }
#endif
    }

#if defined(MTK_WB_SPEECH_SUPPORT)
    if (mSpeechParamAckCount[WB_SPEECH_PARAM] == 0) { ret = false; }
#endif

#if defined(MTK_HAC_SUPPORT)
    if (mSpeechParamAckCount[HAC_SPEECH_PARAM] == 0) { ret = false; }
#endif
#endif

    if (ret == true) {
        ALOGD("%s() Pass", __FUNCTION__);
    } else {
        ALOGE("%s() Fail, NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)/DynamicSPH(%u)", __FUNCTION__,
              mSpeechParamAckCount[NB_SPEECH_PARAM],
              mSpeechParamAckCount[DMNR_SPEECH_PARAM],
              mSpeechParamAckCount[WB_SPEECH_PARAM],
              mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
              mSpeechParamAckCount[HAC_SPEECH_PARAM],
              mSpeechParamAckCount[DYNAMIC_SPH_PARAM]);

        // Send speech parameters to modem side again
        mLad->SetAllSpeechEnhancementInfoToModem();
    }

    // Release SpeechParamLock
    SpeechParamUnLock();

    return ret;
}

/** Do error handling here */
void SpeechMessengerEVDO::SendMsgFailErrorHandling(const ccci_buff_t &ccci_buff) {
    ALOGE("%s(), message: 0x%x", __FUNCTION__, ccci_buff.message);
    switch (GetMessageID(ccci_buff)) {
    case MSG_A2M_SET_SPH_MODE: {
        // Do nothing...
        break;
    }
    case MSG_A2M_SPH_ON: {
        phone_call_mode_t phone_call_mode = (phone_call_mode_t)GetMessageParam(ccci_buff);
        if (phone_call_mode == RAT_3G324M_MODE) {
            SetModemSideModemStatus(VT_STATUS_MASK);
        } else {
            SetModemSideModemStatus(SPEECH_STATUS_MASK);
        }

        //mLad->Signal();

        break;
    }
    case MSG_A2M_SPH_OFF: {
        if (GetModemSideModemStatus(VT_STATUS_MASK) == true) {
            ResetModemSideModemStatus(VT_STATUS_MASK);
        } else if (GetModemSideModemStatus(SPEECH_STATUS_MASK) == true) {
            ResetModemSideModemStatus(SPEECH_STATUS_MASK);
        } else {
            ALOGE("--SpeechOff Ack is not paired!!");
        }

        mLad->Signal();

        break;
    }
    case MSG_A2M_SPH_ROUTER_ON: {
        const bool pcm_route_on = GetMessageParam(ccci_buff) & 0x1;
        if (pcm_route_on == true) { // pcm route on
            SetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
        } else { // pcm route off
            if (GetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK) == true) {
                ResetModemSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
            } else {
                ALOGE("--SpeechRouterOn Ack is not paired!!");
            }
        }

        break;
    }
    case MSG_A2M_PCM_REC_ON: {
        SetModemSideModemStatus(RECORD_STATUS_MASK);
        break;
    }
    case MSG_A2M_PCM_REC_OFF: {
        if (GetModemSideModemStatus(RECORD_STATUS_MASK) == true) {
            ResetModemSideModemStatus(RECORD_STATUS_MASK);
        } else {
            ALOGE("--recordOff Ack is not paired!!");
        }
        break;
    }
    case MSG_A2M_RECORD_RAW_PCM_ON: {
        SetModemSideModemStatus(RAW_RECORD_STATUS_MASK);
        break;
    }
    case MSG_A2M_RECORD_RAW_PCM_OFF: {
        if (GetModemSideModemStatus(RAW_RECORD_STATUS_MASK) == true) {
            ResetModemSideModemStatus(RAW_RECORD_STATUS_MASK);
        } else {
            ALOGE("--RawRecordOff Ack is not paired!!");
        }
        break;
    }
    case MSG_A2M_VM_REC_ON: {
        SetModemSideModemStatus(VM_RECORD_STATUS_MASK);
        break;
    }
    case MSG_A2M_VM_REC_OFF: {
        if (GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true) {
            ResetModemSideModemStatus(VM_RECORD_STATUS_MASK);
        } else {
            ALOGE("--VMRecordOff Ack is not paired!!");
        }
        break;
    }
    case MSG_A2M_BGSND_ON: {
        SetModemSideModemStatus(BGS_STATUS_MASK);
        break;
    }
    case MSG_A2M_BGSND_OFF: {
        if (GetModemSideModemStatus(BGS_STATUS_MASK) == true) {
            ResetModemSideModemStatus(BGS_STATUS_MASK);
        } else {
            ALOGE("--BGSoundOff Ack is not paired!!");
        }
        break;
    }
    case MSG_A2M_PNW_ON: {
        SetModemSideModemStatus(P2W_STATUS_MASK);
        break;
    }
    case MSG_A2M_PNW_OFF: {
        if (GetModemSideModemStatus(P2W_STATUS_MASK) == true) {
            ResetModemSideModemStatus(P2W_STATUS_MASK);
        } else {
            ALOGE("--PCM2WayOff Ack is not paired!!");
        }
        break;
    }
    case MSG_A2M_CTM_ON: {
        SetModemSideModemStatus(TTY_STATUS_MASK);
        break;
    }
    case MSG_A2M_CTM_OFF: {
        if (GetModemSideModemStatus(TTY_STATUS_MASK) == true) {
            ResetModemSideModemStatus(TTY_STATUS_MASK);
        } else {
            ALOGE("--TtyCtmOff Ack is not paired!!");
        }
        break;
    }
    case MSG_A2M_SET_ACOUSTIC_LOOPBACK: {
        const bool loopback_on = GetMessageParam(ccci_buff) & 0x1;
        if (loopback_on == true) { // loopback on
            SetModemSideModemStatus(LOOPBACK_STATUS_MASK);
        } else { // loopback off
            if (GetModemSideModemStatus(LOOPBACK_STATUS_MASK) == true) {
                ResetModemSideModemStatus(LOOPBACK_STATUS_MASK);
            } else {
                ALOGE("--SetAcousticLoopback Ack is not paired!!");
            }
        }

        mLad->Signal();
        break;
    }
    case MSG_A2M_EM_NB:
    case MSG_A2M_EM_WB:
    case MSG_A2M_EM_DMNR:
    case MSG_A2M_EM_MAGICON:
    case MSG_A2M_EM_HAC:
    case MSG_A2M_VIBSPK_PARAMETER:
    case MSG_A2M_SMARTPA_PARAMETER:
#ifndef USE_CCCI_SHARE_BUFFER
    case MSG_A2M_EM_DYNAMIC_SPH:
#endif
        {
            A2MBufUnLock();
            break;
        }
#ifdef USE_CCCI_SHARE_BUFFER
    case MSG_A2M_EM_DYNAMIC_SPH: {
        // clear CCCI parameter Buffer
        InitA2MRawParaRingBuffer();
        break;
    }
#endif
    default: {
        ALOGW("%s(), message: 0x%x, ack don't care", __FUNCTION__, ccci_buff.message);
    }
    }
}

uint16_t SpeechMessengerEVDO::GetPcmFreq(const uint16_t Idx_Freq) {
    uint16_t Pcm_Freq = 8000;

    switch (Idx_Freq) {
    case 0:
        Pcm_Freq = 8000;
        break;

    case 1:
        Pcm_Freq = 16000;
        break;

    case 2:
        Pcm_Freq = 32000;
        break;

    case 3:
        Pcm_Freq = 48000;
        break;

    case 0xf:
        Pcm_Freq = 8000;
        break;

    default:
        Pcm_Freq = 8000;
        break;

    }
    return Pcm_Freq;
}

RingBuf SpeechMessengerEVDO::GetM2AUplinkRingBuffer(const ccci_buff_t &ccci_buff) {
    // check MD side data msg format
    CheckOffsetAndLength(ccci_buff);

    RingBuf ul_ring_buf;

    ul_ring_buf.bufLen   = mM2AShareBufLen;
    ul_ring_buf.pBufBase = mM2AShareBufBase;
    ul_ring_buf.pBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;

    ul_ring_buf.pRead    = ul_ring_buf.pBufBase + GetMessageOffset(ccci_buff);
    if (ul_ring_buf.pRead >= mM2AShareBufEnd) { ul_ring_buf.pRead -= ul_ring_buf.bufLen; }

    // share buffer header
    char *p_sync_word = ul_ring_buf.pRead + 0; // 0 * size(unsigned short)
    char *p_data_type = ul_ring_buf.pRead + 2; // 1 * size(unsigned short)
    char *p_data_len  = ul_ring_buf.pRead + 4; // 2 * size(unsigned short)

    if (p_data_type >= mM2AShareBufEnd) { p_data_type -= ul_ring_buf.bufLen; }
    if (p_data_len  >= mM2AShareBufEnd) { p_data_len  -= ul_ring_buf.bufLen; }

    if (*(uint16_t *)p_sync_word != CCCI_M2A_SHARE_BUFF_HEADER_SYNC) {
        char TotalIdxDump = 10, IdxDump;
        char SphDumpStr[SPH_DUMP_STR_SIZE] = {0};

        snprintf(SphDumpStr, SPH_DUMP_STR_SIZE, "ring_buf[pBase 0x%p, pEnd 0x%p, Len 0x%x, pRead 0x%p, offset 0x%x]= ", ul_ring_buf.pBufBase, mM2AShareBufEnd, mM2AShareBufLen, ul_ring_buf.pRead, ccci_buff.message);
        for (IdxDump = 0; IdxDump < TotalIdxDump; IdxDump++) {
            char SphDumpTemp[100];
            snprintf(SphDumpTemp, 100, "[%d, 0x%p]0x%x, ", IdxDump, ul_ring_buf.pRead + IdxDump, *((uint16_t *)ul_ring_buf.pRead + IdxDump));
            audio_strncat(SphDumpStr, SphDumpTemp, SPH_DUMP_STR_SIZE);
        }
        if (IdxDump != 0) {
            ALOGD("%s(), offset(0x%x), %s", __FUNCTION__, GetMessageOffset(ccci_buff), SphDumpStr);
        }
        ALOGD("%s(), message = 0x%x, *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x)",
              __FUNCTION__, ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len);
    }
    ASSERT(*(uint16_t *)p_sync_word == CCCI_M2A_SHARE_BUFF_HEADER_SYNC);

    ul_ring_buf.pRead += CCCI_SHARE_BUFF_HEADER_LEN;
    if (ul_ring_buf.pRead >= mM2AShareBufEnd) { ul_ring_buf.pRead -= ul_ring_buf.bufLen; }

    ul_ring_buf.pWrite = ul_ring_buf.pRead + (*(uint16_t *)p_data_len);
    if (ul_ring_buf.pWrite >= mM2AShareBufEnd) { ul_ring_buf.pWrite -= ul_ring_buf.bufLen; }

#if 0
    uint16_t count = *(uint16_t *)p_data_len;
    if (ul_ring_buf.pRead <= ul_ring_buf.pWrite) {
        fwrite((void *)ul_ring_buf.pRead, sizeof(char), count, fout2);
    } else {
        char *end = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;
        int r2e = end - ul_ring_buf.pRead;
        if (count <= r2e) {
            fwrite((void *)ul_ring_buf.pRead, sizeof(char), count, fout2);
        } else {
            fwrite((void *)ul_ring_buf.pRead, sizeof(char), r2e, fout2);
            fwrite((void *)ul_ring_buf.pBufBase, sizeof(char), count - r2e, fout2);
        }
    }
#endif

    return ul_ring_buf;
}

uint16_t SpeechMessengerEVDO::GetM2AUplinkData(const ccci_buff_t &ccci_buff, char *TargetBuf) {
    char hdrPcmBuf[CCCI_SHARE_BUFF_HEADER_LEN] = {0};

    // check MD side data msg format
    CheckOffsetAndLength(ccci_buff);

    RingBuf ul_ring_buf;

    ul_ring_buf.bufLen   = mM2AShareBufLen;
    ul_ring_buf.pBufBase = mM2AShareBufBase;

    ul_ring_buf.pRead    = ul_ring_buf.pBufBase + GetMessageOffset(ccci_buff);
    if (ul_ring_buf.pRead >= mM2AShareBufEnd) { ul_ring_buf.pRead -= ul_ring_buf.bufLen; }

    //copy header 6 first
    if ((ul_ring_buf.pRead + CCCI_SHARE_BUFF_HEADER_LEN) >= mM2AShareBufEnd) {
        uint16_t byteBufEnd = mM2AShareBufEnd - ul_ring_buf.pRead;
        memcpy(hdrPcmBuf, ul_ring_buf.pRead, byteBufEnd);
        memcpy(hdrPcmBuf + byteBufEnd, mM2AShareBufBase, CCCI_SHARE_BUFF_HEADER_LEN - byteBufEnd);
        ul_ring_buf.pRead = mM2AShareBufBase + CCCI_SHARE_BUFF_HEADER_LEN - byteBufEnd;
    } else {
        memcpy(hdrPcmBuf, (char *)ul_ring_buf.pRead, CCCI_SHARE_BUFF_HEADER_LEN);
        ul_ring_buf.pRead = ul_ring_buf.pRead + CCCI_SHARE_BUFF_HEADER_LEN;
    }
    // share buffer header
    char *p_sync_word = hdrPcmBuf + 0; // size(unsigned short)
    char *p_data_type = hdrPcmBuf + 2; // size(unsigned short)
    char *p_data_len  = hdrPcmBuf + 4; // size(unsigned short)
    uint16_t ByteData = *(uint16_t *)p_data_len;
    if (*(uint16_t *)p_sync_word != CCCI_M2A_SHARE_BUFF_HEADER_SYNC) {
        char TotalIdxDump = 10, IdxDump;
        char SphDumpStr[SPH_DUMP_STR_SIZE] = {0};

        snprintf(SphDumpStr, SPH_DUMP_STR_SIZE, "share_buf[pBase 0x%p, pEnd 0x%p, Len 0x%x, pRead 0x%p, offset 0x%x]= ", mM2AShareBufBase, mM2AShareBufEnd, mM2AShareBufLen, ul_ring_buf.pRead, ccci_buff.message);
        for (IdxDump = 0; IdxDump < TotalIdxDump; IdxDump++) {
            char SphDumpTemp[100];
            snprintf(SphDumpTemp, 100, "[%d, 0x%p]0x%x, ", IdxDump, ul_ring_buf.pRead + IdxDump, *((uint16_t *)ul_ring_buf.pRead + IdxDump));
            audio_strncat(SphDumpStr, SphDumpTemp, SPH_DUMP_STR_SIZE);
        }
        if (IdxDump != 0) {
            ALOGD("%s(), offset(0x%x), %s", __FUNCTION__, GetMessageOffset(ccci_buff), SphDumpStr);
        }
        ALOGD("%s(), message = 0x%x, *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x)",
              __FUNCTION__, ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len);
    }
    ASSERT(*(uint16_t *)p_sync_word == CCCI_M2A_SHARE_BUFF_HEADER_SYNC);
    //copy data
    if ((ul_ring_buf.pRead + ByteData) >= mM2AShareBufEnd) {
        uint16_t byteBufEnd = mM2AShareBufEnd - ul_ring_buf.pRead;
        memcpy(TargetBuf, ul_ring_buf.pRead, byteBufEnd);
        memcpy(TargetBuf + byteBufEnd, mM2AShareBufBase, ByteData - byteBufEnd);
    } else {
        memcpy(TargetBuf, (char *)ul_ring_buf.pRead, ByteData);
    }
    ALOGV("-%s(), TargetBuf(0x%p), ByteData(0x%x)", __FUNCTION__, TargetBuf, ByteData);
    return ByteData;
}

RingBuf SpeechMessengerEVDO::GetM2ARawPcmRingBuffer(const ccci_buff_t &ccci_buff) {
    spcRAWPCMBufInfo header_RawPcmBufInfo;
    spcApRAWPCMBufHdr header_ApRawPcmBuf;

    char SeqPcmBuf[CCCI_MAX_PAYLOAD_SIZE * 4 + 16] = {0};
    RingBuf ul_ring_buf;
    uint16_t size_copy1, size_copy2, BytesToCopy;
    uint16_t BytesCopied = 0;
    char *PtrTarget = NULL;
    char *PtrSource = NULL;

    // check MD side data msg format
    CheckOffsetAndLength(ccci_buff);
    RingBuf ul_ring_buf1;   // already have data in mM2AShareBufBase when ReadMessage
    ul_ring_buf1.bufLen   = mM2AShareBufLen;
    ul_ring_buf1.pBufBase = mM2AShareBufBase;
    ul_ring_buf1.pRead    = ul_ring_buf1.pBufBase + GetMessageOffset(ccci_buff);
    if (ul_ring_buf1.pRead >= mM2AShareBufEnd) { ul_ring_buf1.pRead -= ul_ring_buf1.bufLen; }
    ALOGD("+%s(), address check: bufLen=0x%x, pBufBase=0x%p, pRead=0x%p, mM2AShareBufEnd=0x%p, pwrite=0x%p",
          __FUNCTION__, ul_ring_buf1.bufLen, ul_ring_buf1.pBufBase, ul_ring_buf1.pRead, mM2AShareBufEnd, ul_ring_buf1.pRead + 6 + 8 + 320 + 640);

    //parse message
    char *p_sync_word = (char *)ul_ring_buf1.pRead + 0; // size(unsigned short)
    char *p_data_type = (char *)ul_ring_buf1.pRead + 2; // size(unsigned short)
    char *p_data_len  = (char *)ul_ring_buf1.pRead + 4; // size(unsigned short)
    if (p_data_type >= mM2AShareBufEnd) { p_data_type -= ul_ring_buf1.bufLen; }
    if (p_data_len  >= mM2AShareBufEnd) { p_data_len  -= ul_ring_buf1.bufLen; }

    if (*(uint16_t *)p_sync_word != CCCI_M2A_SHARE_BUFF_HEADER_SYNC) {
        char TotalIdxDump = 10, IdxDump;
        char SphDumpStr[SPH_DUMP_STR_SIZE] = {0};

        snprintf(SphDumpStr, SPH_DUMP_STR_SIZE, "share_buf[pBase 0x%p, pEnd 0x%p, Len 0x%x, pRead 0x%p, offset 0x%x]= ", mM2AShareBufBase, mM2AShareBufEnd, mM2AShareBufLen, ul_ring_buf1.pRead, ccci_buff.message);
        for (IdxDump = 0; IdxDump < TotalIdxDump; IdxDump++) {
            char SphDumpTemp[100];
            snprintf(SphDumpTemp, 100, "[%d, 0x%p]0x%x, ", IdxDump, ul_ring_buf1.pRead + IdxDump, *((uint16_t *)ul_ring_buf1.pRead + IdxDump));
            audio_strncat(SphDumpStr, SphDumpTemp, SPH_DUMP_STR_SIZE);
        }
        if (IdxDump != 0) {
            ALOGD("%s(), offset(0x%x), %s", __FUNCTION__, GetMessageOffset(ccci_buff), SphDumpStr);
        }
        ALOGD("%s(), message = 0x%x, *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x), mPcmRecordType=%d",
              __FUNCTION__, ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len, mPcmRecordType.direction);
    }
    ASSERT(*(uint16_t *)p_sync_word == CCCI_M2A_SHARE_BUFF_HEADER_SYNC);


    // share buffer header
    if ((ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN) >= mM2AShareBufEnd) {
        char *ptr = mM2AShareBufBase + (ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN - mM2AShareBufEnd);
        memcpy(&header_RawPcmBufInfo, ptr, sizeof(spcRAWPCMBufInfo));
    } else if ((ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN + sizeof(spcRAWPCMBufInfo)) >= mM2AShareBufEnd) {
        uint16_t size = mM2AShareBufEnd - (ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN);
        char *ptr = (char *)(&header_RawPcmBufInfo);
        memcpy(&header_RawPcmBufInfo, ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN, size);
        memcpy(ptr + size, mM2AShareBufBase, sizeof(spcRAWPCMBufInfo) - size);
        //ALOGD("%s(), &header_RawPcmBufInfo=0x%x, ptr+size=0x%x, size=%d, sizeof(spcRAWPCMBufInfo)=%d",__FUNCTION__, &header_RawPcmBufInfo, ptr+size, size, sizeof(spcRAWPCMBufInfo));
    } else {
        memcpy(&header_RawPcmBufInfo, (char *)ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN, sizeof(spcRAWPCMBufInfo));
    }
    PtrTarget = (char *)SeqPcmBuf;
    ALOGD("%s(), header_RawPcmBufInfo: ULFreq=%d, ULLength=%d, DLFreq=%d, DLLength=%d", __FUNCTION__, header_RawPcmBufInfo.u16ULFreq, header_RawPcmBufInfo.u16ULLength, header_RawPcmBufInfo.u16DLFreq, header_RawPcmBufInfo.u16DLLength);

    switch (mPcmRecordType.direction) {
    default:
        break;
    case RECORD_TYPE_UL:
        header_ApRawPcmBuf.u16SyncWord = EEMCS_M2A_SHARE_BUFF_HEADER_SYNC;
        header_ApRawPcmBuf.u16RawPcmDir = RECORD_TYPE_UL;
        header_ApRawPcmBuf.u16Freq = GetPcmFreq(header_RawPcmBufInfo.u16ULFreq);
        header_ApRawPcmBuf.u16Length = header_RawPcmBufInfo.u16ULLength;
        header_ApRawPcmBuf.u16Channel = 1;
        header_ApRawPcmBuf.u16BitFormat = AUDIO_FORMAT_PCM_16_BIT;
        //uplink raw pcm header
        memcpy(PtrTarget, &header_ApRawPcmBuf, sizeof(header_ApRawPcmBuf));
        BytesCopied = sizeof(header_ApRawPcmBuf);

        //uplink raw pcm
        PtrTarget = SeqPcmBuf + BytesCopied;
        PtrSource = (char *)ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN;
        if (PtrSource >= mM2AShareBufEnd) { PtrSource -= ul_ring_buf1.bufLen; }
        BytesToCopy = header_RawPcmBufInfo.u16ULLength;
        if (PtrSource + BytesToCopy >= mM2AShareBufEnd) {
            uint16_t size = mM2AShareBufEnd - PtrSource;
            memcpy(PtrTarget, PtrSource, size);
            memcpy(PtrTarget + size, mM2AShareBufBase, BytesToCopy - size);
        } else {
            memcpy(PtrTarget, PtrSource, BytesToCopy);
        }
        BytesCopied += BytesToCopy;
        break;

    case RECORD_TYPE_DL:
        header_ApRawPcmBuf.u16SyncWord = EEMCS_M2A_SHARE_BUFF_HEADER_SYNC;
        header_ApRawPcmBuf.u16RawPcmDir = RECORD_TYPE_DL;
        header_ApRawPcmBuf.u16Freq = GetPcmFreq(header_RawPcmBufInfo.u16DLFreq);
        header_ApRawPcmBuf.u16Length = header_RawPcmBufInfo.u16DLLength;
        header_ApRawPcmBuf.u16Channel = 1;
        header_ApRawPcmBuf.u16BitFormat = AUDIO_FORMAT_PCM_16_BIT;
        //downlink raw pcm header
        memcpy(PtrTarget, &header_ApRawPcmBuf, sizeof(header_ApRawPcmBuf));
        BytesCopied = sizeof(header_ApRawPcmBuf);

        //downlink raw pcm
        PtrTarget = SeqPcmBuf + BytesCopied;
        PtrSource = (char *)ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN + header_RawPcmBufInfo.u16ULLength;
        if (PtrSource >= mM2AShareBufEnd) { PtrSource -= ul_ring_buf1.bufLen; }
        BytesToCopy = header_RawPcmBufInfo.u16DLLength;
        if (PtrSource + BytesToCopy >= mM2AShareBufEnd) {
            uint16_t size = mM2AShareBufEnd - PtrSource;
            memcpy(PtrTarget, PtrSource, size);
            memcpy(PtrTarget + size, mM2AShareBufBase, BytesToCopy - size);
        } else {
            memcpy(PtrTarget, PtrSource, BytesToCopy);
        }
        BytesCopied += BytesToCopy;
        break;

    case RECORD_TYPE_MIX:
        header_ApRawPcmBuf.u16SyncWord = EEMCS_M2A_SHARE_BUFF_HEADER_SYNC;
        header_ApRawPcmBuf.u16RawPcmDir = RECORD_TYPE_UL;
        header_ApRawPcmBuf.u16Freq = GetPcmFreq(header_RawPcmBufInfo.u16ULFreq);
        header_ApRawPcmBuf.u16Length = header_RawPcmBufInfo.u16ULLength;
        header_ApRawPcmBuf.u16Channel = 1;
        header_ApRawPcmBuf.u16BitFormat = AUDIO_FORMAT_PCM_16_BIT;
        //uplink raw pcm header
        memcpy(PtrTarget, &header_ApRawPcmBuf, sizeof(header_ApRawPcmBuf));
        BytesCopied = sizeof(header_ApRawPcmBuf);

        //uplink raw pcm
        PtrTarget = SeqPcmBuf + BytesCopied;
        PtrSource = (char *)ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN;
        if (PtrSource >= mM2AShareBufEnd) { PtrSource -= ul_ring_buf1.bufLen; }
        BytesToCopy = header_RawPcmBufInfo.u16ULLength;
        if (PtrSource + BytesToCopy >= mM2AShareBufEnd) {
            uint16_t size = mM2AShareBufEnd - PtrSource;
            memcpy(PtrTarget, PtrSource, size);
            memcpy(PtrTarget + size, mM2AShareBufBase, BytesToCopy - size);
        } else {
            memcpy(PtrTarget, PtrSource, BytesToCopy);
        }
        BytesCopied += BytesToCopy;
#if 0   // Dump uplink in speech driver for debug
        FILE *pDumpUplink;
        AudiocheckAndCreateDirectory("/data/vendor/audiohal/Uplink.pcm");
        pDumpUplink = fopen("/data/vendor/audiohal/Uplink.pcm", "ab");
        if (pDumpUplink == NULL) { ALOGW("Fail to Open pDumpUplink"); }
        fwrite(PtrTarget, sizeof(char), BytesToCopy, pDumpUplink);
        fclose(pDumpUplink);
        ALOGD("%s(),ul_ring_buf1.pRead[0]=%x,pRead[1]=0x%x,pRead[2]=%x,pRead[3]=0x%x,pRead[4]=%x,pRead[5]=0x%x,pRead[6]=%x,pRead[7]=0x%x,pRead[8]=%x, pRead[9]=0x%x, ul_ring_buf1.pRead[10]=%x,pRead[11]=0x%x,pRead[12]=%x,pRead[13]=0x%x,pRead[14]=%x,pRead[15]=0x%x,pRead[16]=%x,pRead[17]=0x%x,pRead[18]=%x, pRead[19]=0x%x", __FUNCTION__,
              ul_ring_buf1.pRead[0], ul_ring_buf1.pRead[1], ul_ring_buf1.pRead[2], ul_ring_buf1.pRead[3], ul_ring_buf1.pRead[4],
              ul_ring_buf1.pRead[5], ul_ring_buf1.pRead[6], ul_ring_buf1.pRead[7], ul_ring_buf1.pRead[8], ul_ring_buf1.pRead[9],
              ul_ring_buf1.pRead[10], ul_ring_buf1.pRead[11], ul_ring_buf1.pRead[12], ul_ring_buf1.pRead[13], ul_ring_buf1.pRead[14],
              ul_ring_buf1.pRead[15], ul_ring_buf1.pRead[16], ul_ring_buf1.pRead[17], ul_ring_buf1.pRead[18], ul_ring_buf1.pRead[19]);
#endif
        PtrTarget = SeqPcmBuf + BytesCopied;


        //downlink raw pcm header
        header_ApRawPcmBuf.u16RawPcmDir = RECORD_TYPE_DL;
        header_ApRawPcmBuf.u16Freq = GetPcmFreq(header_RawPcmBufInfo.u16DLFreq);
        header_ApRawPcmBuf.u16Length = header_RawPcmBufInfo.u16DLLength;
        memcpy(PtrTarget, &header_ApRawPcmBuf, sizeof(header_ApRawPcmBuf));
        BytesCopied += sizeof(header_ApRawPcmBuf);

        //downlink raw pcm
        PtrTarget = SeqPcmBuf + BytesCopied;
        PtrSource = (char *)ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN + header_RawPcmBufInfo.u16ULLength;
        if (PtrSource >= mM2AShareBufEnd) { PtrSource -= ul_ring_buf1.bufLen; }
        BytesToCopy = header_RawPcmBufInfo.u16DLLength;
        if (PtrSource + BytesToCopy >= mM2AShareBufEnd) {
            uint16_t size = mM2AShareBufEnd - PtrSource;
            memcpy(PtrTarget, PtrSource, size);
            memcpy(PtrTarget + size, mM2AShareBufBase, BytesToCopy - size);
        } else {
            memcpy(PtrTarget, PtrSource, BytesToCopy);
        }
        BytesCopied += BytesToCopy;
#if 0   // Dump downlink in speech driver for debug
        FILE *pDumpDownlink;
        AudiocheckAndCreateDirectory("/data/vendor/audiohal/Downlink.pcm");
        pDumpDownlink = fopen("/data/vendor/audiohal/Downlink.pcm", "ab");
        if (pDumpDownlink == NULL) { ALOGW("Fail to Open pDumpDownlink"); }
        fwrite(PtrTarget, sizeof(char), BytesToCopy, pDumpDownlink);
        fclose(pDumpDownlink);
        ALOGD("%s(),ul_ring_buf1.pRead[DL last]=0x%x", __FUNCTION__, ul_ring_buf1.pRead[6 + 10 + 320 + 640]);
#endif
        break;
    }


    //ring buffer process, pass to StreamIn
    ul_ring_buf.bufLen   = mM2AShareBufLen;
    ul_ring_buf.pBufBase = mM2AShareBufBase;
    ul_ring_buf.pBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;

    ul_ring_buf.pRead = mM2AShareBufRead;

    ul_ring_buf.pWrite   = ul_ring_buf.pRead + BytesCopied;
    if (ul_ring_buf.pWrite >= mM2AShareBufEnd) {
        ul_ring_buf.pWrite -= ul_ring_buf.bufLen;

        size_copy1 = mM2AShareBufEnd - ul_ring_buf.pRead;
        size_copy2 = BytesCopied - size_copy1;
        ALOGD("%s(), mM2AShareBufEnd(0x%p), BytesCopied(0x%x), size_copy1(0x%x), size_copy2(0x%x), pRead(0x%p), pWrite(0x%p)",
              __FUNCTION__, mM2AShareBufEnd, BytesCopied, size_copy1, size_copy2, ul_ring_buf.pRead, ul_ring_buf.pWrite);
        memcpy(ul_ring_buf.pRead, (char *)SeqPcmBuf, size_copy1);
        memcpy(ul_ring_buf.pBufBase, (char *)SeqPcmBuf + size_copy1, size_copy2);
    } else {
        memcpy(ul_ring_buf.pRead, (char *)SeqPcmBuf, BytesCopied);
    }
    ALOGV("-%s(), pBufBase(0x%p), bufLen(0x%x), BytesCopied(0x%x), mM2AShareBufRead(0x%p), pRead(0x%p), pWrite(0x%p)",
          __FUNCTION__, ul_ring_buf.pBufBase, ul_ring_buf.bufLen, BytesCopied, mM2AShareBufRead, ul_ring_buf.pRead, ul_ring_buf.pWrite);

    mM2AShareBufRead = ul_ring_buf.pWrite;

    return ul_ring_buf;
}

uint16_t SpeechMessengerEVDO::GetM2AShareBufSyncWord(const ccci_buff_t &ccci_buff) {
    char *p_sync_word = GetM2AShareBufBase() + GetMessageOffset(ccci_buff) + 0; // 0 * size(unsigned short)
    if (p_sync_word >= mM2AShareBufEnd) { p_sync_word -= mM2AShareBufLen; }
    SLOGV("%s(), sync = 0x%x", __FUNCTION__, *(uint16_t *)p_sync_word);
    return *(uint16_t *)p_sync_word;
}

uint16_t SpeechMessengerEVDO::GetM2AShareBufDataType(const ccci_buff_t &ccci_buff) {
    char *p_data_type = GetM2AShareBufBase() + GetMessageOffset(ccci_buff) + 2; // 1 * size(unsigned short)
    if (p_data_type >= mM2AShareBufEnd) { p_data_type -= mM2AShareBufLen; }
    SLOGV("%s(), type = 0x%x", __FUNCTION__, *(uint16_t *)p_data_type);
    return *(uint16_t *)p_data_type;
}

uint16_t SpeechMessengerEVDO::GetM2AShareBufDataLength(const ccci_buff_t &ccci_buff) {
    char *p_data_len  = GetM2AShareBufBase() + GetMessageOffset(ccci_buff) + 4; // 2 * size(unsigned short)
    if (p_data_len  >= mM2AShareBufEnd) { p_data_len  -= mM2AShareBufLen; }
    SLOGV("%s(), data_len = %d", __FUNCTION__, *(uint16_t *)p_data_len);
    return *(uint16_t *)p_data_len;
}


bool SpeechMessengerEVDO::GetModemSideModemStatus(const modem_status_mask_t modem_status_mask) const {
    return ((mModemSideModemStatus & modem_status_mask) > 0);
}

void SpeechMessengerEVDO::SetModemSideModemStatus(const modem_status_mask_t modem_status_mask) {
    mModemSideModemStatus |= modem_status_mask;

    // save mModemSideModemStatus in kernel to avoid medieserver die
    set_uint32_to_mixctrl(kPropertyKeyModemStatus, mModemSideModemStatus);
}

void SpeechMessengerEVDO::ResetModemSideModemStatus(const modem_status_mask_t modem_status_mask) {
    mModemSideModemStatus &= (~modem_status_mask);

   // save mModemSideModemStatus in kernel to avoid medieserver die
    set_uint32_to_mixctrl(kPropertyKeyModemStatus, mModemSideModemStatus);
}

status_t SpeechMessengerEVDO::setPcmRecordType(SpcRecordTypeStruct typeRecord) {
    mPcmRecordType = typeRecord;
    ALOGD("%s(), mPcmRecordType(%d)", __FUNCTION__, mPcmRecordType.direction);
    return NO_ERROR;
}

status_t SpeechMessengerEVDO::SetNWCodecInfo(const ccci_buff_t &ccci_buff) {
    spcCodecInfoStruct sCodecInfo;
    int returnPinfo = 0, returnHStatus = 0;
    char SeqPcmBuf[CCCI_MAX_PAYLOAD_SIZE * 4 + 16] = {0};
    RingBuf ul_ring_buf;
    uint16_t size_copy1, size_copy2, BytesCopied, BytesToCopy;
    char *PtrTarget = NULL;
    char *PtrSource = NULL;

    // check MD side data msg format
    CheckOffsetAndLength(ccci_buff);
    RingBuf ul_ring_buf1;   // already have data in mM2AShareBufBase when ReadMessage
    ul_ring_buf1.bufLen   = mM2AShareBufLen;
    ul_ring_buf1.pBufBase = mM2AShareBufBase;
    ul_ring_buf1.pRead    = ul_ring_buf1.pBufBase + GetMessageOffset(ccci_buff);
    if (ul_ring_buf1.pRead >= mM2AShareBufEnd) { ul_ring_buf1.pRead -= ul_ring_buf1.bufLen; }
    ALOGV("%s(), address check: bufLen=0x%x, pBufBase=0x%p, pRead=0x%p, mM2AShareBufEnd=0x%p, pwrite=0x%p",
          __FUNCTION__, ul_ring_buf1.bufLen, ul_ring_buf1.pBufBase, ul_ring_buf1.pRead, mM2AShareBufEnd, ul_ring_buf1.pRead + 6 + 8 + 320 + 640);

    //parse message
    char *p_sync_word = (char *)ul_ring_buf1.pRead + 0; // size(unsigned short)
    char *p_data_type = (char *)ul_ring_buf1.pRead + 2; // size(unsigned short)
    char *p_data_len  = (char *)ul_ring_buf1.pRead + 4; // size(unsigned short)
    if (p_data_type >= mM2AShareBufEnd) { p_data_type -= ul_ring_buf1.bufLen; }
    if (p_data_len  >= mM2AShareBufEnd) { p_data_len  -= ul_ring_buf1.bufLen; }

    ALOGV("%s(), payload header: *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x), mPcmRecordType=%d",
          __FUNCTION__, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len, mPcmRecordType.direction);

    ASSERT(*(uint16_t *)p_sync_word == CCCI_M2A_SHARE_BUFF_HEADER_SYNC);


    // share buffer header
    if ((ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN) >= mM2AShareBufEnd) {
        char *ptr = mM2AShareBufBase + (ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN - mM2AShareBufEnd);
        memcpy(&sCodecInfo, ptr, sizeof(spcCodecInfoStruct));
    } else if ((ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN + sizeof(spcCodecInfoStruct)) >= mM2AShareBufEnd) {
        uint16_t size = mM2AShareBufEnd - (ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN);
        char *ptr = (char *)(&sCodecInfo);
        memcpy(&sCodecInfo, ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN, size);
        memcpy(ptr + size, mM2AShareBufBase, sizeof(sCodecInfo) - size);
        //ALOGD("%s(), &header_RawPcmBufInfo=0x%x, ptr+size=0x%x, size=%d, sizeof(spcRAWPCMBufInfo)=%d",__FUNCTION__, &header_RawPcmBufInfo, ptr+size, size, sizeof(spcRAWPCMBufInfo));
    } else {
        memcpy(&sCodecInfo, (char *)ul_ring_buf1.pRead + CCCI_SHARE_BUFF_HEADER_LEN, sizeof(spcCodecInfoStruct));
    }

    ALOGD("%s(), sCodecInfo.codecInfo = %s, sCodecInfo.codecOp = %s", __FUNCTION__, sCodecInfo.codecInfo, sCodecInfo.codecOp);

    returnPinfo = property_set(PROPERTY_KEY_NW_CODEC_INFO[0], sCodecInfo.codecInfo);
    returnHStatus = property_set(PROPERTY_KEY_NW_CODEC_INFO[1], sCodecInfo.codecOp);

    char pValue_CodecInfo[PROPERTY_VALUE_MAX];
    char pValue_HDVoiceStatus[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_NW_CODEC_INFO[0], pValue_CodecInfo, ""); //"": default
    property_get(PROPERTY_KEY_NW_CODEC_INFO[1], pValue_HDVoiceStatus, ""); //"": default
    ALOGD("%s(), get %s = %s, %s = %s", __FUNCTION__, PROPERTY_KEY_NW_CODEC_INFO[0], pValue_CodecInfo, PROPERTY_KEY_NW_CODEC_INFO[1], pValue_HDVoiceStatus);

    return NO_ERROR;

}
#ifdef USE_CCCI_SHARE_BUFFER
void SpeechMessengerEVDO::InitA2MRawParaRingBuffer() {
    mA2MParaShareBuf.pBufBase =  mA2MParaShareBufBase + CCCI_SHAREBUF_GUARD_LENGTH;
    mA2MParaShareBuf.pRead =  mA2MParaShareBuf.pBufBase;
    mA2MParaShareBuf.pWrite =  mA2MParaShareBuf.pBufBase;
    mA2MParaShareBuf.bufLen =  mA2MParaShareBufLen - 2 * CCCI_SHAREBUF_GUARD_LENGTH;
    mA2MParaShareBuf.pBufEnd = mA2MParaShareBuf.pBufBase + mA2MParaShareBuf.bufLen;
    // add  guard pattern here
    memset((void *)mA2MParaShareBufBase, 0xa, CCCI_SHAREBUF_GUARD_LENGTH);
    memset((void *)mA2MParaShareBuf.pBufEnd, 0xa, CCCI_SHAREBUF_GUARD_LENGTH);
    mIsA2MParaShareBufEmpty = true;
    ALOGD("%s pBufBase %p, pBufEnd %p, bufLen %d ",  __FUNCTION__, mA2MParaShareBuf.pBufBase,
          mA2MParaShareBuf.pBufEnd, mA2MParaShareBuf.bufLen);
    return ;
}
status_t SpeechMessengerEVDO::GetA2MRawParaRingBuffer(uint16_t *offset, uint16_t *avail) {
    *offset  = (uint16_t)(mA2MParaShareBuf.pWrite - mA2MParaShareBufBase);
    if (mA2MParaShareBuf.pWrite > mA2MParaShareBuf.pRead) {
        * avail = (uint16_t)(mA2MParaShareBuf.bufLen - (mA2MParaShareBuf.pWrite - mA2MParaShareBuf.pRead));
    } else if (mA2MParaShareBuf.pWrite == mA2MParaShareBuf.pRead) {
        * avail = (uint16_t)(mIsA2MParaShareBufEmpty ?  mA2MParaShareBuf.bufLen : 0);
    } else {
        * avail = (uint16_t)(mA2MParaShareBuf.pRead - mA2MParaShareBuf.pWrite);
    }
    ALOGV("%s  mA2MParaShareBuf.pWrite %p ,*offset %d, *avail %d", __FUNCTION__,  mA2MParaShareBuf.pWrite, *offset, *avail);
    return NO_ERROR;
}

status_t SpeechMessengerEVDO::AdvanceA2MRawParaRingBuffer(int datalength) {
    ALOGV("%s, %d", __FUNCTION__, datalength);
    mA2MParaShareBuf.pRead += datalength;
    if (mA2MParaShareBuf.pRead >= mA2MParaShareBuf.pBufEnd) {
        mA2MParaShareBuf.pRead -= mA2MParaShareBuf.bufLen;
    }
    if (mA2MParaShareBuf.pRead == mA2MParaShareBuf.pWrite) {
        mIsA2MParaShareBufEmpty = true;
    }
    ALOGV("%s, mA2MParaShareBuf.pRead = %p", __FUNCTION__, mA2MParaShareBuf.pRead);
    return NO_ERROR;
}

status_t SpeechMessengerEVDO::WriteA2MRawParaRingBuffer(char *data, int datalength) {
    if (data == NULL || datalength > mA2MParaShareBuf.bufLen) {
        ALOGD("%s wrong pointer %p or data length %d", __FUNCTION__,  data, datalength);
        return -1;
    }
    int part1;
    part1 = (mA2MParaShareBuf.pWrite >= mA2MParaShareBuf.pRead) ?
            (mA2MParaShareBuf.pBufEnd - mA2MParaShareBuf.pWrite) : (mA2MParaShareBuf.pRead - mA2MParaShareBuf.pWrite);
    if (part1 > datalength) {
        SpeechMemCpy(mA2MParaShareBuf.pWrite, data, datalength);
        mA2MParaShareBuf.pWrite += datalength;
    } else {
        int part2 = datalength - part1;
        SpeechMemCpy(mA2MParaShareBuf.pWrite, data, part1);
        SpeechMemCpy(mA2MParaShareBuf.pBufBase, data + part1, part2);
        mA2MParaShareBuf.pWrite = mA2MParaShareBuf.pBufBase + part2;
    }
    mIsA2MParaShareBufEmpty = false;
    ALOGV("%s  after write %d ,mA2MParaShareBuf.pWrite %p", __FUNCTION__, datalength, mA2MParaShareBuf.pWrite);
    return NO_ERROR;
}
#endif

} // end of namespace android
