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
package android.telephony;

import android.os.Parcel;
import android.os.Parcelable;

import com.android.internal.telephony.RILConstants;
import com.mediatek.internal.telephony.MtkRILConstants;


/**
 * Object to indicate the phone radio type and access technology.
 *
 * @hide
 */
public class MtkRadioAccessFamily {

    // Radio Access Family
    // 2G
    public static final int RAF_UNKNOWN = (int) TelephonyManager.NETWORK_TYPE_BITMASK_UNKNOWN;
    public static final int RAF_GSM = (int) TelephonyManager.NETWORK_TYPE_BITMASK_GSM;
    public static final int RAF_GPRS = (int) TelephonyManager.NETWORK_TYPE_BITMASK_GPRS;
    public static final int RAF_EDGE = (int) TelephonyManager.NETWORK_TYPE_BITMASK_EDGE;
    public static final int RAF_IS95A = (int) TelephonyManager.NETWORK_TYPE_BITMASK_CDMA;
    public static final int RAF_IS95B = (int) TelephonyManager.NETWORK_TYPE_BITMASK_CDMA;
    public static final int RAF_1xRTT = (int) TelephonyManager.NETWORK_TYPE_BITMASK_1xRTT;
    // 3G
    public static final int RAF_EVDO_0 = (int) TelephonyManager.NETWORK_TYPE_BITMASK_EVDO_0;
    public static final int RAF_EVDO_A = (int) TelephonyManager.NETWORK_TYPE_BITMASK_EVDO_A;
    public static final int RAF_EVDO_B = (int) TelephonyManager.NETWORK_TYPE_BITMASK_EVDO_B;
    public static final int RAF_EHRPD = (int) TelephonyManager.NETWORK_TYPE_BITMASK_EHRPD;
    public static final int RAF_HSUPA = (int) TelephonyManager.NETWORK_TYPE_BITMASK_HSUPA;
    public static final int RAF_HSDPA = (int) TelephonyManager.NETWORK_TYPE_BITMASK_HSDPA;
    public static final int RAF_HSPA = (int) TelephonyManager.NETWORK_TYPE_BITMASK_HSPA;
    public static final int RAF_HSPAP = (int) TelephonyManager.NETWORK_TYPE_BITMASK_HSPAP;
    public static final int RAF_UMTS = (int) TelephonyManager.NETWORK_TYPE_BITMASK_UMTS;
    public static final int RAF_TD_SCDMA = (int) TelephonyManager.NETWORK_TYPE_BITMASK_TD_SCDMA;
    // 4G
    public static final int RAF_LTE = (int) TelephonyManager.NETWORK_TYPE_BITMASK_LTE;
    public static final int RAF_LTE_CA = (int) TelephonyManager.NETWORK_TYPE_BITMASK_LTE_CA;

    // 5G
    public static final int RAF_NR = (int) TelephonyManager.NETWORK_TYPE_BITMASK_NR;

    // Grouping of RAFs
    // 2G
    private static final int GSM = RAF_GSM | RAF_GPRS | RAF_EDGE;
    private static final int CDMA = RAF_IS95A | RAF_IS95B | RAF_1xRTT;
    // 3G
    private static final int EVDO = RAF_EVDO_0 | RAF_EVDO_A | RAF_EVDO_B | RAF_EHRPD;
    private static final int HS = RAF_HSUPA | RAF_HSDPA | RAF_HSPA | RAF_HSPAP;
    private static final int WCDMA = HS | RAF_UMTS | RAF_TD_SCDMA;
    // 4G
    private static final int LTE = RAF_LTE | RAF_LTE_CA;

    // 5G
    private static final int NR = RAF_NR;

    public static int getRafFromNetworkType(int type) {
        switch (type) {
            case RILConstants.NETWORK_MODE_WCDMA_PREF:
                return GSM | WCDMA;
            case RILConstants.NETWORK_MODE_GSM_ONLY:
                return GSM;
            case RILConstants.NETWORK_MODE_WCDMA_ONLY:
                return WCDMA;
            case RILConstants.NETWORK_MODE_GSM_UMTS:
                return GSM | WCDMA;
            case RILConstants.NETWORK_MODE_CDMA:
                return CDMA | EVDO;
            case RILConstants.NETWORK_MODE_LTE_CDMA_EVDO:
                return LTE | CDMA | EVDO;
            case RILConstants.NETWORK_MODE_LTE_GSM_WCDMA:
                return LTE | GSM | WCDMA;
            case RILConstants.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA:
                return LTE | CDMA | EVDO | GSM | WCDMA;
            case RILConstants.NETWORK_MODE_LTE_ONLY:
                return LTE;
            case RILConstants.NETWORK_MODE_LTE_WCDMA:
                return LTE | WCDMA;
            case RILConstants.NETWORK_MODE_CDMA_NO_EVDO:
                return CDMA;
            case RILConstants.NETWORK_MODE_EVDO_NO_CDMA:
                return EVDO;
            case RILConstants.NETWORK_MODE_GLOBAL:
                return GSM | WCDMA | CDMA | EVDO;
            case RILConstants.NETWORK_MODE_TDSCDMA_ONLY:
                return RAF_TD_SCDMA;
            case RILConstants.NETWORK_MODE_TDSCDMA_WCDMA:
                return RAF_TD_SCDMA | WCDMA;
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA:
                return LTE | RAF_TD_SCDMA;
            case RILConstants.NETWORK_MODE_TDSCDMA_GSM:
                return RAF_TD_SCDMA | GSM;
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA_GSM:
                return LTE | RAF_TD_SCDMA | GSM;
            case RILConstants.NETWORK_MODE_TDSCDMA_GSM_WCDMA:
                return RAF_TD_SCDMA | GSM | WCDMA;
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA_WCDMA:
                return LTE | RAF_TD_SCDMA | WCDMA;
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA_GSM_WCDMA:
                return LTE | RAF_TD_SCDMA | GSM | WCDMA;
            case RILConstants.NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                return RAF_TD_SCDMA | CDMA | EVDO | GSM | WCDMA;
            case RILConstants.NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
                return LTE | RAF_TD_SCDMA | CDMA | EVDO | GSM | WCDMA;
            case (RILConstants.NETWORK_MODE_NR_ONLY):
                return NR;
            case (RILConstants.NETWORK_MODE_NR_LTE):
                return NR | LTE;
            case (RILConstants.NETWORK_MODE_NR_LTE_CDMA_EVDO):
                return NR | LTE | CDMA | EVDO;
            case (RILConstants.NETWORK_MODE_NR_LTE_GSM_WCDMA):
                return NR | LTE | GSM | WCDMA;
            case (RILConstants.NETWORK_MODE_NR_LTE_CDMA_EVDO_GSM_WCDMA):
                return NR | LTE | CDMA | EVDO | GSM | WCDMA;
            case (RILConstants.NETWORK_MODE_NR_LTE_WCDMA):
                return NR | LTE | WCDMA;
            case (RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA):
                return NR | LTE | RAF_TD_SCDMA;
            case (RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA_GSM):
                return NR | LTE | RAF_TD_SCDMA | GSM;
            case (RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA_WCDMA):
                return NR | LTE | RAF_TD_SCDMA | WCDMA;
            case (RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA_GSM_WCDMA):
                return NR | LTE | RAF_TD_SCDMA | GSM | WCDMA;
            case (RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA):
                return NR | LTE | RAF_TD_SCDMA | CDMA | EVDO | GSM | WCDMA;
            case MtkRILConstants.NETWORK_MODE_LTE_GSM:
                return LTE | GSM;
            case MtkRILConstants.NETWORK_MODE_LTE_TDD_ONLY:
                return LTE;
            case MtkRILConstants.NETWORK_MODE_CDMA_GSM:
                return CDMA | GSM;
            case MtkRILConstants.NETWORK_MODE_CDMA_EVDO_GSM:
                return CDMA | EVDO | GSM;
            case MtkRILConstants.NETWORK_MODE_LTE_CDMA_EVDO_GSM:
                return LTE | CDMA | EVDO | GSM;
            default:
                return RAF_UNKNOWN;
        }
    }

    /**
     * if the raf includes ANY bit set for a group
     * adjust it to contain ALL the bits for that group
     */
    private static int getAdjustedRaf(int raf) {
        raf = ((GSM & raf) > 0) ? (GSM | raf) : raf;
        raf = ((WCDMA & raf) > 0) ? (WCDMA | raf) : raf;
        raf = ((CDMA & raf) > 0) ? (CDMA | raf) : raf;
        raf = ((EVDO & raf) > 0) ? (EVDO | raf) : raf;
        raf = ((LTE & raf) > 0) ? (LTE | raf) : raf;
        raf = ((NR & raf) > 0) ? (NR | raf) : raf;

        return raf;
    }

    public static int getNetworkTypeFromRaf(int raf) {
        raf = getAdjustedRaf(raf);

        switch (raf) {
            case (GSM | WCDMA):
                return RILConstants.NETWORK_MODE_WCDMA_PREF;
            case GSM:
                return RILConstants.NETWORK_MODE_GSM_ONLY;
            case WCDMA:
                return RILConstants.NETWORK_MODE_WCDMA_ONLY;
            case (CDMA | EVDO):
                return RILConstants.NETWORK_MODE_CDMA;
            case (LTE | CDMA | EVDO):
                return RILConstants.NETWORK_MODE_LTE_CDMA_EVDO;
            case (LTE | GSM | WCDMA):
                return RILConstants.NETWORK_MODE_LTE_GSM_WCDMA;
            case (LTE | CDMA | EVDO | GSM | WCDMA):
                return RILConstants.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA;
            case LTE:
                return RILConstants.NETWORK_MODE_LTE_ONLY;
            case (LTE | WCDMA):
                return RILConstants.NETWORK_MODE_LTE_WCDMA;
            case CDMA:
                return RILConstants.NETWORK_MODE_CDMA_NO_EVDO;
            case EVDO:
                return RILConstants.NETWORK_MODE_EVDO_NO_CDMA;
            case (GSM | WCDMA | CDMA | EVDO):
                return RILConstants.NETWORK_MODE_GLOBAL;
            case RAF_TD_SCDMA:
                return RILConstants.NETWORK_MODE_TDSCDMA_ONLY;
            case (LTE | RAF_TD_SCDMA):
                return RILConstants.NETWORK_MODE_LTE_TDSCDMA;
            case (RAF_TD_SCDMA | GSM):
                return RILConstants.NETWORK_MODE_TDSCDMA_GSM;
            case (LTE | RAF_TD_SCDMA | GSM):
                return RILConstants.NETWORK_MODE_LTE_TDSCDMA_GSM;
            case (NR):
                return RILConstants.NETWORK_MODE_NR_ONLY;
            case (NR | LTE):
                return RILConstants.NETWORK_MODE_NR_LTE;
            case (NR | LTE | CDMA | EVDO):
                return RILConstants.NETWORK_MODE_NR_LTE_CDMA_EVDO;
            case (NR | LTE | GSM | WCDMA):
                return RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA_GSM_WCDMA;
            case (NR | LTE | CDMA | EVDO | GSM | WCDMA):
                return RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
            case (NR | LTE | WCDMA):
                return RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA_WCDMA;
            case (NR | LTE | RAF_TD_SCDMA):
                return RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA;
            case (NR | LTE | RAF_TD_SCDMA | GSM):
                return RILConstants.NETWORK_MODE_NR_LTE_TDSCDMA_GSM;
            case (LTE | GSM):
                return MtkRILConstants.NETWORK_MODE_LTE_GSM;
            case (CDMA | GSM):
                return MtkRILConstants.NETWORK_MODE_CDMA_GSM;
            case (CDMA | EVDO | GSM):
                return MtkRILConstants.NETWORK_MODE_CDMA_EVDO_GSM;
            case (LTE | CDMA | EVDO | GSM):
                return MtkRILConstants.NETWORK_MODE_LTE_CDMA_EVDO_GSM;
            default:
                return RILConstants.PREFERRED_NETWORK_MODE;
        }
    }
}
