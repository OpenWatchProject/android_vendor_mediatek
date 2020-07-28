/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.fullscreenswitch;

import android.net.Uri;
import android.provider.BaseColumns;

public class FullscreenSwitch {

    public static final String PACKAGES = "packages";
    public static final String PACKAGE_ITEM = PACKAGES + "/#";
    public static final String AUTHORITIES = "com.mediatek.fullscreenswitch";
    public static final Uri CONTENT_URI_PACKAGES = Uri.parse("content://"
            + AUTHORITIES + "/" + PACKAGES);
    public static final Uri CONTENT_URI_PACKAGE_ITEM = Uri.parse("content://"
            + AUTHORITIES + "/" + PACKAGE_ITEM);

    public static final String SQL_PACKAGE_SELECTION = FullscreenModeColoums.COLOUM_PACKAGE
            + "=?";
    public static final String SQL_ORDER_BY_ID = FullscreenModeColoums._ID
            + " ASC";

    public static final String[] COLOUMS_ALL = new String[] {
            FullscreenModeColoums._ID, FullscreenModeColoums.COLOUM_PACKAGE,
            FullscreenModeColoums.COLOUM_FULLSCREEN_MODE,
            FullscreenModeColoums.COLOUM_APP_LEVEL,
            FullscreenModeColoums.COLOUM_TYPE };

    public static class FullscreenModeColoums implements BaseColumns {
        public static final String COLOUM_PACKAGE = "package";
        public static final String COLOUM_FULLSCREEN_MODE = "fullscreen_mode";
        public static final String COLOUM_APP_LEVEL = "app_level";
        public static final String COLOUM_TYPE = "type";
    }
}
