package com.mediatek.dialer.calllog.presence;

import android.app.Activity;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.RecyclerView.ViewHolder;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.dialer.common.LogUtils;
import com.mediatek.dialer.ext.DefaultCallLogAdapterExt;
import com.mediatek.dialer.ext.ICallLogListItemViewHolderExt;
import com.mediatek.dialer.presence.ContactNumberUtils;
import com.mediatek.dialer.presence.PresenceApiManager;
import com.mediatek.dialer.presence.PresenceApiManager.CapabilitiesChangeListener;
import com.mediatek.dialer.presence.PresenceApiManager.ContactInformation;

import java.util.ArrayList;
import java.util.List;

public class CallLogAdapterExt extends DefaultCallLogAdapterExt
    implements CapabilitiesChangeListener {
    private static final String TAG = "CallLogAdapterExt ";

    private RecyclerView.Adapter mAdapter;
    private Activity mActivity;
    private Handler mHandler = null;
    private PresenceApiManager mTapi = null;
    private List<CallLogListItemViewHolderExt> mViewHolderList =
            new ArrayList<CallLogListItemViewHolderExt>();

    CallLogAdapterExt(Activity activity, RecyclerView.Adapter adapter) {
        Log.d(TAG, "new CallLogFragmentExt activity: " + activity);
        mActivity = activity;
        mAdapter = adapter;
        mHandler = new Handler(Looper.getMainLooper());
        if (PresenceApiManager.initialize(mActivity)) {
            mTapi = PresenceApiManager.getInstance();
        }
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume list size:" + mViewHolderList.size());
        if (mTapi != null) {
            mTapi.addCapabilitiesChangeListener(this);
        }
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause list size:" + mViewHolderList.size());
        if (mTapi != null) {
            mTapi.removeCapabilitiesChangeListener(this);
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy list size:" + mViewHolderList.size());
        mViewHolderList.clear();
    }

    @Override
    public void onViewAttachedToWindow(ICallLogListItemViewHolderExt viewHolder) {
        Log.d(TAG, "onViewAttachedToWindow: " + viewHolder);
        CallLogListItemViewHolderExt holder = (CallLogListItemViewHolderExt)viewHolder;
        if (holder == null || TextUtils.isEmpty(holder.mNumber)) {
            Log.d(TAG, "onViewAttachedToWindow hold or number is null return directly");
            return;
        }

        mViewHolderList.add(holder);
        Log.d(TAG, "onViewAttachedToWindow add list: " + holder);

        String formatNumber = ContactNumberUtils.getDefault()
                .getFormatNumber(holder.mNumber);
        LogUtils.printSensitiveInfo(TAG, "onViewAttachedToWindow: formatNumber = " + formatNumber);
        if (mTapi != null) {
            mTapi.requestContactPresence(formatNumber, false);
        }
    }

    @Override
    public void onViewDetachedFromWindow(ICallLogListItemViewHolderExt viewHolder) {
        Log.d(TAG, "onViewDetachedFromWindow: " + viewHolder);
        CallLogListItemViewHolderExt holder = (CallLogListItemViewHolderExt)viewHolder;
        Log.d(TAG, "onViewDetachedFromWindow remove from list: " + holder);
        mViewHolderList.remove(holder);
    }

    @Override
    public void onCapabilitiesChanged(String contact, ContactInformation info) {
        Log.d(TAG, "onCapabilitiesChanged contact:" + contact);
        mHandler.post(() -> {
            for(CallLogListItemViewHolderExt viewHolder : mViewHolderList) {
                LogUtils.printSensitiveInfo(TAG,
                        "onCapabilitiesChanged viewHolder.mNumber:" + viewHolder.mNumber);
                if (!TextUtils.isEmpty(viewHolder.mNumber) && viewHolder.mNumber.equals(contact)) {
                    Log.d(TAG, "onCapabilitiesChanged update ui");
                    viewHolder.updatePrimaryActionButton();
                    if (viewHolder.mShow) {
                        viewHolder.bindActionButtons();
                    }
                }
            }
        });
    }

    @Override
    public void onErrorReceived(String contact, int type, int status, String reason) {
        LogUtils.printSensitiveInfo(TAG, "onErrorReceived contact:" + contact);
        String errMsg = "onErrorReceived contact:" + contact + " reason:" + reason
                + " type:" + type + " status:" + status;
        mHandler.post(() -> {
            Toast toast = Toast.makeText(mActivity, errMsg, Toast.LENGTH_LONG);
            toast.show();
        });
    }
}
