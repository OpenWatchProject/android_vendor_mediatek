#ifndef __RTC_AGPS_UTIL_H__
#define __RTC_AGPS_UTIL_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/Looper.h>
#include <utils/RefBase.h>
#include <utils/Parcel.h>
#include "RfxBasics.h"
#include "RfxMainThread.h"

/*****************************************************************************
 * Class Declaraion
 *****************************************************************************/


/*****************************************************************************
 * Namespace Declaration
 *****************************************************************************/
using ::android::MessageHandler;
using ::android::Message;
using ::android::sp;
using ::android::Looper;
using ::android::RefBase;
using ::android::Parcel;

/*****************************************************************************
 * Defines
 *****************************************************************************/
#define AGPS_TAG "agps-ril"

/*****************************************************************************
 * Class RtcAgpsMessage
 *****************************************************************************/
class RtcAgpsMessage : public virtual RefBase {
private:
    RtcAgpsMessage() : m_id(-1), m_parcel(NULL){}
    RtcAgpsMessage(const RtcAgpsMessage& o);
    RtcAgpsMessage& operator=(const RtcAgpsMessage& o);
    virtual ~RtcAgpsMessage();

public:
    int32_t getId() const {
        return m_id;
    }

    Parcel* getParcel() const {
        return m_parcel;
    }

    static sp<RtcAgpsMessage> obtainMessage(
        int32_t id,
        Parcel* parcel);

private:
    int32_t m_id;
    Parcel *m_parcel;
};


/*****************************************************************************
 * Class RtcAgpsHandler
 *****************************************************************************/
/*
 * Base handler to handle AGPS message
 */
class RtcAgpsHandler : public RfxMainHandler {
public:
    explicit RtcAgpsHandler(const sp<RtcAgpsMessage>& msg) : m_msg(msg) {}
    // Destructor
    virtual ~RtcAgpsHandler() {}
    // Send AGPS message
    void sendMessage(sp<Looper> looper);

protected:
    // AGPS message referance
    sp<RtcAgpsMessage> m_msg;

    // dummy message that makes handler happy
    Message m_dummyMsg;
};

/*****************************************************************************
 * Class RtcAgpsWorkingThreadHandler
 *****************************************************************************/
/*
 * Handler that is used to send message to AGPS working thread
 */
class RtcAgpsWorkingThreadHandler: public RtcAgpsHandler {
public:
    // Constructor
    explicit RtcAgpsWorkingThreadHandler(
        const sp<RtcAgpsMessage>& msg  // [IN] the AGPS message
        ): RtcAgpsHandler(msg) {}

    // Destructor
    virtual ~RtcAgpsWorkingThreadHandler() {}

// Override
public:
    // Override handleMessage, don't the watch dog in RfxMainHandler
    virtual void handleMessage(const Message& message);
    virtual void onHandleMessage(const Message& message) { RFX_UNUSED(message);}
};

/*****************************************************************************
 * Class RtcAgpsMainThreadHandler
 *****************************************************************************/
/*
 * Handler that is used to send message to main thread
 */
class RtcAgpsMainThreadHandler: public RtcAgpsHandler {
public:
    // Constructor
    explicit RtcAgpsMainThreadHandler(
        const sp<RtcAgpsMessage>& msg  // [IN] the AGPS message
        ) : RtcAgpsHandler(msg) {}
    // Destructor
    virtual ~RtcAgpsMainThreadHandler() {}

// Override
public:
    virtual void onHandleMessage(const Message& message);
};

#endif /* __RTC_AGPS_UTIL_H__ */
