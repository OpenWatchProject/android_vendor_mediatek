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


import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.audio.AudioTuningJni;

/**
 * Class for VoiceWakeupDetector.
 *
 */
public class VoiceWakeupDetector extends Activity implements OnClickListener {

    private static final String TAG = "VoiceWakeupDetector";
    private static final int DETECTOR_PAR_COUNT = 10;
    private Spinner[] mSpnParArr = new Spinner[DETECTOR_PAR_COUNT];
    private Button mBtnSet;
    private static final String CATEGORY = "VOW";
    private static final String TYPE = "VOW,VOW_common";
    private static final String KEY_PRE = "Par_";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.voice_wakeup_detector);
        initUiComponent();
    }

    private void initUiComponent() {
        int[] spnIds = {
                R.id.voice_detector_par01,
                R.id.voice_detector_par02,
                R.id.voice_detector_par03,
                R.id.voice_detector_par04,
                R.id.voice_detector_par05,
                R.id.voice_detector_par06,
                R.id.voice_detector_par07,
                R.id.voice_detector_par08,
                R.id.voice_detector_par09,
                R.id.voice_detector_par10,
        };
        for (int i = 0; i < DETECTOR_PAR_COUNT; i++) {
            Spinner spn = (Spinner) findViewById(spnIds[i]);
            mSpnParArr[i] = spn;
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            int maxIdx = 0;
            if (i <= 4) {
                maxIdx = 15;
            } else {
                maxIdx = 7;
            }
            for (int j = 0; j <= maxIdx; j++) {
                adapter.add(String.valueOf(j));
            }
            spn.setAdapter(adapter);
        }
        mBtnSet = (Button) findViewById(R.id.voice_detector_set_btn);
        mBtnSet.setOnClickListener(this);

        initUiByData();
    }

    private void initUiByData() {
        for (int i = 0; i < mSpnParArr.length; i++) {
            int val = getValue(i);
            mSpnParArr[i].setSelection(val);
            Elog.i(TAG, "val:" + val);
        }
    }

    private void setDetectorSetting() {
        for (int i = 0; i < mSpnParArr.length; i++) {
            int selection = mSpnParArr[i].getSelectedItemPosition();
            String strKey = KEY_PRE + String.format("%02d", i + 1);
            Elog.i(TAG, "strKey:" + strKey);
            AudioTuningJni.setParams(CATEGORY, TYPE, strKey, String.valueOf(selection));

        }
        AudioTuningJni.saveToWork(CATEGORY);
    }

    private int getValue(int index) {
        String strKey = KEY_PRE + String.format("%02d", index + 1);
        Elog.i(TAG, "strKey:" + strKey);
        String result = AudioTuningJni.getParams(CATEGORY, TYPE, strKey);
        Elog.i(TAG, "result:" + result);
        return Integer.parseInt(result);
    }

    private boolean checkSetResult() {
        for (int i = 0; i < mSpnParArr.length; i++) {
            int selection = mSpnParArr[i].getSelectedItemPosition();
            int value = getValue(i);
            if (selection != value) {
                return false;
            }
        }
        return true;
    }

    @Override
    public void onClick(View view) {
        if (view == mBtnSet) {
            setDetectorSetting();
            int msgid = -1;
            if (checkSetResult()) {
                msgid = R.string.voice_set_success_msg;
            } else {
                msgid = R.string.voice_set_fail_msg;
            }
            Toast.makeText(this, msgid, Toast.LENGTH_SHORT).show();
        }

    }
}
