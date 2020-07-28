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

package com.mediatek.engineermode.sbp;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

/**
 * Display SBP info.
 */
public class SbpActivity extends Activity {
    private static final String TAG = "SbpActivity";
    private static final int TEXT_COUNT = 4;
    private static final int MSG_SBP_INFO = 1;
    private static final String QUERY_CMD = "AT+ESBP?";
    private static final String RESPONSE_CMD = "+ESBP: ";

    private TextView[] mTextViews = new TextView[TEXT_COUNT];
    private TextView mTextViewsSimSbpId = null;

    private Handler mATCmdHander = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
            case MSG_SBP_INFO:
                ar = (AsyncResult) msg.obj;
                if (ar != null && ar.exception == null) {
                    handleQuery((String[]) ar.result);
                } else {
                    Toast.makeText(SbpActivity.this, "Send AT command failed", Toast.LENGTH_SHORT);
                }
                break;
            default:
                break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sbp);

        if (FeatureSupport.is93Modem()) {
            mTextViews[0] = (TextView) findViewById(R.id.sbp_id);
            mTextViews[1] = (TextView) findViewById(R.id.sbp_sim_id);
            mTextViews[2] = (TextView) findViewById(R.id.sbp_feature);
            mTextViews[3] = (TextView) findViewById(R.id.sbp_data);
        }
        else{
            mTextViews[0] = (TextView) findViewById(R.id.sbp_id);
            mTextViews[1] = (TextView) findViewById(R.id.sbp_feature);
            mTextViews[2] = (TextView) findViewById(R.id.sbp_data);
            mTextViews[3] = (TextView) findViewById(R.id.sbp_sim_id);
            mTextViewsSimSbpId = (TextView) findViewById(R.id.sbp_sim_id_view);
            mTextViews[3].setVisibility(View.GONE);
            mTextViewsSimSbpId.setVisibility(View.GONE);
        }

    }

    @Override
    protected void onResume() {
        super.onResume();
        String[] cmd = new String[2];
        cmd[0] = QUERY_CMD;
        cmd[1] = RESPONSE_CMD;
        EmUtils.invokeOemRilRequestStringsEm(cmd, mATCmdHander.obtainMessage(MSG_SBP_INFO));
    }

    private void handleQuery(String[] result) {
        if (result != null && result.length > 0) {
            Elog.v(TAG, "Modem return: " + result[0]);
            String[] values = result[0]
                    .substring(RESPONSE_CMD.length(), result[0].length()).trim().split(",");
            if (values != null && values.length > 0) {
                for (int i = 0; i < values.length; i++) {
                    mTextViews[i].setText(values[i]);
                }
            }
        } else {
            Elog.e(TAG, "Modem return null");
        }
    }
}
