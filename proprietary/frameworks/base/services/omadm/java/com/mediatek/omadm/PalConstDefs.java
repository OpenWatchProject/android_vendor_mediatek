/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.omadm;

import android.telephony.TelephonyManager;
import android.util.Slog;

/**
 * @hide
 */
public class PalConstDefs {

    public static boolean DEBUG = true;

    public static final String TAG = PalConstDefs.class.getSimpleName();

    public static final int RET_SUCC = 0;
    public static final int RET_ERR = 1;
    public static final int RET_ERR_ARGS = 7;
    public static final int RET_ERR_UNDEF = 8;
    public static final int RET_ERR_STATE = 10;
    public static final int RET_ERR_NORES = 11;
    public static final int RET_ERR_NOPERM = 12;
    public static final int RET_ERR_TMOUT = 13;

    public final static String TELEPHONYMANAGER        = "TelephonyManager";
    public final static String SIM_STATE               = "Sim state";
    public final static String MANUFACTURER            = "manufacturer";

    public final static String PERMISSION_DENIED       = "Permission denied";
    public final static String OPERATION_NOT_SUPPORTED = "Operation not supported";
    public final static String NOT_READY               = "is not ready";
    public final static String NOT_IDENTIFIED          = "is not identified";
    public final static String VERSION                 = "1.2";
    public final static int    PARENT_DEVICE           = 0;
    public final static int    CURRENT_DEVICE          = 1;
    public final static int    CHILD_DEVICE            = 2;

    public final static String EMPTY_STRING            = "";
    public final static String NULL_STRING             = "NULL";
    public final static String MO_WORK_PATH            = "/data/vendor/verizon/dmclient/data";
    public final static String FIRMWARE_VER_PATH       = "/last_update_firmware_version";
    public final static String APN_CFG_IDEX            = "/apn_cfg_idx";
    public final static String CONFIGURATION_VER_PATH  = "/last_update_configuration_version";

    public final static String SMART_DEVICE            = "Smart Device";
    public final static String FEATURE_PHONE           = "Feature Phone";
    public final static String HOST_OPERATION          = "Host device operation";
    public final static String NOT_AVAILABLE           = "n/a";
    public final static String NOT_AVAILABLE_UPPERCASE = "N/A";

    public final static String IMS_DOMAIN              = "vzims.com";

    public final static int    ADMIN_NET_AVAILABLE     = 1;
    public final static int    ADMIN_NET_UNAVAILABLE   = 2;
    public final static int    ADMIN_NET_LOST          = 3;

    public static void throwEcxeption(int err) {
        switch(err) {
            default:
            case RET_ERR:
            case RET_ERR_UNDEF:
                throw new NullPointerException(Integer.toString(err));
            case RET_ERR_ARGS:
                throw new IllegalArgumentException(Integer.toString(err));
            case RET_ERR_STATE:
                throw new IllegalStateException(Integer.toString(err));
            case RET_SUCC:
                //If status is success then no need to throw exception
                break;
        }
    }
}
