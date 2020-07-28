package com.mediatek.common.carrierexpress;

import java.util.List;

/** {@hide} */
interface ICarrierExpressService
{
    String getActiveOpPack();
    String getOpPackFromSimInfo(String mcc_mnc);
    void setOpPackActive(String opPack, String opSubId, int mainSlot);
    Map getAllOpPackList();
    List getOperatorSubIdList(String opPack);
}