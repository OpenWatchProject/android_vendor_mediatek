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

package com.mediatek.engineermode;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class AutoAnswer extends Activity implements OnClickListener {

    private final String TAG = "AutoAnswer";
    private final String AUTO_ANSWER_PROPERTY = "persist.vendor.em.hidl.auto_answer";
    private Button mEnableButton;
    private Button mDisableButton;
    private String mStatus = "0";
    private int mResult = -1;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.auto_answer);
        mEnableButton = (Button) findViewById(R.id.autoanswer_enable);
        mDisableButton = (Button) findViewById(R.id.autoanswer_disable);
        mEnableButton.setOnClickListener(this);
        mDisableButton.setOnClickListener(this);
        queryAutoAnswerStatus();
        updateButtonStatus();
    }

    @Override
    public void onClick(final View arg0) {
        if (arg0.getId() == R.id.autoanswer_enable) {
            setAutoAnswer(1);
        } else if (arg0.getId() == R.id.autoanswer_disable) {
            setAutoAnswer(0);
        }
        queryAutoAnswerStatus();
        updateButtonStatus();
    }

    private void queryAutoAnswerStatus() {
        Elog.v(TAG, "queryAutoAnswerStatus");
        mStatus = EmUtils.systemPropertyGet(AUTO_ANSWER_PROPERTY, "0");
        Elog.v(TAG, "The autoanswer flag is : " + mStatus);
    }

    private void setAutoAnswer(int mode) {
        Elog.i(TAG, "setAutoAnswer to " + mode);
        try {
            EmUtils.getEmHidlService().setEmConfigure(AUTO_ANSWER_PROPERTY,String.valueOf(mode));
        }
        catch (Exception e) {
            e.printStackTrace();
            Elog.e(TAG, "set property failed ...");
        }
    }

    private void updateButtonStatus() {
        if (mStatus.equals("0")) {
            mEnableButton.setEnabled(true);
            mDisableButton.setEnabled(false);
        } else {
            mEnableButton.setEnabled(false);
            mDisableButton.setEnabled(true);
        }
    }
}
