#include "WpfaDriver.h"
#include "WpfaDriverMessage.h"
#include "WpfaShmDataDispatcher.h"

WpfaShmDataDispatcher::WpfaShmDataDispatcher() {
    mNotifierThread = 0;
    mWpfaRingBuffer = NULL;
    cout << "-new()" << endl;
}

WpfaShmDataDispatcher::~WpfaShmDataDispatcher() {
    cout << "-del()" << endl;
}

bool WpfaShmDataDispatcher::init(WpfaRingBuffer *ringBuf) {
    mWpfaRingBuffer = ringBuf;
    return true;
}

void WpfaShmDataDispatcher::start() {
    int ret;
    pthread_attr_t attr;

    // creat reader thread
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&mNotifierThread, &attr, WpfaShmDataDispatcher::notifierThreadStart, this);
    pthread_attr_destroy(&attr);
    /*
    if (ret != 0) {
        cout << "WpfaShmDataDispatcher-start() fail" << endl;
    } else {
        cout << "WpfaShmDataDispatcher-start() create reader thread OK!" << endl;
    }
    */
}

void *WpfaShmDataDispatcher::notifierThreadStart(void *arg) {
    cout << "WpfaShmDataDispatcher-notifierThreadStart()" << endl;
    WpfaShmDataDispatcher *me = (WpfaShmDataDispatcher*)arg;
    me->runNotifierLoop();
    return NULL;
}

void WpfaShmDataDispatcher::runNotifierLoop() {
    cout << "-runNotifierLoop()" << endl;

    int ret = 0;
    region_info_t* mRegion = NULL;
    WpfaDriver *mWpfaDriver = WpfaDriver::getInstance();

    mRegion = (region_info_t *)malloc(sizeof(region_info_t));

    if (mRegion == NULL){
        cout << "mRegion is NULL, return." << endl;
        return;
    }

    //while(1) {
    for(int i = 0; i < 2; i++) {
        mWpfaRingBuffer->lock("Consumer");
        mWpfaRingBuffer->waitCanRead("Consumer");
        mWpfaRingBuffer->getRegionInfoForReader(mRegion);
        mWpfaRingBuffer->setState(RING_BUFFER_STATE_READING);
        cout << "mRegion.read_idx:" << mRegion->read_idx << " mRegion.read_size:" << mRegion->data_size << endl;
        mWpfaRingBuffer->unlock("Consumer");

        // blocking call
        ret = mWpfaDriver->notifyCallback(EVENT_M2A_READ_DATA_PTK, mRegion);
        // blocking call

        if (ret <= 0) {
            cout << "runNotifierLoop() read fail!!!" << endl;
        } else {
            cout << "Consumer ==> read size:" << ret << endl;
            mWpfaRingBuffer->lock("Consumer");
            mWpfaRingBuffer->readDone();
            mWpfaRingBuffer->signalCanWrite("Consumer");
            mWpfaRingBuffer->unlock("Consumer");
            // reset
            mRegion->read_idx = INVALID_INDEX;
            mRegion->data_size = 0;
        }
    }

    free(mRegion);
}

int WpfaShmDataDispatcher::notifyReader() {
    int ret = 0;
    WpfaDriver *mWpfaDriver = WpfaDriver::getInstance();

    // update state first
    //mControlPara.mRbState = RING_BUFFER_STATE_READING;

    //invoke callback function of Wpfa
    region_info_t* mRegion = NULL;
    mRegion = (region_info_t *)malloc(sizeof(region_info_t));

    ret = mWpfaDriver->notifyCallback(EVENT_M2A_READ_DATA_PTK, mRegion);

    free(mRegion);

    return ret;
}


