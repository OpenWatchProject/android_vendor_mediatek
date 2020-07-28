/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.security;

import android.app.ActionBar;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.os.Bundle;
import android.os.Parcelable;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;

import com.mediatek.security.R;
import com.mediatek.security.Log;
import com.mediatek.security.ActionBarAdapter.TabState;
import com.mediatek.security.datamanager.CheckedPermRecord;

/**
 * Activity to pick an application that will be used to display installation
 * information and options to uninstall/delete user data for system
 * applications. This activity can be launched through Settings or via the
 * ACTION_MANAGE_PACKAGE_STORAGE intent.
 */
public class NetworkDataControllerActivity extends Activity implements
        ActionBarAdapter.Listener {

    static final String TAG = "NetworkDataControllerActivity";

    private static final int PERMISSIONS_INFO = 0;
    private static final int APPS_INFO = 1;
    private static final int NUM_TABS = 2;

    private ViewPager mTabPager;
    private TabPagerAdapter mTabPagerAdapter;
    private ActionBarAdapter mActionBarAdapter;
    private final TabPagerListener mTabPagerListener = new TabPagerListener();

    final String mSystemAppsTag = "tab-systemApps";
    final String mNormalAppsTag = "tab-normalApps";
    private NormalAppFragment mNormalAppFragment;
    private SystemAppFragment mSystemAppFragment;

    private Bundle mSavedInstanceState;

    private class TabPagerAdapter extends PagerAdapter {
        private final FragmentManager mFragmentManager;
        private FragmentTransaction mCurTransaction = null;
        private Fragment mCurrentPrimaryItem;

        public TabPagerAdapter() {
            mFragmentManager = getFragmentManager();
        }

        @Override
        public int getCount() {
            return NUM_TABS;
        }

        /** Gets called when the number of items changes. */
        @Override
        public int getItemPosition(Object object) {
            if (object == mNormalAppFragment) {
                return PERMISSIONS_INFO;
            }

            if (object == mSystemAppFragment) {
                return APPS_INFO;
            }
            return POSITION_NONE;
        }

        @Override
        public void startUpdate(View container) {
        }

        private Fragment getFragment(int position) {
            if (position == PERMISSIONS_INFO) {
                return mNormalAppFragment;
            } else if (position == APPS_INFO) {
                return mSystemAppFragment;
            }

            throw new IllegalArgumentException("position: " + position);
        }

        @Override
        public Object instantiateItem(View container, int position) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
            Fragment f = getFragment(position);
            return f;
        }

        @Override
        public void destroyItem(View container, int position, Object object) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
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
            if (mCurrentPrimaryItem != fragment) {
                if (mCurrentPrimaryItem != null) {
                    mCurrentPrimaryItem.setUserVisibleHint(false);
                }
                if (fragment != null) {
                    fragment.setUserVisibleHint(true);
                }
                mCurrentPrimaryItem = fragment;
            }
        }

        @Override
        public Parcelable saveState() {
            return null;
        }

        @Override
        public void restoreState(Parcelable state, ClassLoader loader) {
        }
    }

    private class TabPagerListener implements ViewPager.OnPageChangeListener {
        @Override
        public void onPageScrollStateChanged(int state) {
        }

        @Override
        public void onPageScrolled(int position, float positionOffset,
                int positionOffsetPixels) {
        }

        @Override
        public void onPageSelected(int position) {
            // Make sure not in the search mode, in which case position !=
            // TabState.ordinal().
            TabState selectedTab = TabState.fromInt(position);
            mActionBarAdapter.setCurrentTab(selectedTab, false);
            NetworkDataControllerActivity.this.updateFragmentsVisibility();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "oncreate() start");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cellular_control_pages);
        mSavedInstanceState = savedInstanceState;
        // add the switch on Action bar
        LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        final int padding = getResources().getDimensionPixelSize(
                R.dimen.action_bar_switch_padding);
        getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_TITLE);
        getActionBar().setHomeButtonEnabled(true);
        getActionBar().setDisplayHomeAsUpEnabled(true);

        // hide fragment firstly , then update it in onResume() according to
        // switch status
        final FragmentManager fragmentManager = getFragmentManager();
        FragmentTransaction transaction = fragmentManager.beginTransaction();

        mNormalAppFragment = (NormalAppFragment) fragmentManager
                .findFragmentByTag(mNormalAppsTag);
        mSystemAppFragment = (SystemAppFragment) fragmentManager
                .findFragmentByTag(mSystemAppsTag);

        if (mNormalAppFragment == null) {
            mNormalAppFragment = new NormalAppFragment();
            mSystemAppFragment = new SystemAppFragment();
            transaction.add(R.id.tab_pager, mNormalAppFragment, mNormalAppsTag);
            transaction.add(R.id.tab_pager, mSystemAppFragment, mSystemAppsTag);
        }

        transaction.hide(mNormalAppFragment);
        transaction.hide(mSystemAppFragment);
        transaction.commit();
        fragmentManager.executePendingTransactions();

        // set page adapter
        mTabPager = (ViewPager) findViewById(R.id.tab_pager);
        mTabPagerAdapter = new TabPagerAdapter();
        mTabPager.setAdapter(mTabPagerAdapter);
        mTabPager.setOnPageChangeListener(mTabPagerListener);

        // Configure action bar
        mActionBarAdapter = new ActionBarAdapter(this, this, getActionBar());

        Log.d(TAG, "oncreate() end");

    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        Log.d(TAG, "onSaveInstanceState()");
        super.onSaveInstanceState(outState);
        if (mActionBarAdapter != null) {
            mActionBarAdapter.onSaveInstanceState(outState);
        }
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "oncreate() start");
        super.onResume();
        addUI();
        Log.d(TAG, "oncreate() end");

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // Some of variables will be null if this Activity redirects Intent.
        // See also onCreate() or other methods called during the Activity's
        // initialization.
        if (mActionBarAdapter != null) {
            mActionBarAdapter.setListener(null);
        }
    }

    @Override
    public void onSelectedTabChanged() {
        updateFragmentsVisibility();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            onBackPressed();
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    private void updateFragmentsVisibility() {
        TabState tab = mActionBarAdapter.getCurrentTab();
        int tabIndex = tab.ordinal();
        if (mTabPager.getCurrentItem() != tabIndex) {
            Log.d(TAG,
                    "mTabPager.getCurrentItem() " + mTabPager.getCurrentItem()
                            + " tabIndex " + tabIndex);
            mTabPager.setCurrentItem(tabIndex);
        }

        invalidateOptionsMenu();

    }

    protected void addUI() {
        Log.d(TAG, "addUI()");
        // must get a new transaction each time
        FragmentTransaction transaction = getFragmentManager()
                .beginTransaction();
        // add all the fragment
        mNormalAppFragment = (NormalAppFragment) getFragmentManager()
                .findFragmentByTag(mNormalAppsTag);
        mSystemAppFragment = (SystemAppFragment) getFragmentManager()
                .findFragmentByTag(mSystemAppsTag);

        if (mNormalAppFragment == null) {
            mNormalAppFragment = new NormalAppFragment();
            mSystemAppFragment = new SystemAppFragment();
            transaction.add(R.id.tab_pager, mNormalAppFragment, mNormalAppsTag);
            transaction.add(R.id.tab_pager, mSystemAppFragment, mSystemAppsTag);
        }
        transaction.show(mNormalAppFragment);
        transaction.show(mSystemAppFragment);

        transaction.commit();

        getFragmentManager().executePendingTransactions();
        // firstly remove tabs ,then add tabs and update it
        mActionBarAdapter.removeAllTab();
        mActionBarAdapter.addUpdateTab(mSavedInstanceState);
    }
}
