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
#include "RfxXmlParser.h"
#include "utils/Timers.h"

#include "MtkPhoneNumberUtils.h"
#include "RtcImsConferenceController.h"
#include "RtcImsConferenceHandler.h"

#define RFX_LOG_TAG "RtcImsConference"

const string RtcImsConferenceHandler::TAG_NEXT_LINE("<ascii_10>");
const string RtcImsConferenceHandler::TAG_RETURN("<ascii_13>");
const string RtcImsConferenceHandler::TAG_DOUBLE_QUOTE("<ascii_34>");

#define ANONYMOUS_URI  "sip:anonymous@anonymous.invalid"

RtcImsConferenceHandler::RtcImsConferenceHandler(int slot) :
        mIsCepNotified(false),
        mConfCallId(-1),
        mCepVersion(-1),
        mHostAddr(""),
        mAddingParticipant("null"),
        mRemovingParticipant("null")
{
    m_slot_id = slot;
    RFX_LOG_D(RFX_LOG_TAG, "RtcImsConferenceHandler()");
}

RtcImsConferenceHandler::~RtcImsConferenceHandler() {
    RFX_LOG_D(RFX_LOG_TAG, "~RtcImsConferenceHandler()");
}

void RtcImsConferenceHandler::startConference() {
    mIsConferenceActive = true;
    mRestoreImsConferenceParticipant =
            checkCarrierConfig(RFX_STATUS_KEY_RESTORE_IMS_CONFERENCE_PARTICIPANT);
}

void RtcImsConferenceHandler::closeConference() {
    RFX_LOG_D(RFX_LOG_TAG, "closeConference()");
    mConfCallId = -1;
    mCepVersion = -1;
    mIsCepNotified = false;
    mRemovingParticipant = "null";
    mAddingParticipant = "null";
    mHostAddr = "";
    // clean the number variable
    mLocalParticipants.clear();
    mConfParticipants.clear();
    mParticipants.clear();
    mParticipantsAddrMap.clear();
    mFirstMergeParticipants.clear();
    mRestoreImsConferenceParticipant = false;
    mIsConferenceActive = false;
    mHaveUpdatedConferenceWithMember = false;
    mOneKeyAddingParticipants.clear();
    mLastConfParticipants.clear();
    mConnectedCount = 0;
}

void RtcImsConferenceHandler::firstMerge(string callId_1, string callId_2, string num_1, string num_2) {
    RFX_LOG_D(RFX_LOG_TAG,
            "firstMerge, callId_1: %s, callId_2: %s, num_1: %s, num_2: %s",
            RfxRilUtils::pii(RFX_LOG_TAG, callId_1.data()), RfxRilUtils::pii(RFX_LOG_TAG, callId_2.data()),
            RfxRilUtils::pii(RFX_LOG_TAG, num_1.data()), RfxRilUtils::pii(RFX_LOG_TAG, num_2.data()));
    mFirstMergeParticipants.clear();
    mFirstMergeParticipants.insert(pair<string, string>(callId_1,
            encodeSpecialChars(normalizeNumberFromCLIR(num_1))));
    mFirstMergeParticipants.insert(pair<string, string>(callId_2,
            encodeSpecialChars(normalizeNumberFromCLIR(num_2))));
}

void RtcImsConferenceHandler::tryAddParticipant(string addr) {
    string number = getUserNameFromSipTelUriString(addr);
    number = encodeSpecialChars(normalizeNumberFromCLIR(number));
    mAddingParticipant = number;
}

void RtcImsConferenceHandler::tryOneKeyAddParticipant(string addr) {
    string number = getUserNameFromSipTelUriString(addr);
    number = encodeSpecialChars(normalizeNumberFromCLIR(number));
    if (!isContainParticipant(mOneKeyAddingParticipants, number)) {
        mOneKeyAddingParticipants.push_back(number);
    }
}

void RtcImsConferenceHandler::tryRemoveParticipant(string addr) {
    string number = getUserNameFromSipTelUriString(addr);
    number = normalizeNumberFromCLIR(number);
    mRemovingParticipant = number;
}

bool RtcImsConferenceHandler::modifyParticipantComplete() {
    RFX_LOG_D(RFX_LOG_TAG, "modifyParticipantComplete");
    bool isFirstMerge = (mAddingParticipant == "null" && mRemovingParticipant == "null");
    bool isContain = isContainParticipant(mLocalParticipants, mAddingParticipant);
    if (mAddingParticipant != "null" && (!isContain || mAddingParticipant == "")) {
        mLocalParticipants.push_back(mAddingParticipant);
        mAddingParticipant = "null";
    }

    for (int i = 0; i < (int)mLocalParticipants.size(); i++) {
        RFX_LOG_D(RFX_LOG_TAG, "modifyParticipantComplete: "
                "mLocalParticipants:%s", RfxRilUtils::pii(RFX_LOG_TAG, mLocalParticipants[i].data()));
    }
    return mIsCepNotified && isFirstMerge;
}

void RtcImsConferenceHandler::modifyParticipantFailed() {
    RFX_LOG_D(RFX_LOG_TAG, "modifyParticipantFailed");
    mRemovingParticipant = "null";
    mAddingParticipant = "null";
    if ((int)mOneKeyAddingParticipants.size() > 0) {
        mOneKeyAddingParticipants.pop_back();
    }
}

string RtcImsConferenceHandler::getUserNameFromSipTelUriString(string uriString) {
    RFX_LOG_D(RFX_LOG_TAG,
        "getUserNameFromSipTelUriString uriString: %s", RfxRilUtils::pii(RFX_LOG_TAG, uriString.data()));
    if (uriString.empty()) {
        return "";
    }
    //Uri uri = Uri.parse(uriString);
    // Get the address part, i.e. everything between 'sip:' and the fragment separator '#'.
    // ex: '+8618407404132@10.185.184.137:5087;transport=UDP'
    // or '1234;phone-context=munich.example.com;isub=@1234'
    vector<string> hostParts = splitString(uriString, ";");
    string headParts = hostParts[0];
    vector<string> addressParts = splitString(headParts, ":");
    string addressPart = addressParts[0];
    if(addressParts.size() > 1) {
        addressPart = addressParts[1];
    }
    vector<string> umberParts = splitString(addressPart, "@");
    string address = umberParts[0];//uri.getSchemeSpecificPart();
    if (address.empty()) {
        return "";
    }
    string userName = MtkPhoneNumberUtils::getUsernameFromUriNumber(address);
    if (userName.empty()) {
        return "";
    }

    int pIndex = userName.find(";");
    int wIndex = userName.find(",");
    if (pIndex >= 0 && wIndex >= 0) {
        userName = userName.substr(0, std::min(pIndex,wIndex));
    } else if (pIndex >=0) {
        userName = userName.substr(0, pIndex);
    } else if (wIndex >=0) {
        userName = userName.substr(0, wIndex);
    }
    return userName;
}

RtcImsConferenceCallMessageHandler* RtcImsConferenceHandler::parseXmlPackage(string data) {
    RtcImsConferenceCallMessageHandler* parsedData = new RtcImsConferenceCallMessageHandler();
    sp<RfxXmlParser> parser = new RfxXmlParser();
    // RFX_LOG_V(RFX_LOG_TAG, "parseXmlPackage data: %s", data.data());
    parser->parse(parsedData, data);
    //Read conference data and parse it
    return parsedData;
}

void RtcImsConferenceHandler::updateConferenceStateWithLocalCache() {
    RFX_LOG_D(RFX_LOG_TAG, "updateConferenceStateWithLocalCache");
    if (mIsCepNotified) {
        return;
    }

    mParticipants.clear();

    if (mRemovingParticipant != "null") {
        removeParticipant(mLocalParticipants, mRemovingParticipant, true);
        mRemovingParticipant = "null";
    }
    int count = (int)mLocalParticipants.size();
    for (int i = 0; i < count; i++) {
        string addr = mLocalParticipants[i];
        sp<ConferenceCallUser> user = new ConferenceCallUser();
        user->mUserAddr = addr;
        user->mDisplayText = addr;
        user->mEndPoint = addr;
        user->mStatus = RtcImsConferenceCallMessageHandler::STATUS_CONNECTED;
        user->mEntity = addr;
        mParticipants.push_back(user);
        RFX_LOG_D(RFX_LOG_TAG, "updateConferenceStateWithLocalCache: "
                "mLocalParticipants %d:%s", i, RfxRilUtils::pii(RFX_LOG_TAG, addr.data()));
    }

    // Terminate the empty conference.
    bool autoTerminate = false;
    if (count == 0 && mHaveUpdatedConferenceWithMember) {
        RFX_LOG_D(RFX_LOG_TAG,
                "updateConferenceStateWithLocalCache, no participants, terminate the conference");
        autoTerminate = true;
    } else if (count != 0 && !mHaveUpdatedConferenceWithMember) {
        RFX_LOG_D(RFX_LOG_TAG,
                "updateConferenceStateWithLocalCache, set mHaveUpdatedConferenceWithMember = true");
        mHaveUpdatedConferenceWithMember = true;
    }

    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    RtcImsConferenceController *ctrl =
            (RtcImsConferenceController *)root->findController(m_slot_id,
                    RFX_OBJ_CLASS_INFO(RtcImsConferenceController));
    if (ctrl != NULL) {
        ctrl->onParticipantsUpdate(autoTerminate);
    }
}

void RtcImsConferenceHandler::setupHost(RtcImsConferenceCallMessageHandler* xmlData){
    //get host address from the optional xml element <host-info>
    RFX_LOG_D(RFX_LOG_TAG, "setupHost");
    string hostAddr = getUserNameFromSipTelUriString(xmlData->getHostInfo());

    bool isContain = hostAddr.find(' ') != string::npos;
    while (!hostAddr.empty() && isContain) {
        hostAddr.erase(hostAddr.find(' '));
    }
    if (!hostAddr.empty()) {
        mHostAddr = hostAddr;
    }
}

bool RtcImsConferenceHandler::isSelfAddress(string address) {
    if (address.empty()) return false;
    RFX_LOG_D(RFX_LOG_TAG,
        "isSelfAddress(): address: %s, mHostAddr: %s", RfxRilUtils::pii(RFX_LOG_TAG, address.data()),
        RfxRilUtils::pii(RFX_LOG_TAG, mHostAddr.data()));
    if (mHostAddr.compare(address) == 0 || MtkPhoneNumberUtils::compareLoosely(mHostAddr, address)) {
        RFX_LOG_D(RFX_LOG_TAG, "isSelfAddress(): true, meet host info in xml");
        return true;
    }

    RfxRootController* root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    Vector<String8> xuis = root->getStatusManager(m_slot_id)->getString8VectorValue(
            RFX_STATUS_KEY_XUI_INFO);
    for (int i = 0; i < (int)xuis.size(); ++i) {
        string xui = getUserNameFromSipTelUriString(xuis[i].string());
        if (xui.empty()) continue;
        if (MtkPhoneNumberUtils::compareLoosely(address, xui)) {
            return true;
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "isSelfAddress(): false");
    return false;
}

bool RtcImsConferenceHandler::updateParticipants(sp<ConferenceCallUser> oldParticipant, string to) {
    bool updated = false;
    string from = oldParticipant->mUserAddr;

    RFX_LOG_D(RFX_LOG_TAG, "updateParticipants() : from %s to %s",
            RfxRilUtils::pii(RFX_LOG_TAG, from.data()), RfxRilUtils::pii(RFX_LOG_TAG, to.data()));

    for (int i = 0; i < (int)mConfParticipants.size(); i++) {
        sp<ConferenceCallUser> participant = mConfParticipants[i];

        if (participant->mUserAddr == from && participant->mEndPoint == oldParticipant->mEndPoint) {
            if (from != to) {
                participant->mUserAddr = to;
                insertParticipantsAddrMap(to , from);
            }
            updated = true;
            break;
        }
    }

    if (!updated) {
        RFX_LOG_D(RFX_LOG_TAG, "updateParticipants() : add new item");

        oldParticipant->mUserAddr = to;
        insertParticipantsAddrMap(to , from);
        mConfParticipants.push_back(oldParticipant);
    }

    return updated;
}

void RtcImsConferenceHandler::insertParticipantsAddrMap(string key, string value) {
    map<string, string>::iterator iter = mParticipantsAddrMap.find(key);
    if (iter != mParticipantsAddrMap.end()) {
        iter->second = value;
    } else {
        mParticipantsAddrMap.insert(pair<string, string>(key , value));
    }
}

void RtcImsConferenceHandler::showCacheAndXmlData(string callerName) {
    RFX_LOG_D(RFX_LOG_TAG, "showCacheAndXmlData()  : call by %s ", callerName.data());

    RFX_LOG_D(RFX_LOG_TAG, " - mLocalParticipants =>");
    for (int i = 0; i < (int)mLocalParticipants.size(); i++) {
        RFX_LOG_D(RFX_LOG_TAG, "    + element : %s",
                RfxRilUtils::pii(RFX_LOG_TAG, mLocalParticipants[i].data()));
    }

    RFX_LOG_D(RFX_LOG_TAG, " - mConfParticipants =>");
    for (int i = 0; i < (int)mConfParticipants.size(); i++) {
        sp<ConferenceCallUser> participant = mConfParticipants[i];
        RFX_LOG_D(RFX_LOG_TAG, "    + element : %s %s",
                RfxRilUtils::pii(RFX_LOG_TAG, participant->mUserAddr.data()),
                participant->mStatus.data());
    }

    RFX_LOG_D(RFX_LOG_TAG, " - mUnknownParticipants =>");
    for (int i = 0; i < (int)mUnknownParticipants.size(); i++) {
        sp<ConferenceCallUser> participant = mUnknownParticipants[i];
        RFX_LOG_D(RFX_LOG_TAG, "    + element : %s",
                RfxRilUtils::pii(RFX_LOG_TAG, participant->mUserAddr.data()));
    }
}

void RtcImsConferenceHandler::restoreParticipantsAddressByLocalCache() {
    RFX_LOG_D(
            RFX_LOG_TAG,
            "restoreParticipantsAddressByLocalCache() :  Local() = %d , Xml.C = %d, , Xml.U = %d",
            (int)mLocalParticipants.size(),
            (int)mConfParticipants.size(),
            (int)mUnknownParticipants.size());

    showCacheAndXmlData("restoreParticipantsAddressByLocalCache (before)");

    //copy mLocalParticipants
    vector<string> LocalUnMatchParticipants(mLocalParticipants);
    //copy mConfParticipants
    vector<sp<ConferenceCallUser>> XmlParticipants(mConfParticipants);

    vector<sp<ConferenceCallUser>> XmlUnMatchParticipants;

    for(int i = 0; i < (int)XmlParticipants.size(); i++) {

        sp<ConferenceCallUser> participant = XmlParticipants[i];
        string userHandle = getPairedAddress(participant->mUserAddr);

        if (participant->mStatus == RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTED) {
            // do not restore the disconnected user, the disconnected user does not contain
            // in the cached
            continue;
        }

        if ((checkCarrierConfig(RFX_STATUS_KEY_IMS_CONFERENCE_FIRST_PARTICIPANT_AS_HOST) ||
                isSelfAddress(userHandle) == false) &&
                isContainParticipant(LocalUnMatchParticipants, userHandle) == false) {
           // Not self and match failed, keep addr and wait for restore.
            // but if RFX_STATUS_KEY_IMS_CONFERENCE_FIRST_PARTICIPANT_AS_HOST is true,
            // don't check isSelf because host should not be here
            XmlUnMatchParticipants.push_back(participant);

        } else {
            // update paired data
            updateParticipants(participant, userHandle);

            // remove paired address from restoreCandidate
            removeParticipant(LocalUnMatchParticipants, userHandle, true);
        }
    }

    // use the "not paired" local address to restored the special user entity
    for(int i = 0; i < (int)XmlUnMatchParticipants.size(); i++) {

        sp<ConferenceCallUser> participant = XmlUnMatchParticipants[i];
        string userHandle = participant->mUserAddr;

        if ((int)LocalUnMatchParticipants.size() == 0) {
            // No more candidates, just ignore the data.
            break;
        }

        // Always get first candidate to restore.
        string restoreHandle = LocalUnMatchParticipants[0];
        LocalUnMatchParticipants.erase(LocalUnMatchParticipants.begin());

        RFX_LOG_D(
            RFX_LOG_TAG,
            "restoreParticipantsAddressByLocalCache() : restore participants %s to: %s",
            RfxRilUtils::pii(RFX_LOG_TAG, userHandle.data()),
            RfxRilUtils::pii(RFX_LOG_TAG, restoreHandle.data()));

        // update restore data
        updateParticipants(participant, restoreHandle);

    }

    restoreUnknowParticipants(LocalUnMatchParticipants);

    // show restore result
    //
    showCacheAndXmlData("restoreParticipantsAddressByLocalCache (after)");

}

void RtcImsConferenceHandler::restoreUnknowParticipants(vector<string> LocalUnMatchParticipants) {
    //copy mConfParticipants
    vector<sp<ConferenceCallUser>> unkownXmlParticipants(mUnknownParticipants);

    int restoreIndex = 0;

    for(int i = 0; i < (int)mUnknownParticipants.size(); i++) {

        sp<ConferenceCallUser> participant = mUnknownParticipants[i];

        if ((int)LocalUnMatchParticipants.size() <= restoreIndex) {
            unkownXmlParticipants.push_back(participant);
            continue;
        }

        string userHandle = LocalUnMatchParticipants[restoreIndex];

        // update data
        updateParticipants(participant, userHandle);

        if(unkownXmlParticipants.size() > 0) {
            unkownXmlParticipants.erase(unkownXmlParticipants.begin());
        }

        RFX_LOG_D(
            RFX_LOG_TAG,
            "restoreUnknowParticipants() : restore unknown participants( %d ) to: %s",
            restoreIndex,
            RfxRilUtils::pii(RFX_LOG_TAG, userHandle.data()));

        restoreIndex++;
    }

    mUnknownParticipants = unkownXmlParticipants;

}

void RtcImsConferenceHandler::fullUpdateParticipants(vector<sp<ConferenceCallUser>> users) {
    RFX_LOG_D(RFX_LOG_TAG, "fullUpdateParticipants");
    mConfParticipants.clear();
    mUnknownParticipants.clear();

    for (int i = 0; i < (int)users.size(); i++) {
        sp<ConferenceCallUser> user = users[i];
        string userAddr = getUserNameFromSipTelUriString(user->mEntity);

        if (i == 0 && checkCarrierConfig(RFX_STATUS_KEY_IMS_CONFERENCE_FIRST_PARTICIPANT_AS_HOST) &&
                (int)users.size() > (int)mLocalParticipants.size()) {
            /*
             * For the operation don't provide host-info in CEP and set the host entity as anonymous
             * don't update the host as conference participant because it could not matched
             * to the host address from XUI in telephony FW, just skip.
             */
            RFX_LOG_D(RFX_LOG_TAG, "skip the 1st participant if consider it as host");
            continue;
        }

        while (!userAddr.empty() && userAddr.find(' ') != string::npos) {
            userAddr.erase(userAddr.find(' '));
        }
        if (userAddr.empty()) {
            user->mUserAddr = "";
            mUnknownParticipants.push_back(user);
            continue;
        }
        user->mUserAddr = userAddr;
        mConfParticipants.push_back(user);
    }
}

void RtcImsConferenceHandler::partialUpdateParticipants(vector<sp<ConferenceCallUser>> users) {
    RFX_LOG_D(RFX_LOG_TAG, "partialUpdateParticipants");

    // Clear disconnected participant.
    vector<sp<ConferenceCallUser>>::iterator iter = mConfParticipants.begin();
    while (iter != mConfParticipants.end()) {
        sp<ConferenceCallUser> participant = (sp<ConferenceCallUser>)*iter;
        if (participant->mStatus == RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTED) {
            iter = mConfParticipants.erase(iter);
        } else {
            ++iter;
        }
    }
    for (int i = 0; i < (int)users.size(); i++) {
        sp<ConferenceCallUser> user = users[i];
        string originalUserAddr = getUserNameFromSipTelUriString(user->mEntity);

        while (!originalUserAddr.empty() && originalUserAddr.find(' ') != string::npos) {
            originalUserAddr.erase(originalUserAddr.find(' '));
        }

        string userAddr = originalUserAddr;
        if (mRestoreImsConferenceParticipant) {
            userAddr = getPairedRestoredAddress(userAddr);
        }
        user->mUserAddr = userAddr;
        string status = user->mStatus;

        // Carefully, for "" user-enity case, the userAddr may get wrong.
        RFX_LOG_D(RFX_LOG_TAG, "partialUpdateParticipants original: %s, userAddr: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, originalUserAddr.data()),
                RfxRilUtils::pii(RFX_LOG_TAG, user->mUserAddr.data()));

        // Handle special case. We assume normally partial update only update the change.
        if (originalUserAddr.empty()) {
            // Case 1: "" and Connected is a new unknown participant.
            if (status == RtcImsConferenceCallMessageHandler::STATUS_CONNECTED) {
                if (!user->mUserAddr.empty()) {
                    user->mUserAddr = "";
                }
                mUnknownParticipants.push_back(user);
                RFX_LOG_D(RFX_LOG_TAG, "add unknown participants");
                continue;

            // Case 2: "" and Disconnected with not enable restore.
            //         Remove an unknown participant.
            } else if (!mRestoreImsConferenceParticipant && status ==
                    RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTED) {
                // remove last unknown participants
                if (mUnknownParticipants.size() > 0) {
                    mUnknownParticipants.pop_back();
                    RFX_LOG_D(RFX_LOG_TAG, "remove unknown participants");
                }
                continue;
            }
        }

        // Case 3: "" with enable restore or others. Just update as normal case.
        if (!(status == RtcImsConferenceCallMessageHandler::STATUS_DIALING_OUT)) {
            for (int j = 0; j < (int)mConfParticipants.size(); j++) {
                if ((mConfParticipants[j]->mEntity.compare(user->mEntity) == 0
                        // For Vodafone ES.
                        || mConfParticipants[j]->mEntity.empty()) &&
                        mConfParticipants[j]->mEndPoint.compare(user->mEndPoint) == 0) {
                    user->mUserAddr = mConfParticipants[j]->mUserAddr;
                    mConfParticipants.erase(mConfParticipants.begin() + j);
                    RFX_LOG_D(RFX_LOG_TAG, "Find participant, update it.");
                    break;
                }
            }
            mConfParticipants.push_back(user);
        }
    }
}

void RtcImsConferenceHandler::tryOneKeyAddLocalList(vector<string> users) {
    for (int i = 0; i < (int)users.size(); i++) {
        tryOneKeyAddParticipant(users[i]);
    }
}

void RtcImsConferenceHandler::updateLocalCache(int cepState) {
    int disconnectedCount = 0;
    int connectedCount = 0;
    for (int i = 0; i < (int)mConfParticipants.size(); i++) {
        if (RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTED.compare(
                mConfParticipants[i]->mStatus) == 0) {
            disconnectedCount++ ;
        } else {
            if (!isSelfAddress(mConfParticipants[i]->mUserAddr)) {
                connectedCount++;
            }
        }
    }

    // ALPS04532167, Removed participant may keep in CEP with disconnected status.
    // So we remove number from local list only when some participants dropped
    // and local list size larger than connected participant count.
    int localParticipantsSize = (int)mLocalParticipants.size();
    int oneKeyAddingParticipantsSize = (int)mOneKeyAddingParticipants.size();
    if (disconnectedCount && localParticipantsSize > connectedCount) {
        //copy mLastConfParticipants
        vector<sp<ConferenceCallUser>> unMatchedLastParticipants(mLastConfParticipants);
        for (int i = 0; i < (int)mConfParticipants.size(); i++) {
            if (RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTED.compare(
                    mConfParticipants[i]->mStatus) == 0) {
                // ALPS04948668: Removed participant from local list only when its
                // new disconnected.
                if (cepState == RtcImsConferenceCallMessageHandler::CEP_STATE_FULL) {
                    vector<sp<ConferenceCallUser>>::iterator iter = unMatchedLastParticipants.begin();
                    bool isNewDisconnected = true;
                    while (iter != unMatchedLastParticipants.end()) {
                        sp<ConferenceCallUser> lastParticipant = (sp<ConferenceCallUser>)*iter;
                        if (RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTED.compare(
                                lastParticipant->mStatus) == 0 &&
                                lastParticipant->mUserAddr == mConfParticipants[i]->mUserAddr) {
                            iter = unMatchedLastParticipants.erase(iter);
                            isNewDisconnected = false;
                            break;
                        } else {
                            ++iter;
                        }
                    }
                    if (!isNewDisconnected) {
                        continue;
                    }
                }

                string restoredAddress = getPairedRestoredAddress(mConfParticipants[i]->mUserAddr);
                if (mRemovingParticipant != "null") {
                    // ALPS04297642, User want to remove A but B disconnected, we assume the mapping was wrong,
                    // So we remove the both mapping.
                    if (cepState == RtcImsConferenceCallMessageHandler::CEP_STATE_FULL) {
                        if (disconnectedCount == 1 && restoredAddress != mRemovingParticipant) {
                            restoredAddress = mRemovingParticipant;
                        }
                    }
                    mRemovingParticipant = "null";
                }
                RFX_LOG_D(RFX_LOG_TAG, "updateLocalCache: remove %s",
                        RfxRilUtils::pii(RFX_LOG_TAG, restoredAddress.data()));
                // ALPS04672817, Only remove number from local list when exactly match.
                removeParticipant(mLocalParticipants, restoredAddress, false);
                mParticipantsAddrMap.erase(restoredAddress);
            }
        }
    // Hanle one key invite case.
    } else if (oneKeyAddingParticipantsSize > 0 &&
            connectedCount > mConnectedCount && connectedCount > localParticipantsSize) {
        int newJoinedCount = connectedCount - mConnectedCount;

        RFX_LOG_D(RFX_LOG_TAG, "updateLocalCache: newJoinedCount %d, oneKeyAdding %d",
                newJoinedCount, oneKeyAddingParticipantsSize);
        for (int j = 0; j < oneKeyAddingParticipantsSize; j++) {
            RFX_LOG_D(RFX_LOG_TAG, "updateLocalCache: mOneKeyAddingParticipants:%s",
                    RfxRilUtils::pii(RFX_LOG_TAG, mOneKeyAddingParticipants[j].data()));
        }
        // All joined conference.
        if (newJoinedCount == oneKeyAddingParticipantsSize
                || connectedCount - localParticipantsSize == oneKeyAddingParticipantsSize) {
            for (int j = 0; j < oneKeyAddingParticipantsSize; j++) {
                RFX_LOG_D(RFX_LOG_TAG, "updateLocalCache: add %s",
                        RfxRilUtils::pii(RFX_LOG_TAG, mOneKeyAddingParticipants[j].data()));
                mLocalParticipants.push_back(mOneKeyAddingParticipants[j]);
            }
            mOneKeyAddingParticipants.clear();
        // Only some joined conference.
        } else if (newJoinedCount < oneKeyAddingParticipantsSize) {
            vector<string>::iterator iter = mOneKeyAddingParticipants.begin();
            while (iter != mOneKeyAddingParticipants.end()) {
                bool matched = false;
                for (int i = 0; i < (int)mConfParticipants.size(); i++) {
                    if (RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTED.compare(
                        mConfParticipants[i]->mStatus) != 0) {
                        if (MtkPhoneNumberUtils::compareLoosely(mConfParticipants[i]->mUserAddr,
                                (string)*iter)) {
                            RFX_LOG_D(RFX_LOG_TAG, "updateLocalCache: add %s",
                                    RfxRilUtils::pii(RFX_LOG_TAG,
                                    ((string)*iter).data()));
                            mLocalParticipants.push_back((string)*iter);
                            iter = mOneKeyAddingParticipants.erase(iter);
                            matched = true;
                        }
                    }
                }
                if (!matched) {
                    ++iter;
                }
            }
        }
    }

    if (mAddingParticipant != "null" && connectedCount > mConnectedCount
            && connectedCount > localParticipantsSize) {
        RFX_LOG_D(RFX_LOG_TAG, "updateLocalCache: add %s",
                RfxRilUtils::pii(RFX_LOG_TAG, mAddingParticipant.data()));
        mLocalParticipants.push_back(mAddingParticipant);
        mAddingParticipant = "null";
    }
    mConnectedCount = connectedCount;
}

void RtcImsConferenceHandler::notifyConfStateUpdate() {
    RFX_LOG_D(RFX_LOG_TAG, "notifyConfStateUpdate()");
    mParticipants.clear();
    for (int i = 0; i < (int)mConfParticipants.size(); i++) {
        mParticipants.push_back(mConfParticipants[i]);
    }

    for (int i = 0; i < (int)mUnknownParticipants.size(); i++) {
        mParticipants.push_back(mUnknownParticipants[i]);
    }

    // Terminate the empty conference.
    bool autoTerminate = false;
    if (isEmptyConference() && mHaveUpdatedConferenceWithMember) {
        RFX_LOG_D(RFX_LOG_TAG, "no participants, terminate the conference");
        autoTerminate = true;
    }

    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    RtcImsConferenceController *ctrl =
            (RtcImsConferenceController *)root->findController(m_slot_id,
                    RFX_OBJ_CLASS_INFO(RtcImsConferenceController));
    if (ctrl != NULL) {
        ctrl->onParticipantsUpdate(autoTerminate);
    }
}

vector<sp<ConferenceCallUser>> RtcImsConferenceHandler::getConfParticipantsInfo() {
    return mParticipants;
}

string RtcImsConferenceHandler::getConfParticipantUri(string addr, bool isRetry) {
    bool byUserEntity = checkCarrierConfig(RFX_STATUS_KEY_OPERATE_IMS_CONFERENCE_PARTICIPANTS_BY_USER_ENTITY);
    // byUserEntity: true, try user entity from CEP first, then try restored number.
    // byUserEntity: false, try restored number first, then try user entity from CEP.
    if (byUserEntity != isRetry) {
        for (int i = 0; i < (int)mConfParticipants.size(); i++) {
            sp<ConferenceCallUser> participant = mConfParticipants[i];
            if (participant->mUserAddr == addr) {
                if (participant->mEntity.find("sip:") == 0) {
                    return participant->mEntity;
                }
            }
        }
        map<string, string>::iterator iter = mParticipantsAddrMap.find(addr);
        if (iter != mParticipantsAddrMap.end()) {
            if (iter->second.empty()) {
                return ANONYMOUS_URI;
            }
            return iter->second;
        }
    }
    if (addr.empty()) {
        return ANONYMOUS_URI;
    }
    return addr;
}

bool RtcImsConferenceHandler::isEmptyConference() {
    RFX_LOG_D(RFX_LOG_TAG, "isEmptyConference");
    int userCount = mUnknownParticipants.size();

    for (int i = 0; i < (int)mConfParticipants.size(); i++) {
        sp<ConferenceCallUser> participant = mConfParticipants[i];
        string address = participant->mUserAddr;
        string status = participant->mStatus;
        if (status != RtcImsConferenceCallMessageHandler::STATUS_DISCONNECTED) {
            if (isSelfAddress(address)) continue;
            ++userCount;
        }
    }

    RFX_LOG_D(RFX_LOG_TAG, "isEmptyConference, userCount: %d", userCount);
    if (userCount == 0) {
        return true;
    }

    if (!mHaveUpdatedConferenceWithMember) {
        RFX_LOG_D(RFX_LOG_TAG, "isEmptyConference, set mHaveUpdatedConferenceWithMember = true");
        mHaveUpdatedConferenceWithMember = true;
    }
    return false;
}

/**
 * To handle IMS conference call message
 *
 * @param len The length of data
 * @param data Conference call message
 */
void RtcImsConferenceHandler::handleImsConfCallMessage(const sp<RfxMessage>& message) {
    RFX_LOG_D(RFX_LOG_TAG, "handleVolteConfCallMessage");
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

    // ALPS04037793: Don't handle CEP if not conference host.
    if (mIsConferenceActive == false) {
        return;
    }

    if (callId == 255) {
        return;
    }
    int cepState;
    bool isPartialCEP;
    int version;
    int userCount;

    bool isFirstPkt = (index == 1);
    mCepData = concatData(isFirstPkt, mCepData, rawData);
    if (index != count) {
        //do nothing
        return;

    }
    mCepData = recoverDataFromAsciiTag(mCepData);
    if (mCepData.empty()) {
        RFX_LOG_D(RFX_LOG_TAG, "Failed to handleImsConfCallMessage due to data is empty");
        return;
    }

    RtcImsConferenceCallMessageHandler* xmlData = parseXmlPackage(mCepData);
    if (xmlData == NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "can't create xmlData object, update conf state with local cache");
        updateConferenceStateWithLocalCache();
        return;
    }


    //get user data from xml and fill them into ImsConferenceState data structure.
    vector<sp<ConferenceCallUser>> users = xmlData->getUsers();
    if (users.size() == 0 && mConfCallId == -1) {
        goto back;
    }
    //get CPE state
    cepState = xmlData->getCEPState();
    RFX_LOG_D(RFX_LOG_TAG, "cepState: %d", cepState);
    isPartialCEP = (cepState == RtcImsConferenceCallMessageHandler::CEP_STATE_PARTIAL);

    version = xmlData->getVersion();
    //Full CEP should process by the serial number.
    //Partial CEP is independent, no need to process sequentially.
    if (isPartialCEP == false) {
        if (mCepVersion >= version && mCepVersion != -1) {
            RFX_LOG_W(RFX_LOG_TAG, "version is equal or less than local version: %d, version: %d",
                    mCepVersion, version);
            goto back;
        }
    }
    mIsCepNotified = true;
    mCepVersion = version;
    //setup host
    if (mHostAddr.empty()) {
        setupHost(xmlData);
        RFX_LOG_D(RFX_LOG_TAG, "setupHost: mHostAddr: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, mHostAddr.data()));
    }
    mConfCallId = callId;

    //get optional xml element:user count
    userCount = xmlData->getUserCount();

    mLastConfParticipants.clear();
    mLastConfParticipants = mConfParticipants;
    //no optional user count element,
    //remove the participants who is not included in the xml.
    switch (cepState) {
        case RtcImsConferenceCallMessageHandler::CEP_STATE_FULL:
            fullUpdateParticipants(users);
            break;
        case RtcImsConferenceCallMessageHandler::CEP_STATE_PARTIAL:
            partialUpdateParticipants(users);
            break;
        default:
            if ((userCount == -1) || (userCount == (int)users.size())) {
                fullUpdateParticipants(users);
            } else {
                partialUpdateParticipants(users);
                isPartialCEP = true;
            }
            break;
    }
    RFX_LOG_D(RFX_LOG_TAG, "isPartialCEP: %d", isPartialCEP);

    // Update local cache before restore to prevent some unmatched participants
    // to be restored as the address which already leave the conferece.
    updateLocalCache(cepState);

    if (mRestoreImsConferenceParticipant) {
        restoreParticipantsAddressByLocalCache();
    }
    notifyConfStateUpdate();
    dumpParticipantsAddrMap();
back:
    delete xmlData;
    xmlData = NULL;
}

bool RtcImsConferenceHandler::checkCarrierConfig(const RfxStatusKeyEnum key) {
    RfxRootController* root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    if (root != NULL && root->getStatusManager(m_slot_id)->getString8Value(key) == "true") {
        return true;
    }

    return false;
}

bool RtcImsConferenceHandler::isContainParticipant(vector<string> participants, string participant) {
    for (int i = 0; i < (int)participants.size(); i++) {
        if (MtkPhoneNumberUtils::compareLoosely(participants[i], participant)) {
            RFX_LOG_D(RFX_LOG_TAG, "isContainParticipant: true");
            return true;
        }
    }
    return false;
}

void RtcImsConferenceHandler::removeParticipant(vector<string>& participants, string participant,
        bool compareLoosely) {
    for (int i = 0; i < (int)participants.size(); i++) {
        if ((compareLoosely && MtkPhoneNumberUtils::compareLoosely(participants[i], participant)) ||
                (!compareLoosely && participants[i].compare(participant) == 0)) {
            participants.erase(participants.begin() + i);
            break;
        }
    }
}

string RtcImsConferenceHandler::getPairedAddress(const string &addr) {
    string restoredAddress = getPairedRestoredAddress(addr);
    if (restoredAddress != addr) {
        RFX_LOG_D(RFX_LOG_TAG, "getPairedAddress: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, restoredAddress.data()));
        return restoredAddress;
    }
    for (int i = 0; i < (int)mLocalParticipants.size(); i++) {
        if (MtkPhoneNumberUtils::compareLoosely(mLocalParticipants[i], addr)) {
            RFX_LOG_D(RFX_LOG_TAG, "getPairedAddress: true");
            return mLocalParticipants[i];
        }
    }
    return addr;
}

string RtcImsConferenceHandler::concatData(int isFirst, string origData, string appendData) {
    if (isFirst) {
        return appendData;
    }
    return origData + appendData;
}

vector<string> RtcImsConferenceHandler::splitString(string str, string c) {
    string::size_type pos;
    vector<string> result;
    str += c;
    string::size_type len = str.size();
    for (string::size_type i = 0; i < len; i ++) {
        pos = str.find(c, i);
        if (pos < len) {
            string temp = str.substr(i, pos-i);
            result.push_back(temp);
            i = pos + c.size() -1;
        }
    }
    return result;
}

string RtcImsConferenceHandler::recoverDataFromAsciiTag(string data) {
    data = replaceAll(data, TAG_RETURN, "\r");
    data = replaceAll(data, TAG_DOUBLE_QUOTE, "\"");
    data = replaceAll(data, TAG_NEXT_LINE, "\n");
    return data;
}

string RtcImsConferenceHandler::normalizeNumberFromCLIR(string number) {
    number = replaceAll(number, "*31#", "");
    number = replaceAll(number, "#31#", "");
    return number;
}

string RtcImsConferenceHandler::replaceAll(string &str,
        const string &old_value, const string &new_value) {
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

string RtcImsConferenceHandler::getPairedRestoredAddress(string userAddr) {
    map<string, string>::reverse_iterator it;
    for (it = mParticipantsAddrMap.rbegin(); it != mParticipantsAddrMap.rend(); ++it ) {
        if (it->second == userAddr) {
            return it->first;
        }
    }
    for (it = mParticipantsAddrMap.rbegin(); it != mParticipantsAddrMap.rend(); ++it ) {
        if (MtkPhoneNumberUtils::compareLoosely(it->first, userAddr)) {
            return it->first;
        }
    }
    return userAddr;
}

void RtcImsConferenceHandler::dumpParticipantsAddrMap() {
    map<string, string>::iterator it;
    for (it = mParticipantsAddrMap.begin(); it != mParticipantsAddrMap.end(); ++it ) {
        RFX_LOG_D(RFX_LOG_TAG, "dumpParticipantsAddrMap Restored: %s, User address: %s",
                RfxRilUtils::pii(RFX_LOG_TAG, it->first.data()),
                RfxRilUtils::pii(RFX_LOG_TAG, it->second.data()));
    }
}

/**
 * encode '#' at first position only.
 * # -> %23, or else it will show private numnber.
 */
string RtcImsConferenceHandler::encodeSpecialChars(string number) {
    if (number.size() >= 1 && (number.compare(0, 1, "#") == 0)) {
        number.replace(0, 1, "%23");
    }
    return number;
}

void RtcImsConferenceHandler::addFirstMergeParticipant(string callId) {
    map<string, string>::iterator iter = mFirstMergeParticipants.find(callId);
    if (iter != mFirstMergeParticipants.end()) {
        mLocalParticipants.push_back(iter->second);
    }
}
