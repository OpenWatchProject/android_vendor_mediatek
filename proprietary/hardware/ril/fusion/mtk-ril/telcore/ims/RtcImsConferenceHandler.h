/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * File name:  RfxTimer.h
 * Author: Jun Liu (MTK80064)
 * Description:
 * Prototype of timer for Ril proxy framework
 */

#ifndef __RFX_IMS_CONFERENCE_HANDLER_H__
#define __RFX_IMS_CONFERENCE_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <map>
#include <string.h>
#include <string>
#include <vector>


#include "RtcImsConferenceCallMessageHandler.h"

#include "RfxSignal.h"
#include "RfxTimer.h"
#include "utils/RefBase.h"
#include "utils/StrongPointer.h"
#include "utils/Looper.h"


using ::android::RefBase;
using ::android::sp;
using ::android::Looper;
using ::android::MessageHandler;

using namespace std;
/*****************************************************************************
 * Class RtcImsConferenceHandler
 *****************************************************************************/

class RtcImsConferenceHandler {
public:
    RtcImsConferenceHandler(int slot);
    ~RtcImsConferenceHandler();
    void handleImsConfCallMessage(const sp<RfxMessage>& message);
    void startConference();
    void closeConference();
    void firstMerge(string callId_1, string callId_2, string num_1, string num_2);
    void tryAddParticipant(string addr);
    void tryOneKeyAddParticipant(string addr);
    void tryRemoveParticipant(string addr);
    bool modifyParticipantComplete();
    void modifyParticipantFailed();
    void addFirstMergeParticipant(string callId);
    void tryOneKeyAddLocalList(vector<string> users);
    void updateConferenceStateWithLocalCache();
    vector<sp<ConferenceCallUser>> getConfParticipantsInfo();
    string getConfParticipantUri(string addr, bool isRetry);
    static string getUserNameFromSipTelUriString(string uriString);
    static vector<string> splitString(string str, string c);
    static string normalizeNumberFromCLIR(string number);
    static string concatData(int isFirst, string origData, string appendData);
    static string recoverDataFromAsciiTag(string data);
    static string replaceAll(string &str, const string &old_value, const string &new_value);
    static string encodeSpecialChars(string number);

private:
    RtcImsConferenceCallMessageHandler* parseXmlPackage(string data);
    void restoreParticipantsAddressByLocalCache();
    void restoreUnknowParticipants(vector<string> restoreUnknowCandidates);
    void setupHost(RtcImsConferenceCallMessageHandler* xmlData);
    void fullUpdateParticipants(vector<sp<ConferenceCallUser>> user);
    void partialUpdateParticipants(vector<sp<ConferenceCallUser>> user);
    void updateLocalCache(int cepState);
    void notifyConfStateUpdate();
    bool isSelfAddress(string address);
    bool updateParticipants(sp<ConferenceCallUser> oldParticipant, string to);
    void showCacheAndXmlData(string callerName);
    bool isContainParticipant(vector<string> participants, string participant);
    void removeParticipant(vector<string>& participants, string participant, bool compareLoosely);
    string getPairedAddress(const string &addr);
    bool checkCarrierConfig(const RfxStatusKeyEnum key);
    string getPairedRestoredAddress(string userAddr);
    void dumpParticipantsAddrMap();
    bool isEmptyConference();
    void insertParticipantsAddrMap(string key, string value);
private:
    static const string TAG_NEXT_LINE;
    static const string TAG_RETURN;
    static const string TAG_DOUBLE_QUOTE;

    int m_slot_id;
    bool mIsCepNotified;
    int mConfCallId;
    int mCepVersion;
    string mCepData;
    string mHostAddr;
    string mAddingParticipant;
    string mRemovingParticipant;
    bool mRestoreImsConferenceParticipant = false;
    bool mIsConferenceActive = false;
    bool mHaveUpdatedConferenceWithMember = false;
    int mConnectedCount = 0;

    vector<sp<ConferenceCallUser>> mParticipants;
    vector<sp<ConferenceCallUser>> mConfParticipants;
    vector<sp<ConferenceCallUser>> mUnknownParticipants;
    vector<string> mLocalParticipants;
    map<string, string> mParticipantsAddrMap;
    map<string, string> mFirstMergeParticipants;
    vector<string> mOneKeyAddingParticipants;
    vector<sp<ConferenceCallUser>> mLastConfParticipants;
};

#endif /* __RFX_IMS_CONFERENCE_HANDLER_H__ */
