/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.android.browser;

import android.Manifest;
import android.app.Activity;
import android.app.KeyguardManager;
import android.content.BroadcastReceiver;
/// M: add for save page
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.PowerManager;
import android.util.Log;
import android.view.ActionMode;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.WebView;

/// M: add for runtime permission check @{
import android.content.pm.PackageManager;
import android.widget.Toast;
import java.util.List;
import java.util.ArrayList;
import com.android.browser.PermissionHelper.PermissionCallback;
/// @}



import com.android.browser.stub.NullController;
/// M: add for save page
import com.android.browser.provider.SnapshotProvider.Snapshots;
import com.google.common.annotations.VisibleForTesting;

import com.mediatek.common.carrierexpress.CarrierExpressManager;

public class BrowserActivity extends Activity {

    public static final String ACTION_SHOW_BOOKMARKS = "show_bookmarks";
    public static final String ACTION_SHOW_BROWSER = "show_browser";
    public static final String ACTION_RESTART = "--restart--";
    private static final String EXTRA_STATE = "state";
    public static final String EXTRA_DISABLE_URL_OVERRIDE = "disable_url_override";

    private final static String LOGTAG = "browser";
    private static final String XLOGTAG = "browser/BrowserActivity";

    private final static boolean LOGV_ENABLED = Browser.LOGV_ENABLED;
    private static final boolean DEBUG = Browser.ENGONLY;

    private ActivityController mController = NullController.INSTANCE;
    private UI mUi;
    private IntentFilter mIntentFilter;
    private static final String ACTION_SEARCH_SEARCH_ENGINE_CHANGED =
        "com.mediatek.search.SEARCH_ENGINE_CHANGED";
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (DEBUG) {
                    Log.v("@M_" + LOGTAG, "mReceiver action = " + action);
                }
                if (action.equals(ACTION_SEARCH_SEARCH_ENGINE_CHANGED)) {
                    handleSearchEngineChanged();
                }
                if (action.equals(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED)) {
                    Extensions.resetPlugins();
                    Extensions.getRegionalPhonePlugin(context)
                            .updateBookmarks(context);
                }
            }
    };
    private void handleSearchEngineChanged() {
        String searchEngineName = BrowserSettings.getInstance().getSearchEngineName();
        if (DEBUG) {
            Log.v("@M_" + LOGTAG, "ChangeSearchEngineReceiver (search): "
                + BrowserSettings.PREF_SEARCH_ENGINE
                + "---" + searchEngineName);
        }
    }

    /// M: add for save page @ {
    private static final String DELETE_WHERE
                                = Snapshots.PROGRESS + " < ? AND " + Snapshots.IS_DONE + " = ?";
    private static final String[] DELETE_WHERE_ARGS = {"100", "0"};
    private class DeleteFailedDownload implements Runnable {
        public void run() {
            ContentResolver cr = getContentResolver();
            cr.delete(Snapshots.CONTENT_URI, DELETE_WHERE, DELETE_WHERE_ARGS);
        }
    }
    /// @ }

    // / M: add for runtime permission check @{
    private boolean mAllGranted;
    // / @ }

    @Override
    protected void onStart() {
        super.onStart();
    }
    @Override
    protected void onStop() {
        mController.onPause();
        super.onStop();
    }

    @Override
    public void onCreate(Bundle icicle) {
        if (DEBUG) {
            Log.v(LOGTAG, this + " onCreate, has state: "
                    + (icicle == null ? "false" : "true"));
        }
        super.onCreate(icicle);
        mAllGranted = false;
        PermissionHelper.init(this);

        /// M: reset soft input mode for tablet(issue ALPS00447558) @{
        if (isTablet(this)) {
            getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        }
        /// @}
        handleSearchEngineChanged();
        mIntentFilter = new IntentFilter(ACTION_SEARCH_SEARCH_ENGINE_CHANGED);
        mIntentFilter.addAction(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED);
        registerReceiver(mReceiver, mIntentFilter);
        if (shouldIgnoreIntents()) {
            finish();
            return;
        }

        // If this was a web search request, pass it on to the default web
        // search provider and finish this activity.
        if (IntentHandler.handleWebSearchIntent(this, null, getIntent())) {
            finish();
            return;
        }
        mController = createController();

        Intent intent = (icicle == null) ? getIntent() : null;
        mController.start(intent);
        /// M: add for save page @ {
        Thread deleteFailDownload = new Thread(new DeleteFailedDownload());
        deleteFailDownload.start();
        /// @ }
    }

    public static boolean isTablet(Context context) {
        return context.getResources().getBoolean(R.bool.isTablet);
    }

    private Controller createController() {
        Controller controller = new Controller(this);
        boolean xlarge = isTablet(this);
        UI ui = null;
        if (xlarge) {
            ui = new XLargeUi(this, controller);
        } else {
            ui = new PhoneUi(this, controller);
        }
        controller.setUi(ui);
        return controller;
    }

    @VisibleForTesting
    Controller getController() {
        return (Controller) mController;
    }

    @Override
    protected void onNewIntent(Intent intent) {
        if (shouldIgnoreIntents()) return;
        /// M: intent of Action_Search will be handled in handleWebSearch() of onCreate(),
        ///and a new intent of Action_View and search engine URI will be sent.
        ///call finish() after sending new intent.
        ///The new intent will be received by BrowserActivity.
        ///But the finish() haven't finished, so new intent will be handled by onNewIntent()
        ///instead of onCreate().
        ///That's not right. @{
        if (mController == NullController.INSTANCE) {
            Log.w(XLOGTAG, "onNewIntent for Action_Search Intent reached before finish(), " +
                    "so enter onNewIntent instead of on create");
            startActivity(intent);
            finish();
            return;
        }
        /// @}

        if (ACTION_RESTART.equals(intent.getAction())) {
            Bundle outState = new Bundle();
            mController.onSaveInstanceState(outState);
            finish();
            getApplicationContext().startActivity(
                    new Intent(getApplicationContext(), BrowserActivity.class)
                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                    .putExtra(EXTRA_STATE, outState));
            return;
        }
        mController.handleNewIntent(intent);
    }

    private KeyguardManager mKeyguardManager;
    private PowerManager mPowerManager;
    private boolean shouldIgnoreIntents() {
        // Only process intents if the screen is on and the device is unlocked
        // aka, if we will be user-visible
        if (mKeyguardManager == null) {
            mKeyguardManager = (KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE);
        }
        if (mPowerManager == null) {
            mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        }
        boolean ignore = !mPowerManager.isScreenOn();
        /// M: remove keyguard check for multi-user and HotKnot.
        Log.v(LOGTAG, "ignore intents: " + ignore);
        return ignore;
    }

    @Override
    protected void onResume() {
        super.onResume();
        /// M: add for runtime permission check @{
        if (mAllGranted) {
            doResume();
        } else {
            List<String> permissionsRequestList = PermissionHelper.getInstance()
                    .getAllUngrantedPermissions();
            if (permissionsRequestList.size() > 0) {
                PermissionHelper.getInstance().requestPermissions(permissionsRequestList,
                        mPermissionCallback);
            } else {
                doResume();
            }
        }
        /// @}
    }

    private void doResume() {
        if (DEBUG) {
            Log.v(LOGTAG, "BrowserActivity.onResume: this=" + this);
        }
        mController.onResume();
    }

    @Override
    public boolean onMenuOpened(int featureId, Menu menu) {
        if (Window.FEATURE_OPTIONS_PANEL == featureId) {
            mController.onMenuOpened(featureId, menu);
        }
        return true;
    }

    @Override
    public void onOptionsMenuClosed(Menu menu) {
        mController.onOptionsMenuClosed(menu);
    }

    @Override
    public void onContextMenuClosed(Menu menu) {
        super.onContextMenuClosed(menu);
        mController.onContextMenuClosed(menu);
    }

    /**
     *  onSaveInstanceState(Bundle map)
     *  onSaveInstanceState is called right before onStop(). The map contains
     *  the saved state.
     */
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        if (LOGV_ENABLED) {
            Log.v(LOGTAG, "BrowserActivity.onSaveInstanceState: this=" + this);
        }
        mController.onSaveInstanceState(outState);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        if (DEBUG) {
            Log.v(LOGTAG, "BrowserActivity.onDestroy: this=" + this);
        }
        unregisterReceiver(mReceiver);
        super.onDestroy();
        mController.onDestroy();
        mController = NullController.INSTANCE;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mController.onConfgurationChanged(newConfig);
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
        mController.onLowMemory();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        return mController.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        return mController.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (!mController.onOptionsItemSelected(item)) {
            return super.onOptionsItemSelected(item);
        }
        return true;
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {
        mController.onCreateContextMenu(menu, v, menuInfo);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        return mController.onContextItemSelected(item);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return mController.onKeyDown(keyCode, event) ||
            super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event) {
        return mController.onKeyLongPress(keyCode, event) ||
            super.onKeyLongPress(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return mController.onKeyUp(keyCode, event) ||
            super.onKeyUp(keyCode, event);
    }

    @Override
    public void onActionModeStarted(ActionMode mode) {
        super.onActionModeStarted(mode);
        mController.onActionModeStarted(mode);
    }

    @Override
    public void onActionModeFinished(ActionMode mode) {
        super.onActionModeFinished(mode);
        mController.onActionModeFinished(mode);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode,
            Intent intent) {
        mController.onActivityResult(requestCode, resultCode, intent);
    }

    @Override
    public boolean onSearchRequested() {
        return mController.onSearchRequested();
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        return mController.dispatchKeyEvent(event)
                || super.dispatchKeyEvent(event);
    }

    @Override
    public boolean dispatchKeyShortcutEvent(KeyEvent event) {
        return mController.dispatchKeyShortcutEvent(event)
                || super.dispatchKeyShortcutEvent(event);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        return mController.dispatchTouchEvent(ev)
                || super.dispatchTouchEvent(ev);
    }

    @Override
    public boolean dispatchTrackballEvent(MotionEvent ev) {
        return mController.dispatchTrackballEvent(ev)
                || super.dispatchTrackballEvent(ev);
    }

    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent ev) {
        return mController.dispatchGenericMotionEvent(ev) ||
                super.dispatchGenericMotionEvent(ev);
    }

    // / M: add for runtime permission check @{
    @Override
    public void onRequestPermissionsResult(int requestCode,
            String[] permissions, int[] grantResults) {
        if (DEBUG) {
            Log.d(XLOGTAG, " onRequestPermissionsResult " + requestCode);
        }
        PermissionHelper.getInstance().onPermissionsResult(requestCode, permissions, grantResults);
    }

    private PermissionHelper.PermissionCallback mPermissionCallback =
            new PermissionHelper.PermissionCallback() {
        public void onPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
            if (grantResults != null && grantResults.length > 0) {
                mAllGranted = true;
                for (int i = 0; i < grantResults.length; i++) {
                    if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                        mAllGranted = false;
                        if (DEBUG) {
                            Log.d(XLOGTAG, permissions[i] + " is not granted !");
                        }
                        break;
                    }
                }
                if (!mAllGranted) {
                    String toastStr =
                            getString(R.string.denied_required_permission);
                    Toast.makeText(getApplicationContext(), toastStr, Toast.LENGTH_LONG).show();
                    finish();
                }
                doResume();
            }
        }
    };

    /// @}
 /// M: add for ALPS03021283 @{
    public void onMultiWindowModeChanged(boolean isInMultiWindowMode){
        if(!isInMultiWindowMode&&isTablet(this)){
           final WindowManager.LayoutParams lp = getWindow().getAttributes();
              if(lp!=null){
                 lp.flags |= WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED;
                 lp.flags &= WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS;
                 getWindow().setAttributes(lp);
                 Log.d(LOGTAG, "BrowserActivity.onMultiWindowModeChanged");
           }
        }
     }
/// @}
}

