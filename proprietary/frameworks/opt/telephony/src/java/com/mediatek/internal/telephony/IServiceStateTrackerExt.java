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
import android.telephony.ServiceState;

import mediatek.telephony.MtkServiceState;

public interface IServiceStateTrackerExt {
    String onUpdateSpnDisplay(String plmn, MtkServiceState ss, int phoneId);

    /**
     * @check if IMEI locked.
     */
    boolean isImeiLocked();

    boolean isBroadcastEmmrrsPsResume(int value);
    boolean needEMMRRS();
    boolean needSpnRuleShowPlmnOnly();
    boolean needBrodcastAcmt(int errorType, int errorCause);
    boolean needRejectCauseNotification(int cause);
    boolean needIgnoredState(int state, int new_state, int cause);
    boolean operatorDefinedInternationalRoaming(String operatorNumeric);
    boolean needIgnoreFemtocellUpdate(int state, int cause);
    boolean needToShowCsgId();
    public boolean needBlankDisplay(int cause);
    int needAutoSwitchRatMode(int phoneId, String nwPlmn);

    /**
     * Return if allow display SPN.
     */
    boolean allowSpnDisplayed();

    /**
     * Return if support rat balancing
     */
    boolean isSupportRatBalancing();

    /**
     * Return if roaming for special SIM.
     *
     * @param strServingPlmn The operator numberic get from service state.
     * @param strHomePlmn The mcc+mnc get from SIM IMSI.
     * @return if roaming for the special SIM
     */
    boolean isRoamingForSpecialSIM(String strServingPlmn, String strHomePlmn);

    /**
     * Return if need disable IVSR.
     * @return if need disable IVSR
     */
    boolean isNeedDisableIVSR();

    /**
     * Modify the operator text to be shown
     *
     * @param plmn The current plmn text
     * @param phoneId The phoneId of the SIM
     * @param lac The lac of cell
     * @param simRecords The sim record
     */
    String onUpdateSpnDisplayForIms(
                String plmn, MtkServiceState ss, int lac, int phoneId, Object simRecords);

    /**
     * For TMO, show ECC when PS is in service but IMS's ECC is support.
     * @hide
     */
    boolean showEccForIms();

    /**
     * For LTE customization for operators.
     *
     * @return true if we consider rsrp only
     * @hide
     */
    boolean getMtkRsrpOnly();

    /**
     * For LTE customization for operators.
     *
     * @return integer array as threshold
     * @hide
     */
    int[] getMtkLteRsrpThreshold();

    /**
     * For LTE customization for operators.
     *
     * @return integer array as threshold
     * @hide
     */
    int[] getMtkLteRssnrThreshold();
}
