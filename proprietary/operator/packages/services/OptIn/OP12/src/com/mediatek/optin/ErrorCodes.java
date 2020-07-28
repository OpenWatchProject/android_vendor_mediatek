/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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


package com.mediatek.optin;


/**
 * E911 error codes & levels.
 */
public class ErrorCodes {

    private static final String TAG = "OP12ErrorCodes";

    /********************* E911 Error Code: Begin ******************/
    /* Query address only. Successful address query. Add address only. Successful address add.*/
    public static final int E911_ERR_CODE_SUCCESS = 0;
    /* Invalid header params. Does not apply */
    public static final int E911_ERR_CODE_INVALID_USERNAME_PWD = 12001;
    /* Invalid client id. Does not apply */
    public static final int E911_ERR_CODE_INVALID_CLIENT_ID = 12002;
    /* General error */
    public static final int E911_ERR_CODE_INTERNAL_SERVER_ERROR = 13000;
    /* Invalid response from SPC authentication service.*/
    public static final int E911_ERR_CODE_INVALID_TOKEN = 13013;
    // TODO: SAME ERROR CODE
    /* SPC token was not provided */
//    public static int E911_ERR_CODE_TOKEN_NOT_PROVIDED = 13013;
    /* Request timed out */
    public static final int E911_ERR_CODE_TIMEOUT = 13019;
    /* Full Address add only. Address is invalid, alternate address found. */
    public static final int E911_ERR_CODE_ADR_INVALID_ALT_ADR_FOUND = 13025;
    /* Query address only. Account does not have an E911Address on account. */
    public static final int E911_ERR_CODE_NO_RECORD_FOUND = 13026;
    /* Full address add only. Address is invalid, no alternate addresses were supplied */
    public static final int E911_ERR_CODE_ADR_INVALID_ALT_ADR_NOT_FOUND = 13027;
    /* Partial address add only. Address is invalid, no alternate addresses were supplied */
    public static final int E911_ERR_CODE_PARTIAL_ADR_INVALID = 13028;

    // TODO: will server send codes as above or strings as below?????
    public static final String FOUND = "FOUND";
    public static final String NOTFOUND = "NOTFOUND";
    public static final String INVALID = "INVALID";
    public static final String SUCCESS = "SUCCESS";

    /********************** E911 Error Code: end **********************/

    /********************** Error Levels: Begin *******************/
    public static final int E911_ERR_LEVEL_0 = 0;
    public static final int E911_ERR_LEVEL_1 = 1;
    public static final int E911_ERR_LEVEL_2 = 2;
    public static final int E911_ERR_LEVEL_3 = 3;
    /********************** Error Levels: End ********************/

    /********************* Internal errors: Begin **********************/
    public static final int E911_ERR_MALFORMED_URL = 50;
    public static final int E911_ERR_PROTOCOL_EXP = 51;
    public static final int E911_ERR_SOCKET_IO_EXP = 52;
    public static final int E911_ERR_IO_EXP = 53;
    public static final int E911_ERR_OTHER_EXP = 54;
    public static final int E911_ERR_USER_CANCELED = 55;
    /********************* Internal errors: End **********************/
}

