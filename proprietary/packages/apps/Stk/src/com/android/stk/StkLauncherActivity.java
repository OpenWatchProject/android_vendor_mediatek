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

import android.app.ActionBar;
import android.app.ListActivity;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.support.v4.content.LocalBroadcastManager;
import android.view.Gravity;
import android.view.View;
import android.view.KeyEvent;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.cat.Item;
import com.android.internal.telephony.cat.Menu;
import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;


import android.telephony.TelephonyManager;
import com.android.internal.telephony.SubscriptionController;

import java.util.ArrayList;

import com.mediatek.internal.telephony.cat.MtkCatLog;
import com.mediatek.telephony.MtkTelephonyManagerEx;


/**
 * Launcher class. Serve as the app's MAIN activity, send an intent to the
 * StkAppService and finish.
 *
 */
public class StkLauncherActivity extends ListActivity {
    private TextView mTitleTextView = null;
    private ImageView mTitleIconView = null;
    private static final String className = new Object(){}.getClass().getEnclosingClass().getName();
    private static final String LOG_TAG = className.substring(className.lastIndexOf('.') + 1);
    private ArrayList<Item> mStkMenuList = null;
    private int mSingleSimId = -1;
    private Context mContext = null;
    private TelephonyManager mTm = null;
    private Bitmap mBitMap = null;
    /// M:  @{
    //private boolean mAcceptUsersInput = true;
    private boolean mIsRegisterReceiverDone = false;
    private boolean mStkMainVisible = false;
    /// @}
    static final boolean isOP154
            = "OP154".equalsIgnoreCase(SystemProperties.get("persist.vendor.operator.optr", ""));

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        MtkCatLog.d(LOG_TAG, "onCreate+");
        mContext = getBaseContext();
        mTm = (TelephonyManager) mContext.getSystemService(
                Context.TELEPHONY_SERVICE);

        StkAppService.mIsLauncherAcceptInput = true;


        if (isShowSTKListMenu()) {
            ActionBar actionBar = getActionBar();
            actionBar.setCustomView(R.layout.stk_title);
            actionBar.setDisplayShowCustomEnabled(true);

            setContentView(R.layout.stk_menu_list);
            mTitleTextView = (TextView) findViewById(R.id.title_text);
            mTitleIconView = (ImageView) findViewById(R.id.title_icon);
            mTitleTextView.setText(R.string.app_name);
            mBitMap = BitmapFactory.decodeResource(getResources(),
                    R.drawable.ic_launcher_sim_toolkit);
        } else {
            if (mSingleSimId < 0) {
                showTextToast(mContext, R.string.no_sim_card_inserted);
                finish();
                return;
            }
        }

        //MTK add begin
        mContext.registerReceiver(mBroadcastReceiver, new IntentFilter(
                TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED));
        IntentFilter intentFilter = new IntentFilter(StkAppService.ACTION_SIM_ABSENT);
        intentFilter.addAction(StkAppService.ACTION_ALL_SIM_ABSENT);
        LocalBroadcastManager.getInstance(this).registerReceiver(mLocalBroadcastReceiver,
                intentFilter);
        mIsRegisterReceiverDone = true;
        //MTK add end
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
        if (!StkAppService.mIsLauncherAcceptInput) {
            MtkCatLog.d(LOG_TAG, "mAcceptUsersInput:false");
            return;
        }
        int simCount = TelephonyManager.from(mContext).getSimCount();
        Item item = getSelectedItem(position);
        if (item == null) {
            MtkCatLog.d(LOG_TAG, "Item is null");
            return;
        }
        MtkCatLog.d(LOG_TAG, "launch stk menu id: " + item.id);
        if (item.id >= PhoneConstants.SIM_ID_1 && item.id < simCount) {
            StkAppService.mIsLauncherAcceptInput = false;
            launchSTKMainMenu(item.id);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        MtkCatLog.d(LOG_TAG, "mAcceptUsersInput: " + StkAppService.mIsLauncherAcceptInput);
        if (!StkAppService.mIsLauncherAcceptInput) {
            return true;
        }
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                MtkCatLog.d(LOG_TAG, "KEYCODE_BACK.");
                StkAppService.mIsLauncherAcceptInput = false;
                finish();
                return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onResume() {
        super.onResume();
        MtkCatLog.d(LOG_TAG, "onResume");
        StkAppService.mIsLauncherAcceptInput = true;
        mStkMainVisible = true;
        int itemSize = addStkMenuListItems();
        if (itemSize == 0) {
            MtkCatLog.d(LOG_TAG, "item size = 0 so finish.");
            showTextToast(mContext, R.string.lable_sim_not_ready);
            finish();
        } else if (itemSize == 1) {
            launchSTKMainMenu(mSingleSimId);
            finish();
        } else {
            MtkCatLog.d(LOG_TAG, "resume to show multiple stk list.");
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        MtkCatLog.d(LOG_TAG, "onPause");
        mStkMainVisible = false;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        MtkCatLog.d(LOG_TAG, "onDestroy");
        if (mIsRegisterReceiverDone) {
            unregisterReceiver(mBroadcastReceiver);
            LocalBroadcastManager.getInstance(this).unregisterReceiver(mLocalBroadcastReceiver);
        }
    }

    private Item getSelectedItem(int position) {
        Item item = null;
        if (mStkMenuList != null) {
            try {
                item = mStkMenuList.get(position);
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

    private int addStkMenuListItems() {
        StkAppService appService = StkAppService.getInstance();
        if (appService == null) {
            return 0;
        }

        String appName = mContext.getResources().getString(R.string.app_name);
        String stkItemName = null;
        int simCount = TelephonyManager.from(mContext).getSimCount();
        mStkMenuList = new ArrayList<Item>();

        MtkCatLog.d(LOG_TAG, "addStkMenuListItems, simCount: " + simCount);
        for (int i = 0; i < simCount; i++) {
            // Check if the card is inserted.
            if (mTm.hasIccCard(i)) {
                Menu menu = appService.getMainMenu(i);
                // Check if the card has a main menu.
                if (menu != null) {
                    MtkCatLog.d(LOG_TAG, "SIM #" + (i + 1) + " is add to menu.");
                    mSingleSimId = i;
                    //if get menu title, no need add index.
                    if (menu.title != null) {
                        stkItemName = new StringBuilder(menu.title).toString();
                    } else {
                        stkItemName = new StringBuilder(appName)
                                .append(" ").append(Integer.toString(i + 1)).toString();
                    }
                    // Display the default application icon if there is no icon specified by SET-UP
                    // MENU command nor preset.
                    Bitmap icon = mBitMap;
                    if (menu.titleIcon != null) {
                        icon = menu.titleIcon;
                        if (menu.titleIconSelfExplanatory) {
                            stkItemName = null;
                        }
                    }
                    Item item = new Item(i, stkItemName, icon);
                    mStkMenuList.add(item);
                } else {
                    MtkCatLog.d(LOG_TAG, "SIM #" + (i + 1) + " does not have main menu.");
                }
            } else {
                MtkCatLog.d(LOG_TAG, "SIM #" + (i + 1) + " is not inserted.");
            }
        }
        if (mStkMenuList != null && mStkMenuList.size() > 0) {
            if (mStkMenuList.size() > 1) {
                StkMenuAdapter adapter = new StkMenuAdapter(this,
                        mStkMenuList, null, false);
                // Bind menu list to the new adapter.
                this.setListAdapter(adapter);
            }
            return mStkMenuList.size();
        } else {
            MtkCatLog.d(LOG_TAG, "No stk menu item add.");
            return 0;
        }
    }
    private void launchSTKMainMenu(int slotId) {
        Bundle args = new Bundle();
        MtkCatLog.d(LOG_TAG, "launchSTKMainMenu.");
        if (!isStkAvailable(slotId)) {
            finish();
            return;
        }
        args.putInt(StkAppService.OPCODE, StkAppService.OP_LAUNCH_APP);
        args.putInt(StkAppService.SLOT_ID
                , PhoneConstants.SIM_ID_1 + slotId);
        startService(new Intent(this, StkAppService.class)
                .putExtras(args));
    }

    /// M:  @{
    private boolean isShowSTKListMenu() {
        int simCount = TelephonyManager.from(mContext).getSimCount();
        int simInsertedCount = 0;
        int insertedSlotId = -1;

        MtkCatLog.v(LOG_TAG, "simCount: " + simCount);
        for (int i = 0; i < simCount; i++) {
            //Check if the card is inserted.
            if (mTm.hasIccCard(i)) {
                MtkCatLog.v(LOG_TAG, "SIM " + i + " is inserted.");
                mSingleSimId = i;
                simInsertedCount++;
            } else {
                MtkCatLog.v(LOG_TAG, "SIM " + i + " is not inserted.");
            }
        }
        if (simInsertedCount > 1) {
            return true;
        } else {
            //No card or only one card.
            MtkCatLog.w(LOG_TAG, "do not show stk list menu.");
            return false;
        }
    }

    private boolean isStkAvailable(int slotId) {
        int resId = R.string.lable_sim_not_ready;
        boolean isWfcEnabled = isWifiCallingAvailable(slotId);

        if (true == isOnFlightMode()) {
            MtkCatLog.d(LOG_TAG, "isOnFlightMode");
            if( isOP154 && isWfcEnabled ){
                MtkCatLog.d(LOG_TAG, "OP154, wfc enabled");
                return true;
            }else{
                resId = R.string.lable_on_flight_mode;
                showTextToast(mContext, resId);
                return false;
            }
        } else if (true == isOnLockMode(slotId) ||
                false == checkSimRadioState(slotId)) {
            MtkCatLog.d(LOG_TAG, "isOnLockMode or radio off or sim off");
            showTextToast(mContext, resId);
            return false;
        }

        StkAppService service = StkAppService.getInstance();

        if (service != null && service.StkQueryAvailable(slotId) !=
                StkAppService.STK_AVAIL_AVAILABLE) {
            int simState = TelephonyManager.getDefault().getSimState(slotId);

            MtkCatLog.d(LOG_TAG, "slotId: " + slotId + "is not available simState:" + simState);
            showTextToast(mContext, resId);
            return false;
        }
        return true;
    }

    private void showTextToast(Context context, int resId) {
        Toast toast = Toast.makeText(context, resId, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }

    private boolean isOnFlightMode() {
        int mode = 0;
        try {
            mode = Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON);
        } catch (SettingNotFoundException e) {
            MtkCatLog.e(LOG_TAG, "fail to get airlane mode");
            mode = 0;
        }

        MtkCatLog.v(LOG_TAG, "airlane mode is " + mode);
        return (mode != 0);
    }
    private boolean isRadioOnState(int slotId) {
        boolean radioOn = true;
        MtkCatLog.v(LOG_TAG, "isRadioOnState check = " + slotId);

        try {
            ITelephony phone = ITelephony.Stub.asInterface(
                    ServiceManager.getService(Context.TELEPHONY_SERVICE));
            if (phone != null) {
                int subId[] = SubscriptionManager.getSubId(slotId);
                radioOn = phone.isRadioOnForSubscriber(subId[0], mContext.getOpPackageName());
            }
            MtkCatLog.v(LOG_TAG, "isRadioOnState - radio_on[" + radioOn + "]");
        } catch (RemoteException e) {
            e.printStackTrace();
            MtkCatLog.e(LOG_TAG, "isRadioOnState - Exception happen ====");
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

    private boolean isWifiCallingAvailable(int phoneId) {
        final MtkTelephonyManagerEx tm = MtkTelephonyManagerEx.getDefault();
        int subId = getSubIdUsingPhoneId(phoneId);
        return tm.isWifiCallingEnabled(subId);
    }

    private int getSubIdUsingPhoneId(int phoneId) {
        SubscriptionController subCon = SubscriptionController.getInstance();
        int subId = (subCon != null)?
                subCon.getSubIdUsingPhoneId(phoneId) : SubscriptionManager.INVALID_SUBSCRIPTION_ID;

        MtkCatLog.d(LOG_TAG, "[getSubIdUsingPhoneId] subId " + subId + ", phoneId " + phoneId);
        return subId;
    }

    boolean isOnLockMode(int slotId) {
        int simState = TelephonyManager.getDefault().getSimState(slotId);
        MtkCatLog.d(LOG_TAG, "lock mode is " + simState);
        if (TelephonyManager.SIM_STATE_PIN_REQUIRED == simState ||
                TelephonyManager.SIM_STATE_PUK_REQUIRED == simState ||
                TelephonyManager.SIM_STATE_NETWORK_LOCKED == simState) {
            return true;
        } else {
            return false;
        }
    }

    private final BroadcastReceiver mLocalBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
             if (StkAppService.ACTION_SIM_ABSENT.equals(intent.getAction())) {
                int slotId = intent.getIntExtra(StkAppService.SLOT_ID,0);
                MtkCatLog.d(this, "ACTION_SIM_ABSENT, slotId: " + slotId);
                int itemSize = addStkMenuListItems();
                if (itemSize == 0) {
                    finish();
                } else if (itemSize == 1) {
                    if (true == mStkMainVisible) {
                        launchSTKMainMenu(mSingleSimId);
                    }
                    finish();
                }
            } else if (StkAppService.ACTION_ALL_SIM_ABSENT.equals(intent.getAction())) {
                MtkCatLog.d(this, "ACTION_ALL_SIM_ABSENT, finish activity");
                finish();
            }
        }
    };

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null){
                return;
            }
            MtkCatLog.d(LOG_TAG, "onReceive, action=" + action);
            if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                addStkMenuListItems();
            }
        }
    };
    /// @}
}
