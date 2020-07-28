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

import android.app.ListActivity;
import android.app.ActionBar;
import android.app.AlarmManager;
import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.SystemClock;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.support.v4.content.LocalBroadcastManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.cat.Item;
import com.android.internal.telephony.cat.Menu;
import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

//MTK add begin
import com.mediatek.internal.telephony.cat.MtkAppInterface;
import com.mediatek.internal.telephony.cat.MtkMenu;
import com.mediatek.internal.telephony.cat.MtkCatLog;

import com.mediatek.telephony.MtkTelephonyManagerEx;
//MTK add end

/**
 * ListActivity used for displaying STK menus. These can be SET UP MENU and
 * SELECT ITEM menus. This activity is started multiple times with different
 * menu content.
 *
 */
public class StkMenuActivity extends ListActivity implements View.OnCreateContextMenuListener {
    private Context mContext;
    private Menu mStkMenu = null;
    private int mState = STATE_MAIN;
    private boolean mAcceptUsersInput = true;
    private int mSlotId = -1;
    private boolean mIsResponseSent = false;
    // Determines whether this is in the pending state.
    private boolean mIsPending = false;

    private TextView mTitleTextView = null;
    private ImageView mTitleIconView = null;
    private ProgressBar mProgressView = null;

    private static final String className = new Object(){}.getClass().getEnclosingClass().getName();
    private static final String LOG_TAG = className.substring(className.lastIndexOf('.') + 1);

    private StkAppService appService = StkAppService.getInstance();

    //if broadcast receiver is registered already
    private boolean mIsRegisterReceiverDone = false;

    // Keys for saving the state of the dialog in the bundle
    private static final String STATE_KEY = "state";
    private static final String MENU_KEY = "menu";
    private static final String ACCEPT_USERS_INPUT_KEY = "accept_users_input";
    private static final String RESPONSE_SENT_KEY = "response_sent";
    private static final String ALARM_TIME_KEY = "alarm_time";
    private static final String PENDING = "pending";

    private static final String SELECT_ALARM_TAG = LOG_TAG;
    private static final long NO_SELECT_ALARM = -1;
    private long mAlarmTime = NO_SELECT_ALARM;

    // Internal state values
    static final int STATE_INIT = 0;
    static final int STATE_MAIN = 1;
    static final int STATE_SECONDARY = 2;

    // Finish result
    static final int FINISH_CAUSE_NORMAL = 1;
    static final int FINISH_CAUSE_NULL_SERVICE = 2;
    static final int FINISH_CAUSE_NULL_MENU = 3;
    static final int FINISH_CAUSE_FLIGHT_MODE = 4;
    static final int FINISH_CAUSE_NOT_AVAILABLE = 5;
    static final int FINISH_CAUSE_SIM_REMOVED = 6;

    private static final int CONTEXT_MENU_HELP = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MtkCatLog.d(LOG_TAG, "onCreate");

        ActionBar actionBar = getActionBar();
        actionBar.setCustomView(R.layout.stk_title);
        actionBar.setDisplayShowCustomEnabled(true);

        // Set the layout for this activity.
        setContentView(R.layout.stk_menu_list);
        mTitleTextView = (TextView) findViewById(R.id.title_text);
        mTitleIconView = (ImageView) findViewById(R.id.title_icon);
        mProgressView = (ProgressBar) findViewById(R.id.progress_bar);
        mContext = getBaseContext();
        mAcceptUsersInput = true;
        getListView().setOnCreateContextMenuListener(this);

        // appService can be null if this activity is automatically recreated by the system
        // with the saved instance state right after the phone process is killed.
        if (appService == null) {
            MtkCatLog.d(LOG_TAG, "onCreate - appService is null");
            finish();
            return;
        }

        IntentFilter intentFilter = new IntentFilter(StkAppService.SESSION_ENDED);
        intentFilter.addAction(StkAppService.ACTION_SIM_ABSENT);
        intentFilter.addAction(StkAppService.ACTION_ALL_SIM_ABSENT);
        LocalBroadcastManager.getInstance(this).registerReceiver(mLocalBroadcastReceiver,
                intentFilter);

        registerReceiver(mAirplaneModeReceiver, new IntentFilter(
                Intent.ACTION_AIRPLANE_MODE_CHANGED));

        mIsRegisterReceiverDone = true;

        initFromIntent(getIntent());
        if (!SubscriptionManager.isValidSlotIndex(mSlotId)) {
            MtkCatLog.d(LOG_TAG, "onCreate - inValid slotId");
            finish();
            return;
        }
        // Set a new task description to change icon
        if (StkAppService.isOP02  && PhoneConstants.SIM_ID_1 < mSlotId) {
            setTaskDescription(new ActivityManager.TaskDescription(null,
            BitmapFactory.decodeResource(getResources(),
            R.drawable.ic_launcher_sim2_toolkit)));
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);

        if (!mAcceptUsersInput) {
            MtkCatLog.d(LOG_TAG, "mAcceptUsersInput:false");
            return;
        }

        Item item = getSelectedItem(position);
        if (item == null) {
            MtkCatLog.d(LOG_TAG, "Item is null");
            return;
        }

        MtkCatLog.d(LOG_TAG, "onListItemClick Id: " + item.id + ", mState: " + mState);
        sendResponse(StkAppService.RES_ID_MENU_SELECTION, item.id, false);
        invalidateOptionsMenu();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        MtkCatLog.d(LOG_TAG, "mAcceptUsersInput: " + mAcceptUsersInput);
        if (!mAcceptUsersInput) {
            return true;
        }

        switch (keyCode) {
        case KeyEvent.KEYCODE_BACK:
            MtkCatLog.d(LOG_TAG, "KEYCODE_BACK - mState[" + mState + "]");
            switch (mState) {
            case STATE_SECONDARY:
                MtkCatLog.d(LOG_TAG, "STATE_SECONDARY");
                sendResponse(StkAppService.RES_ID_BACKWARD);
                return true;
            case STATE_MAIN:
                MtkCatLog.d(LOG_TAG, "STATE_MAIN");
                //MTK add begin
                //clear cached mainmene due to backkey on main menu
                appService.getStkContext(mSlotId).setMainActivityInstance(null);
                //MTK add end
                finish();
                return true;
            }
            break;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onResume() {
        super.onResume();

        MtkCatLog.d(LOG_TAG, "onResume, slot id: " + mSlotId + "," + mState);

        int res = onResumePreConditionCheck(mSlotId);
        if (res != FINISH_CAUSE_NORMAL) {
            MtkCatLog.w(LOG_TAG, "onResume get fail cause: " + res);
            showTextToast(getApplicationContext(), res);
            cancelTimeOut();
            if (mState == STATE_SECONDARY && !mIsResponseSent
                    && !appService.isMenuPending(mSlotId)) {
                sendResponse(StkAppService.RES_ID_END_SESSION);
            }
            finish();
            return;
        }
        appService.indicateMenuVisibility(true, mSlotId);
        if (mState == STATE_MAIN) {
            mStkMenu = appService.getMainMenu(mSlotId);
        } else {
            mStkMenu = appService.getMenu(mSlotId);
        }
        if (mStkMenu == null) {
            MtkCatLog.d(LOG_TAG, "menu is null");
            showTextToast(getApplicationContext(), FINISH_CAUSE_NULL_MENU);
            cancelTimeOut();
            finish();
            return;
        }
        //MTK add begin
        //save main menu for further use
        if (mState == STATE_MAIN) {
            MtkCatLog.v(LOG_TAG, "set main menu instance.");
            appService.getStkContext(mSlotId).setMainActivityInstance(this);
        }
        /*
        no need to reset mAcceptUsersInput to ture for main menu here,
        because if back to mainmenu and no following proactivit command,
        MD should send end session, and then reset mAcceptUsersInput by handling end session
        */
        //MTK add end

        displayMenu();

        // If the terminal has already sent response to the card when this activity is resumed,
        // keep this as a pending activity as this should be finished when the session ends.
        if (!mIsResponseSent) {
            setPendingState(false);
        }
        if (mAlarmTime == NO_SELECT_ALARM) {
            startTimeOut();
        }

        invalidateOptionsMenu();
    }

    @Override
    public void onPause() {
        super.onPause();
        MtkCatLog.d(LOG_TAG, "onPause, slot id: " + mSlotId + "," + mState);
        //If activity is finished in onResume and it reaults from null appService.
        if (appService != null) {
            appService.indicateMenuVisibility(false, mSlotId);
        } else {
            MtkCatLog.d(LOG_TAG, "onPause: null appService.");
        }

        /*
         * do not cancel the timer here cancelTimeOut(). If any higher/lower
         * priority events such as incoming call, new sms, screen off intent,
         * notification alerts, user actions such as 'User moving to another activtiy'
         * etc.. occur during SELECT ITEM ongoing session,
         * this activity would receive 'onPause()' event resulting in
         * cancellation of the timer. As a result no terminal response is
         * sent to the card.
         */

    }

    @Override
    public void onStop() {
        super.onStop();
        MtkCatLog.d(LOG_TAG, "onStop, slot id: " + mSlotId + "," + mIsResponseSent + "," + mState);

        if (null == appService || null == appService.getStkContext(mSlotId)) {
            MtkCatLog.w(LOG_TAG, "null appService or StkContex");
            return;
        }

        // Nothing should be done here if this activity is being restarted now.
        if (isFinishing() || isChangingConfigurations()) {
            MtkCatLog.w(LOG_TAG, "isFinishing || isChangingConfigurations");
            return;
        }

        if (mIsResponseSent) {
            // It is unnecessary to keep this activity if the response was already sent and
            // the dialog activity is NOT on the top of this activity.
            if (mState == STATE_SECONDARY && !appService.isStkDialogActivated()) {
                finish();
            }
        } else {
            // This instance should be registered as the pending activity here
            // only when no response has been sent back to the card.
            setPendingState(true);
        }
    }

    @Override
    public void onDestroy() {
        getListView().setOnCreateContextMenuListener(null);
        super.onDestroy();
        MtkCatLog.d(LOG_TAG, "onDestroy" + ", " + mState);
        if (appService == null || !SubscriptionManager.isValidSlotIndex(mSlotId)) {
            return;
        }
        cancelTimeOut();

        //isMenuPending: if input act is finish by stkappservice when OP_LAUNCH_APP again,
        //we can not send TR here, since the input cmd is waiting user to process.
        if (mState == STATE_SECONDARY && !mIsResponseSent && !appService.isMenuPending(mSlotId)) {
            // Avoid sending the terminal response while the activty is being restarted
            // due to some kind of configuration change.
            if (!isChangingConfigurations()) {
                MtkCatLog.d(LOG_TAG, "handleDestroy - Send End Session");
                sendResponse(StkAppService.RES_ID_END_SESSION);
            }
        }
        //MTK add begin
        //if this is main menu, and was saved in mMainActivityInstance before
        //then clear mMainActivityInstance as it is destroyed
        if (mState == STATE_MAIN && this ==
                appService.getStkContext(mSlotId).getMainActivityInstance()) {
            appService.getStkContext(mSlotId).setMainActivityInstance(null);
        }
        //MTK add end

        if (mIsRegisterReceiverDone) {
            unregisterReceiver(mAirplaneModeReceiver);
            LocalBroadcastManager.getInstance(this).unregisterReceiver(mLocalBroadcastReceiver);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(android.view.Menu menu) {
        super.onCreateOptionsMenu(menu);
        menu.add(0, StkApp.MENU_ID_END_SESSION, 1, R.string.menu_end_session);
        menu.add(0, StkApp.MENU_ID_DEFAULT_ITEM, 2, R.string.help);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(android.view.Menu menu) {
        super.onPrepareOptionsMenu(menu);
        boolean mainVisible = false;
        boolean defaultItemVisible = false;
        if (mState == STATE_SECONDARY && mAcceptUsersInput) {
            mainVisible = true;
        }

        menu.findItem(StkApp.MENU_ID_END_SESSION).setVisible(mainVisible);

        // for defaut item
        if (mStkMenu != null && mStkMenu.items.size() > 0) {
            Item item = getDefaultItem();
            if (item != null) {
                MtkCatLog.v(LOG_TAG, "item: " + item);
            }
            if(item == null || item.text == null || item.text.length() == 0 ) {
                MtkCatLog.v(LOG_TAG, "Set visible of default item to false.");
                menu.findItem(StkApp.MENU_ID_DEFAULT_ITEM).setVisible(false);
            } else {
                MtkCatLog.v(LOG_TAG, "Set visible of default item to true.");
                defaultItemVisible = true;
                menu.findItem(StkApp.MENU_ID_DEFAULT_ITEM).setTitle(item.text);
                menu.findItem(StkApp.MENU_ID_DEFAULT_ITEM).setVisible(true);
            }
        } else {
            MtkCatLog.d(LOG_TAG, "no menu or item, Set visible of default item to false");
            menu.findItem(StkApp.MENU_ID_DEFAULT_ITEM).setVisible(false);
        }

        return mainVisible || defaultItemVisible;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (!mAcceptUsersInput) {
            return true;
        }
        switch (item.getItemId()) {
        case StkApp.MENU_ID_END_SESSION:
            // send session end response.
            sendResponse(StkAppService.RES_ID_END_SESSION);
            finish();
            return true;
        case StkApp.MENU_ID_DEFAULT_ITEM:
            if (mStkMenu != null) {
                Item defaultItem = getDefaultItem();
                if (defaultItem == null) {
                    return true;
                }
                if (mState == STATE_SECONDARY) {
                    appService.getStkContext(mSlotId).setPendingActivityInstance(this);
                }
                sendResponse(StkAppService.RES_ID_MENU_SELECTION, defaultItem.id,
                        false);
                mAcceptUsersInput = false;
                mProgressView.setVisibility(View.VISIBLE);
                mProgressView.setIndeterminate(true);
            }
            return true;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {
        MtkCatLog.d(this, "onCreateContextMenu");
        boolean helpVisible = false;
        if (mStkMenu != null) {
            helpVisible = mStkMenu.helpAvailable;
        }
        if (helpVisible) {
            MtkCatLog.d(this, "add menu");
            menu.add(0, CONTEXT_MENU_HELP, 0, R.string.help);
        }
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        AdapterView.AdapterContextMenuInfo info;
        try {
            info = (AdapterView.AdapterContextMenuInfo) item.getMenuInfo();
        } catch (ClassCastException e) {
            return false;
        }
        switch (item.getItemId()) {
            case CONTEXT_MENU_HELP:
                int position = info.position;
                MtkCatLog.d(this, "Position:" + position);
                Item stkItem = getSelectedItem(position);
                if (stkItem != null) {
                    MtkCatLog.d(this, "item id:" + stkItem.id);
                    sendResponse(StkAppService.RES_ID_MENU_SELECTION, stkItem.id, true);
                } else {
                    MtkCatLog.d(this, "error, item is null");
                }
                return true;

            default:
                return super.onContextItemSelected(item);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        MtkCatLog.d(LOG_TAG, "onSaveInstanceState: " + mSlotId);
        outState.putInt(STATE_KEY, mState);
        outState.putParcelable(MENU_KEY, mStkMenu);
        outState.putBoolean(ACCEPT_USERS_INPUT_KEY, mAcceptUsersInput);
        outState.putBoolean(RESPONSE_SENT_KEY, mIsResponseSent);
        outState.putLong(ALARM_TIME_KEY, mAlarmTime);
        outState.putBoolean(PENDING, mIsPending);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        MtkCatLog.d(LOG_TAG, "onRestoreInstanceState: " + mSlotId);
        mState = savedInstanceState.getInt(STATE_KEY);
        if(StkAppService.mAppStatus == StkAppService.APP_STATUS_FOCE_KILLED) {
            // Phone reset, restart APP from MainActivity
            MtkCatLog.w(LOG_TAG, "onRestoreInstanceState: Phone reset before!! mState = "
                    + mState);
            if(mState == STATE_MAIN) {
                String clsName = "com.android.stk.StkMain";
                if(StkAppService.isOP02) {
                    if(mSlotId == 0) {
                        clsName = "com.android.stk.StkLauncherActivityI";
                    } else {
                        clsName = "com.android.stk.StkLauncherActivityII";
                    }
                }
                Intent mainIntent = new Intent();
                mainIntent.setClassName(this, clsName);
                MtkCatLog.w(LOG_TAG, "onRestoreInstanceState: Launch STK from main");
                startActivity(mainIntent);
            }
            finish();
        }
        mStkMenu = savedInstanceState.getParcelable(MENU_KEY);
        mAcceptUsersInput = savedInstanceState.getBoolean(ACCEPT_USERS_INPUT_KEY);
        if (!mAcceptUsersInput) {
            // Check the latest information as the saved instance state can be outdated.
            if ((mState == STATE_MAIN) && appService.isMainMenuAvailable(mSlotId)) {
                mAcceptUsersInput = true;
            } else {
                showProgressBar(true);
            }
        }
        mIsResponseSent = savedInstanceState.getBoolean(RESPONSE_SENT_KEY);

        mAlarmTime = savedInstanceState.getLong(ALARM_TIME_KEY, NO_SELECT_ALARM);
        if (mAlarmTime != NO_SELECT_ALARM) {
            startTimeOut();
        }

        if (!mIsResponseSent && !savedInstanceState.getBoolean(PENDING)) {
            // If this is in the foreground and no response has been sent to the card,
            // this must not be registered as pending activity by the previous instance.
            // No need to renew nor clear pending activity in this case.
        } else {
            // Renew the instance of the pending activity.
            setPendingState(true);
        }
    }

    private void setPendingState(boolean on) {
        if (mState == STATE_SECONDARY) {
            if (mIsPending != on) {
                appService.getStkContext(mSlotId).setPendingActivityInstance(on ? this : null);
                mIsPending = on;
            }
        }
    }

    private void cancelTimeOut() {
        if (mAlarmTime != NO_SELECT_ALARM) {
            MtkCatLog.d(LOG_TAG, "cancelTimeOut - slot id: " + mSlotId);
            AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            am.cancel(mAlarmListener);
            mAlarmTime = NO_SELECT_ALARM;
        }
    }

    private void startTimeOut() {
        // No need to set alarm if this is the main menu or device sent TERMINAL RESPONSE already.
        if (mState != STATE_SECONDARY || mIsResponseSent) {
            return;
        }

        if (mAlarmTime == NO_SELECT_ALARM) {
            mAlarmTime = SystemClock.elapsedRealtime() + StkApp.UI_TIMEOUT;
        }

        MtkCatLog.d(LOG_TAG, "startTimeOut: " + mAlarmTime + "ms, slot id: " + mSlotId);
        AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        am.setExact(AlarmManager.ELAPSED_REALTIME_WAKEUP, mAlarmTime, SELECT_ALARM_TAG,
                mAlarmListener, null);
    }

    // Bind list adapter to the items list.
    private void displayMenu() {

        if (mStkMenu != null) {
            String title;

            if (mStkMenu.title != null) {
                MtkCatLog.d(LOG_TAG, "mStkMenu.title: " + mStkMenu.title);
            }

            if (mStkMenu.title == null) {
                int resId = R.string.app_name;
                title = getString(resId);
            } else {
                title = mStkMenu.title;
            }
            // Display title & title icon
            if (mStkMenu.titleIcon != null) {
                mTitleIconView.setImageBitmap(mStkMenu.titleIcon);
                mTitleIconView.setVisibility(View.VISIBLE);
                mTitleTextView.setVisibility(View.INVISIBLE);
                if (!mStkMenu.titleIconSelfExplanatory) {
                    mTitleTextView.setText(title);
                    mTitleTextView.setVisibility(View.VISIBLE);
                }
            } else {
                mTitleIconView.setVisibility(View.GONE);
                mTitleTextView.setVisibility(View.VISIBLE);
                mTitleTextView.setText(title);
            }
            //MTK add begin
            for (int i = 0; i < mStkMenu.items.size();) {
                if (mStkMenu.items.get(i) == null) {
                    mStkMenu.items.remove(i);
                    MtkCatLog.v(LOG_TAG, "Remove null item from menu.items");
                    continue;
                }
                ++i;
            }

            if (mStkMenu.items.size() == 0) {
                MtkCatLog.v(LOG_TAG, "should not display the SET_UP_MENU because no item");
                return;
            }
            //MTK add end

            // create an array adapter for the menu list

            StkMenuAdapter adapter = new StkMenuAdapter(this,
                    ((MtkMenu)mStkMenu).items,
                    ((MtkMenu)mStkMenu).nextActionIndicator,
                    ((MtkMenu)mStkMenu).itemsIconSelfExplanatory);

            // Bind menu list to the new adapter.
            setListAdapter(adapter);
            // Set default item
            setSelection(mStkMenu.defaultItem);
        }
    }

    private void showProgressBar(boolean show) {
        if (show) {
            mProgressView.setIndeterminate(true);
            mProgressView.setVisibility(View.VISIBLE);
        } else {
            mProgressView.setIndeterminate(false);
            mProgressView.setVisibility(View.GONE);
        }
    }

    private void initFromIntent(Intent intent) {

        if (intent != null) {
            mState = intent.getIntExtra("STATE", STATE_MAIN);
            mSlotId = intent.getIntExtra(StkAppService.SLOT_ID, -1);
            MtkCatLog.d(LOG_TAG, "slot id: " + mSlotId + ", state: " + mState);
        } else {
            MtkCatLog.d(LOG_TAG, "init intent null, finish!");
            finish();
        }
    }

    private Item getSelectedItem(int position) {
        Item item = null;
        if (mStkMenu != null) {
            try {
                item = mStkMenu.items.get(position);
            } catch (IndexOutOfBoundsException e) {
                if (StkApp.DBG) {
                    MtkCatLog.d(LOG_TAG, "IOOBE Invalid menu");
                }
            } catch (NullPointerException e) {
                if (StkApp.DBG) {
                    MtkCatLog.d(LOG_TAG, "NPE Invalid menu");
                }
            }
        }
        return item;
    }

    private Item getDefaultItem() {
        Item item = null;
        if (mStkMenu != null) {
            for(int i = 0; i < mStkMenu.items.size(); ++i)
            {
                item = mStkMenu.items.get(i);
                if (item.id == mStkMenu.defaultItem)
                    break;
            }
        }
        return item;
    }

    private void sendResponse(int resId) {
        sendResponse(resId, 0, false);
    }

    private void sendResponse(int resId, int itemId, boolean help) {
        MtkCatLog.d(LOG_TAG, "sendResponse resID[" + resId + "] itemId[" + itemId +
            "] help[" + help + "]");

        // Disallow user operation temporarily until receiving the result of the response.
        mAcceptUsersInput = false;
        if (resId == StkAppService.RES_ID_MENU_SELECTION) {
            showProgressBar(true);
        }
        cancelTimeOut();

        mIsResponseSent = true;
        Bundle args = new Bundle();
        args.putInt(StkAppService.RES_ID, resId);
        args.putInt(StkAppService.MENU_SELECTION, itemId);
        args.putBoolean(StkAppService.HELP, help);
        appService.sendResponse(args, mSlotId);

        // This instance should be set as a pending activity and finished by the service.
        if (resId != StkAppService.RES_ID_END_SESSION) {
            setPendingState(true);
        }
    }

    private final BroadcastReceiver mLocalBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (StkAppService.SESSION_ENDED.equals(intent.getAction())) {
                int slotId = intent.getIntExtra(StkAppService.SLOT_ID, 0);
                if ((mState == STATE_MAIN) && (mSlotId == slotId)) {
                    mAcceptUsersInput = true;
                    showProgressBar(false);
                }
            } else if (StkAppService.ACTION_SIM_ABSENT.equals(intent.getAction())) {
                int slotId = intent.getIntExtra(StkAppService.SLOT_ID,0);
                MtkCatLog.d(this, "ACTION_SIM_ABSENT, slotId: " + slotId);
                if (slotId == mSlotId) {
                    cancelTimeOut();
                    mIsResponseSent = true;
                    finish();
                }
            } else if (StkAppService.ACTION_ALL_SIM_ABSENT.equals(intent.getAction())) {
                MtkCatLog.d(this, "ACTION_ALL_SIM_ABSENT");
                cancelTimeOut();
                mIsResponseSent = true;
                finish();
            }
        }
    };

    private final AlarmManager.OnAlarmListener mAlarmListener =
            new AlarmManager.OnAlarmListener() {
                @Override
                public void onAlarm() {
                    CatLog.d(LOG_TAG, "The alarm time is reached");
                    mAlarmTime = NO_SELECT_ALARM;
                    sendResponse(StkAppService.RES_ID_TIMEOUT);
                }
            };

    /// M:  @{
    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        MtkCatLog.v(LOG_TAG, "[onNewIntent]");
        mAcceptUsersInput = true;
        mIsResponseSent = false;
        initFromIntent(intent);
    }

    private int onResumePreConditionCheck(int slodId) {
        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(
                Context.TELEPHONY_SERVICE);

        if (isOnFlightMode() == true) {
            MtkCatLog.v(LOG_TAG, "flight mode - don't make stk be visible");
            return FINISH_CAUSE_FLIGHT_MODE;
        } else if (tm.hasIccCard(slodId) == false) {
            MtkCatLog.v(LOG_TAG, "SIM card was removed");
            return FINISH_CAUSE_SIM_REMOVED;
        } else if (isOnLockMode(slodId) == true || checkSimRadioState(slodId) == false) {
            MtkCatLog.v(LOG_TAG, "radio off or sim off - don't make stk be visible");
            return FINISH_CAUSE_NOT_AVAILABLE;
        } else if (appService == null) {
            MtkCatLog.v(LOG_TAG, "can not launch stk menu 'cause null StkAppService");
            return FINISH_CAUSE_NULL_SERVICE;
        }
        return FINISH_CAUSE_NORMAL;
    }


    private boolean isOnFlightMode() {
        int mode = 0;
        try {
            mode = Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON);
        } catch (SettingNotFoundException e) {
            MtkCatLog.w(LOG_TAG, "fail to get airlane mode");
            mode = 0;
        }

        MtkCatLog.v(LOG_TAG, "airlane mode is " + mode);
        return (mode != 0);
    }

    private boolean isRadioOnState(int slotId) {
        boolean radioOn = true;
        try {
            ITelephony phone = ITelephony.Stub.asInterface(
                    ServiceManager.getService(Context.TELEPHONY_SERVICE));
            if (phone != null) {
                int subId[] = SubscriptionManager.getSubId(slotId);
                radioOn = phone.isRadioOnForSubscriber(subId[0],
                    getApplicationContext().getOpPackageName());
            }
            MtkCatLog.v(LOG_TAG, "slotId"+ slotId + "isRadioOnState - radio_on[" + radioOn + "]");
        } catch (RemoteException e) {
            e.printStackTrace();
            MtkCatLog.w(LOG_TAG, "isRadioOnState - Exception happen ====");
        }
        return radioOn;
    }

    private boolean isSimOnState(int slotId) {
        boolean simOn = (MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId)
                == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON );
        MtkCatLog.d(LOG_TAG, "isSimOnState - slotId[" + slotId + "], sim_on[" + simOn + "]");

        return simOn;
    }

    private boolean checkSimRadioState(int slotId) {
        boolean isSimOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        MtkCatLog.d(LOG_TAG, "checkSimRadioState - slotId[" + slotId +
                "], isSimOnOffEnabled[" + isSimOnOffEnabled + "]");
        if (isSimOnOffEnabled) {
            return isSimOnState(slotId);
        } else {
            return isRadioOnState(slotId);
        }
    }

    private boolean isOnLockMode(int slotId) {
        int simState = TelephonyManager.getDefault().getSimState(slotId);
        MtkCatLog.v(LOG_TAG, "lock mode is " + simState);
        if (TelephonyManager.SIM_STATE_PIN_REQUIRED == simState ||
                TelephonyManager.SIM_STATE_PUK_REQUIRED == simState ||
                TelephonyManager.SIM_STATE_NETWORK_LOCKED == simState) {
            return true;
        } else {
            return false;
        }
    }

    private void showTextToast(Context context, int cause) {
        String msg = null;
        switch (cause) {
            case FINISH_CAUSE_FLIGHT_MODE:
                msg = getString(R.string.lable_on_flight_mode);
                break;
            case FINISH_CAUSE_NOT_AVAILABLE:
                msg = getString(R.string.lable_sim_not_ready);
                break;
            case FINISH_CAUSE_SIM_REMOVED:
                msg = getString(R.string.no_sim_card_inserted);
                break;
            case FINISH_CAUSE_NULL_MENU:
                msg = getString(R.string.main_menu_not_initialized);
                break;
            case FINISH_CAUSE_NULL_SERVICE:
            default:
                return;
        }
        Toast toast = Toast.makeText(context, msg, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }

    private BroadcastReceiver mAirplaneModeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            boolean airplaneModeEnabled = isOnFlightMode();
            StkAppInstaller appInstaller = StkAppInstaller.getInstance();
            MtkCatLog.v(LOG_TAG, "mAirplaneModeReceiver AIRPLANE_MODE_CHANGED: "
                    + airplaneModeEnabled);
            if (airplaneModeEnabled) {
                mIsResponseSent = true;
                cancelTimeOut();
                finish();
            }
        }
    };

    /// @}
}
