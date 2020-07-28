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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
package com.mediatek.duraspeed;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Process;
import android.util.Log;

public class DuraSpeedMainActivity extends Activity {
    private static final String TAG = "DuraSpeedMainActivity";
    private Fragment mFragment;
    private PackageReceiver mPackageReceiver = new PackageReceiver();
    private boolean mRegistered = false;

    @Override
    protected void onCreate(Bundle savedState) {
        super.onCreate(savedState);
        // If start by monkey, finish it
        if (isMonkeyRunning()) {
            finish();
        }

        Utils.createDatabaseManager(this);

        // Setup UI
        setContentView(R.layout.main_activity);
        mFragment = createFragment(DuraSpeedFragment.class.getName(), null, false);

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_PACKAGE_ADDED);
        filter.addAction(Intent.ACTION_PACKAGE_REMOVED);
        filter.addDataScheme("package");
        registerReceiver(mPackageReceiver, filter);
        mRegistered = true;
        Utils.sStarted = true;
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Utils.sDatabaseManager = null;
        if (mRegistered) {
            unregisterReceiver(mPackageReceiver);
            mRegistered = false;
        }
        if (Utils.sLowRamDevice && Utils.sStarted) {
            Utils.sStarted = false;
            Process.killProcessQuiet(Process.myPid());
        }
    }

    private Fragment createFragment(String fragmentName, Bundle args, boolean backStack) {
        Fragment f = Fragment.instantiate(this, fragmentName, args);
        FragmentTransaction transaction = getFragmentManager().beginTransaction();
        transaction.replace(R.id.main_content, f);
        transaction.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN);
        if (backStack) {
            transaction.addToBackStack("DuraSpeed");
        }
        transaction.commitAllowingStateLoss();
        return f;
    }

    private class PackageReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null) {
                String action = intent.getAction();
                Log.d(TAG, "onReceive, action: " + action);

                String packageName = intent.getData().getSchemeSpecificPart();
                if (Intent.ACTION_PACKAGE_ADDED.equals(action)) {
                    // Add package info to DB and cache
                    if ((Utils.sDatabaseManager != null)
                            && Utils.sDatabaseManager.insert(packageName)) {
                        notifyPackageUpdated();
                    }
                } else if (Intent.ACTION_PACKAGE_REMOVED.equals(action)) {
                    // Remove package info from DB and cache
                    if ((Utils.sDatabaseManager != null)
                            && Utils.sDatabaseManager.delete(packageName)) {
                        notifyPackageUpdated();
                    }
                }
            }
        }

        private void notifyPackageUpdated() {
            if (mFragment instanceof DuraSpeedFragment) {
                ((DuraSpeedFragment) mFragment).onPackageUpdated();
            } else {
                Log.d(TAG, "Can not find fragment: " + mFragment);
            }
        }
    }

    private boolean isMonkeyRunning() {
        return ActivityManager.isUserAMonkey();
    }
}
