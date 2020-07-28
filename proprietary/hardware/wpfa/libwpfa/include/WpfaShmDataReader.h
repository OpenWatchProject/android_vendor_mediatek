#ifndef WPFA_WPFASHMDATAREADER_H
#define WPFA_WPFASHMDATAREADER_H

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

class WpfaShmDataReader {
public:
    WpfaShmDataReader();
    virtual ~WpfaShmDataReader();

    bool init(WpfaRingBuffer *ringBuf);
    void start();

protected:

private:
    WpfaRingBuffer *mWpfaRingBuffer;
    
    pthread_t mReaderThread;
    static void *readerThreadStart(void *arg);
    void runReaderLoop();
};

#endif /* end of WPFA_WPFASHMDATAREADER_H */

