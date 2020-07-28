#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechMessengerECCCI"
#include "SpeechMessengerECCCI.h"

#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <sys/prctl.h>
#include <sys/resource.h>

#include <cutils/properties.h>

#include <AudioLock.h>

#include "SpeechDriverLAD.h"
#include <SpeechPcmMixerBGSPlayer.h>
#include "SpeechPcm2way.h"
#include "SpeechVMRecorder.h"
#include "AudioVolumeFactory.h"

#include "AudioALSAHardwareResourceManager.h"

#include "AudioALSACaptureDataProviderVoice.h"
#include "SpeechDataProcessingHandler.h"
#ifdef SPH_VCL_SUPPORT
#include "SpeechVoiceCustomLogger.h"
#endif
#include "LoopbackManager.h"

#if defined (MTK_SPEECH_ENCRYPTION_SUPPORT)
#include "SpeechDataEncrypter.h"
#endif
#include <AudioEventThreadManager.h>
#include <SpeechUtility.h>

//#include "AudioMTKStreamInManager.h"
//#include "AudioResourceManager.h"
extern "C" {
#include "hardware/ccci_intf.h"
}

#ifndef sph_msleep
#define sph_msleep(ms) usleep((ms)*1000)
#endif

//#define TEST_NEW_RECOED_FOR_OLD_CAPTURE

namespace android {

/** CCCI driver & ioctl */
/* ECCCI channel buffer structure */
typedef struct {
    unsigned int data[2];
    unsigned int channel;
    unsigned int reserved;
    unsigned int payload[CCCI_MAX_PAYLOAD_SIZE];
} ECCCI_BUFF_T;

/** Property keys*/
static const char *kPropertyKeyModemStatus = "vendor.audiohal.modem_1.status";
static const char* PROPERTY_KEY_RF_INFO[2] = {"vendor.audiohal.rf_info", "vendor.audiohal.rf_mode"};
static const char* PROPERTY_KEY_NW_CODEC_INFO[2] = {"vendor.audiohal.ril.speech.codec.info", "vendor.audiohal.ril.hd.voice.status"};


/** CCCI channel No */
static const uint8_t    CCCI_M2A_CHANNEL = 4;
static const uint8_t    CCCI_A2M_CHANNEL = 5;

/** CCCI magic number */
static const uint32_t   CCCI_MAILBOX_MAGIC_NUMBER = 0xFFFFFFFF;
static bool mA2M_ECCCI_DATA_READ_ACK;
static const uint32_t ap_working_buf_byte = 4096;


#define SPH_DUMP_STR_SIZE (500)

SpeechMessengerECCCI::SpeechMessengerECCCI(modem_index_t modem_index, SpeechDriverLAD *pLad) :
    mModemIndex(modem_index),
    mLad(pLad) {
    ALOGD("%s()", __FUNCTION__);
    CCCIEnable = false;
    mA2M_ECCCI_DATA_READ_ACK = false;

    fHdl = -1;
#ifdef USE_CCCI_SHARE_BUFFER
    CCCIBuf_Hdl = -1;
    mA2MParaShareBufBase = NULL;
    mA2MParaShareBufLen = 0;
#endif

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

    //initial modem side modem status
    mModemSideModemStatus = get_uint32_from_mixctrl(kPropertyKeyModemStatus);
    ALOGD("%s(), mModemIndex(%d), mModemSideModemStatus(0x%x)", __FUNCTION__, mModemIndex, mModemSideModemStatus);

    ResetSpeechParamAckCount();
    mLogEnable = 0;
}

status_t SpeechMessengerECCCI::Initial() {
    ALOGD("%s()", __FUNCTION__);
#if defined(MTK_ECCCI_LPBK_TEST)    //use eccci loopback for test
    fHdl  = open("/dev/ccci_lp", O_RDWR);
    if (fHdl < 0) {
        ALOGE("%s() fail to open/dev/ccci_lp ", __FUNCTION__);
        return UNKNOWN_ERROR;
    }
#else
    // Open CCCI and get handler
    char dev_node[32];
    // Open CCCI and get handler
    switch (mModemIndex) {
    case MODEM_1:
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_AUDIO_RX, MD_SYS1));
        break;
    case MODEM_2:
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_AUDIO_RX, MD_SYS2));
        break;
    default:
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_AUDIO_RX, MD_SYS1));
        break;
    }
    ALOGD("%s(), dev_node=%s", __FUNCTION__, dev_node);
    fHdl = open(dev_node, O_RDWR);
    if (fHdl < 0) {
        ALOGE("%s() fail to open %s, errno: %d", __FUNCTION__, dev_node, errno);
        return UNKNOWN_ERROR;
    }
#endif

#ifdef USE_CCCI_SHARE_BUFFER
    CCCI_MD md_id = mModemIndex == MODEM_1 ? MD_SYS1 : MD_SYS3;
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

    mIsModemResetDuringPhoneCall = false;
    mIsModemReset = false;
    mIsModemEPOF = false;
    mPcmRecordType.direction = RECORD_TYPE_UL;
    LastSentMessage = 0;
    LastNeedAckSentMessage = 0;

    /* create the CCCI event reading thread */
    CCCIEnable = true;
#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
    //trigger initialization
    SpeechDataEncrypter::GetInstance()->GetDumpStatus();
#endif
    return CreateReadingThread();
}

status_t SpeechMessengerECCCI::Deinitial() {
    ALOGD("%s()", __FUNCTION__);
    CCCIEnable = false;

    if (fHdl >= 0) {
        close(fHdl);
        fHdl = -1;
    }
#ifdef USE_CCCI_SHARE_BUFFER
    if (CCCIBuf_Hdl >= 0) {
        ccci_smem_put(CCCIBuf_Hdl, (unsigned char *) mA2MParaShareBufBase, mA2MParaShareBufLen);
        CCCIBuf_Hdl = -1;
    }
    mA2MParaShareBufBase = NULL;
    mA2MParaShareBufLen = 0;
#endif

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

    return NO_ERROR;
}


SpeechMessengerECCCI::~SpeechMessengerECCCI() {
    ALOGD("%s()", __FUNCTION__);
}

/** Create CCCI message */
ccci_buff_t SpeechMessengerECCCI::InitCcciMailbox(uint16_t id, uint16_t param_16bit, uint32_t param_32bit) {
    ccci_buff_t ccci_buff;
    memset(&ccci_buff, 0, sizeof(ccci_buff));


    //package message
    switch (id) {
    case MSG_A2M_PNW_DL_DATA_NOTIFY:
    case MSG_A2M_BGSND_DATA_NOTIFY:
    case MSG_A2M_CTM_DATA_NOTIFY:
    case MSG_A2M_EM_NB:
    case MSG_A2M_EM_WB:
    case MSG_A2M_EM_DMNR:
    case MSG_A2M_EM_MAGICON:
    case MSG_A2M_EM_HAC:
    case MSG_A2M_VIBSPK_PARAMETER:
    case MSG_A2M_SMARTPA_PARAMETER:
    case MSG_A2M_SPH_UL_ENCRYPTION:
    case MSG_A2M_SPH_DL_DECRYPTION:
#ifndef USE_CCCI_SHARE_BUFFER
    case MSG_A2M_EM_DYNAMIC_SPH:
#endif
        ccci_buff.magic    = param_32bit;//offset
        ccci_buff.message  = param_16bit;//payload length
        ccci_buff.channel  = CCCI_A2M_CHANNEL;
        ccci_buff.reserved = (id << 16) | param_16bit;
        break;
    default:
        ccci_buff.magic    = CCCI_MAILBOX_MAGIC_NUMBER;
        ccci_buff.message  = (id << 16) | param_16bit;
        ccci_buff.channel  = CCCI_A2M_CHANNEL;
        ccci_buff.reserved = param_32bit;
        break;
    }


    return ccci_buff;
}

/** Get CCCI message's ID */
uint16_t SpeechMessengerECCCI::GetMessageID(const ccci_buff_t &ccci_buff) {
    if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER) {
        return (ccci_buff.reserved) >> 16;
    } else {
        return (ccci_buff.message) >> 16;
    }
}

/** Get CCCI message's parameters */
uint16_t SpeechMessengerECCCI::GetMessageParam(const ccci_buff_t &ccci_buff) {
    if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER) {
        return ccci_buff.reserved;
    } else {
        return ccci_buff.message;
    }
}
uint32_t SpeechMessengerECCCI::GetMessageReserved(const ccci_buff_t &ccci_buff) {
    return ccci_buff.reserved;
}

uint16_t SpeechMessengerECCCI::GetMessageLength(const ccci_buff_t &ccci_buff) {
    //payload length(6+data)
    if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER) {
        return ccci_buff.message;
    } else {
        return ccci_buff.message & 0xFFFF;
    }
}

uint16_t SpeechMessengerECCCI::GetMessageOffset(const ccci_buff_t &ccci_buff) {
    if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER) {
        return ccci_buff.magic;
    } else {
        return 0;//no offset
    }
}


char SpeechMessengerECCCI::GetModemCurrentStatus() {
    unsigned int  status = (unsigned int)MODEM_STATUS_INVALID;

    int retval = ::ioctl(fHdl, CCCI_IOC_GET_MD_STATE, &status);
    if (mIsModemEPOF == true) {
        ALOGD("%s() MD EPOF!", __FUNCTION__);
        status = MODEM_STATUS_INVALID;
    } else if (retval < 0) {
        ALOGE("%s() ioctl CCCI_IOC_GET_MD_STATE fail!! retval = %d, errno: %d", __FUNCTION__, retval, errno);
        status = MODEM_STATUS_EXPT;
    }

#if defined(MTK_ECCCI_LPBK_TEST)    //use eccci loopback for test, md status always set ready
    status = MODEM_STATUS_READY;
#endif
    return (char)status;
}

bool SpeechMessengerECCCI::CheckModemIsReady() {
    return (GetModemCurrentStatus() == MODEM_STATUS_READY);
}

status_t SpeechMessengerECCCI::WaitUntilModemReady() {
    char status = 0;
    uint32_t trycnt = 0;
    const uint32_t kMaxTryCnt = 10; // total 200 msec
    do {
        status = GetModemCurrentStatus();
        if (status == MODEM_STATUS_READY) {
            ALOGD("%s(): Modem ready", __FUNCTION__);
            break;
        } else {
            ALOGW("Wait CCCI open #%d times, modem current status = %d, errno: %d", ++trycnt, status, errno);
            sph_msleep(20);
            if (trycnt == kMaxTryCnt) { break; }
        }
    } while (1);

    return (trycnt < kMaxTryCnt) ? NO_ERROR : TIMED_OUT;
}

#undef LOG_TAG
#define LOG_TAG "SpeechMessengerECCCI_Send"

int32_t SpeechMessengerECCCI::SendMessage(const ccci_buff_t &ccci_buff) {
    ECCCI_BUFF_T msgBuf;
    uint16_t length_write, size_message;
    int8_t *pShareBuf;
    ALOGV("%s()", __FUNCTION__);

    // check if already initialized
    if (fHdl < 0) {
        if (Initial() != NO_ERROR) {
            return UNKNOWN_ERROR;
        }
    }

    // check if need ack
    uint16_t message_id = GetMessageID(ccci_buff);
    const bool b_need_ack = (JudgeAckOfMsg(message_id) == MESSAGE_NEED_ACK) ? true : false;
    uint16_t size_payload = GetMessageLength(ccci_buff);//(6+len_data)
    uint16_t offset_buffer = GetMessageOffset(ccci_buff);

    // check modem status during phone call
    char modem_status = GetModemCurrentStatus();
    if (modem_status != MODEM_STATUS_READY) {
        ALOGE("%s(), modem_status(%d) != MODEM_STATUS_READY, errno: %d, mIsModemEPOF = %d", __FUNCTION__, modem_status, errno, mIsModemEPOF);
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

    // send message
    int i = 0;
    int write_length = 0;
    modem_status = 0;
    status_t ret = UNKNOWN_ERROR;
    message_id = GetMessageID(ccci_buff);
    memcpy(&msgBuf, &ccci_buff, sizeof(ccci_buff_t));
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
        msgBuf.data[0] = 0;
        //copy buffer
        pShareBuf = (int8_t *)GetA2MShareBufBase() + offset_buffer;
        size_message = size_payload + CCCI_BUF_HEADER_SIZE;
        memcpy(msgBuf.payload, pShareBuf, size_payload);
        ALOGV("SendMessage with Buffer: msgBuf.message = 0x%x, size_message = 0x%x, size_payload = 0x%x", msgBuf.data[1], size_message, size_payload);
        memcpy(msgBuf.payload, pShareBuf, size_payload);
#ifdef CONFIG_MT_ENG_BUILD // eng load
        ALOGD("SendMessage ch = 0x%x, reserved=0x%x, msgBuf.payload[0] = 0x%x, payload[1]=0x%x, payload[2] = 0x%x, payload[3]=0x%x", msgBuf.channel, msgBuf.reserved, msgBuf.payload[0], msgBuf.payload[1], msgBuf.payload[2], msgBuf.payload[3]);
#else
        if (JudgeLogPrintOfMsg(message_id)) {
            ALOGD("SendMessage ch = 0x%x, reserved=0x%x, msgBuf.payload[0] = 0x%x, payload[1]=0x%x, payload[2] = 0x%x, payload[3]=0x%x", msgBuf.channel, msgBuf.reserved, msgBuf.payload[0], msgBuf.payload[1], msgBuf.payload[2], msgBuf.payload[3]);
        } else {
            ALOGV("SendMessage ch = 0x%x, reserved=0x%x, msgBuf.payload[0] = 0x%x, payload[1]=0x%x, payload[2] = 0x%x, payload[3]=0x%x", msgBuf.channel, msgBuf.reserved, msgBuf.payload[0], msgBuf.payload[1], msgBuf.payload[2], msgBuf.payload[3]);
        }
#endif
        break;

    case MSG_A2M_SPH_UL_ENCRYPTION:
    case MSG_A2M_SPH_DL_DECRYPTION:
        msgBuf.data[0] = 0;
        //copy buffer
        pShareBuf = (int8_t *)mSphPCMBuf + offset_buffer;

        ALOGD_IF(mLogEnable, "%s(), with Buffer: message[0x%x], pShareBuf = %p, size_payload = 0x%x",  __FUNCTION__, msgBuf.data[1], pShareBuf, size_payload);
        size_message = size_payload + CCCI_BUF_HEADER_SIZE;
        memcpy(msgBuf.payload, pShareBuf, size_payload);
        ALOGD_IF(mLogEnable, "SendMessage with Buffer: msgBuf.message = 0x%x, size_message = 0x%x, size_payload = 0x%x", msgBuf.data[1], size_message, size_payload);
        memcpy(msgBuf.payload, pShareBuf, size_payload);
        ALOGD_IF(mLogEnable, "SendMessage ch = 0x%x, reserved=0x%x, msgBuf.payload[0] = 0x%x, payload[1]=0x%x, payload[2] = 0x%x, payload[3]=0x%x", msgBuf.channel , msgBuf.reserved, msgBuf.payload[0], msgBuf.payload[1], msgBuf.payload[2], msgBuf.payload[3]);
        break;

    default:
        size_message = CCCI_BUF_HEADER_SIZE;
        ALOGD("SendMessage only command: msgBuf.message = 0x%x, size_message = 0x%x", msgBuf.data[1], size_message);
        break;
    }
    //    ALOGD("SendMessage ccci_buff.magic = 0x%x, message=0x%x, channel = 0x%x, reserved=0x%x", ccci_buff.magic, ccci_buff.message, ccci_buff.channel, msgBuf.reserved);
#ifdef CONFIG_MT_ENG_BUILD // eng load
    ALOGD("SendMessage msgBuf.data[0] = 0x%x, data[1]=0x%x, ch = 0x%x, reserved=0x%x", msgBuf.data[0], msgBuf.data[1], msgBuf.channel, msgBuf.reserved);
#else // user and user-debug load
    ALOGD_IF(mLogEnable, "SendMessage msgBuf.data[0] = 0x%x, data[1]=0x%x, ch = 0x%x, reserved=0x%x", msgBuf.data[0], msgBuf.data[1], msgBuf.channel , msgBuf.reserved);
#endif


    for (i = 0; i < 150; i++) { // try 150 times for every 2 ms if sent message fail
        length_write = write(fHdl, (void *)&msgBuf, size_message);
        ALOGV("%s() write %d times, return length_write=%d, size_message=%d", __FUNCTION__, i, length_write, size_message);
        switch (message_id) {
        case MSG_A2M_EM_NB:
        case MSG_A2M_EM_WB: // case AUDIO_HD_RECORD_PARAMETER:
        case MSG_A2M_EM_DMNR:
        case MSG_A2M_EM_MAGICON:
        case MSG_A2M_EM_HAC:
        case MSG_A2M_VIBSPK_PARAMETER:
        case MSG_A2M_SMARTPA_PARAMETER:
            A2MBufUnLock();
            break;
#ifndef USE_CCCI_SHARE_BUFFER
        case MSG_A2M_EM_DYNAMIC_SPH:
            A2MBufUnLock();
            break;
#endif
        }

        if (length_write == size_message) {
            LastSentMessage = message_id;
            if (MESSAGE_NEED_ACK == JudgeAckOfMsg(message_id)) {
                LastNeedAckSentMessage = message_id;
            }
            ret = NO_ERROR;
            break;
        } else {
            modem_status = GetModemCurrentStatus();
            if (errno == 3 || modem_status != MODEM_STATUS_READY) { // md reset cause ccci message fail
                ALOGE("%s(), MD RESET SKIP MESSAGE: 0x%x, try: #%d, write_length: %d, errno: %d, modem status: %d",
                      __FUNCTION__, ccci_buff.message,  i, write_length, errno, modem_status);
                // if modem reset during phone call, raise mIsModemResetDuringPhoneCall
                if (message_id != MSG_A2M_SPH_OFF && message_id != MSG_A2M_SET_ACOUSTIC_LOOPBACK) {
                    mIsModemResetDuringPhoneCall = true;
                }

                mIsModemReset = true;
                ResetSpeechParamAckCount();
                break;
            }
            ALOGW("%s(), message: 0x%x, try: #%d, write_length: %d, errno: %d, modem status: %d",
                  __FUNCTION__, ccci_buff.message, i, write_length, errno, modem_status);

            sph_msleep(2);
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
#define LOG_TAG "SpeechMessengerECCCI_Read"
status_t SpeechMessengerECCCI::ReadMessage(ccci_buff_t &ccci_buff) {
    uint32_t length_payload, modem_status, temp_msg1, temp_msg2;
    ALOGV("%s()", __FUNCTION__);

    /* check if already initialized */
    if (fHdl < 0) {
        if (Initial() != NO_ERROR) {
            return UNKNOWN_ERROR;
        }
    }

    /* read message */
    int length_read = read(fHdl, (void *)&ccci_buff, sizeof(ccci_buff_t));
    if (length_read == -1) {
        modem_status = GetModemCurrentStatus();
        ALOGE("%s() fail, read_length: %d, modem current status: %d, errno: %d",
              __FUNCTION__, length_read, modem_status, errno);
        return UNKNOWN_ERROR;
    } else {
        uint16_t message_id = GetMessageID(ccci_buff);
        if (ccci_buff.magic != CCCI_MAILBOX_MAGIC_NUMBER) {
            ccci_buff.message -= CCCI_BUF_HEADER_SIZE; // reduce header size
        }
#ifndef CONFIG_MT_ENG_BUILD// user and user debug load
        // do not print per frame message for message with data payload in user and user-debug load
        if (ccci_buff.magic == CCCI_MAILBOX_MAGIC_NUMBER && JudgeLogPrintOfMsg(message_id)) {
            ALOGD("%s(), length_read=%d, data[0](magic)=0x%x, data[1](message)=0x%x, ch=0x%x, reserved=0x%x",
                  __FUNCTION__, length_read, ccci_buff.magic, ccci_buff.message, ccci_buff.channel,
                  ccci_buff.reserved);
        }
#else // eng load
        ALOGD("%s(), length_read=%d, data[0](magic)=0x%x, data[1](message)=0x%x, ch=0x%x, reserved=0x%x",
              __FUNCTION__, length_read, ccci_buff.magic, ccci_buff.message, ccci_buff.channel, ccci_buff.reserved);
#endif
        if (message_id == MSG_M2A_EM_DATA_REQUEST) {
            mIsModemEPOF = false;
        }
        modem_status = GetModemCurrentStatus();
        if (modem_status != MODEM_STATUS_READY) {
            ALOGE("%s() fail, read_length: %d, modem current status: %d, errno: %d, mIsModemEPOF(%d)",
                  __FUNCTION__, length_read, modem_status, errno, mIsModemEPOF);
            return UNKNOWN_ERROR;
        }
    }
    return NO_ERROR;
}

#undef LOG_TAG
#define LOG_TAG "SpeechMessengerECCCI"

uint32_t SpeechMessengerECCCI::GetQueueCount() const {
    int32_t count = (iQWrite - iQRead);
    if (count < 0) { count += CCCI_MAX_QUEUE_NUM; }
    return count;
}

bool SpeechMessengerECCCI::CheckOffsetAndLength(const ccci_buff_t &ccci_buff) {
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


ccci_message_ack_t SpeechMessengerECCCI::JudgeAckOfMsg(const uint16_t message_id) {
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
    case MSG_A2M_EM_MAGICON:
    case MSG_A2M_EM_HAC:
    case MSG_A2M_EM_WB:
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
    return ack;
}

bool SpeechMessengerECCCI::JudgeLogPrintOfMsg(const uint16_t message_id) {
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

bool SpeechMessengerECCCI::IsModemFunctionOnOffMessage(const uint16_t message_id) {
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

status_t SpeechMessengerECCCI::SendMessageInQueue(ccci_buff_t ccci_buff) {
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
    ASSERT(count < (CCCI_MAX_QUEUE_NUM - 1));  // check queue full

    ccci_message_ack_t ack_type = JudgeAckOfMsg(GetMessageID(ccci_buff));

    if (ack_type == MESSAGE_NEED_ACK) {
        ALOGV("%s(), mModemIndex = %d, need ack message: 0x%x, reserved param: 0x%x",
              __FUNCTION__, mModemIndex, ccci_buff.message, ccci_buff.reserved);
    }
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

            ALOGD("%s(), Send message(0x%x) to queue, reserve param(0x%x), mModemIndex(%d),count(%u), LastSentMessage(0x%x), LastNeedAckSentMessage(0x%x)", __FUNCTION__, ccci_buff.message, ccci_buff.reserved, mModemIndex, GetQueueCount(), LastSentMessage, LastNeedAckSentMessage);
        }
    }
    AL_UNLOCK(mCCCIMessageQueueMutex);
    return ret;
}

status_t SpeechMessengerECCCI::ConsumeMessageInQueue() {
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

bool SpeechMessengerECCCI::MDReset_CheckMessageInQueue() {
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
#ifdef USE_CCCI_SHARE_BUFFER
    // init CCCI buffer again no matter what
    InitA2MRawParaRingBuffer();
#endif
    if (count != 0) {
        ALOGE("%s(), queue is not empty!!", __FUNCTION__);
        iQWrite = 0;
        iQRead = 0;
    }

    AL_UNLOCK(mCCCIMessageQueueMutex);
    return ret;
}

bool SpeechMessengerECCCI::GetMDResetFlag() {
    ALOGD("%s(), mIsModemReset=%d", __FUNCTION__, mIsModemReset);
    return mIsModemReset;
}

status_t SpeechMessengerECCCI::CreateReadingThread() {
    int ret = pthread_create(&hReadThread, NULL, SpeechMessengerECCCI::CCCIReadThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

void *SpeechMessengerECCCI::CCCIReadThread(void *arg) {
    pthread_detach(pthread_self());

    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);

    SpeechMessengerECCCI *pCCCI = (SpeechMessengerECCCI *)arg;
#if 1
    // Adjust thread priority
    int rtnPrio = setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    if (0 != rtnPrio) {
        ALOGE("[%s] failed, errno: %d, return=%d", __FUNCTION__, errno, rtnPrio);
    } else {
        ALOGD("%s setpriority ok, priority: %d", __FUNCTION__, ANDROID_PRIORITY_AUDIO);
    }
#else
    // force to set priority
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = 92;//RTPM_PRIO_AUDIO_CCCI_THREAD;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p)) {
        ALOGE("[%s] failed, errno: %d", __FUNCTION__, errno);
    } else {
        sched_p.sched_priority = 92;//RTPM_PRIO_AUDIO_CCCI_THREAD;
        sched_getparam(0, &sched_p);
        ALOGD("%s sched_setscheduler ok, priority: %d", __FUNCTION__, sched_p.sched_priority);
    }
#endif
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

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

            //pCCCI->mLad->Signal();

            ALOGD("--SpeechOn Ack done(0x%x)", ccci_buff.message);
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

            ALOGD("--SpeechOff Ack done(0x%x)", ccci_buff.message);
            pCCCI->mLad->Signal();

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
            ALOGV("%s() MSG_M2A_PCM_REC_DATA_NOTIFY", __FUNCTION__);
            ASSERT(pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                uint16_t Bytes_PCM = pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN;
                SLOGV("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x), data_length: %d", ccci_buff.message, Bytes_PCM);

                // Phone record
                AudioALSACaptureDataProviderVoice::getInstance()->provideModemRecordDataToProvider(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                if ((pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == true) && (mA2M_ECCCI_DATA_READ_ACK == true)) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PCM_REC_DATA_READ_ACK, 0, 0));
                    mA2M_ECCCI_DATA_READ_ACK = false;
                } else {
                    ALOGD("%s() RECORD_STATUS_MASK(%d)", __FUNCTION__, pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK));
                }
            }

            break;
        }
        case MSG_M2A_RAW_PCM_REC_DATA_NOTIFY: { // meaning that we are recording, modem have some data
            ALOGV("%s() MSG_M2A_RAW_PCM_REC_DATA_NOTIFY", __FUNCTION__);
            ASSERT(pCCCI->GetModemSideModemStatus(RAW_RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(RAW_RECORD_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                uint16_t Bytes_PCM = pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN;
                SLOGV("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x), data_length: %d", ccci_buff.message, Bytes_PCM);

                // Phone record

#ifndef TEST_NEW_RECOED_FOR_OLD_CAPTURE
                SpeechDataProcessingHandler::getInstance()->provideModemRecordDataToProvider(pCCCI->GetM2ARawPcmRingBuffer(ccci_buff));
#else
                AudioALSACaptureDataProviderVoice::getInstance()->provideModemRecordDataToProvider(pCCCI->GetM2ARawPcmRingBuffer(ccci_buff));
#endif

                if ((pCCCI->mLad->GetApSideModemStatus(RAW_RECORD_STATUS_MASK) == true) && (mA2M_ECCCI_DATA_READ_ACK == true)) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_RAW_PCM_REC_DATA_READ_ACK, 0, 0));
                    mA2M_ECCCI_DATA_READ_ACK = false;
                } else {
                    ALOGD("%s() RAW_RECORD_STATUS_MASK(%d)", __FUNCTION__, pCCCI->mLad->GetApSideModemStatus(RAW_RECORD_STATUS_MASK));
                }
            }

            break;
        }

        case MSG_M2A_VM_REC_DATA_NOTIFY: { // meaning that we are recording, modem have some data
            ASSERT(pCCCI->GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(VM_RECORD_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_VM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_VM_REC_DATA_NOTIFY(0x%x), data_length: %u", ccci_buff.message, (unsigned int)(pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN));

                SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
                // VM
                pSpeechVMRecorder->getVmDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                if ((pCCCI->mLad->GetApSideModemStatus(VM_RECORD_STATUS_MASK) == true) && (mA2M_ECCCI_DATA_READ_ACK == true)) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_VM_REC_DATA_READ_ACK, 0, 0));
                    mA2M_ECCCI_DATA_READ_ACK = false;
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
                ALOGD("MSG_M2A_CUST_DUMP_NOTIFY(0x%x), data_length: %d", ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN);

                pSpeechVoiceCustomLogger->CopyBufferToVCL(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                if (mA2M_ECCCI_DATA_READ_ACK == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_CUST_DUMP_READ_ACK, 0, 0));
                    mA2M_ECCCI_DATA_READ_ACK = false;
                }
            }
            break;
        }
#endif
        case MSG_M2A_PCM_REC_OFF_ACK: { // ack of MSG_A2M_PCM_REC_OFF
            if (pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(RECORD_STATUS_MASK);
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
                ALOGW("MSG_M2A_BGSND_DATA_REQUEST(0x%x) after AP side trun off BGS!! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_BGSND_DATA_REQUEST(0x%x), num_data_request: %d", ccci_buff.message, pCCCI->GetMessageParam(ccci_buff));

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
                const uint16_t message_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
                if (pCCCI->mLad->GetApSideModemStatus(BGS_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_BGSND_DATA_NOTIFY, message_length, offset));
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
                SLOGV("MSG_M2A_PNW_UL_DATA_NOTIFY(0x%x), data_length:%u", ccci_buff.message, (unsigned int)(pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN));
                pRecord2Way->GetDataFromMicrophone(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));
                if ((pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == true) && (mA2M_ECCCI_DATA_READ_ACK == true)) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PNW_UL_DATA_READ_ACK, 0, 0));
                    mA2M_ECCCI_DATA_READ_ACK = false;
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
                uint32_t max_buf_length = A2M_SHARED_BUFFER_P2W_DL_DATA_SIZE - CCCI_SHARE_BUFF_HEADER_LEN;
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
                uint16_t message_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
                if (pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PNW_DL_DATA_NOTIFY, message_length, offset));
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
                ALOGD_IF(pCCCI->mLogEnable, "+%s(), MSG_M2A_SPH_DL_DECRYPTION(0x%x), ByteDataSource=%d", __FUNCTION__, m2a_message_id, ByteDataSource);
                ALOGD_IF(pCCCI->mLogEnable, "+%s(), MSG_M2A_SPH_DL_DECRYPTION(0x%x), BufferSource[0] = 0x%x, BufferSource[1]=0x%x, BufferSource[2] = 0x%x, BufferSource[3]=0x%x", __FUNCTION__, m2a_message_id, *((uint16_t *)BufferSource), *((uint16_t *)BufferSource + 1), *((uint16_t *)BufferSource + 2), *((uint16_t *)BufferSource + 3));

                uint16_t ByteDataTarget = SpeechDataEncrypter::GetInstance()->Decryption(BufferDataTarget, BufferSource, ByteDataSource);

                // fill header info
                pCCCI->SetShareBufHeader((uint16_t *)BufferTarget,
                                         CCCI_A2M_SHARE_BUFF_HEADER_SYNC,
                                         SHARE_BUFF_DATA_TYPE_CCCI_DL_DEC,
                                         ByteDataTarget);

                ALOGD_IF(pCCCI->mLogEnable, "%s(), MSG_M2A_SPH_UL_ENCRYPTION(0x%x), BufferTarget = %p, ByteDataTarget = 0x%x", __FUNCTION__, m2a_message_id, BufferTarget, ByteDataTarget);
                ALOGD_IF(pCCCI->mLogEnable, "%s(), MSG_M2A_SPH_UL_ENCRYPTION(0x%x), BufferDataTarget[0] = 0x%x, BufferDataTarget[1]=0x%x, BufferDataTarget[2] = 0x%x, BufferDataTarget[3]=0x%x", __FUNCTION__, m2a_message_id, *((uint16_t *)BufferDataTarget), *((uint16_t *)BufferDataTarget + 1), *((uint16_t *)BufferDataTarget + 2), *((uint16_t *)BufferDataTarget + 3));
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
                SLOGV("MSG_M2A_CTM_DEBUG_DATA_NOTIFY(0x%x), data_length: %u", ccci_buff.message, (unsigned int)(pCCCI->GetMessageLength(ccci_buff) - CCCI_PAYLOAD_BUFF_HEADER_LEN));
                SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
                const int data_type = pCCCI->GetM2AShareBufDataType(ccci_buff);
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

                if (mA2M_ECCCI_DATA_READ_ACK == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_CTM_DEBUG_DATA_READ_ACK, 0, 0));
                    mA2M_ECCCI_DATA_READ_ACK = false;
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
            ALOGD("--TtyCtmOff Ack done(0x%x)", ccci_buff.message);
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
            //                pCCCI->A2MBufUnLock();
            ALOGV("--SetNBSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_DMNR_ACK: { // ack of MSG_A2M_EM_DMNR
            pCCCI->AddSpeechParamAckCount(DMNR_SPEECH_PARAM);
            //                pCCCI->A2MBufUnLock();
            ALOGV("--SetDualMicSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_MAGICON_ACK: { // ack of MSG_A2M_EM_MAGICON
            pCCCI->AddSpeechParamAckCount(MAGICON_SPEECH_PARAM);
            //                pCCCI->A2MBufUnLock();
            ALOGV("--SetMagiConSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_HAC_ACK: { // ack of MSG_A2M_EM_HAC
            pCCCI->AddSpeechParamAckCount(HAC_SPEECH_PARAM);
            //                pCCCI->A2MBufUnLock();
            ALOGV("--SetHACSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_WB_ACK: { // ack of MSG_A2M_EM_WB
            pCCCI->AddSpeechParamAckCount(WB_SPEECH_PARAM);
            //                pCCCI->A2MBufUnLock();
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
            ALOGW("..[MD Reset Notify(MSG_M2A_EM_DATA_REQUEST: 0x%x)]..", ccci_buff.message);
            pCCCI->mWasModemReset = true;
            // Clean mWaitAckMessageID to avoid the queue receiving the wrong ack
            pCCCI->mWaitAckMessageID = 0;

            // close analog downlink path when modem reset during phone call
            if (pCCCI->mLad->GetApSideModemStatus(SPEECH_STATUS_MASK) == true ||
                pCCCI->mLad->GetApSideModemStatus(VT_STATUS_MASK)     == true) {
                //notify event to do reopen voice call
                AudioEventThreadManager::getInstance()->notifyCallback(AUDIO_EVENT_PHONECALL_REOPEN, pCCCI);
            }
            pCCCI->mIsModemResetDuringPhoneCall = false;
            pCCCI->mIsModemReset = false;
            pCCCI->mIsModemEPOF = false;
            // Check the first message in queue. If need ack, take action.
            // Modem reset, flush all CCCI queue first. Don't care for the CCCI queue.
            pCCCI->ResetSpeechParamAckCount();
            pCCCI->MDReset_CheckMessageInQueue();

            break;
        }
        case MSG_M2A_VIBSPK_PARAMETER_ACK: { // ack of MSG_M2A_VIBSPK_PARAMETER
            //                pCCCI->A2MBufUnLock();
            ALOGV("--SetVibSpkParam Ack done(0x%x)", ccci_buff.message);
            break;
        }

        case MSG_M2A_SMARTPA_PARAMETER_ACK: {
            //                pCCCI->A2MBufUnLock();
            ALOGV("--SetSmartpaParam Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_NETWORK_STATUS_NOTIFY: {
            uint16_t mRfMode = ccci_buff.message & 0xFFFF;

            // save mRfMode in property to avoid mediaserver die
            pCCCI->SetRFInfo(1, mRfMode);
            pCCCI->GetRFInfo();

#ifdef MTK_AUDIO_GAIN_TABLE
            // speech network type change
            //bit 0~3: Network; bit 4,5: Voice Band, bit 15: 0 no network support, 1: network support
            AudioVolumeFactory::CreateAudioVolumeController()->speechNetworkChange(ccci_buff.message);
#endif
            ALOGV("--MSG_M2A_NETWORK_STATUS_NOTIFY (0x%x)", ccci_buff.message);
            pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_NETWORK_STATUS_ACK, 0, 0));
            break;
        }
        /* Modem EPOF */
        case MSG_M2A_EPOF_NOTIFY: { // Modem reset. Requset for all EM data(NB/DMNR/WB)
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
            //ALOGD("Read modem message(0x%x), don't care. (or no such message)", ccci_buff.message);
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
                ALOGW("Message(0x%x) might double ack!! The current mWaitAckMessageID is 0x%x", ccci_buff.message, pCCCI->mWaitAckMessageID);
            }
        }
    }
    pthread_exit(NULL);
    return 0;
}

status_t SpeechMessengerECCCI::CreateSendSphParaThread() {
    int ret = pthread_create(&hSendSphThread, NULL, SpeechMessengerECCCI::SendSphParaThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

void *SpeechMessengerECCCI::SendSphParaThread(void *arg) {
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

    SpeechMessengerECCCI *pCCCI = (SpeechMessengerECCCI *)arg;

    // Check the first message in queue. If need ack, take action.
    // Modem reset, flush all CCCI queue first. Don't care for the CCCI queue.
    pCCCI->MDReset_CheckMessageInQueue();

    // Get SpeechParamLock
    if (pCCCI->SpeechParamLock() == false) {
        ALOGE("%s() fail to get SpeechParamLock!!", __FUNCTION__);
        return 0;
    }

    // Send speech parameters to modem side
    pCCCI->ResetSpeechParamAckCount();
    pCCCI->mLad->SetAllSpeechEnhancementInfoToModem();

    // Release SpeechParamLock
    pCCCI->SpeechParamUnLock();

    pthread_exit(NULL);
    return 0;
}

bool SpeechMessengerECCCI::A2MBufLock() {
    const uint32_t kA2MBufLockTimeout = 3000; // 3 sec

    int rc = AL_LOCK_MS(mA2MShareBufMutex, kA2MBufLockTimeout);
    ALOGD("%s()", __FUNCTION__);
    if (rc != 0) {
        ALOGE("%s(), Cannot get Lock!! Timeout : %u msec, rc %d", __FUNCTION__, kA2MBufLockTimeout, rc);
        return false;
    }
    return true;
}

void SpeechMessengerECCCI::A2MBufUnLock() {
    AL_UNLOCK(mA2MShareBufMutex);
    ALOGD("%s()", __FUNCTION__);
}

bool SpeechMessengerECCCI::SpeechParamLock() {
    const uint32_t kSphParamLockTimeout = 10000; // 10 sec

    ALOGD("%s()", __FUNCTION__);
    int rc = AL_LOCK_MS(mSetSpeechParamMutex, kSphParamLockTimeout);
    if (rc != 0) {
        ALOGE("%s(), Cannot get Lock!! Timeout : %u msec", __FUNCTION__, kSphParamLockTimeout);
        return false;
    }
    return true;
}

void SpeechMessengerECCCI::SpeechParamUnLock() {
    ALOGD("%s()", __FUNCTION__);
    AL_UNLOCK(mSetSpeechParamMutex);
}

void SpeechMessengerECCCI::ResetSpeechParamAckCount() {
    memset(&mSpeechParamAckCount, 0, sizeof(mSpeechParamAckCount));
    ALOGD("%s(), NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)/DynamicSPH(%u)", __FUNCTION__,
          mSpeechParamAckCount[NB_SPEECH_PARAM],
          mSpeechParamAckCount[DMNR_SPEECH_PARAM],
          mSpeechParamAckCount[WB_SPEECH_PARAM],
          mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
          mSpeechParamAckCount[HAC_SPEECH_PARAM],
          mSpeechParamAckCount[DYNAMIC_SPH_PARAM]);
}

void SpeechMessengerECCCI::AddSpeechParamAckCount(speech_param_ack_t type) {
    if (type >= NUM_SPEECH_PARAM_ACK_TYPE || type < 0) {
        ALOGE("%s(), no such type: %d", __FUNCTION__, type);
    } else {
        if (mSpeechParamAckCount[type] < 0xFFFFFFFF) { //prevent overflow
            mSpeechParamAckCount[type]++;
        }
        ALOGD("%s(%d), NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)/DynamicSPH(%u)", __FUNCTION__, type,
              mSpeechParamAckCount[NB_SPEECH_PARAM],
              mSpeechParamAckCount[DMNR_SPEECH_PARAM],
              mSpeechParamAckCount[WB_SPEECH_PARAM],
              mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
              mSpeechParamAckCount[HAC_SPEECH_PARAM],
              mSpeechParamAckCount[DYNAMIC_SPH_PARAM]);
    }
}

bool SpeechMessengerECCCI::CheckSpeechParamAckAllArrival() {
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
void SpeechMessengerECCCI::SendMsgFailErrorHandling(const ccci_buff_t &ccci_buff) {
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
                ALOGE("--SpeechOn(PCM Route On) Ack is not paired!!");
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
        //InitA2MRawParaRingBuffer();
        break;
    }
#endif
    default: {
        ALOGW("%s(), message: 0x%x, ack don't care", __FUNCTION__, ccci_buff.message);
    }
    }
}

uint16_t SpeechMessengerECCCI::GetPcmFreq(const uint16_t Idx_Freq) {
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

RingBuf SpeechMessengerECCCI::GetM2AUplinkRingBuffer(const ccci_buff_t &ccci_buff) {
    RingBuf ul_ring_buf;
    uint16_t size_copy1, size_copy2;
    ul_ring_buf.bufLen   = mM2AShareBufLen;
    ul_ring_buf.pBufBase = mM2AShareBufBase;
    ul_ring_buf.pBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;

    ul_ring_buf.pRead = mM2AShareBufRead;

    ul_ring_buf.pWrite   = ul_ring_buf.pRead + GetMessageLength(ccci_buff);
    if (ul_ring_buf.pWrite >= mM2AShareBufEnd) {
        ul_ring_buf.pWrite -= ul_ring_buf.bufLen;

        size_copy1 = mM2AShareBufEnd - ul_ring_buf.pRead;
        size_copy2 = GetMessageLength(ccci_buff) - size_copy1;
        ALOGV("%s(), mM2AShareBufEnd(%p), payloadLen(0x%x), size_copy1(0x%x), size_copy2(0x%x), pRead(%p), pWrite(%p)",
              __FUNCTION__, mM2AShareBufEnd, GetMessageLength(ccci_buff), size_copy1, size_copy2, ul_ring_buf.pRead, ul_ring_buf.pWrite);
        memcpy(ul_ring_buf.pRead, (char *)ccci_buff.payload, size_copy1);
        memcpy(ul_ring_buf.pBufBase, (char *)ccci_buff.payload + size_copy1, size_copy2);
    } else {
        memcpy(ul_ring_buf.pRead, (char *)ccci_buff.payload, GetMessageLength(ccci_buff));
    }
    ALOGV("%s(), pBufBase(%p), bufLen(0x%x), payloadLen(0x%x), mM2AShareBufRead(%p), pRead(%p), pWrite(%p)",
          __FUNCTION__, ul_ring_buf.pBufBase, ul_ring_buf.bufLen, GetMessageLength(ccci_buff), mM2AShareBufRead, ul_ring_buf.pRead, ul_ring_buf.pWrite);

    // share buffer header
    char *p_sync_word = (char *)ccci_buff.payload + 0; // size(unsigned short)
    char *p_data_type = (char *)ccci_buff.payload + 2; // size(unsigned short)
    char *p_data_len  = (char *)ccci_buff.payload + 4; // size(unsigned short)
    char *p_data_curIdx = (char *)ccci_buff.payload + 6; // size(unsigned short)
    char *p_data_totalIdx  = (char *)ccci_buff.payload + 8; // size(unsigned short)

    if (*(uint16_t *)p_data_curIdx  == *(uint16_t *)p_data_totalIdx) {  mA2M_ECCCI_DATA_READ_ACK = true; }

#ifdef CONFIG_MT_ENG_BUILD
    ALOGD("%s(), *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x), *p_data_curIdx(0x%x), *p_data_totalIdx(0x%x), mA2M_ECCCI_DATA_READ_ACK(%d)",
          __FUNCTION__, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len, *(uint16_t *)p_data_curIdx, *(uint16_t *)p_data_totalIdx, mA2M_ECCCI_DATA_READ_ACK);
#else
    ALOGV("%s(), *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x), *p_data_curIdx(0x%x), *p_data_totalIdx(0x%x), mA2M_ECCCI_DATA_READ_ACK(%d)",
          __FUNCTION__, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len, *(uint16_t *)p_data_curIdx, *(uint16_t *)p_data_totalIdx, mA2M_ECCCI_DATA_READ_ACK);
#endif

    SLOGV("CCCI get uplink data from modem. message = 0x%x, sync = 0x%x, type = 0x%x, data_len = %d",
          ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len);

    ASSERT(*(uint16_t *)p_sync_word == EEMCS_M2A_SHARE_BUFF_HEADER_SYNC);

    ul_ring_buf.pRead += CCCI_PAYLOAD_BUFF_HEADER_LEN;
    if (ul_ring_buf.pRead >= mM2AShareBufEnd) { ul_ring_buf.pRead -= ul_ring_buf.bufLen; }

    mM2AShareBufRead = ul_ring_buf.pWrite;

    return ul_ring_buf;
}
#ifdef USE_CCCI_SHARE_BUFFER
void SpeechMessengerECCCI::InitA2MRawParaRingBuffer() {
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

// return the offset of ring buffer for driver to write parameter
// get offset and avail length
// avail and offset's range in 20K , 16 bit should cover it.
status_t SpeechMessengerECCCI::GetA2MRawParaRingBuffer(uint16_t *offset, uint16_t *avail) {
    *offset  = (uint16_t)(mA2MParaShareBuf.pWrite - mA2MParaShareBufBase);
    if (mA2MParaShareBuf.pWrite > mA2MParaShareBuf.pRead) {
        * avail = (uint16_t)(mA2MParaShareBuf.bufLen - (mA2MParaShareBuf.pWrite - mA2MParaShareBuf.pRead));
    } else if (mA2MParaShareBuf.pWrite == mA2MParaShareBuf.pRead) {
        * avail = (uint16_t)(mIsA2MParaShareBufEmpty ?  mA2MParaShareBuf.bufLen : 0);
    } else {
        * avail = (uint16_t)(mA2MParaShareBuf.pRead - mA2MParaShareBuf.pWrite);
    }
#ifdef CONFIG_MT_ENG_BUILD
    ALOGD("%s  mA2MParaShareBuf.pWrite %p ,*offset %d, *avail %d", __FUNCTION__,  mA2MParaShareBuf.pWrite, *offset, *avail);
#else
    ALOGV("%s  mA2MParaShareBuf.pWrite %p ,*offset %d, *avail %d", __FUNCTION__,  mA2MParaShareBuf.pWrite, *offset, *avail);
#endif
    return NO_ERROR;
}

// renew  share buffer pointer when receive ack
// advance read pointer
status_t SpeechMessengerECCCI::AdvanceA2MRawParaRingBuffer(int datalength) {
    ASSERT(datalength < mA2MParaShareBuf.bufLen);
    ALOGV("%s, %d", __FUNCTION__, datalength);
    mA2MParaShareBuf.pRead += datalength;
    if (mA2MParaShareBuf.pRead >= mA2MParaShareBuf.pBufEnd) {
        mA2MParaShareBuf.pRead -= mA2MParaShareBuf.bufLen;
    }
    if (mA2MParaShareBuf.pRead == mA2MParaShareBuf.pWrite) {
        mIsA2MParaShareBufEmpty = true;
    }

#ifdef CONFIG_MT_ENG_BUILD
    ALOGD("%s, mA2MParaShareBuf.pRead = %p", __FUNCTION__, mA2MParaShareBuf.pRead);
#else
    ALOGV("%s, mA2MParaShareBuf.pRead = %p", __FUNCTION__, mA2MParaShareBuf.pRead);
#endif
    return NO_ERROR;
}


// user must make sure that data length < available length
status_t SpeechMessengerECCCI::WriteA2MRawParaRingBuffer(char *data, int datalength) {
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

RingBuf SpeechMessengerECCCI::GetM2ARawPcmRingBuffer(const ccci_buff_t &ccci_buff) {
    spcRAWPCMBufInfo header_RawPcmBufInfo;
    spcApRAWPCMBufHdr header_ApRawPcmBuf;

    char SeqPcmBuf[CCCI_MAX_PAYLOAD_SIZE * 4 + 16] = {0};
    RingBuf ul_ring_buf;
    uint16_t size_copy1, size_copy2, BytesCopied, BytesToCopy;
    char *PtrTarget = NULL;
    char *PtrSource = NULL;
    //parse message
    char *p_sync_word = (char *)ccci_buff.payload + 0; // size(unsigned short)
    char *p_data_type = (char *)ccci_buff.payload + 2; // size(unsigned short)
    char *p_data_len  = (char *)ccci_buff.payload + 4; // size(unsigned short)
    char *p_data_curIdx = (char *)ccci_buff.payload + 6; // size(unsigned short)
    char *p_data_totalIdx  = (char *)ccci_buff.payload + 8; // size(unsigned short)

    if (*(uint16_t *)p_data_curIdx  == *(uint16_t *)p_data_totalIdx) {  mA2M_ECCCI_DATA_READ_ACK = true; }
    ALOGD("+%s(), message(0x%x) ,sync(0x%x), type(0x%x), len(0x%x)",
          __FUNCTION__, ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len);

    ALOGV("CCCI get uplink data from modem. message = 0x%x, sync = 0x%x, type = 0x%x, data_len = %d",
          ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len);

    ASSERT(*(uint16_t *)p_sync_word == EEMCS_M2A_SHARE_BUFF_HEADER_SYNC);


    // share buffer header
    memcpy(&header_RawPcmBufInfo, (char *)ccci_buff.payload + CCCI_PAYLOAD_BUFF_HEADER_LEN, sizeof(spcRAWPCMBufInfo));
    PtrTarget = (char *)SeqPcmBuf;

    switch (mPcmRecordType.direction) {
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
        PtrSource = (char *)ccci_buff.payload + CCCI_PAYLOAD_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN;
        BytesToCopy = header_RawPcmBufInfo.u16ULLength;
        memcpy(PtrTarget, PtrSource, BytesToCopy);
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
        PtrSource = (char *)ccci_buff.payload + CCCI_PAYLOAD_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN + header_RawPcmBufInfo.u16ULLength;
        BytesToCopy = header_RawPcmBufInfo.u16DLLength;
        memcpy(PtrTarget, PtrSource, BytesToCopy);
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
        PtrSource = (char *)ccci_buff.payload + CCCI_PAYLOAD_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN;
        BytesToCopy = header_RawPcmBufInfo.u16ULLength;
        memcpy(PtrTarget, PtrSource, BytesToCopy);
        BytesCopied += BytesToCopy;
        PtrTarget = SeqPcmBuf + BytesCopied;


        //downlink raw pcm header
        header_ApRawPcmBuf.u16RawPcmDir = RECORD_TYPE_DL;
        header_ApRawPcmBuf.u16Freq = GetPcmFreq(header_RawPcmBufInfo.u16DLFreq);
        header_ApRawPcmBuf.u16Length = header_RawPcmBufInfo.u16DLLength;
        memcpy(PtrTarget, &header_ApRawPcmBuf, sizeof(header_ApRawPcmBuf));
        BytesCopied += sizeof(header_ApRawPcmBuf);

        //downlink raw pcm
        PtrTarget = SeqPcmBuf + BytesCopied;
        PtrSource = (char *)ccci_buff.payload + CCCI_PAYLOAD_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN + header_RawPcmBufInfo.u16ULLength;
        BytesToCopy = header_RawPcmBufInfo.u16DLLength;
        memcpy(PtrTarget, PtrSource, BytesToCopy);
        BytesCopied += BytesToCopy;
        break;
    default:
	ALOGV("%s(), not match any case BytesCopied = 0", __FUNCTION__);
	BytesCopied = 0;
	break;
    }

    //ring buffer process
    ul_ring_buf.bufLen   = mM2AShareBufLen;
    ul_ring_buf.pBufBase = mM2AShareBufBase;
    ul_ring_buf.pBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;
    ul_ring_buf.pRead = mM2AShareBufRead;

    ul_ring_buf.pWrite   = ul_ring_buf.pRead + BytesCopied;
    if (ul_ring_buf.pWrite >= mM2AShareBufEnd) {
        ul_ring_buf.pWrite -= ul_ring_buf.bufLen;

        size_copy1 = mM2AShareBufEnd - ul_ring_buf.pRead;
        size_copy2 = BytesCopied - size_copy1;
        ALOGV("%s(), mM2AShareBufEnd(%p), BytesCopied(0x%x), size_copy1(0x%x), size_copy2(0x%x), pRead(%p), pWrite(%p)",
              __FUNCTION__, mM2AShareBufEnd, BytesCopied, size_copy1, size_copy2, ul_ring_buf.pRead, ul_ring_buf.pWrite);
        memcpy(ul_ring_buf.pRead, (char *)SeqPcmBuf, size_copy1);
        memcpy(ul_ring_buf.pBufBase, (char *)SeqPcmBuf + size_copy1, size_copy2);
    } else {
        memcpy(ul_ring_buf.pRead, (char *)SeqPcmBuf, BytesCopied);
    }
    ALOGV("-%s(), pBufBase(%p), bufLen(0x%x), BytesCopied(0x%x), mM2AShareBufRead(%p), pRead(%p), pWrite(%p)",
          __FUNCTION__, ul_ring_buf.pBufBase, ul_ring_buf.bufLen, BytesCopied, mM2AShareBufRead, ul_ring_buf.pRead, ul_ring_buf.pWrite);

    mM2AShareBufRead = ul_ring_buf.pWrite;

    return ul_ring_buf;
}

uint16_t SpeechMessengerECCCI::GetM2AUplinkData(const ccci_buff_t &ccci_buff, char *TargetBuf) {
    uint16_t BytePayload = GetMessageLength(ccci_buff);
    uint16_t ByteData;


    // share buffer header
    char *p_sync_word = (char *)ccci_buff.payload + 0; // size(unsigned short)
    char *p_data_type = (char *)ccci_buff.payload + 2; // size(unsigned short)
    char *p_data_len  = (char *)ccci_buff.payload + 4; // size(unsigned short)
    char *p_data_curIdx = (char *)ccci_buff.payload + 6; // size(unsigned short)
    char *p_data_totalIdx  = (char *)ccci_buff.payload + 8; // size(unsigned short)
    ByteData = *(uint16_t *)p_data_len;

    if (*(uint16_t *)p_data_curIdx  == *(uint16_t *)p_data_totalIdx) {  mA2M_ECCCI_DATA_READ_ACK = true; }

    if (*(uint16_t *)p_sync_word != EEMCS_M2A_SHARE_BUFF_HEADER_SYNC) {
        char idxDump = 10, IdxDump;
        char SphDumpStr[SPH_DUMP_STR_SIZE] = {0};

        snprintf(SphDumpStr, SPH_DUMP_STR_SIZE, "payload[pBase 0x%p, msg 0x%x]= ", ccci_buff.payload, ccci_buff.message);
        for (IdxDump = 0; IdxDump < idxDump; IdxDump++) {
            char SphDumpTemp[100];
            snprintf(SphDumpTemp, 100, "[%d, 0x%p]0x%x, ", IdxDump, ccci_buff.payload + IdxDump, *((uint16_t *)ccci_buff.payload + IdxDump));
            audio_strncat(SphDumpStr, SphDumpTemp, SPH_DUMP_STR_SIZE);
        }
        if (IdxDump != 0) {
            ALOGD("%s(), %s", __FUNCTION__, SphDumpStr);
        }
        ALOGD("%s(), message = 0x%x, *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(%d)",
              __FUNCTION__, ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len);
    }
    ASSERT(*(uint16_t *)p_sync_word == EEMCS_M2A_SHARE_BUFF_HEADER_SYNC);
    //reset the data ptr of buffer
    memcpy(TargetBuf, (char *)ccci_buff.payload + CCCI_PAYLOAD_BUFF_HEADER_LEN, ByteData);
    ALOGV("-%s(), TargetBuf(%p), ByteData(0x%x)", __FUNCTION__, TargetBuf, ByteData);

    return ByteData;
}


uint16_t SpeechMessengerECCCI::GetM2AShareBufSyncWord(const ccci_buff_t &ccci_buff) {
    char *p_sync_word = (char *)ccci_buff.payload + 0; // 0 * size(unsigned short)
    SLOGV("%s(), sync = 0x%x", __FUNCTION__, *(uint16_t *)p_sync_word);
    return *(uint16_t *)p_sync_word;
}

uint16_t SpeechMessengerECCCI::GetM2AShareBufDataType(const ccci_buff_t &ccci_buff) {
    char *p_data_type = (char *)ccci_buff.payload + 2; // 1 * size(unsigned short)
    SLOGV("%s(), type = 0x%x", __FUNCTION__, *(uint16_t *)p_data_type);
    return *(uint16_t *)p_data_type;
}

uint16_t SpeechMessengerECCCI::GetM2AShareBufDataLength(const ccci_buff_t &ccci_buff) {
    char *p_data_len  = (char *)ccci_buff.payload + 4; // 2 * size(unsigned short)
    SLOGV("%s(), data_len = %d", __FUNCTION__, *(uint16_t *)p_data_len);
    return *(uint16_t *)p_data_len;
}


bool SpeechMessengerECCCI::GetModemSideModemStatus(const modem_status_mask_t modem_status_mask) const {
    return ((mModemSideModemStatus & modem_status_mask) > 0);
}

void SpeechMessengerECCCI::SetModemSideModemStatus(const modem_status_mask_t modem_status_mask) {
    mModemSideModemStatus |= modem_status_mask;

    // save mModemSideModemStatus in kernel to avoid medieserver die
    set_uint32_to_mixctrl(kPropertyKeyModemStatus, mModemSideModemStatus);
}

void SpeechMessengerECCCI::ResetModemSideModemStatus(const modem_status_mask_t modem_status_mask) {
    mModemSideModemStatus &= (~modem_status_mask);

    // save mModemSideModemStatus in kernel to avoid medieserver die
    set_uint32_to_mixctrl(kPropertyKeyModemStatus, mModemSideModemStatus);
}

void SpeechMessengerECCCI::GetRFInfo(void) {
    uint16_t mRfMode = 0, mRfInfo = 0;
    char property_value[PROPERTY_VALUE_MAX], property_value2[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_RF_INFO[0], property_value, "0");  //"0": default all off
    mRfInfo = atoi(property_value);

    property_get(PROPERTY_KEY_RF_INFO[1], property_value2, "0");  //"0": default all off
    mRfMode = atoi(property_value2);

    ALOGD("%s(), mRfInfo(0x%x), mRfMode(%d)", __FUNCTION__, mRfInfo, mRfMode);
}

void SpeechMessengerECCCI::SetRFInfo(char mRFIdx, uint16_t mRfData) {
    // save mRfMode, mRfInfo in property to avoid medieserver die
    char property_value[PROPERTY_VALUE_MAX];
    sprintf(property_value, "%u", mRfData);
    property_set(PROPERTY_KEY_RF_INFO[(unsigned char)mRFIdx], property_value);
}

void SpeechMessengerECCCI::ResetRFInfo(void) {
    uint16_t mRfMode = 0, mRfInfo = 0;

    // save mRfMode, mRfInfo in property to avoid medieserver die
    char property_value[PROPERTY_VALUE_MAX], property_value2[PROPERTY_VALUE_MAX];
    sprintf(property_value, "%u", mRfMode);
    property_set(PROPERTY_KEY_RF_INFO[0], property_value);

    sprintf(property_value, "%u", mRfInfo);
    property_set(PROPERTY_KEY_RF_INFO[1], property_value);
}

status_t SpeechMessengerECCCI::setPcmRecordType(SpcRecordTypeStruct typeRecord) {
    mPcmRecordType = typeRecord;
    ALOGD("%s(), mPcmRecordType(%d)", __FUNCTION__, mPcmRecordType.direction);
    return NO_ERROR;
}

status_t SpeechMessengerECCCI::SetNWCodecInfo(const ccci_buff_t &ccci_buff) {
    spcCodecInfoStruct sCodecInfo;
    int returnPinfo = 0, returnHStatus = 0;
    //parse message
    char *p_sync_word = (char *)ccci_buff.payload + 0; // size(unsigned short)
    char *p_data_type = (char *)ccci_buff.payload + 2; // size(unsigned short)
    char *p_data_len  = (char *)ccci_buff.payload + 4; // size(unsigned short)
    char *p_data_curIdx = (char *)ccci_buff.payload + 6; // size(unsigned short)
    char *p_data_totalIdx  = (char *)ccci_buff.payload + 8; // size(unsigned short)
    ALOGV("%s(), *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x), *p_data_curIdx(0x%x), *p_data_totalIdx(0x%x)",
          __FUNCTION__, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type, *(uint16_t *)p_data_len, *(uint16_t *)p_data_curIdx, *(uint16_t *)p_data_totalIdx);

    ASSERT(*(uint16_t *)p_sync_word == EEMCS_M2A_SHARE_BUFF_HEADER_SYNC);

    // share buffer header
    memcpy(&sCodecInfo, (char *)ccci_buff.payload + CCCI_PAYLOAD_BUFF_HEADER_LEN, sizeof(spcCodecInfoStruct));

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

} // end of namespace android
