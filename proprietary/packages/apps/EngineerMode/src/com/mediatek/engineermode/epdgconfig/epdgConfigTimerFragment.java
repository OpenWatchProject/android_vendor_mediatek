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

package com.mediatek.engineermode.epdgconfig;

import android.app.AlertDialog;
import android.app.Dialog;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

public class epdgConfigTimerFragment extends Fragment {
    private final static  String TAG = "epdgConfig/TimerFragment";
    private final static int UI_DATA_INVALID = 0;
    private final static int DATA_SET_FAIL = 1;
    private static int mInvalidPara = -1;
    private static String mSetFailPara = "";
    private static String mErrMsg = "";
    private static boolean mChange = false;
    private EditText mIkeRekeyTimer;
    private EditText mEspRekeyTimer;
    private EditText mRekeyMargin;
    private EditText mDpdTimer;
    private EditText mKeepTimer;
    private EditText mEspSetupTimer;
    private EditText mPdnSetupTimer;
    private EditText mDnsTimer;
    private EditText mOos;
    private EditText mDetachSoftTimer;
    private EditText mDetachHardTimer;
    private Button mSet;
    private String mAtCmd = "wodemset=";
    private String[] mCfg = {"ike_rekey_timer", "esp_rekey_timer", "rekey_margin",
            "dpd_timer", "keep_timer", "esp_setup_time", "pdn_setup_time", "dns_timer", "oos",
            "detach_soft_timer", "detach_hard_timer"};
    private EditText[] mEditText;
    private String[] mEditStatus;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.epdg_timer, container, false);
        Elog.d(TAG, "epdgConfigTimerFragment create");
        mIkeRekeyTimer = (EditText) view.findViewById(R.id.ike_rekey_timer);
        mEspRekeyTimer = (EditText) view.findViewById(R.id.esp_rekey_timer);
        mRekeyMargin = (EditText) view.findViewById(R.id.rekey_margin);
        mDpdTimer = (EditText) view.findViewById(R.id.dpd_timer);
        mKeepTimer = (EditText) view.findViewById(R.id.keep_timer);
        mEspSetupTimer = (EditText) view.findViewById(R.id.esp_setup_time);
        mPdnSetupTimer = (EditText) view.findViewById(R.id.pdn_setup_time);
        mDnsTimer = (EditText) view.findViewById(R.id.dns_timer);
        mOos = (EditText) view.findViewById(R.id.oos);
        mDetachSoftTimer = (EditText) view.findViewById(R.id.detach_soft_timer);
        mDetachHardTimer = (EditText) view.findViewById(R.id.detach_hard_timer);
        mEditText = new EditText[]{mIkeRekeyTimer, mEspRekeyTimer, mRekeyMargin,
                mDpdTimer, mKeepTimer, mEspSetupTimer, mPdnSetupTimer, mDnsTimer, mOos,
                mDetachSoftTimer, mDetachHardTimer};

        mEditStatus = new String[mCfg.length];

        mSet = (Button) view.findViewById(R.id.timer_set);
        mSet.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mInvalidPara = checkDataValid();
                Elog.d(TAG, "mInvalidPara = " + mInvalidPara);
                if (mInvalidPara >= 0) {
                    showDialog(UI_DATA_INVALID).show();
                    return;
                }
                mSetFailPara = "";
                for (int i = 0; i < mCfg.length; i++) {
                    String set_value = mEditText[i].getText().toString();
                    if (set_value.equals(mEditStatus[i])) {
                        Elog.d(TAG, mCfg[i] + " had not bean changed");
                        continue;
                    } else {
                        mChange = true;
                        if (epdgConfig.setCfgValue(mCfg[i], set_value)) {
                            Elog.d(TAG, mCfg[i] + " set succeed");
                            mEditStatus[i] = set_value;
                        } else {
                            mSetFailPara += mCfg[i];
                            Elog.d(TAG, mCfg[i] + " set failed");
                        }
                    }
                }
                if (mChange && mSetFailPara.equals("")) {

                    if (FeatureSupport.is93Modem()) {
                        epdgConfig.setCfgValue("", "");
                    }
                    Toast.makeText(getActivity(),
                            "Set successfully!",
                            Toast.LENGTH_SHORT).show();
                } else if (!mChange) {
                    Toast.makeText(getActivity(),
                            "No item changes!",
                            Toast.LENGTH_SHORT).show();
                } else {
                    showDialog(DATA_SET_FAIL).show();
                }
                mChange = false;
            }
        });

        return view;
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        if (isVisibleToUser) {
            Elog.d(TAG, "epdgConfigTimerFragment show");
            getCfgValue();
        }
    }

    private void getCfgValue() {
        Elog.d(TAG, "get the currect value of Timer: ");
        for (int i = 0; i < mCfg.length; i++) {
            String respValue = epdgConfig.getCfgValue(mCfg[i]);
            if (respValue != null) {
                mEditText[i].setText(respValue);
                mEditStatus[i] = respValue;
            } else {
                mEditText[i].setEnabled(false);
                continue;
            }
        }
    }

    private int checkDataValid() {
        int ret = -1;
        for (int i = 0; i < mEditText.length; i++) {

            if ((mEditText[i].getText().toString() != null)
                    && ("".equals(mEditText[i].getText().toString()))) {
                mEditText[i].setText(mEditStatus[i]);
                continue;
            }

            long data = 0;
            try {
                data = Long.valueOf(mEditText[i].getText().toString());
            } catch (NumberFormatException e) {
                data = -1;
            }
            Elog.d(TAG, "data = " + data);
            if (data < 0 || data > 4294967294L) {
                mErrMsg = "The range is 0 ~ 4294967294.";
                ret = i;
                break;
            } else {
                mEditText[i].setText(String.valueOf(data));
            }
        }
        Elog.d(TAG, "ret = " + ret);
        return ret;
    }

    protected Dialog showDialog(int id) {
        switch (id) {
            case UI_DATA_INVALID:
                return new AlertDialog.Builder(getActivity()).setTitle(
                        "Check").setMessage(
                        "The input of " + mCfg[mInvalidPara] + " is invalid! " + mErrMsg)
                        .setPositiveButton("OK", null).create();
            case DATA_SET_FAIL:
                return new AlertDialog.Builder(getActivity()).setTitle(
                        "Set fail").setMessage("Set " + mSetFailPara + " fail!")
                        .setPositiveButton("OK", null).create();
        }
        return null;
    }

}
