/*
 * Dialog.h
 *
 *  Created on: Apr 26, 2018
 *      Author: mtk54092
 */

#ifndef RFX_DIALOG_H_
#define RFX_DIALOG_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <telephony/mtk_ril.h>
#include "utils/RefBase.h"

using namespace std;
using ::android::RefBase;
using ::android::sp;

/**
 * Represents mediaAttributes element in Dialog Event Package.
 */
class RfxMediaAttribute : public virtual RefBase {
public:
    RfxMediaAttribute();
    virtual ~RfxMediaAttribute();
public:
    string mediaType;
    string mediaDirection;
    bool port0;
};

/**
 * Represents dialog element in Dialog Event Package.
 */
class RfxDialog: public virtual RefBase {
public:
    RfxDialog();
    virtual ~RfxDialog();
public:
    static const string AUDIO;
    static const string VIDEO;
    static const string SIP_RENDERING;
    static const string NO;

    static const string CONFIRMED;
    static const string EARLY;
    static const string TRYING;
    static const string PROCEEDING;
    static const string TERMINATED;

    static const string INACTIVE;
    static const string SENDRECV;
    static const string SENDONLY;
    static const string RECVONLY;

    typedef enum {
        CALL_STATE_CONFIRMED = 1,
        CALL_STATE_TERMINATED = 2,
        CALL_STATE_TRYING = 3,
        CALL_STATE_PROCEEDING = 4,
        CALL_STATE_EARLY = 5,
    } DialogCallState;

    //call type is corresponding to value in ImsCallProfile
    typedef enum {
        CALL_TYPE_VOICE = 2,
        CALL_TYPE_VT = 4,
        CALL_TYPE_VT_TX = 5,
        CALL_TYPE_VT_RX = 6,
        CALL_TYPE_VT_NODIR = 7,
    } DialogCallType;

public:
    int dialogId;
    bool exclusive;
    string state;
    bool initiator;

    //local start
    string identity;
    string targetUri;
    vector<sp<RfxMediaAttribute>> mediaAttributes;

    //Param start
    string pname;
    string pval;
    //Param end
    //local end

    //remote start
    string remoteIdentity;
    string remoteTargetUri;

    //Param start
    string remotePname;
    string remotePval;
    //Param end
    //remote end
public:
    int getDialogId();
    string getAddress();
    string getRemoteAddress();
    bool isMt();
    bool isCallHeld();
    bool isPullable();
    int getCallState();
    int getCallType();
private:
    bool isCallHeld(vector<sp<RfxMediaAttribute>> mediaAttributes);
    bool isVideoCallInBackground();
};

#endif /* DIALOG_H_ */
