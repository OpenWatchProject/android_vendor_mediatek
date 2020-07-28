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

import android.os.SystemProperties;
import android.telephony.Rlog;

import com.mediatek.internal.telephony.MtkPhoneConstants;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Operator Utility to get operator information.
 */
public class MtkOperatorUtils {

    public enum OPID {
        OP01,   // CMCC
        OP02,   // CU
        OP03,   // Orange
        OP05,   // TMO EU (DTAG, TeleKom)
        OP06,   // VDF
        OP07,   // AT&T
        OP08,   // TMO US
        OP09,   // CT
        OP11,   // H3G
        OP15,   // Telefonica
        OP16,   // EE
        OP18,   // RJIL
        OP19,   // Telstra
        OP50,   // SoftBank
        OP100,  // CSL
        OP101,  // PCCW
        OP102,  // SmarTone
        OP103,  // SingTel
        OP106,  // H3G HK
        OP107,  // SFR
        OP108,  // TWM
        OP109,  // CHT
        OP110,  // FET
        OP117,  // Smartfren
        OP118,  // YTL
        OP124,  // APTG
        OP125,  // DTAC
        OP128,  // DNA
        OP131,  // TrueMove
        OP132,  // Vivo
        OP134,  // Elisa
        OP137,  // Tele2
        OP143,  // Turkcell
        OP147,  // Airtel
        OP148,  // Seatel
        OP150,  // Swisscom
        OP152,  // Optus
        OP154,  // Telia
        OP156,  // Telenor
        OP161,  // Play Poland
        OP175,  // Denmark TDC
        OP176,  // TStar
    }

    private static final String TAG = "MtkOperatorUtils";

    private static final Map<OPID, List> mOPMap = new HashMap<OPID, List>() {
        {
            put(OPID.OP01, Arrays.asList("46000", "46002", "46004", "46007", "46008"));
            put(OPID.OP02, Arrays.asList("46001", "46006", "46009", "45407"));
            put(OPID.OP03, Arrays.asList("20801", "20802", "23101"));
            put(OPID.OP05, Arrays.asList("20201", "20202","20416", "20420", "21630",
                    "21901", "22004", "23001", "23203", "23204", "23430", "26002",
                    "26201", "26206", "26278", "29702"));
            put(OPID.OP06, Arrays.asList("20205", "20404", "21401", "21406", "21670",
                    "22210", "22601", "23415", "23591", "26202", "26204", "26209",
                    "26801", "27201", "27402", "27403", "27801", "28602", "90128"));
            put(OPID.OP07, Arrays.asList("31030", "31070", "31080", "31090", "310150",
                    "310170", "310280", "310380", "310410", "310560", "310680", "311180"));
            put(OPID.OP08, Arrays.asList("310160", "310260", "310490", "310580", "310660"));
            put(OPID.OP09, Arrays.asList("45502", "46003", "46011", "46012", "45507"));
            put(OPID.OP11, Arrays.asList("23420","23594"));
            put(OPID.OP15, Arrays.asList("26203", "26207", "26208", "26211", "26277"));
            put(OPID.OP16, Arrays.asList("23430", "23431", "23432", "23433", "23434",
                    "23486", "23501", "23502"));
            put(OPID.OP18, Arrays.asList("405840","405854", "405855", "405856", "405857",
                    "405858", "405859", "405860", "405861", "405862", "405863", "405864",
                    "405865", "405866", "405867", "405868", "405869", "405870", "405871",
                    "405872", "405873", "405874"));
            put(OPID.OP19, Arrays.asList("50501", "50511", "50571", "50572"));
            put(OPID.OP50, Arrays.asList("44020"));
            put(OPID.OP100, Arrays.asList("45400", "45402", "45410", "45418"));
            put(OPID.OP101, Arrays.asList("45416", "45419", "45420", "45429"));
            put(OPID.OP102, Arrays.asList("45406", "45415", "45417", "45500", "45506"));
            put(OPID.OP103, Arrays.asList("52501", "52502", "52507"));
            put(OPID.OP106, Arrays.asList("45403", "45404", "45405"));
            put(OPID.OP107, Arrays.asList("20809", "20810", "20811", "20813"));
            put(OPID.OP108, Arrays.asList("46697"));
            put(OPID.OP109, Arrays.asList("46692"));
            put(OPID.OP110, Arrays.asList("46601", "46602", "46603", "46606", "46607", "46688"));
            put(OPID.OP117, Arrays.asList("51009", "51028"));
            put(OPID.OP118, Arrays.asList("502152"));
            put(OPID.OP124, Arrays.asList("46605"));
            put(OPID.OP125, Arrays.asList("52005", "52018"));
            put(OPID.OP128, Arrays.asList("24403", "24404", "24412", "24413"));
            put(OPID.OP131, Arrays.asList("52000", "52004", "52088", "52099"));
            put(OPID.OP132, Arrays.asList("72406", "72410", "72411", "72423"));
            put(OPID.OP134, Arrays.asList("24405", "24421"));
            put(OPID.OP137, Arrays.asList("20402", "25020"));
            put(OPID.OP143, Arrays.asList("28601"));
            put(OPID.OP150, Arrays.asList("22801", "29501"));
            put(OPID.OP147, Arrays.asList("40410", "40431", "40440", "40445", "40449", "40551",
                    "40552", "40553", "40554", "40555", "40556", "40490", "40492", "40493",
                    "40494", "40495", "40496", "40497", "40498", "40402", "40403", "40416",
                    "40470"));
            put(OPID.OP148, Arrays.asList("45611"));
            put(OPID.OP152, Arrays.asList("50502"));
            put(OPID.OP154, Arrays.asList("24001", "24005"));
            put(OPID.OP156, Arrays.asList("23802", "23877"));
            put(OPID.OP161, Arrays.asList("26006"));
            put(OPID.OP175, Arrays.asList("23801", "23810"));
            put(OPID.OP176, Arrays.asList("46689"));
        }
    };

    public static boolean isOperator(String mccMnc, OPID id) {
        boolean r = false;
        if (mOPMap.get(id).contains(mccMnc)) {
            r = true;
        }
        Rlog.d(TAG, "" + mccMnc + (r ? " = " : " != ") + idToString(id));
        return r;
    }

    private static String idToString(OPID id) {
        if (id == OPID.OP01) {
            return "OP01";
        } else if (id == OPID.OP02) {
            return "OP02";
        } else if (id == OPID.OP03) {
            return "OP03";
        } else if (id == OPID.OP05) {
            return "OP05";
        } else if (id == OPID.OP06) {
            return "OP06";
        } else if (id == OPID.OP07) {
            return "OP07";
        } else if (id == OPID.OP08) {
            return "OP08";
        } else if (id == OPID.OP09) {
            return "OP09";
        } else if (id == OPID.OP11) {
            return "OP11";
        } else if (id == OPID.OP15) {
            return "OP15";
        } else if (id == OPID.OP16) {
            return "OP16";
        } else if (id == OPID.OP18) {
            return "OP18";
        } else if (id == OPID.OP50) {
            return "OP50";
        } else if (id == OPID.OP100) {
            return "OP100";
        } else if (id == OPID.OP101) {
            return "OP101";
        } else if (id == OPID.OP102) {
            return "OP102";
        } else if (id == OPID.OP106) {
            return "OP106";
        } else if (id == OPID.OP107) {
            return "OP107";
        } else if (id == OPID.OP108) {
            return "OP108";
        } else if (id == OPID.OP109) {
            return "OP109";
        } else if (id == OPID.OP110) {
            return "OP110";
        } else if (id == OPID.OP118) {
            return "OP118";
        } else if (id == OPID.OP124) {
            return "OP124";
        } else if (id == OPID.OP128) {
            return "OP128";
        } else if (id == OPID.OP131) {
            return "OP131";
        } else if (id == OPID.OP134) {
            return "OP134";
        } else if (id == OPID.OP137) {
            return "OP137";
        } else if (id == OPID.OP143) {
            return "OP143";
        } else if (id == OPID.OP152) {
            return "OP152";
        } else if (id == OPID.OP154) {
            return "OP154";
        } else if (id == OPID.OP156) {
            return "OP156";
        } else if (id == OPID.OP161) {
            return "OP161";
        } else if (id == OPID.OP175) {
            return "OP175";
        } else if (id == OPID.OP176) {
            return "OP176";
        }
        return "ERR";
    }
}
