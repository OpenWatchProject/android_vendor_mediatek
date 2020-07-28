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

package com.mediatek.internal.telephony;

import android.os.Message;
import android.os.Parcel;

/**
 * SuppSrvRequest: to encapsulate a Supplementary Service Request.
 *
 *  @hide
 */
public class MtkSuppSrvRequest {
    int mRequestCode;
    Message mResultCallback;
    public Parcel mParcel;

    // Request Code
    public static final int SUPP_SRV_REQ_SET_CLIP               = 1;
    public static final int SUPP_SRV_REQ_GET_CLIP               = 2;
    public static final int SUPP_SRV_REQ_SET_CLIR               = 3;
    public static final int SUPP_SRV_REQ_GET_CLIR               = 4;
    public static final int SUPP_SRV_REQ_SET_COLP               = 5;
    public static final int SUPP_SRV_REQ_GET_COLP               = 6;
    public static final int SUPP_SRV_REQ_SET_COLR               = 7;
    public static final int SUPP_SRV_REQ_GET_COLR               = 8;
    public static final int SUPP_SRV_REQ_SET_CB                 = 9;
    public static final int SUPP_SRV_REQ_GET_CB                 = 10;
    public static final int SUPP_SRV_REQ_SET_CF                 = 11;
    public static final int SUPP_SRV_REQ_GET_CF                 = 12;
    public static final int SUPP_SRV_REQ_SET_CW                 = 13;
    public static final int SUPP_SRV_REQ_GET_CW                 = 14;
    public static final int SUPP_SRV_REQ_MMI_CODE               = 15;
    public static final int SUPP_SRV_REQ_GET_CF_IN_TIME_SLOT    = 16;
    public static final int SUPP_SRV_REQ_SET_CF_IN_TIME_SLOT    = 17;

    public static final int SUPP_SRV_REQ_CFU_QUERY              = 18;

    /**
     * Create SuppSrvRequest object with request and callback.
     * @param request SS request code
     * @param resultCallback callback Message
     * @return the created SuppSrvRequest object
     */
    public static MtkSuppSrvRequest obtain(int request, Message resultCallback) {
        MtkSuppSrvRequest ss = new MtkSuppSrvRequest();

        ss.mRequestCode = request;
        ss.mResultCallback = resultCallback;
        ss.mParcel = Parcel.obtain();

        return ss;
    }

    private MtkSuppSrvRequest() {
    }

    public Message getResultCallback() {
        return mResultCallback;
    }

    public void setResultCallback(Message resultCallback) {
        mResultCallback = resultCallback;
    }

    public int getRequestCode() {
        return mRequestCode;
    }
}

