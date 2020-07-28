/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.sensorhub.ui;

import android.app.ActivityManager;
import android.hardware.Sensor;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.sensorhub.settings.MtkSensor;
import com.mediatek.sensorhub.settings.Utils;

import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.HashMap;

public class SensorSettings extends PreferenceActivity implements
        ActivityCompat.OnRequestPermissionsResultCallback{

    private static final String TAG = "SH/SensorSettings";
    private static final String[] KEY_SET = {/*Sensor.STRING_TYPE_STEP_COUNTER,*/
    Utils.KEY_STRESS_FULL_TEST,
    Utils.KEY_STRESS_TEST};
    private static final String PROP_MONKEY = "ro.monkey";
    private PermissionManager mPermissionManager;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(isAutoTest()) {
            Log.i(TAG, "isAutoTest return.");
            finish();
            return;
        }

        mPermissionManager = new PermissionManager(this);
        mPermissionManager.requestLaunchPermissions();

        addPreferencesFromResource(R.xml.sensorhub_settings_pref);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Utils.acquireScreenWakeLock(this);
        updatePreferenceStatus();
    }

    @Override
    protected void onPause() {
        Utils.releaseScreenWakeLock();
        super.onPause();
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return false;
    }

    private void updatePreferenceStatus() {
        for(String key : KEY_SET) {
            findPreference(key).setSummary(Utils.getSensorStatus(key) ?
                    R.string.running_summary : R.string.space_summary);
        }
    }


    private static boolean isAutoTest() {

        // MTK method
        String monkeyProp = getProperty(PROP_MONKEY);
        Log.v(TAG, "isAutoTest()-> Monkey running flag is " + monkeyProp);
        boolean isMonkey = ("true".equals(monkeyProp)
                || "y".equals(monkeyProp)
                || "yes".equals(monkeyProp)
                || "1".equals(monkeyProp)
                || "on".equals(monkeyProp));
//        boolean isMonkey = getBooleanProperty(PROP_MONKEY, false);
        // Andriod default API
        boolean isUserAMonkey = ActivityManager.isUserAMonkey();
        Log.v(TAG, "isAutoTest()-> isUserAMonkey=" + isUserAMonkey);
        return (isMonkey || isUserAMonkey);
    }
    /**
     * get system.properties
     *
     * @param key
     *            The property key
     * @return property value
     */
    public static String getProperty(String key) {
        String value = "";
        try {
            Class<?> c = Class.forName("android.os.SystemProperties");
            Method get = c.getMethod("get", String.class, String.class);
            value = (String)(get.invoke(c, key, "" ));
        } catch (Exception e) {
            e.printStackTrace();
        }finally {
            return value;
        }
    }

    public void onRequestPermissionsResult(int requestCode, String permissions[], int[]
            grantResults) {
        Log.i(TAG, "onRequestPermissionsResult(), requestCode = " + requestCode);
        if (grantResults == null || grantResults.length <= 0) {
            return;
        }
        if (mPermissionManager.getLaunchPermissionRequestCode() == requestCode) {
            if (!mPermissionManager.isLaunchPermissionsResultReady(permissions, grantResults)) {
                // more than one critical permission was denied activity finish, exit and destroy
                Toast.makeText(this, R.string.denied_required_permission,
                        Toast.LENGTH_LONG).show();
                finish();
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

}
