/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.op18.systemui;

import android.content.Context;
import android.graphics.Bitmap;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.MeasureSpec;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.op18.systemui.R;


/** View for quick Sim selection view.
**/
public class SimSelectionView  implements View.OnClickListener {
    private static final String TAG = "OP18SimSelectionView";

    private static Context sContext;

    private static SimSelectionView sSimSelectionView;
    private View mView;
    private ImageView mSimZeroImage;
    private ImageView mSimOneImage;
    private TextView mSimZeroName;
    private TextView mSimOneName;
    private TextView mSimZeroNumber;
    private TextView mSimOneNumber;
    private static ViewGroup sViewGroup;
    private SimSelectionController mSimSelectionController = null;

    private TelephonyManager mTelephonyManager;
    private SubscriptionManager mSubscriptionManager;
    private TelecomManager mTelecomManager;

    /**
     * Constructor.
     * @param host host
     */
    private SimSelectionView(Context pluginContext, ViewGroup vg) {
        mView = LayoutInflater.from(pluginContext).inflate(
                R.layout.quick_settings_sim_selection_dialog, vg, false);
        vg.addView(mView);
        sViewGroup = vg;
        mSubscriptionManager = SubscriptionManager.from(pluginContext);
        mTelephonyManager = TelephonyManager.from(pluginContext);
        mTelecomManager = TelecomManager.from(pluginContext);

        mSimZeroImage = (ImageView) mView.findViewById(R.id.sim_zero_img);
        mSimOneImage = (ImageView) mView.findViewById(R.id.sim_one_img);

        mSimZeroName = (TextView) mView.findViewById(R.id.sim_zero_name);
        mSimOneName = (TextView) mView.findViewById(R.id.sim_one_name);

        mSimZeroNumber = (TextView) mView.findViewById(R.id.sim_zero_number);
        mSimOneNumber = (TextView) mView.findViewById(R.id.sim_one_number);

        FrameLayout sim1 = (FrameLayout) mView.findViewById(R.id.sim_zero_container);
        sim1.setOnClickListener(this);
        FrameLayout sim2 = (FrameLayout) mView.findViewById(R.id.sim_one_container);
        sim2.setOnClickListener(this);

        mSimSelectionController = new SimSelectionController(pluginContext);
    }

    /**
     * Provides SimSelectionView instance.
     * @param context A Context object
     * @param vg view group
     * @return SimSelectionView
     */
    public static SimSelectionView getInstance(Context context, ViewGroup vg) {
        if (sSimSelectionView == null) {
            sContext = context;
            sSimSelectionView = new SimSelectionView(context, vg);
        }
        return sSimSelectionView;
    }

    /**
     * Provides SimSelectionView's view group.
     * @return ViewGroup
     */
    public static ViewGroup getSimSelectionViewGroup() {
        return sViewGroup;
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.sim_zero_container) {
            setPhoneAccount(0);
            Log.d(TAG, "onClick: sim0 selected");
        } else if (view.getId() == R.id.sim_one_container) {
            setPhoneAccount(1);
            Log.d(TAG, "onClick: sim1 selected");
        }
        updateSimSelectionView();
    }

    /**
         * Adds SimSelectionView in view group.
         * @return
         */
    public void addView() {
        Log.d(TAG, "addView");
        //getInstance(pluginContext, vg);
        if (SimSelectionController.showSimSelectionDialog(sContext)) {
            updateSimSelectionView();
            Log.d(TAG, "Show view");
        } else {
            mView.setVisibility(View.GONE);
            Log.d(TAG, "Hide view");
        }
    }

    /**
     * Registers SimSelectionView callbacks.
     * @return
     */
    public void registerCallbacks() {
        Log.d(TAG, "registerCallbacks");
        mSimSelectionController.registerCallbacks();
    }

    /**
         * UnRegisters SimSelectionView callbacks.
         * @return
         */
    public void unregisterCallbacks() {
        Log.d(TAG, "unregisterCallbacks");
        mSimSelectionController.unregisterCallbacks();
    }

    /**
         * Sets SimSelectionView visibility as passed.
         * @param visibility visibility
         * @return
         */
    public void setVisibility(int visibility) {
        if (SimSelectionController.showSimSelectionDialog(sContext)) {
            mView.setVisibility(visibility);
        } else {
            mView.setVisibility(View.GONE);
        }
    }

    /**
         * Calls measure of SimSelectionView.
         * @param width wisth
         * @return
         */
    public void measure(int width) {
        if (SimSelectionController.showSimSelectionDialog(sContext)) {
            int w = MeasureSpec.makeMeasureSpec(width, MeasureSpec.EXACTLY);
            mView.measure(w, MeasureSpec.UNSPECIFIED);
        }
    }

    /**
         * Retuns previous view as SimSelectionView, if its visible, else default view passes.
         * @param v default view
         * @return View
         */
    public View getPreviousView(View v) {
        if (SimSelectionController.showSimSelectionDialog(sContext)) {
            //Log.d(TAG, "Previous View: " + mView);
            return mView;
        } else {
            return v;
        }
    }

    /**
         * Provides SimSelectionView height.
         * @return height
         */
    public int getViewHeight() {
        if (SimSelectionController.showSimSelectionDialog(sContext)) {
            return mView.getMeasuredHeight();
        } else {
            return 0;
        }
    }

    /**
         * Calls SimSelectionView layout method.
         * @param aboveViewHeight height of view above SimSelectionView
         * @return total height till SimSelectionView
         */
    public int setLayout(int aboveViewHeight) {
        if (SimSelectionController.showSimSelectionDialog(sContext)) {
            mView.layout(0, aboveViewHeight, mView.getMeasuredWidth(),
                    aboveViewHeight + mView.getMeasuredHeight());
            return aboveViewHeight + mView.getMeasuredHeight();
        } else {
            return 0;
        }
    }

    /**
         * Updates SimSelectionView.
         * @return
         */
    public void updateSimSelectionView() {
        Log.d(TAG, "updateSimSelectionView");
        if (SimSelectionController.showSimSelectionDialog(sContext)) {
            SubscriptionInfo sir0 = mSubscriptionManager
                    .getActiveSubscriptionInfoForSimSlotIndex(0);
            SubscriptionInfo sir1 = mSubscriptionManager
                    .getActiveSubscriptionInfoForSimSlotIndex(1);
            Log.d(TAG, "sir0:" + sir0);
            Log.d(TAG, "sir1" + sir1);
            if (sir0 != null && sir1 != null) {
                populateDialog(sir0, sir1);
                //sViewGroup.addView(mView);
                mView.setVisibility(View.VISIBLE);
                Log.d(TAG, "Show view");
            }
        } else {
            //sViewGroup.removeView(mView);
            mView.setVisibility(View.GONE);
            Log.d(TAG, "Hide view");
        }
    }

    private void populateDialog(SubscriptionInfo sir0, SubscriptionInfo sir1) {
        Bitmap simZeroBitmap = sir0.createIconBitmap(sContext);
        mSimZeroImage.setImageBitmap(simZeroBitmap);
        mSimZeroName.setText(sir0.getDisplayName());
        Log.d(TAG, "number0:" + mTelephonyManager
                .getLine1Number(sir0.getSubscriptionId()));
        mSimZeroNumber.setText(mTelephonyManager
                .getLine1Number(sir0.getSubscriptionId()));

        Bitmap simOneBitmap = sir1.createIconBitmap(sContext);
        mSimOneImage.setImageBitmap(simOneBitmap);
        mSimOneName.setText(sir1.getDisplayName());
        Log.d(TAG, "number1:" + mTelephonyManager
                .getLine1Number(sir1.getSubscriptionId()));
        mSimOneNumber.setText(mTelephonyManager
                .getLine1Number(sir1.getSubscriptionId()));
        if (getSelectedSim() == 0) {
            FrameLayout fl = ((FrameLayout) mView.findViewById(R.id.sim_zero_highlight_divider));
            fl.setVisibility(View.VISIBLE);
            ((TextView) fl.findViewById(R.id.highlight_divider_tv))
                    .setBackgroundColor(sir0.getIconTint());
            ((FrameLayout) mView.findViewById(R.id.sim_zero_normal_divider))
                    .setVisibility(View.GONE);

            ((FrameLayout) mView.findViewById(R.id.sim_one_highlight_divider))
                    .setVisibility(View.GONE);
            ((FrameLayout) mView.findViewById(R.id.sim_one_normal_divider))
                    .setVisibility(View.VISIBLE);
        } else {
            FrameLayout fl = ((FrameLayout) mView.findViewById(R.id.sim_one_highlight_divider));
            fl.setVisibility(View.VISIBLE);
            ((TextView) fl.findViewById(R.id.highlight_divider_tv))
                    .setBackgroundColor(sir1.getIconTint());
            ((FrameLayout) mView.findViewById(R.id.sim_one_normal_divider))
                    .setVisibility(View.GONE);

            ((FrameLayout) mView.findViewById(R.id.sim_zero_highlight_divider))
                    .setVisibility(View.GONE);
            ((FrameLayout) mView.findViewById(R.id.sim_zero_normal_divider))
                    .setVisibility(View.VISIBLE);
        }
    }

    private void setPhoneAccount(int simId) {
        SubscriptionInfo sir = mSubscriptionManager.getActiveSubscriptionInfoForSimSlotIndex(simId);
        mSimSelectionController.setPhoneAccount(sir.getSubscriptionId());
    }

    private int getSelectedSim() {
        PhoneAccountHandle accountHandle = mTelecomManager.getUserSelectedOutgoingPhoneAccount();
        PhoneAccount phoneAccount = mTelecomManager.getPhoneAccount(accountHandle);
        int slotId = SubscriptionManager.getSlotIndex(mTelephonyManager
                .getSubIdForPhoneAccount(phoneAccount));
        // If in any case no sim selected, set it as mainCapability sim
        if (slotId == -1) {
            int subId = mSimSelectionController.setMainCapabilityAccount();
            slotId = SubscriptionManager.getSlotIndex(subId);
        }
        Log.d(TAG, "selected sim:" + slotId);
        return slotId;
    }

}
