/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.security;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.FragmentTransaction;
import android.content.Context;
import android.os.Bundle;

import com.mediatek.security.R;
import com.mediatek.security.Log;

/**
 * Adapter for the action bar at the top of the
 * NetworkDataControllerActivity.java.
 */
public class ActionBarAdapter {
    private static final String TAG = "ActionBarAdapter";

    private final Context mContext;
    private final ActionBar mActionBar;

    private static final String EXTRA_KEY_SELECTED_TAB = "navBar.selectedTab";
    private static final TabState DEFAULT_TAB = TabState.PERMISSIONS_INFO;
    private TabState mCurrentTab = DEFAULT_TAB;

    private Listener mListener;
    private final MyTabListener mTabListener = new MyTabListener();

    public interface Listener {
        void onSelectedTabChanged();
    }

    public enum TabState {
        PERMISSIONS_INFO, APPS_INFO;

        public static TabState fromInt(int value) {

            if (PERMISSIONS_INFO.ordinal() == value) {
                return PERMISSIONS_INFO;
            }
            if (APPS_INFO.ordinal() == value) {
                return APPS_INFO;
            }
            throw new IllegalArgumentException("Invalid value: " + value);
        }
    }

    public ActionBarAdapter(Context context, Listener listener,
            ActionBar actionBar) {
        mContext = context;
        mListener = listener;
        mActionBar = actionBar;
    }

    public void addUpdateTab(Bundle savedState) {
        // Set up tabs
        addTab(TabState.PERMISSIONS_INFO, R.string.normal_apps);
        addTab(TabState.APPS_INFO, R.string.system_apps);
        Log.d(TAG, "initialize() .....");
        if (savedState != null) {
            mCurrentTab = TabState.fromInt(savedState
                    .getInt(EXTRA_KEY_SELECTED_TAB));
            Log.d(TAG, "get saved tab  " + mCurrentTab);
        }
        update();
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    private void addTab(TabState tabState, int description) {
        final Tab tab = mActionBar.newTab();
        tab.setTag(tabState);
        tab.setTabListener(mTabListener);
        tab.setText(description);
        mActionBar.addTab(tab);
    }

    public void removeAllTab() {
        mActionBar.removeAllTabs();
        // must set the navigation mode to standard ,
        // or will show the tab background
        mTabListener.mIgnoreTabSelected = true;
        mActionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);
        mTabListener.mIgnoreTabSelected = false;
    }

    private class MyTabListener implements ActionBar.TabListener {
        /**
         * If true, it won't call {@link #setCurrentTab} in
         * {@link #onTabSelected}. This flag is used when we want to
         * programmatically update the current tab without
         * {@link #onTabSelected} getting called.
         */
        public boolean mIgnoreTabSelected;

        @Override
        public void onTabReselected(Tab tab, FragmentTransaction ft) {
        }

        @Override
        public void onTabUnselected(Tab tab, FragmentTransaction ft) {
        }

        @Override
        public void onTabSelected(Tab tab, FragmentTransaction ft) {
            Log.d(TAG, "onTabSelected() " + " ignore ? " + mIgnoreTabSelected);
            if (!mIgnoreTabSelected) {
                Log.d(TAG, "setCurrentTab()");
                setCurrentTab((TabState) tab.getTag(), true);
            }
        }
    }

    /*
     * change the current tab
     *
     * @param tab TabState, tab will be set to the current
     *
     * @param notifyListener boolean , if true , will initiative update fragment
     * visibility
     */

    public void setCurrentTab(TabState tab, boolean notifyListener) {
        if (tab == null) {
            throw new NullPointerException();
        }
        if (tab == mCurrentTab) {
            Log.d(TAG, "tab == mCurrentTab ,directly return " + tab);
            return;
        }
        mCurrentTab = tab;
        Log.d(TAG, "mCurrentTab = " + mCurrentTab + " notifyListner "
                + notifyListener);
        int index = mCurrentTab.ordinal();
        if ((mActionBar.getNavigationMode() == ActionBar.NAVIGATION_MODE_TABS)
                && (index != mActionBar.getSelectedNavigationIndex())) {
            mActionBar.setSelectedNavigationItem(index);
        }

        if (notifyListener && mListener != null) {
            mListener.onSelectedTabChanged();
        }
    }

    public TabState getCurrentTab() {
        return mCurrentTab;
    }

    private void update() {
        if (mActionBar.getNavigationMode() != ActionBar.NAVIGATION_MODE_TABS) {
            Log.d(TAG, "update() " + mCurrentTab.ordinal() + mCurrentTab);

            /*
             * setNavigationMode will trigger onTabSelected() with the tab which
             * was previously selected. The issue is that when we're first
             * switching to the tab navigation mode after screen orientation
             * changes, onTabSelected() will get called with the first tab (i.e.
             * "Apps"), which would results in mCurrentTab getting set to "Apps"
             * and we'd lose restored tab. So let's just disable the callback
             * here temporarily. We'll notify the listener after this anyway.
             */
            mTabListener.mIgnoreTabSelected = true;
            mActionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
            mActionBar.setSelectedNavigationItem(mCurrentTab.ordinal());
            mTabListener.mIgnoreTabSelected = false;
        }

    }

    public void onSaveInstanceState(Bundle outState) {
        Log.d(TAG, "onSaveInstanceState() " + mCurrentTab.ordinal());
        outState.putInt(EXTRA_KEY_SELECTED_TAB, mCurrentTab.ordinal());
    }
}
