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
/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package mediatek.telephony;

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.SystemProperties;
import android.os.PersistableBundle;

import android.app.ActivityThread;

import android.content.Context;

import android.text.TextUtils;
import android.telephony.Rlog;
import android.telephony.SignalStrength;
import android.telephony.ServiceState;

import com.mediatek.internal.telephony.MtkOpTelephonyCustomizationUtils;
import com.mediatek.internal.telephony.MtkOpTelephonyCustomizationFactoryBase;

import java.util.Arrays;

/**
 * Contains phone signal strength related information.
 */
public class MtkSignalStrength extends SignalStrength {

    private static final String LOG_TAG = "MtkSignalStrength";
    private static final boolean DBG = true;

    private MtkOpTelephonyCustomizationFactoryBase mTelephonyCustomizationFactory = null;
    private static ISignalStrengthExt mSignalStrengthExt = null;

    protected int mPhoneId;

    public static final String PROPERTY_OPERATOR_OPTR = "persist.vendor.operator.optr";
    private static String mOpId = null;
    private int mMtkLevel = -1;
    private boolean mMtkRsrpOnly = false;
    private int[] mMtkLteRsrpThreshold = null;
    private int[] mMtkLteRssnrThreshold = null;
    /**
     * Empty constructor
     */
    public MtkSignalStrength(int phoneId) {
        super();
        mPhoneId = phoneId;
    }

    /**
     * Copy constructors
     *
     * @param phoneId provide phoneId to get phone
     * @param s Source MtkSignalStrength
     */
    public MtkSignalStrength(int phoneId, SignalStrength s) {
        super(s);
        mPhoneId = phoneId;
        if (s instanceof MtkSignalStrength) {
            MtkSignalStrength mtkSignal = (MtkSignalStrength) s;
            this.mMtkLevel = mtkSignal.getMtkLevel();
            this.setMtkRsrpOnly(mtkSignal.isMtkRsrpOnly());
            this.setMtkLteRsrpThreshold(mtkSignal.getMtkLteRsrpThreshold());
            this.setMtkLteRssnrThreshold(mtkSignal.getMtkLteRssnrThreshold());
        }
    }

    /**
     * Construct a SignalStrength object from the given parcel.
     *
     * @hide
     */
    public MtkSignalStrength(Parcel in) {
        super(in);
        mPhoneId = in.readInt();
        mMtkLevel = in.readInt();
        mMtkRsrpOnly = in.readBoolean();

        int size;
        size = in.readInt();
        if (size > 0) {
            mMtkLteRsrpThreshold = new int[size];
            in.readIntArray(mMtkLteRsrpThreshold);
        } else {
            mMtkLteRsrpThreshold = null;
        }
        size = in.readInt();
        if (size > 0) {
            mMtkLteRssnrThreshold = new int[size];
            in.readIntArray(mMtkLteRssnrThreshold);
        } else {
            mMtkLteRssnrThreshold = null;
        }
    }

    /**
     * {@link Parcelable#writeToParcel}
     */
    @Override
    public void writeToParcel(Parcel out, int flags) {
        super.writeToParcel(out, flags);
        out.writeInt(mPhoneId);
        out.writeInt(mMtkLevel);
        out.writeBoolean(mMtkRsrpOnly);
        int size = mMtkLteRsrpThreshold == null? 0: mMtkLteRsrpThreshold.length;
        out.writeInt(size);
        if (size > 0) out.writeIntArray(mMtkLteRsrpThreshold);
        size  = mMtkLteRssnrThreshold == null? 0: mMtkLteRssnrThreshold.length;
        out.writeInt(size);
        if (size > 0) out.writeIntArray(mMtkLteRssnrThreshold);
    }

    /**
     * log
     */
    private static void log(String s) {
        Rlog.w(LOG_TAG, s);
    }

    /**
     * @return string representation.
     */
    @Override
    public String toString() {
        return new StringBuilder().append(super.toString())
            .append(", phoneId=").append(mPhoneId)
            .append(", mMtkLevel=").append(mMtkLevel)
            .append(", mMtkRsrpOnly=").append(mMtkRsrpOnly)
            .append(", mMtkLteRsrpThreshold=").append(Arrays.toString(mMtkLteRsrpThreshold))
            .append(", mMtkLteRssnrThreshold=").append(Arrays.toString(mMtkLteRssnrThreshold))
            .toString();
    }

    private boolean useMtkLevel() {
        // We have customization for LTE
        if (mLte.isValid() && // check signal type first
                (mMtkLteRsrpThreshold != null ||
                mMtkLteRssnrThreshold != null)) return true;
        return false;
    }

    @Override
    public int getLevel() {
        if (useMtkLevel()) return mMtkLevel;
        return super.getLevel();
    }

    /**
     * Get LTE as level 0..4.
     * @return int the level for lte singal strength
     * @hide
     */
    @Override
    public int getLteLevel() {
        if (useMtkLevel()) return mMtkLevel;
        return mLte.getLevel();
    }

    public boolean isMtkRsrpOnly() {
        return mMtkRsrpOnly;
    }

    public void setMtkRsrpOnly(boolean a) {
        mMtkRsrpOnly = a;
    }

    public int[] getMtkLteRsrpThreshold() {
        return mMtkLteRsrpThreshold;
    }

    public void setMtkLteRsrpThreshold(int[] threshold) {
        if (threshold == null) mMtkLteRsrpThreshold = null;
        else {
            mMtkLteRsrpThreshold = new int[threshold.length];
            for (int i = 0; i < threshold.length; i++)
                mMtkLteRsrpThreshold[i] = threshold[i];
        }
    }

    public int[] getMtkLteRssnrThreshold() {
        return mMtkLteRssnrThreshold;
    }
    public void setMtkLteRssnrThreshold(int [] threshold) {
        if (threshold == null) mMtkLteRssnrThreshold = null;
        else {
            mMtkLteRssnrThreshold = new int[threshold.length];
            for (int i = 0; i < threshold.length; i++)
                mMtkLteRssnrThreshold[i] = threshold[i];
        }
    }

    public int getMtkLevel() {
        return mMtkLevel;
    }

    public void updateMtkLevel(PersistableBundle cc, ServiceState ss) {
        updateMtkLteLevel(cc, ss);
    }

    public int getNrLevel() {
        return mNr.getLevel();
    }

    private void updateMtkLteLevel(PersistableBundle cc, ServiceState ss) {
        int[] rsrpThresholds;
        int[] rssnrThresholds;
        boolean rsrpOnly;

        if (mMtkLteRsrpThreshold == null ||
                (mMtkRsrpOnly == false &&
                mMtkLteRssnrThreshold == null)) return;

        rsrpThresholds = mMtkLteRsrpThreshold;
        rssnrThresholds = mMtkLteRssnrThreshold;
        rsrpOnly = mMtkRsrpOnly;

        int rsrpBoost = 0;
        if (ss != null) {
            rsrpBoost = ss.getLteEarfcnRsrpBoost();
        }

        int rssiIconLevel = SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        int rsrpIconLevel = SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        int snrIconLevel = -1;

        int rsrp = mLte.getRsrp() + rsrpBoost;

        if (rsrp < -140 || rsrp > -44) {
            rsrpIconLevel = -1;
        } else {
            rsrpIconLevel = rsrpThresholds.length;
            while (rsrpIconLevel > 0 && rsrp < rsrpThresholds[rsrpIconLevel - 1]) rsrpIconLevel--;
        }

        if (rsrpOnly) {
            if (DBG) log("updateLevel() - rsrp = " + rsrpIconLevel);
            if (rsrpIconLevel != -1) {
                mMtkLevel= rsrpIconLevel;
                return;
            }
        }

        /*
         * Values are -200 dB to +300 (SNR*10dB) RS_SNR >= 13.0 dB =>4 bars 4.5
         * dB <= RS_SNR < 13.0 dB => 3 bars 1.0 dB <= RS_SNR < 4.5 dB => 2 bars
         * -3.0 dB <= RS_SNR < 1.0 dB 1 bar RS_SNR < -3.0 dB/No Service Antenna
         * Icon Only
         */
        int rssnr = mLte.getRssnr();
        if (rssnr > 300) {
            snrIconLevel = -1;
        } else {
            snrIconLevel = mMtkLteRssnrThreshold.length;
            while (snrIconLevel > 0 && rssnr < mMtkLteRssnrThreshold[snrIconLevel - 1])
                snrIconLevel--;
        }

        if (DBG) log("updateLevel() - rsrp:" + rsrp + " snr:" + rssnr + " rsrpIconLevel:"
                + rsrpIconLevel + " snrIconLevel:" + snrIconLevel
                + " lteRsrpBoost:" + rsrpBoost);

        /* Choose a measurement type to use for notification */
        if (snrIconLevel != -1 && rsrpIconLevel != -1) {
            /*
             * The number of bars displayed shall be the smaller of the bars
             * associated with LTE RSRP and the bars associated with the LTE
             * RS_SNR
             */
            mMtkLevel = (rsrpIconLevel < snrIconLevel ? rsrpIconLevel : snrIconLevel);
            return;
        }

        if (snrIconLevel != -1) {
            mMtkLevel = snrIconLevel;
            return;
        }

        if (rsrpIconLevel != -1) {
            mMtkLevel = rsrpIconLevel;
            return;
        }

        int rssi = mLte.getRssi();
        if (rssi > -51) rssiIconLevel = SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        else if (rssi >= -89) rssiIconLevel = SIGNAL_STRENGTH_GREAT;
        else if (rssi >= -97) rssiIconLevel = SIGNAL_STRENGTH_GOOD;
        else if (rssi >= -103) rssiIconLevel = SIGNAL_STRENGTH_MODERATE;
        else if (rssi >= -113) rssiIconLevel = SIGNAL_STRENGTH_POOR;
        else rssiIconLevel = SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        if (DBG) log("getLteLevel - rssi:" + rssi + " rssiIconLevel:"
                + rssiIconLevel);
        mMtkLevel = rssiIconLevel;
    }
}
