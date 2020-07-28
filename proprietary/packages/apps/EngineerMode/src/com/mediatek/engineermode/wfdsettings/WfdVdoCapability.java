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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERfETO. RECEIVER EXPRESSLY ACKNOWLEDGES
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

package com.mediatek.engineermode.wfdsettings;

import android.app.Activity;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.provider.MtkSettingsExt;

/**
 * Class for config WFD VDO capability.
 *
 */
public class WfdVdoCapability extends Activity implements OnClickListener {
    private static final String TAG = "WfdVdoCapability";
    private static final int RADIO_RESOLUTION_720P_1 = 0;
    private static final int RADIO_RESOLUTION_720P_2 = 2;
    private static final int RADIO_RESOLUTION_1080P = 1;


    private Button  mBtDone = null;

    // 0: 720p(30p), 1: 720p(60p), 2: 1080p(30p)
    private RadioButton[] mRdResoulution =  new RadioButton[2];

    private RadioGroup mRg720pDefinition = null;
    private RadioGroup mRg1080pDefinition = null;

    private CheckBox mCb720pSettingMenu = null;
    private CheckBox mCb1080pSettingMenu = null;
    private CheckBox mCbPortraitResolution = null;

    /* 0: 720p , setting hide
     * 1: 1080p, setting off,
     * 2: 1080p, setting on, high,
     * 3: 1080p, setting on, general.
     */
    private int mResolutionInfo = 0;

    private void submitPortraitReslt() {
        int value = 0;
        if (mCbPortraitResolution.isChecked()) {
            value = 1;
        }
        Elog.i(TAG, "PortraitResolution val:" + value);
        Settings.Global.putInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_PORTRAIT_RESOLUTION, value);
    }


    private final RadioGroup.OnCheckedChangeListener mCheckedListener =
            new RadioGroup.OnCheckedChangeListener() {
        public void onCheckedChanged(RadioGroup group, int checkedId) {
            if (group.equals(mRg720pDefinition)) {
                if (checkedId == R.id.Wfd_Vdo_720p_def_l) {
                    Elog.v(TAG, "check 720-720p(30p)");
                }
            }

            if (group.equals(mRg1080pDefinition)) {
                if (checkedId == R.id.Wfd_Vdo_def_f) {
                    Elog.v(TAG, "check 1080-full");
                } else if (checkedId == R.id.Wfd_Vdo_def_l) {
                    Elog.v(TAG, "check 1080-720p(30p)");
                }
            }
        }
    };

    private final View.OnClickListener mRdClickListener = new View.OnClickListener() {
        public void onClick(View v) {
                boolean newState = false;
                if (v.equals(mRdResoulution[RADIO_RESOLUTION_720P_1])) {
                    newState = mRdResoulution[RADIO_RESOLUTION_720P_1].isChecked();
                    if (newState == true) {
                        mRdResoulution[RADIO_RESOLUTION_1080P].setChecked(false);
                        mCb720pSettingMenu.setVisibility(View.GONE);
                        mRg720pDefinition.setVisibility(View.GONE);
                        mCb1080pSettingMenu.setVisibility(View.GONE);
                        mRg1080pDefinition.setVisibility(View.GONE);
                    }
                } else if (v.equals(mRdResoulution[RADIO_RESOLUTION_1080P])) {
                    newState = mRdResoulution[RADIO_RESOLUTION_1080P].isChecked();
                    if (newState == true) {
                        mRdResoulution[RADIO_RESOLUTION_720P_1].setChecked(false);
                        mCb720pSettingMenu.setVisibility(View.GONE);
                        mRg720pDefinition.setVisibility(View.GONE);
                        mCb1080pSettingMenu.setVisibility(View.VISIBLE);
                        mRg1080pDefinition.setVisibility(View.VISIBLE);
                        if (mCb1080pSettingMenu.isChecked() == false) {
                            for (int i = 0; i < mRg1080pDefinition.getChildCount(); i++) {
                                mRg1080pDefinition.getChildAt(i).setEnabled(false);
                            }
                        }
                    }
                }
            }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wfd_vdo_capability);

        mRdResoulution[RADIO_RESOLUTION_720P_1] = (RadioButton) findViewById(R.id.Wfd_Vdo_720p_1);
        mRdResoulution[RADIO_RESOLUTION_1080P] = (RadioButton) findViewById(R.id.Wfd_Vdo_1080p);

        for (int i = 0; i < mRdResoulution.length; i++) {
            mRdResoulution[i].setOnClickListener(mRdClickListener);
        }
        mCb720pSettingMenu = (CheckBox) findViewById(R.id.Wfd_Setting_Menu_720p);
        mRg720pDefinition = (RadioGroup) findViewById(R.id.Wfd_Vdo_Definition_720p);

        mCb1080pSettingMenu = (CheckBox) findViewById(R.id.Wfd_Setting_Menu_1080p);
        mRg1080pDefinition = (RadioGroup) findViewById(R.id.Wfd_Vdo_Definition_1080p);

        mBtDone = (Button) findViewById(R.id.Wfd_Done);
        mBtDone.setOnClickListener(this);

        mCbPortraitResolution = (CheckBox) findViewById(R.id.wfd_vdo_portrait_resolution_cb);
        int portraitStatus = 0;
        portraitStatus = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_PORTRAIT_RESOLUTION, 1);
        if (portraitStatus == 1) {
            mCbPortraitResolution.setChecked(true);
        }

        mResolutionInfo = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_RESOLUTION, 0);

        setInitState(mResolutionInfo);

        mRg720pDefinition.setOnCheckedChangeListener(mCheckedListener);
        mRg1080pDefinition.setOnCheckedChangeListener(mCheckedListener);

        mCb1080pSettingMenu.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCb1080pSettingMenu.isChecked();
                Elog.i(TAG, "Enable1080pSettingMenu : " + newState);
                for (int i = 0; i < mRg1080pDefinition.getChildCount(); i++) {
                    mRg1080pDefinition.getChildAt(i).setEnabled(newState);
                }

                if (mRg1080pDefinition.getCheckedRadioButtonId() != R.id.Wfd_Vdo_def_l &&
                    mRg1080pDefinition.getCheckedRadioButtonId() != R.id.Wfd_Vdo_def_f) {
                    mRg1080pDefinition.check(R.id.Wfd_Vdo_def_f);
                }
            }
        });
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == mBtDone.getId()) {
            onClickBtnDone();
            finish();
        }
    }

    private void setInitState(int state) {
        if (state == 0) {
            mRdResoulution[RADIO_RESOLUTION_720P_1].setChecked(true);
            mCb1080pSettingMenu.setVisibility(View.GONE);
            mRg1080pDefinition.setVisibility(View.GONE);
        } else if (state == 1) {
            mRdResoulution[RADIO_RESOLUTION_1080P].setChecked(true);
            mCb1080pSettingMenu.setVisibility(View.GONE);
            mRg1080pDefinition.setVisibility(View.GONE);
        } else if (state == 2 || state == 3) {
            mRdResoulution[RADIO_RESOLUTION_1080P].setChecked(true);
            mCb1080pSettingMenu.setVisibility(View.VISIBLE);
            mRg1080pDefinition.setVisibility(View.VISIBLE);
            mCb1080pSettingMenu.setChecked(true);
            mRg1080pDefinition.setEnabled(true);
            if (state == 2) {
                mRg1080pDefinition.check(R.id.Wfd_Vdo_def_f);
            } else if (state == 3) {
                mRg1080pDefinition.check(R.id.Wfd_Vdo_def_l);
            }

        } else {
            Elog.w(TAG, "Wrong input resolution info:" + state);
        }
    }



    private void onClickBtnDone() {
        int state = 0;

        if (mRdResoulution[RADIO_RESOLUTION_720P_1].isChecked()) {
            state = 0;
        } else {
            if (mCb1080pSettingMenu.isChecked() == false) {
                state = 1;
            } else {
                if (mRg1080pDefinition.getCheckedRadioButtonId() == R.id.Wfd_Vdo_def_f) {
                    state = 2;
                } else {
                    state = 3;
                }
            }
        }

        Settings.Global.putInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_RESOLUTION, state);

        Elog.v(TAG, "Last solution = " + state);
        submitPortraitReslt();
    }
}
