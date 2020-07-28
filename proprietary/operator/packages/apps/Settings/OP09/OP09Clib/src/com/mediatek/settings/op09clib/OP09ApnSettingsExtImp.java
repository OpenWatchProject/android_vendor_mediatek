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

package com.mediatek.settings.op09clib;

import android.content.Context;
import android.os.SystemProperties;

import android.telephony.SubscriptionManager;
import android.util.Log;
import androidx.preference.PreferenceScreen;

import com.android.ims.ImsManager;
import com.android.settings.network.ApnPreference;

import com.mediatek.internal.telephony.MtkIccCardConstants.CardType;
import com.mediatek.settings.ext.DefaultApnSettingsExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.Arrays;

/**
 * APN CT feature..
 */
public class OP09ApnSettingsExtImp extends DefaultApnSettingsExt {
    private static final String TAG = "OP09ClibApnSettingsExtImp";
    private Context mContext;
    private static final int SOURCE_TYPE_DEFAULT = 0;
    private static final String IMS_TYPE = "IMS";
    private static final String CTNET_TYPE = "ctnet";
    private static final String CTWAP_TYPE = "ctwap";

    /**
    * Constructor method.
    * @param context plugin context
    */
    public OP09ApnSettingsExtImp(Context context) {
        super();
        mContext = context;
    }

    /**
     * For China telecom feature.
     * remove ctwap apn,
     * remove ims apn if volte support.
     * @param subId subid for judge is China telecom sim card or not
     * @param apnType for judge apn type
     * @param apnList apnlist common settings use ArrayList<ApnPreference>
     */
    @Override
    public void customizeApnState(int subId, String apnType, Object apnList) {

        ArrayList<ApnPreference> newApnList = (ArrayList<ApnPreference>)apnList;
        ImsManager imsMgr =
            ImsManager.getInstance(mContext, SubscriptionManager.getPhoneId(subId));
        boolean isCTSimCard = isCtSim(subId);
        Log.i(TAG, "customizeApnState subId = " + subId
                + " apnType = " + apnType
                + " isCTSimCard = " + isCTSimCard);
        if (!isCTSimCard) {
            Log.i(TAG, "customizeApnState is not CT card");
            return;
        }
        if (CTWAP_TYPE.equals(apnType)) {
            Log.i(TAG, "customizeApnState remove ctwap");
            newApnList.remove(newApnList.size() - 1);
        }
        boolean isVolteOn = imsMgr.isEnhanced4gLteModeSettingEnabledByUser()
                && imsMgr.isNonTtyOrTtyOnVolteEnabled();
        boolean isVolteSupport = imsMgr.isVolteEnabledByPlatform();
        Log.i(TAG, "customizeApnState subId = " + subId
                + " isVolteOn = " + isVolteOn
                + " isVolteSupport = " + isVolteSupport);
        if (IMS_TYPE.equals(apnType)) {
            /// volte can not support || volte button is off, IMS apn need show
            if (!isVolteSupport || !isVolteOn) {
                Log.i(TAG, "customizeApnState remove ims");
                newApnList.remove(newApnList.size() - 1);
            }
        }
    }

    /**
     * Check whether the SIM is CT or not.
     */
    public static boolean isCtSim(int subId) {
        boolean result = false;
        int slotId = SubscriptionManager.getPhoneId(subId);
        CardType cdmaCardType = MtkTelephonyManagerEx.getDefault().getCdmaCardType(slotId);
        /// if can not get cardType, set apn items enable false first,
        /// after sim state changed, will update again.
        if (null == cdmaCardType) {
            Log.i(TAG, "cdmaCardType = null return true");
            return true;
        }
        switch (cdmaCardType) {
            case CT_3G_UIM_CARD:
            case CT_UIM_SIM_CARD:
            case CT_4G_UICC_CARD:
            case CT_EXCEL_GG_CARD:
                result = true;
                break;

            default:
                break;
        }
        Log.i(TAG, "isCtSim, subId=" + subId
        		+ ", slotId = " + slotId
                + ", cdmaCardType=" + cdmaCardType
                + ", ctSim=" + result);
        return result;
    }

    /**
     * Update the customized status(enable , disable).
     * Called at update screen status
     * @param subId sub id
     * @param sourceType 0 means google default apn.
     * @param root PPP's parent
     * @param apnType apnType="IMS", apn="ctnet" apn="ctwap".
     */
    @Override
    public void updateFieldsStatus(int subId, int sourceType,
            PreferenceScreen root, String apnType) {
        boolean isCTSimCard = isCtSim(subId);
        Log.d(TAG, "updateFieldsStatus subId=" + subId
                + " sourceType = " + sourceType
                + " apnType = " + apnType
                + " isCTSimCard = " + isCTSimCard);

        if (sourceType != SOURCE_TYPE_DEFAULT) {
            Log.i(TAG, "updateFieldsStatus return because is not default apn");
            return;
        }
        if (!isCTSimCard) {
            Log.i(TAG, "updateFieldsStatus return because is not CT card");
            return;
        }
        if (IMS_TYPE.equals(apnType) || CTNET_TYPE.equals(apnType)) {
            Log.i(TAG, "updateFieldsStatus set all item enbale false");
            int count = root.getPreferenceCount();
            for (int i = 0; i < count; i++) {
                root.getPreference(i).setEnabled(false);
            }
        }
    }
}