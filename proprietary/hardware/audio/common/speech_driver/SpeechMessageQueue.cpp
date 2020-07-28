#include <SpeechMessageQueue.h>

#include <errno.h>

#include <pthread.h>

#include <utils/threads.h> // for ANDROID_PRIORITY_AUDIO

#include <cutils/properties.h> /* for PROPERTY_KEY_MAX */


#include <SpeechType.h>

#include <SpeechUtility.h>

#include <AudioLock.h>

#include <AudioAssert.h>

#include <SpeechMessageID.h>



#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechMessageQueue"



namespace android {

/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define MIN_SPH_MSG_QUEUE_SIZE (32)
#define MAX_SPH_MSG_QUEUE_SIZE (64)

#define MAX_SPEECH_QUEUE_AUTO_LOCK_TIMEOUT_MS (2000)
#define MAX_SPEECH_QUEUE_WAIT_ELEMENT_LOCK_TIMEOUT_MS (2000)

#define MAX_SPEECH_QUEUE_WAIT_ACK_TIMEOUT_MS (60000)

static const char *kPropertyKeyLowRam = "ro.vendor.config.low_ram";


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

class SpeechQueueElement {
public:
    SpeechQueueElement() {
        p_sph_msg_client = NULL;
        memset(&sph_msg, 0, sizeof(sph_msg_t));
        wait_in_thread = false;
        signal_arrival = false;
        send_msg_to_modem_retval = 0;
    }
    virtual ~SpeechQueueElement() { }

    sph_msg_t *p_sph_msg_client;    /* the address of client's message */
    sph_msg_t sph_msg;              /* the copy msg in queue */
    AudioLock    mElementLock;      /* wait/signal */
    bool wait_in_thread;
    bool signal_arrival;
    int send_msg_to_modem_retval;
};


/*
 * =============================================================================
 *                     create/destroy/init/deinit functions
 * =============================================================================
 */

SpeechMessageQueue::SpeechMessageQueue(
    send_message_wrapper_fp_t send_message_wrapper,
    error_handle_message_wrapper_fp_t error_handle_message_wrapper,
    void *wrapper_arg) {

    int ret = 0;
    char lowRam[PROPERTY_VALUE_MAX] = {0};

    /* init var */
    get_string_from_property(kPropertyKeyLowRam, lowRam, PROPERTY_VALUE_MAX);
    if (strcmp(lowRam, "true") == 0) { /* low memory project */
        mQueueSize = MIN_SPH_MSG_QUEUE_SIZE;
    } else {
        mQueueSize = MAX_SPH_MSG_QUEUE_SIZE;
    }
    mQueue = new SpeechQueueElement[mQueueSize];
    mQueueIndexRead = 0;
    mQueueIndexWrite = 0;

    mWaitAckFlag = false;
    mStopWaitAck = false;

    mSphMsgAck = NULL;
    AUDIO_ALLOC_STRUCT(sph_msg_t, mSphMsgAck);

    /* callback to send msg to modem */
    sendMessageWrapper = send_message_wrapper;
    errorHandleMessageWrapper = error_handle_message_wrapper;
    mWrapperArg = wrapper_arg;

    mEnableThread = true;
    hProcessElementThread = 0;
    ret = pthread_create(&hProcessElementThread, NULL,
                         SpeechMessageQueue::processElementThread,
                         (void *)this);
    ASSERT(ret == 0);

}


SpeechMessageQueue::~SpeechMessageQueue() {
    mEnableThread = false;
    pthread_join(hProcessElementThread, NULL);
    ALOGD("pthread_join hProcessElementThread done");

    /* init var */
    if (mQueue) {
        delete mQueue;
    }

    AUDIO_FREE(mSphMsgAck);
}


bool SpeechMessageQueue::checkQueueEmpty() const {
    return (mQueueIndexRead == mQueueIndexWrite);
}


bool SpeechMessageQueue::checkQueueToBeFull() const {
    uint8_t idx_w_to_be = 0;

    idx_w_to_be = mQueueIndexWrite + 1;
    if (idx_w_to_be == mQueueSize) {
        idx_w_to_be = 0;
    }

    return (idx_w_to_be == mQueueIndexRead) ? true : false;
}


bool SpeechMessageQueue::checkQueueIndexValid(const uint32_t index) const {
    return (index < mQueueSize) ? true : false;
}


uint32_t SpeechMessageQueue::getQueueSize() const {
    return mQueueSize;
}


uint32_t SpeechMessageQueue::getQueueNumElements() const {
    return (mQueueIndexWrite >= mQueueIndexRead) ?
           (mQueueIndexWrite - mQueueIndexRead) :
           ((mQueueSize - mQueueIndexRead) + mQueueIndexWrite);
}


int SpeechMessageQueue::pushElement(sph_msg_t *p_sph_msg, uint32_t *idx_msg) {
    /* error handling */
    if (p_sph_msg == NULL || idx_msg == NULL) {
        ALOGE("%s(), NULL!! p_sph_msg: %p, idx_msg: %p", __FUNCTION__,
              p_sph_msg, idx_msg);
        return -EFAULT;
    }


    *idx_msg = 0xFFFFFFFF;
    AL_AUTOLOCK_MS(mQueueLock, MAX_SPEECH_QUEUE_AUTO_LOCK_TIMEOUT_MS);

    /* check mQueue not full */
    if (checkQueueToBeFull() == true) {
        ALOGW("%s(), Queue FULL!! mQueueIndexRead: %u, mQueueIndexWrite: %u", __FUNCTION__, mQueueIndexRead, mQueueIndexWrite);
        PRINT_SPH_MSG(ALOGE, "Queue FULL!! drop msg", p_sph_msg);
        return -EOVERFLOW;
    }

    /* push */
    AL_LOCK_MS(mQueue[mQueueIndexWrite].mElementLock, MAX_SPEECH_QUEUE_AUTO_LOCK_TIMEOUT_MS);
    mQueue[mQueueIndexWrite].p_sph_msg_client = p_sph_msg;
    memcpy(&mQueue[mQueueIndexWrite].sph_msg, p_sph_msg, sizeof(sph_msg_t));
    mQueue[mQueueIndexWrite].wait_in_thread = true;
    mQueue[mQueueIndexWrite].signal_arrival = false;
    mQueue[mQueueIndexWrite].send_msg_to_modem_retval = 0;

    /* memory copy speech info buffer */
    if (p_sph_msg->buffer_type == SPH_MSG_BUFFER_TYPE_PAYLOAD) {
        ALOGV("%s(), memory copy speech info buffer, msg_id:0x%x", __FUNCTION__, p_sph_msg->msg_id);
        mQueue[mQueueIndexWrite].sph_msg.payload_data_addr = NULL;
        ASSERT(p_sph_msg->payload_data_size > 0);
        AUDIO_ALLOC_BUFFER(mQueue[mQueueIndexWrite].sph_msg.payload_data_addr,
                           p_sph_msg->payload_data_size);
        memcpy(mQueue[mQueueIndexWrite].sph_msg.payload_data_addr,
               p_sph_msg->payload_data_addr, p_sph_msg->payload_data_size);
    }

    AL_UNLOCK(mQueue[mQueueIndexWrite].mElementLock);

    *idx_msg = mQueueIndexWrite;
    mQueueIndexWrite++;
    if (mQueueIndexWrite == mQueueSize) {
        mQueueIndexWrite = 0;
    }
    AL_SIGNAL(mQueueLock);

    SPH_LOG_T("%s(), push msg: 0x%x, read_idx: %u, write_idx: %u, queue(%u/%u), idx_msg: %u",
              __FUNCTION__,
              mQueue[*idx_msg].sph_msg.msg_id,
              mQueueIndexRead, mQueueIndexWrite,
              getQueueNumElements(), getQueueSize(), *idx_msg);
    return 0;
}


int SpeechMessageQueue::popElement() {
    uint16_t msg_id = 0;

    AL_AUTOLOCK_MS(mQueueLock, MAX_SPEECH_QUEUE_AUTO_LOCK_TIMEOUT_MS);

    /* check mQueue not empty */
    if (checkQueueEmpty() == true) {
        ALOGW("%s(), Queue EMPTY!! mQueueIndexRead: %u, mQueueIndexWrite: %u", __FUNCTION__, mQueueIndexRead, mQueueIndexWrite);
        return -ENOMEM;
    }

    /* get msg_id for debug log */
    msg_id = mQueue[mQueueIndexRead].sph_msg.msg_id;

    /* free speech info buffer*/
    if (mQueue[mQueueIndexRead].sph_msg.buffer_type == SPH_MSG_BUFFER_TYPE_PAYLOAD) {
        ALOGV("%s(), free speech info buffer of Queue element, msg_id:0x%x",
              __FUNCTION__, msg_id);
        AUDIO_FREE_POINTER(mQueue[mQueueIndexRead].sph_msg.payload_data_addr);
    }

    /* pop */
    mQueueIndexRead++;
    if (mQueueIndexRead == mQueueSize) {
        mQueueIndexRead = 0;
    }

    SPH_LOG_T("%s(), pop msg:  0x%x, read_idx: %u, write_idx: %u, queue(%u/%u)",
              __FUNCTION__,
              msg_id, mQueueIndexRead, mQueueIndexWrite,
              getQueueNumElements(), getQueueSize());
    return 0;
}


int SpeechMessageQueue::frontElement(sph_msg_t **pp_sph_msg, uint32_t *idx_msg) {
    ASSERT(AL_TRYLOCK(mQueueLock) != 0);

    /* error handling */
    if (pp_sph_msg == NULL) {
        ALOGE("%s(), pp_sph_msg = NULL, return", __FUNCTION__);
        return -EFAULT;
    }
    *pp_sph_msg = NULL;

    if (idx_msg == NULL) {
        ALOGE("%s(), idx_msg = NULL, return", __FUNCTION__);
        return -EFAULT;
    }
    *idx_msg = 0xFFFFFFFF;

    /* check mQueue empty */
    if (checkQueueEmpty() == true) {
        ALOGW("%s(), Queue EMPTY!! mQueueIndexRead: %u, mQueueIndexWrite: %u", __FUNCTION__, mQueueIndexRead, mQueueIndexWrite);
        return -ENOMEM;
    }

    *pp_sph_msg = &mQueue[mQueueIndexRead].sph_msg;
    *idx_msg = mQueueIndexRead;

    return 0;
}


int SpeechMessageQueue::waitUntilElementProcessDone(const uint32_t idx_msg, const uint32_t ms) {
    int retval = 0;

    if (checkQueueIndexValid(idx_msg) == false) {
        ALOGW("%s(), idx_msg %d is invalid!! return", __FUNCTION__, idx_msg);
        return -EOVERFLOW;
    }

    if (ms == 0) {
        AL_LOCK_MS(mQueue[idx_msg].mElementLock, MAX_SPEECH_QUEUE_WAIT_ELEMENT_LOCK_TIMEOUT_MS);
        mQueue[idx_msg].wait_in_thread = false;
        AL_UNLOCK(mQueue[idx_msg].mElementLock);
        return 0;
    }


    CLEANUP_PUSH_ALOCK(mQueue[idx_msg].mElementLock.getAlock());
    AL_LOCK_MS(mQueue[idx_msg].mElementLock, MAX_SPEECH_QUEUE_WAIT_ELEMENT_LOCK_TIMEOUT_MS);

    if (mQueue[idx_msg].signal_arrival == false) {
        retval = AL_WAIT_MS(mQueue[idx_msg].mElementLock, ms);
    }
    mQueue[idx_msg].wait_in_thread = false;

    if (retval == 0) { /* retval of SpeechDriverNormal::sendSpeechMessageToModem() */
        retval = mQueue[idx_msg].send_msg_to_modem_retval;
    }

    AL_UNLOCK(mQueue[idx_msg].mElementLock);
    CLEANUP_POP_ALOCK(mQueue[idx_msg].mElementLock.getAlock());

    return retval;
}


int SpeechMessageQueue::signalElementProcessDone(const uint32_t idx_msg) {
    int retval = 0;

    if (checkQueueIndexValid(idx_msg) == false) {
        ALOGW("%s(), idx_msg %d is invalid!! return", __FUNCTION__, idx_msg);
        return -EOVERFLOW;
    }

    AL_LOCK_MS(mQueue[idx_msg].mElementLock, MAX_SPEECH_QUEUE_WAIT_ELEMENT_LOCK_TIMEOUT_MS);
    mQueue[idx_msg].signal_arrival = true;
    AL_SIGNAL(mQueue[idx_msg].mElementLock); /* => waitUntilElementProcessDone */
    AL_UNLOCK(mQueue[idx_msg].mElementLock);
    return retval;
}


int SpeechMessageQueue::getQueueElementUntilPushed(sph_msg_t **pp_sph_msg, uint32_t *idx_msg) {
    int retval = 0;

    CLEANUP_PUSH_ALOCK(mQueueLock.getAlock());
    AL_LOCK_MS(mQueueLock, MAX_SPEECH_QUEUE_WAIT_ELEMENT_LOCK_TIMEOUT_MS);
    if (checkQueueEmpty() == true) {
        AL_WAIT_NO_TIMEOUT(mQueueLock);
    }
    retval = frontElement(pp_sph_msg, idx_msg); /* not p_sph_msg_client!!! */
    AL_UNLOCK(mQueueLock);
    CLEANUP_POP_ALOCK(mQueueLock.getAlock());

    return retval;
}


int SpeechMessageQueue::sendSpeechMessageToQueue(
    sph_msg_t *p_sph_msg,
    const uint32_t block_thread_ms) {

    uint32_t idx_msg = 0xFFFFFFFF;
    uint32_t idx_msg_head = 0xFFFFFFFF;
    int retval = 0;

    /* error handling */
    if (p_sph_msg == NULL) {
        ALOGE("%s(), p_sph_msg = NULL, return", __FUNCTION__);
        return -EFAULT;
    }

    /* push message to mQueue */
    retval = pushElement(p_sph_msg, &idx_msg);
    if (retval != 0) {
        ALOGW("%s(), pushElement fail!! return", __FUNCTION__);
        PRINT_SPH_MSG(ALOGE, "pushElement fail!! drop msg", p_sph_msg);
        WARNING("pushElement fail");
        return retval;
    }

    if (checkQueueIndexValid(idx_msg) == false) {
        ALOGW("%s(), idx_msg %u invalid!! return", __FUNCTION__, idx_msg);
        PRINT_SPH_MSG(ALOGE, "idx_msg invalid!!drop msg", p_sph_msg);
        WARNING("checkQueueIndexValid fail");
        return -EOVERFLOW;
    }


    /* wait until message processed */
    retval = waitUntilElementProcessDone(idx_msg, block_thread_ms);
    if (retval == -ETIMEDOUT) {
        idx_msg_head = mQueueIndexRead;
        PRINT_SPH_MSG(ALOGW, "block thread timeout", p_sph_msg);
        PRINT_SPH_MSG(ALOGW, "=> queue head info", &mQueue[idx_msg_head].sph_msg);
#if 0
        WARNING("block queue timeout");
#endif
    }

    return retval;
}


int SpeechMessageQueue::sendSpeechMessageAckToQueue(sph_msg_t *p_sph_msg_ack) {
    /* error handling */
    if (p_sph_msg_ack == NULL) {
        ALOGE("%s(), p_sph_msg_ack = NULL, return", __FUNCTION__);
        return -EFAULT;
    }

    if (isMdAckBack(p_sph_msg_ack) == false) {
        ALOGE("%s(), p_sph_msg_ack: 0x%x is not ack, return", __FUNCTION__, p_sph_msg_ack->msg_id);
        return -EINVAL;
    }

#ifdef CONFIG_MT_ENG_BUILD
    PRINT_SPH_MSG(ALOGD, "ack back", p_sph_msg_ack);
#else
    /* reduce log */
    if (p_sph_msg_ack->msg_id != MSG_M2A_MUTE_SPH_UL_ACK &&
        p_sph_msg_ack->msg_id != MSG_M2A_MUTE_SPH_DL_ACK &&
        p_sph_msg_ack->msg_id != MSG_M2A_MUTE_SPH_UL_SOURCE_ACK) {
        PRINT_SPH_MSG(ALOGD, "ack back", p_sph_msg_ack);
    }
#endif


    /* get msg ack & wake up queue */
    AL_LOCK_MS(mWaitAckLock, MAX_SPEECH_QUEUE_AUTO_LOCK_TIMEOUT_MS);
    if (mSphMsgAck->msg_id != 0) {
        ALOGE("%s(), p_sph_msg_ack: 0x%x, mSphMsgAck msg_id: 0x%x != 0", __FUNCTION__,
              p_sph_msg_ack->msg_id, mSphMsgAck->msg_id);
    }
    memcpy(mSphMsgAck, p_sph_msg_ack, sizeof(sph_msg_t));
    AL_SIGNAL(mWaitAckLock);
    AL_UNLOCK(mWaitAckLock);
    return 0;
}


void SpeechMessageQueue::notifyQueueToStopWaitingAck() {
    AL_LOCK_MS(mWaitAckLock, MAX_SPEECH_QUEUE_AUTO_LOCK_TIMEOUT_MS);
    if (mWaitAckFlag == true) { /* someone is waiting */
        ALOGW("%s(), stop waiting ack", __FUNCTION__);
        mWaitAckFlag = false;
        AL_SIGNAL(mWaitAckLock); /* notify to stop waiting ack */
    } else {
        /* no msg is waiting, set flag to prevent msg wait */
        ALOGW("%s(), no msg is waiting ack, set mStopWaitAck = true", __FUNCTION__);
        mStopWaitAck = true;
    }
    AL_UNLOCK(mWaitAckLock);
}


int SpeechMessageQueue::waitUntilModemAckBack(sph_msg_t *p_sph_msg, uint32_t idx_msg) {
    int retval = 0;
    sph_msg_t *p_sph_msg_client = NULL;

    /* error handling */
    if (p_sph_msg == NULL) {
        ALOGE("%s(), p_sph_msg = NULL, return", __FUNCTION__);
        return -EFAULT;
    }

    if (checkQueueIndexValid(idx_msg) == false) {
        ALOGW("%s(), idx_msg %d is invalid!! return", __FUNCTION__, idx_msg);
        return -EOVERFLOW;
    }


    CLEANUP_PUSH_ALOCK(mWaitAckLock.getAlock());
    AL_LOCK_MS(mWaitAckLock, MAX_SPEECH_QUEUE_AUTO_LOCK_TIMEOUT_MS);

    /* should no ack before wait */
    if (mSphMsgAck->msg_id != 0) {
        if (isAckMessageInPair(p_sph_msg, mSphMsgAck) == false) {
            ALOGE("%s(), p_sph_msg: 0x%x, mSphMsgAck msg_id: 0x%x != 0!! drop previous ack", __FUNCTION__,
                  p_sph_msg->msg_id, mSphMsgAck->msg_id);
            memset(mSphMsgAck, 0, sizeof(sph_msg_t));
        }
    }

    /* wait ack if need */
    if (mSphMsgAck->msg_id == 0) {
        if (mStopWaitAck) {
            PRINT_SPH_MSG(ALOGW, "MD status error, no need to wait ack!!", p_sph_msg);
            mStopWaitAck = false;
            retval = -ECANCELED;
            goto WAIT_ACK_DONE;
        }
        PRINT_SPH_MSG(ALOGV, "wait(+)", p_sph_msg);
        mWaitAckFlag = true;
        retval = AL_WAIT_MS(mWaitAckLock, MAX_SPEECH_QUEUE_WAIT_ACK_TIMEOUT_MS);
        PRINT_SPH_MSG(ALOGV, "wait(-)", p_sph_msg);

        if (mWaitAckFlag == false) { /* canceled */
            PRINT_SPH_MSG(ALOGW, "wait ack canceled!!", p_sph_msg);
            retval = -ECANCELED;
            goto WAIT_ACK_DONE;
        }

        if (retval != 0) { /* timeout */
            PRINT_SPH_MSG(ALOGE, "wait ack timeout!!", p_sph_msg);
            retval = -ETIMEDOUT;
            goto WAIT_ACK_DONE;
        }
    }

    /* should be in pair after wait */
    if (isAckMessageInPair(p_sph_msg, mSphMsgAck) == false) {
        ALOGE("%s(), msg_id: 0x%x, ack msg id: 0x%x not in pair!!", __FUNCTION__,
              p_sph_msg->msg_id, mSphMsgAck->msg_id);
        WARNING("ack not in pair");
        retval = -EINVAL;
        goto WAIT_ACK_DONE;
    }

    /* copy return mailbox value to original thread */
    AL_LOCK_MS(mQueue[idx_msg].mElementLock, MAX_SPEECH_QUEUE_WAIT_ELEMENT_LOCK_TIMEOUT_MS);
    if (mQueue[idx_msg].wait_in_thread == true) {
        p_sph_msg_client = mQueue[idx_msg].p_sph_msg_client;
        if (p_sph_msg_client->buffer_type == SPH_MSG_BUFFER_TYPE_MAILBOX &&
            mSphMsgAck->buffer_type == SPH_MSG_BUFFER_TYPE_MAILBOX) {
            p_sph_msg_client->param_16bit = mSphMsgAck->param_16bit;
            p_sph_msg_client->param_32bit = mSphMsgAck->param_32bit;
        }
    }
    AL_UNLOCK(mQueue[idx_msg].mElementLock);


WAIT_ACK_DONE:
    mWaitAckFlag = false;
    memset(mSphMsgAck, 0, sizeof(sph_msg_t));
    AL_UNLOCK(mWaitAckLock);
    CLEANUP_POP_ALOCK(mWaitAckLock.getAlock());

    if (retval == -ETIMEDOUT) {
        WARNING("wait ack timeout");
    }
    return retval;
}



void *SpeechMessageQueue::processElementThread(void *arg) {
    char thread_name[128];

    SpeechMessageQueue *pSpeechMessageQueue = NULL;

    sph_msg_t *p_sph_msg = NULL;
    uint32_t idx_msg = 0xFFFFFFFF;

    int retval = 0;


    CONFIG_THREAD(thread_name, ANDROID_PRIORITY_AUDIO);

    pSpeechMessageQueue = static_cast<SpeechMessageQueue *>(arg);
    if (pSpeechMessageQueue == NULL) {
        ALOGE("%s(), NULL!! pSpeechMessageQueue %p", __FUNCTION__, pSpeechMessageQueue);
        goto PROCESS_ELEMENT_THREAD_DONE;
    }


    while (pSpeechMessageQueue->mEnableThread == true) {
        /* wait until element pushed */
        retval = pSpeechMessageQueue->getQueueElementUntilPushed(&p_sph_msg, &idx_msg);
        if (retval != 0 ||
            p_sph_msg == NULL ||
            pSpeechMessageQueue->checkQueueIndexValid(idx_msg) == false) {
            ALOGW("%s(), getQueueElementUntilPushed fail!!", __FUNCTION__);
            usleep(5 * 1000);
            continue;
        }

        /* send to modem */
        retval = pSpeechMessageQueue->sendSpeechMessage(p_sph_msg);

        /* need ack, wait */
        if (isApNeedAck(p_sph_msg) == true) {
            if (retval != 0) {
                PRINT_SPH_MSG(ALOGE, "send fail, don't wait ack", p_sph_msg);
            } else {
                retval = pSpeechMessageQueue->waitUntilModemAckBack(p_sph_msg, idx_msg);
            }
        }

        /* error handing for send/wait_ack fail */
        if (retval != 0) {
            pSpeechMessageQueue->errorHandleSpeechMessage(p_sph_msg);
        }

        /* signal */
        pSpeechMessageQueue->mQueue[idx_msg].send_msg_to_modem_retval = retval;
        pSpeechMessageQueue->signalElementProcessDone(idx_msg);

        /* pop message from mQueue */
        pSpeechMessageQueue->popElement();
    }


PROCESS_ELEMENT_THREAD_DONE:
    ALOGV("%s terminated", thread_name);
    pthread_exit(NULL);
    return NULL;
}


int SpeechMessageQueue::sendSpeechMessage(sph_msg_t *p_sph_msg) {
    /* => SpeechDriverNormal::sendSpeechMessageToModem() */
    if (!sendMessageWrapper) {
        ASSERT(sendMessageWrapper != NULL);
        return -EFAULT;
    }
    return sendMessageWrapper(mWrapperArg, p_sph_msg);
}


int SpeechMessageQueue::errorHandleSpeechMessage(sph_msg_t *p_sph_msg) {
    /* => SpeechDriverNormal::errorHandleSpeechMessage() */
    if (!errorHandleMessageWrapper) {
        ASSERT(errorHandleMessageWrapper != NULL);
        return -EFAULT;
    }
    return errorHandleMessageWrapper(mWrapperArg, p_sph_msg);
}



} /* end of namespace android */

