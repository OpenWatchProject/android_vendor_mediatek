/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <memory>
#include <regex>
#include <string>
#include "RfxVoidData.h"
#include "RfxStringData.h"
#include "RfxIntsData.h"
#include "RfxMessageId.h"
#include "RfxImsBearerNotifyData.h"
#include "RfxImsDataInfoNotifyData.h"
#include "RmcDcUtility.h"
#include "RmcDcImsDataChannel2ReqHandler.h"

#define RFX_LOG_TAG "RmcDcImsDc2ReqHandler"
#define MAX_PROFILEID_LENGTH 10   // max profileID length

/*****************************************************************************
 * Class RmcDcImsDataChannel2ReqHandler
 * Handle RIL request on DATA2 (RIL_CMD_PROXY_10) channel
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcDcImsDataChannel2ReqHandler, RIL_CMD_PROXY_10);

RFX_REGISTER_DATA_TO_URC_ID(RfxImsBearerNotifyData, RFX_MSG_URC_IMS_BEARER_STATE_NOTIFY);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_IMS_BEARER_INIT);
RFX_REGISTER_DATA_TO_URC_ID(RfxImsDataInfoNotifyData, RFX_MSG_URC_IMS_DATA_INFO_NOTIFY);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_IMS_BEARER_STATE_CONFIRM);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_IMS_BEARER_NOTIFICATION);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxVoidData, RFX_MSG_EVENT_IMS_REQUEST_HANDLER_REGISTER_DONE);


RmcDcImsDataChannel2ReqHandler::RmcDcImsDataChannel2ReqHandler(int slot_id, int channel_id) :
    RfxBaseHandler(slot_id, channel_id) {

    RFX_LOG_D(RFX_LOG_TAG, "RmcDcImsDataChannel2ReqHandler Ctor, slot_id=%d, channel_id=%d", slot_id, channel_id);

    const int requestList[] = {
        RFX_MSG_REQUEST_IMS_BEARER_STATE_CONFIRM,
        RFX_MSG_REQUEST_SET_IMS_BEARER_NOTIFICATION,
    };

    const int eventList[] = {
        RFX_MSG_EVENT_URC_HANDLER_REGISTER_DONE,
        RFX_MSG_EVENT_DATA_IMS_PDN_NOTIFY,
        RFX_MSG_EVENT_DATA_PCSCF_ADDRESS_DISCOVERY,
    };

    rfx_property_set(IMS_EIMS_PDN_INFO,"");

    registerToHandleRequest(requestList, sizeof(requestList) / sizeof(int));
    registerToHandleEvent(eventList, sizeof(eventList) / sizeof(int));
    sendEvent(RFX_MSG_EVENT_IMS_REQUEST_HANDLER_REGISTER_DONE, RfxVoidData(),
            RIL_CMD_PROXY_URC, m_slot_id);
}

RmcDcImsDataChannel2ReqHandler::~RmcDcImsDataChannel2ReqHandler() {
}

void RmcDcImsDataChannel2ReqHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    switch (msg->getId()) {
        case RFX_MSG_REQUEST_IMS_BEARER_STATE_CONFIRM:
            handleImsBearerConfirm(msg);
            break;
         case RFX_MSG_REQUEST_SET_IMS_BEARER_NOTIFICATION:
            handleImsBearerNotificationSet(msg);
            break;
        default:
            RFX_LOG_D(RFX_LOG_TAG, "unknown request, ignore!");
            break;
    }
}

void RmcDcImsDataChannel2ReqHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    switch (msg->getId()) {
        case RFX_MSG_EVENT_URC_HANDLER_REGISTER_DONE:
            handleRegisterUrcDone();
            break;
        case RFX_MSG_EVENT_DATA_IMS_PDN_NOTIFY:
            handleImsBearerNotify(msg);
            break;
        case RFX_MSG_EVENT_DATA_PCSCF_ADDRESS_DISCOVERY:
            handlePcscfAddressDiscovery(msg);
            break;
        default:
            RFX_LOG_E(RFX_LOG_TAG, "should not be here");
            break;
    }
}


/*****************************************************************************
 * REQUEST HANDLER
 *****************************************************************************/

void RmcDcImsDataChannel2ReqHandler::notifyImsBearerRequest(int requestId, RIL_IMS_BearerNotification* notification, bool isUpdateState) {
    RFX_LOG_D(RFX_LOG_TAG, "%s,isEnableBearerNotification=%d", __FUNCTION__,isEnableBearerNotification);
    /* Customized IMS and Emergency(EIMS) PDN notify/process rule support
       0 : DO NOT notify both IMS/EIMS PDN and do NOT request/releaseNetwork in FWK
       1 or Unset : Notify both IMS/EIMS PDN and do request/releaseNetwork in FWK (Default rule)
       2 : Notify both IMS/EIMS PDN but do NOT request/releaseNetwork in FWK
       3 : DO notify IMS PDN and DO request/releaseNetwork in FWK
           DO NOT notify EIMS PDN and DO NOT request/releaseNetwork in FWK*/
    int action = notification->action;
    int pdnState = (action == 1) ? IMS_OR_EIMS_PDN_ACTIVATED : IMS_OR_EIMS_PDN_DEACTIVATED;

    if(isEnableBearerNotification == 1 || isEnableBearerNotification == 2) {
        sp<RfxMclMessage> urc_to_tel_core = RfxMclMessage::obtainUrc(requestId,
            m_slot_id, RfxImsBearerNotifyData((void*)notification, sizeof(RIL_IMS_BearerNotification)));
        responseToTelCore(urc_to_tel_core);
        if (isUpdateState) {
            if (action == 1) {
                if(strncmp(notification->type, "ims", 3) == 0) {
                    getMclStatusManager(m_slot_id)->setIntValue(RFX_STATUS_KEY_IMS_PDN_STATE,pdnState);
                    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_PDN_ACT_AID,notification->aid);
                } else if (strncmp(notification->type, "emergency", 9) == 0) {
                    getMclStatusManager(m_slot_id)->setIntValue(RFX_STATUS_KEY_EIMS_PDN_STATE,pdnState);
                    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_EIMS_PDN_ACT_AID,notification->aid);
                }
            } else if (action == 0) {
                if(strncmp(notification->type, "ims", 3) == 0) {
                    getMclStatusManager(m_slot_id)->setIntValue(RFX_STATUS_KEY_IMS_PDN_STATE,pdnState);
                    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_PDN_DEACT_AID,notification->aid);
                } else if (strncmp(notification->type, "emergency", 9) == 0) {
                    getMclStatusManager(m_slot_id)->setIntValue(RFX_STATUS_KEY_EIMS_PDN_STATE,pdnState);
                    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_EIMS_PDN_DEACT_AID,notification->aid);
                }
            }
        }
    }

    if(isEnableBearerNotification == 0 || isEnableBearerNotification == 2) {
        int aid = notification->aid;
        int err = RIL_E_SUCCESS;
        int rid = m_slot_id;
        sp<RfxAtResponse> p_response;
        String8 cmd = String8::format("AT+EIMSPDN= \"confirm\", %d, %d", aid, err);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] send %s, isEnableBearerNotification=%d ", rid, __FUNCTION__, cmd.string(), isEnableBearerNotification);
        p_response = atSendCommand(cmd);
        if (p_response->isAtResponseFail()) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s], %s returns ERROR: %d", rid, __FUNCTION__,
                cmd.string(), p_response->getError());
        }
    }

    if( isEnableBearerNotification == 3) {
        int aid = notification->aid;
        int err = RIL_E_SUCCESS;
        int rid = m_slot_id;

        if(action == 1) {
            if(strncmp(notification->type, "ims", 3) == 0) {
                sp<RfxMclMessage> urc_to_tel_core = RfxMclMessage::obtainUrc(requestId,
                        m_slot_id, RfxImsBearerNotifyData((void*)notification, sizeof(RIL_IMS_BearerNotification)));
                responseToTelCore(urc_to_tel_core);
                if (isUpdateState) {
                    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_PDN_ACT_AID,notification->aid);
                    getMclStatusManager(m_slot_id)->setIntValue(RFX_STATUS_KEY_IMS_PDN_STATE,pdnState);
                }
            } else if (strncmp(notification->type, "emergency", 9) == 0) {
                sp<RfxAtResponse> p_response;
                String8 cmd = String8::format("AT+EIMSPDN= \"confirm\", %d, %d", aid, err);
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] send %s, isEnableBearerNotification=%d ", rid, __FUNCTION__, cmd.string(), isEnableBearerNotification);
                p_response = atSendCommand(cmd);
                if (p_response->isAtResponseFail()) {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s], %s returns ERROR: %d", rid, __FUNCTION__,
                        cmd.string(), p_response->getError());
                }
            }
        } else if(action == 0){
            if(strncmp(notification->type, "ims", 3) == 0) {
                sp<RfxMclMessage> urc_to_tel_core = RfxMclMessage::obtainUrc(requestId,
                        m_slot_id, RfxImsBearerNotifyData((void*)notification, sizeof(RIL_IMS_BearerNotification)));
                responseToTelCore(urc_to_tel_core);
                if (isUpdateState) {
                    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_PDN_DEACT_AID,notification->aid);
                    getMclStatusManager(m_slot_id)->setIntValue(RFX_STATUS_KEY_IMS_PDN_STATE,pdnState);
                }
            } else if (strncmp(notification->type, "emergency", 9) == 0) {
                sp<RfxAtResponse> p_response;
                String8 cmd = String8::format("AT+EIMSPDN= \"confirm\", %d, %d", aid, err);
                RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] send %s, isEnableBearerNotification=%d ", rid, __FUNCTION__, cmd.string(), isEnableBearerNotification);
                p_response = atSendCommand(cmd);
                if (p_response->isAtResponseFail()) {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s], %s returns ERROR: %d", rid, __FUNCTION__,
                        cmd.string(), p_response->getError());
                }
            }
        }
    }
}

void RmcDcImsDataChannel2ReqHandler::notifyImsDataInfo(RIL_IMS_DataInfoNotify* notification) {
    RFX_LOG_D(RFX_LOG_TAG, "%s", __FUNCTION__);

    sp<RfxMclMessage> urc_to_tel_core = RfxMclMessage::obtainUrc(RFX_MSG_URC_IMS_DATA_INFO_NOTIFY,
        m_slot_id, RfxImsDataInfoNotifyData((void*)notification, sizeof(RIL_IMS_DataInfoNotify)));
    responseToTelCore(urc_to_tel_core);
}
// AT+EIMSPDN activation/deactivation confirm to DDM
void RmcDcImsDataChannel2ReqHandler::handleImsBearerConfirm(const sp<RfxMclMessage>& msg) {
    //RFX_LOG_D(RFX_LOG_TAG, "handleImsBearerConfirm");

    const int *pReqData = (const int*)msg->getData()->getData();
    int aid = pReqData[0];
    int action = pReqData[1];
    int status = pReqData[2];
    int rid = m_slot_id;

    int tempImsAid = -1;
    int tempEimsAid = -1;

    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> responseMsg;

    String8 cmd = String8::format("AT+EIMSPDN= \"confirm\", %d, %d", aid, status);
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] send %s", rid, __FUNCTION__, cmd.string());
    p_response = atSendCommand(cmd);
    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s], %s returns ERROR: %d", rid, __FUNCTION__,
            cmd.string(), p_response->getError());
    }
    responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
    responseToTelCore(responseMsg);

    if (action == 1) {
        tempImsAid = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_IMS_PDN_ACT_AID, -1);
        tempEimsAid = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_EIMS_PDN_ACT_AID, -1);
        if (aid == tempImsAid) {
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_PDN_ACT_AID,RFX_NO_ONGOING_IMS_PDN_AID);
        } else if (aid == tempEimsAid) {
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_EIMS_PDN_ACT_AID,RFX_NO_ONGOING_IMS_PDN_AID);
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s], no match aid=%d", rid, __FUNCTION__, aid);
        }
    }
    if (action == 0) {
        tempImsAid = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_IMS_PDN_DEACT_AID, -1);
        tempEimsAid = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_EIMS_PDN_DEACT_AID, -1);
        if (aid == tempImsAid) {
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_PDN_DEACT_AID,RFX_NO_ONGOING_IMS_PDN_AID);
        } else if (aid == tempEimsAid) {
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_EIMS_PDN_DEACT_AID,RFX_NO_ONGOING_IMS_PDN_AID);
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s], no match aid=%d", rid, __FUNCTION__, aid);
        }
    }
}


/*****************************************************************************
 * EVENT HANDLER
 *****************************************************************************/

// Callback function when RmcDcUrcHandler URC registration ready
void RmcDcImsDataChannel2ReqHandler::handleRegisterUrcDone() {
    //RFX_LOG_D(RFX_LOG_TAG, "handleRegisterUrcDone");

    sp<RfxAtResponse> p_response;
    String8 cmd("");
    if (RmcDcUtility::isImsSupport()) {
        cmd.append(String8::format("AT+EIMSPDN= \"onoff\", 1"));
    } else {
        cmd.append(String8::format("AT+EIMSPDN= \"onoff\", 0"));
    }
    RFX_LOG_D(RFX_LOG_TAG, "[%d][onRegisterUrcDone], send %s", m_slot_id, cmd.string());
    p_response = atSendCommand(cmd);
    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "%s returns ERROR", cmd.string());
    }

}

// URC +EIMSPDN "notify" URC, msg is RfxStringData format
void RmcDcImsDataChannel2ReqHandler::handleImsBearerNotify(const sp<RfxMclMessage>& msg) {
        //RFX_LOG_D(RFX_LOG_TAG, "handleImsBearerNotify");

        //+EIMSPDN: <cmd>, <aid>, <state>, <type>
        //+EIMSPDN: "notify", <aid>, <state>, <type>, <if_id>[, <dnsv4_num>, <dnsv4_addr> [,<dnsv6_num>, <dnsv6_addr>]]

        //<cmd> :
        //       "notify" -> MD notify AP to construct IMS PDN
        static int ACTION_IMS_BEARER_DEACTIVATION = 0;
        static int ACTION_IMS_BEARER_ACTIVATION = 1;
        int action = -1;

        char *urc = (char*)msg->getData()->getData();
        int rid = m_slot_id;
        int err = 0;
        char *cmdFormat = NULL;
        std::unique_ptr<RIL_IMS_BearerNotification> notification(new RIL_IMS_BearerNotification());
        std::unique_ptr<RfxAtLine> pLine(new RfxAtLine(urc, NULL));
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] urc=%s", rid, __FUNCTION__, urc);
        pLine->atTokStart(&err);
        if (err < 0) return;

        cmdFormat = pLine->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing cmd",
                    rid, __FUNCTION__);
            return;
        }
        if (strncmp("notify", cmdFormat, strlen("notify")) == 0) {
            int aid = -1;
            int state = -1;
            char *type = NULL;
            int interfaceId = -1;

            aid = pLine->atTokNextint(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing aid",
                        rid, __FUNCTION__);
                return;
            }

            action = pLine->atTokNextint(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing state",
                        rid, __FUNCTION__);
                return;
            }

            type = pLine->atTokNextstr(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing type",
                        rid, __FUNCTION__);
                return;
            }

            notification->phone = m_slot_id;
            notification->aid = aid;
            notification->type = type;
            notification->action = action;
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] IMS notification phone=%d, aid=%d, type=%s, action = %d",
                        rid, __FUNCTION__, notification->phone, notification->aid, notification->type, action);

            //LinkMaster
            //"<PDN_TYPE>_<Slot ID>_<Interface_ID>,<PDN_TYPE>_<Slot ID>_<Interface_ID>, ..."
            //ims_0_0,emergency_1_2,ims_1_2,
            char currProp[MTK_PROPERTY_VALUE_MAX] = {0};
            rfx_property_get(IMS_EIMS_PDN_INFO,currProp,"");
            char tempProp[MTK_PROPERTY_VALUE_MAX] = {0};
            //LinkMaster

        // Emergency PDN shall be always sync to FWK to meet AGPS IR.92 requirement.
#if 0
            if (needIgnoreImsBearerNotify(action, type)) {
                confirmImsBearerNotify(aid, err);
                return;
            }
#endif

            if (action == ACTION_IMS_BEARER_ACTIVATION) {
                //LinkMaster
                interfaceId =  pLine->atTokNextint(&err);
                if (err >= 0) {
                    snprintf(tempProp,sizeof(tempProp), "%s_%d_%d,", type, rid, interfaceId);
                    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] activate pdn type is %s, slotid is %d, interfaceID is %d",
                            rid, __FUNCTION__, type, rid, interfaceId);
                    if (strcmp(currProp,"") == 0) { //first property
                        strncpy(currProp, tempProp, MTK_PROPERTY_VALUE_MAX-1);
                    } else { //second property or more
                        //Clean previous info at same SIM
                        string tempPropStr;
                        tempPropStr = currProp;
                        char begin_Index[MAX_PROFILEID_LENGTH+2]={0};
                        snprintf(begin_Index,sizeof(begin_Index),"%s_%d",type,rid);
                        string::size_type begin_ = tempPropStr.find(begin_Index);
                        if (begin_!= std::string::npos) {
                            begin_=begin_+strlen(begin_Index)+1;
                            string::size_type end_ = tempPropStr.rfind(type,begin_);
                            if(end_ != std::string::npos) {
                                if(end_ == 0) {
                                    tempPropStr.erase(end_,begin_-end_+2);
                                } else {
                                    tempPropStr.erase(end_-1,begin_-end_+2);
                                }
                                strncpy(currProp, tempPropStr.c_str(), MTK_PROPERTY_VALUE_MAX-1);
                                rfx_property_set(IMS_EIMS_PDN_INFO,currProp);
                            } else {
                                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] End index is not found, no need to clean property", rid, __FUNCTION__);
                            }
                        } else {
                            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Begin index is not found, no need to clean property", rid, __FUNCTION__);
                        }
                        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] current vendor.ims.eims.pdn.info= %s", rid, __FUNCTION__, currProp);
                        strncat(currProp+strlen(currProp), tempProp, MTK_PROPERTY_VALUE_MAX-strlen(currProp)-1);
                    }
                    rfx_property_set(IMS_EIMS_PDN_INFO,currProp);
                    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] updated vendor.ims.eims.pdn.info= %s", rid, __FUNCTION__, currProp);
                } else {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Cannot get interfaceID", rid, __FUNCTION__);
                }//LinkMaster

                notifyImsBearerRequest(RFX_MSG_URC_IMS_BEARER_STATE_NOTIFY, notification.get(), true);
            } else if (action == ACTION_IMS_BEARER_DEACTIVATION) {
                //LinkMaster
                char begin_Index[MAX_PROFILEID_LENGTH+2]={0};
                snprintf(begin_Index,sizeof(begin_Index),"%s_%d",type,rid);
                RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] deactivate pdn type is %s, slotid is %d",
                        rid, __FUNCTION__, type, rid);
                if (strcmp(currProp,"") != 0) {
                    string tempPropStr;
                    tempPropStr = currProp;
                    string::size_type begin_ = tempPropStr.find(begin_Index);
                    if (begin_!= std::string::npos) {
                        begin_=begin_+strlen(begin_Index)+1;
                        string::size_type end_ = tempPropStr.rfind(type,begin_);
                        if(end_ != std::string::npos) {
                            if(end_ == 0) {
                                tempPropStr.erase(end_,begin_-end_+2);
                            } else {
                                tempPropStr.erase(end_-1,begin_-end_+2);
                            }
                            strncpy(currProp, tempPropStr.c_str(), MTK_PROPERTY_VALUE_MAX-1);
                            rfx_property_set(IMS_EIMS_PDN_INFO,currProp);
                        } else {
                            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] End index is not found, no need to clean property", rid, __FUNCTION__);
                        }
                    } else {
                        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Begin index is not found, no need to clean property", rid, __FUNCTION__);
                    }
                    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] updated vendor.ims.eims.pdn.info= %s", rid, __FUNCTION__, currProp);
                } else {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] Property is null, no need to clean property", rid, __FUNCTION__);
                }//LinkMaster

                notifyImsBearerRequest(RFX_MSG_URC_IMS_BEARER_STATE_NOTIFY, notification.get(), true);
            }
        } else if(strncmp("init", cmdFormat, strlen("init")) == 0) {

            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] IMS notification phone=%d bearer initial...",
                    rid, __FUNCTION__, m_slot_id);
            sp<RfxMclMessage> urc_to_tel_core = RfxMclMessage::obtainUrc(
                    RFX_MSG_URC_IMS_BEARER_INIT, m_slot_id,
                    RfxIntsData((void*)&m_slot_id, sizeof(int)));
            responseToTelCore(urc_to_tel_core);
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_PDN_ACT_AID,RFX_NO_ONGOING_IMS_PDN_AID);
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_PDN_DEACT_AID,RFX_NO_ONGOING_IMS_PDN_AID);
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_EIMS_PDN_ACT_AID,RFX_NO_ONGOING_IMS_PDN_AID);
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_EIMS_PDN_DEACT_AID,RFX_NO_ONGOING_IMS_PDN_AID);
            getMclStatusManager(rid)->setIntValue(RFX_STATUS_KEY_IMS_PDN_STATE,IMS_OR_EIMS_PDN_INIT);
            getMclStatusManager(rid)->setIntValue(RFX_STATUS_KEY_EIMS_PDN_STATE,IMS_OR_EIMS_PDN_INIT);
        }else if(strncmp("reject", cmdFormat, strlen("reject")) == 0){
            int aid = -1;
            int cause = -1;
            char *type = NULL;
            char *strEvent = NULL;
            char *strCause = NULL;

            std::unique_ptr<RIL_IMS_DataInfoNotify> notification(new RIL_IMS_DataInfoNotify());

            // Notify Clear Codes 33/29 event for IMS PDN activation
            //+EIMSPDN: "reject", <cause> , <type>
            //<cause> : 29 or 33
            //<type> : "ims" or "emergency"

            cause = pLine->atTokNextint(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing cause",
                        rid, __FUNCTION__);
                return;
            }

            type = pLine->atTokNextstr(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing type",
                        rid, __FUNCTION__);
                return;
            }

            asprintf(&strEvent, "ClearCodes");
            asprintf(&strCause, "%d", cause);
            notification->phone = m_slot_id;
            notification->type = type;
            notification->event = strEvent;
            notification->extra = strCause;
            notifyImsDataInfo(notification.get());
            if(strEvent != NULL) free(strEvent);
            if(strCause != NULL) free(strCause);
        }
}

void RmcDcImsDataChannel2ReqHandler::handleImsBearerNotificationSet(const sp<RfxMclMessage>& msg) {
    const int *pReqData = (const int*)msg->getData()->getData();
    int enable = pReqData[0];
    int rid = m_slot_id;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> responseMsg;

    isEnableBearerNotification = enable;

    checkImsServiceRestartAfterSystemCrash();

    notifyUnsyncImsBearerRequest();

    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] isEnableBearerNotification: %d", rid, __FUNCTION__, isEnableBearerNotification);
    responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
    responseToTelCore(responseMsg);
}


// P-CSCF discovery, not support now, msg is RfxStringData format
void RmcDcImsDataChannel2ReqHandler::handlePcscfAddressDiscovery(const sp<RfxMclMessage>& msg) {
        //RFX_LOG_D(RFX_LOG_TAG, "handlePcscfAddressDiscovery");

        //+EIMSPDIS:  <transaction_id>,<em_ind>,<method>, <nw_if_name[]>
        //AT+EIMSPCSCF= <transaction_id>,<method>, <protocol_type>, <port_num>, <addr>
        //AT+EIMSPDIS= <transaction_id>, <method>, <is_success>
        char *urc = (char*)msg->getData()->getData();
        int rid = m_slot_id;
        int err = 0;
        int tranid = -1;
        int em_ind = -1;
        int method = -1;
        char *interfaceId = NULL;
        sp<RfxAtResponse> p_response;
        String8 pcscfCmd;

        RfxAtLine *pLine = new RfxAtLine(urc, NULL);
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] urc=%s", rid, __FUNCTION__, urc);

        pLine->atTokStart(&err);
        if (err < 0) goto error;

        tranid = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing tranid",
                    rid, __FUNCTION__);
            goto error;
        }

        em_ind = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing em_ind",
                    rid, __FUNCTION__);
            goto error;
        }

        method = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing method",
                    rid, __FUNCTION__);
            goto error;
        }

        interfaceId = pLine->atTokNextstr(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing interfaceId",
                    rid, __FUNCTION__);
            goto error;
        }

        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] pcscf discovery tranid=%d, em_ind=%d, method = %d,ifaceId=%s",
                rid, __FUNCTION__, tranid, em_ind, method, interfaceId);

        //Current not support pcscf discovery, so always response fail to MD.
        pcscfCmd = String8::format("AT+EIMSPDIS= %d, %d, 0", tranid, method);
        //RFX_LOG_D(RFX_LOG_TAG, "[%d][%s], rid, __FUNCTION__, send %s", rid, __FUNCTION__, pcscfCmd.string());
        p_response = atSendCommand(pcscfCmd);
        if (p_response->isAtResponseFail()) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s], %s returns ERROR", rid, __FUNCTION__, pcscfCmd.string());
        }

    error:
        AT_LINE_FREE(pLine);
        return;

}

// handle unsynchronized scenario after phone/ims service has been killed
void RmcDcImsDataChannel2ReqHandler::checkImsServiceRestartAfterSystemCrash() {
    char value[RFX_PROPERTY_VALUE_MAX] = {0};
    char zygote_value[RFX_PROPERTY_VALUE_MAX] = {0};
    int imsPdnState  = IMS_OR_EIMS_PDN_INIT;
    int eimsPdnState  = IMS_OR_EIMS_PDN_INIT;

    // get property to check if zygote restarted
    rfx_property_get("vendor.ril.zygote.state", zygote_value, "");
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] vendor.ril.zygote.state=%s", m_slot_id, __FUNCTION__, zygote_value);

    if (strcmp(zygote_value, "stopped") == 0 || strcmp(zygote_value, "restarting") == 0) {
        // trigger TRM to handle unsynchronized scenario
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] trigger TRM after zygote restarting",
                m_slot_id, __FUNCTION__);

        rfx_property_set("vendor.ril.zygote.state", "");

        rfx_property_set("vendor.ril.mux.report.case", "2");
        rfx_property_set("vendor.ril.muxreport", "1");

        return;
    }
}

void RmcDcImsDataChannel2ReqHandler::notifyUnsyncImsBearerRequest() {
    static int ACTION_IMS_BEARER_DEACTIVATION = 0;
    static int ACTION_IMS_BEARER_ACTIVATION = 1;
    char type[] = "ims";

    int imsPdnState  = IMS_OR_EIMS_PDN_INIT;
    int aid = -1;

    std::unique_ptr<RIL_IMS_BearerNotification> notification(new RIL_IMS_BearerNotification());

    imsPdnState = getMclStatusManager()->getIntValue(
        RFX_STATUS_KEY_IMS_PDN_STATE,IMS_OR_EIMS_PDN_INIT);

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] imsPdnState=%d", m_slot_id, __FUNCTION__, imsPdnState);

    notification->phone = m_slot_id;
    notification->type = type;

    if (imsPdnState == IMS_OR_EIMS_PDN_ACTIVATED) {
        aid = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_IMS_PDN_ACT_AID, -1);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] aid=%d", m_slot_id, __FUNCTION__, aid);
        if (aid != RFX_NO_ONGOING_IMS_PDN_AID) {
            notification->aid = aid;
            notification->action = ACTION_IMS_BEARER_ACTIVATION;

            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] notifyUnsyncImsBearerRequest phone=%d, aid=%d, type=%s, action = %d",
                        m_slot_id, __FUNCTION__, notification->phone, notification->aid, notification->type, notification->action);

            notifyImsBearerRequest(RFX_MSG_URC_IMS_BEARER_STATE_NOTIFY, notification.get(), false);
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no ongoing IMS PDN ACT aid",
                    m_slot_id, __FUNCTION__);
        }
    } else if (imsPdnState == IMS_OR_EIMS_PDN_DEACTIVATED) {
        aid = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_IMS_PDN_DEACT_AID, -1);
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] aid=%d", m_slot_id, __FUNCTION__, aid);
        if (aid != RFX_NO_ONGOING_IMS_PDN_AID) {
            notification->aid = aid;
            notification->action = ACTION_IMS_BEARER_DEACTIVATION;

            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] notifyUnsyncImsBearerRequest phone=%d, aid=%d, type=%s, action = %d",
                        m_slot_id, __FUNCTION__, notification->phone, notification->aid, notification->type, notification->action);

            notifyImsBearerRequest(RFX_MSG_URC_IMS_BEARER_STATE_NOTIFY, notification.get(), false);
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no ongoing IMS PDN DEACT aid",
                    m_slot_id, __FUNCTION__);
        }
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] no ongoing ACTIVATED or DEACTIVATED event",
                m_slot_id, __FUNCTION__);
    }
}