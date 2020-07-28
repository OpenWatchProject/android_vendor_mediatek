#ifndef __RTC_AGPS_THREAD_H__
#define __RTC_AGPS_THREAD_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/Looper.h>
#include <utils/threads.h>

/*****************************************************************************
 * Name Space Declaration
 *****************************************************************************/
using ::android::Looper;
using ::android::Thread;
using ::android::sp;
using ::android::status_t;

/*****************************************************************************
 * Class RtcAgpsThread
 *****************************************************************************/
/*
 * RtcAgpsThread is AGPS working thread that is used to communicate with
 * AGPSD.
 */
class RtcAgpsThread : public Thread {
public:
    // Constructor
    RtcAgpsThread();

    // Destructor
    virtual ~RtcAgpsThread();

    // Create the AGPS working thread
    static sp<RtcAgpsThread> create();

    // Get the Looper of the AGPS working thread
    // RETURNS: the looper of the thead
    sp<Looper> getLooper();

// Override
protected:
    virtual bool threadLoop();
    virtual status_t readyToRun();

// Implement
private:
    // the looper that is attached to this thread
    sp<Looper> m_looper;
};

#endif /* __RTC_AGPS_THREAD_H__ */