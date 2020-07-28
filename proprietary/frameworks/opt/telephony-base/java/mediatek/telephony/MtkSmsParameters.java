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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package mediatek.telephony;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * @hide
 */
public class MtkSmsParameters implements Parcelable {
    /*
     * The format of validity period
     */
    public int format;

    /*
     * Validity period
     */
    public int vp;

    /*
     * Protocol ID
     */
    public int pid;

    /*
     * Decode char set
     */
    public int dcs;

    public MtkSmsParameters(int format, int vp, int pid, int dcs) {
        this.format = format;
        this.vp = vp;
        this.pid = pid;
        this.dcs = dcs;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(format);
        dest.writeInt(vp);
        dest.writeInt(pid);
        dest.writeInt(dcs);
    }

    public static final Parcelable.Creator<MtkSmsParameters> CREATOR =
            new Parcelable.Creator<MtkSmsParameters>() {
        public MtkSmsParameters createFromParcel(Parcel source) {
            int format = source.readInt();
            int vp = source.readInt();
            int pid = source.readInt();
            int dcs = source.readInt();

            return new MtkSmsParameters(format, vp, pid, dcs);
        }

        public MtkSmsParameters[] newArray(int size) {
            return new MtkSmsParameters[size];
        }
    };

    public String toString() {
        StringBuilder sb = new StringBuilder(50);
        sb.append("[");
        sb.append(this.format);
        sb.append(", ");
        sb.append(this.vp);
        sb.append(", ");
        sb.append(this.pid);
        sb.append(", ");
        sb.append(this.dcs);
        sb.append("]");

        return sb.toString();
    }
}
