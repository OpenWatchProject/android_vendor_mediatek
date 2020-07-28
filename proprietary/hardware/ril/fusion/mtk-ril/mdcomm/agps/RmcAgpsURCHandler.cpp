#include "RmcAgpsURCHandler.h"
#include "telephony/mtk_ril.h"
#include "RfxVoidData.h"
#include "RfxViaUtils.h"

RFX_IMPLEMENT_HANDLER_CLASS(RmcAgpsURCHandler, RIL_CMD_PROXY_URC);

RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_VIA_GPS_EVENT);

const char * RmcAgpsURCHandler::agps_urc[] = {
  (char *)"^GPSTCPCONNREQ",
  (char *)"^GPSTCPCLOSEREQ",
};

RmcAgpsURCHandler::RmcAgpsURCHandler(int slot_id, int channel_id):
    RfxBaseHandler(slot_id, channel_id) {
    logD(AGPS_TAG, "[RmcAgpsURCHandler]Construct. num:%d", 2);
    registerToHandleURC(agps_urc, 2);
}

RmcAgpsURCHandler::~RmcAgpsURCHandler() {
    logV(AGPS_TAG, "[RmcAgpsURCHandler]destory");
}

void RmcAgpsURCHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    const char *urc = msg->getRawUrc()->getLine();
    int isConnect;
    logD(AGPS_TAG, "[RmcAgpsURCHandler]onHandleUrc:%s", urc);
    if(strStartsWith(urc, agps_urc[0])) {
        isConnect = 0;//REQUEST_DATA_CONNECTION;
    } else if (strStartsWith(urc, agps_urc[1])){
        isConnect = 1;//CLOSE_DATA_CONNECTION;
    } else {
        logE(AGPS_TAG, "[RmcAgpsURCHandler]unkown URC:%s", urc);
        return;
    }
    handleAgpsUrc(isConnect);
}

void RmcAgpsURCHandler::handleAgpsUrc(int isConnect) {
    int val = isConnect;

    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(
        RFX_MSG_URC_VIA_GPS_EVENT, m_slot_id, RfxIntsData((void*)&val, sizeof(int)));
    logD(AGPS_TAG, "[RmcAgpsURCHandler]handleAgpsUrc: slot_id=%d", m_slot_id);
    responseToTelCore(urc);
}

