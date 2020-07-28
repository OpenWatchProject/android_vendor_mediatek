/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation
 * ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.audio;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ShellExe;

import java.io.IOException;

public class AudioDebugSession extends Activity implements OnClickListener, OnCheckedChangeListener {
    private static final String TAG = "Audio/DebugSession";
    private static final String CUST_XML_PARAM = "GET_CUST_XML_ENABLE";
    private static final String CUST_XML_SET_SUPPORT_PARAM = "SET_CUST_XML_ENABLE=1";
    private static final String CUST_XML_SET_UNSUPPORT_PARAM = "SET_CUST_XML_ENABLE=0";
    private static final String RESULT_SUPPORT = "GET_CUST_XML_ENABLE=1";
    private static final String RESULT_UNSUPPORT = "GET_CUST_XML_ENABLE=0";

    private static final String MTK_AUDIO_HYBRID_NLE_SUPPORT = "MTK_AUDIO_HYBRID_NLE_SUPPORT";
    private static final String HEAD_DETECT_PATH = "/sys/bus/platform/drivers/Accdet_Driver/state";

    private static final String AUDIO_HYBRID_NLE = "AudioEnableHybridNLE";
    private static final String AUDIO_ENABLE_HYBRID_NLE = "AudioEnableHybridNLE=1";
    private static final String AUDIO_DISENABLE_HYBRID_NLE = "AudioEnableHybridNLE=0";
    private static final String AUDIO_HYBRID_NLE_EOP = "AudioHybridNLEEOP";

    private static final int SET_MAGIASR_TEST_ENABLE = 0xA2;
    private static final int GET_MAGIASR_TEST_ENABLE = 0xA3;
    private static final int SET_AECREC_TEST_ENABLE = 0xA4;
    private static final int GET_AECREC_TEST_ENABLE = 0xA5;

    private Button mBtnSetNLE;
    private EditText mEtNLE;
    private CheckBox mCbNLE;
    private LinearLayout mLinearLayoutNLE ;

    private Button mBtnDetect;
    private TextView mTvDetect;
    private CheckBox mCbMagi;
    private CheckBox mCbAecRec;
    private CheckBox mCbCustParam;
    private Toast mToast = null;
    private AudioManager mAudioMgr = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_debugsession);
        mAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mBtnDetect = (Button) findViewById(R.id.Audio_Headset_Detect_Button);
        mBtnDetect.setOnClickListener(this);
        mTvDetect = (TextView) findViewById(R.id.Audio_Headset_Detect_Text);
        mCbMagi = (CheckBox) findViewById(R.id.Audio_MagiAsr);
        mCbAecRec = (CheckBox) findViewById(R.id.Audio_AecRec);
        mCbCustParam = (CheckBox) findViewById(R.id.Audio_Custom_Param);
        mBtnSetNLE = (Button) findViewById(R.id.Audio_Hybrid_Dynamic_set);
        mEtNLE = (EditText) findViewById(R.id.Audio_Hybrid_Dynamic_edit);
        mCbNLE = (CheckBox) findViewById(R.id.Audio_Hybrid_Dynamic);
        mLinearLayoutNLE = (LinearLayout) findViewById(R.id.Audio_Hybrid_Dynamic_LinearLayout);

        String audioHybridNleSupport
                       = AudioSystem.getParameters(MTK_AUDIO_HYBRID_NLE_SUPPORT);
        Elog.d(TAG, audioHybridNleSupport);
        mLinearLayoutNLE.setVisibility(View.GONE);
        if(audioHybridNleSupport.equals("MTK_AUDIO_HYBRID_NLE_SUPPORT=1")){
            mLinearLayoutNLE.setVisibility(View.VISIBLE);
            String ret = AudioSystem.getParameters(AUDIO_HYBRID_NLE);
            Elog.d(TAG, "getParameters(AUDIO_HYBRID_NLE) ret = " + ret);
            mCbNLE.setChecked("AudioEnableHybridNLE=1".equals(ret));
            mCbNLE.setOnCheckedChangeListener(this);
            String rets = AudioSystem.getParameters(AUDIO_HYBRID_NLE_EOP);
            Elog.d(TAG, "getParameters(AUDIO_HYBRID_NLE_EOP) rets = " + rets);
            String [] values = rets.split("=");
            if( values.length == 2 ) {
               mEtNLE.setText(values[1]);
            }
            mBtnSetNLE.setOnClickListener(this);
        }

        TextView tvCustParam =  (TextView) findViewById(R.id.Audio_Custom_Param_Title);
        View divider = findViewById(R.id.Audio_Custom_Param_Divider);
        if (Audio.AudioTuningVer.VER_2_2 != Audio.getAudioTuningVer()) {
            mCbCustParam.setVisibility(View.GONE);
            tvCustParam.setVisibility(View.GONE);
            divider.setVisibility(View.GONE);
        } else {
        if (FeatureSupport.isEngLoad()) {
            mCbCustParam.setChecked(true);
            mCbCustParam.setEnabled(false);
        } else {
            String check = mAudioMgr.getParameters(CUST_XML_PARAM);
            if ((check != null) && (RESULT_SUPPORT.equals(check))) {
                mCbCustParam.setChecked(true);
            } else {
                mCbCustParam.setChecked(false);
            }
            mCbCustParam.setOnCheckedChangeListener(this);
        }
}
        int ret = AudioTuningJni.getAudioCommand(GET_MAGIASR_TEST_ENABLE);
        Elog.d(TAG, "getAudioCommand(0xA3) ret " + ret);
        if (ret == 0) {
            findViewById(R.id.Audio_MagiAsr_Group).setVisibility(View.GONE);
        } else {
            mCbMagi.setChecked(ret == 1);
            mCbMagi.setOnCheckedChangeListener(this);
        }

        ret = AudioTuningJni.getAudioCommand(GET_AECREC_TEST_ENABLE);
        Elog.d(TAG, "getAudioCommand(0xA5) ret " + ret);
        mCbAecRec.setChecked(ret == 1);
        mCbAecRec.setOnCheckedChangeListener(this);
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (buttonView.equals(mCbMagi)) {
            int ret = AudioTuningJni.setAudioCommand(SET_MAGIASR_TEST_ENABLE, isChecked ? 1 : 0);
            Elog.d(TAG, "setAudioCommand(0xA2, " + (isChecked ? 1 : 0) + ") ret " + ret);
            if (ret == -1) {
                showToast("set audio parameter 0xA2 failed.");
            }
        } else if (buttonView.equals(mCbAecRec)) {
             int ret = AudioTuningJni.setAudioCommand(SET_AECREC_TEST_ENABLE, isChecked ? 1 : 0);
            Elog.d(TAG, "setAudioCommand(0xA4, " + (isChecked ? 1 : 0) + ") ret " + ret);
            if (ret == -1) {
                showToast("set audio parameter 0xA4 failed.");
            }
        } else if (buttonView.equals(mCbCustParam)) {
           mAudioMgr.setParameters(mCbCustParam.isChecked() ?
              CUST_XML_SET_SUPPORT_PARAM : CUST_XML_SET_UNSUPPORT_PARAM);
            AudioTuningJni.CustXmlEnableChanged(mCbCustParam.isChecked() ? 1 : 0 );
        }
        else if (buttonView.equals(mCbNLE)) {
            mAudioMgr.setParameters(mCbNLE.isChecked() ?
                    AUDIO_ENABLE_HYBRID_NLE : AUDIO_DISENABLE_HYBRID_NLE);
            Elog.d(TAG, "mCbNLE changed " + (mCbNLE.isChecked() ?
                    AUDIO_ENABLE_HYBRID_NLE : AUDIO_DISENABLE_HYBRID_NLE));
        }
    }

    @Override
    public void onClick(View arg0) {
        if (arg0.getId() == mBtnDetect.getId()) {
            int ret = ShellExe.RESULT_FAIL;
            String cmd = "cat " + HEAD_DETECT_PATH;
            try {
                ret = ShellExe.execCommand(cmd, true);
                if (ShellExe.RESULT_SUCCESS == ret) {
                    int mode = Integer.parseInt(ShellExe.getOutput());
                    Elog.d(TAG, HEAD_DETECT_PATH + ": " + ShellExe.getOutput());
                    if (mode == 1) {
                        mTvDetect.setText(getString(R.string.Audio_Headset_Jak_Headset));
                    } else if (mode == 2) {
                        mTvDetect.setText(getString(R.string.Audio_Headset_Jak_Headphone));
                    } else {
                        mTvDetect.setText(getString(R.string.Audio_Headset_None));
                    }
                } else {
                    showToast("Detection failed");
                }
            } catch (IOException e) {
                Elog.d(TAG, cmd.toString() + e.getMessage());
                showToast("Detection failed");
            } catch (NumberFormatException e) {
                showToast("Detection failed");
            }
        }
        else if (arg0.getId() == mBtnSetNLE.getId()) {
            if(mEtNLE == null || mEtNLE.getText() == null) {
                showToast("Please input an num 0 ~~ -96");
                return;
            }
            String input = mEtNLE.getText().toString();
            Elog.d(TAG, "input AUDIO_HYBRID_NLE_EOP = " + input);

            if( input.isEmpty() || Long.valueOf(input) > 0  || Long.valueOf(input) < -96 ) {
                showToast("Please input an num 0 ~~ -96");
                return;
            }
            String sets = AUDIO_HYBRID_NLE_EOP + "=" + input;
            AudioSystem.setParameters(sets);
            Elog.d(TAG, "set AUDIO_HYBRID_NLE_EOP = " + sets);

            String ret = AudioSystem.getParameters(AUDIO_HYBRID_NLE_EOP);
            Elog.d(TAG, "get AUDIO_HYBRID_NLE_EOP = " + ret);
            if(ret.equals(sets)){
                showToast("The value set succeeful");
            }
            else
            {
                showToast("The value set failed");
            }
        }
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}

