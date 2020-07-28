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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.bandselect;

public class BandModeContent {
    /** GSM mode bit. */
    public static final int GSM_EGSM900_BIT = 1;
    public static final int GSM_DCS1800_BIT = 3;
    public static final int GSM_PCS1900_BIT = 4;
    public static final int GSM_GSM850_BIT = 7;
    public static final int [] GSM_BAND_BIT =
    {GSM_EGSM900_BIT,GSM_DCS1800_BIT,GSM_PCS1900_BIT,GSM_GSM850_BIT};

    /** Event or message id. */
    public static final int EVENT_QUERY_SUPPORTED_GSM = 100;
    public static final int EVENT_QUERY_CURRENT_GSM = 101;
    public static final int EVENT_QUERY_CURRENT_CDMA = 102;
    public static final int EVENT_QUERY_SUPPORTED_NR = 103;
    public static final int EVENT_QUERY_CURRENT_NR = 104;
    public static final int EVENT_SET_CDMA = 111;
    public static final int EVENT_SET_NR = 112;
    public static final int EVENT_SET_BUTTON = 110;


    public static final int EVENT_SET_FAIL = 1;
    public static final int EVENT_RESET = 2;

    public static final long GSM_MAX_VALUE = 0xFFL;
    public static final long UMTS_MAX_VALUE = 0xFFFFL;
    public static final long LTE_MAX_VALUE = 0xFFFFFFFFL;

    /** AT Command. */
    public static final String QUERY_SUPPORT_COMMAND = "AT+EPBSE=?";
    public static final String QUERY_CURRENT_COMMAND = "AT+EPBSE?";
    public static final String SET_COMMAND = "AT+EPBSE=";
    public static final String SAME_COMMAND = "+EPBSE:";

    public static final String QUERY_CURRENT_COMMAND_CDMA = "AT+ECBANDCFG?";
    public static final String SET_COMMAND_CDMA = "AT+ECBANDCFG=";
    public static final String SAME_COMMAND_CDMA = "+ECBANDCFG:";


    public static final String QUERY_SUPPORT_NR_COMMAND = "AT+EPBSEH=?";
    public static final String QUERY_CURRENT_NR_COMMAND = "AT+EPBSEH?";
    public static final String SET_NR_COMMAND = "AT+EPBSEH=";
    public static final String SAME_NR_COMMAND = "+EPBSEH:";
}
