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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.camera.tests.v3.observer;

/**
 * Observer used to check focus mode has been changed normally.
 */
public class FocusModeChangeObserver extends LogListPrintObserver {
    private static final String LOG_TAG_AF_MGR = "af_mgr_v3";

    public static final int INDEX_INFINITY = 0;
    public static final int INDEX_AUTO = 1;
    public static final int INDEX_MACRO = 2;
    public static final int INDEX_CONTINUOUS_VIDEO = 3;
    public static final int INDEX_CONTINUOUS_PICTURE = 4;
    public static final int INDEX_EDOF = 5;

    private static final String[] TAG = new String[]{
            LOG_TAG_AF_MGR,
            LOG_TAG_AF_MGR
    };

    private static final String[] LOG_KEY_LIST_AUTO = new String[]{
            "ctl_afmode(1)",
            "EVENT_CMD_CHANGE_MODE"
    };

    private static final String[] LOG_KEY_LIST_CONTINUOUS_VIDEO = new String[]{
            "ctl_afmode(3)",
            "EVENT_CMD_CHANGE_MODE"
    };

    private static final String[] LOG_KEY_LIST_CONTINUOUS_PICTURE = new String[]{
            "ctl_afmode(4)",
            "EVENT_CMD_CHANGE_MODE"
    };

    @Override
    protected String[] getObservedTagList(int index) {
        return TAG;
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        switch (index) {
            case INDEX_INFINITY:
                return null;
            case INDEX_AUTO:
                return LOG_KEY_LIST_AUTO;
            case INDEX_MACRO:
                return null;
            case INDEX_CONTINUOUS_VIDEO:
                return LOG_KEY_LIST_CONTINUOUS_VIDEO;
            case INDEX_CONTINUOUS_PICTURE:
                return LOG_KEY_LIST_CONTINUOUS_PICTURE;
            case INDEX_EDOF:
                return null;
            default:
                return null;
        }
    }

    @Override
    public boolean isSupported(int index) {
        return true;
    }

    @Override
    public int getObserveCount() {
        return 6;
    }
}
