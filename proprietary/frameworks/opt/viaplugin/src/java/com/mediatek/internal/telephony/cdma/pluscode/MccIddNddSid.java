/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cdma.pluscode;

/**
 * This class represents mcc, cc, min sid, max sid, idd, ndd states.
 * @hide
 */
public class MccIddNddSid {
    public int mMcc;
    public String mCc;
    public int mSidMin;
    public int mSidMax;
    public String mIdd;
    public String mNdd;

    /**
     * Default constructor.
     */
    public MccIddNddSid() {
        mMcc = -1;
        mCc = null;
        mSidMin = -1;
        mSidMax = -1;
        mIdd = null;
        mNdd = null;
    }

    /**
     * Constructor.
     * @param mcc MCC
     * @param cc CC
     * @param sidmin min SID
     * @param sidmax max SID
     * @param idd IDD
     * @param ndd NDD
     */
    public MccIddNddSid(int mcc, String cc, int sidmin, int sidmax, String idd,
            String ndd) {
        mMcc = mcc;
        mCc = cc;
        mSidMin = sidmin;
        mSidMax = sidmax;
        mIdd = idd;
        mNdd = ndd;
    }

    /**
     * Copy constructor.
     * @param t the MccIddNddSid object.
     */
    public MccIddNddSid(MccIddNddSid t) {
        copyFrom(t);
    }

    /**
     * @hide
     */
    protected void copyFrom(MccIddNddSid t) {
        mMcc = t.mMcc;
        mCc = t.mCc;
        mSidMin = t.mSidMin;
        mSidMax = t.mSidMax;
        mIdd = t.mIdd;
        mNdd = t.mNdd;
    }

    public int getMcc() {
        return mMcc;
    }

    public String getCc() {
        return mCc;
    }

    public int getSidMin() {
        return mSidMin;
    }

    public int getSidMax() {
        return mSidMax;
    }

    public String getIdd() {
        return mIdd;
    }

    public String getNdd() {
        return mNdd;
    }

    @Override
    public String toString() {
        return ("Mcc =" + mMcc + ", Cc = " + mCc + ", SidMin = " + mSidMin
                + ", SidMax = " + mSidMax + ", Idd = " + mIdd + ", Ndd = " + mNdd);
    }
}
