package com.mediatek.op01.phone.plugin;

import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnCancelListener;
import android.content.SharedPreferences.Editor;
import android.database.ContentObserver;
import android.os.Handler;
import android.os.IBinder;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.TelephonyIntents;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
* Data pick dialog.
*/
public class OP01SimDialogService extends Service {

    private static final String TAG = "OP01SimDialogService";
    private static final String DATA_SIM = "data_sim";
    private static final String SLOT_PREFIX = "sim_slot_";
    private static final String REG_OBSERVER_FLAG = "reg_observer";
    private static final int SLOT_EMPTY = -1;
    private static final int INVALID_SLOT = -2;
    private static final int sId = 1984;

    private AlertDialog mDialog;
    private SubscriptionManager mSubscriptionManager = null;
    private int[] mSubscriptionIdListCache = null;
    ArrayAdapter<String> mAdapter = null;
    private SharedPreferences mPreference;
    private TelephonyManager mTelephonyManager;
    private Intent mServiceIntent = null;
    public static boolean sIsShow = false;
    private boolean userSelect = false;

    private BroadcastReceiver mHotSwapReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            int[] subscriptionIdListCurrent = mSubscriptionManager.getActiveSubscriptionIdList();
            print("current subId list: ", subscriptionIdListCurrent);
            boolean isEqual = true;
            isEqual = Arrays.equals(mSubscriptionIdListCache, subscriptionIdListCurrent);
            Log.d(TAG, "isEqual: " + isEqual);
            if (!isEqual && mDialog != null && mDialog.isShowing()) {
                mDialog.dismiss();
                mDialog = null;
            }
            if (isEqual && mDialog != null && mDialog.isShowing()) {
                Log.d(TAG, "update dialog!");
                mAdapter.notifyDataSetChanged();
            }
        }
    };

    @Override
    public void onCreate() {
        Log.d(TAG, "create");
        mSubscriptionManager = SubscriptionManager.from(this);
        mTelephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        mPreference = this.createDeviceProtectedStorageContext().getSharedPreferences(DATA_SIM, MODE_PRIVATE);
    }

    @Override
    public int onStartCommand(Intent intent, int flag, int startId) {
        Log.d(TAG, "OP01SimDialogService start. startId = " + startId);
        showSimDialog(startId);
        mSubscriptionIdListCache = mSubscriptionManager.getActiveSubscriptionIdList();
        print("Init subId list: ", mSubscriptionIdListCache);
        IntentFilter filter = new IntentFilter(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        registerReceiver(mHotSwapReceiver, filter);
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "destroy");
        if (mDialog != null) {
            unregisterReceiver(mHotSwapReceiver);
        }
        mAdapter = null;
        stopForeground(true);
    }

    private void showSimDialog(int id) {
        final int serviceId = id;
        final ArrayList<String> list = new ArrayList<String>();
        final List<SubscriptionInfo> subInfoList =
                mSubscriptionManager.getActiveSubscriptionInfoList();
        final int selectableSubInfoLength = subInfoList == null ? 0 : subInfoList.size();

        for (int i = 0; i < selectableSubInfoLength; ++i) {
            final SubscriptionInfo sir = subInfoList.get(i);
            CharSequence displayName = sir.getDisplayName();
            if (displayName == null) {
                displayName = "";
            }
            list.add(displayName.toString());
        }
        String[] arr = list.toArray(new String[0]);

        mAdapter = new SelectAccountListAdapter(subInfoList,
                this, R.layout.select_account_list_item, arr);
        final DialogInterface.OnClickListener selectionListener =
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int value) {
                        userSelect = true;
                        Log.d(TAG, "set SystemProperties and " +
                                "click dialog value : " + value
                                + " userSelect = true ");
                        SubscriptionInfo sir = subInfoList.get(value);
                        setDefaultDataSubId(sir.getSubscriptionId());
                    }
                };

        final DialogInterface.OnDismissListener dismissListener =
                new DialogInterface.OnDismissListener() {
                    @Override
                    public void onDismiss(DialogInterface dialog) {
                        Log.d(TAG, "dialog onDismiss!");
                        if (!userSelect) {
                            Log.d(TAG, "dialog called!, set default to first sim card");
                            SubscriptionInfo sir = subInfoList.get(0);
                            setDefaultDataSubId(sir.getSubscriptionId());
                        }
                        sIsShow = false;
                        OP01SimDialogService.this.stopSelf(serviceId);
                    }
                };
        AlertDialog.Builder builder =
                new AlertDialog.Builder(this, AlertDialog.THEME_DEVICE_DEFAULT_LIGHT);
        builder.setTitle(R.string.select_sim_for_data);
        builder.setAdapter(mAdapter, selectionListener);
        builder.setOnDismissListener(dismissListener);
        mDialog = builder.create();
        mDialog.setCancelable(true);
        mDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        mDialog.show();
        sIsShow = true;
    }

    private void setDefaultDataSubId(final int subId) {
        if (TelecomManager.from(this).isInCall()) {
            String textErr =
                    getResources().getString(R.string.default_data_switch_err_msg1);
            Toast.makeText(this, textErr, Toast.LENGTH_SHORT).show();
            return;
        }
        mSubscriptionManager.setDefaultDataSubId(subId);
        String text = getResources().getString(R.string.data_switch_started);
        Toast.makeText(this, text, Toast.LENGTH_LONG).show();
    }

    private void print(String msg, int[] lists) {
        if (lists != null) {
            for (int i : lists) {
                Log.d(TAG, msg + i);
            }
        } else {
            Log.d(TAG, msg + "is null");
        }
    }

    /**
    * Adapter for sim list view in the dialog.
    */
    private class SelectAccountListAdapter extends ArrayAdapter<String> {
        private Context mContext;
        private final static float OPACITY = 0.54f;
        private int mResId;
        private List<SubscriptionInfo> mSubInfoList;

        public SelectAccountListAdapter(List<SubscriptionInfo> subInfoList,
                Context context, int resource, String[] arr) {
            super(context, resource, arr);
            mContext = context;
            mResId = resource;
            mSubInfoList = subInfoList;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LayoutInflater inflater = (LayoutInflater)
            mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View rowView = null;
            final ViewHolder holder;

            if (convertView == null) {
                rowView = inflater.inflate(mResId, null);
                holder = new ViewHolder();
                holder.mTitle = (TextView) rowView.findViewById(R.id.title);
                holder.mSummary = (TextView) rowView.findViewById(R.id.summary);
                holder.mIcon = (ImageView) rowView.findViewById(R.id.icon);
                rowView.setTag(holder);
            } else {
                rowView = convertView;
                holder = (ViewHolder) rowView.getTag();
            }
            mSubInfoList = mSubscriptionManager.getActiveSubscriptionInfoList();
            if (mSubInfoList == null) {
                Log.d(TAG, "mSubInfoList is null");
                return rowView;
            }
            int size = mSubInfoList.size();
            if (size <= position) {
                Log.d(TAG, "mSubInfoList size = " + size);
                return rowView;
            }
            final SubscriptionInfo sir = mSubInfoList.get(position);
            if (sir == null) {
                Log.d(TAG, "position " + position + " is null!");
                holder.mTitle.setText(getItem(position));
                holder.mSummary.setText("");
                holder.mIcon.setImageDrawable(getResources().getDrawable(R.drawable.ic_live_help));
                holder.mIcon.setAlpha(OPACITY);
            } else {
                holder.mTitle.setText(sir.getDisplayName());
                holder.mSummary.setText(sir.getNumber());
                holder.mIcon.setImageBitmap(sir.createIconBitmap(mContext));
            }
            return rowView;
        }
        /**
        * ViewHolder for each line of the list view.
        */
        private class ViewHolder {
            TextView mTitle;
            TextView mSummary;
            ImageView mIcon;
        }
    }
}