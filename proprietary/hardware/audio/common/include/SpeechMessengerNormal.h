#ifndef ANDROID_SPEECH_MESSENGER_NORMAL_H
#define ANDROID_SPEECH_MESSENGER_NORMAL_H

#include <stdint.h>

#include <SpeechType.h>

#include <AudioLock.h>


namespace android {
/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct ccci_msg_t;
class SpeechShareMemBase;


/*
 * =============================================================================
 *                     class
 * =============================================================================
 */

class SpeechMessengerNormal {
public:
    SpeechMessengerNormal(const modem_index_t modem_index);
    virtual ~SpeechMessengerNormal();

    virtual bool checkModemReady();
    virtual bool checkModemAlive();


    virtual uint32_t getMaxApPayloadDataSize();
    virtual uint32_t getMaxMdPayloadDataSize();

    virtual int sendSpeechMessage(sph_msg_t *p_sph_msg);
    virtual int readSpeechMessage(sph_msg_t *p_sph_msg);

    virtual int resetShareMemoryIndex();


    virtual int writeSphParamToShareMemory(const void *p_sph_param,
                                           uint32_t sph_param_length,
                                           uint32_t *p_write_idx);


    virtual int writeApDataToShareMemory(const void *data_buf,
                                         uint16_t data_type,
                                         uint16_t data_size,
                                         uint16_t *p_payload_length,
                                         uint32_t *p_write_idx);


    virtual int readMdDataFromShareMemory(void *p_data_buf,
                                          uint16_t *p_data_type,
                                          uint16_t *p_data_size,
                                          uint16_t payload_length,
                                          uint32_t read_idx);
    virtual int getShareMemoryType();

protected:
    SpeechMessengerNormal() {}


    virtual int openCcciDriver();
    virtual int closeCcciDriver();

    virtual int openShareMemory();
    virtual int closeShareMemory();

    virtual int checkCcciStatusAndRecovery();

    virtual int formatShareMemory();
    static void *formatShareMemoryThread(void *arg);
    pthread_t   hFormatShareMemoryThread;

    virtual int speechMessageToCcciMessage(sph_msg_t *p_sph_msg, ccci_msg_t *p_ccci_msg);
    virtual int ccciMessageToSpeechMessage(ccci_msg_t *p_ccci_msg, sph_msg_t *p_sph_msg);


    modem_index_t mModemIndex;


    int mCcciDeviceHandler;
    int mCcciShareMemoryHandler;
    AudioLock    mCcciHandlerLock;
    AudioLock    mShareMemoryHandlerLock;

    ccci_msg_t *mCcciMsgSend;
    AudioLock    mCcciMsgSendLock;


    ccci_msg_t *mCcciMsgRead;
    AudioLock    mCcciMsgReadLock;

    SpeechShareMemBase *mSpeechShareMem;


};

} /* end namespace android */

#endif /* end of ANDROID_SPEECH_MESSENGER_NORMAL_H */

