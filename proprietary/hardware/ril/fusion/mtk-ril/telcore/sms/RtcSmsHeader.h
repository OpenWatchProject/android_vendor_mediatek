/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#ifndef __RTC_SMS_HEADER_H__
#define __RTC_SMS_HEADER_H__

/*****************************************************************************
 * Class ConcatRef
 *****************************************************************************/
class ConcatRef {
public:
    ConcatRef()
        : refNumber(-1), seqNumber(-1), msgCount(-1), is3Gpp2(false), isEightBits(false) {
    }

public:
    int refNumber;
    int seqNumber;
    int msgCount;
    bool is3Gpp2;
    bool isEightBits;
};

/*****************************************************************************
 * Class RtcSmsUserDataHeader
 *****************************************************************************/
class RtcSmsUserDataHeader {
// Constructor/Destructor
public:
    // Construct user data header with UDH array.
    RtcSmsUserDataHeader(
        unsigned char *header,    // [IN] point to first elemement of header array
        int len,                  // [IN] the length of header array
        bool is3Gpp2              // [IN] the format of the SMS, true is 3GPP2, false is 3GPP
    );

    // Destructor
    virtual ~RtcSmsUserDataHeader() {}

// External Method
public:
    // Check if the SMS is concatenated SMS.
    //
    // RETURNS: true if the SMS is concatenated SMS
    bool isConcatSms() const {
        return mConcatSms;
    }

    // Get the reference number of the concatenated SMS.
    //
    // RETURNS: the reference number
    int getRefNumber() const {
        return mConcatRef.refNumber;
    }

    // Get the message count of the concatenated SMS.
    //
    // RETURNS: the message count
    int getMsgCount() const {
        return mConcatRef.msgCount;
    }

    // Get the sequency number of the concatenated SMS.
    int getSeqNumber() const {
        return mConcatRef.seqNumber;
    }

    // Check the SMS format is 3GPP2 or not.
    //
    // RETURNS: true if 3GPP2 format SMS
    bool is3Gpp2() {
        return mConcatRef.is3Gpp2;
    }

    // Error happens or not when parsing the SMS header.
    //
    // RETURNS: true if error happen
    bool isError() {
        return mError;
    }

private:
    // Set Error when parsing SMS header error.
    //
    // RETURNS: NONE
    void setError(
        bool error = true   // [IN] true for error
    ) {
        mError = error;
    }

// Implement
private:
    ConcatRef mConcatRef;
    bool mConcatSms;
    bool mError;
};

#endif /* __RTC_SMS_HEADER_H__ */