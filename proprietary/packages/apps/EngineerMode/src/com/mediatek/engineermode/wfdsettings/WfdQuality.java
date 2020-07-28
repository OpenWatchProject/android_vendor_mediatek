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
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.provider.MtkSettingsExt;

/**
 * Class for WFD quality.
 *
 */
public class WfdQuality extends Activity implements OnClickListener {
    private static final String TAG = "WfdQuality";

    private EditText mEtToastTime = null;
    private EditText mEtNotifTime = null;
    private Button  mBtDone = null;
    private CheckBox mCbSqcInfo = null;
    private CheckBox mCbQeInfo = null;
    private CheckBox mCbAcsInfo = null;

    private int mToastTime = 0;
    private int mNotifTime = 0;
    private int mSqcInfo = 0;
    private int mQeInfo = 0;
    private int mAcsInfo = 0; //Auto channel selection

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wfd_quality);

        mEtToastTime = (EditText) findViewById(R.id.Wfd_Display_Toast_Time_Edit);
        mEtNotifTime = (EditText) findViewById(R.id.Wfd_Display_Notif_Time_Edit);
        mCbSqcInfo = (CheckBox) findViewById(R.id.Wfd_Sqc_Info);
        mCbQeInfo = (CheckBox) findViewById(R.id.Wfd_qe_Info);
        mCbAcsInfo = (CheckBox) findViewById(R.id.Wfd_acs_Info);
        mBtDone = (Button) findViewById(R.id.Wfd_Done);
        mBtDone.setOnClickListener(this);

        mToastTime = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_DISPLAY_TOAST_TIME, 0);
        mNotifTime = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_DISPLAY_NOTIFICATION_TIME, 0);
        mSqcInfo = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_SQC_INFO_ON, 0);
        mCbSqcInfo.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCbSqcInfo.isChecked();
                mSqcInfo = newState ? 1 : 0;
                Settings.Global.putInt(getContentResolver(),
                        MtkSettingsExt.Global.WIFI_DISPLAY_SQC_INFO_ON, mSqcInfo);
            }
        });

        mQeInfo = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_QE_ON, 0);
        mCbQeInfo.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCbQeInfo.isChecked();
                mQeInfo = newState ? 1 : 0;
                Settings.Global.putInt(getContentResolver(),
                        MtkSettingsExt.Global.WIFI_DISPLAY_QE_ON, mQeInfo);
            }
        });
        mAcsInfo = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_AUTO_CHANNEL_SELECTION, 0);
        mCbAcsInfo.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCbAcsInfo.isChecked();
                mAcsInfo = newState ? 1 : 0;
                Settings.Global.putInt(getContentResolver(),
                        MtkSettingsExt.Global.WIFI_DISPLAY_AUTO_CHANNEL_SELECTION, mAcsInfo);
            }
        });

        mEtToastTime.setText(String.valueOf(mToastTime));
        mEtNotifTime.setText(String.valueOf(mNotifTime));

        if (mSqcInfo == 0) {
            mCbSqcInfo.setChecked(false);
        } else {
            mCbSqcInfo.setChecked(true);
        }

        if (mQeInfo == 0) {
            mCbQeInfo.setChecked(false);
        } else {
            mCbQeInfo.setChecked(true);
        }

        if (mAcsInfo == 0) {
            mCbAcsInfo.setChecked(false);
        } else {
            mCbAcsInfo.setChecked(true);
        }
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == mBtDone.getId()) {
            if (onClickBtnDone()) {
                finish();
            }
        }
    }
    private boolean onClickBtnDone() {
        try {
            mToastTime = Integer.valueOf(mEtToastTime.getText().toString());
            mNotifTime = Integer.valueOf(mEtNotifTime.getText().toString());
        } catch (NumberFormatException e) {
            Toast.makeText(this, "Toast time or Notification time is invalid",
                    Toast.LENGTH_SHORT).show();
            return false;
        }

        Settings.Global.putInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_DISPLAY_TOAST_TIME, mToastTime);
        Settings.Global.putInt(getContentResolver(),
                MtkSettingsExt.Global.WIFI_DISPLAY_DISPLAY_NOTIFICATION_TIME, mNotifTime);

        Elog.i(TAG, "mToastTime: " + mToastTime + " mNotifTime: " + mNotifTime);
        Elog.i(TAG, "Sqc info: " + mCbSqcInfo.isChecked() + " Quality enhancement : "
                + mCbQeInfo.isChecked() + " Auto channel selection : " + mCbAcsInfo.isChecked());
        return true;
    }


}
