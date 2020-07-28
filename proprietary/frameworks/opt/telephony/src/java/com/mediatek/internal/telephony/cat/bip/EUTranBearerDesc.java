/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import android.os.Parcel;
import android.os.Parcelable;

public class EUTranBearerDesc extends BearerDesc {
    public int QCI = 0;
    public int maxBitRateU = 0;
    public int maxBitRateD = 0;
    public int guarBitRateU = 0;
    public int guarBitRateD = 0;
    public int maxBitRateUEx = 0;
    public int maxBitRateDEx = 0;
    public int guarBitRateUEx = 0;
    public int guarBitRateDEx = 0;
    public int pdnType = 0;

    public EUTranBearerDesc() {
        bearerType = BipUtils.BEARER_TYPE_EUTRAN;
    }

    private EUTranBearerDesc(Parcel in) {
        bearerType = in.readInt();
        QCI = in.readInt();
        maxBitRateU = in.readInt();
        maxBitRateD = in.readInt();
        guarBitRateU = in.readInt();
        guarBitRateD = in.readInt();
        maxBitRateUEx = in.readInt();
        maxBitRateDEx = in.readInt();
        guarBitRateUEx = in.readInt();
        guarBitRateDEx = in.readInt();
        pdnType = in.readInt();
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(bearerType);
        dest.writeInt(QCI);
        dest.writeInt(maxBitRateU);
        dest.writeInt(maxBitRateD);
        dest.writeInt(guarBitRateU);
        dest.writeInt(guarBitRateD);
        dest.writeInt(maxBitRateUEx);
        dest.writeInt(maxBitRateDEx);
        dest.writeInt(guarBitRateUEx);
        dest.writeInt(guarBitRateDEx);
        dest.writeInt(pdnType);
    }

    public static final Parcelable.Creator<EUTranBearerDesc> CREATOR
            = new Parcelable.Creator<EUTranBearerDesc>() {
        public EUTranBearerDesc createFromParcel(Parcel in) {
            return new EUTranBearerDesc(in);
        }

        public EUTranBearerDesc[] newArray(int size) {
            return new EUTranBearerDesc[size];
        }
    };
}

