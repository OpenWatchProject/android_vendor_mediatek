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



package com.mediatek.internal.telephony.gsm;

import android.os.SystemProperties;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;

/**
 * Represents a Supplementary Service CRSS Notification received from the network.
 *
 * {@hide}
 */
public class MtkSuppCrssNotification {

    /* Used by RIL_UNSOL_CRSS_NOTIFICATION sync to RIL_CrssNotification */

    public int code;            /*
                                 * 0: +CCWA
                                 * 1: +CDIP
                                 * 2: +CLIP
                                 * 3: +COLP
                                 */
    public int type;            /*
                                 * type of address octet in integer format
                                 *  (refer GSM 04.08 [8] subclause 10.5.4.7)
                                 */
    public String number;      /* string type phone number of format specified by <type> */
    public String alphaid;     /*
                                 * optional string type alphanumeric representation of <number>
                                 * corresponding to the entry found in phonebook;
                                 */
    public int cli_validity;   /* CLI validity value,
                                  0: PRESENTATION_ALLOWED,
                                  1: PRESENTATION_RESTRICTED,
                                  2: PRESENTATION_UNKNOWN
                               */

    static public final int CRSS_CALL_WAITING             = 0;
    static public final int CRSS_CALLED_LINE_ID_PREST     = 1;
    static public final int CRSS_CALLING_LINE_ID_PREST    = 2;
    static public final int CRSS_CONNECTED_LINE_ID_PREST  = 3;

    static private final boolean SDBG = SystemProperties.get("ro.build.type").equals("user")
            ? false : true;

    public String toString()
    {
        return super.toString() + " CRSS Notification:"
            + " code: " + code
            + " \""
            + Rlog.pii(SDBG, PhoneNumberUtils.stringFromStringAndTOA(number, type)) + "\" "
            + alphaid + " cli_validity: " + cli_validity;
    }

}

