#ifndef WPFA_WPFASHMDATADISPATCHER_H
#define WPFA_WPFASHMDATADISPATCHER_H

#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include "stdint.h"
#include "WpfaRingBuffer.h"

using namespace std;


/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */


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

class WpfaShmDataDispatcher {
public:
    WpfaShmDataDispatcher();
    virtual ~WpfaShmDataDispatcher();

    bool init(WpfaRingBuffer *ringBuf);
    void start();

protected:

private:
    WpfaRingBuffer *mWpfaRingBuffer;
    
    pthread_t mNotifierThread;
    static void *notifierThreadStart(void *arg);
    void runNotifierLoop();

    int notifyReader();
};

#endif /* end of WPFA_WPFASHMDATADISPATCHER_H */

