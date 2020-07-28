#ifndef ANDROID_SPEECH_MESSAGE_INTERFACE_H
#define ANDROID_SPEECH_MESSAGE_INTERFACE_H

#include <pthread.h>

#include "AudioType.h"
#include "SpeechType.h"
#include "AudioUtility.h"

#include <SpeechPcmMixerBGSPlayer.h>
#include "SpeechPcm2way.h"

#include <SpeechMessageID.h>



namespace android {

#define CCCI_BUF_HEADER_SIZE 16
//EEMCS MTU 3584-128 = 3456 byte
#define CCCI_MAX_PAYLOAD_DATA_BYTE 3434 //(3456-16)-6
#define CCCI_MAX_PAYLOAD_SIZE 860 //(3456-16)/4 = 860
#define CCCI_MAX_PAYLOAD_BYTE 3440 //(3456-16)/4 = 860
/** CCCI buffer structure */
typedef struct {
    uint32_t magic;
    uint32_t message; // message[31:16] = id, message[15:0] = parameters
    uint32_t channel;
    uint32_t reserved;
    uint32_t payload[CCCI_MAX_PAYLOAD_SIZE];
} ccci_buff_t;
/** CCCI driver & ioctl */

/** CCCI message need/no need ack type */
enum ccci_message_ack_t {
    MESSAGE_BYPASS_ACK = 0,
    MESSAGE_NEED_ACK   = 1,
    MESSAGE_CANCELED   = 8
};

/** CCCI message queue structure */
typedef struct ccci_queue_element_t {
    ccci_buff_t        ccci_buff;
    ccci_message_ack_t ack_type;
} ccci_queue_element_t;


/** Speech parameter ack type */
enum speech_param_ack_t {
    NB_SPEECH_PARAM = 0,
    DMNR_SPEECH_PARAM,
    WB_SPEECH_PARAM,
    MAGICON_SPEECH_PARAM,
    HAC_SPEECH_PARAM,
    DYNAMIC_SPH_PARAM,
    NUM_SPEECH_PARAM_ACK_TYPE
};


/** CCCI share buffer related infomation */
const size_t CCCI_SHARE_BUFF_HEADER_LEN = 6;
const size_t CCCI_PAYLOAD_BUFF_HEADER_LEN = 10;
const size_t CCCI_RAW_PCM_BUFF_HEADER_LEN = 8;
#define CCCI_SHAREBUF_GUARD_LENGTH 32

enum share_buff_sync_t {
    CCCI_A2M_SHARE_BUFF_HEADER_SYNC = 0xA2A2,
    CCCI_M2A_SHARE_BUFF_HEADER_SYNC = 0x2A2A,
    EEMCS_M2A_SHARE_BUFF_HEADER_SYNC = 0x1234
};


enum speech_band_info_t {
    SPEECH_NARROW_BAND = 0,
    SPEECH_WIDEBAND,
    SPEECH_SUPER_WIDBAND,
    SPEECH_FULL_BAND,
};


//For VT case, the CCCI message for every 20ms, UL/DL have 2 CCCI message (Put to Speaker / Get from Mic)
//For BGS off ack message, the worst case maybe pending 150 ms. And for other change device control. (BGSoff,2WAY off,SPH off,...)
//The total message maybe > 20 for this period. So enlarge the total CCCI message queue.
//For the CCCI queue in CCCI kernel driver, the size is 60.
//For the CCCI queue in Modem side, the size is 32.
//Modem side would keep on optimized the BGS off ack period.
const size_t CCCI_MAX_QUEUE_NUM = 60;

//BGSound use MAX 2026 bytes  for 16K SR(2020+6 bytes)
//MAX Payload size 3440 bytes
// payload length inculde 6 byte sync words and data size
#ifdef CCCI_FORCE_USE
const size_t A2M_SHARED_BUFFER_BGS_DATA_SIZE = 2026;
const size_t A2M_SHARED_BUFFER_P2W_DL_DATA_SIZE = PCM2WAY_PLAY_BUFFER_WB_LEN;
#else
const size_t A2M_SHARED_BUFFER_BGS_DATA_SIZE = CCCI_MAX_PAYLOAD_BYTE;
const size_t A2M_SHARED_BUFFER_P2W_DL_DATA_SIZE = CCCI_MAX_PAYLOAD_BYTE;
#endif
const size_t A2M_SHARED_BUFFER_BGS_DATA_BASE = 0;
const size_t A2M_SHARED_BUFFER_BGS_DATA_END  = A2M_SHARED_BUFFER_BGS_DATA_BASE + CCCI_SHARE_BUFF_HEADER_LEN + A2M_SHARED_BUFFER_BGS_DATA_SIZE;

// PCM2WAY playback use MAX 640 bytes for 16K SR( 640+6 bytes)
const size_t A2M_SHARED_BUFFER_P2W_DL_DATA_BASE = A2M_SHARED_BUFFER_BGS_DATA_END;
const size_t A2M_SHARED_BUFFER_P2W_DL_DATA_END  = A2M_SHARED_BUFFER_P2W_DL_DATA_BASE + CCCI_SHARE_BUFF_HEADER_LEN + A2M_SHARED_BUFFER_P2W_DL_DATA_SIZE;

// Speech enhacement parameters use MAX 640 bytes for WB Param( 2919+6 bytes)
const size_t A2M_SHARED_BUFFER_SPH_PARAM_BASE = A2M_SHARED_BUFFER_P2W_DL_DATA_END;
const size_t A2M_SHARED_BUFFER_SPH_PARAM_SIZE = CCCI_MAX_PAYLOAD_BYTE;

//UL DL Encryptor
const size_t A2M_SHARED_BUFFER_ULDL_DATA_SIZE = 1024;
const size_t A2M_SHARED_BUFFER_ULENC_SOURCE_BASE = 0;
const size_t A2M_SHARED_BUFFER_ULENC_TARGET_BASE = A2M_SHARED_BUFFER_ULENC_SOURCE_BASE + A2M_SHARED_BUFFER_ULDL_DATA_SIZE;
const size_t A2M_SHARED_BUFFER_DLDEC_SOURCE_BASE = A2M_SHARED_BUFFER_ULENC_TARGET_BASE + A2M_SHARED_BUFFER_ULDL_DATA_SIZE;
const size_t A2M_SHARED_BUFFER_DLDEC_TARGET_BASE = A2M_SHARED_BUFFER_DLDEC_SOURCE_BASE + A2M_SHARED_BUFFER_ULDL_DATA_SIZE;

/* CCCI Message ID */
const uint32_t CCCI_MSG_A2M_BASE = 0x2F00;
const uint32_t CCCI_MSG_M2A_BASE = 0xAF00;

class SpeechMessengerInterface {
public:
    virtual ~SpeechMessengerInterface() {}
    virtual status_t    Initial() = 0;
    virtual status_t    Deinitial() = 0;
    virtual bool          A2MBufLock() = 0;
    virtual void          A2MBufUnLock() = 0;

    virtual status_t    WaitUntilModemReady() = 0;

    virtual ccci_buff_t InitCcciMailbox(uint16_t id, uint16_t param_16bit, uint32_t param_32bit) = 0;
    virtual status_t    SendMessageInQueue(ccci_buff_t ccci_buff) = 0;

    inline uint32_t     GetA2MShareBufLen() const { return mA2MShareBufLen; }
    inline char        *GetA2MShareBufBase() const { return mA2MShareBufBase; }

    inline uint8_t      SetShareBufHeader(uint16_t *ptr16, uint16_t sync, share_buff_data_type_t type, uint16_t length) const {
        ptr16[0] = sync;
        ptr16[1] = type;
        ptr16[2] = length;
        return CCCI_SHARE_BUFF_HEADER_LEN; // 3 * sizeof(uint16_t);
    }
    inline uint32_t     GetM2AShareBufLen() const { return mM2AShareBufLen; }
    inline char        *GetM2AShareBufBase() const { return mM2AShareBufBase; }
    virtual uint16_t    GetM2AShareBufSyncWord(const ccci_buff_t &ccci_buff) = 0;
    virtual uint16_t    GetM2AShareBufDataType(const ccci_buff_t &ccci_buff) = 0;
    virtual uint16_t    GetM2AShareBufDataLength(const ccci_buff_t &ccci_buff) = 0;

    inline bool     WasModemReset() const { return mWasModemReset;}
    inline void     ResetWasModemReset(bool val) {
        mWasModemReset = val;
        return;
    }

    /**
     * get modem side modem function status
     */
    virtual bool        GetModemSideModemStatus(const modem_status_mask_t modem_status_mask) const = 0;


    /**
     * check whether modem side get all necessary speech enhancement parameters here
     */
    virtual bool        CheckSpeechParamAckAllArrival() = 0;


    /**
     * check whether modem is ready. (if w/o SIM && phone_2 => modem sleep)
     */
    virtual bool        CheckModemIsReady() = 0;

    /**
     * check whether modem audio is ready from reset.
     */
    virtual bool        GetMDResetFlag() = 0;

    /**
     * set raw record type.
     */
    virtual status_t        setPcmRecordType(SpcRecordTypeStruct typeRecord) = 0;

#ifdef USE_CCCI_SHARE_BUFFER
    virtual void InitA2MRawParaRingBuffer() = 0;
    virtual status_t GetA2MRawParaRingBuffer(uint16_t *offset, uint16_t *avail) = 0;
    virtual status_t AdvanceA2MRawParaRingBuffer(int datalength) = 0;
    virtual status_t WriteA2MRawParaRingBuffer(char *data, int datalength) = 0;
#endif

protected:
    virtual char        GetModemCurrentStatus() = 0;

    virtual uint16_t    GetMessageID(const ccci_buff_t &ccci_buff) = 0;
    virtual uint16_t    GetMessageParam(const ccci_buff_t &ccci_buff) = 0;

    virtual uint16_t    GetMessageLength(const ccci_buff_t &ccci_buff) = 0;
    virtual uint16_t    GetMessageOffset(const ccci_buff_t &ccci_buff) = 0;
    virtual bool        CheckOffsetAndLength(const ccci_buff_t &ccci_buff) = 0;

    virtual ccci_message_ack_t JudgeAckOfMsg(const uint16_t message_id) = 0;

    virtual bool        IsModemFunctionOnOffMessage(const uint16_t message_id) = 0;

    virtual status_t    SendMessage(const ccci_buff_t &ccci_buff) = 0;
    virtual status_t    ReadMessage(ccci_buff_t &ccci_buff) = 0;
    virtual void        SendMsgFailErrorHandling(const ccci_buff_t &ccci_buff) = 0;

    virtual RingBuf     GetM2AUplinkRingBuffer(const ccci_buff_t &ccci_buff) = 0;

    virtual status_t    CreateReadingThread() = 0;
    virtual status_t    CreateSendSphParaThread() = 0;

    // for message queue
    virtual uint32_t    GetQueueCount() const = 0;
    virtual status_t    ConsumeMessageInQueue() = 0;
    virtual bool        MDReset_CheckMessageInQueue() = 0;

    virtual void        ResetSpeechParamAckCount() = 0;
    virtual void        AddSpeechParamAckCount(speech_param_ack_t type) = 0;


    /**
     * set/reset AP side modem function status
     */

    virtual void        SetModemSideModemStatus(const modem_status_mask_t modem_status_mask) = 0;
    virtual void        ResetModemSideModemStatus(const modem_status_mask_t modem_status_mask) = 0;

    // lock
    virtual bool SpeechParamLock() = 0;
    virtual void SpeechParamUnLock() = 0;

    // share buffer base and len
    uint32_t mA2MShareBufLen;
    uint32_t mM2AShareBufLen;

    char    *mA2MShareBufBase;
    char    *mM2AShareBufBase;

    char    *mA2MShareBufEnd;
    char    *mM2AShareBufEnd;

    RingBuf  mM2AShareBuf;
    bool mWasModemReset;
};


} // end namespace android

#endif // end of ANDROID_SPEECH_MESSAGE_INTERFACE_H
