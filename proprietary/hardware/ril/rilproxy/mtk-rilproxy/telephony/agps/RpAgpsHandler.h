#ifndef __RP_AGPS_HANDLER_H__
#define __RP_AGPS_HANDLER_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/Looper.h>
#include "RpAgpsMessage.h"
#include "RpAgpsLog.h"
#include "RfxMainThread.h"

/*****************************************************************************
 * Namespace Declaration
 *****************************************************************************/
using ::android::MessageHandler;
using ::android::Message;
using ::android::sp;
using ::android::Looper;

/*****************************************************************************
 * Class RpAgpsHandler
 *****************************************************************************/
/*
 * Base handler to handle AGPS message
 */
class RpAgpsHandler : public RfxMainHandler {
public:
    explicit RpAgpsHandler(const sp<RpAgpsMessage>& msg) : m_msg(msg) {
        //AGPS_LOGD("RpAgpsHandler Construct 0x%x", this);
    }
    // Destructor
    virtual ~RpAgpsHandler() {
        //AGPS_LOGD("RpAgpsHandler Destructor 0x%x", this);
    }
    // Send AGPS message
    void sendMessage(
        sp<Looper> looper // [IN]  the looper that the message will be sent
        );

protected:
    // AGPS message referance
    sp<RpAgpsMessage> m_msg;

    // dummy message that makes handler happy
    Message m_dummyMsg;
};

/*****************************************************************************
 * Class RpAgpsWorkingThreadHandler
 *****************************************************************************/
/*
 * Handler that is used to send message to AGPS working thread
 */
class RpAgpsWorkingThreadHandler: public RpAgpsHandler {
public:
    // Constructor
    explicit RpAgpsWorkingThreadHandler(
        const sp<RpAgpsMessage>& msg  // [IN] the AGPS message
        ): RpAgpsHandler(msg) {}

    // Destructor
    virtual ~RpAgpsWorkingThreadHandler() {}

// Override
public:
    // Override handleMessage, don't the watch dog in RfxMainHandler
    virtual void handleMessage(const Message& message);
    virtual void onHandleMessage(const Message& message) { RFX_UNUSED(message);}
};

/*****************************************************************************
 * Class RpAgpsMainThreadHandler
 *****************************************************************************/
/*
 * Handler that is used to send message to main thread
 */
class RpAgpsMainThreadHandler: public RpAgpsHandler {
public:
    // Constructor
    explicit RpAgpsMainThreadHandler(
        const sp<RpAgpsMessage>& msg  // [IN] the AGPS message
        ) : RpAgpsHandler(msg) {}
    // Destructor
    virtual ~RpAgpsMainThreadHandler() {}

// Override
public:
    virtual void onHandleMessage(const Message& message);
};

#endif
