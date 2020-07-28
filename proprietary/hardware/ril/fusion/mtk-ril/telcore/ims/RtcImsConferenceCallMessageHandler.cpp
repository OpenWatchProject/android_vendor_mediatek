/*
 * RtcImsConferenceHandler.cpp
 *
 */
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>

#include "RfxRootController.h"
#include "RfxLog.h"
#include "RfxRilUtils.h"
#include "RfxStringsData.h"
#include "utils/Timers.h"

#include "RtcImsConferenceCallMessageHandler.h"


#define RFX_LOG_TAG "RtcImsConference"

const string RtcImsConferenceCallMessageHandler::USER("user");
const string RtcImsConferenceCallMessageHandler::DISPLAY_TEXT("display-text");
const string RtcImsConferenceCallMessageHandler::ENDPOINT("endpoint");
const string RtcImsConferenceCallMessageHandler::STATUS("status");
const string RtcImsConferenceCallMessageHandler::HOST_INFO("host-info");
const string RtcImsConferenceCallMessageHandler::URI("uri");
const string RtcImsConferenceCallMessageHandler::STATE("state");
const string RtcImsConferenceCallMessageHandler::ENTITY("entity");
const string RtcImsConferenceCallMessageHandler::USER_COUNT("user-count");
const string RtcImsConferenceCallMessageHandler::MAX_USER_COUNT("maximum-user-count");
const string RtcImsConferenceCallMessageHandler::VERSION("version");
const string RtcImsConferenceCallMessageHandler::XML_TAG("xmlns");
const string RtcImsConferenceCallMessageHandler::CONFERENCE_INFO("conference-info");
const string RtcImsConferenceCallMessageHandler::MEDIA("media");

const string RtcImsConferenceCallMessageHandler::STATUS_PENDING("pending");
const string RtcImsConferenceCallMessageHandler::STATUS_DIALING_OUT("dialing-out");
const string RtcImsConferenceCallMessageHandler::STATUS_DIALING_IN("dialing-in");
const string RtcImsConferenceCallMessageHandler::STATUS_ALERTING("alerting");
const string RtcImsConferenceCallMessageHandler::STATUS_ON_HOLD("on-hold");
const string RtcImsConferenceCallMessageHandler::STATUS_CONNECTED("connected");
const string RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTING("disconnecting");
const string RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTED("disconnected");
const string RtcImsConferenceCallMessageHandler::STATUS_MUTED_VIA_FOCUS("muted-via-focus");
const string RtcImsConferenceCallMessageHandler::STATUS_CONNECT_FAIL("connect-fail");
const string RtcImsConferenceCallMessageHandler::SIP_STATUS_CODE("sipstatuscode");

/******************************************************
 * RtcImsConferenceCallMessageHandler
 *****************************************************/

RtcImsConferenceCallMessageHandler::RtcImsConferenceCallMessageHandler() :
        mCallId(-1),
        mIndex(0),
        mUserCount(-1),
        mMaxUserCount(0),
        mParsingHostInfo(false),
        mVersion(-1),
        mCEPState(CEP_STATE_UNKNOWN),
        mHostInfo(""),
        mUser(NULL),
        mMediaStart(false) {
}

RtcImsConferenceCallMessageHandler::~RtcImsConferenceCallMessageHandler(){
}

/**
 * To retrieve all conf. call participants.
 *
 * @return all conf. call participants
 */
vector<sp<ConferenceCallUser>> RtcImsConferenceCallMessageHandler::getUsers() const {
    return mUsers;
}

/*
 * To set the maximum count of participants.
 *
 * @param maxUserCount the maximum count of participants
 */
void RtcImsConferenceCallMessageHandler::setMaxUserCount(string maxUserCount) {
    mMaxUserCount = atoi(maxUserCount.c_str());
}

/**
 * To retrieve the maimum count of participants.
 *
 * @return the maximum count of participants
 */
int RtcImsConferenceCallMessageHandler::getMaxUserCount() const {
    return mMaxUserCount;
}

/**
 * To set call is.
 *
 * @param callId the call id
 */
void RtcImsConferenceCallMessageHandler::setCallId(const int &callId) {
    mCallId = callId;
}

/**
 * To retrieve call id
 *
 * @return call id
 */
int RtcImsConferenceCallMessageHandler::getCallId() const {
    return mCallId;
}

/**
 * To retrieve the count of participants.
 *
 * @return the count of participants
 */
int RtcImsConferenceCallMessageHandler::getUserCount() const {
    return mUserCount;
}

/**
 * To retrieve the host information
 *
 * @return the host information
 */
string RtcImsConferenceCallMessageHandler::getHostInfo() const {
    return mHostInfo;
}

/**
 * To get conference version
 *
 * @return the conference version
 */
int RtcImsConferenceCallMessageHandler::getVersion() const {
    return mVersion;
}

/**
 * To retrieve the CEP State
 *
 * @return the CEP State
 */
int RtcImsConferenceCallMessageHandler::getCEPState() const {
    return mCEPState;
}

int RtcImsConferenceCallMessageHandler::updateCEPState(string val) {
    if (val.empty()) {
        return CEP_STATE_UNKNOWN;
    } else if (val == "full") {
        return CEP_STATE_FULL;
    } else if (val == "partial") {
        return CEP_STATE_PARTIAL;
    } else {
        return CEP_STATE_UNKNOWN;
    }
}

void RtcImsConferenceCallMessageHandler::startElement(string nodeName, string nodeValue,
        string attributeName, string attributeValue) {
    if (nodeName == CONFERENCE_INFO) {
        if (attributeName == VERSION) {
            mVersion = atoi(attributeValue.c_str());
            RFX_LOG_D(RFX_LOG_TAG, ":startElement version: %d", mVersion);
        } else if (attributeName == STATE) {
            mCEPState = updateCEPState(attributeValue);
            RFX_LOG_D(RFX_LOG_TAG, ":startElement mCEPState: %d", mCEPState);
        } else if (attributeName == XML_TAG) {
            RFX_LOG_D(RFX_LOG_TAG, "startElement Conference uri: %s", attributeName.data());
        }
    } else if (nodeName == USER && attributeName == ENTITY) {
        mIndex++;
        mUser = new ConferenceCallUser();
        mUser->mIndex = mIndex;
        mUser->mEntity = attributeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement user - entity: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, mUser->mEntity.data()));
    } else if (mUser != NULL && nodeName == ENDPOINT && attributeName == ENTITY) {
        mUser->mEndPoint = attributeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement endpoint - entity: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, mUser->mEndPoint.data()));
    } else if (nodeName == HOST_INFO) {
        mParsingHostInfo = true;
        RFX_LOG_D(RFX_LOG_TAG, "startElement start parsing host info");
    } else if (nodeName == MAX_USER_COUNT) {
        setMaxUserCount(nodeValue);
        RFX_LOG_D(RFX_LOG_TAG, "startElement MaxUserCount: %d", getMaxUserCount());
    } else if (nodeName ==  USER_COUNT) {
        mUserCount = atoi(nodeValue.data());
        RFX_LOG_D(RFX_LOG_TAG, "startElement UserCount: %d", getUserCount());
    } else if (mParsingHostInfo && nodeName == URI) {
        mHostInfo = nodeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement host-info: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, getHostInfo().data()));
    } else if (mUser != NULL && nodeName == DISPLAY_TEXT) {
        mUser->mDisplayText = nodeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement display-text: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, nodeValue.data()));
    // ALPS04747734: Ignore the status in media tag.
    } else if (mUser != NULL && nodeName == STATUS && !mMediaStart) {
        mUser->mStatus = nodeValue;
        RFX_LOG_D(RFX_LOG_TAG, "startElement status: %s", nodeValue.data());
    } else if (mUser != NULL && nodeName == MEDIA) {
        mMediaStart = true;
        RFX_LOG_D(RFX_LOG_TAG, "startElement media");
    }
}

void RtcImsConferenceCallMessageHandler::endElement(string nodeName) {
    if (nodeName == USER) {
        mUsers.push_back(mUser);
        RFX_LOG_D(RFX_LOG_TAG, "endElement end user mUsers.size: %zu", mUsers.size());
    } else if (nodeName == HOST_INFO) {
        mParsingHostInfo = false;
        RFX_LOG_D(RFX_LOG_TAG, "endElement end host-info");
    } else if (nodeName == MEDIA) {
        mMediaStart = false;
        RFX_LOG_D(RFX_LOG_TAG, "endElement end media");
    }
}

ConferenceCallUser::ConferenceCallUser() : mEndPoint(""),
        mEntity(""), mDisplayText(""),mStatus(""),
        mUserAddr(""), mIndex(0) {
}

ConferenceCallUser::~ConferenceCallUser() {
}
