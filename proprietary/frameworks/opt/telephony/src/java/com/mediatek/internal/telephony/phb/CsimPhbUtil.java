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

package com.mediatek.internal.telephony.phb;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.Rlog;

import com.android.internal.telephony.uicc.AdnRecord;
import com.android.internal.telephony.uicc.CsimFileHandler;
import com.android.internal.telephony.uicc.IccFileHandler;

import java.util.ArrayList;

/**
 * This class is used to get Csim phonebook storage information
 * and other utility functions.
 */
public class CsimPhbUtil extends Handler {
    private static final String LOG_TAG = "CsimPhbUtil";
    // { used, total, max number len, max name len}
    private static int[] sAdnRecordSize = {-1, -1, -1, -1};
    private static final int MAX_SIM_CNT = 4;
    private static final int MAX_NUMBER_LENGTH = 20;
    private static final int MAX_NAME_LENGTH = 14;
    private static final String[]  PROPERTY_RIL_FULL_UICC_TYPE = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };

    /**
     * check the current phb storage informtion and send it to target.
     *
     * @param response message to be posted when done check
     */
    public static void getPhbRecordInfo(Message response) {
        // it may return -1 values when query SIM process not finished.
        // so please show loading PHB title on APP when adnRecordSize[0] return -1.
        sAdnRecordSize[2] = MAX_NUMBER_LENGTH;
        sAdnRecordSize[3] = MAX_NAME_LENGTH;

        Rlog.d(LOG_TAG, "[getPhbRecordInfo] sAdnRecordSize[] {" + sAdnRecordSize[0]
                + ", " + sAdnRecordSize[1] + ", " + sAdnRecordSize[2]
                + ", " + sAdnRecordSize[3] + "}");

        if (null != response) {
            AsyncResult.forMessage(response).result = sAdnRecordSize;
            response.sendToTarget();
        }
    }

    /**
     * clear the phb storage informtion.
     */
    public static void clearAdnRecordSize() {
        Rlog.d(LOG_TAG, "[clearAdnRecordSize]");
        if (null != sAdnRecordSize) {
            for (int i = 0; i < sAdnRecordSize.length; i++) {
                sAdnRecordSize[i] = -1;
            }
        }
    }

    /**
     * Update the phb storage informtion according to the operation.
     *
     * @param update mark the operation,if insert update is 1, if delete update is -1
     * @return true if update successful and false if fail
     */
    public static boolean updatePhbStorageInfo(int update) {
        int used = sAdnRecordSize[0];
        int total = sAdnRecordSize[1];

        Rlog.d(LOG_TAG, "[updatePhbStorageInfo] used: " + used + ", total: "
                + total + ", update: " + update);
        if (used > -1) {
            int newUsed = used + update;
            setPhbRecordStorageInfo(total, newUsed);
            return true;
        } else {
            Rlog.d(LOG_TAG, "[updatePhbStorageInfo] Storage info is not ready!");
            return false;
        }
    }

    /**
     * check if read all the adn entries and set the right storage information.
     *
     * @param adnList the whole adn load from sim
     */
    public static void initPhbStorage(ArrayList<MtkAdnRecord> adnList) {
        int totalSize = -1;
        int usedRecord = -1;
        if (adnList != null) {
            totalSize = adnList.size();
            usedRecord = 0;

            for (int i = 0; i < totalSize; i++) {
                if (!adnList.get(i).isEmpty()) {
                    usedRecord++;
                }
            }

            Rlog.d(LOG_TAG, "[initPhbStorage] Current total: " + sAdnRecordSize[1]
                    + ", used:" + sAdnRecordSize[0]
                    + ", update total: " + totalSize + ", used: " + usedRecord);

            // check the storage, if it not -1. means there already has value.
            // we need add two values for count total size.
            if (sAdnRecordSize[1] > -1) {
                // Add second ADN file info
                int newUsed = usedRecord + sAdnRecordSize[0];
                int newTotal = sAdnRecordSize[1] + totalSize;
                setPhbRecordStorageInfo(newTotal, newUsed);
            } else {
                // Add first ADN file info
                setPhbRecordStorageInfo(totalSize, usedRecord);
            }
        }
    }

    /**
     * set the right phb storage informtion after the check.
     *
     * @param totalSize the size phb support to store
     * @param usedRecord the used Record size in phb
     */
    private static void setPhbRecordStorageInfo(int totalSize, int usedRecord) {
        sAdnRecordSize[0] = usedRecord;
        sAdnRecordSize[1] = totalSize;
        Rlog.d(LOG_TAG, "[setPhbRecordStorageInfo] usedRecord: " + usedRecord
                + ", totalSize: " + totalSize);
    }

    /**
     * Check if has CSIM PHB enhance for fast read/write (CPBR/CPBW) capability in MD
     * MD capability:
     *     GSM: Support SIM/USIM PHB enhance for all MD version
     *     C2K:
     *         1. Before 93: Only support RUIM PHB enhance for fast read/write
     *         2. After 93: Support RUIM/CSIM enhance for fast read/write
     * @param fileHandler file handler to distinguish RUIM and CSIM
     * @return true for support
     */
    public static boolean hasModemPhbEnhanceCapability(IccFileHandler fileHandler) {
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            return true;
        } else {
            // before 93MD, only support RUIM PHB enhance (CPBR/CPBW)
            if (fileHandler != null && fileHandler instanceof CsimFileHandler) {
                // For CSIM/USIM dual mode card, we can still access through MD1 PHB
                // Without considering the case of SIM1: CSIM/USIM + SIM2: CSIM only
                for (int i = 0; i < MAX_SIM_CNT; i++) {
                    String cardType = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[i]);
                    if ((cardType.indexOf("CSIM") >= 0) && (cardType.indexOf("USIM") >= 0)) {
                        return true;
                    }
                }
                return false;
            } else {
                return true;
            }
        }
    }

    /**
     * Check if need to use GSM PHB ready event from MD1 for Ruim/Csim
     * Before 93MD, C2K and GSM using different PHB ready event from GSM/C2K MD
     * After 93MD, C2K and GSM using same PHB ready event from GSM
     * @return true for yes
     */
    public static boolean isUsingGsmPhbReady(IccFileHandler fileHandler) {
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            return true;
        } else {
            if (fileHandler != null && fileHandler instanceof CsimFileHandler) {
                // For CSIM/USIM dual mode card, we can still access through MD1 PHB
                // Without considering the case of SIM1: CSIM/USIM + SIM2: CSIM only
                for (int i = 0; i < MAX_SIM_CNT; i++) {
                    String cardType = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[i]);
                    if ((cardType.indexOf("CSIM") >= 0) && (cardType.indexOf("USIM") >= 0)) {
                        return true;
                    }
                }
                return false;
            }
            return false;
        }
    }
}
