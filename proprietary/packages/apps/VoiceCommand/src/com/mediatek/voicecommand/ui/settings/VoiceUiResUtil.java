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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.voicecommand.ui.settings;

import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.util.Log;

/**
 * Voice Setting UI resource manager class.
 * 
 */
public class VoiceUiResUtil {
    private final static String TAG = "VoiceUiResUtil";

    private final static int APP_PHONE = 1;
    private final static int APP_GALLERY3D = 2;
    private final static int APP_DESKCLOCK = 3;
    private final static int APP_MUSIC = 4;

    /**
     * According to the process ID, get the process summary id.
     * 
     * @param processID
     *            process id in voiceprocess.xml
     * @return the process summary id
     */
    public static int getSummaryResourceId(int processID) {
        if (processID == APP_DESKCLOCK) {
            return R.string.alarm_command_summary_format;
        } else if (processID == APP_PHONE) {
            return R.string.incomming_command_summary_format;
        } else if (processID == APP_MUSIC) {
            return R.string.music_command_summary_format;
        } else if (processID == APP_GALLERY3D) {
            return R.string.camera_command_summary_format;
        } else {
            Log.i(TAG, "[getSummaryResourceId]voice ui not support processID:" + processID);
            return 0;
        }
    }

    /**
     * According to the process ID, get the process icon id.
     * 
     * @param processID
     *            process id in voiceprocess.xml
     * @return the process icon id
     */
    public static int getIconId(int processID) {
        if (processID == APP_DESKCLOCK) {
            return R.drawable.ic_menu_alarm;
        } else if (processID == APP_PHONE) {
            return R.drawable.ic_menu_call;
        } else if (processID == APP_GALLERY3D) {
            return R.drawable.ic_menu_camera;
        } else {
            Log.i(TAG, "[getIconId]voice ui not support processID:" + processID);
            return 0;
        }
    }

    /**
     * According to the process ID, get the process title id.
     * 
     * @param processID
     *            process id in voiceprocess.xml
     * @return the process title id
     */
    public static int getProcessTitleResourceId(int processID) {
        if (processID == APP_DESKCLOCK) {
            return R.string.alarm_app_name;
        } else if (processID == APP_PHONE) {
            return R.string.incoming_call_app_name;
        } else if (processID == APP_MUSIC) {
            return R.string.music_app_name;
        } else if (processID == APP_GALLERY3D) {
            return R.string.camera_app_name;
        } else {
            Log.i(TAG, "[getProcessTitleResourceId]voice ui not support processID:" + processID);
            return 0;
        }
    }

    /**
     * According to the process ID, get the command title id.
     * 
     * @param processID
     *            process id in voiceprocess.xml
     * @return the command title id
     */
    public static int getCommandTitleResourceId(int processID) {
        if (processID == APP_DESKCLOCK) {
            return R.string.voice_ui_alarm_command_title;
        } else if (processID == APP_PHONE) {
            return R.string.voice_ui_phone_command_title;
        } else if (processID == APP_GALLERY3D) {
            return R.string.voice_ui_camera_command_title;
        } else {
            Log.i(TAG, "[getCommandTitleResourceId]voice ui not support processID:" + processID);
            return 0;
        }
    }

    /**
     * Get the preference id in wakeup of title.
     * 
     * @return the preference id
     */
    public static int getWakeupTitleResourceId() {
        return R.string.voice_wakeup_title;
    }

    /**
     * Get the preference id in wakeup by anyone.
     * 
     * @return the preference id
     */
    public static int getWakeupAnyoneResourceId() {
        return R.string.wakeup_by_anyone_summary;
    }

    /**
     * Get the preference id in wakeup by command owner.
     * 
     * @return the preference id
     */
    public static int getWakeupCommandResourceId() {
        return R.string.wakeup_by_command_summary;
    }
}
