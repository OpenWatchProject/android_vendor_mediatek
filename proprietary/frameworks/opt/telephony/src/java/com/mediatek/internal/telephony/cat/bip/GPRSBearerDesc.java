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

public class GPRSBearerDesc extends BearerDesc {
    public int precedence = 0;
    public int delay = 0;
    public int reliability = 0;
    public int peak = 0;
    public int mean = 0;
    public int pdpType = 0;

    public int dataCompression = 0;
    public int headerCompression = 0;

    public int dataRate = 0;
    public int bearerService = 0;
    public int connectionElement = 0;

    public GPRSBearerDesc() {
        bearerType = BipUtils.BEARER_TYPE_GPRS;
    }

    private GPRSBearerDesc(Parcel in) {
        bearerType = in.readInt();
        precedence = in.readInt();
        delay = in.readInt();
        reliability = in.readInt();
        peak = in.readInt();
        mean = in.readInt();
        pdpType = in.readInt();
        dataCompression = in.readInt();
        headerCompression = in.readInt();
        dataRate = in.readInt();
        bearerService = in.readInt();
        connectionElement = in.readInt();
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(bearerType);
        dest.writeInt(precedence);
        dest.writeInt(delay);
        dest.writeInt(reliability);
        dest.writeInt(peak);
        dest.writeInt(mean);
        dest.writeInt(pdpType);
        dest.writeInt(dataCompression);
        dest.writeInt(headerCompression);
        dest.writeInt(dataRate);
        dest.writeInt(bearerService);
        dest.writeInt(connectionElement);
    }

    public static final Parcelable.Creator<GPRSBearerDesc> CREATOR
            = new Parcelable.Creator<GPRSBearerDesc>() {
        public GPRSBearerDesc createFromParcel(Parcel in) {
            return new GPRSBearerDesc(in);
        }

        public GPRSBearerDesc[] newArray(int size) {
            return new GPRSBearerDesc[size];
        }
    };
}
