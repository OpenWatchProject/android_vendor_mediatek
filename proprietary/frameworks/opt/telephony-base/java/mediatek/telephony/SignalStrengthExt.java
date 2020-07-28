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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import android.content.Context;

import android.os.RemoteException;
import android.os.ServiceManager;

import android.telephony.SignalStrength;
import android.telephony.Rlog;
import android.telephony.RadioAccessFamily;

import mediatek.telephony.ISignalStrengthExt;

import com.android.internal.telephony.ITelephony;

public class SignalStrengthExt implements ISignalStrengthExt {
    private static final boolean DBG = true;
    static final String TAG = "SignalStrengthExt";

    public SignalStrengthExt() {
    }

    public int mapUmtsSignalLevel(int phoneId, int UmtsRscp) {
        // [ALPS03252546] -- START , for 3G network
        int phoneRaf;
        boolean isTdd3G = false;
        int level = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;

        log("mapUmtsSignalLevel, phoneId=" + phoneId);

        // TDD/FDD mode
        if ((UmtsRscp > -25) || (UmtsRscp == SignalStrength.INVALID))
            level = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        else if (UmtsRscp >= -72)
            level = SignalStrength.SIGNAL_STRENGTH_GREAT;
        else if (UmtsRscp >= -88)
            level = SignalStrength.SIGNAL_STRENGTH_GOOD;
        else if (UmtsRscp >= -104)
            level = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        else if (UmtsRscp >= -120)
            level = SignalStrength.SIGNAL_STRENGTH_POOR;
        else
            level = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        log("mapUmtsSignalLevel, level=" + level);
        return level;
        // [ALPS03252546] -- END
    }

    //[ALPS01440836][ALPS01594704]-START: change level mapping rule of signal for CMCC
    public int mapLteSignalLevel(int mLteRsrp, int mLteRssnr, int mLteSignalStrength) {
        /*
         * TS 36.214 Physical Layer Section 5.1.3 TS 36.331 RRC RSSI = received
         * signal + noise RSRP = reference signal dBm RSRQ = quality of signal
         * dB= Number of Resource blocksxRSRP/RSSI SNR = gain=signal/noise ratio
         * = -10log P1/P2 dB
         */
        int rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        int rsrpIconLevel = -1;
        int snrIconLevel = -1;

        if (mLteRsrp > -44) {
            rsrpIconLevel = -1;
        } else if (mLteRsrp >= -85) {
            rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_GREAT;
        } else if (mLteRsrp >= -95) {
            rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_GOOD;
        } else if (mLteRsrp >= -105) {
            rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        } else if (mLteRsrp >= -115) {
            rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_POOR;
        } else if (mLteRsrp >= -140) {
            rsrpIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        }

        /*
         * Values are -200 dB to +300 (SNR*10dB) RS_SNR >= 13.0 dB =>4 bars 4.5
         * dB <= RS_SNR < 13.0 dB => 3 bars 1.0 dB <= RS_SNR < 4.5 dB => 2 bars
         * -3.0 dB <= RS_SNR < 1.0 dB 1 bar RS_SNR < -3.0 dB/No Service Antenna
         * Icon Only
         */
        if (mLteRssnr > 300) {
            snrIconLevel = -1;
        } else if (mLteRssnr >= 130) {
            snrIconLevel = SignalStrength.SIGNAL_STRENGTH_GREAT;
        } else if (mLteRssnr >= 45) {
            snrIconLevel = SignalStrength.SIGNAL_STRENGTH_GOOD;
        } else if (mLteRssnr >= 10) {
            snrIconLevel = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        } else if (mLteRssnr >= -30) {
            snrIconLevel = SignalStrength.SIGNAL_STRENGTH_POOR;
        } else if (mLteRssnr >= -200) {
            snrIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        }
        Rlog.i(TAG, "getLTELevel - rsrp:" + mLteRsrp + " snr:" + mLteRssnr + " rsrpIconLevel:"
                + rsrpIconLevel + " snrIconLevel:" + snrIconLevel);

        /* Choose a measurement type to use for notification */
        if (snrIconLevel != -1 && rsrpIconLevel != -1) {
            /*
             * The number of bars displayed shall be the smaller of the bars
             * associated with LTE RSRP and the bars associated with the LTE
             * RS_SNR
             */
            return (rsrpIconLevel < snrIconLevel ? rsrpIconLevel : snrIconLevel);
        }

        if (snrIconLevel != -1) {
            return snrIconLevel;
        }

        if (rsrpIconLevel != -1) {
            return rsrpIconLevel;
        }

        /* Valid values are (0-63, 99) as defined in TS 36.331 */
        if (mLteSignalStrength > 63) {
            rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_NONE_OR_UNKNOWN;
        } else if (mLteSignalStrength >= 12) {
            rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_GREAT;
        } else if (mLteSignalStrength >= 8) {
            rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_GOOD;
        } else if (mLteSignalStrength >= 5) {
            rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_MODERATE;
        } else if (mLteSignalStrength >= 0) {
            rssiIconLevel = SignalStrength.SIGNAL_STRENGTH_POOR;
        }
        Rlog.i(TAG, "getLTELevel - rssi:" + mLteSignalStrength + " rssiIconLevel:"
                + rssiIconLevel);
        return rssiIconLevel;
    }
    //[ALPS01440836][ALPS01594704]-END: change level mapping rule of signal for CMCC

    public static void log(String text) {
        Rlog.d(TAG, text);
    }

    private void loge(String txt) {
        if (DBG) {
            Rlog.e(TAG, txt);
        }
    }
}
