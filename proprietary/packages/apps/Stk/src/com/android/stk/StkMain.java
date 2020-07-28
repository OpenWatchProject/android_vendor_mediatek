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

package com.android.stk;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

import android.view.View;

import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.PhoneConstants;

import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import com.android.internal.telephony.SubscriptionController;

import android.view.Gravity;
import android.widget.Toast;

import com.android.internal.telephony.ITelephony;
import com.mediatek.internal.telephony.cat.MtkCatLog;

/**
 * Launcher class. Serve as the app's MAIN activity, send an intent to the
 * StkAppService and finish.
 *
 */
 public class StkMain extends Activity {
    private static final String className = new Object(){}.getClass().getEnclosingClass().getName();
    private static final String LOG_TAG = className.substring(className.lastIndexOf('.') + 1);
    private int mSingleSimId = -1;
    private Context mContext = null;
    private TelephonyManager mTm = null;
    private static final String PACKAGE_NAME = "com.android.stk";
    private static final String STK_LAUNCHER_ACTIVITY_NAME = PACKAGE_NAME + ".StkLauncherActivity";

    static final boolean isOP154
            = "OP154".equalsIgnoreCase(SystemProperties.get("persist.vendor.operator.optr", ""));

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        MtkCatLog.d(LOG_TAG, "onCreate+");
        mContext = getBaseContext();
        mTm = (TelephonyManager) mContext.getSystemService(
                Context.TELEPHONY_SERVICE);
        //Check if needs to show the meun list.
        if (isShowSTKListMenu()) {
            Intent newIntent = new Intent(Intent.ACTION_VIEW);
            newIntent.setClassName(PACKAGE_NAME, STK_LAUNCHER_ACTIVITY_NAME);
            startActivity(newIntent);
        } else {
            //launch stk menu activity for the SIM.
            if (mSingleSimId < 0) {
                showTextToast(mContext, R.string.no_sim_card_inserted);
            } else {
                launchSTKMainMenu(mSingleSimId);
            }
        }
        finish();
    }

    private boolean isShowSTKListMenu() {
        int simCount = TelephonyManager.from(mContext).getSimCount();
        int simInsertedCount = 0;
        int insertedSlotId = -1;

        MtkCatLog.d(LOG_TAG, "simCount: " + simCount);
        for (int i = 0; i < simCount; i++) {
            //Check if the card is inserted.
            if (mTm.hasIccCard(i)) {
                MtkCatLog.d(LOG_TAG, "SIM " + i + " is inserted.");
                mSingleSimId = i;
                simInsertedCount++;
            } else {
                MtkCatLog.d(LOG_TAG, "SIM " + i + " is not inserted.");
            }
        }
        if (simInsertedCount > 1) {
            return true;
        } else {
            //No card or only one card.
            MtkCatLog.d(LOG_TAG, "do not show stk list menu.");
            return false;
        }
    }

    private void launchSTKMainMenu(int slotId) {
        Bundle args = new Bundle();
        MtkCatLog.d(LOG_TAG, "launchSTKMainMenu.");

        if (!isStkAvailable(slotId)) {
            return;
        }

        args.putInt(StkAppService.OPCODE, StkAppService.OP_LAUNCH_APP);
        args.putInt(StkAppService.SLOT_ID
                , PhoneConstants.SIM_ID_1 + slotId);
        startService(new Intent(this, StkAppService.class)
                .putExtras(args));
    }

    private void showTextToast(Context context, int resId) {
        Toast toast = Toast.makeText(context, resId, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }

    /// M:  @{
    private boolean isStkAvailable(int slotId) {
        int resId = R.string.lable_sim_not_ready;
        boolean isWfcEnabled = isWifiCallingAvailable(slotId);

        if (true == isOnFlightMode()) {
            MtkCatLog.d(LOG_TAG, "isOnFlightMode");
            if( isOP154 && isWfcEnabled ){
                MtkCatLog.d(LOG_TAG, "OP154, wfc enabled");
                return true;
            }else{
                resId = R.string.lable_on_flight_mode;
                showTextToast(mContext, resId);
                return false;
            }
        } else if (true == isOnLockMode(slotId) ||
                false == checkSimRadioState(slotId)) {
            MtkCatLog.d(LOG_TAG, "isOnLockMode or radio off or sim off");
            showTextToast(mContext, resId);
            return false;
        }

        StkAppService service = StkAppService.getInstance();

        if (service != null && service.StkQueryAvailable(slotId) !=
                StkAppService.STK_AVAIL_AVAILABLE) {
            int simState = TelephonyManager.getDefault().getSimState(slotId);

            MtkCatLog.d(LOG_TAG, "slotId: " + slotId + "is not available simState:" + simState);
            showTextToast(mContext, resId);
            return false;
        }
        return true;
    }

    private boolean isOnFlightMode() {
        int mode = 0;
        try {
            mode = Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON);
        } catch (SettingNotFoundException e) {
            MtkCatLog.d(LOG_TAG, "fail to get airlane mode");
            mode = 0;
        }

        MtkCatLog.d(LOG_TAG, "airlane mode is " + mode);
        return (mode != 0);
    }

    boolean isOnLockMode(int slotId) {
        int simState = TelephonyManager.getDefault().getSimState(slotId);
        MtkCatLog.d(LOG_TAG, "lock mode is " + simState);
        if (TelephonyManager.SIM_STATE_PIN_REQUIRED == simState ||
                TelephonyManager.SIM_STATE_PUK_REQUIRED == simState ||
                TelephonyManager.SIM_STATE_NETWORK_LOCKED == simState) {
            return true;
        } else {
            return false;
        }
    }

    private boolean isRadioOnState(int slotId) {
        boolean radioOn = true;
        MtkCatLog.d(LOG_TAG, "isRadioOnState check = " + slotId);

        try {
            ITelephony phone = ITelephony.Stub.asInterface(
                    ServiceManager.getService(Context.TELEPHONY_SERVICE));
            if (phone != null) {
                int subId[] = SubscriptionManager.getSubId(slotId);
                radioOn = phone.isRadioOnForSubscriber(subId[0],
                    getApplicationContext().getOpPackageName());
            }
            MtkCatLog.d(LOG_TAG, "isRadioOnState - radio_on[" + radioOn + "]");
        } catch (RemoteException e) {
            e.printStackTrace();
            MtkCatLog.d(LOG_TAG, "isRadioOnState - Exception happen ====");
        }
        return radioOn;
    }

    private boolean isWifiCallingAvailable(int phoneId) {
        final MtkTelephonyManagerEx tm = MtkTelephonyManagerEx.getDefault();
        int subId = getSubIdUsingPhoneId(phoneId);
        return tm.isWifiCallingEnabled(subId);
    }

    private int getSubIdUsingPhoneId(int phoneId) {
        SubscriptionController subCon = SubscriptionController.getInstance();
        int subId = (subCon != null)?
                subCon.getSubIdUsingPhoneId(phoneId) : SubscriptionManager.INVALID_SUBSCRIPTION_ID;

        MtkCatLog.d(LOG_TAG, "[getSubIdUsingPhoneId] subId " + subId + ", phoneId " + phoneId);
        return subId;
    }

    private boolean isSimOnState(int slotId) {
        boolean simOn = (MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId)
                == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON );
        MtkCatLog.d(LOG_TAG, "isSimOnState - slotId[" + slotId + "], sim_on[" + simOn + "]");

        return simOn;
    }

    private boolean checkSimRadioState(int slotId) {
        boolean isSimOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        MtkCatLog.d(LOG_TAG, "checkSimRadioState - slotId[" + slotId +
                "], isSimOnOffEnabled[" + isSimOnOffEnabled + "]");
        if (isSimOnOffEnabled) {
            return isSimOnState(slotId);
        } else {
            return isRadioOnState(slotId);
        }
    }
    /// @}
}
