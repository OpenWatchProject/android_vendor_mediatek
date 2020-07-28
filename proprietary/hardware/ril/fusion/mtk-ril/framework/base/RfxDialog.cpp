#include <stdio.h>
#include <memory>
#include <iostream>
#include <vector>
#include "RfxDialog.h"
#include "RfxRilUtils.h"

#include "RfxLog.h"
#define RFX_LOG_TAG "RfxDialog"

const string RfxDialog::AUDIO("audio");
const string RfxDialog::VIDEO("video");
const string RfxDialog::SIP_RENDERING("+sip.rendering");
const string RfxDialog::NO("no");

const string RfxDialog::CONFIRMED("confirmed");
const string RfxDialog::TRYING("trying");
const string RfxDialog::PROCEEDING("proceeding");
const string RfxDialog::EARLY("early");

const string RfxDialog::INACTIVE("inactive");
const string RfxDialog::SENDRECV("sendrecv");
const string RfxDialog::SENDONLY("sendonly");
const string RfxDialog::RECVONLY("recvonly");

RfxMediaAttribute::RfxMediaAttribute() :
        mediaType(""),
        mediaDirection(""),
        port0(false) {
}

RfxMediaAttribute::~RfxMediaAttribute(){
}

RfxDialog::RfxDialog() :
        dialogId(-1),
        exclusive(false),
        state(""),
        initiator(false),
        identity(""),
        targetUri(""),
        pname(""),
        pval(""),
        remoteIdentity(""),
        remoteTargetUri("") {
}

RfxDialog::~RfxDialog() {

}

int RfxDialog::getDialogId() {
    RFX_LOG_D(RFX_LOG_TAG, "getDialogId: %d", dialogId);
    return dialogId;
}

string RfxDialog::getAddress() {
    string address = identity;
    if (address.empty()) {
        address = targetUri;
    }
    RFX_LOG_D(RFX_LOG_TAG, "getAddress: %s", RfxRilUtils::pii(RFX_LOG_TAG, address.c_str()));
    return address;
}

string RfxDialog::getRemoteAddress() {
    string remoteAddress = remoteIdentity;
    if (remoteAddress.empty()) {
        remoteAddress = remoteTargetUri;
    }
    RFX_LOG_D(RFX_LOG_TAG, "getRemoteAddress: %s",
            RfxRilUtils::pii(RFX_LOG_TAG, remoteAddress.c_str()));
    return remoteAddress;
}

bool RfxDialog::isMt() {
    return !initiator;
}

bool RfxDialog::isCallHeld(vector<sp<RfxMediaAttribute>> mediaAttributes) {
    for (sp<RfxMediaAttribute> mediaAttribute : mediaAttributes) {
        if (strcasecmp(AUDIO.c_str(), mediaAttribute->mediaType.c_str()) == 0
                && strcasecmp(SENDRECV.c_str(), mediaAttribute->mediaDirection.c_str()) != 0) {
            return true;
        }
    }
    return false;
}

bool RfxDialog::isCallHeld() {
    bool isHeld = false;
    if (strcasecmp(SIP_RENDERING.c_str(), pname.c_str()) == 0
            && strcasecmp(NO.c_str(), pval.c_str()) == 0) {
        isHeld = true;
    } else if (isCallHeld(mediaAttributes)) {
        isHeld = true;
    }
    RFX_LOG_D(RFX_LOG_TAG, "isCallHeld:%d", isHeld);
    return isHeld;
}

bool RfxDialog::isVideoCallInBackground() {
    for (sp<RfxMediaAttribute> mediaAttribute : mediaAttributes) {
        if (strcasecmp(VIDEO.c_str(), mediaAttribute->mediaType.c_str()) == 0
                && strcasecmp(INACTIVE.c_str(), mediaAttribute->mediaDirection.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

bool RfxDialog::isPullable() {
    bool isPullable = true;
    if (exclusive) {
        isPullable = false;
    } else {
        if (isCallHeld()) {
            isPullable = false;
        } else if (isVideoCallInBackground()) {
            isPullable = false;
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "isPullable:%d", isPullable);
    return isPullable;
}

int RfxDialog::getCallState() {
    int callState = DialogCallState::CALL_STATE_TERMINATED;
    if (strcasecmp(CONFIRMED.c_str(), state.c_str()) == 0) {
        callState = DialogCallState::CALL_STATE_CONFIRMED;
    } else if (strcasecmp(EARLY.c_str(), state.c_str()) == 0) {
        callState = DialogCallState::CALL_STATE_EARLY;
    } else if (strcasecmp(TRYING.c_str(), state.c_str()) == 0) {
        callState = DialogCallState::CALL_STATE_TRYING;
    } else if (strcasecmp(PROCEEDING.c_str(), state.c_str()) == 0) {
        callState = DialogCallState::CALL_STATE_PROCEEDING;
    }
    RFX_LOG_D(RFX_LOG_TAG, "getCallState:%d", callState);
    return callState;
}

int RfxDialog::getCallType() {
    int callType = DialogCallType::CALL_TYPE_VOICE;

    for (sp<RfxMediaAttribute> mediaAttribute : mediaAttributes) {
        if (strcasecmp(AUDIO.c_str(), mediaAttribute->mediaType.c_str()) == 0) {
            continue;
        }
        if (mediaAttribute->port0) {
            continue;
        } else if (strcasecmp(INACTIVE.c_str(), mediaAttribute->mediaDirection.c_str()) == 0) {
            callType = DialogCallType::CALL_TYPE_VT_NODIR;
        } else if (strcasecmp(SENDRECV.c_str(), mediaAttribute->mediaDirection.c_str()) == 0) {
            callType = DialogCallType::CALL_TYPE_VT;
        } else if (strcasecmp(SENDONLY.c_str(), mediaAttribute->mediaDirection.c_str()) == 0) {
            callType = DialogCallType::CALL_TYPE_VT_TX;
        } else if (strcasecmp(RECVONLY.c_str(), mediaAttribute->mediaDirection.c_str()) == 0) {
            callType = DialogCallType::CALL_TYPE_VT_RX;
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "getCallType:%d", callType);
    return callType;
}

