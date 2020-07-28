/*******************************************************************************
 *      Copyright (C) 2012 Google Inc.
 *      Licensed to The Android Open Source Project.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *           http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 *******************************************************************************/

package com.android.mail.ui;

import android.Manifest;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.Dialog;
import android.app.Fragment;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.nfc.NfcEvent;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.view.ActionMode;
import androidx.appcompat.widget.Toolbar;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.accessibility.AccessibilityManager;
import android.widget.Toast;

import com.android.bitmap.BitmapCache;
import com.android.bitmap.UnrefedBitmapCache;
import com.android.mail.R;
import com.android.mail.analytics.Analytics;
import com.android.mail.analytics.AnalyticsTimer;
import com.android.mail.bitmap.ContactResolver;
import com.android.mail.providers.Account;
import com.android.mail.providers.Folder;
import com.android.mail.providers.SearchRecentSuggestionsProvider;
import com.android.mail.providers.SuggestionsProvider;
import com.android.mail.utils.EmailConnectivityManager;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.Utils;
import com.android.mail.utils.ViewUtils;

import com.android.email.EmailApplication;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.HashSet;

/**
 * This is the root activity container that holds the left navigation fragment
 * (usually a list of folders), and the main content fragment (either a
 * conversation list or a conversation view).
 */
public class MailActivity extends AbstractMailActivity implements ControllableActivity {

    private static final String PERMISSION_READ_ATTACHMENT = "com.android.email.permission.READ_ATTACHMENT";
    private static final String PERMISSION_SEND_EMAIL = "com.mediatek.permission.CTA_SEND_EMAIL";
    private static final int GRANT_PERMISSION_REQUEST_CODE = 1;
    private boolean mIsChecking = false;

    /** 339KB cache fits 10 bitmaps at 33856 bytes each. */
    private static final int SENDERS_IMAGES_CACHE_TARGET_SIZE_BYTES = 1024 * 339;
    private static final float SENDERS_IMAGES_PREVIEWS_CACHE_NON_POOLED_FRACTION = 0f;
    /** Each string has upper estimate of 50 bytes, so this cache would be 5KB. */
    private static final int SENDERS_IMAGES_PREVIEWS_CACHE_NULL_CAPACITY = 100;

    /**
     * The activity controller to which we delegate most Activity lifecycle events.
     */
    private ActivityController mController;

    private ViewMode mViewMode;

    private ToastBarOperation mPendingToastOp;
    private boolean mAccessibilityEnabled;
    private AccessibilityManager mAccessibilityManager;

    protected ConversationListHelper mConversationListHelper;

    /**
     * The account name currently in use. Used to construct the NFC mailto: message. This needs
     * to be static since the {@link ComposeActivity} needs to statically change the account name
     * and have the NFC message changed accordingly.
     */
    protected static String sAccountName = null;

    private BitmapCache mSendersImageCache;

    private CustomViewToolbar mCustomViewToolbar;

    /** M: Support for "Smart Push" data collection @{ */
    // The flag to indicate whether the account opening should
    // be recorded
    public static boolean sRecordOpening = true;
    // The flag to indicate whether EmailActivity was resumed,
    // mainly used for marking that if the first UI was EmailActivity
    // after coming back from home screen
    public static boolean sEmailActivityResumed;
    /** @} */

    public boolean mPermissionGranted;

    /**
     * Create an NFC message (in the NDEF: Nfc Data Exchange Format) to instruct the recepient to
     * send an email to the current account.
     */
    private static class NdefMessageMaker implements NfcAdapter.CreateNdefMessageCallback {
        @Override
        public NdefMessage createNdefMessage(NfcEvent event) {
            if (sAccountName == null) {
                return null;
            }
            return getMailtoNdef(sAccountName);
        }

        /**
         * Returns an NDEF message with a single mailto URI record
         * for the given email address.
         */
        private static NdefMessage getMailtoNdef(String account) {
            byte[] accountBytes;
            try {
                accountBytes = URLEncoder.encode(account, "UTF-8").getBytes("UTF-8");
            } catch (UnsupportedEncodingException e) {
                accountBytes = account.getBytes();
            }
            byte prefix = 0x06; // mailto:
            byte[] recordBytes = new byte[accountBytes.length + 1];
            recordBytes[0] = prefix;
            System.arraycopy(accountBytes, 0, recordBytes, 1, accountBytes.length);
            NdefRecord mailto = new NdefRecord(NdefRecord.TNF_WELL_KNOWN, NdefRecord.RTD_URI,
                    new byte[0], recordBytes);
            return new NdefMessage(new NdefRecord[] { mailto });
        }
    }

    private final NdefMessageMaker mNdefHandler = new NdefMessageMaker();

    public MailActivity() {
        super();
        mConversationListHelper = new ConversationListHelper();
    }

    /**
     * M: Create an activity controller for this
     * @param activity
     * @param viewMode
     * @param tabletUi
     * @return
     */
    protected ActivityController createActivityController(MailActivity activity,
            ViewMode viewMode, boolean tabletUi) {
        return ControllerFactory.forActivity(this, mViewMode, tabletUi);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        mController.onTouchEvent(ev);
        return super.dispatchTouchEvent(ev);
    }

    /**
     * Default implementation returns a null view mode.
     */
    @Override
    public ViewMode getViewMode() {
        return mViewMode;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        LogUtils.v(LogTag.getLogTag(), "MA onActivityResult reqCode %d retCode %d", requestCode,
                resultCode);
        mController.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onBackPressed() {
        if (!mController.onBackPressed()) {
            super.onBackPressed();
        }
    }

    @Override
    public void onAttachFragment(Fragment frag) {
        LogUtils.d(LogTag.getLogTag(), "onAttachFragment");
       
        LogUtils.d(LogTag.getLogTag(), "onAttachFragment - Add Fragment to List");
        mController.addToFragmentList(frag);
            
        
    }
    
    @Override
    public void onCreate(Bundle savedState) {

        mViewMode = new ViewMode();
        final boolean tabletUi = Utils.useTabletUI(this.getResources());

        /// M: Create controller @{
        mController = createActivityController(this, mViewMode, tabletUi);
        /// @}

        
        super.onCreate(savedState);
        if (ActivityManager.isUserAMonkey()) {
            LogUtils.d(this.getLocalClassName(), "Finish activity as it is being started by monkey");
            finish();
        }
        // Log the start time if this is launched from the launcher with no saved states
        Intent i = getIntent();
        if (i != null && i.getCategories() != null &&
                i.getCategories().contains(Intent.CATEGORY_LAUNCHER)) {
            AnalyticsTimer.getInstance().trackStart(AnalyticsTimer.COLD_START_LAUNCHER);
        }

        resetSenderImageCache();
        
        
        setContentView(mController.getContentViewResource());

        final Toolbar toolbar = (Toolbar) findViewById(R.id.mail_toolbar);
        if (toolbar instanceof CustomViewToolbar) {
            // Tablets use CustomViewToolbar to override the default search menu item positioning.
            mCustomViewToolbar = (CustomViewToolbar) toolbar;
            mCustomViewToolbar.setController(this, mController, mViewMode);
            mController.addConversationListLayoutListener(mCustomViewToolbar);
        }

        setSupportActionBar(toolbar);
        toolbar.setNavigationOnClickListener(mController.getNavigationViewClickListener());

        final ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            // Hide the app icon.
            actionBar.setIcon(android.R.color.transparent);
            actionBar.setDisplayUseLogoEnabled(false);
        }

        /// M: for Android M runtime permission enhancement. @{
        
        mIsChecking = ((EmailApplication)getApplication()).getPermissionCheckingStateFlag();
        
        if (!mIsChecking && hasRequiredPermission(this, PERMISSIONS_LIST,
                GRANT_PERMISSION_REQUEST_CODE)) {
            mController.updatePermissionGrantState(true);
        } else {
            mController.updatePermissionGrantState(false);
        }
        /// @}

        // Must be done after setting up action bar
        mController.onCreate(mIsChecking ? null : savedState);

        mAccessibilityManager =
                (AccessibilityManager) getSystemService(Context.ACCESSIBILITY_SERVICE);
        mAccessibilityEnabled = mAccessibilityManager.isEnabled();
        final NfcAdapter nfcAdapter = NfcAdapter.getDefaultAdapter(this);
        if (nfcAdapter != null) {
            nfcAdapter.setNdefPushMessageCallback(mNdefHandler, this);
        }

        // Detect presence of hardware keyboard and log it on Analytics
        final int hardKeyboardHidden = getResources().getConfiguration().hardKeyboardHidden;
        if (hardKeyboardHidden == Configuration.HARDKEYBOARDHIDDEN_NO) {
            Analytics.getInstance().sendEvent("configuration", "keyboard", "use_hardware_keyboard",
                    0);
        } else {
            Analytics.getInstance().sendEvent("configuration", "keyboard",
                    "do_not_use_hardware_keyboard", 0);
        }
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);

        mController.onPostCreate(savedInstanceState);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mController.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        mController.onRestart();
    }

    /**
     * Constructs and sets the default NFC message. This message instructs the receiver to send
     * email to the account provided as the argument. This message is to be shared with
     * "zero-clicks" using NFC. The message will be available as long as the current activity is in
     * the foreground.
     *
     * @param account The email address to send mail to.
     */
    public static void setNfcMessage(String account) {
        sAccountName = account;
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        mIsChecking = ((EmailApplication)getApplication()).getPermissionCheckingStateFlag();
        if (!mIsChecking) {
        mController.onRestoreInstanceState(savedInstanceState);
    }
    }

    @Override
    public Dialog onCreateDialog(int id, Bundle bundle) {
        final Dialog dialog = mController.onCreateDialog(id, bundle);
        return dialog == null ? super.onCreateDialog(id, bundle) : dialog;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        return mController.onCreateOptionsMenu(menu) || super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return mController.onKeyDown(keyCode, event) || super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        return mController.onOptionsItemSelected(item) || super.onOptionsItemSelected(item);
    }

    @Override
    public void onPause() {
        super.onPause();
        mController.onPause();
    }

    @Override
    public void onPrepareDialog(int id, Dialog dialog, Bundle bundle) {
        super.onPrepareDialog(id, dialog, bundle);
        mController.onPrepareDialog(id, dialog, bundle);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        mController.onPrepareOptionsMenu(menu);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public void onResume() {
        super.onResume();
        mController.onResume();
        final boolean enabled = mAccessibilityManager.isEnabled();
        if (enabled != mAccessibilityEnabled) {
            onAccessibilityStateChanged(enabled);
        }
        /// M: no need to check, cause we do it in {@link MailIntentService}
        // App has resumed, re-check the top-level storage situation.
        //StorageLowState.checkStorageLowMode(this);
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mController.onSaveInstanceState(outState);
    }

    @Override
    protected void onStart() {
        super.onStart();
        mController.onStart();
    }

    @Override
    public boolean onSearchRequested() {
        mController.startSearch();
        return true;
    }

    @Override
    protected void onStop() {
        super.onStop();
        mController.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mController.onDestroy();

        if (mCustomViewToolbar != null) {
            mCustomViewToolbar.onDestroy();
        }
        ((EmailApplication)getApplication()).setPermissionCheckingStateFlag(false);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        mController.onWindowFocusChanged(hasFocus);
    }

    @Override
    public String toString() {
        final StringBuilder sb = new StringBuilder(super.toString());
        sb.append("{ViewMode=");
        sb.append(mViewMode);
        sb.append(" controller=");
        sb.append(mController);
        sb.append(" current_focus=");
        sb.append(getCurrentFocus());
        sb.append("}");
        return sb.toString();
    }

    @Override
    public ConversationListCallbacks getListHandler() {
        return mController;
    }

    @Override
    public FolderSelector getFolderSelector() {
        return mController;
    }

    @Override
    public FolderController getFolderController() {
        return mController;
    }

    @Override
    public ConversationCheckedSet getCheckedSet() {
        return mController.getCheckedSet();
    }

    @Override
    public void onUndoAvailable(ToastBarOperation undoOp) {
        mController.onUndoAvailable(undoOp);
    }

    @Override
    public Folder getHierarchyFolder() {
        return mController.getHierarchyFolder();
    }

    @Override
    public ConversationUpdater getConversationUpdater() {
        return mController;
    }

    @Override
    public ErrorListener getErrorListener() {
        return mController;
    }

    @Override
    public void setPendingToastOperation(ToastBarOperation op) {
        mPendingToastOp = op;
    }

    @Override
    public ToastBarOperation getPendingToastOperation() {
        return mPendingToastOp;
    }

    @Override
    public void onAnimationEnd(AnimatedAdapter animatedAdapter) {
        mController.onAnimationEnd(animatedAdapter);
    }

    @Override
    public AccountController getAccountController() {
        return mController;
    }

    @Override
    public RecentFolderController getRecentFolderController() {
        return mController;
    }

    @Override
    public DrawerController getDrawerController() {
        return mController.getDrawerController();
    }

    @Override
    public KeyboardNavigationController getKeyboardNavigationController() {
        return mController;
    }

    @Override
    public void onFooterViewLoadMoreClick(Folder folder) {
        mController.onFooterViewLoadMoreClick(folder);
    }

    /** M: add for local search*/
    @Override
    public void onFooterViewRemoteSearchClick(Folder folder) {
        mController.onFooterViewRemoteSearchClick(folder);
    }

    @Override
    public boolean isAccessibilityEnabled() {
        return mAccessibilityEnabled;
    }

    public void onAccessibilityStateChanged(boolean enabled) {
        mAccessibilityEnabled = enabled;
        mController.onAccessibilityStateChanged();
    }

    @Override
    public final ConversationListHelper getConversationListHelper() {
        return mConversationListHelper;
    }

    @Override
    public FragmentLauncher getFragmentLauncher() {
        return mController;
    }

    @Override
    public ContactLoaderCallbacks getContactLoaderCallbacks() {
        return new ContactLoaderCallbacks(getActivityContext());
    }

    @Override
    public ContactResolver getContactResolver(ContentResolver resolver, BitmapCache bitmapCache) {
        return new ContactResolver(resolver, bitmapCache);
    }

    @Override
    public BitmapCache getSenderImageCache() {
        return mSendersImageCache;
    }

    @Override
    public void resetSenderImageCache() {
        mSendersImageCache = createNewSenderImageCache();
    }

    private BitmapCache createNewSenderImageCache() {
        return new UnrefedBitmapCache(Utils.isLowRamDevice(this) ?
                0 : SENDERS_IMAGES_CACHE_TARGET_SIZE_BYTES,
                SENDERS_IMAGES_PREVIEWS_CACHE_NON_POOLED_FRACTION,
                SENDERS_IMAGES_PREVIEWS_CACHE_NULL_CAPACITY);
    }

    @Override
    public void showHelp(Account account, int viewMode) {
        int helpContext = ViewMode.isConversationMode(viewMode)
                ? R.string.conversation_view_help_context
                : R.string.conversation_list_help_context;
        Utils.showHelp(this, account, getString(helpContext));
    }

    public SearchRecentSuggestionsProvider getSuggestionsProvider() {
        return new SuggestionsProvider(this);
    }

    @Override
    public void onSupportActionModeStarted(ActionMode mode) {
        super.onSupportActionModeStarted(mode);
        ViewUtils.setStatusBarColor(this, R.color.action_mode_statusbar_color);
    }

    @Override
    public void onSupportActionModeFinished(ActionMode mode) {
        super.onSupportActionModeFinished(mode);
        ViewUtils.setStatusBarColor(this, R.color.mail_activity_status_bar_color);
    }

    @Override
    public EmailConnectivityManager getConnMgr() {
        /// M: get current EmailConnectivityManager of this Activity
        return mController.getConnMgr();
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        LogUtils.v(LogTag.getLogTag(), "MailActivity dispatchKeyEvent event: %s", event);
        if (KeyEvent.ACTION_UP == event.getAction() && event.isCtrlPressed() &&
                (keyCode == KeyEvent.KEYCODE_MINUS || keyCode == KeyEvent.KEYCODE_EQUALS)) {
            /** M: Dispatch keyevent to webview for Zoom in/out on SmartBook
             *  Keyevent Ctrl + "=" for Zoom in
             *  Keyevent Ctrl + "-" for Zoom out
             */
            return mController.dispatchKeyEvent(event);
        } else {
            return super.dispatchKeyEvent(event);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
            String[] permissions, int[] grantResults) {
        LogUtils.d(LogTag.getLogTag(), "Permission grant result for code: " + requestCode);
        if (requestCode == GRANT_PERMISSION_REQUEST_CODE) {
            if(grantResults.length == 0) {
                LogUtils.d(LogTag.getLogTag(), "Permission result ignored due to empty list");
                return;
            }
            boolean granted = true;
            mIsChecking = false;
            ((EmailApplication)getApplication()).setPermissionCheckingStateFlag(false);

            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    granted = false;
                }
            }
            if (granted) {
                mController.updatePermissionGrantState(true);
                //Restart Activity as Fragments could have been removed 
                recreate();
            } else {
                Toast.makeText(this, "You have disabled some required permissions.",
                        Toast.LENGTH_LONG).show();
                finishAndRemoveTask();
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions,
                    grantResults);
        }
    }

    public static final String[] PERMISSIONS_LIST = {
        Manifest.permission.READ_CONTACTS,
        Manifest.permission.WRITE_CONTACTS,
        Manifest.permission.READ_CALENDAR,
        Manifest.permission.WRITE_CALENDAR,
        Manifest.permission.READ_PHONE_STATE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.READ_EXTERNAL_STORAGE,
        PERMISSION_READ_ATTACHMENT,
        PERMISSION_SEND_EMAIL,
    };

    public static boolean hasRequiredPermission(final Activity activity,
            final String[] permissions, final int requestCode) {

        HashSet<String> absenceSet = new HashSet<String>();
        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(activity,
                    permission) != PackageManager.PERMISSION_GRANTED) {
                absenceSet.add(permission);
            }
        }

        if (!absenceSet.isEmpty()) {
            if (!((EmailApplication)activity.getApplication()).isDuplicateRequest()) {
            String[] requestArray = new String[absenceSet.size()];
            absenceSet.toArray(requestArray);
                LogUtils.d(LogTag.getLogTag(), "Request permission code: " + GRANT_PERMISSION_REQUEST_CODE);
                ((MailActivity)activity).mIsChecking = true;
                ((EmailApplication)activity.getApplication()).setPermissionCheckingStateFlag(true);
            ActivityCompat.requestPermissions(activity, requestArray,
                    GRANT_PERMISSION_REQUEST_CODE);
            } else {
                ((EmailApplication)activity.getApplication()).setDuplicateRequest(false);
            }
            return false;
        }
        return true;

    }
}
