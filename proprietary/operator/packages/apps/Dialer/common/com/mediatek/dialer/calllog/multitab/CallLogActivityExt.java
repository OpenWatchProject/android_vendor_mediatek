package com.mediatek.dialer.calllog.multitab;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.provider.CallLog.Calls;
import android.support.v4.view.ViewPager;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.style.ImageSpan;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuItem.OnMenuItemClickListener;
import android.widget.HorizontalScrollView;
import android.view.LayoutInflater;

import com.android.dialer.app.calllog.CallLogActivity;
import com.android.dialer.app.calllog.CallLogFragment;
import com.android.dialer.calllogutils.CallTypeIconsView;

import com.mediatek.dialer.compat.CallLogCompat.CallsCompat;
import com.mediatek.dialer.ext.DefaultCallLogActivityExt;

import com.mediatek.dialer.common.plugin.R;
import com.mediatek.dialer.calllog.accountfilter.FilterOptions;
import com.mediatek.dialer.calllog.accountfilter.PhoneAccountUtils;
import com.mediatek.dialer.calllog.accountfilter.PhoneAccountPickerActivity;
import com.mediatek.dialer.calllog.accountfilter.PhoneAccountInfoHelper;

public class CallLogActivityExt extends DefaultCallLogActivityExt {
    private static final String TAG = "CallLogActivityExt ";

    private PhoneAccountInfoHelper mPhoneAccountInfoHelper;
    private Activity mActivity;
    private Context mPluginContext;

    public CallLogActivityExt(Activity callLogActivity, Context pluginContext) {
      mActivity = callLogActivity;
      mPluginContext = pluginContext;
      Log.d(TAG, "CallLogActivityExt");

      mPhoneAccountInfoHelper = PhoneAccountInfoHelper.getInstance(mActivity);
      mPhoneAccountInfoHelper.registerSimChangeReceiver();
    }

    public static final int PHONE_ACCOUNT_FILTER_MENU_ID = 0;

    private static final int NOTICE_ID = 0x10ffffff;
    private static final int ACCOUNT_MENU_ID = 0x2000;

    private static final int TAB_INDEX_ALL = 0;
    private static final int TAB_INDEX_MISSED = 1;
    private static final int TAB_INDEX_INCOMING = 2;
    private static final int TAB_INDEX_OUTGOING = 3;
    private static final int MAX_TAB_COUNT = 4;

    CallLogFragment mIncomingCallFragment;
    CallLogFragment mOutgoingCallFragment;
    CallLogFragment mMissedCallFragment;
    CallLogFragment mAllCallFragment;
    private int mCurrentIndex = -1;

    /**
     * for op02
     * called when host create menu, to add plug-in own menu here
     * @param menu menu
     * @param viewPagerTabs the ViewPagerTabs used in activity
     */
    @Override
    public void createCallLogMenu(Menu menu, HorizontalScrollView viewPagerTabs) {
        Log.d(TAG, "createCallLogMenu");
        MenuItem accountFilterMenuItem = menu.add(Menu.NONE, PHONE_ACCOUNT_FILTER_MENU_ID,
            ACCOUNT_MENU_ID, mPluginContext.getText(R.string.select_account));
        //accountFilterMenuItem.setIcon(mPluginContext.getDrawable(R.drawable.ic_account_chooser));
        //accountFilterMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        accountFilterMenuItem.setOnMenuItemClickListener(
            new OnMenuItemClickListener() {
            @Override
            public boolean onMenuItemClick(MenuItem item) {
                Log.d(TAG, "accountFilterMenu onMenuItemClick");
                Intent intent = new Intent(mPluginContext, PhoneAccountPickerActivity.class);
                intent.putExtra(FilterOptions.FILTER_ACCOUNT_PREFER,
                        mPhoneAccountInfoHelper.getPreferAccountId());
                mActivity.startActivityForResult(intent, FilterOptions.ACTIVITY_REQUEST_CODE);
                return true;
            }
        });
    }
    /// @}

    /**
     * for op02
     * called when host prepare menu, prepare plug-in own menu here
     * @param menu the Menu Created
     * @param fragment the current fragment
     * @param itemDeleteAll the optionsmenu delete all item
     * @param adapterCount adapterCount
     */
    public void prepareCallLogMenu(Menu menu) {
        Log.d(TAG, "prepareCallLogMenu");
        /// M :[Call Log Account Filter] @{
        // hide choose account menu if only one or no account
        final MenuItem itemChooseAccount = menu.findItem(PHONE_ACCOUNT_FILTER_MENU_ID);
        if (itemChooseAccount != null) {
            itemChooseAccount.setVisible(PhoneAccountUtils
                    .hasMultipleCallCapableAccounts(mPluginContext));
        } else {
            if (itemChooseAccount != null) {
                itemChooseAccount.setVisible(false);
            }
        }
        /// @}
    }

    /**
     * for op02
     * Get Tab Index Count
     * @return int
     */
    @Override
    public int getTabIndexCount(int defCount) {
        Log.d(TAG, "getTabIndexCount defCount:" + defCount + "+2");
        return defCount + 2;
    }

    /**
     * for op02
     * Init call Log tab
     * @param tabTitles tabTitles
     * @param viewPager viewPager
     */
    @Override
    public void initCallLogTab(CharSequence[] tabTitles, ViewPager viewPager) {
        Log.d(TAG, "initCallLogTab");
        Log.d(TAG, "initCallLogRes");
        CallTypeIconsView.Resources res = new CallTypeIconsView.Resources(mActivity, false);
        if (res != null) {
            tabTitles[1] = createSpannableString(res.missed);
            tabTitles[2] = createSpannableString(res.incoming);
            tabTitles[3] = createSpannableString(res.outgoing);
        }
        viewPager.setOffscreenPageLimit(3);
    }

    /**
      * for op02
      * Get the CallLog Item
      * @param position position
      * @return Fragment
      */
    @Override
    public Fragment getCallLogFragmentItem(int position) {
        Log.d(TAG, "getCallLogFragmentItem position:" + position);
        if (position == TAB_INDEX_INCOMING) {
            return new CallLogFragment(Calls.INCOMING_TYPE, true);
        } else if (position == TAB_INDEX_OUTGOING) {
            return new CallLogFragment(Calls.OUTGOING_TYPE, true);
        }
        return null;
    }

    /**
     * for op02
     * Instantiate CallLog Item
     * @param position position
     * @param fragment fragment
     * @return Object if plugin init the item return the fragment, or else return null
     */
    @Override
    public boolean instantiateItemByOp(int position, Fragment fragment) {
        Log.d(TAG, "instantiateItemByOpNa");

        if (position == TAB_INDEX_MISSED) {
            mMissedCallFragment = (CallLogFragment) fragment;
            return false;
        } else if (position == TAB_INDEX_INCOMING) {
            mIncomingCallFragment = (CallLogFragment) fragment;
            return true;
        } else if (position == TAB_INDEX_OUTGOING) {
            mOutgoingCallFragment = (CallLogFragment) fragment;
            return true;
        } else if (position == TAB_INDEX_ALL) {
            mAllCallFragment = (CallLogFragment) fragment;
            return false;
        } else {
            return false;
        }
    }

    @Override
    public boolean updateMissedCalls(int index) {
        Log.d(TAG, "updateMissedCalls index:" + index);
        boolean ret = false;
        switch (index) {
            case TAB_INDEX_INCOMING:
                if (mIncomingCallFragment != null) {
                    mIncomingCallFragment.markMissedCallsAsReadAndRemoveNotifications();
                }
                ret = true;
                break;
            case TAB_INDEX_OUTGOING:
                if (mOutgoingCallFragment != null) {
                    mOutgoingCallFragment.markMissedCallsAsReadAndRemoveNotifications();
                }
                ret = true;
                break;
        }
        return ret;
    }

    /**.
     * for op02
     * plug-in manage the state and unregister receiver
     * @param activity the current activity
     */
    @Override
    public void onDestroy() {
        mPhoneAccountInfoHelper.unregisterSimChangeReceiver();
    }


    private SpannableString createSpannableString(Drawable drawable) {
        drawable.setBounds(0, 0, (drawable.getIntrinsicWidth() * 3) / 2,
                (drawable.getIntrinsicHeight() * 3) / 2);
        SpannableString sp = new SpannableString("i");
        ImageSpan iconsp = new ImageSpan(drawable, ImageSpan.ALIGN_BOTTOM);
        sp.setSpan(iconsp, 0, 1, Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
        return sp;
    }

    /**
     * for op02
     * plug-in handle Activity Result
     * @param requestCode requestCode
     * @param resultCode resultCode
     * @param data the intent return by setResult
     */
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(TAG, "onActivityResult requestCode,,resultCode,,data = " +
                requestCode + ",," + resultCode + ",," + data);
        if (requestCode == FilterOptions.ACTIVITY_REQUEST_CODE) {
            if (resultCode == FilterOptions.ACTIVITY_RESULT_CODE) {
                Bundle bundle = data.getExtras();
                String selectedId = bundle.getString(FilterOptions.SELECTED_ID);
                String perAccount = mPhoneAccountInfoHelper.getPreferAccountId();
                if (selectedId != null && (!perAccount.equals(selectedId))) {
                    Log.d(TAG, "onActivityResult setPreferAccountId: " + selectedId);
                    mPhoneAccountInfoHelper.setPreferAccountId(selectedId);
                }
            } else {
                Log.e(TAG, "onActivityResult failed");
            }
        }
    }
}
