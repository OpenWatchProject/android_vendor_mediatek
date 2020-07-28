#ifndef __RP_AGPS_THREAD_H__
#define __RP_AGPS_THREAD_H__
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
 * Class RpAgpsThread
 *****************************************************************************/
/*
 * RpAgpsThread is AGPS working thread that is used to communicate with
 * AGPSD.
 */
class RpAgpsThread : public Thread {
public:
    // Constructor
    RpAgpsThread();

    // Destructor
    virtual ~RpAgpsThread();

    // Create the AGPS working thread
    static sp<RpAgpsThread> create();

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

#endif /* __RP_AGPS_THREAD_H__ */