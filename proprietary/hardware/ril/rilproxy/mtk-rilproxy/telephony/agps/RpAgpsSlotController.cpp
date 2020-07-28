/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RpAgpsSlotController.h"
#include "RpAgpsHandler.h"
#include "RpAgpsdAdapter.h"
#include "RpAgpsNSlotController.h"
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>
#include "RpAgpsThread.h"
#include "RpAgpsLog.h"
#include "util/RpFeatureOptionUtils.h"

/*****************************************************************************
 * Name Space Declarations
 *****************************************************************************/
using ::android::Parcel;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpAgpsSlotController",
        RpAgpsSlotController, RfxController);


RpAgpsSlotController::RpAgpsSlotController()
    :m_settingApn(false),
    m_testMode(false),
    m_mockOp09(false),
    m_isWap(false){
    AGPS_LOGD("RpAgpsSlotController::Constructor 0x%x", this);
}


RpAgpsSlotController::~RpAgpsSlotController() {
    AGPS_LOGD("RpAgpsSlotController::Destructor 0x%x", this);
}


void RpAgpsSlotController::onInit() {
    RfxController::onInit();
    AGPS_LOGD("RpAgpsSlotController::onInit slotID = %d, this = 0x%x",
            getSlotId(), this);
    const int urcList[] = {
        RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT,
    };
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleUrc(urcList, sizeof(urcList) / sizeof(urcList[0]));
    }
    registerDataConnectionStateChanged();
}


void RpAgpsSlotController::onDeinit() {
    AGPS_LOGD("RpAgpsSlotController::onDeinit this = 0x%x", this);
    const int urcList[] = {
        RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT,
    };
    unregisterDataConnectionStateChanged();
    if (RpFeatureOptionUtils::isC2kSupport()) {
        unregisterToHandleUrc(urcList, sizeof(urcList) / sizeof(urcList[0]));
    }
    RfxController::onDeinit();
}


void RpAgpsSlotController::registerDataConnectionStateChanged() {
    getStatusManager()->registerStatusChanged(
            RFX_STATUS_KEY_DATA_CONNECTION,
            RfxStatusChangeCallback(this,
                    &RpAgpsSlotController::onDataConnectionChanged));
}


void RpAgpsSlotController::unregisterDataConnectionStateChanged() {
    getStatusManager()->unRegisterStatusChanged(
            RFX_STATUS_KEY_DATA_CONNECTION,
            RfxStatusChangeCallback(this,
                    &RpAgpsSlotController::onDataConnectionChanged));
}


void RpAgpsSlotController::onDataConnectionChanged(
    RfxStatusKeyEnum key,RfxVariant oldValue,RfxVariant newValue) {

    RFX_ASSERT(RFX_STATUS_KEY_DATA_CONNECTION == key);
    int status = newValue.asInt();
    AGPS_LOGD("RpAgpsSlotController::onDataConnectionChanged %d, this = 0x%x",
        status, this);
    if (status == DATA_STATE_DISCONNECTED) {
        RFX_ASSERT(oldValue.asInt() == DATA_STATE_CONNECTED);
        controlApn(EVENT_AGPS_DESTROY_APN, false);
    }
}


bool RpAgpsSlotController::isDataOff() {
    RfxStatusManager *statusManager = getStatusManager();
    int status = statusManager->getIntValue(RFX_STATUS_KEY_DATA_CONNECTION,
                         DATA_STATE_DISCONNECTED);
    AGPS_LOGD("RpAgpsSlotController::isDataOff status = %d", status);
    return (status == DATA_STATE_DISCONNECTED) ? true : false;
}


void RpAgpsSlotController::setConnectionState(int state) {
    AGPS_LOGW("RpAgpsSlotController::setConnectionState state = %d", state);
    sp<RfxMessage> requestMsg =
        RfxMessage::obtainRequest(
            getSlotId(),
            RADIO_TECH_GROUP_C2K,
            RIL_LOCAL_C2K_REQUEST_AGPS_TCP_CONNIND);
    Parcel *q = requestMsg->getParcel();
    q->writeInt32(1);    // count is 1
    q->writeInt32(state);
    requestToRild(requestMsg);
}


void RpAgpsSlotController::controlApn(int event, bool force) {
    bool checkValue = (event == EVENT_AGPS_SET_APN) ? false : true;
    AGPS_LOGD("RpAgpsSlotController::controlApn event=%d, m_settingApn=%d, m_isWap=%d, force=%d",
                event, m_settingApn, m_isWap, force);
    if (m_settingApn == checkValue) {
        if (m_isWap) {
            m_isWap = false;
        } else if (event == EVENT_AGPS_DESTROY_APN && !force) {
            // Ignore destory APN event if CTWAP havne't been setup
            AGPS_LOGW("RpAgpsSlotController::controlApn ignore DESTORY event!!!");
            return;
        }
        Parcel *p = new Parcel();
        p->writeInt32(APN_TYPE_CTWAP);
        sp<RpAgpsMessage> msg = RpAgpsMessage::obtainMessage(event, p);
        sp<RpAgpsWorkingThreadHandler> handler
                = new RpAgpsWorkingThreadHandler(msg);
        RpAgpsNSlotController *ctrl =
            (RpAgpsNSlotController *)findController(
                    RFX_OBJ_CLASS_INFO(RpAgpsNSlotController));
        if (ctrl != NULL) {
            sp<RpAgpsThread> t =ctrl->getAgpsThread();
            handler->sendMessage(t->getLooper());
            m_settingApn = !checkValue;
        }
    }
}


bool RpAgpsSlotController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    if (msg_id == RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT) {
        Parcel *p = message->getParcel();
        int32_t len = -1;
        int32_t event = -1;
        int32_t status = -1;
        p->readInt32(&len);
        p->readInt32(&event);
        p->readInt32(&status);
        RFX_ASSERT(len == 2);
        RFX_ASSERT(status == 0);
        AGPS_LOGW("RpAgpsSlotController::onHandleUrc event = %d", event);
        if (event == REQUEST_DATA_CONNECTION) {
            if (isDataOff()) {
                setConnectionState(0);
            } else {
                controlApn(EVENT_AGPS_SET_APN, true);
            }

        } else if (event == CLOSE_DATA_CONNECTION) {
            controlApn(EVENT_AGPS_DESTROY_APN, true);
        } else {
            AGPS_LOGE("RpAgpsSlotController::onHandleUrc unkown event");
        }
        return true;
    } else {
        AGPS_LOGE("RpAgpsSlotController::onHandleUrc error msg = %d", msg_id);
        return false;
    }
}

void RpAgpsSlotController::onApnSetResult(bool result) {
    AGPS_LOGW("RpAgpsSlotController::onApnSetResult (%d, %d)",
        result, m_settingApn);
    if (m_settingApn) {
        int status = result? 1 : 0;
        m_isWap = result;
        setConnectionState(status);
        if (0 == status) {
            /* If set data connection fail(eg. timeout), destory apn */
            AGPS_LOGW("RpAgpsSlotController::onApnSetResult() Apn set fail and destory !");
            controlApn(EVENT_AGPS_DESTROY_APN, true);
        }
    }
}
