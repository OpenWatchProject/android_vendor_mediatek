/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxRootController.h"
#include "RtcAgpsUtil.h"
#include "RtcAgpsNSlotController.h"

/*****************************************************************************
 * Class RtcAgpsMessage
 *****************************************************************************/
RtcAgpsMessage::~RtcAgpsMessage(){
    RFX_LOG_V(AGPS_TAG, "[RtcAgpsMessage]destroyMessage %d, 0x%zu",
                        m_id, (size_t)m_parcel);
    if (m_parcel != NULL) {
        delete(m_parcel);
        m_parcel = NULL;
    }
}

sp<RtcAgpsMessage> RtcAgpsMessage::obtainMessage(int32_t id, Parcel* parcel) {
    sp<RtcAgpsMessage> new_msg = new RtcAgpsMessage();
    RFX_LOG_V(AGPS_TAG, "[RtcAgpsMessage]obtainMessage %d, 0x%zu",
                        id, (size_t)parcel);
    new_msg->m_id = id;
    new_msg->m_parcel = parcel;
    return new_msg;
};

/*****************************************************************************
 * Class RtcAgpsHandler
 *****************************************************************************/
void RtcAgpsHandler::sendMessage(sp<Looper> looper){
    looper->sendMessage(this, m_dummyMsg);
}


/*****************************************************************************
 * Class RtcAgpsWorkingThreadHandler
 *****************************************************************************/
void RtcAgpsWorkingThreadHandler::handleMessage(const Message& message){
    RFX_UNUSED(message);
    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    RtcAgpsNSlotController *ctrl =
            (RtcAgpsNSlotController *)root->findController(
                    RFX_OBJ_CLASS_INFO(RtcAgpsNSlotController));
    ctrl->getAgpsdAdapter()->processOutputMessage(m_msg);
}

/*****************************************************************************
 * Class RtcAgpsMainThreadHandler
 *****************************************************************************/
void RtcAgpsMainThreadHandler::onHandleMessage(const Message& message){
    RFX_UNUSED(message);
    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    RtcAgpsNSlotController *ctrl =
            (RtcAgpsNSlotController *)root->findController(
                    RFX_OBJ_CLASS_INFO(RtcAgpsNSlotController));
    if (ctrl != NULL) {
        ctrl->onHandleAgpsMessage(m_msg);
    }
}

