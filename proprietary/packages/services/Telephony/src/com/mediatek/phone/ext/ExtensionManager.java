/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.phone.ext;


import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.common.carrierexpress.CarrierExpressManager;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

public final class ExtensionManager {

    private static final String LOG_TAG = "ExtensionManager";
    private static Context sContext;

    private static ICallFeaturesSettingExt sCallFeaturesSettingExt;
    private static ISimDialogExt sSimDialogExt;
    private static IAccessibilitySettingsExt sAccessibilitySettingsExt;
    private static ITtyModeListPreferenceExt sTtyModeListPreferenceExt;
    private static IEmergencyDialerExt sEmergencyDialerExt;
    private static IRttUtilExt sRttUtilExt;
    private static ISCBMHelperExt sSCBMHelperExt;
    private static IIncomingCallExt sIncomingCallExt;
    private static IPhoneGlobalsExt sPhoneGlobalsExt;

    private ExtensionManager() {
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

    public static void init(Application application) {
        sContext = application.getApplicationContext();
    }

    public static void registerApplicationContext(Context context) {
        if (sContext == null) {
            sContext = context;
            log("registerApplicationContext");
            sContext.registerReceiver(new BroadcastReceiver() {
                    @Override
                    public void onReceive(Context context, Intent intent) {
                        log("[onReceive] Clear plugin due to " + intent.getAction());
                        OpPhoneCustomizationUtils.resetOpFactory(sContext);
                        synchronized (ICallFeaturesSettingExt.class) {
                            sCallFeaturesSettingExt = null;
                        }
                        synchronized (ISimDialogExt.class) {
                            sSimDialogExt = null;
                        }
                        synchronized (IAccessibilitySettingsExt.class) {
                            sAccessibilitySettingsExt = null;
                        }
                        synchronized (ITtyModeListPreferenceExt.class) {
                            sTtyModeListPreferenceExt = null;
                        }
                        synchronized (IEmergencyDialerExt.class) {
                            sEmergencyDialerExt = null;
                        }
                        synchronized (IRttUtilExt.class) {
                            sRttUtilExt = null;
                        }
                        synchronized (ISCBMHelperExt.class) {
                            sSCBMHelperExt = null;
                        }
                        synchronized (IIncomingCallExt.class) {
                            sIncomingCallExt = null;
                        }
                    }
                },
                new IntentFilter(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED));
        }
    }

    public static void resetApplicationContext(Context context) {
        sContext = context;
        log("resetApplicationContext");
        OpPhoneCustomizationUtils.resetOpFactory(sContext);
        synchronized (ICallFeaturesSettingExt.class) {
            sCallFeaturesSettingExt = null;
        }
        synchronized (IAccessibilitySettingsExt.class) {
            sAccessibilitySettingsExt = null;
        }
        synchronized (ITtyModeListPreferenceExt.class) {
            sTtyModeListPreferenceExt = null;
        }
        synchronized (IEmergencyDialerExt.class) {
            sEmergencyDialerExt = null;
        }
        synchronized (IRttUtilExt.class) {
            sRttUtilExt = null;
        }
    }

    public static ICallFeaturesSettingExt getCallFeaturesSettingExt() {
        if (sCallFeaturesSettingExt == null) {
            synchronized (ICallFeaturesSettingExt.class) {
                if (sCallFeaturesSettingExt == null) {
                    sCallFeaturesSettingExt = OpPhoneCustomizationUtils.getOpFactory(sContext)
                            .makeCallFeaturesSettingExt();
                    log("[getCallFeaturesSettingExt]create ext instance: " +
                                 sCallFeaturesSettingExt);
                }
            }
        }
        return sCallFeaturesSettingExt;
    }

    public static ISimDialogExt getSimDialogExt() {
        if (sSimDialogExt == null) {
            synchronized (ISimDialogExt.class) {
                if (sSimDialogExt == null) {
                    sSimDialogExt = OpPhoneCustomizationUtils.getOpFactory(sContext)
                            .makeSimDialogExt();
                    log("[sSimDialogExt] create ext instance: "
                            + sSimDialogExt);
                }
            }
        }
        return sSimDialogExt;
    }

    public static IAccessibilitySettingsExt getAccessibilitySettingsExt() {
        if (sAccessibilitySettingsExt == null) {
            synchronized (IAccessibilitySettingsExt.class) {
                if (sAccessibilitySettingsExt == null) {
                    sAccessibilitySettingsExt = OpPhoneCustomizationUtils.getOpFactory(sContext)
                            .makeAccessibilitySettingsExt();
                    log("[getAccessibilitySettingsExt]create ext instance: " +
                        sAccessibilitySettingsExt);
                }
            }
        }
        return sAccessibilitySettingsExt;
    }

    public static ITtyModeListPreferenceExt getTtyModeListPreferenceExt() {
        if (sTtyModeListPreferenceExt == null) {
            synchronized (ITtyModeListPreferenceExt.class) {
                if (sTtyModeListPreferenceExt == null) {
                    sTtyModeListPreferenceExt = OpPhoneCustomizationUtils.getOpFactory(sContext)
                            .makeTtyModeListPreferenceExt();
                    log("[getTtyModeListPreferenceExt]create ext instance: " +
                        sTtyModeListPreferenceExt);
                }
            }
        }
        return sTtyModeListPreferenceExt;
    }

    public static IEmergencyDialerExt getEmergencyDialerExt() {
        if (sEmergencyDialerExt == null) {
            synchronized (IEmergencyDialerExt.class) {
                if (sEmergencyDialerExt == null) {
                    sEmergencyDialerExt = OpPhoneCustomizationUtils.getOpFactory(sContext)
                            .makeEmergencyDialerExt();
                    log("[sEmergencyDialerExt] create ext instance: "
                            + sEmergencyDialerExt);
                }
            }
        }
        return sEmergencyDialerExt;
    }

    /**
     * Get RTT extension.
     *
     * @return IRttUtilExt interface implementation.
     */
    public static IRttUtilExt getRttUtilExt() {
        if (sRttUtilExt == null) {
            synchronized (IRttUtilExt.class) {
                if (sRttUtilExt == null) {
                    sRttUtilExt = CommonPhoneCustomizationUtils.getOpFactory(sContext)
                            .makeRttUtilExt();
                    log("[sRttUtilExt] create ext instance: " + sRttUtilExt);
                }
            }
        }
        return sRttUtilExt;
    }

    public static void initPhoneHelper() {
        if ("OP01".equals(SystemProperties.get("persist.vendor.operator.optr", "")) ||
            "1".equals(SystemProperties.get("ro.vendor.cmcc_light_cust_support", ""))) {
            try {
                Class<?> cls = Class.forName("cn.richinfo.dm.CtmApplication");
                Method method = cls.getMethod("getInstance", Application.class);
                method.invoke(null, (Application) sContext);
            } catch (ClassNotFoundException |
                     NoSuchMethodException |
                     IllegalAccessException |
                     InvocationTargetException e) {
                e.printStackTrace();
            }
        }
    }

    public static ISCBMHelperExt getSCBMHelperExt() {
        if (sSCBMHelperExt == null) {
            synchronized (ISCBMHelperExt.class) {
                if (sSCBMHelperExt == null) {
                    sSCBMHelperExt = OpPhoneCustomizationUtils.getOpFactory(sContext)
                            .makeSCBMHelperExt();
                    log("[sSCBMHelperExt] create ext instance: "
                            + sSCBMHelperExt);
                }
            }
        }
        return sSCBMHelperExt;
    }

    public static IIncomingCallExt getIncomingCallExt() {
        if (sIncomingCallExt == null) {
            synchronized (IIncomingCallExt.class) {
                if (sIncomingCallExt == null) {
                    sIncomingCallExt = OpPhoneCustomizationUtils.getOpFactory(sContext)
                            .makeIncomingCallExt();
                    log("[sIncomingCallExt] create ext instance: "
                            + sIncomingCallExt);
                }
            }
        }
        return sIncomingCallExt;
    }

    public static IPhoneGlobalsExt getPhoneGlobalsExt() {
        if (sPhoneGlobalsExt == null) {
            synchronized (IPhoneGlobalsExt.class) {
                if (sPhoneGlobalsExt == null) {
                    sPhoneGlobalsExt = OpPhoneCustomizationUtils.getOpFactory(sContext)
                            .makePhoneGlobalsExt();
                    log("[sPhoneGlobalsExt] create ext instance: "
                            + sPhoneGlobalsExt);
                }
            }
        }
        return sPhoneGlobalsExt;
    }

}
