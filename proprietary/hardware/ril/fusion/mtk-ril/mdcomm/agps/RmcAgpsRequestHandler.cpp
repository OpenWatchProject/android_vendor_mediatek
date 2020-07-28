#include "RmcAgpsRequestHandler.h"
#include "telephony/mtk_ril.h"
#include "RfxVoidData.h"
#include "RfxIntsData.h"
#include "RfxViaUtils.h"

static const int AGPS_REQUEST[] = {
    RFX_MSG_REQUEST_AGPS_TCP_CONNIND,
};

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcAgpsRequestHandler, RIL_CMD_PROXY_1);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_AGPS_TCP_CONNIND);

RmcAgpsRequestHandler::RmcAgpsRequestHandler(int slot_id, int channel_id):
    RfxBaseHandler(slot_id, channel_id){

    logV(AGPS_TAG, "[RmcAgpsRequestHandler]Construct num:%lu",
             (unsigned long )(sizeof(AGPS_REQUEST)/sizeof(int)));
    registerToHandleRequest(AGPS_REQUEST, sizeof(AGPS_REQUEST)/sizeof(int));
}

RmcAgpsRequestHandler::~RmcAgpsRequestHandler() {
    logV(AGPS_TAG, "[RmcAgpsRequestHandler]RmcAgpsRequestHandler Destory");
}

void RmcAgpsRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(AGPS_TAG, "[RmcAgpsRequestHandler]onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_AGPS_TCP_CONNIND:
            requestAgpsConnind(msg);
            break;
        default:
            logE(AGPS_TAG, "[RmcAgpsRequestHandler]Unknown request !");
            break;
    }
}

void RmcAgpsRequestHandler::requestAgpsConnind(const sp<RfxMclMessage>& msg) {

    RIL_Errno err;
    int requestId = msg->getId();
    int *pInt = (int *)msg->getData()->getData();
    int connected = ((int*) pInt)[0];

    if (RfxViaUtils::getViaHandler() != NULL) {
        RfxViaUtils::getViaHandler()->requestAgpsConnind(this, connected, &err);
    } else {
        err = RIL_E_REQUEST_NOT_SUPPORTED;
        logE(LOG_TAG, "[RmcAgpsRequestHandler]requestAgpsConnind RIL_E_REQUEST_NOT_SUPPORTED!/n");
    }
    if (err != RIL_E_SUCCESS) {
        logE(AGPS_TAG, "[RmcAgpsRequestHandler]atSendCommand error: %d", err);
    }
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(),
                (RIL_Errno)err, RfxVoidData(), msg, true);
    logD(AGPS_TAG, "[RmcAgpsRequestHandler]reponse: toke=%d, id=%d", msg->getToken(), msg->getId());
    responseToTelCore(response);
}
