/*
 * RtcImsDialogHandler.cpp
 *
 */
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include "RfxRootController.h"
#include "RfxLog.h"
#include "RfxStringsData.h"
#include "RfxXmlParser.h"
#include "RfxDialogInfoData.h"
#include "rfx_properties.h"
#include "RfxRilUtils.h"

#include "RtcImsConferenceController.h"
#include "RtcImsDialogHandler.h"

#define RFX_LOG_TAG "RtcImsDialog"

const string RtcImsDialogHandler::TAG_NEXT_LINE("<ascii_10>");
const string RtcImsDialogHandler::TAG_RETURN("<ascii_13>");
const string RtcImsDialogHandler::TAG_DOUBLE_QUOTE("<ascii_34>");

RtcImsDialogHandler::RtcImsDialogHandler(int slot){
    mSlot = slot;
    RFX_LOG_D(RFX_LOG_TAG, "RtcImsDialogHandler()");
}

RtcImsDialogHandler::~RtcImsDialogHandler() {
}

//Handle IMS Dialog event package raw data.
void RtcImsDialogHandler::handleImsDialogMessage(const sp<RfxMessage>& message) {
    RFX_LOG_D(RFX_LOG_TAG, "handleImsDialogMessage");
    /*
     * +EIMSEVTPKG: <call_id>,<type>,<urc_index>,<total_urc_count>,<data>
     * <call_id>:  0~255
     * <type>: 1 = Conference Event Package; 2 = Dialog Event Package; 3 = Message Waiting Event Package
     * <urc_index>: 1~255, the index of URC part
     * <total_urc_count>: 1~255
     * <data>: xml raw data, max length = 1950
     */

    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    int callId = atoi(params[0]);
    int msgType = atoi(params[1]);
    int index = atoi(params[2]);
    int count = atoi(params[3]);
    char* rawData = params[4];

    bool isFirstPkt = (index == 1);
    mDepData = concatData(isFirstPkt, mDepData, rawData);
    if (index != count) {
        //do nothing
        return;
    }
    mDepData = recoverDataFromAsciiTag(mDepData);
    if (mDepData.empty()) {
        RFX_LOG_D(RFX_LOG_TAG, "Failed to handleImsDialogMessage due to data is empty");
        return;
    }

    sp<RfxXmlParser> parser = new RfxXmlParser();
    DepMessageHandler* xmlData = new DepMessageHandler();
    if (xmlData == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "can't create xmlData object.");
        return;
    }

    parser->parse(xmlData, mDepData);

    vector<sp<RfxDialog>> dialogList = xmlData->getDialogInfo();

    int size = dialogList.size();
    RFX_LOG_D(RFX_LOG_TAG, "dialog_info size: %d", size);
    if (size > 0) {
        RIL_DialogInfo ** dialogInfo = (RIL_DialogInfo **) alloca(size * sizeof(RIL_DialogInfo*));
        RIL_DialogInfo * dialogs = (RIL_DialogInfo *) alloca(size * sizeof(RIL_DialogInfo));

        /* init the pointer array */
        for (int i = 0; i < size; i++) {
            dialogInfo[i] = &(dialogs[i]);
        }
        for (int i = 0; i < size; i++) {
            RFX_LOG_D(RFX_LOG_TAG, "dialog_info copy data: %d", i);
            dialogInfo[i]->dialogId = dialogList[i]->getDialogId();
            dialogInfo[i]->callState = dialogList[i]->getCallState();
            dialogInfo[i]->callType = dialogList[i]->getCallType();
            dialogInfo[i]->isCallHeld = dialogList[i]->isCallHeld();
            dialogInfo[i]->isPullable = dialogList[i]->isPullable();
            dialogInfo[i]->isMt = dialogList[i]->isMt();
            int len = strlen(dialogList[i]->getAddress().c_str());
            dialogInfo[i]->address = (char *) alloca((len + 1)* sizeof(char));
            memset(dialogInfo[i]->address, 0, len + 1);
            strncpy(dialogInfo[i]->address, dialogList[i]->getAddress().c_str(), len);

            len = strlen(dialogList[i]->getRemoteAddress().c_str());
            dialogInfo[i]->remoteAddress = (char *) alloca((len + 1)* sizeof(char));
            memset(dialogInfo[i]->remoteAddress, 0, len + 1);
            strncpy(dialogInfo[i]->remoteAddress, dialogList[i]->getRemoteAddress().c_str(), len);
            RFX_LOG_D(RFX_LOG_TAG, "dialog_info copy data end: dialogId:%d, addr:%s, remoteAddr:%s",
                    dialogInfo[i]->dialogId, RfxRilUtils::pii(RFX_LOG_TAG, dialogInfo[i]->address),
                    RfxRilUtils::pii(RFX_LOG_TAG, dialogInfo[i]->remoteAddress));
        }
        RFX_LOG_D(RFX_LOG_TAG, "dialog_info to obtainRequest");
        sp<RfxMessage> msg = RfxMessage::obtainUrc(mSlot, RFX_MSG_URC_IMS_DIALOG_INDICATION,
                RfxDialogInfoData(dialogInfo, size * sizeof(RIL_DialogInfo*)),
                message->getSource());

        RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
        RtcImsConferenceController *ctrl =
                (RtcImsConferenceController *)root->findController(mSlot,
                        RFX_OBJ_CLASS_INFO(RtcImsConferenceController));
        ctrl->responseToRilj(msg);
        RFX_LOG_D(RFX_LOG_TAG, "send a RFX_MSG_URC_IMS_DIALOG_INDICATION");
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "dialog_info to obtainRequest");
        sp<RfxMessage> msg = RfxMessage::obtainUrc(mSlot, RFX_MSG_URC_IMS_DIALOG_INDICATION,
                RfxDialogInfoData(NULL, 0), message->getSource());

        RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
        RtcImsConferenceController *ctrl =
                (RtcImsConferenceController *)root->findController(mSlot,
                        RFX_OBJ_CLASS_INFO(RtcImsConferenceController));
        ctrl->responseToRilj(msg);
        RFX_LOG_D(RFX_LOG_TAG, "send a RFX_MSG_URC_IMS_DIALOG_INDICATION");
    }
    delete xmlData;
    xmlData = NULL;
}

string RtcImsDialogHandler::concatData(int isFirst, string origData, string appendData) {
    if (isFirst) {
        return appendData;
    }
    return origData + appendData;
}

string RtcImsDialogHandler::recoverDataFromAsciiTag(string data) {
    data = replaceAll(data, TAG_RETURN, "\r");
    data = replaceAll(data, TAG_DOUBLE_QUOTE, "\"");
    data = replaceAll(data, TAG_NEXT_LINE, "\n");
    return data;
}

string RtcImsDialogHandler::replaceAll(string &str, const string &old_value, const string &new_value) {
    while(true) {
        string::size_type pos(0);
        if((pos=str.find(old_value)) != string::npos) {
            str.replace(pos, old_value.length(), new_value);
        } else {
            break;
        }
    }
    return str;
}

//============================================================

const string DepMessageHandler::DIALOG_INFO("dialog-info");
const string DepMessageHandler::DIALOG("dialog");
const string DepMessageHandler::VERSION("version");
const string DepMessageHandler::ID("id");
const string DepMessageHandler::SA_EXCLUSIVE("sa:exclusive");
const string DepMessageHandler::STATE("state");

const string DepMessageHandler::LOCAL("local");
const string DepMessageHandler::REMOTE("remote");
const string DepMessageHandler::IDENTITY("identity");
const string DepMessageHandler::TARGET("target");
const string DepMessageHandler::URI("uri");

const string DepMessageHandler::DIRECTION("direction");
const string DepMessageHandler::INITIATOR("initiator");
const string DepMessageHandler::RECEIVER("receiver");


const string DepMessageHandler::PARAM("param");
const string DepMessageHandler::PNAME("pname");
const string DepMessageHandler::PVAL("pval");

const string DepMessageHandler::MEDIA_ATTRIBUTES("mediaAttributes");
const string DepMessageHandler::MEDIA_TYPE("mediaType");
const string DepMessageHandler::MEDIA_DIRECTION("mediaDirection");
const string DepMessageHandler::PORT0("port0");

const string DepMessageHandler::TRUE("true");

DepMessageHandler::DepMessageHandler() :
    mDialogStart(false), mLocalStart(false), mMediaAttrStart(false),
    mRemoteStart(false), mVersion(-1), mDepState(DEP_STATE_UNKNOWN) {
}

DepMessageHandler::~DepMessageHandler() {
    for (sp<RfxDialog> dialog : mDialogInfo) {
        dialog->mediaAttributes.clear();
    }
    mDialogInfo.clear();
}

/**
 * To retrieve all dialog info.
 * @return all dialog info
 */
vector<sp<RfxDialog>> DepMessageHandler::getDialogInfo() {
    return mDialogInfo;
}

/**
 * To get dialog version
 * @return the dialog version
 */
int DepMessageHandler::getVersion() const {
    return mVersion;
}

void DepMessageHandler::startElement(string nodeName, string nodeValue, string attributeName,
        string attributeValue) {
    if (nodeName == DIALOG_INFO) {
        if (attributeName == VERSION) {
            mVersion = atoi(attributeValue.c_str());
            RFX_LOG_D(RFX_LOG_TAG, ":startElement version: %d", mVersion);
        }
    } else if (nodeName == DIALOG) {
        if (!mDialogStart) {
            mDialogStart = true;
            RFX_LOG_D(RFX_LOG_TAG, ":startElement dialog");
            mDialog = new RfxDialog();
        }
        if (attributeName == ID) {
            mDialog->dialogId = atoi(attributeValue.c_str());
            RFX_LOG_D(RFX_LOG_TAG, ":startElement dialogId: %s", attributeValue.c_str());
        }
        if (attributeName == DIRECTION) {
            if (attributeValue == RECEIVER) {
                mDialog->initiator = false;
            } else {
                mDialog->initiator = true;
            }
        }
    } else if (mDialogStart && nodeName == SA_EXCLUSIVE) {
        mDialog->exclusive = (nodeValue.compare(TRUE) == 0);
        RFX_LOG_D(RFX_LOG_TAG, "startElement sa:exclusive: %s", nodeValue.c_str());
    } else if (mDialogStart && nodeName == STATE) {
        mDialog->state = nodeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement state: %s", nodeValue.c_str());
    } else if (mDialogStart && nodeName == LOCAL) {
        mLocalStart = true;
        RFX_LOG_D(RFX_LOG_TAG, "startElement local");
    } else if (mLocalStart && nodeName == IDENTITY) {
        mDialog->identity = nodeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement identity: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, nodeValue.c_str()));
    } else if (mLocalStart && nodeName == TARGET) {
        if (attributeName == URI) {
            mDialog->targetUri = attributeValue;
            RFX_LOG_D(RFX_LOG_TAG, "startElement targetUri: %s",
                    RfxRilUtils::pii(RFX_LOG_TAG, attributeValue.c_str()));
        }
    } else if (mLocalStart && nodeName == MEDIA_ATTRIBUTES) {
        if (!mMediaAttrStart) {
            mMediaAttrStart = true;
            RFX_LOG_D(RFX_LOG_TAG, "startElement MediaAttribute");
            mMediaAttr = new RfxMediaAttribute();
        }
    } else if (mLocalStart && nodeName == PARAM) {
        // Param
        if (attributeName == PNAME) {
            mDialog->pname = attributeValue;
            RFX_LOG_D(RFX_LOG_TAG, ":startElement pname: %s", attributeValue.c_str());
        } else if (attributeName == PVAL) {
            mDialog->pval = attributeValue;
            RFX_LOG_D(RFX_LOG_TAG, ":startElement pval: %s", attributeValue.c_str());
        }
    } else if (mMediaAttrStart && nodeName == MEDIA_TYPE) {
        mMediaAttr->mediaType = nodeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement mediaType: %s", nodeValue.c_str());
    } else if (mMediaAttrStart && nodeName == MEDIA_DIRECTION) {
        mMediaAttr->mediaDirection = nodeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement mediaDirection: %s", nodeValue.c_str());
    } else if (mMediaAttrStart && nodeName == PORT0) {
        // This is a complete empty-element tag.
        mMediaAttr->port0 = true;
        RFX_LOG_D(RFX_LOG_TAG, "startElement port0 true");
    } else if (mDialogStart && nodeName == REMOTE) {
        mRemoteStart = true;
        RFX_LOG_D(RFX_LOG_TAG, "startElement remote");
    } else if (mRemoteStart && nodeName == IDENTITY) {
        mDialog->remoteIdentity = nodeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement remoteIdentity: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, nodeValue.c_str()));
    } else if (mRemoteStart && nodeName == TARGET) {
        if (attributeName == URI) {
            mDialog->remoteTargetUri = attributeValue;
            RFX_LOG_D(RFX_LOG_TAG, "startElement remoteTargetUri: %s",
                    RfxRilUtils::pii(RFX_LOG_TAG, attributeValue.c_str()));
        }
    } else if (mRemoteStart && nodeName == PARAM) {
        // Param
        if (attributeName == PNAME) {
            mDialog->remotePname = attributeValue;
            RFX_LOG_D(RFX_LOG_TAG, ":startElement remotePname: %s", attributeValue.c_str());
        } else if (attributeName == PVAL) {
            mDialog->remotePval = attributeValue;
            RFX_LOG_D(RFX_LOG_TAG, ":startElement remotePval: %s", attributeValue.c_str());
        }
    }
}

void DepMessageHandler::endElement(string nodeName) {
    if (nodeName == DIALOG) {
        mDialogStart = false;
        mDialogInfo.push_back(mDialog);
        RFX_LOG_D(RFX_LOG_TAG, "endElement end Dialog mDialogInfo");
    } else if (nodeName == MEDIA_ATTRIBUTES) {
        mMediaAttrStart = false;
        mDialog->mediaAttributes.push_back(mMediaAttr);
        RFX_LOG_D(RFX_LOG_TAG, "endElement end mediaAttributes");
    } else if (nodeName == LOCAL) {
        mLocalStart = false;
        RFX_LOG_D(RFX_LOG_TAG, "endElement end local");
    } else if (nodeName == REMOTE) {
        mRemoteStart = false;
        RFX_LOG_D(RFX_LOG_TAG, "endElement end remote");
    }
}

