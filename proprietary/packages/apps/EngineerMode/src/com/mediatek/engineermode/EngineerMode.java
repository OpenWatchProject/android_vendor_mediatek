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
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.provider.Settings;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.PagerTitleStrip;
import android.support.v4.view.ViewPager;
import android.view.View;



/**
 * This is main UI of EngineerMode. It uses viewPager to show each classified
 * modules. There are six viewPager, each viewPager is in one category:
 * 1.telephony;
 * 2.connectivity;
 * 3.hardware testing;
 * 4.location;
 * 5.log&debugging;
 * 6.others.
 */
public class EngineerMode extends Activity {

    private static final String TAG = "MainEntrance";
    private static final String PROP_MONKEY = "ro.monkey";
    private int mTabCount;
    private static final int TAB_COUNT = 6; // Total count of PagerView
    private static final int TAB_COUNT_WIFIONLY = 5; // Total count of PagerView
    // Define each tabs which will attach to PagerView
    private PrefsFragment mTabs[] = new PrefsFragment[TAB_COUNT];

    private static final int DIALOG_DEVELOPMENT_SETTINGS_DISABLED = 0;
    private boolean mLastEnabledState;
    // Record each viewPager title string IDs in array:
    private int[] mTabTitleList;
    private static final int[] TAB_TITLE_IDS = { R.string.tab_telephony,
            R.string.tab_connectivity, R.string.tab_hardware_testing,
            R.string.tab_location, R.string.tab_log_and_debugging,
            R.string.tab_others, };

    private static final int[] TAB_TITLE_IDS_WIFIONLY = {
            R.string.tab_connectivity, R.string.tab_hardware_testing,
            R.string.tab_location, R.string.tab_log_and_debugging,
            R.string.tab_others, };

    private MyPagerAdapter mPagerAdapter;
    public static boolean sWifiOnly = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (isAutoTest()) {
            finish();
            return;
        }
        setContentView(R.layout.main);


        final FragmentManager fragmentManager = getFragmentManager();
        final FragmentTransaction transaction = fragmentManager
                 .beginTransaction();

        if (isWifiOnly()) {
            mTabTitleList = TAB_TITLE_IDS_WIFIONLY;
            mTabCount = TAB_COUNT_WIFIONLY;
        } else if (UserHandle.MU_ENABLED && UserManager.supportsMultipleUsers()
               && UserManager.get(EngineerMode.this).getUserHandle() != UserHandle.USER_OWNER) {
            mTabTitleList = TAB_TITLE_IDS_WIFIONLY;
            mTabCount = TAB_COUNT_WIFIONLY;
       } else {
           mTabTitleList = TAB_TITLE_IDS;
           mTabCount = TAB_COUNT;
       }

        for (int i = 0; i < mTabCount; i++) {
            if (savedInstanceState != null) {
                mTabs[i] = (PrefsFragment) fragmentManager.findFragmentByTag(String.valueOf(i));
                if (mTabs[i] != null) {
                    Elog.i(TAG, "load old fragment:" + i);
                    mTabs[i].setResource(i);
                    transaction.hide(mTabs[i]);
                    continue;
                }
            }
            mTabs[i] = new PrefsFragment();
            mTabs[i].setResource(i);
            transaction.add(R.id.viewpager, mTabs[i], String.valueOf(i));
            transaction.hide(mTabs[i]);
        }

        ViewPager viewPager;
        PagerTitleStrip pagerTitleStrip;

        viewPager = (ViewPager) findViewById(R.id.viewpager);
        pagerTitleStrip = (PagerTitleStrip) findViewById(R.id.pagertitle);
        pagerTitleStrip.setTextSpacing(100);

        transaction.commitAllowingStateLoss();

        mPagerAdapter = new MyPagerAdapter();
        viewPager.setAdapter(mPagerAdapter);
        viewPager.setCurrentItem(0);

        mLastEnabledState = Settings.Global.getInt(getContentResolver(),
        Settings.Global.DEVELOPMENT_SETTINGS_ENABLED, 0) != 0;
        if ( mLastEnabledState
            || ChipSupport.isFeatureSupported(ChipSupport.MTK_INIERNAL_LOAD)
            || FeatureSupport.isEngLoad()
            || FeatureSupport.isUserDebugLoad() ) {
            Elog.v(TAG, "mLastEnabledState=" + mLastEnabledState);
        } else {
            Elog.i(TAG, "you must a developer,mLastEnabledState=" + mLastEnabledState);
            showDialog(DIALOG_DEVELOPMENT_SETTINGS_DISABLED);
        }
    }

   private boolean isWifiOnly() {
        ConnectivityManager connManager = (ConnectivityManager) EngineerMode.this
                .getSystemService(Context.CONNECTIVITY_SERVICE);

        if (null != connManager) {
            sWifiOnly = !connManager
                    .isNetworkSupported(ConnectivityManager.TYPE_MOBILE);
            Elog.i(TAG, "sWifiOnly: " + sWifiOnly);
        }
        return sWifiOnly;
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        if (mLastEnabledState) {
            mPagerAdapter.updateCurrentFragment();
        }
    }


    /**
     * EngineerMode UI tab page content adapter.
     *
     */
    class MyPagerAdapter extends PagerAdapter {
        private final FragmentManager mFragmentManager;
        private FragmentTransaction mCurTransaction = null;
        private Fragment mCurPrimaryItem;

        MyPagerAdapter() {
            mFragmentManager = getFragmentManager();
        }

        @Override
        public int getCount() {
            return mTabCount;
        }

        @Override
        public void destroyItem(View container, int position, Object object) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
            mCurTransaction.hide((Fragment) object);
        }

        @Override
        public CharSequence getPageTitle(int position) {
            return getString(mTabTitleList[position]).toString();
        }

        @Override
        public Object instantiateItem(View container, int position) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
            Fragment fragment = getFragment(position);
            mCurTransaction.show(fragment);

            // Non primary pages are not visible.
            fragment.setUserVisibleHint(fragment.equals(mCurPrimaryItem));
            return fragment;
        }

        @Override
        public void finishUpdate(View container) {
            if (mCurTransaction != null) {
                mCurTransaction.commitAllowingStateLoss();
                mCurTransaction = null;
                mFragmentManager.executePendingTransactions();
            }
        }

        @Override
        public boolean isViewFromObject(View view, Object object) {
            return ((Fragment) object).getView() == view;
        }

        @Override
        public void setPrimaryItem(View container, int position, Object object) {
            Fragment fragment = (Fragment) object;
            if (!fragment.equals(mCurPrimaryItem)) {
                if (mCurPrimaryItem != null) {
                    mCurPrimaryItem.setUserVisibleHint(false);
                }
                mCurPrimaryItem = fragment;
                mCurPrimaryItem.setUserVisibleHint(true);
            }
        }

        private Fragment getFragment(int position) {
            if (position < mTabCount) {
                return mTabs[position];
            }
            throw new IllegalArgumentException("position: " + position);
        }

        public void updateCurrentFragment() {
            if (mCurPrimaryItem != null) {
                mCurPrimaryItem.setUserVisibleHint(true);
            }

        }
    }
    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        switch (id) {
        case DIALOG_DEVELOPMENT_SETTINGS_DISABLED:
            builder = new AlertDialog.Builder(this);
            builder.setTitle("Notice:");
            builder.setCancelable(false);
            builder.setMessage("EM is an advanced debug mode." +
                               " if you want to entry the EM," +
                      "please entry the Developer options at settings!");
            builder.setPositiveButton(R.string.dialog_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
            break;
        default:
            Elog.d(TAG, "error dialog ID");
            break;
        }
        return dialog;
    }

    /**
     * Check if device is in monkey test which is not allowed for EngineerMode.
     * @return true if the device is in monkey test
     */
    public static boolean isAutoTest() {

        // MTK method
        boolean isMonkeyRunning = SystemProperties.getBoolean(PROP_MONKEY, false);
        Elog.i(TAG, "isAutoTest()-> Monkey running flag is " + isMonkeyRunning);

        // Android default API
        boolean isUserAMonkey = ActivityManager.isUserAMonkey();
        Elog.i(TAG, "isAutoTest()-> isUserAMonkey=" + isUserAMonkey);

        return (isMonkeyRunning || isUserAMonkey);
    }
}
