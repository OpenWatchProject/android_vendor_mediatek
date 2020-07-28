#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechMessengerCCCI"
#include "SpeechMessengerCCCI.h"
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

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
#include "hardware/ccci_intf.h"
#include <AudioEventThreadManager.h>
#include <SpeechUtility.h>

#ifndef sph_msleep
#define sph_msleep(ms) usleep((ms)*1000)
#endif

namespace android {

/** CCCI driver & ioctl */
/** CCCI ioctl */
extern "C" {
#include "hardware/ccci_intf.h"
}

/** Property keys*/
static const char *kPropertyKeyModemStatus = "vendor.audiohal.modem_1.status";
static const char* PROPERTY_KEY_RF_INFO[2] = {
    "vendor.audiohal.rf_info", "vendor.audiohal.rf_mode"
};


/** CCCI channel No */
static const uint8_t    CCCI_M2A_CHANNEL = 4;
static const uint8_t    CCCI_A2M_CHANNEL = 5;

/** CCCI magic number */
static const uint32_t   CCCI_MAILBOX_MAGIC_NUMBER = 0xFFFFFFFF;
static const uint32_t ap_working_buf_length = 4096;

#define SPH_DUMP_STR_SIZE (500)


SpeechMessengerCCCI::SpeechMessengerCCCI(modem_index_t modem_index, SpeechDriverLAD *pLad)
    : mModemIndex(modem_index), mLad(pLad) {
    ALOGD("%s()", __FUNCTION__);
    CCCIEnable = false;

    fHdlRead = -1;
    fHdlWrite = -1;

    mA2MShareBufLen = 0;
    mM2AShareBufLen = 0;

    mA2MShareBufBase = NULL;
    mM2AShareBufBase = NULL;

    mA2MShareBufEnd = NULL;
    mM2AShareBufEnd = NULL;
    mWasModemReset = false;

    //initial the message queue
    memset((void *)pQueue, 0, sizeof(pQueue));
    iQRead = 0;
    iQWrite = 0;

    mWaitAckMessageID = 0;
    LastSentMessage = 0;
    LastNeedAckSentMessage = 0;

    //initial modem side modem status
    mModemSideModemStatus = get_uint32_from_mixctrl(kPropertyKeyModemStatus);
    ALOGD("%s(), mModemIndex(%d), mModemSideModemStatus(0x%x)", __FUNCTION__, mModemIndex, mModemSideModemStatus);

    ResetSpeechParamAckCount();

    mIsModemResetDuringPhoneCall = false;
    mIsModemReset = false;
    mIsModemEPOF = false;
}

status_t SpeechMessengerCCCI::Initial() {
    // Open CCCI and get handler
    char dev_node_read[32], dev_node_write[32];
    switch (mModemIndex) {
    case MODEM_1:
        snprintf(dev_node_read, 32, "%s", ccci_get_node_name(USR_AUDIO_RX, MD_SYS1));
        snprintf(dev_node_write, 32, "%s", ccci_get_node_name(USR_AUDIO_TX, MD_SYS1));
        break;
    default:
        snprintf(dev_node_read, 32, "%s", ccci_get_node_name(USR_AUDIO_RX, MD_SYS1));
        snprintf(dev_node_write, 32, "%s", ccci_get_node_name(USR_AUDIO_TX, MD_SYS1));
        break;
    }
    ALOGD("%s(), dev_node_read=%s, dev_node_write=%s", __FUNCTION__, dev_node_read, dev_node_write);
    fHdlRead = open(dev_node_read, O_RDWR);
    fHdlWrite = open(dev_node_write, O_RDWR);

    if (fHdlRead < 0 || fHdlWrite < 0) {
        ALOGE("%s(), fail to open %s or %s", __FUNCTION__, dev_node_read, dev_node_write);
        return UNKNOWN_ERROR;
    }

    // Get total share buffer length & base address
    uint32_t share_buf_length;
    char    *share_buf_address;

    ::ioctl(fHdlRead, CCCI_IOC_PCM_LEN, &share_buf_length);
    ALOGD("%s(), ioctl share_buf_length=%d", __FUNCTION__, share_buf_length);

    share_buf_address = (char *)mmap(NULL, share_buf_length, PROT_READ | PROT_WRITE, MAP_SHARED, fHdlRead, 0);
    ALOGD("%s(), ShareBufAdd:0x%p, ShareBufLen:%u", __FUNCTION__, share_buf_address, share_buf_length);

    mA2MShareBufLen = share_buf_length >> 1; // a2m buffer lengh should be half of share_buf_length
    mM2AShareBufLen = share_buf_length >> 1; // m2a buffer lengh should be half of share_buf_length

    mA2MShareBufBase = share_buf_address;
    mM2AShareBufBase = share_buf_address + mA2MShareBufLen;

    mA2MShareBufEnd = mA2MShareBufBase + mA2MShareBufLen;
    mM2AShareBufEnd = mM2AShareBufBase + mM2AShareBufLen;

    mSphCCCITmpBuf = new char[ap_working_buf_length];
    mSphPCMBuf = new char[ap_working_buf_length];

    mSphPCMBufEnd = mSphPCMBuf + ap_working_buf_length;

    pcm_ring_buf.bufLen = ap_working_buf_length;
    pcm_ring_buf.pBufBase = mSphPCMBuf;
    pcm_ring_buf.pRead  = pcm_ring_buf.pBufBase;
    pcm_ring_buf.pWrite = pcm_ring_buf.pRead;

    /* create the CCCI event reading thread */
    CCCIEnable = true;
    return CreateReadingThread();
}

status_t SpeechMessengerCCCI::Deinitial() {
    CCCIEnable = false;

    mA2MShareBufLen = 0;
    mM2AShareBufLen = 0;

    mA2MShareBufBase = NULL;
    mM2AShareBufBase = NULL;

    mA2MShareBufEnd = NULL;
    mM2AShareBufEnd = NULL;
    mSphPCMBufEnd = NULL;
    if (mSphCCCITmpBuf != NULL) {
        delete mSphCCCITmpBuf;
        mSphCCCITmpBuf = NULL;
    }

    if (mSphPCMBuf != NULL) {
        delete mSphPCMBuf;
        mSphPCMBuf = NULL;
    }
    if (fHdlRead >= 0) { close(fHdlRead); }
    if (fHdlWrite >= 0) { close(fHdlWrite); }

    return NO_ERROR;
}

SpeechMessengerCCCI::~SpeechMessengerCCCI() {
    ALOGD("%s()", __FUNCTION__);
}

/** Create CCCI message */
ccci_buff_t SpeechMessengerCCCI::InitCcciMailbox(uint16_t id, uint16_t param_16bit, uint32_t param_32bit) {
    ccci_buff_t ccci_buff;
    memset(&ccci_buff, 0, sizeof(ccci_buff));

    ccci_buff.magic    = CCCI_MAILBOX_MAGIC_NUMBER;
    ccci_buff.message  = (id << 16) | param_16bit;
    ccci_buff.channel  = CCCI_A2M_CHANNEL;
    ccci_buff.reserved = param_32bit;

    return ccci_buff;
}

/** Get CCCI message's ID */
uint16_t SpeechMessengerCCCI::GetMessageID(const ccci_buff_t &ccci_buff) {
    return (ccci_buff.message) >> 16;
}

/** Get CCCI message's parameters */
uint16_t SpeechMessengerCCCI::GetMessageParam(const ccci_buff_t &ccci_buff) {
    return (ccci_buff.message) & 0xFFFF;
}

uint16_t SpeechMessengerCCCI::GetMessageLength(const ccci_buff_t &ccci_buff) {
    return (ccci_buff.message) & 0xFFFF; //GetMessageParam(ccci_buff);
}

uint16_t SpeechMessengerCCCI::GetMessageOffset(const ccci_buff_t &ccci_buff) {
    return (ccci_buff.reserved) & 0xFFFF;
}


char SpeechMessengerCCCI::GetModemCurrentStatus() {
    unsigned int retStatus = (unsigned int)MODEM_STATUS_INVALID;
    char bufferStatus[2]= {0};
    char status = (char)MODEM_STATUS_INVALID;

    int retval = ::ioctl(fHdlRead, CCCI_IOC_GET_MD_STATE, &retStatus);
    sprintf(bufferStatus, "%c", retStatus);
    status = atoi(bufferStatus);

    if (mIsModemEPOF == true) {
        ALOGD("%s() MD EPOF!", __FUNCTION__);
        status = MODEM_STATUS_INVALID;
    } else if (retval < 0) {
        ALOGE("%s() ioctl _IOR(CCCI_IOC_GET_MD_STATEt) fail!! retval = %d", __FUNCTION__, retval);
        status = MODEM_STATUS_EXPT;
    }
    ALOGV("%s(): retStatus(%c), int status(%d)", __FUNCTION__, retStatus, status);

    return status;
}

bool SpeechMessengerCCCI::CheckModemIsReady() {
    return (GetModemCurrentStatus() == MODEM_STATUS_READY);
}

status_t SpeechMessengerCCCI::WaitUntilModemReady() {
    char status = 0;
    uint32_t trycnt = 0;
    const uint32_t kMaxTryCnt = 10; // total 200 msec
    do {
        status = GetModemCurrentStatus();
        if (status == MODEM_STATUS_READY) {
            ALOGD("%s(), Modem ready", __FUNCTION__);
            break;
        } else {
            ALOGW("%s(), Wait CCCI open #%d times, modem current status = %d, errno: %d",
                  __FUNCTION__, ++trycnt, status, errno);
            sph_msleep(20);
            if (trycnt == kMaxTryCnt) { break; }
        }
    } while (1);

    return (trycnt < kMaxTryCnt) ? NO_ERROR : TIMED_OUT;
}


int32_t SpeechMessengerCCCI::SendMessage(const ccci_buff_t &ccci_buff) {
    // check if already initialized
    if (fHdlWrite < 0) {
        if (Initial() != NO_ERROR) { return UNKNOWN_ERROR; }
    }

    // check if need ack
    const uint16_t message_id = GetMessageID(ccci_buff);
    const bool     b_need_ack = (JudgeAckOfMsg(message_id) == MESSAGE_NEED_ACK) ? true : false;

    // check modem status during phone call
    char modem_status = GetModemCurrentStatus();
    if (modem_status != MODEM_STATUS_READY) {
        ALOGE("%s(), modem_status(%d) != MODEM_STATUS_READY, errno: %d, mIsModemEPOF = %d",
              __FUNCTION__, modem_status, errno, mIsModemEPOF);
        mIsModemResetDuringPhoneCall = true;
        mIsModemReset = true;
        ResetSpeechParamAckCount();
    }

    // Do not send any on/off message when mIsModemResetDuringPhoneCall is true
    if (mIsModemResetDuringPhoneCall == true && IsModemFunctionOnOffMessage(message_id) == true) {
        ALOGE("%s(), mIsModemResetDuringPhoneCall == true, drop on/off message: 0x%x",
              __FUNCTION__, ccci_buff.message);
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
    status_t ret = UNKNOWN_ERROR;
    for (i = 0; i < 150; i++) { // try 150 times for every 2 ms if sent message fail
        write_length = write(fHdlWrite, (void *)&ccci_buff, CCCI_BUF_HEADER_SIZE);
        if (write_length == CCCI_BUF_HEADER_SIZE) {
            LastSentMessage = message_id;
            if (MESSAGE_NEED_ACK == JudgeAckOfMsg(message_id)) {
                LastNeedAckSentMessage = message_id;
            }
            ret = NO_ERROR;
            break;
        } else {
            modem_status = GetModemCurrentStatus();
            ALOGW("%s(), message: 0x%x, try: #%d, write_length: %d, errno: %d, modem status: %d",
                  __FUNCTION__, ccci_buff.message, i, write_length, errno, modem_status);

            if (errno == 3 || modem_status != MODEM_STATUS_READY) { // md reset cause ccci message fail
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

    // error handling for ack message
    if (ret != NO_ERROR && b_need_ack == true) {
        mWaitAckMessageID = 0;
        SendMsgFailErrorHandling(ccci_buff);
    }


    return ret;
}

status_t SpeechMessengerCCCI::ReadMessage(ccci_buff_t &ccci_buff) {
    ALOGV("%s()", __FUNCTION__);
    /* check if already initialized */
    if (fHdlRead < 0) {
        if (Initial() != NO_ERROR) { return UNKNOWN_ERROR; }
    }

    /* read message */
    int read_length = read(fHdlRead, (void *)&ccci_buff, CCCI_BUF_HEADER_SIZE);

    uint16_t message_id = GetMessageID(ccci_buff);
    if (message_id == MSG_M2A_EM_DATA_REQUEST) {
        mIsModemEPOF = false;
    }

    if (read_length != CCCI_BUF_HEADER_SIZE && GetModemCurrentStatus() != MODEM_STATUS_READY) {
        ALOGE("%s() fail, read_length: %d, modem current status: %d",
              __FUNCTION__, read_length, GetModemCurrentStatus());
        return UNKNOWN_ERROR;
    } else if (read_length != -1) {
        ALOGV("%s(), read_length=%d, data[0](magic)=0x%x, data[1](message)=0x%x, ch=0x%x, reserved=0x%x",
              __FUNCTION__, read_length, ccci_buff.magic, ccci_buff.message, ccci_buff.channel, ccci_buff.reserved);
    }
    return NO_ERROR;
}


uint32_t SpeechMessengerCCCI::GetQueueCount() const {
    int32_t count = (iQWrite - iQRead);
    if (count < 0) { count += CCCI_MAX_QUEUE_NUM; }
    return count;
}

bool SpeechMessengerCCCI::CheckOffsetAndLength(const ccci_buff_t &ccci_buff) {
    uint16_t message_id = GetMessageID(ccci_buff);
    uint16_t length     = GetMessageLength(ccci_buff);
    uint16_t offset     = GetMessageOffset(ccci_buff);
    ALOGV("%s(), CheckOffsetAndLength message_id 0x%x , length 0x%x, offset 0x%x",
          __FUNCTION__, message_id, length, offset);
    if (offset > mM2AShareBufLen || length > mM2AShareBufLen) {
        ALOGE("%s(), message_id = 0x%x, length(0x%x), offset(0x%x), mM2AShareBufLen(0x%x)",
              __FUNCTION__, message_id, length, offset, mM2AShareBufLen);
        ASSERT(offset > mM2AShareBufLen || length > mM2AShareBufLen);
        return false;
    } else {
        return true;
    }
}


ccci_message_ack_t SpeechMessengerCCCI::JudgeAckOfMsg(const uint16_t message_id) {
    ccci_message_ack_t ack;
    switch (message_id) {
    case MSG_A2M_SET_SPH_MODE:
    case MSG_A2M_SPH_ON:
    case MSG_A2M_SPH_OFF:
    case MSG_A2M_SPH_ROUTER_ON:
#ifdef SPEECH_PCM_VM_SUPPORT
    case MSG_A2M_PCM_REC_ON:
    case MSG_A2M_VM_REC_ON:
    case MSG_A2M_PCM_REC_OFF:
    case MSG_A2M_VM_REC_OFF:
#else
    case MSG_A2M_RECORD_ON:
    case MSG_A2M_RECORD_OFF:
#endif
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
    //#if defined(MTK_VIBSPK_SUPPORT)
    case MSG_A2M_VIBSPK_PARAMETER:
    //#endif
    case MSG_A2M_SMARTPA_PARAMETER:
    case MSG_A2M_RECORD_RAW_PCM_ON:
    case MSG_A2M_RECORD_RAW_PCM_OFF:
        ack = MESSAGE_NEED_ACK;
        break;
    default:
        ack = MESSAGE_BYPASS_ACK;
    }
    return ack;
}

bool SpeechMessengerCCCI::IsModemFunctionOnOffMessage(const uint16_t message_id) {
    bool bIsModemFunctionOnOffMessage = false;

    switch (message_id) {
    case MSG_A2M_SPH_ON:
    case MSG_A2M_SPH_OFF:
    case MSG_A2M_SPH_ROUTER_ON:
#ifdef SPEECH_PCM_VM_SUPPORT
    case MSG_A2M_PCM_REC_ON:
    case MSG_A2M_VM_REC_ON:
    case MSG_A2M_PCM_REC_OFF:
    case MSG_A2M_VM_REC_OFF:
#else
    case MSG_A2M_RECORD_ON:
    case MSG_A2M_RECORD_OFF:
#endif
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

status_t SpeechMessengerCCCI::SendMessageInQueue(ccci_buff_t ccci_buff) {
    AL_LOCK(mCCCIMessageQueueMutex);

    uint32_t count = GetQueueCount();
    ASSERT(count < (CCCI_MAX_QUEUE_NUM - 1));  // check queue full

    ccci_message_ack_t ack_type = JudgeAckOfMsg(GetMessageID(ccci_buff));

    if (ack_type == MESSAGE_NEED_ACK) {
        ALOGD("%s(), mModemIndex = %d, need ack message: 0x%x, reserved param: 0x%x",
              __FUNCTION__, mModemIndex, ccci_buff.message, ccci_buff.reserved);
    }


    status_t ret = NO_ERROR;

    if (mIsModemEPOF == true) {
        ALOGD("%s(), mIsModemEPOF=%d, Skip message(0x%x) to queue, now count(%u)",
              __FUNCTION__, mIsModemEPOF, ccci_buff.message, GetQueueCount());
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

            ALOGD("%s(), Send message(0x%x) to queue, reserve param(0x%x), mModemIndex(%d),count(%u), "
                  "LastSentMessage(0x%x), LastNeedAckSentMessage(0x%x)",
                  __FUNCTION__, ccci_buff.message, ccci_buff.reserved, mModemIndex, GetQueueCount(),
                  LastSentMessage, LastNeedAckSentMessage);
        }
    }
    AL_UNLOCK(mCCCIMessageQueueMutex);
    return ret;
}

status_t SpeechMessengerCCCI::ConsumeMessageInQueue() {
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
            ALOGD("%s(), no need ack message: 0x%x, count: %u", __FUNCTION__, pQueue[iQRead].ccci_buff.message, count);
            ret = SendMessage(pQueue[iQRead].ccci_buff);
        } else if (pQueue[iQRead].ack_type == MESSAGE_NEED_ACK) {
            ALOGD("%s(), need ack message: 0x%x, count: %u", __FUNCTION__, pQueue[iQRead].ccci_buff.message, count);
            ret = SendMessage(pQueue[iQRead].ccci_buff);
            if (ret == NO_ERROR) { // Send CCCI message success and wait for ack
                break;
            }
        } else if (pQueue[iQRead].ack_type == MESSAGE_CANCELED) { // the cancelled message, ignore it
            ALOGD("%s(), cancel on-off-on message: 0x%x, count: %u",
                  __FUNCTION__, pQueue[iQRead].ccci_buff.message, count);
            ret = NO_ERROR;
        }
    }

    AL_UNLOCK(mCCCIMessageQueueMutex);
    return ret;
}

bool SpeechMessengerCCCI::GetMDResetFlag() {
    ALOGD("%s(), mIsModemReset=%d", __FUNCTION__, mIsModemReset);
    return mIsModemReset;
}

bool SpeechMessengerCCCI::MDReset_CheckMessageInQueue() {
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

status_t SpeechMessengerCCCI::CreateReadingThread() {
    int ret = pthread_create(&hReadThread, NULL, SpeechMessengerCCCI::CCCIReadThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

void *SpeechMessengerCCCI::CCCIReadThread(void *arg) {
    pthread_detach(pthread_self());

    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);

    SpeechMessengerCCCI *pCCCI = (SpeechMessengerCCCI *)arg;
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
    sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p)) {
        ALOGE("[%s] failed, errno: %d", __FUNCTION__, errno);
    } else {
        sched_p.sched_priority = RTPM_PRIO_AUDIO_CCCI_THREAD;
        sched_getparam(0, &sched_p);
        ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
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
        ALOGV("%s(), m2a_message_id = 0x%x", __FUNCTION__, m2a_message_id);

        switch (m2a_message_id) {
        case MSG_M2A_SET_SPH_MODE_ACK: { // ack of MSG_A2M_SET_SPH_MODE
            // Do nothing... just leave a log
            ALOGD("--SetSpeechMode Ack done(0x%x)", ccci_buff.message);
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

            ALOGD("--SpeechRouterOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
        //case MSG_M2A_SPH_TYPE_NOTIFY://0xAF29
        case MSG_M2A_NETWORK_STATUS_NOTIFY: {
            //speech network type change
            bool nb = ((ccci_buff.message & 0x8) >> 3) ? false : true;
#ifdef MTK_AUDIO_GAIN_TABLE
            AudioVolumeFactory::CreateAudioVolumeController()->speechBandChange(nb);
#endif

            //pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_SPH_TYPE_ACK, 0, 0));//0x2F29
            ALOGD("--MSG_M2A_NETWORK_STATUS_NOTIFY (0x%x)", ccci_buff.message);
            pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_NETWORK_STATUS_ACK, 0, 0));
            break;
        }
            /* Record */
#ifdef SPEECH_PCM_VM_SUPPORT
        case MSG_M2A_PCM_REC_ON_ACK: { // ack of MSG_A2M_PCM_REC_ON
            pCCCI->SetModemSideModemStatus(RECORD_STATUS_MASK);
            ALOGD("--recordOn Ack done(0x%x)", ccci_buff.message);
            break;
        }

        case MSG_M2A_VM_REC_ON_ACK: { // ack of MSG_A2M_VM_REC_ON
            pCCCI->SetModemSideModemStatus(VM_RECORD_STATUS_MASK);
            ALOGD("--VMRecordOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
#else
        case MSG_M2A_RECORD_ON_ACK: { // ack of MSG_A2M_RECORD_ON
            pCCCI->SetModemSideModemStatus(RECORD_STATUS_MASK);
            ALOGD("--recordOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
#endif

        case MSG_M2A_RECORD_RAW_PCM_ON_ACK: { // ack of MSG_A2M_RECORD_RAW_PCM_ON
            pCCCI->SetModemSideModemStatus(RAW_RECORD_STATUS_MASK);
            ALOGD("--RawRecordOn Ack done(0x%x)", ccci_buff.message);
            break;
        }


#ifdef SPEECH_PCM_VM_SUPPORT
        case MSG_M2A_PCM_REC_DATA_NOTIFY: { // meaning that we are recording, modem have some data
            ALOGD("%s() MSG_M2A_PCM_REC_DATA_NOTIFY", __FUNCTION__);
            ASSERT(pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                ALOGV("MSG_M2A_REC_DATA_NOTIFY(0x%x), data_length: %d",
                      ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN);

                // Phone record
                AudioALSACaptureDataProviderVoice::getInstance()->provideModemRecordDataToProvider(
                    pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                if (pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_PCM_REC_DATA_READ_ACK, 0, 0));
                }
            }

            break;
        }

        case MSG_M2A_VM_REC_DATA_NOTIFY: { // meaning that we are recording, modem have some data
            ASSERT(pCCCI->GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(VM_RECORD_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_VM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                ALOGV("MSG_M2A_VM_REC_DATA_NOTIFY(0x%x), data_length: %d",
                      ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN);

                SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
                // VM
                pSpeechVMRecorder->getVmDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));

                if (pCCCI->mLad->GetApSideModemStatus(VM_RECORD_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_VM_REC_DATA_READ_ACK, 0, 0));
                }
            }

            break;
        }
#else
        case MSG_M2A_REC_DATA_NOTIFY: { // meaning that we are recording, modem have some data
            ASSERT(pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                ALOGV("MSG_M2A_REC_DATA_NOTIFY(0x%x), data_length: %d",
                      ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN);

                SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
                if (pSpeechVMRecorder->getVMRecordStatus() == true) { // VM
                    pSpeechVMRecorder->getVmDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff));
                } else { // Phone record
                    AudioALSACaptureDataProviderVoice::getInstance()->provideModemRecordDataToProvider(
                        pCCCI->GetM2AUplinkRingBuffer(ccci_buff));
                }

                if (pCCCI->mLad->GetApSideModemStatus(RECORD_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_REC_DATA_READ_ACK, 0, 0));
                }
            }

            break;
        }

#endif
        case MSG_M2A_RAW_PCM_REC_DATA_NOTIFY: { // meaning that we are recording, modem have some data
            ALOGD("%s() MSG_M2A_RAW_PCM_REC_DATA_NOTIFY", __FUNCTION__);
            ASSERT(pCCCI->GetModemSideModemStatus(RAW_RECORD_STATUS_MASK) == true);

            if (pCCCI->mLad->GetApSideModemStatus(RAW_RECORD_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x) after AP side trun off record!! Drop it.", ccci_buff.message);
            } else {
                uint16_t Bytes_PCM = pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN;
                SLOGV("MSG_M2A_PCM_REC_DATA_NOTIFY(0x%x), data_length: %d", ccci_buff.message, Bytes_PCM);

                // Phone record
                SpeechDataProcessingHandler::getInstance()->provideModemRecordDataToProvider(
                    pCCCI->GetM2ARawPcmRingBuffer(ccci_buff));

                if (pCCCI->mLad->GetApSideModemStatus(RAW_RECORD_STATUS_MASK) == true) {
                    pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_RAW_PCM_REC_DATA_READ_ACK, 0, 0));
                } else {
                    ALOGD("%s() RAW_RECORD_STATUS_MASK(%d)",
                          __FUNCTION__, pCCCI->mLad->GetApSideModemStatus(RAW_RECORD_STATUS_MASK));
                }
            }

            break;
        }
#ifdef SPEECH_PCM_VM_SUPPORT
        case MSG_M2A_PCM_REC_OFF_ACK: { // ack of MSG_A2M_PCM_REC_OFF
            if (pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(RECORD_STATUS_MASK);
            } else {
                ALOGE("--recordOff Ack is not paired!!");
            }
            ALOGD("--recordOff Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_VM_REC_OFF_ACK: { // ack of MSG_A2M_VM_REC_OFF
            if (pCCCI->GetModemSideModemStatus(VM_RECORD_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(VM_RECORD_STATUS_MASK);
            } else {
                ALOGE("--VMRecordOff Ack is not paired!!");
            }
            ALOGD("--VMRecordOff Ack done(0x%x)", ccci_buff.message);
            break;
        }
#else
        case MSG_M2A_RECORD_OFF_ACK: { // ack of MSG_A2M_RECORD_OFF
            if (pCCCI->GetModemSideModemStatus(RECORD_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(RECORD_STATUS_MASK);
            } else {
                ALOGE("--recordOff Ack is not paired!!");
            }
            ALOGD("--recordOff Ack done(0x%x)", ccci_buff.message);
            break;
        }
#endif
        case MSG_M2A_RECORD_RAW_PCM_OFF_ACK: { // ack of MSG_A2M_RECORD_RAW_PCM_OFF
            if (pCCCI->GetModemSideModemStatus(RAW_RECORD_STATUS_MASK) == true) {
                pCCCI->ResetModemSideModemStatus(RAW_RECORD_STATUS_MASK);
            } else {
                ALOGE("--RawRecordOff Ack is not paired!!");
            }
            ALOGD("--RawRecordOff Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* Background sound */
        case MSG_M2A_BGSND_ON_ACK: { // ack of MSG_A2M_BGSND_ON
            pCCCI->SetModemSideModemStatus(BGS_STATUS_MASK);
            ALOGD("--BGSoundOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_BGSND_DATA_REQUEST: { // modem request bgs data to play
            ASSERT(pCCCI->GetModemSideModemStatus(BGS_STATUS_MASK) == true);

            SpeechPcmMixerBGSPlayer *pBGSPlayer = SpeechPcmMixerBGSPlayer::GetInstance();
            if (pCCCI->mLad->GetApSideModemStatus(BGS_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_BGSND_DATA_REQUEST(0x%x) after AP side trun off BGS!! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_BGSND_DATA_REQUEST(0x%x), num_data_request: %d",
                      ccci_buff.message, pCCCI->GetMessageParam(ccci_buff));

                // parse size of data request
                uint16_t num_data_request = pCCCI->GetMessageParam(ccci_buff);
                uint32_t max_buf_length = A2M_SHARED_BUFFER_BGS_DATA_SIZE;
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
            ALOGD("--BGSoundOff Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* PCM2Way */
        case MSG_M2A_PNW_ON_ACK: { // ack of MSG_A2M_PNW_ON
            pCCCI->SetModemSideModemStatus(P2W_STATUS_MASK);
            ALOGD("--PCM2WayOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_PNW_UL_DATA_NOTIFY: { // Get Microphone data from Modem
            ASSERT(pCCCI->GetModemSideModemStatus(P2W_STATUS_MASK) == true);

            Record2Way *pRecord2Way = Record2Way::GetInstance();
            if (pCCCI->mLad->GetApSideModemStatus(P2W_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_PNW_UL_DATA_NOTIFY(0x%x) after AP side trun off PCM2Way!! Drop it.", ccci_buff.message);
            } else {
                ALOGD("MSG_M2A_PNW_UL_DATA_NOTIFY(0x%x), data_length:%d",
                      ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN);
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
                ALOGW("MSG_M2A_PNW_DL_DATA_REQUEST(0x%x) after AP side trun off PCM2Way! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_PNW_DL_DATA_REQUEST(0x%x), num_data_request: %d",
                      ccci_buff.message, pCCCI->GetMessageParam(ccci_buff));

                // parse size of data request
                uint16_t num_data_request = pCCCI->GetMessageParam(ccci_buff);
                uint32_t max_buf_length = A2M_SHARED_BUFFER_P2W_DL_DATA_SIZE;
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
                const uint16_t message_length = CCCI_SHARE_BUFF_HEADER_LEN + data_length;
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
            ALOGD("--PCM2WayOff Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* TTY */
        case MSG_M2A_CTM_ON_ACK: { // ack of MSG_A2M_CTM_ON
            pCCCI->SetModemSideModemStatus(TTY_STATUS_MASK);
            ALOGD("--TtyCtmOn Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_CTM_DEBUG_DATA_NOTIFY: {
            if (pCCCI->mLad->GetApSideModemStatus(TTY_STATUS_MASK) == false) {
                ALOGW("MSG_M2A_CTM_DEBUG_DATA_NOTIFY(0x%x) after AP side trun off TTY!! Drop it.", ccci_buff.message);
            } else {
                SLOGV("MSG_M2A_CTM_DEBUG_DATA_NOTIFY(0x%x), data_length: %d",
                      ccci_buff.message, pCCCI->GetMessageLength(ccci_buff) - CCCI_SHARE_BUFF_HEADER_LEN);
                SpeechVMRecorder *pSpeechVMRecorder = SpeechVMRecorder::getInstance();
                int i = 4, j;
                if (pCCCI->GetM2AShareBufSyncWord(ccci_buff) == CCCI_M2A_SHARE_BUFF_HEADER_SYNC) {
                    while (i) {
                        int data_type = pCCCI->GetM2AShareBufDataType(ccci_buff);
                        if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_IN) {
                            pSpeechVMRecorder->getCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff),
                                                                        pSpeechVMRecorder->pCtmDumpFileUlIn);
                        } else if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_IN) {
                            pSpeechVMRecorder->getCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff),
                                                                        pSpeechVMRecorder->pCtmDumpFileDlIn);
                        } else if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_OUT) {
                            pSpeechVMRecorder->getCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff),
                                                                        pSpeechVMRecorder->pCtmDumpFileUlOut);
                        } else if (data_type == SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_OUT) {
                            pSpeechVMRecorder->getCtmDebugDataFromModem(pCCCI->GetM2AUplinkRingBuffer(ccci_buff),
                                                                        pSpeechVMRecorder->pCtmDumpFileDlOut);
                        }
                        j = pCCCI->GetM2AShareBufDataLength(ccci_buff) + CCCI_SHARE_BUFF_HEADER_LEN;
                        ccci_buff.reserved += j;
                        if (ccci_buff.reserved > pCCCI->mM2AShareBufLen) {
                            ccci_buff.reserved -= pCCCI->mM2AShareBufLen;
                        }
                        i--;
                    }
                }
                pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_CTM_DEBUG_DATA_READ_ACK, 0, 0));
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

            ALOGD("--SetAcousticLoopback Ack done(0x%x)", ccci_buff.message);
            break;
        }

        /* Speech Enhancement parameters */
        case MSG_M2A_EM_NB_ACK: { // ack of MSG_A2M_EM_NB
            pCCCI->AddSpeechParamAckCount(NB_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGD("--SetNBSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_DMNR_ACK: { // ack of MSG_A2M_EM_DMNR
            pCCCI->AddSpeechParamAckCount(DMNR_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGD("--SetDualMicSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_MAGICON_ACK: { // ack of MSG_A2M_EM_MAGICON
            pCCCI->AddSpeechParamAckCount(MAGICON_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGD("--SetMagiConSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_HAC_ACK: { // ack of MSG_A2M_EM_HAC
            pCCCI->AddSpeechParamAckCount(HAC_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGD("--SetHACSpeechParameters Ack done(0x%x)", ccci_buff.message);
            break;
        }
        case MSG_M2A_EM_WB_ACK: { // ack of MSG_A2M_EM_WB
            pCCCI->AddSpeechParamAckCount(WB_SPEECH_PARAM);
            pCCCI->A2MBufUnLock();
            ALOGD("--SetWBSpeechParameters Ack done(0x%x)", ccci_buff.message);
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
            pCCCI->MDReset_CheckMessageInQueue();
            pCCCI->ResetSpeechParamAckCount();

            break;
        }
        case MSG_M2A_VIBSPK_PARAMETER_ACK: { // ack of MSG_M2A_VIBSPK_PARAMETER
            pCCCI->A2MBufUnLock();
            ALOGD("--SetVibSpkParam Ack done(0x%x)", ccci_buff.message);
            break;
        }

        case MSG_M2A_SMARTPA_PARAMETER_ACK: {
            pCCCI->A2MBufUnLock();
            ALOGD("--SetSmartpaParam Ack done(0x%x)", ccci_buff.message);
            break;
        }
        /* Modem EPOF */
        case MSG_M2A_EPOF_NOTIFY: {
            ALOGW("..[MD EPOF Notify(MSG_M2A_EPOF_NOTIFY: 0x%x)]..", ccci_buff.message);

            // Check the first message in queue. If need ack, take action.
            // Modem reset, flush all CCCI queue first. Don't care for the CCCI queue.
            pCCCI->MDReset_CheckMessageInQueue();
            pCCCI->ResetSpeechParamAckCount();

            pCCCI->SendMessage(pCCCI->InitCcciMailbox(MSG_A2M_EPOF_ACK, 0, 0));
            pCCCI->mIsModemEPOF = true;

            break;
        }

        default: {
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
                ALOGW("Message(0x%x) might double ack!! The current mWaitAckMessageID is 0x%x",
                      ccci_buff.message, pCCCI->mWaitAckMessageID);
            }
        }
    }
    pthread_exit(NULL);
    return 0;
}

status_t SpeechMessengerCCCI::CreateSendSphParaThread() {
    int ret = pthread_create(&hSendSphThread, NULL, SpeechMessengerCCCI::SendSphParaThread, (void *)this);
    if (ret != 0) {
        ALOGE("%s() create thread fail!!", __FUNCTION__);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

void *SpeechMessengerCCCI::SendSphParaThread(void *arg) {
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

    SpeechMessengerCCCI *pCCCI = (SpeechMessengerCCCI *)arg;

    // Get SpeechParamLock
    if (pCCCI->SpeechParamLock() == false) {
        ALOGE("%s() fail to get SpeechParamLock!!", __FUNCTION__);
        return 0;
    }

    // Check the first message in queue. If need ack, take action.
    // Modem reset, flush all CCCI queue first. Don't care for the CCCI queue.
    pCCCI->MDReset_CheckMessageInQueue();

    // Send speech parameters to modem side
    pCCCI->ResetSpeechParamAckCount();
    pCCCI->mLad->SetAllSpeechEnhancementInfoToModem();

    // Release SpeechParamLock
    pCCCI->SpeechParamUnLock();

    pthread_exit(NULL);
    return 0;
}


bool SpeechMessengerCCCI::A2MBufLock() {
    const uint32_t kA2MBufLockTimeout = 3000; // 3 sec

    int rc = AL_LOCK_MS(mA2MShareBufMutex, kA2MBufLockTimeout);
    ALOGD("%s()", __FUNCTION__);
    if (rc != 0) {
        ALOGE("%s(), Cannot get Lock!! Timeout : %u msec", __FUNCTION__, kA2MBufLockTimeout);
        return false;
    }
    return true;
}

void SpeechMessengerCCCI::A2MBufUnLock() {
    AL_UNLOCK(mA2MShareBufMutex);
    ALOGD("%s()", __FUNCTION__);
}

bool SpeechMessengerCCCI::SpeechParamLock() {
    const uint32_t kSphParamLockTimeout = 10000; // 10 sec

    ALOGD("%s()", __FUNCTION__);
    int rc = AL_LOCK_MS(mSetSpeechParamMutex, kSphParamLockTimeout);
    if (rc != 0) {
        ALOGE("%s(), Cannot get Lock!! Timeout : %u msec", __FUNCTION__, kSphParamLockTimeout);
        return false;
    }
    return true;
}

void SpeechMessengerCCCI::SpeechParamUnLock() {
    ALOGD("%s()", __FUNCTION__);
    AL_UNLOCK(mSetSpeechParamMutex);
}

void SpeechMessengerCCCI::ResetSpeechParamAckCount() {
    memset(&mSpeechParamAckCount, 0, sizeof(mSpeechParamAckCount));
    ALOGD("%s(), NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)", __FUNCTION__,
          mSpeechParamAckCount[NB_SPEECH_PARAM],
          mSpeechParamAckCount[DMNR_SPEECH_PARAM],
          mSpeechParamAckCount[WB_SPEECH_PARAM],
          mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
          mSpeechParamAckCount[HAC_SPEECH_PARAM]);
}

void SpeechMessengerCCCI::AddSpeechParamAckCount(speech_param_ack_t type) {
    if (type >= NUM_SPEECH_PARAM_ACK_TYPE || type < 0) {
        ALOGE("%s(), no such type: %d", __FUNCTION__, type);
    } else {
        if (mSpeechParamAckCount[type] < 0xFFFFFFFF) { //prevent overflow
            mSpeechParamAckCount[type]++;
        }
        ALOGD("%s(%d), NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)", __FUNCTION__, type,
              mSpeechParamAckCount[NB_SPEECH_PARAM],
              mSpeechParamAckCount[DMNR_SPEECH_PARAM],
              mSpeechParamAckCount[WB_SPEECH_PARAM],
              mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
              mSpeechParamAckCount[HAC_SPEECH_PARAM]);
    }
}

bool SpeechMessengerCCCI::CheckSpeechParamAckAllArrival() {
    bool ret = true;

    // Get SpeechParamLock
    if (SpeechParamLock() == false) {
        ALOGE("%s() fail to get SpeechParamLock!!", __FUNCTION__);
        return false;
    }

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

    if (ret == true) {
        ALOGD("%s() Pass", __FUNCTION__);
    } else {
        ALOGE("%s() Fail, NB(%u)/DMNR(%u)/WB(%u)/MAGICON(%u)/HAC(%u)", __FUNCTION__,
              mSpeechParamAckCount[NB_SPEECH_PARAM],
              mSpeechParamAckCount[DMNR_SPEECH_PARAM],
              mSpeechParamAckCount[WB_SPEECH_PARAM],
              mSpeechParamAckCount[MAGICON_SPEECH_PARAM],
              mSpeechParamAckCount[HAC_SPEECH_PARAM]);

        // Send speech parameters to modem side again
        mLad->SetAllSpeechEnhancementInfoToModem();
        ResetWasModemReset(false);
    }

    // Release SpeechParamLock
    SpeechParamUnLock();

    return ret;
}

/** Do error handling here */
void SpeechMessengerCCCI::SendMsgFailErrorHandling(const ccci_buff_t &ccci_buff) {
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
#ifdef SPEECH_PCM_VM_SUPPORT

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
#else
    case MSG_A2M_RECORD_ON: {
        SetModemSideModemStatus(RECORD_STATUS_MASK);
        break;
    }
    case MSG_A2M_RECORD_OFF: {
        if (GetModemSideModemStatus(RECORD_STATUS_MASK) == true) {
            ResetModemSideModemStatus(RECORD_STATUS_MASK);
        } else {
            ALOGE("--recordOff Ack is not paired!!");
        }
        break;
    }
#endif
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
    case MSG_A2M_SMARTPA_PARAMETER: {
        A2MBufUnLock();
        break;
    }
    default: {
        ALOGW("%s(), message: 0x%x, ack don't care", __FUNCTION__, ccci_buff.message);
    }
    }
}

RingBuf SpeechMessengerCCCI::GetM2AUplinkRingBuffer(const ccci_buff_t &ccci_buff) {
    // check MD side data msg format
    CheckOffsetAndLength(ccci_buff);

    RingBuf ul_ring_buf;

    ul_ring_buf.bufLen   = mM2AShareBufLen;
    ul_ring_buf.pBufBase = mM2AShareBufBase;
    ul_ring_buf.pBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;

    ul_ring_buf.pRead    = ul_ring_buf.pBufBase + GetMessageOffset(ccci_buff);
    if (ul_ring_buf.pRead  >= mM2AShareBufEnd) { ul_ring_buf.pRead  -= ul_ring_buf.bufLen; }

    // share buffer header
    char *p_sync_word = ul_ring_buf.pRead + 0; // 0 * size(unsigned short)
    char *p_data_type = ul_ring_buf.pRead + 2; // 1 * size(unsigned short)
    char *p_data_len  = ul_ring_buf.pRead + 4; // 2 * size(unsigned short)

    if (p_data_type >= mM2AShareBufEnd) { p_data_type -= ul_ring_buf.bufLen; }
    if (p_data_len  >= mM2AShareBufEnd) { p_data_len  -= ul_ring_buf.bufLen; }

    if (*(uint16_t *)p_sync_word != CCCI_M2A_SHARE_BUFF_HEADER_SYNC) {
        char idxDump = 10, u4I;
        char SphDumpStr[SPH_DUMP_STR_SIZE] = {0};

        snprintf(SphDumpStr, SPH_DUMP_STR_SIZE, "ring_buf[pBase %p, pEnd %p, Len 0x%x, pRead %p, offset 0x%x]= ",
                 ul_ring_buf.pBufBase, mM2AShareBufEnd, mM2AShareBufLen, ul_ring_buf.pRead, ccci_buff.message);
        for (u4I = 0; u4I < idxDump; u4I++) {
            char SphDumpTemp[100];
            snprintf(SphDumpTemp, 100, "[%d, %p]0x%x, ", u4I, (uint16_t *)ul_ring_buf.pRead + u4I,
                     *((uint16_t *)ul_ring_buf.pRead + u4I));
            audio_strncat(SphDumpStr, SphDumpTemp, SPH_DUMP_STR_SIZE);
        }

        if (u4I != 0) {
            ALOGD("%s(), offset(0x%x), %s", __FUNCTION__, GetMessageOffset(ccci_buff), SphDumpStr);
        }
        ALOGD("%s(), message = 0x%x, *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x)",
              __FUNCTION__, ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type,
              *(uint16_t *)p_data_len);

    }
    ASSERT(*(uint16_t *)p_sync_word == CCCI_M2A_SHARE_BUFF_HEADER_SYNC);

    ul_ring_buf.pRead += CCCI_SHARE_BUFF_HEADER_LEN;
    if (ul_ring_buf.pRead >= mM2AShareBufEnd) { ul_ring_buf.pRead -= ul_ring_buf.bufLen; }

    ul_ring_buf.pWrite   = ul_ring_buf.pRead    + (*(uint16_t *)p_data_len);
    if (ul_ring_buf.pWrite >= mM2AShareBufEnd) { ul_ring_buf.pWrite -= ul_ring_buf.bufLen; }

#if 0
    uint16_t count = *(uint16_t *)p_data_len;
    if (ul_ring_buf.pRead <= ul_ring_buf.pWrite) {
        fwrite((void *)ul_ring_buf.pRead, sizeof(char), count, mLad->fp);
    } else {
        char *end = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;
        int r2e = end - ul_ring_buf.pRead;
        if (count <= r2e) {
            fwrite((void *)ul_ring_buf.pRead, sizeof(char), count, mLad->fp);
        } else {
            fwrite((void *)ul_ring_buf.pRead, sizeof(char), r2e, mLad->fp);
            fwrite((void *)ul_ring_buf.pBufBase, sizeof(char), count - r2e, mLad->fp);
        }
    }
#endif

    return ul_ring_buf;
}

RingBuf SpeechMessengerCCCI::GetM2ACtmRingBuffer(const ccci_buff_t &ccci_buff) {
    // check MD side data msg format
    CheckOffsetAndLength(ccci_buff);

    RingBuf ul_ring_buf;

    ul_ring_buf.bufLen   = mM2AShareBufLen;
    ul_ring_buf.pBufBase = mM2AShareBufBase;
    ul_ring_buf.pBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;

    ul_ring_buf.pRead    = ul_ring_buf.pBufBase + GetMessageOffset(ccci_buff);
    if (ul_ring_buf.pRead  >= mM2AShareBufEnd) { ul_ring_buf.pRead  -= ul_ring_buf.bufLen; }

    // share buffer header
    char *p_sync_word = ul_ring_buf.pRead + 0; // 0 * size(unsigned short)
    char *p_data_type = ul_ring_buf.pRead + 2; // 1 * size(unsigned short)
    char *p_data_len  = ul_ring_buf.pRead + 4; // 2 * size(unsigned short)

    if (p_data_type >= mM2AShareBufEnd) { p_data_type -= ul_ring_buf.bufLen; }
    if (p_data_len  >= mM2AShareBufEnd) { p_data_len  -= ul_ring_buf.bufLen; }

    if (*(uint16_t *)p_sync_word != CCCI_M2A_SHARE_BUFF_HEADER_SYNC) {
        char idxDump = 10, u4I;
        char SphDumpStr[SPH_DUMP_STR_SIZE] = {0};

        snprintf(SphDumpStr, SPH_DUMP_STR_SIZE, "ring_buf[pBase %p, pEnd %p, Len 0x%x, pRead %p, offset 0x%x]= ",
                 ul_ring_buf.pBufBase, mM2AShareBufEnd, mM2AShareBufLen, ul_ring_buf.pRead, ccci_buff.message);
        for (u4I = 0; u4I < idxDump; u4I++) {
            char SphDumpTemp[100];
            snprintf(SphDumpTemp, 100, "[%d, %p]0x%x, ", u4I, (uint16_t *)ul_ring_buf.pRead + u4I,
                     *((uint16_t *)ul_ring_buf.pRead + u4I));
            audio_strncat(SphDumpStr, SphDumpTemp, SPH_DUMP_STR_SIZE);
        }

        if (u4I != 0) {
            ALOGD("%s(), offset(0x%x), %s", __FUNCTION__, GetMessageOffset(ccci_buff), SphDumpStr);
        }

        ALOGD("%s(), message = 0x%x, *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x)",
              __FUNCTION__, ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type,
              *(uint16_t *)p_data_len);
    }

    ASSERT(*(uint16_t *)p_sync_word == CCCI_M2A_SHARE_BUFF_HEADER_SYNC);

    ul_ring_buf.pRead += CCCI_SHARE_BUFF_HEADER_LEN;
    if (ul_ring_buf.pRead >= mM2AShareBufEnd) { ul_ring_buf.pRead -= ul_ring_buf.bufLen; }

    ul_ring_buf.pWrite   = ul_ring_buf.pRead    + GetM2AShareBufDataLength(ccci_buff);
    if (ul_ring_buf.pWrite >= mM2AShareBufEnd) { ul_ring_buf.pWrite -= ul_ring_buf.bufLen; }

    return ul_ring_buf;
}

uint16_t SpeechMessengerCCCI::GetPcmFreq(const uint16_t Idx_Freq) {
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

RingBuf SpeechMessengerCCCI::GetM2ARawPcmRingBuffer(const ccci_buff_t &ccci_buff) {
    ALOGV("%s(), id(0x%x), mM2AShareBufLen(0x%x), A2M_SHARED_BUFFER_BGS_DATA_SIZE(%d)",
          __FUNCTION__, GetMessageID(ccci_buff), mM2AShareBufLen, A2M_SHARED_BUFFER_BGS_DATA_SIZE);

    // check MD side data msg format
    CheckOffsetAndLength(ccci_buff);

    spcRAWPCMBufInfo header_RawPcmBufInfo;
    spcApRAWPCMBufHdr header_ApRawPcmBuf;

    char *SeqPcmBuf = new char [mM2AShareBufLen];
    memset(SeqPcmBuf, 0, mM2AShareBufLen);
    //    char SeqPcmBuf[CCCI_MAX_PAYLOAD_SIZE * 4 + 16] = {0};
    RingBuf ul_ring_buf;

    uint16_t size_copy1, size_copy2, BytesCopied, BytesToCopy;
    char *PtrTarget = NULL;
    char *PtrSource = NULL;

    ul_ring_buf.bufLen   = mM2AShareBufLen;
    ul_ring_buf.pBufBase = mM2AShareBufBase;
    ul_ring_buf.pBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;

    ul_ring_buf.pRead    = ul_ring_buf.pBufBase + GetMessageOffset(ccci_buff);
    if (ul_ring_buf.pRead  >= mM2AShareBufEnd) { ul_ring_buf.pRead  -= ul_ring_buf.bufLen; }

    // share buffer header
    //parse message
    char *p_sync_word = ul_ring_buf.pRead + 0; // size(unsigned short)
    char *p_data_type = ul_ring_buf.pRead + 2; // size(unsigned short)
    char *p_data_len  = ul_ring_buf.pRead + 4;// size(unsigned short)

    if (p_data_type >= mM2AShareBufEnd) { p_data_type -= ul_ring_buf.bufLen; }
    if (p_data_len  >= mM2AShareBufEnd) { p_data_len  -= ul_ring_buf.bufLen; }

    if (*(uint16_t *)p_sync_word != CCCI_M2A_SHARE_BUFF_HEADER_SYNC) {
        char idxDump = 10, u4I;
        char SphDumpStr[SPH_DUMP_STR_SIZE] = {0};

        snprintf(SphDumpStr, SPH_DUMP_STR_SIZE, "ring_buf[pBase %p, pEnd %p, Len 0x%x, pRead %p, offset 0x%x]= ",
                 ul_ring_buf.pBufBase, mM2AShareBufEnd, mM2AShareBufLen, ul_ring_buf.pRead, ccci_buff.message);
        for (u4I = 0; u4I < idxDump; u4I++) {
            char SphDumpTemp[100];
            snprintf(SphDumpTemp, 100, "[%d, %p]0x%x, ", u4I, (uint16_t *)ul_ring_buf.pRead + u4I,
                     *((uint16_t *)ul_ring_buf.pRead + u4I));
            audio_strncat(SphDumpStr, SphDumpTemp, SPH_DUMP_STR_SIZE);
        }

        if (u4I != 0) {
            ALOGD("%s(), offset(0x%x), %s", __FUNCTION__, GetMessageOffset(ccci_buff), SphDumpStr);
        }

        ALOGD("%s(), message = 0x%x, *p_sync_word(0x%x), *p_data_type(0x%x), *p_data_len(0x%x)",
              __FUNCTION__, ccci_buff.message, *(uint16_t *)p_sync_word, *(uint16_t *)p_data_type,
              *(uint16_t *)p_data_len);
    }

    ASSERT(*(uint16_t *)p_sync_word == CCCI_M2A_SHARE_BUFF_HEADER_SYNC);

    BytesCopied = (*(uint16_t *)p_data_len) << 1;
    ul_ring_buf.pWrite   = ul_ring_buf.pRead + BytesCopied;

    ALOGV("%s(), before memcpy mM2AShareBufEnd(%p), BytesCopied(0x%x), p_data_len(0x%x), pRead(%p), pWrite(%p)",
          __FUNCTION__, mM2AShareBufEnd, BytesCopied, (*(uint16_t *)p_data_len), ul_ring_buf.pRead,
          ul_ring_buf.pWrite);

    // share buffer header
    if (ul_ring_buf.pWrite >= mM2AShareBufEnd) {
        ul_ring_buf.pWrite -= ul_ring_buf.bufLen;

        size_copy1 = mM2AShareBufEnd - ul_ring_buf.pRead;
        size_copy2 = BytesCopied - size_copy1;
        ALOGV("%s(), mM2AShareBufEnd(%p), BytesCopied(0x%x), size_copy1(0x%x), size_copy2(0x%x),"
              " pRead(%p), pWrite(%p)",
              __FUNCTION__, mM2AShareBufEnd, BytesCopied, size_copy1, size_copy2, ul_ring_buf.pRead,
              ul_ring_buf.pWrite);
        memcpy((char *)mSphCCCITmpBuf, (char *)ul_ring_buf.pRead,  size_copy1);
        memcpy((char *)mSphCCCITmpBuf + size_copy1, (char *)ul_ring_buf.pBufBase, size_copy2);
    } else {
        ALOGV("%s(), message = 0x%x, mSphCCCITmpBuf(%p), ul_ring_buf.pRead(%p), BytesCopied(0x%x)",
              __FUNCTION__, ccci_buff.message, (char *)mSphCCCITmpBuf, (uint16_t *)ul_ring_buf.pRead, BytesCopied);
        memcpy((char *)mSphCCCITmpBuf, (char *)ul_ring_buf.pRead,  BytesCopied);
    }
    memcpy(&header_RawPcmBufInfo, (char *)mSphCCCITmpBuf + CCCI_SHARE_BUFF_HEADER_LEN, sizeof(spcRAWPCMBufInfo));
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
        PtrSource = (char *)mSphCCCITmpBuf + CCCI_SHARE_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN;
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
        PtrSource = (char *)mSphCCCITmpBuf + CCCI_SHARE_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN +
                    header_RawPcmBufInfo.u16ULLength;
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
        PtrSource = (char *)mSphCCCITmpBuf + CCCI_SHARE_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN;
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
        PtrSource = (char *)mSphCCCITmpBuf + CCCI_SHARE_BUFF_HEADER_LEN + CCCI_RAW_PCM_BUFF_HEADER_LEN +
                    header_RawPcmBufInfo.u16ULLength;
        BytesToCopy = header_RawPcmBufInfo.u16DLLength;
        memcpy(PtrTarget, PtrSource, BytesToCopy);
        BytesCopied += BytesToCopy;
        break;
    }
    //ring buffer process
    pcm_ring_buf.pRead  = pcm_ring_buf.pWrite;
    pcm_ring_buf.pWrite  = pcm_ring_buf.pRead + BytesCopied;
    if (pcm_ring_buf.pWrite >= mSphPCMBufEnd) {
        pcm_ring_buf.pWrite -= pcm_ring_buf.bufLen;

        size_copy1 = mSphPCMBufEnd - pcm_ring_buf.pRead;
        size_copy2 = BytesCopied - size_copy1;
        ALOGV("%s(), mSphPCMBufEnd(%p), BytesCopied(0x%x), size_copy1(0x%x), size_copy2(0x%x), pRead(%p), pWrite(%p)",
              __FUNCTION__, mSphPCMBufEnd, BytesCopied, size_copy1, size_copy2, pcm_ring_buf.pRead,
              pcm_ring_buf.pWrite);
        memcpy(pcm_ring_buf.pRead, (char *)SeqPcmBuf, size_copy1);
        memcpy(pcm_ring_buf.pBufBase, (char *)SeqPcmBuf + size_copy1, size_copy2);
    } else {
        memcpy(pcm_ring_buf.pRead, (char *)SeqPcmBuf, BytesCopied);
    }
    ALOGV("-%s(), pBufBase(%p), bufLen(0x%x), BytesCopied(0x%x), pRead(%p), pWrite(%p)",
          __FUNCTION__, pcm_ring_buf.pBufBase, pcm_ring_buf.bufLen, BytesCopied, pcm_ring_buf.pRead,
          pcm_ring_buf.pWrite);

    if (SeqPcmBuf != NULL) {
        delete[] SeqPcmBuf;
    }
    return pcm_ring_buf;
}

uint16_t SpeechMessengerCCCI::GetM2AShareBufSyncWord(const ccci_buff_t &ccci_buff) {
    char *p_sync_word = GetM2AShareBufBase() + GetMessageOffset(ccci_buff) + 0; // 0 * size(unsigned short)
    if (p_sync_word >= mM2AShareBufEnd) { p_sync_word -= mM2AShareBufLen; }
    SLOGV("%s(), sync = 0x%x", __FUNCTION__, *(uint16_t *)p_sync_word);
    return *(uint16_t *)p_sync_word;
}

uint16_t SpeechMessengerCCCI::GetM2AShareBufDataType(const ccci_buff_t &ccci_buff) {
    char *p_data_type = GetM2AShareBufBase() + GetMessageOffset(ccci_buff) + 2; // 1 * size(unsigned short)
    if (p_data_type >= mM2AShareBufEnd) { p_data_type -= mM2AShareBufLen; }
    SLOGV("%s(), type = 0x%x", __FUNCTION__, *(uint16_t *)p_data_type);
    return *(uint16_t *)p_data_type;
}

uint16_t SpeechMessengerCCCI::GetM2AShareBufDataLength(const ccci_buff_t &ccci_buff) {
    char *p_data_len  = GetM2AShareBufBase() + GetMessageOffset(ccci_buff) + 4; // 2 * size(unsigned short)
    if (p_data_len  >= mM2AShareBufEnd) { p_data_len  -= mM2AShareBufLen; }
    SLOGV("%s(), data_len = %d", __FUNCTION__, *(uint16_t *)p_data_len);
    return *(uint16_t *)p_data_len;
}


bool SpeechMessengerCCCI::GetModemSideModemStatus(const modem_status_mask_t modem_status_mask) const {
    return ((mModemSideModemStatus & modem_status_mask) > 0);
}

void SpeechMessengerCCCI::SetModemSideModemStatus(const modem_status_mask_t modem_status_mask) {
    mModemSideModemStatus |= modem_status_mask;

    // save mModemSideModemStatus in kernel to avoid medieserver die
    set_uint32_to_mixctrl(kPropertyKeyModemStatus, mModemSideModemStatus);
}

void SpeechMessengerCCCI::ResetModemSideModemStatus(const modem_status_mask_t modem_status_mask) {
    mModemSideModemStatus &= (~modem_status_mask);

    // save mModemSideModemStatus in kernel to avoid medieserver die
    set_uint32_to_mixctrl(kPropertyKeyModemStatus, mModemSideModemStatus);
}
status_t SpeechMessengerCCCI::setPcmRecordType(SpcRecordTypeStruct typeRecord) {
    mPcmRecordType = typeRecord;
    ALOGD("%s(), mPcmRecordType(%d)", __FUNCTION__, mPcmRecordType.direction);
    return NO_ERROR;
}

void SpeechMessengerCCCI::GetRFInfo(void) {
    uint16_t mRfMode = 0, mRfInfo = 0;
    char property_value[PROPERTY_VALUE_MAX], property_value2[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_RF_INFO[0], property_value, "0");  //"0": default all off
    mRfInfo = atoi(property_value);

    property_get(PROPERTY_KEY_RF_INFO[1], property_value2, "0");  //"0": default all off
    mRfMode = atoi(property_value2);

    ALOGD("%s(), mRfInfo(0x%x), mRfMode(%d)", __FUNCTION__, mRfInfo, mRfMode);
}

void SpeechMessengerCCCI::SetRFInfo(char mRFIdx, uint16_t mRfData) {
    // save mRfMode, mRfInfo in property to avoid medieserver die
    char property_value[PROPERTY_VALUE_MAX];
    sprintf(property_value, "%u", mRfData);
    property_set(PROPERTY_KEY_RF_INFO[(unsigned char)mRFIdx], property_value);
}

void SpeechMessengerCCCI::ResetRFInfo(void) {
    uint16_t mRfMode = 0, mRfInfo = 0;

    // save mRfMode, mRfInfo in property to avoid medieserver die
    char property_value[PROPERTY_VALUE_MAX], property_value2[PROPERTY_VALUE_MAX];
    sprintf(property_value, "%u", mRfMode);
    property_set(PROPERTY_KEY_RF_INFO[0], property_value);

    sprintf(property_value, "%u", mRfInfo);
    property_set(PROPERTY_KEY_RF_INFO[1], property_value);
}

#ifdef USE_CCCI_SHARE_BUFFER
void SpeechMessengerCCCI::InitA2MRawParaRingBuffer() {
    return ;
}
status_t SpeechMessengerCCCI::GetA2MRawParaRingBuffer(uint16_t *offset, uint16_t *avail) {
    return NO_ERROR;
}

status_t SpeechMessengerCCCI::AdvanceA2MRawParaRingBuffer(int datalength) {
    return NO_ERROR;
}

status_t SpeechMessengerCCCI::WriteA2MRawParaRingBuffer(char *data, int datalength) {
    return NO_ERROR;
}
#endif
} // end of namespace android

