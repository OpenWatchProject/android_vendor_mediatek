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

package com.mediatek.engineermode.voicesettings;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ListActivity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.R;

import java.util.ArrayList;

/**
 * Show Voice application list.
 *
 * @author mtk54040
 *
 */
public class VoiceList extends ListActivity {

    private static final String MTK_VOW_SUPPORT = "MTK_VOW_SUPPORT";
    private static final String MTK_VOW_SUPPORT_ON = "MTK_VOW_SUPPORT=true";
    private static final String MTK_VOW_TRIGGER_SUPPORT = "MTK_VOW_2E2K_SUPPORT";
    private static final String MTK_VOW_TRIGGER_SUPPORT_ON = "MTK_VOW_2E2K_SUPPORT=true";
    private static final int DIALOG_NOT_SUPPORT = 0;
    private ArrayList<String> mModuleList = null;

    /**
     * used to check whether voice wakeup feature was supported.
     * @param context android context instance
     * @return if true, the feature was supported
     */
    private static boolean isVoiceWakeupSupported(Context context) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (am == null) {
            return false;
        }
        String state = am.getParameters(MTK_VOW_SUPPORT);
        if (state != null) {
            return state.equalsIgnoreCase(MTK_VOW_SUPPORT_ON);
        }
        return false;
    }

    /**
     * used to check whether voice trigger feature is supported.
     * @param context android context instance
     * @return if true, the feature was supported
     */
    public static boolean isVoiceTriggerSupported(Context context) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (am == null) {
            return false;
        }
        String state = am.getParameters(MTK_VOW_TRIGGER_SUPPORT);
        if (state != null) {
            return state.equalsIgnoreCase(MTK_VOW_TRIGGER_SUPPORT_ON);
        }
        return false;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.voice_list);
        if (!VoiceList.isVoiceWakeupSupported(this)) {
            showDialog(DIALOG_NOT_SUPPORT);
            return;
        }
        mModuleList = new ArrayList<String>();
        if (isVoiceWakeupSupported(this)) {
            mModuleList.add(getString(R.string.voice_wakeup_detector));
            mModuleList.add(getString(R.string.voice_wakeup_recognition));
        }

        ArrayAdapter<String> moduleAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mModuleList);
        setListAdapter(moduleAdapter);

    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        if (mModuleList.get(position).equals(
                getString(R.string.voice_settings_wakeup))) {
                startActivity(new Intent(VoiceList.this, VoiceWakeup.class));

        } else if (mModuleList.get(position).equals(
                getString(R.string.voice_wakeup_detector))) {
            startActivity(new Intent(VoiceList.this, VoiceWakeupDetector.class));
        } else if (mModuleList.get(position).equals(
                getString(R.string.voice_wakeup_recognition))) {
            startActivity(new Intent(VoiceList.this, VoiceWakeupRecognition.class));
        }
    }


    @Override
    protected Dialog onCreateDialog(int id) {
        // TODO Auto-generated method stub
        switch (id) {
        case DIALOG_NOT_SUPPORT:
            return new AlertDialog.Builder(this).setTitle(R.string.voice_settings_warning)
                        .setCancelable(false)
                        .setMessage(getString(R.string.voice_settings_not_support))
                        .setPositiveButton(R.string.dialog_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    }).create();
        default:
            return null;
        }

    }

}
