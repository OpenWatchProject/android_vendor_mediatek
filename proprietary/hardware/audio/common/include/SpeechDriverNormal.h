#ifndef ANDROID_SPEECH_DRIVER_NORMAL_H
#define ANDROID_SPEECH_DRIVER_NORMAL_H

#include "SpeechDriverInterface.h"

#include <pthread.h>

#include <AudioLock.h>

#include "SpeechType.h"
#include "SpeechParserType.h"

#if defined(MTK_AUDIODSP_SUPPORT)
struct ipi_msg_t;
struct aurisys_dsp_config_t;
struct aurisys_lib_manager_t;
struct arsi_task_config_t;
#endif

namespace android {

/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

class SpeechMessageQueue;
class SpeechMessengerNormal;


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */


/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class SpeechDriverNormal : public SpeechDriverInterface {
public:
    /** virtual dtor */
    virtual ~SpeechDriverNormal();

    /** singleton */
    static SpeechDriverNormal *GetInstance(modem_index_t modem_index);


    /** speech */
    virtual status_t SetModemSideSamplingRate(uint16_t sample_rate);
    virtual status_t switchBTMode(uint32_t sample_rate);
    virtual void     setBTMode(const int mode);
    virtual void     setBtSpkDevice(const bool flag);
    virtual status_t SetSpeechMode(const audio_devices_t input_device, const audio_devices_t output_device);
    virtual status_t SpeechOn();
    virtual status_t SpeechOff();
    virtual status_t VideoTelephonyOn();
    virtual status_t VideoTelephonyOff();
    virtual status_t SpeechRouterOn();
    virtual status_t SpeechRouterOff();
    virtual status_t setMDVolumeIndex(int stream, int device, int index);


    /** record */
    virtual status_t recordOn() { return -ENOSYS; }
    virtual status_t recordOff() { return -ENOSYS; }

    virtual status_t VoiceMemoRecordOn();
    virtual status_t VoiceMemoRecordOff();

    virtual uint16_t GetRecordSampleRate() const;
    virtual uint16_t GetRecordChannelNumber() const;

    virtual status_t recordOn(SpcRecordTypeStruct typeRecord);
    virtual status_t recordOff(SpcRecordTypeStruct typeRecord);

    virtual status_t setPcmRecordType(SpcRecordTypeStruct typeRecord);


    /** background sound */
    virtual status_t BGSoundOn();
    virtual status_t BGSoundConfig(uint8_t ul_gain, uint8_t dl_gain);
    virtual status_t BGSoundOff();

    /** voip rx on/off */
    virtual status_t VoipRxOn();
    virtual status_t VoipRxOff();
    virtual status_t VoipRxConfig(const uint8_t direction, const uint8_t mixType);

    /** telephony tx */
    virtual status_t TelephonyTxOn();
    virtual status_t TelephonyTxConfig(uint8_t ul_gain, uint8_t dl_gain);
    virtual status_t TelephonyTxOff();

    /** pcm 2 way */
    virtual status_t PCM2WayOn(const bool wideband_on);
    virtual status_t PCM2WayOff();


    /** tty ctm */
    virtual status_t TtyCtmOn();
    virtual status_t TtyCtmOff();
    virtual status_t TtyCtmDebugOn(bool tty_debug_flag);

    /** rtt */
    virtual int RttConfig(int rttMode);

    /** acoustic loopback */
    virtual status_t SetAcousticLoopback(bool loopback_on);
    virtual status_t SetAcousticLoopbackBtCodec(bool enable_codec);

    virtual status_t SetAcousticLoopbackDelayFrames(int32_t delay_frames);
    virtual status_t setLpbkFlag(bool enableLpbk __unused) { return -ENOSYS; }

    /**
     * Modem Audio DVT and Debug
     */
    virtual status_t SetModemLoopbackPoint(uint16_t loopback_point);


    /** volume */
    virtual status_t SetDownlinkGain(int16_t gain);
    virtual status_t SetEnh1DownlinkGain(int16_t gain);
    virtual status_t SetUplinkGain(int16_t gain);
    virtual status_t SetDownlinkMute(bool mute_on);
    virtual status_t SetUplinkMute(bool mute_on);
    virtual status_t SetUplinkSourceMute(bool mute_on);
    virtual status_t SetSidetoneGain(int16_t gain __unused) { return -ENOSYS; }
    virtual status_t SetDSPSidetoneFilter(const bool dsp_stf_on __unused) { return -ENOSYS; }
    virtual status_t SetDownlinkMuteCodec(bool mute_on);


    /** speech enhancement */
    virtual status_t SetSpeechEnhancement(bool enhance_on);
    virtual status_t SetSpeechEnhancementMask(const sph_enh_mask_struct_t &mask);
    virtual uint16_t speechEnhancementMaskWrapper(const uint32_t enh_dynamic_mask);

    virtual status_t SetBtHeadsetNrecOn(const bool bt_headset_nrec_on);


    /** speech enhancement parameters setting */
    virtual int updateSpeechParam(const SpeechScenario scenario);
    virtual int updateSpeechFeature(const SpeechFeatureType featureType, const bool flagOn);
    virtual int setParameter(const char *keyParameter);
    virtual const char *getParameter(const char *keyParameter);

    virtual status_t GetVibSpkParam(void *eVibSpkParam);
    virtual status_t SetVibSpkParam(void *eVibSpkParam);

    virtual status_t GetSmartpaParam(void *eParamSmartpa);
    virtual status_t SetSmartpaParam(void *eParamSmartpa);


    /** check whether modem is ready */
    virtual bool     CheckModemIsReady();
#if defined(MTK_AUDIODSP_SUPPORT)
    void SetArsiTaskConfigSmartpa(struct arsi_task_config_t *pTaskConfig);
    void SetArsiAttributeSmartpa();
    void CreateAurisysLibManagerSmartpa();
    void DestroyAurisysLibManagerSmartpa();
    void UpdateAurisysConfigSmartpa();
#endif

    virtual void setNetworkChangeCallback(void *arg, nwRateChangeCbFunc callback);

protected:
    /** hide ctor */
    SpeechDriverNormal() {}
    SpeechDriverNormal(modem_index_t modem_index);


    /** recover status (speech/record/bgs/vt/p2w/tty) */
    virtual void RecoverModemSideStatusToInitState();

    /** delay time */
    virtual int  getBtDelayTime(uint16_t *p_bt_delay_ms);
    virtual int  getUsbDelayTime(uint8_t *usbDelayMs);
    virtual int  getDriverParam(uint8_t paramType, void *paramBuf);


    virtual int configSpeechInfo(sph_info_t *p_sph_info);

    virtual int configMailBox(
        sph_msg_t *p_sph_msg,
        uint16_t msg_id,
        uint16_t param_16bit,
        uint32_t param_32bit);


    virtual int configPayload(
        sph_msg_t *p_sph_msg,
        uint16_t msg_id,
        uint16_t data_type,
        void    *data_buf,
        uint16_t data_size);

    virtual int sendSpeechMessageToQueue(sph_msg_t *p_sph_msg);

    virtual int sendSpeechMessageAckToQueue(sph_msg_t *p_sph_msg);

    static int sendSpeechMessageToModemWrapper(void *arg, sph_msg_t *p_sph_msg);
    virtual int sendSpeechMessageToModem(sph_msg_t *p_sph_msg);

    static int errorHandleSpeechMessageWrapper(void *arg, sph_msg_t *p_sph_msg);
    virtual int errorHandleSpeechMessage(sph_msg_t *p_sph_msg);


    virtual int sendMailbox(sph_msg_t *p_sph_msg,
                            uint16_t msg_id,
                            uint16_t param_16bit,
                            uint32_t param_32bit);

    virtual int sendPayload(sph_msg_t *p_sph_msg,
                            uint16_t msg_id,
                            uint16_t data_type,
                            void    *data_buf,
                            uint16_t data_size);


    virtual int readSpeechMessageFromModem(sph_msg_t *p_sph_msg);


    AudioLock    mReadMessageLock;


    /* not speech on/off but new/delete */
    virtual void createThreads();
    virtual void joinThreads();
    bool mEnableThread;

    static void *readSpeechMessageThread(void *arg);
    pthread_t hReadSpeechMessageThread;
    bool mReadMsgThreadCreated;
    AudioLock mReadMsgThreadCreatedLock;


    /* speech on/off */
    virtual void createThreadsDuringSpeech();
    virtual void joinThreadsDuringSpeech();
    bool mEnableThreadDuringSpeech;

    static void *modemStatusMonitorThread(void *arg);
    pthread_t hModemStatusMonitorThread;
    AudioLock    mModemStatusMonitorThreadLock;


    virtual int parseRawRecordPcmBuffer(void *raw_buf, void *parsed_buf, uint16_t *p_data_size);

    virtual int processModemMessage(sph_msg_t *p_sph_msg);
    virtual int processModemAckMessage(sph_msg_t *p_sph_msg);
    virtual int processModemControlMessage(sph_msg_t *p_sph_msg);
    virtual int processModemDataMessage(sph_msg_t *p_sph_msg);

    virtual void processModemEPOF();
    virtual void processModemAlive(sph_msg_t *sphMsg);
    virtual void processNetworkCodecInfo(sph_msg_t *p_sph_msg);


    SpeechMessengerNormal *mSpeechMessenger;


    uint8_t  mSampleRateEnum;


    SpeechMessageQueue *mSpeechMessageQueue;
    AudioLock           mWaitAckLock;


    bool getModemSideModemStatus(const modem_status_mask_t modem_status_mask) const;
    void setModemSideModemStatus(const modem_status_mask_t modem_status_mask);
    void resetModemSideModemStatus(const modem_status_mask_t modem_status_mask);
    void cleanAllModemSideModemStatus();


    virtual int parseSpeechParam(const SpeechScenario scenario);
    virtual int writeAllSpeechParametersToModem(uint32_t *p_length, uint32_t *p_index);


    uint32_t mModemSideModemStatus; // value |= modem_status_mask_t
    AudioLock    mModemSideModemStatusLock;


    virtual int SpeechOnByApplication(const uint8_t application);
    virtual int SpeechOffByApplication(const uint8_t application);
    bool isSpeechApplicationOn() { return (mApplication != SPH_APPLICATION_INVALID); }

    uint32_t kMaxApPayloadDataSize;
    uint32_t kMaxMdPayloadDataSize;

    void *mBgsBuf;
    void *mVmRecBuf;
    void *mRawRecBuf;
    void *mParsedRecBuf;
    void *mP2WUlBuf;
    void *mP2WDlBuf;
    void *mTtyDebugBuf;
    void *mVoipRxUlBuf;
    void *mVoipRxDlBuf;
    void *mTelephonyTxBuf;

    uint8_t mApplication;
    speech_mode_t mSpeechMode;
    audio_devices_t mInputDevice;
    audio_devices_t mOutputDevice;
    bool isBtSpkDevice;
    int mBTMode; // BT mode, 0:NB, 1:WB

#if defined(MTK_AUDIODSP_SUPPORT)
    struct aurisys_dsp_config_t *mAurisysDspConfigSmartpa;
    struct aurisys_lib_manager_t *mAurisysLibManagerSmartpa;
    static void processDmaMsgSmartpa(struct ipi_msg_t *msg, void *buf, uint32_t size, void *arg);
#endif

    AudioLock mRecordTypeLock;
    AudioLock    mSpeechParamLock;
    AudioLock mFeatureMaskLock;

    bool mTtyDebugEnable;

    bool mApResetDuringSpeech;
    bool mModemResetDuringSpeech;
    bool mModemDead;
    AudioLock mModemDeadLock;


    /* loopback delay frames (1 frame = 20 ms) */
    uint8_t mModemLoopbackDelayFrames;

    /* RTT Mode */
    int mRttMode;

    /* to handle the case that ap crash before get ack */
    virtual void waitModemAckAfterApDie();
    bool mNeedWaitModemAckAfterApDie;
    AudioLock mWaitModemAckAfterApDieLock;


    /* keep ap need-ack control msg after send to md */
    void setApWaitAckMsgID(sph_msg_t *p_sph_msg);
    void resetApWaitAckMsgID();
    uint16_t mApWaitAckMsgID;
    AudioLock mApWaitAckMsgIDLock;

    /* Speech Parser */
    SpeechDataBufType mSpeechParam;
    SpeechParserAttribute mSpeechParserAttribute;
    MdAliveInfo mMdAliveInfo;
    bool mIsParseFail;
    bool mIsUsePreviousParam;
    uint8_t mVoipRxTypeDl;
    uint8_t mVoipRxTypeUl;

    /* BT config */
    bool mIsBTSwitchConfig;

private:
    /** singleton */
    static SpeechDriverNormal *mSpeechDriver;

    int updateFeatureMask(const SpeechFeatureType featureType, const bool flagOn);
    nwRateChangeCbFunc mNetworkChangeCallback;
    void *mNetworkChangeCallbackArg;
};

} /* end of namespace android */

#endif /* end of ANDROID_SPEECH_DRIVER_NORMAL_H */

