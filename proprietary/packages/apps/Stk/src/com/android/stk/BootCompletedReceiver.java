/*
 * Copyright (C) 2007 The Android Open Source Project
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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.provider.Settings;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.internal.telephony.cat.MtkCatLog;
import com.mediatek.internal.telephony.cat.MtkCatService;
import com.mediatek.provider.MtkSettingsExt;

/**
 * Boot completed receiver. used to reset the app install state every time the
 * device boots.
 *
 */
public class BootCompletedReceiver extends BroadcastReceiver {
    private static final String LOG_TAG = new Object(){}.getClass().getEnclosingClass().getName();


    /// M:  @{
    private static boolean mHasBootComplete = false;
    /// @}

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action == null) {
            return;
        }

        MtkCatLog.d(LOG_TAG, "[onReceive] action = " + action);

        // make sure the app icon is removed every time the device boots.
        if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            if (StkAppService.isOP02) {
                int phoneCount = TelephonyManager.getDefault().getPhoneCount();
                for (int slotIndex = 0 ; slotIndex < phoneCount; slotIndex++) {
                    if (!MtkCatService.getSaveNewSetUpMenuFlag(slotIndex)) {
                        StkAppInstaller.getInstance().unInstall(context, slotIndex);
                    }
                }
            }
            if (android.os.Process.myUserHandle().isSystem()) {
                Bundle args = new Bundle();
                args.putInt(StkAppService.OPCODE, StkAppService.OP_BOOT_COMPLETED);
                try {
                    context.startService(new Intent(context, StkAppService.class)
                            .putExtras(args));
                } catch (IllegalStateException e) {
                    e.printStackTrace();
                    MtkCatLog.d(LOG_TAG, "start StkAppService fail");
                }
            } else {
                MtkCatLog.d(LOG_TAG, "secondary users, not start StkAppService");
            }
            mHasBootComplete = true;
        } else if(action.equals(Intent.ACTION_USER_INITIALIZE)) {
            // TODO: http://b/25155491
            if (!android.os.Process.myUserHandle().isSystem()) {
                //Disable package for all secondary users. Package is only required for device
                //owner.
                context.getPackageManager().setApplicationEnabledSetting(context.getPackageName(),
                        PackageManager.COMPONENT_ENABLED_STATE_DISABLED, 0);
                return;
            }
        } else if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
            if (intent.getBooleanExtra(TelephonyIntents.EXTRA_REBROADCAST_ON_UNLOCK, false)) {
                MtkCatLog.d(LOG_TAG, "ignore rebroadcast since stk app is direct boot aware.");
                return;
            }
            int slotId = intent.getIntExtra(PhoneConstants.PHONE_KEY,-1);
            if (slotId < 0)
            {
                MtkCatLog.e(LOG_TAG, "Invalid slot id.");
                return;
            }
            String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);

            MtkCatLog.d(LOG_TAG, "ACTION_SIM_STATE_CHANGED, simStatus: " + simStatus);

            if ((IccCardConstants.INTENT_VALUE_ICC_READY).equals(simStatus) ||
                        (IccCardConstants.INTENT_VALUE_ICC_LOADED).equals(simStatus)) {

                if (!MtkCatService.getSaveNewSetUpMenuFlag(slotId)) {
                    if (mHasBootComplete) {
                        MtkCatLog.d(LOG_TAG, "Disable the STK of sim" + (slotId + 1) +
                                " because still not receive SET_UP_MENU after boot up");
                        /* Remove cached SET_UP_MENU */
                        if (android.os.Process.myUserHandle().isSystem()) {
                            Bundle args = new Bundle();
                            args.putInt(StkAppService.OPCODE,
                                    StkAppService.OP_REMOVE_CACHED_SETUP_MENU);
                            args.putInt(StkAppService.SLOT_ID, slotId);
                            context.startService(new Intent(context,
                                    StkAppService.class).putExtras(args));
                        } else {
                            MtkCatLog.d(LOG_TAG, "secondary users, not start StkAppService");
                        }
                    } else { //mHasBootComplete == false
                         if ((IccCardConstants.INTENT_VALUE_ICC_LOADED).equals(simStatus)) {
                            //load cached setupmenu because of
                            //maybe phone process is killed and restart
                            if (android.os.Process.myUserHandle().isSystem()) {
                                MtkCatLog.v(LOG_TAG, "Launch cached SET_UP_MENU.");
                                Bundle args = new Bundle();
                                args.putInt(StkAppService.OPCODE,
                                        StkAppService.OP_LAUNCH_CACHED_SETUP_MENU);
                                args.putInt(StkAppService.SLOT_ID, slotId);
                                context.startService(new Intent(
                                        context, StkAppService.class).putExtras(args));
                            } else {
                                MtkCatLog.d(LOG_TAG, "secondary users, not start StkAppService");
                            }
                         }
                    }
                }
            }
        }
    }
}
