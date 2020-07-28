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

package com.mediatek.ppl.ui;

import android.content.Intent;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.text.Html;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.mediatek.ppl.PlatformManager;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;
import com.mediatek.ppl.SimTracker;
import com.mediatek.ppl.ui.DialogChooseSimFragment.IChooseSim;

public class SetupManualActivity extends PplBasicActivity implements IChooseSim {

    private final static String TAG = "PPL/SetupManualActivity";

    private LinearLayout mLayoutUp;
    private LinearLayout mLayoutDown;
    private ProgressBar mProgressBar;
    private CheckBox    mCheckBox;
    private Button      mBtnNext;
    private TextView    mContent;

    private SimTracker mSimTracker;

    @Override
    protected void onCreate(Bundle saveInstanceState) {
        super.onCreate(saveInstanceState);

        TelephonyManager telephonyManager = new TelephonyManager(this);
        int sim_number = telephonyManager.getSimCount();
        mSimTracker = new SimTracker(sim_number, this);
    }

    @Override
    protected void onResume() {
        mCheckBox.setText(R.string.checkbox_send_sms);
        super.onResume();
    }

    @Override
    protected void onRegisterEvent() {
        mEventReceiver.addAction(PplService.Intents.UI_QUIT_SETUP_WIZARD);
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);
    }

    @Override
    protected void onPrepareLayout() {

        setContentView(R.layout.setup_manual);

        mLayoutUp = (LinearLayout) findViewById(R.id.layout_setup_manual_up);
        mLayoutDown = (LinearLayout) findViewById(R.id.layout_setup_manual_down);
        mProgressBar = (ProgressBar) findViewById(R.id.common_progress);
        mCheckBox = (CheckBox) findViewById(R.id.cb_send_manual_sendSms);
        mBtnNext = (Button) findViewById(R.id.btn_bottom_next);
        mContent = (TextView) findViewById(R.id.tv_setup_manual_content);
        mContent.setText(Html.fromHtml(getString(R.string.content_setup_send_manual)));


        mCheckBox.setOnCheckedChangeListener(new OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    mBtnNext.setText(R.string.button_send_sms);
                } else {
                    mBtnNext.setText(R.string.button_finish);
                }
            }

        });

        mBtnNext.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                onButtonClicked();
            }
        });

    }

    @Override
    protected void onInitLayout() {

        mProgressBar.setVisibility(View.VISIBLE);
        mLayoutUp.setVisibility(View.GONE);
        mLayoutDown.setVisibility(View.GONE);
        mCheckBox.setChecked(false);
        mBtnNext.setText(R.string.button_finish);
    }

    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        mProgressBar.setVisibility(View.GONE);
        mLayoutUp.setVisibility(View.VISIBLE);
        mLayoutDown.setVisibility(View.VISIBLE);
    }

    protected void onButtonClicked() {

        mSimTracker.takeSnapshot();
        if (mSimTracker.getInsertedSim().length == 0) {
            Log.e(TAG, "onButtonClicked() but no sim found !");
            Toast.makeText(this, R.string.toast_no_sim, Toast.LENGTH_SHORT).show();
            return;
        } else if (!mSimTracker.isAllSimReady()) {
            Log.e(TAG, "onButtonClicked() but not all sim ready !");
            Toast.makeText(this, R.string.toast_sim_not_ready, Toast.LENGTH_SHORT).show();
            return;
        }

        if (mCheckBox.isChecked()) {
            onConfirmWithMessage();
        } else {
            onConfirm();
        }
    }

    protected void onConfirm() {

        mBinder.setProvision(true);
        mBinder.enable(true);

        gotoActivity(this, ControlPanelActivity.class);

        Intent intent = new Intent(PplService.Intents.UI_QUIT_SETUP_WIZARD);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    protected void onConfirmWithMessage() {
        if (PlatformManager.isAirplaneModeEnabled(this)) {
            DialogTurnoffAirplaneFragment frg = DialogTurnoffAirplaneFragment.newInstance();
            frg.show(getFragmentManager(), "turnOff_airplane");
        } else {
            int[] insertedSimSlots = mBinder.getInsertedSim();
            if (1 < insertedSimSlots.length) {
                String[] itemList = new String[insertedSimSlots.length];
                String itemTemplate = getResources().getString(R.string.item_sim_n);
                for (int i = 0; i < insertedSimSlots.length; ++i) {
                    itemList[i] = itemTemplate + i;
                }
                DialogChooseSimFragment frg = DialogChooseSimFragment.
                        newInstance(itemList, insertedSimSlots);
                frg.show(getFragmentManager(), "choose_sim");
            } else if (1 == insertedSimSlots.length) {
                onSimSelected(insertedSimSlots[0]);
            } else {
                Toast.makeText(this, R.string.toast_no_sim, Toast.LENGTH_SHORT).show();
            }
        }
    }

    @Override
    public void onSimSelected(int simId) {
        onConfirm();
        mBinder.sendInstructionDescriptionMessage(simId);
    }
}
