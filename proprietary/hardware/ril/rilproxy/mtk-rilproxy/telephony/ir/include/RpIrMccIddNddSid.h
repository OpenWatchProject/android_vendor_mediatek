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
 * File name:  RpIrMccIddNddSid.h
 * Author: Gang Xu (MTK80181)
 * Description:
 * This class represents mcc, idd, ndd, sid
 */

#ifndef __RP_IR_MCCIDDNDDSID_H__
#define __RP_IR_MCCIDDNDDSID_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "utils/String8.h"

using ::android::String8;

class RpIrMccIddNddSid {
public:
    int mMcc;
    String8 mCc;
    int mSidMin;
    int mSidMax;
    String8 mIdd;
    String8 mNdd;

    /**
    * Default constructor.
    */
    RpIrMccIddNddSid() {
        mMcc = -1;
        mSidMin = -1;
        mSidMax = -1;
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
    RpIrMccIddNddSid(int mcc, const char *cc, int sidmin, int sidmax, const char *idd,
            const char * ndd) {
        mMcc = mcc;
        mCc = String8(cc);
        mSidMin = sidmin;
        mSidMax = sidmax;
        mIdd = String8(idd);
        mNdd = String8(ndd);
    }

    /**
    * Copy constructor.
    * @param t the RpIrMccIddNddSid object.
    */
    RpIrMccIddNddSid(const RpIrMccIddNddSid &t) {
        copyFrom(t);
    }

    int getMcc() {
        return mMcc;
    }

    String8 getCc() {
        return mCc;
    }

    int getSidMin() {
        return mSidMin;
    }

    int getSidMax() {
        return mSidMax;
    }

    String8 getIdd() {
        return mIdd;
    }

    String8 getNdd() {
        return mNdd;
    }

    String8 toString() {
        return String8::format("Mcc=%d, Cc=%s, SidMin=%d, SidMax=%d, Idd=%s, Ndd=%s",
                mMcc, mCc.string(), mSidMin, mSidMax, mIdd.string(), mNdd.string());
    }

   /**
    * @hide
    */
protected:
    void copyFrom(const RpIrMccIddNddSid &t) {
        mMcc = t.mMcc;
        mCc = t.mCc;
        mSidMin = t.mSidMin;
        mSidMax = t.mSidMax;
        mIdd = t.mIdd;
        mNdd = t.mNdd;
    }
};
#endif /* __RP_IR_MCCIDDNDDSID_H__ */
