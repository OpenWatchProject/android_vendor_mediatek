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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.op18.mms;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.Window;
import android.widget.Toast;

import com.mediatek.op18.mms.utils.Constants;
import com.mediatek.op18.mms.utils.MyLogger;
import com.mediatek.op18.mms.utils.NotifyManager;
import com.mediatek.op18.mms.utils.SDCardUtils;
import com.mediatek.op18.mms.utils.Utils;
import com.mediatek.op18.mms.*;
import com.mediatek.op18.mms.R;

/**
 * @author mtk81330
 *
 */
public class MainActivity extends Activity {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/MainActivity";
    private static final String STATE_TAB = "tab";
    public static final String ACTION_WHERE = "action_where";
    public static final String ACTION_BACKUP = "action_backup";
    public static final String ACTION_RESTORE = "ction_restore";
    private Fragment mBackupFragment;
    private Fragment mRestoreFragment;
    MainOnSDCardStatusChangedListener mMainOnSDCardStatusChangedListener =
            new MainOnSDCardStatusChangedListener();
    private boolean mShouldFinish = false;
    private Bundle mSavedInstanceState;
    private boolean onCreateProcessed = false;
    private Toast mToast = null;
    private boolean mFragmentsInitDone = false;
    private boolean requestPermissionHandled = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSavedInstanceState = savedInstanceState;
        MyLogger.logD(CLASS_TAG, "Anuj onCreate: ");
        if (checkSDCardStatus()) {
            //MyLogger.logD(CLASS_TAG, "onCreate mHostActivityContext = " + mHostActivityContext);
            String[] unsatisfiedPermissions = Utils.getUnsatisfiedPermissions(this);
            if (unsatisfiedPermissions != null && !mShouldFinish) {
                MyLogger.logD(CLASS_TAG, "Request runtime permissions onCreate: "
                    + unsatisfiedPermissions);
                requestPermissionHandled = false;
                requestPermissions(unsatisfiedPermissions, 0); // the request code is ignored
                return;
            }
        }
        else {
            return;
        }
        onCreateProcessed = true;
        continueOnCreate();
    }

    private void continueOnCreate() {
        mShouldFinish = false;
        //isCreating = true;
        //mSavedInstanceState = savedInstanceState;
        MyLogger.logD(CLASS_TAG, "onCreate");
        //requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        initFragments(mSavedInstanceState);
        setupActionBar(mSavedInstanceState);
        setProgressBarIndeterminateVisibility(false);
        registerSDCardListener();
        NotifyManager.getInstance(this).clearNotification();

    }

    @Override
    protected void onResume() {
        super.onResume();
        if (onCreateProcessed) {
            MyLogger.logD(CLASS_TAG, "onResume");
            if (checkSDCardStatus()) {
                String[] unsatisfiedPermissions = Utils.getUnsatisfiedPermissions(this);
                if (unsatisfiedPermissions != null && !mShouldFinish) {
                    MyLogger.logD(CLASS_TAG, "Request runtime permissions onResume: "
                        + unsatisfiedPermissions);

                    /*getFragmentManager().beginTransaction().
                    remove(getFragmentManager().findFragmentByTag(Constants.BACKUP)).commit();

                    getFragmentManager().beginTransaction().
                    remove(getFragmentManager().findFragmentByTag(Constants.RESTORE)).commit();*/


                    requestPermissionHandled = false;
                    requestPermissions(unsatisfiedPermissions, 0); // the request code is ignored
                    return;
                }
            }
            else {
                return;
            }
            continueOnResume();
        }
        //isCreating = false;
    }

    private void continueOnResume() {
        Intent actionIntent = getIntent();
        if (getIntent() != null) {
            ActionBar actionBar = getActionBar();
            String actionString = actionIntent.getAction();
            String actionWhere = actionIntent.getStringExtra(ACTION_WHERE);
            MyLogger.logD(CLASS_TAG, "actionWhere is " + actionWhere);
            if (actionString != null
                    && actionString.equals(NotifyManager.FP_NEW_DETECTION_INTENT_LIST)) {
                actionBar.selectTab(actionBar.getTabAt(actionBar.getTabCount() - 1));
                MyLogger.logD(CLASS_TAG, "New detected page here!");
                getIntent().setAction("");
            } else if (actionWhere != null && actionWhere.equals(ACTION_RESTORE)) {
                actionBar.selectTab(actionBar.getTabAt(actionBar.getTabCount() - 1));
                MyLogger.logD(CLASS_TAG, "Restore page here!");
            } else if (actionWhere != null && actionWhere.equals(ACTION_BACKUP)) {
                MyLogger.logD(CLASS_TAG, "Backup page here!");
                actionBar.selectTab(actionBar.getTabAt(0));
            }
            actionIntent.putExtra(ACTION_WHERE, "");

        } else {
            MyLogger.logE(CLASS_TAG, "Intent is null");
        }

    }

    @Override
    public void onRequestPermissionsResult(
            int requestCode,
            String[] permissions,
            int[] grantResult) {
        MyLogger.logD(CLASS_TAG, "onRequestPermissionsResult");
        mShouldFinish = false;
        for (int i = 0; i < grantResult.length; ++i) {
            if (grantResult[i] != PackageManager.PERMISSION_GRANTED) {
                MyLogger.logW(CLASS_TAG, "Permission " + permissions[i] + " not granted");
                mShouldFinish = true;
            }
        }
        if (mShouldFinish) {
            MyLogger.logW(CLASS_TAG, "Permission not satisified");
            Toast.makeText(
                    this,
                    R.string.permission_not_satisfied_exit,
                    Toast.LENGTH_SHORT).show();
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    Utils.exitLockTaskModeIfNeeded(MainActivity.this);
                    finish();
                }

            });
        }
        else {
            MyLogger.logD(CLASS_TAG, "onRequestPermissionsResult requestPermissionHandled = "
                + requestPermissionHandled);
            if (!requestPermissionHandled) {
                MyLogger.logD
                (CLASS_TAG, "onRequestPermissionsResult now do the onCreate and onResume");
                continueOnCreate();
                continueOnResume();
                requestPermissionHandled = true;
            }
        }
    }

    private Handler mHandler = new Handler();

    private boolean checkSDCardStatus() {
        if (SDCardUtils.getExternalStoragePath(this) == null) {
            MyLogger.logD(CLASS_TAG, "checkSDCardStatus before show toast");

            if (mToast != null) {
                mToast.cancel();
            }
            mToast = Toast.makeText(this, R.string.nosdcard_notice, Toast.LENGTH_SHORT);
            mToast.show();

            Utils.exitLockTaskModeIfNeeded(this);
            finish();
            return false;
        } else {
            //removeDialog(DialogID.DLG_NO_SDCARD);
            return true;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        MyLogger.logD(CLASS_TAG, "onDestroy");
        unRegisterSDCardListener();
        onCreateProcessed = false;
        if (isFinishing()) {
            MyLogger.logD(CLASS_TAG, "Anuj killMyProcess");
            Utils.killMyProcess();
            mFragmentsInitDone = false;
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        MyLogger.logD(CLASS_TAG, "onPause");
        //removeDialog(DialogID.DLG_NO_SDCARD);
    }

    protected void  onSaveInstanceState(Bundle outState) {
        ActionBar bar = getActionBar();
        int position = bar.getSelectedNavigationIndex();
        MyLogger.logD(CLASS_TAG, "onSaveInstanceState saved_position =  " + position);
        outState.putInt(STATE_TAB, position);
        super.onSaveInstanceState(outState);
    }

    private void initFragments(Bundle state) {
        FragmentManager fm = getFragmentManager();
        if (state != null) {
            mBackupFragment = fm.findFragmentByTag(Constants.BACKUP);
            //Anuj
            mRestoreFragment = fm.findFragmentByTag(Constants.RESTORE);
        }
        if (mBackupFragment == null) {
            mBackupFragment = new BackupTabFragment();
        }
        if (mRestoreFragment == null) {
            //Anuj
            mRestoreFragment = new RestoreTabFragment();
        }
        mFragmentsInitDone = true;
    }

    private void registerSDCardListener() {
        SDCardReceiver.getInstance().registerOnSDCardChangedListener(
                mMainOnSDCardStatusChangedListener);
    }

    private void unRegisterSDCardListener() {
        SDCardReceiver.getInstance().unRegisterOnSDCardChangedListener(
                mMainOnSDCardStatusChangedListener);
    }

    /**
     * @author mtk81330
     *
     */
    class MainOnSDCardStatusChangedListener implements
    SDCardReceiver.OnSDCardStatusChangedListener {

        @Override
        public void onSDCardStatusChanged(boolean mount) {
            MyLogger.logD(CLASS_TAG, "Anuj onSDCardStatusChanged from MainActivity- " + mount);
            if (!mount) {
                Toast.makeText(MainActivity.this, R.string.sd_removed, Toast.LENGTH_SHORT).show();
                finish();
            }
            //checkSDCardStatus();
        }
    }

    /**
     * @param newConfig Configuration
     */
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        MyLogger.logD(CLASS_TAG, "onConfigurationChanged");
        if (mFragmentsInitDone) {
            MyLogger.logD(CLASS_TAG, "onConfigurationChanged set fragments title");
            getActionBar().getTabAt(0).setText(R.string.tab_title_backup);
            getActionBar().getTabAt(1).setText(R.string.tab_title_restore);
        }
    }

    private void setupActionBar(Bundle state) {

        MyLogger.logD(CLASS_TAG, "setupActionBar");

        ActionBar actionBar = getActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
        actionBar.setDisplayOptions(0, ActionBar.DISPLAY_SHOW_HOME | ActionBar.DISPLAY_SHOW_TITLE);

        MyLogger.logD(CLASS_TAG, "setupActionBar tabs = " + actionBar.getTabCount());

        if (actionBar.getTabCount() == 0) {
            Tab tab = actionBar.newTab();
            tab.setText(R.string.tab_title_backup);
            tab.setTabListener(new TabListener(mBackupFragment, Constants.BACKUP));
            actionBar.addTab(tab);

            tab = actionBar.newTab();
            tab.setText(R.string.tab_title_restore);
            tab.setTabListener(new TabListener(mRestoreFragment, Constants.RESTORE));
            actionBar.addTab(tab);
        }

        if (state != null) {
            int position = state.getInt(STATE_TAB, 0);
            MyLogger.logD(CLASS_TAG, "In setupActionBar get_position = " + position);
            if (position >= 0) {
                actionBar.setSelectedNavigationItem(position);
            }
            /*else {
                actionBar.setSelectedNavigationItem(0);
            }*/
        }
        //setTabsMaxWidth();
        /*View action_bar_view = getActionBarView();
        View toolbar = ((ViewGroup)action_bar_view).getChildAt(0);
        View tab_container_view = ((ViewGroup)toolbar).getChildAt(0);
        int w = toolbar.getWidth();
        tab_container_view.getLayoutParams().width = w;*/
    }

    public View getActionBarView() {
        Window window = getWindow();
        View v = window.getDecorView();
        int resId = getResources().getIdentifier("action_bar_container", "id", "android");
        return v.findViewById(resId);
    }


    /*private void setTabsMaxWidth() {
       DisplayMetrics displaymetrics = new DisplayMetrics();
       getWindowManager().getDefaultDisplay().getMetrics(displaymetrics);
       int screenWidth = displaymetrics.widthPixels;
       final ActionBar actionBar = getActionBar();
       Tab tab = actionBar.getTabAt(0);
       //final View tabContainerView = (View) tabView.getParent();
       //final int tabPadding = tabContainerView.getPaddingLeft()
        + tabContainerView.getPaddingRight();
       //final int tabs = actionBar.getTabCount();
       //for(int i=0 ; i < tabs ; i++) {
          View backupTab = actionBar.getTabAt(0).getCustomView();
          //View nextChild = ((ViewGroup)tab).getChildAt(0);
          //TextView backupText = (TextView) nextChild;
          //backupText.setMaxWidth(screenWidth/2-1);

          View restoreTab = actionBar.getTabAt(1).getCustomView();
          TextView restoreText = (TextView) restoreTab.findViewById(R.string.tab_title_restore);
          restoreText.setMaxWidth(screenWidth/2-1);
      //}
    }*/


    /*protected Dialog onCreateDialog(int id, Bundle args) {

        MyLogger.logI(CLASS_TAG, "oncreateDialog, id = " + id);
        switch (id) {
            case DialogID.DLG_DELETE_AND_WAIT: {
                ProgressDialog dialog = new ProgressDialog(this);
                dialog.setCancelable(false);
                dialog.setMessage(getString(R.string.delete_please_wait));
                dialog.setIndeterminate(true);
                return dialog;
            }

            case DialogID.DLG_LOADING: {
                ProgressDialog dialog = new ProgressDialog(this);
                dialog.setCancelable(false);
                dialog.setMessage(getString(R.string.loading_please_wait));
                dialog.setIndeterminate(true);
                return dialog;
            }
            case DialogID.DLG_NO_SDCARD: {
                return new AlertDialog.Builder(this)
                        .setTitle(R.string.notice)
                        .setMessage(SDCardUtils.getSDStatueMessage(this))
                        .setCancelable(false)
                        .setPositiveButton(android.R.string.ok, new OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Utils.exitLockTaskModeIfNeeded(MainActivity.this);
                                MainActivity.this.finish();
                            }
                        }).create();
            }
            default: {
                MyLogger.logI(CLASS_TAG, "oncreateDialog, default");
            }
        }
        return null;
    }*/

    /**
     * A TabListener receives event callbacks from the action bar as tabs
     * are deselected, selected, and reselected. A FragmentTransaction
     * is provided to each of these callbacks; if any operations are added
     * to it, it will be committed at the end of the full tab switch operation.
     * This lets tab switches be atomic without the app needing to track
     * the interactions between different tabs.
     *
     * NOTE: This is a very simple implementation that does not retain
     * fragment state of the non-visible tabs across activity instances.
     * Look at the FragmentTabs example for how to do a more complete
     * implementation.
     */
    private class TabListener implements ActionBar.TabListener {
        private Fragment mFragment;
        private String mTag;


        public TabListener(Fragment fragment, String tag) {
            MyLogger.logD(CLASS_TAG, "TabListener");
            mFragment = fragment;
            mTag = tag;
        }

        public void onTabSelected(Tab tab, FragmentTransaction ft) {
            MyLogger.logD(CLASS_TAG, "onTabSelected");
            checkSDCardStatus();
            FragmentManager fm = getFragmentManager();
            Fragment f = fm.findFragmentByTag(mTag);
            if (f == null) {
                ft.add(android.R.id.content, mFragment, mTag);
            }
            //RestoreTabFragment restoreFragment = (RestoreTabFragment) mRestoreFragment;
            /*if (mFragment != null && (mFragment instanceof BackupTabFragment)
                    && mRestoreFragment != null && restoreFragment.getDeleteMode() != null) {
                restoreFragment.getDeleteMode().finish();
            }*/
        }

        public void onTabUnselected(Tab tab, FragmentTransaction ft) {
            MyLogger.logD(CLASS_TAG, "onTabUnselected");
            ft.remove(mFragment);
        }

        public void onTabReselected(Tab tab, FragmentTransaction ft) {
            //do nothing
        }
    }

}
