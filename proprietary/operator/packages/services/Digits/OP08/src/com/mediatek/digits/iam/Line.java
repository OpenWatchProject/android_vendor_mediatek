/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.digits.iam;

public class Line {
    private static final String TAG = "Line";
    private String mMsisdn;
    private String mTmBAN;
    private String mSOC;
    private String mGivenName;
    private String mTmIMSI;
    private String mTmSubscriberType;
    private String mLineId;
    private String mTmAccountStatus;
    private String mAccountStatus;
    private String mTmEmail;
    private String mCustomerType;
    private String mTmAuthZipFailCount;
    private String mTmbillingbirthdate;
    private String mIMSI;
    private String mCustomerId;

    public Line(String msisdn, String tmBAN, String soc, String givenName, String tmIMSI,
                String tmSubscriberType, String lineId, String tmAccountStatus, String accountStatus,
                String tmEmail, String customerType, String tmAuthZipFailCount,
                String tmbillingbirthdate, String imsi, String customerId) {
        mMsisdn = msisdn;
        mTmBAN = tmBAN;
        mSOC = soc;
        mGivenName = givenName;
        mTmIMSI = tmIMSI;
        mTmSubscriberType = tmSubscriberType;
        mLineId = lineId;
        mTmAccountStatus = tmAccountStatus;
        mAccountStatus = accountStatus;
        mTmEmail = tmEmail;
        mCustomerType = customerType;
        mTmAuthZipFailCount = tmAuthZipFailCount;
        mTmbillingbirthdate = tmbillingbirthdate;
        mIMSI = imsi;
        mCustomerId = customerId;
    }

    public void setMsisdn(String msisdn) {
        mMsisdn = msisdn;
    }

    public String getMsisdn() {
        return mMsisdn;
    }
}