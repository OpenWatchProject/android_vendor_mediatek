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

package com.mediatek.internal.telephony;

import android.content.Intent;

import android.hardware.radio.V1_0.CellInfoType;
import android.hardware.radio.V1_4.CellInfoLte;
import android.hardware.radio.V1_4.CellInfo.Info;

import com.android.internal.telephony.RadioIndication;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.TelephonyIntents;

import android.os.RemoteException;
import android.os.AsyncResult;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.util.Log;
import android.telephony.SubscriptionManager;

import com.mediatek.internal.telephony.MtkSubscriptionManager;

import java.util.Iterator;

public class MtkRadioIndication extends RadioIndication {
    // TAG
    private static final String TAG = "MtkRadioInd";
    private MtkRIL mMtkRil;

    MtkRadioIndication(RIL ril) {
        super(ril);
        mMtkRil = (MtkRIL) ril;
    }

    @Override
    public void rilConnected(int indicationType) {
        mMtkRil.processIndication(indicationType);

        if (MtkRIL.MTK_RILJ_LOGD) mMtkRil.unsljLog(RILConstants.RIL_UNSOL_RIL_CONNECTED);

        // Initial conditions
        //mRadioIndication.mRil.setRadioPower(false, null);
        mMtkRil.setCdmaSubscriptionSource(
                mMtkRil.mCdmaSubscription, null);
        mMtkRil.setCellInfoListRate();
        // todo: this should not require a version number now. Setting it to latest RIL version for
        // now.
        mMtkRil.notifyRegistrantsRilConnectionChanged(15);
    }

    /**
     * Indicates when radio state changes.
     * @param indicationType RadioIndicationType
     * @param radioState android.hardware.radio.V1_0.RadioState
     */
    @Override
    public void radioStateChanged(int indicationType, int radioState) {
        int oldState = mMtkRil.getRadioState();
        super.radioStateChanged(indicationType, radioState);
        int newState = mMtkRil.getRadioState();
        if (newState != oldState) {
            Intent intent = new Intent(TelephonyIntents.ACTION_RADIO_STATE_CHANGED);
            int transferedState = getRadioStateFromInt(radioState);
            intent.putExtra("radioState", transferedState);
            intent.putExtra("subId", MtkSubscriptionManager
                        .getSubIdUsingPhoneId(mMtkRil.mInstanceId));
            mMtkRil.mMtkContext.sendBroadcastAsUser(intent, UserHandle.ALL);
            if (MtkRIL.MTK_RILJ_LOGD) {
                mMtkRil.riljLog("Broadcast for RadioStateChanged: state=" + transferedState);
            }
        }
    }

    private int getSubId(int phoneId) {
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        int[] subIds = SubscriptionManager.getSubId(phoneId);
        if (subIds != null && subIds.length > 0) {
            subId = subIds[0];
        }
        return subId;
    }

    @Override
    public void networkScanResult_1_2(int indicationType,
                                      android.hardware.radio.V1_2.NetworkScanResult result) {
        final boolean showRat = mMtkRil.showRat;
        String mccmnc;
        Iterator<android.hardware.radio.V1_2.CellInfo> it = result.networkInfos.iterator();
        // Process the operator name because RILD only consider TS.25 and NITZ
        // for (int i = 0; i < result.networkInfos.size(); i++) {
        while (it.hasNext()) {
            mccmnc = null;
            android.hardware.radio.V1_2.CellInfo record = it.next();
            switch (record.cellInfoType) {
                case CellInfoType.GSM: {
                    android.hardware.radio.V1_2.CellInfoGsm cellInfoGsm = record.gsm.get(0);
                    mccmnc = cellInfoGsm.cellIdentityGsm.base.mcc +
                            cellInfoGsm.cellIdentityGsm.base.mnc;
                    int nLac = cellInfoGsm.cellIdentityGsm.base.lac;
                    cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, true, nLac);
                    cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, false, nLac);
                    if (showRat) {
                        cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong =
                                cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong.concat(" 2G");
                        cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort =
                                cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort.concat(" 2G");
                    }
                    mMtkRil.riljLog("mccmnc=" + mccmnc + ", lac=" + nLac + ", longName=" +
                            cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong + " shortName=" +
                            cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort);
                    break;
                }

                case CellInfoType.CDMA: {
                    // android.hardware.radio.V1_2.CellInfoCdma cellInfoCdma = record.cdma.get(0);
                    break;
                }

                case CellInfoType.LTE: {
                    android.hardware.radio.V1_2.CellInfoLte cellInfoLte = record.lte.get(0);
                    mccmnc = cellInfoLte.cellIdentityLte.base.mcc +
                            cellInfoLte.cellIdentityLte.base.mnc;
                    int nLac = cellInfoLte.cellIdentityLte.base.tac;
                    cellInfoLte.cellIdentityLte.operatorNames.alphaLong =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, true, nLac);
                    cellInfoLte.cellIdentityLte.operatorNames.alphaShort =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, false, nLac);
                    if (showRat) {
                        cellInfoLte.cellIdentityLte.operatorNames.alphaLong =
                                cellInfoLte.cellIdentityLte.operatorNames.alphaLong.concat(" 4G");
                        cellInfoLte.cellIdentityLte.operatorNames.alphaShort =
                                cellInfoLte.cellIdentityLte.operatorNames.alphaShort.concat(" 4G");
                    }
                    mMtkRil.riljLog("mccmnc=" + mccmnc + ", lac=" + nLac + ", longName=" +
                            cellInfoLte.cellIdentityLte.operatorNames.alphaLong + " shortName=" +
                            cellInfoLte.cellIdentityLte.operatorNames.alphaShort);
                    break;
                }

                case CellInfoType.WCDMA: {
                    android.hardware.radio.V1_2.CellInfoWcdma cellInfoWcdma = record.wcdma.get(0);
                    mccmnc = cellInfoWcdma.cellIdentityWcdma.base.mcc +
                            cellInfoWcdma.cellIdentityWcdma.base.mnc;
                    int nLac = cellInfoWcdma.cellIdentityWcdma.base.lac;
                    cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, true, nLac);
                    cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, false, nLac);
                    if (showRat) {
                        cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong =
                            cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong.concat(" 3G");
                        cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort =
                            cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort.concat(" 3G");
                    }
                    mMtkRil.riljLog("mccmnc=" + mccmnc + ", lac=" + nLac + ", longName=" +
                            cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong +
                            " shortName=" +
                            cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort);
                    break;
                }

                default:
                    throw new RuntimeException("unexpected cellinfotype: " + record.cellInfoType);
            }
            if (mMtkRil.hidePLMN(mccmnc)) {
                it.remove();
                mMtkRil.riljLog("remove this one " + mccmnc);
            }
        }
        super.networkScanResult_1_2(indicationType, result);
    }

    @Override
    public void networkScanResult_1_4(int indicationType,
                                      android.hardware.radio.V1_4.NetworkScanResult result) {
        final boolean showRat = mMtkRil.showRat;
        String mccmnc;
        Iterator<android.hardware.radio.V1_4.CellInfo> it = result.networkInfos.iterator();
        // Process the operator name because RILD only consider TS.25 and NITZ
        // for (int i = 0; i < result.networkInfos.size(); i++) {
        while (it.hasNext()) {
            mccmnc = null;
            android.hardware.radio.V1_4.CellInfo record = it.next();
            switch (record.info.getDiscriminator()) {
                case Info.hidl_discriminator.gsm: {
                    android.hardware.radio.V1_2.CellInfoGsm cellInfoGsm = record.info.gsm();
                    mccmnc = cellInfoGsm.cellIdentityGsm.base.mcc +
                            cellInfoGsm.cellIdentityGsm.base.mnc;
                    int nLac = cellInfoGsm.cellIdentityGsm.base.lac;
                    cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, true, nLac);
                    cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, false, nLac);
                    if (showRat) {
                        cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong =
                                cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong.concat(" 2G");
                        cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort =
                                cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort.concat(" 2G");
                    }
                    mMtkRil.riljLog("mccmnc=" + mccmnc + ", lac=" + nLac + ", longName=" +
                            cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong + " shortName=" +
                            cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort);
                    break;
                }

                case Info.hidl_discriminator.cdma: {
                    // android.hardware.radio.V1_4.CellInfoCdma cellInfoCdma = record.cdma.get(0);
                    break;
                }

                case Info.hidl_discriminator.lte: {
                    android.hardware.radio.V1_4.CellInfoLte cellInfoLte = record.info.lte();
                    mccmnc = cellInfoLte.base.cellIdentityLte.base.mcc +
                            cellInfoLte.base.cellIdentityLte.base.mnc;
                    int nLac = cellInfoLte.base.cellIdentityLte.base.tac;
                    cellInfoLte.base.cellIdentityLte.operatorNames.alphaLong =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, true, nLac);
                    cellInfoLte.base.cellIdentityLte.operatorNames.alphaShort =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, false, nLac);
                    if (showRat) {
                        cellInfoLte.base.cellIdentityLte.operatorNames.alphaLong = cellInfoLte.
                                base.cellIdentityLte.operatorNames.alphaLong.concat(" 4G");
                        cellInfoLte.base.cellIdentityLte.operatorNames.alphaShort = cellInfoLte.
                                base.cellIdentityLte.operatorNames.alphaShort.concat(" 4G");
                    }
                    mMtkRil.riljLog("mccmnc=" + mccmnc + ", lac=" + nLac + ", longName=" +
                            cellInfoLte.base.cellIdentityLte.operatorNames.alphaLong +
                            " shortName=" +
                            cellInfoLte.base.cellIdentityLte.operatorNames.alphaShort);
                    break;
                }

                case Info.hidl_discriminator.wcdma: {
                    android.hardware.radio.V1_2.CellInfoWcdma cellInfoWcdma = record.info.wcdma();
                    mccmnc = cellInfoWcdma.cellIdentityWcdma.base.mcc +
                            cellInfoWcdma.cellIdentityWcdma.base.mnc;
                    int nLac = cellInfoWcdma.cellIdentityWcdma.base.lac;
                    cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, true, nLac);
                    cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort =
                            mMtkRil.lookupOperatorName(
                                    getSubId(mMtkRil.mInstanceId), mccmnc, false, nLac);
                    if (showRat) {
                        cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong =
                            cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong.concat(" 3G");
                        cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort =
                            cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort.concat(" 3G");
                    }
                    mMtkRil.riljLog("mccmnc=" + mccmnc + ", lac=" + nLac + ", longName=" +
                            cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong +
                            " shortName=" +
                            cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort);
                    break;
                }

                default:
                    throw new RuntimeException("unexpected cellinfotype: " +
                            record.info.getDiscriminator());
            }
            if (mMtkRil.hidePLMN(mccmnc)) {
                it.remove();
                mMtkRil.riljLog("remove this one " + mccmnc);
            }
        }
        super.networkScanResult_1_4(indicationType, result);
    }
}
