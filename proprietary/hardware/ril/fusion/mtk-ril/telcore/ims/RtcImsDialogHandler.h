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

#ifndef __RFX_IMS_DIALOG_HANDLER_H__
#define __RFX_IMS_DIALOG_HANDLER_H__

#include <string>
#include <vector>
#include "RfxSignal.h"
#include "RfxDialog.h"

using ::android::sp;

/**
 * RtcImsDialogHandler, the class is used to handle DEP(Dialog event package) data,
 * the data is xml raw data that received from EIMSEVTPKG, and type is 2.
 */
class RtcImsDialogHandler {
public:
    RtcImsDialogHandler(int slot);
    ~RtcImsDialogHandler();

    //Handle IMS Dialog event package raw data.
    void handleImsDialogMessage(const sp<RfxMessage>& message);
    string concatData(int isFirst, string origData, string appendData);
    string recoverDataFromAsciiTag(string data);
    string replaceAll(string &str, const string &old_value, const string &new_value);
    static void replace(char * str, int n, const char * newStr);
    int getPhoneId();

private:
    static const string TAG_NEXT_LINE;
    static const string TAG_RETURN;
    static const string TAG_DOUBLE_QUOTE;
    int mSlot;
    string mDepData;
};

/**
 * DepMessageHandler, which is responsible for parsing Dialog event package.
 */
class DepMessageHandler: public DefaultHandler {
public:
    static const string DIALOG_INFO;
    static const string DIALOG;
    static const string VERSION;
    static const string ID;
    static const string SA_EXCLUSIVE;
    static const string STATE;

    static const string LOCAL;
    static const string REMOTE;
    static const string IDENTITY;
    static const string TARGET;
    static const string URI;

    static const string DIRECTION;
    static const string INITIATOR;
    static const string RECEIVER;

    static const string PARAM;
    static const string PNAME;
    static const string PVAL;

    static const string MEDIA_ATTRIBUTES;
    static const string MEDIA_TYPE;
    static const string MEDIA_DIRECTION;
    static const string PORT0;

    static const string TRUE;
public:
    DepMessageHandler();
    virtual ~DepMessageHandler();
    void startElement(string nodeName, string nodeValue, string attributeName,
            string attributeValue);
    void endElement(string nodeName);

private:

    vector<sp<RfxDialog>> mDialogInfo;
    sp<RfxDialog> mDialog;
    sp<RfxMediaAttribute> mMediaAttr;

    bool mDialogStart;
    bool mLocalStart;
    bool mMediaAttrStart;
    bool mRemoteStart;

    int mVersion;
    int mDepState;

public:
    vector<sp<RfxDialog>> getDialogInfo();
    int getVersion() const;

    enum {
        DEP_STATE_UNKNOWN,
        DEP_STATE_FULL,
        DEP_STATE_PARTIAL
    };
};
#endif /* __RFX_IMS_DIALOG_HANDLER_H__ */
