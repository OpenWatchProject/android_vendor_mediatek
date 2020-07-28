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

public class UTranBearerDesc extends BearerDesc {
    public int trafficClass = 0;
    public int maxBitRateUL_High = 0;
    public int maxBitRateUL_Low = 0;
    public int maxBitRateDL_High = 0;
    public int maxBitRateDL_Low = 0;
    public int guarBitRateUL_High = 0;
    public int guarBitRateUL_Low = 0;
    public int guarBitRateDL_High = 0;
    public int guarBitRateDL_Low = 0;
    public int deliveryOrder = 0;
    public int maxSduSize = 0;
    public int sduErrorRatio = 0;
    public int residualBitErrorRadio = 0;
    public int deliveryOfErroneousSdus = 0;
    public int transferDelay = 0;
    public int trafficHandlingPriority = 0;
    public int pdpType = 0;

    public UTranBearerDesc() {
        bearerType = BipUtils.BEARER_TYPE_UTRAN;
    }

    private UTranBearerDesc(Parcel in) {
        bearerType = in.readInt();
        trafficClass = in.readInt();
        maxBitRateUL_High = in.readInt();
        maxBitRateUL_Low = in.readInt();
        maxBitRateDL_High = in.readInt();
        maxBitRateDL_Low = in.readInt();
        guarBitRateUL_High = in.readInt();
        guarBitRateUL_Low = in.readInt();
        guarBitRateDL_High = in.readInt();
        guarBitRateDL_Low = in.readInt();
        deliveryOrder = in.readInt();
        maxSduSize = in.readInt();
        sduErrorRatio = in.readInt();
        residualBitErrorRadio = in.readInt();
        deliveryOfErroneousSdus = in.readInt();
        transferDelay = in.readInt();
        trafficHandlingPriority = in.readInt();
        pdpType = in.readInt();
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(bearerType);
        dest.writeInt(trafficClass);
        dest.writeInt(maxBitRateUL_High);
        dest.writeInt(maxBitRateUL_Low);
        dest.writeInt(maxBitRateDL_High);
        dest.writeInt(maxBitRateDL_Low);
        dest.writeInt(guarBitRateUL_High);
        dest.writeInt(guarBitRateUL_Low);
        dest.writeInt(guarBitRateDL_High);
        dest.writeInt(guarBitRateDL_Low);
        dest.writeInt(deliveryOrder);
        dest.writeInt(maxSduSize);
        dest.writeInt(sduErrorRatio);
        dest.writeInt(residualBitErrorRadio);
        dest.writeInt(deliveryOfErroneousSdus);
        dest.writeInt(transferDelay);
        dest.writeInt(trafficHandlingPriority);
        dest.writeInt(pdpType);
    }

    public static final Parcelable.Creator<UTranBearerDesc> CREATOR
            = new Parcelable.Creator<UTranBearerDesc>() {
        public UTranBearerDesc createFromParcel(Parcel in) {
            return new UTranBearerDesc(in);
        }

        public UTranBearerDesc[] newArray(int size) {
            return new UTranBearerDesc[size];
        }
    };
}

