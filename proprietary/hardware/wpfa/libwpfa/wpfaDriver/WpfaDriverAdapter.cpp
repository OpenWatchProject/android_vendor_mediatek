
#include "WpfaDriverAdapter.h"
#include "WpfaControlMsgHandler.h"
#include "WpfaFilterRuleReqHandler.h"
#include "WpfaShmReadMsgHandler.h"
#include "WpfaShmWriteMsgHandler.h"

#define WPFA_D_LOG_TAG "WpfaDriverAdapter"

WpfaDriverAdapter *WpfaDriverAdapter::sInstance = NULL;
Mutex WpfaDriverAdapter::sWpfaDriverAdapterInitMutex;
int WpfaDriverAdapter::mCcciHandler = -1;

WpfaDriverAdapter::WpfaDriverAdapter() {
    mtkLogD(WPFA_D_LOG_TAG, "-new()");
    init();
}

void WpfaDriverAdapter::init() {
    mtkLogD(WPFA_D_LOG_TAG, "-init()");
    WpfaFilterRuleReqHandler::init();
    WpfaShmWriteMsgHandler::init();
    WpfaShmReadMsgHandler::init();
    WpfaControlMsgHandler::init(mCcciHandler);
}

WpfaDriverAdapter::~WpfaDriverAdapter() {
    mtkLogD(WPFA_D_LOG_TAG, "-del()");
}

WpfaDriverAdapter* WpfaDriverAdapter::getInstance() {
    if (sInstance != NULL) {
       return sInstance;
    } else {
       sWpfaDriverAdapterInitMutex.lock();
       sInstance = new WpfaDriverAdapter();
       if (sInstance == NULL) {
          mtkLogE(WPFA_D_LOG_TAG, "new WpfaDriverAdapter fail");
       }
       sWpfaDriverAdapterInitMutex.unlock();
       return sInstance;
    }
}

int WpfaDriverAdapter::sendMsgToControlMsgDispatcher(const sp<WpfaDriverMessage>& message) {
    int retValue = 0;
    WpfaControlMsgHandler::enqueueDriverMessage(message);
    return retValue;
}

int WpfaDriverAdapter::sendMsgToFilterRuleReqHandler(const sp<WpfaDriverMessage>& message) {
    int retValue = 0;
    WpfaFilterRuleReqHandler::enqueueFilterRuleReqMessage(message);
    return retValue;
}

int WpfaDriverAdapter::sendMsgToShmReadMsgHandler(const sp<WpfaDriverMessage>& message) {
    int retValue = 0;
    WpfaShmReadMsgHandler::enqueueShmReadMessage(message);
    return retValue;
}

int WpfaDriverAdapter::sendMsgToShmWriteMsgHandler(const sp<WpfaDriverMessage>& message) {
    int retValue = 0;
    WpfaShmWriteMsgHandler::enqueueShmWriteMessage(message);
    return retValue;
}

void WpfaDriverAdapter::setCcciHandler(int fd) {
    mCcciHandler = fd;
    mtkLogD(WPFA_D_LOG_TAG, "setCcciHandler:%d", mCcciHandler);
}

int WpfaDriverAdapter::getCcciHandler() {
    return mCcciHandler;
}