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
#ifndef __RTC_SMS_ADDRESS_H__
#define __RTC_SMS_ADDRESS_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "sms/RmcCdmaSmsConverter.h"

/*****************************************************************************
 * RtcSmsAddress
 *****************************************************************************/
class RtcSmsAddress {
// Constructor / Destrutor
public:
    // Constructor
    RtcSmsAddress();

// External Method
public:
    // Get the address of the SMS.
    //
    // RETURNS: the SMS address
    const String8 & getAddressString() const {
        return mAddress;
    }

    // Check if error happens when parsing SMS address
    //
    // RETURNS: true if error happens
    bool isError() {
        return mError;
    }

// Constants
public:
    static const int TON_UNKNOWN        = 0;
    static const int TON_INTERNATIONAL  = 1;
    static const int TON_NATIONAL       = 2;
    static const int TON_NETWORK        = 3;
    static const int TON_SUBSCRIBER     = 4;
    static const int TON_ALPHANUMERIC   = 5;
    static const int TON_ABBREVIATED    = 6;

// Method for sub class
protected:
    // Set parsing error.
    //
    // RETURNS: None
    void setError(
        bool error = true  // [IN] true for error, false for no error
    ) {
        mError = error;
    }

    // Set the type of number(TON).
    //
    // RETRURNS: None
    void setTon(int ton) {
        mTon = ton;
    }

    // Get the type of number(TON).
    //
    // RETURNS: the type of number
    const int & getTon() const {
        return mTon;
    }

    // Get the address for address modification.
    //
    // RETURNS: the reference of the address
    String8 & getAddr() {
        return mAddress;
    }

    // Check the number type if alphanumeric.
    //
    // RETURNS: true if alphanumeric number
    bool isAlphanumeric() {
        return mTon == TON_ALPHANUMERIC;
    }

// Implement
private:
    int mTon;
    String8 mAddress;
    bool mError;
};

/*****************************************************************************
 * RtcGsmSmsAddress
 *****************************************************************************/
class RtcGsmSmsAddress: public RtcSmsAddress {
// Constructor / Destructor
public:
    // Constructor
    RtcGsmSmsAddress(unsigned char* data, int offset, int length);

// Implement
private:
    bool bcd2Char(unsigned char bcd, String8 &addr);
    bool calledPartyBCDFragmentToString(unsigned char *bcd, int offset, int length, String8 &addr);

    static const int MAX_BUFFER_SIZE       = 40;
    static const int OFFSET_ADDRESS_LENGTH  = 0;
    static const int OFFSET_TOA             = 1;
    static const int OFFSET_ADDRESS_VALUE   = 2;
    unsigned char mOrigBytes[MAX_BUFFER_SIZE];
};

/*****************************************************************************
 * RtcCdmaSmsAddress
 *****************************************************************************/
class RtcCdmaSmsAddress: public RtcSmsAddress {
// Constructor / Destructor
public:
    RtcCdmaSmsAddress(RIL_CDMA_SMS_Address &addr);
};
#endif /* __RTC_SMS_ADDRESS_H__ */