package com.mediatek.dialer.calllog.multitab;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.Bundle;
import android.provider.CallLog.Calls;
import android.telecom.PhoneAccountHandle;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.widget.TextView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.android.dialer.app.calllog.CallLogFragment;
import com.mediatek.dialer.calllog.accountfilter.PhoneAccountUtils;
import com.mediatek.dialer.calllog.accountfilter.PhoneAccountInfoHelper;
import com.mediatek.dialer.ext.DefaultCallLogFragmentExt;
import com.mediatek.dialer.common.plugin.R;
import java.util.function.Consumer;
/*
 * this ext is only for callLogFragment contained in CallLogActivity.
 */
public class CallLogFragmentExt extends DefaultCallLogFragmentExt
    implements SharedPreferences.OnSharedPreferenceChangeListener{
    private static final String TAG = "CallLogFragmentExt ";

    private Context mPluginContext;
    private Context mHostContext;
    private static final int NOTICE_ID = 0x10ffffff;

    private int mCallTypeFilter;
    private boolean mIsCallLogActivity;
    private Fragment mFragment;
    private Consumer mForceFresh;

    public CallLogFragmentExt(Fragment fragment, boolean isCallLogActivity,
            int callTypeFilter, Context pluginContext, Consumer<Boolean> forceFresh) {
        mFragment = fragment;
        mCallTypeFilter = callTypeFilter;
        mIsCallLogActivity = isCallLogActivity;
        mPluginContext = pluginContext;
        mHostContext = mFragment.getActivity();
        mForceFresh = forceFresh;

        Log.d(TAG, "new CallLogFragmentExt  mIsCallLogActivity:" + mIsCallLogActivity);
        PhoneAccountInfoHelper.getInstance(mHostContext)
        .registerSharedPreferenceChangeListener(this);
    }

    public void onResume() {
        Log.d(TAG, "onResume");
        updateNotice();
    }

    /**.
     * for op02
     * plug-in manage the state and unregister receiver
     * @param activity the current activity
     */
    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy unRegisterSharedPreferenceChangeListener ");
        PhoneAccountInfoHelper.getInstance(mHostContext)
        .unRegisterSharedPreferenceChangeListener(this);
    }

    /**
     * for op02
     * plug-in to create account info in calllog fragment
     * @param fragment the current fragment
     * @param view the current view
     */
    @Override
    public void onViewCreated(View view) {
        Activity activity = mFragment.getActivity();
        Resources resource = activity.getResources();
        String packageName = activity.getPackageName();
        View recyclerView = view.findViewById(resource.getIdentifier("recycler_view",
                "id", packageName));

        if (recyclerView == null) {
            Log.d(TAG, "onViewCreated, recyclerView is null");
            return;
        }

        ViewGroup group = (ViewGroup) recyclerView.getParent();
        if (group == null) {
            Log.d(TAG, "onViewCreated, recyclerView parent is null");
            return;
        }

        addViewContent(group);
    }

    private void addViewContent(ViewGroup groupParent) {
        Log.d(TAG, "addViewContent groupParent:" + groupParent);
        LayoutInflater inflater;
        inflater = LayoutInflater.from(mPluginContext);
        ViewGroup noticeLayout =
                (ViewGroup) inflater.inflate(R.layout.account_notice_indicator, groupParent, false);
        noticeLayout.setId(NOTICE_ID);
        groupParent.addView(noticeLayout, 1);
    }

    private void updateNotice() {
        Log.d(TAG, "updateNotice");
        View view = mFragment.getView();
        ViewGroup viewChild = (ViewGroup) view.findViewById(NOTICE_ID);
        if (viewChild == null) {
            return;
        }

        TextView noticeText = (TextView) viewChild.getChildAt(0);
        View noticeTextDivider = (View) viewChild.getChildAt(1);
        String lable = null;
        int color = -1;
        String id = PhoneAccountInfoHelper.getPreferAccountId();
        if (mHostContext != null && !PhoneAccountInfoHelper.FILTER_ALL_ACCOUNT_ID.equals(id)) {
            PhoneAccountHandle account = PhoneAccountUtils.getPhoneAccountById(mHostContext, id);
            if (account != null) {
                lable = PhoneAccountUtils.getAccountLabel(mHostContext, account);
                color = PhoneAccountUtils.getAccountColor(mHostContext, account);
            }
        }
        Log.d(TAG, "updateNotice, lable = " + lable + ", id = " + id);
        if (!TextUtils.isEmpty(lable) && noticeText != null && noticeTextDivider != null) {
            String noticeString = mPluginContext.getResources().getString(
                    R.string.call_log_via_sim_name_notice, lable);
            SpannableStringBuilder style = applySpannableStyle(noticeString, lable, color);
            noticeText.setText(style);
            noticeText.setVisibility(View.VISIBLE);
            noticeTextDivider.setVisibility(View.VISIBLE);
        } else {
            noticeText.setVisibility(View.GONE);
            noticeTextDivider.setVisibility(View.GONE);
        }
    }

    private SpannableStringBuilder applySpannableStyle(String target, String source, int color) {
        SpannableStringBuilder style = new SpannableStringBuilder(target);

        int start = target.indexOf(source);
        int end = target.length() - 1;
        Log.d(TAG, "applySpannableStyle, start = " + start + ", end = " + end);

        if (end > start && start > 0 && color != -1) {
            style.setSpan(new ForegroundColorSpan(color),
                  start, end, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
        return style;
    }

    /**
     * for op02/op09
     * called to update empty message
     * @param fragment the CallLogFragment
     * @param filter the type of call filter
     * @return boolean if plugin processed return true, or else return false to process in host
     */
    @Override
    public boolean updateEmptyMessage(int filter) {
        Log.d(TAG, "updateEmptyMessage, filter = " + filter);
        View view = mFragment.getView();
        Activity activity = mFragment.getActivity();
        Resources resource = activity.getResources();
        String packageName = activity.getPackageName();
        TextView emptyTextView = (TextView) view.findViewById(
                resource.getIdentifier("empty_list_view_message", "id", packageName));
        TextView actionTextView = (TextView) view.findViewById(
                resource.getIdentifier("empty_list_view_action", "id", packageName));

        final int messageId;
        switch(filter) {
            case Calls.INCOMING_TYPE:
                messageId = R.string.call_log_incoming_empty;
                break;
            case Calls.OUTGOING_TYPE:
                messageId = R.string.call_log_outgoing_empty;
                break;
            default:
                return false;
        }

        if (emptyTextView != null) {
            emptyTextView.setText(mPluginContext.getText(messageId));
        }

        if (actionTextView != null) {
            actionTextView.setText(null);
            actionTextView.setVisibility(View.GONE);
        }
        return true;
    }

   @Override
   public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
       if (!TextUtils.equals(key, PhoneAccountInfoHelper.PRE_KEY)) {
           return;
       }
       Log.d(TAG, "onSharedPreferenceChanged: update Ui now");
       mForceFresh.accept(true);
   }
}
