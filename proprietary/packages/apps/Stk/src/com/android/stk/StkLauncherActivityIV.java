/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.view.Gravity;
import android.widget.Toast;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SubscriptionController;

import com.mediatek.internal.telephony.cat.MtkCatLog;
import com.mediatek.telephony.MtkTelephonyManagerEx;


/**
 * Launcher class. Serve as the app's MAIN activity, send an intent to the
 * StkAppService and finish.
 *
 */
public class StkLauncherActivityIV extends Activity {
    static final boolean isOP154
            = "OP154".equalsIgnoreCase(SystemProperties.get("persist.vendor.operator.optr", ""));

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (!isStkAvailable(PhoneConstants.SIM_ID_4)) {
            finish();
            return;
        }

        Bundle args = new Bundle();
        args.putInt(StkAppService.OPCODE, StkAppService.OP_LAUNCH_APP);
        args.putInt(StkAppService.SLOT_ID, PhoneConstants.SIM_ID_4);
        startService(new Intent(this, StkAppService.class).putExtras(args));

        finish();
    }

    private boolean isStkAvailable(int slotId) {
        int resId = R.string.lable_sim_not_ready;
        boolean isWfcEnabled = isWifiCallingAvailable(slotId);

        if (true == isOnFlightMode()) {
            MtkCatLog.d("Stk-LAIV", "isOnFlightMode");
            if( isOP154 && isWfcEnabled ){
                MtkCatLog.d("Stk-LAIV", "OP154, wfc enabled");
                return true;
            }else{
                resId = R.string.lable_on_flight_mode;
                showTextToast(getApplicationContext(), resId);
                return false;
            }
        } else if (true == isOnLockMode(slotId) ||
                false == checkSimRadioState(slotId)) {
            MtkCatLog.d("Stk-LAIV", "isOnLockMode or radio off or sim off");
            showTextToast(getApplicationContext(), resId);
            return false;
        }

        StkAppService service = StkAppService.getInstance();

        if (service != null && service.StkQueryAvailable(slotId) !=
                StkAppService.STK_AVAIL_AVAILABLE) {
            int simState = TelephonyManager.getDefault().getSimState(slotId);

            MtkCatLog.d("Stk-LAIV", "slotId: " + slotId + "is not available simState:" + simState);
            showTextToast(getApplicationContext(), resId);
            return false;
        }
        return true;
    }

    private void showTextToast(Context context, int resId) {
        Toast toast = Toast.makeText(context, resId, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
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

        MtkCatLog.d("Stk-LAIV", "[getSubIdUsingPhoneId] subId " + subId + ", phoneId " + phoneId);
        return subId;
    }

    boolean isOnFlightMode() {
        int mode = 0;
        try {
            mode = Settings.Global.getInt(getApplicationContext().getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON);
        } catch (SettingNotFoundException e) {
            MtkCatLog.w("Stk-LAIV", "fail to get airlane mode");
            mode = 0;
        }

        MtkCatLog.v("Stk-LAIV", "airlane mode is " + mode);
        return (mode != 0);
    }
    boolean isOnLockMode(int slotId) {
        int simState = TelephonyManager.getDefault().getSimState(slotId);
        MtkCatLog.v("Stk-LAIV", "lock mode is " + simState);
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
        try {
            ITelephony phone = ITelephony.Stub.asInterface(
                    ServiceManager.getService(Context.TELEPHONY_SERVICE));
            if (phone != null) {
                int subId[] = SubscriptionManager.getSubId(slotId);
                radioOn = phone.isRadioOnForSubscriber(subId[0],
                    getApplicationContext().getOpPackageName());
            }
            MtkCatLog.v("Stk-LAIV", "slotId: " + slotId
                     + "isRadioOnState - radio_on[" + radioOn + "]");
        } catch (RemoteException e) {
            e.printStackTrace();
            MtkCatLog.w("Stk-LAIV", "isRadioOnState - Exception happen ====");
        }
        return radioOn;
    }

    private boolean isSimOnState(int slotId) {
        boolean simOn = (MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId)
                == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON );
        MtkCatLog.d("Stk-LAIV", "isSimOnState - slotId[" + slotId + "], sim_on[" + simOn + "]");

        return simOn;
    }

    private boolean checkSimRadioState(int slotId) {
        boolean isSimOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        MtkCatLog.d("Stk-LAIV", "checkSimRadioState - slotId[" + slotId +
                "], isSimOnOffEnabled[" + isSimOnOffEnabled + "]");
        if (isSimOnOffEnabled) {
            return isSimOnState(slotId);
        } else {
            return isRadioOnState(slotId);
        }
    }
}
