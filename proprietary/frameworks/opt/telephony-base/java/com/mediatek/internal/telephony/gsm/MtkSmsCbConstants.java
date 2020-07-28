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

/**
 * Constants used in SMS Cell Broadcast messages (see 3GPP TS 23.041). This class is used by the
 * boot-time broadcast channel enable and database upgrade code in CellBroadcastReceiver, so it
 * is public, but should be avoided in favor of the radio technology independent constants in
 * {@link android.telephony.SmsCbMessage}, {@link android.telephony.SmsCbEtwsInfo}, and
 * {@link android.telephony.SmsCbCmasInfo} classes.
 *
 * {@hide}
 */
public class MtkSmsCbConstants {

    /** Private constructor for utility class. */
    private MtkSmsCbConstants() { }

    /** Channel 911 required by Taiwan NCC. ID 0~999 is allocated by GSMA */
    public static final int MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_911
            = 0x038F; // 911

    /** Channel 919 required by Taiwan NCC and Israel. ID 0~999 is allocated by GSMA */
    public static final int MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_919
            = 0x0397; // 919

    /** Channel 921 required by Chile. ID 0~999 is allocated by GSMA */
    public static final int MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_921
            = 0x399;

    /** CMAS Message Identifier for CMAS Public Safety Alerts */
    public static final int MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT
            = 0x112C; // 4396

    /** CMAS Message Identifier for CMAS Public Safety Alerts for additional languages */
    public static final int MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT_LANGUAGE
            = 0x112D; // 4397

    /** CMAS Message Identifier for CMAS State/Local WEA test */
    public static final int MESSAGE_ID_CMAS_WEA_TEST
            = 0x112E; // 4398

    /** CMAS Message Identifier for CMAS State/Local WEA test for additional languages */
    public static final int MESSAGE_ID_CMAS_WEA_TEST_LANGUAGE
            = 0x112F; // 4399

    /** WHAM Message Identifier */
    public static final int MESSAGE_ID_WHAM
            = 0x1130; // 4400
}
