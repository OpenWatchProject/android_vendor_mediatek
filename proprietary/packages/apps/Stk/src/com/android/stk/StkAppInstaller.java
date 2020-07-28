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

import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;

import android.content.ComponentName;
import android.content.Context;
import android.content.pm.PackageManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.os.SystemProperties;

import com.mediatek.internal.telephony.cat.MtkCatLog;

/**
 * Application installer for SIM Toolkit.
 *
 */

 //MTK add change abstract class to normal class
class StkAppInstaller {
    private static final String STK_MAIN_ACTIVITY = "com.android.stk.StkMain";
    private static final String LOG_TAG = "StkAppInstaller";


    /// M:  @{
    Context mContext;
    private static int sSimCount = 0;
    private static StkAppInstaller mInstance = new StkAppInstaller();
    private InstallThread[] installThread = null;
    private UnInstallThread[] uninstallThread = null;

    public static final int STK_NOT_INSTALLED = 1;
    public static final int STK_INSTALLED = 2;
    public static final int STK_LAUNCH_ID = 0xFF;

    private static int[] miSTKInstalled = null;  // 1 -not_ready, 2-ready

    private static final String STK_MAIN = "com.android.stk.StkMain";
    private static final String STK_MENU_LAUNCHER_ACTIVITY = "com.android.stk.StkLauncherActivity";
    private static final String STK1_LAUNCHER_ACTIVITY = "com.android.stk.StkLauncherActivityI";
    private static final String STK2_LAUNCHER_ACTIVITY = "com.android.stk.StkLauncherActivityII";
    private static final String STK3_LAUNCHER_ACTIVITY = "com.android.stk.StkLauncherActivityIII";
    private static final String STK4_LAUNCHER_ACTIVITY = "com.android.stk.StkLauncherActivityIV";
    /// @}


    private StkAppInstaller() {
        MtkCatLog.d(LOG_TAG, "init");
    }

    //MTK add begin
    public void install(Context context) {
        //setAppState(context, true);
        setAppState(context, true, STK_LAUNCH_ID);
    }

    public void unInstall(Context context) {
        //setAppState(context, false);
        setAppState(context, false, STK_LAUNCH_ID);

    }
    //MTK add end

    private static void setAppState(Context context, boolean install, int sim_id) {
        MtkCatLog.d(LOG_TAG, "[setAppState]+ sim_id: " + sim_id + ", install: " + install);
        if (context == null) {
            MtkCatLog.d(LOG_TAG, "[setAppState]- no context, just return.");
            return;
        }
        PackageManager pm = context.getPackageManager();
        if (pm == null) {
            MtkCatLog.d(LOG_TAG, "[setAppState]- no package manager, just return.");
            return;
        }

        //MTK add begin

        String classname = null;
        if (StkAppService.isOP02) {
            switch (sim_id) {
                case PhoneConstants.SIM_ID_1:
                    classname = STK1_LAUNCHER_ACTIVITY;
                    break;
                case PhoneConstants.SIM_ID_2:
                    classname = STK2_LAUNCHER_ACTIVITY;
                    break;
                case PhoneConstants.SIM_ID_3:
                    classname = STK3_LAUNCHER_ACTIVITY;
                    break;
                case PhoneConstants.SIM_ID_4:
                    classname = STK4_LAUNCHER_ACTIVITY;
                    break;
                case STK_LAUNCH_ID:
                    classname = STK_MAIN;
                    break;

                default:
                    MtkCatLog.v("StkAppInstaller2", "setAppState, ready to return because sim id "
                    + sim_id + " is wrong.");
                    return;
            }
        } else {
            switch (sim_id) {
                case STK_LAUNCH_ID:
                    classname = STK_MAIN;
                    break;

                default:
                    MtkCatLog.v("StkAppInstaller2", "setAppState, ready to return because sim id "
                    + sim_id + " is wrong.");
                    return;
            }
        }
        //MTK add end

        ComponentName cName = new ComponentName("com.android.stk", classname);
        int state = install ? PackageManager.COMPONENT_ENABLED_STATE_ENABLED
                : PackageManager.COMPONENT_ENABLED_STATE_DISABLED;

        if (((PackageManager.COMPONENT_ENABLED_STATE_ENABLED == state) &&
                (PackageManager.COMPONENT_ENABLED_STATE_ENABLED ==
                pm.getComponentEnabledSetting(cName))) ||
                ((PackageManager.COMPONENT_ENABLED_STATE_DISABLED == state) &&
                (PackageManager.COMPONENT_ENABLED_STATE_DISABLED ==
                pm.getComponentEnabledSetting(cName)))) {
            MtkCatLog.d(LOG_TAG, "Need not change app state!!");
        } else {
            MtkCatLog.d(LOG_TAG, "Change app state[" + install + "]");
            try {
                pm.setComponentEnabledSetting(cName, state, PackageManager.DONT_KILL_APP);
            } catch (Exception e) {
                MtkCatLog.d(LOG_TAG, "Could not change STK app state");
            }
        }

        //MTK add begin
        if (STK_LAUNCH_ID == sim_id) {
            try {
                miSTKInstalled[(sSimCount - 1)] = install ? STK_INSTALLED : STK_NOT_INSTALLED;
            } catch (ArrayIndexOutOfBoundsException e) {
                MtkCatLog.e(LOG_TAG, "IOOB of setting miSTKInstalled[]");
            }
        } else if ((SubscriptionManager.isValidSlotIndex(sim_id)) || StkAppService.isOP02) {
            try {
                miSTKInstalled[sim_id] = install ? STK_INSTALLED : STK_NOT_INSTALLED;
            } catch (ArrayIndexOutOfBoundsException e) {
                MtkCatLog.e(LOG_TAG, "IOOB3 of setting miSTKInstalled[]");
            }
        }
        //MTK add end

        MtkCatLog.d(LOG_TAG, "[setAppState]-, sim_id: " + sim_id +
                " Change app state[" + install + "]");
    }


    /// M:  @{
    public static StkAppInstaller getInstance() {
        if (mInstance != null) {
            mInstance.initThread();
        }
        return mInstance;
    }

    public static int getIsInstalled(int sim_id) {
        if (STK_LAUNCH_ID == sim_id) {
            sim_id = (sSimCount - 1);
        }
        return miSTKInstalled[sim_id];
    }

    void install(Context context, int sim_id) {
        MtkCatLog.d(LOG_TAG, "[install]+ sim_id: " + sim_id);
        if (STK_LAUNCH_ID == sim_id) {
            sim_id = (sSimCount - 1);
        }
        if (installThread[sim_id] != null) {
            mContext = context;
            installThread[sim_id].setSim(sim_id);
            new Thread(installThread[sim_id]).start();
        }
    }

    void unInstall(Context context, int sim_id) {
        MtkCatLog.v(LOG_TAG, "[unInstall]+ sim_id: " + sim_id);
        if (STK_LAUNCH_ID == sim_id) {
            sim_id = (sSimCount - 1);
        }
        if (uninstallThread[sim_id] != null) {
            mContext = context;
            uninstallThread[sim_id].setSim(sim_id);
            new Thread(uninstallThread[sim_id]).start();
        }
    }

    private void initThread() {
        int i = 0;
        // The STK_GEMINI_SIM_NUM index is for STK_MAIN
        sSimCount = StkAppService.STK_GEMINI_SIM_NUM + 1;
        MtkCatLog.v(LOG_TAG, "installThread = " + installThread + ", uninstallThread = "
                + uninstallThread + ", miSTKInstalled = " + miSTKInstalled);
        if (installThread == null) {
            installThread = new InstallThread[sSimCount];
            for (i = 0; i < sSimCount; i++) {
                if (installThread[i] == null) {
                    installThread[i] = new InstallThread();
                }
            }
        }
        if (uninstallThread == null) {
            uninstallThread = new UnInstallThread[sSimCount];
            for (i = 0; i < sSimCount; i++) {
                uninstallThread[i] = new UnInstallThread();
            }
        }
        if (miSTKInstalled == null) {
            miSTKInstalled = new int[sSimCount];
            for (i = 0; i < sSimCount; i++) {
                miSTKInstalled[i] = -1;
            }
        }
    }

    private class InstallThread implements Runnable {
        private int mSimId = -1;
        @Override
        public void run() {
            if ((sSimCount - 1) == mSimId) {
                setAppState(mContext, true, STK_LAUNCH_ID);
            } else if (SubscriptionManager.isValidSlotIndex(mSimId)) {
                setAppState(mContext, true, mSimId);
            }
        }

        public void setSim(int sim_id) {
            mSimId = sim_id;
        }
    }

    private class UnInstallThread implements Runnable {
        private int mSimId = -1;
        @Override
        public void run() {
            String optr = SystemProperties.get("persist.operator.optr");
            if ((sSimCount - 1) == mSimId) {
                setAppState(mContext, false, STK_LAUNCH_ID);
            } else if ((SubscriptionManager.isValidSlotIndex(mSimId)) || StkAppService.isOP02) {
                setAppState(mContext, false, mSimId);
            }
        }

        public void setSim(int sim_id) {
            mSimId = sim_id;
        }
    }
    /// @}
}
