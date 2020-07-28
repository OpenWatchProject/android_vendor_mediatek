#ifndef ANDROID_SPEECH_MESSAGE_QUEUE_H
#define ANDROID_SPEECH_MESSAGE_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

#include <pthread.h>

#include <AudioLock.h>


namespace android {

/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct sph_msg_t;


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

/* callback to send msg to modem */
typedef int (*send_message_wrapper_fp_t)(void *arg, sph_msg_t *p_sph_msg);

/* callback to msg error handle */
typedef int (*error_handle_message_wrapper_fp_t)(void *arg, sph_msg_t *p_sph_msg);


/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

class SpeechQueueElement;


class SpeechMessageQueue {
public:
    SpeechMessageQueue(
        send_message_wrapper_fp_t send_message_wrapper,
        error_handle_message_wrapper_fp_t error_handle_message_wrapper,
        void *wrapper_arg);
    virtual ~SpeechMessageQueue();

    int sendSpeechMessageToQueue(sph_msg_t *p_sph_msg,
                                 const uint32_t block_thread_ms);

    int sendSpeechMessageAckToQueue(sph_msg_t *p_sph_msg_ack);
    void notifyQueueToStopWaitingAck();
    inline void resetStopWaitAckFlag() { mStopWaitAck = false; }



private:
    SpeechMessageQueue() {}

    bool            checkQueueEmpty() const;
    bool            checkQueueToBeFull() const;
    bool            checkQueueIndexValid(const uint32_t index) const;

    uint32_t        getQueueSize() const;
    uint32_t        getQueueNumElements() const;

    int             pushElement(sph_msg_t *p_sph_msg, uint32_t *idx_msg);
    int             popElement();
    int             frontElement(sph_msg_t **pp_sph_msg, uint32_t *idx_msg);

    int             waitUntilElementProcessDone(const uint32_t idx_msg, const uint32_t ms);
    int             signalElementProcessDone(const uint32_t idx_msg);

    int             waitUntilModemAckBack(sph_msg_t *p_sph_msg, uint32_t idx_msg);


    int             getQueueElementUntilPushed(sph_msg_t **pp_sph_msg, uint32_t *idx_msg);

    bool            mEnableThread; /* not speech on/off but new/delete status */
    static void    *processElementThread(void *arg);
    pthread_t       hProcessElementThread;

    int             sendSpeechMessage(sph_msg_t *p_sph_msg);
    int             errorHandleSpeechMessage(sph_msg_t *p_sph_msg);


    AudioLock    mQueueLock;
    SpeechQueueElement *mQueue;
    uint32_t mQueueSize;
    uint32_t mQueueIndexRead;
    uint32_t mQueueIndexWrite;


    AudioLock    mWaitAckLock;

    bool mWaitAckFlag;
    bool mStopWaitAck;
    sph_msg_t *mSphMsgAck;


    send_message_wrapper_fp_t sendMessageWrapper;
    error_handle_message_wrapper_fp_t errorHandleMessageWrapper;
    void *mWrapperArg;
};



} /* end of namespace android */

#endif /* end of ANDROID_SPEECH_MESSAGE_QUEUE_H */

