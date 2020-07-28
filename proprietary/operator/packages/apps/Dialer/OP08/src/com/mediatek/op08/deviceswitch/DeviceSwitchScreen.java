package com.mediatek.op08.deviceswitch;

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

import android.R.color;
import android.app.Activity;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.telephony.PhoneNumberFormattingTextWatcher;
import android.telephony.PhoneNumberUtils;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.View;
import android.view.Window;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import android.util.Log;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.android.incallui.call.TelecomAdapter;
import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.mediatek.contacts.util.PhoneNumberFormatter;
import com.mediatek.digits.DigitsManager;
import com.mediatek.digits.DigitsDevice;

import com.mediatek.op08.dialer.R;
import com.mediatek.op08.incallui.OP08InCallExt;

//import mediatek.telecom.MtkTelecomManager;

public class DeviceSwitchScreen extends AlertActivity implements
        DialogInterface.OnClickListener{

    private static final String TAG = "DeviceSwitchScreen";
    private ImageButton mChooseContact;
    private Button mTransferButton;
    private String mCallId;
    private static DeviceSwitchListAdapter mAdapter;
    private List<DeviceSwitchInfo> mAllowedDevicesList = new ArrayList<DeviceSwitchInfo>();
    private static ListView mAllowedDeivesListView;
    private TextView mEmptyText;
    private static TextView mAllMyDevices;
    private static TextView mProgressTextView;
    private static ImageView mLineDivider;
    private Button mSwitchButton;
    private Button mCancelButton;
    private static LinearLayout mDeviceSwitchProgressBar;
    public static String mSelectedDeviceName;
    public static DialogFragment mDeviceSwitchProgressDialog = null;
    private Context mContext;
    private String mNumber;
    /**
     * Action to dismiss device switch screen after success or fail event.
     */
    public static final String ACTION_DEVICE_SWITCH_DISMISS =
            "mediatek.incallui.event.ACTION_DEVICE_SWITCH_DISMISS";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate...");
        super.onCreate(savedInstanceState);
        mContext = getApplicationContext();
        //setContentView(R.layout.mtk_device_switch_screen);
        createDeviceListDialog();
        DigitsManager mDigitsManager = DeviceSwitchScreenController
                .getInstance().getDigitsManager();
        mNumber = getIntent().getExtras().getString("activeLine");
        Log.d(TAG, "onCreate...mDigitsManager" + mDigitsManager + "& virtualLine=" + mNumber);
        DigitsDevice[] device = null;
        try {
            device = mDigitsManager.getRegisteredDevice();
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        if(mAllowedDevicesList.size() > 0) {
            mAllowedDevicesList.clear();
        }
        if (device != null) {
        for (DigitsDevice d : device) {
            String id = d.getDeviceId();
            String name = d.getDeviceName();
            int type = d.getDeviceType();
            boolean my = d.getIsMy();
                // Don't add device in list, if not contains the active line MSISDN
                boolean isActiveLine = false;
                String virtualLine = null;
            String[] msisdns = d.getMsisdns();
                for (int i =0 ; i< msisdns.length ; i++) {
                    if(mNumber != null && mNumber.contains(msisdns[i])) {
                        isActiveLine = true;
                        virtualLine = msisdns[i];
                }
                }
                DeviceSwitchInfo deviceInfo = new DeviceSwitchInfo(virtualLine,
                        null, name, id, null);
                Log.d(TAG, "DigitsDevice added...id" + id + ", name=" + name
                        + ", msisdns" + msisdns +"isMy=" + my + "activeLine=" + isActiveLine);
                if(!my && isActiveLine)
                    mAllowedDevicesList.add(deviceInfo);
        }
        }
        mCallId = getIntent().getExtras().getString("CallId");
        mAllowedDeivesListView = (ListView) findViewById(R.id.device_switch_list);
        mEmptyText = (TextView) findViewById(android.R.id.empty);
        mLineDivider = (ImageView) findViewById(R.id.under_line);
        mAllMyDevices = (TextView) findViewById(R.id.all_my_devices);
        mProgressTextView = (TextView) findViewById(R.id.progress_text);
        mDeviceSwitchProgressBar = (LinearLayout) findViewById(R.id.switchProgress);
        mAdapter = new DeviceSwitchListAdapter(this, mAllowedDevicesList, mCallId);
        mAllowedDeivesListView.setAdapter(mAdapter);
        Log.d(TAG, "mAllowedDevicesList size..." + mAllowedDevicesList.size());
        mAdapter.notifyDataSetChanged();
        if(mAllowedDevicesList.size() == 0) {
          mAllowedDeivesListView.setEmptyView(mEmptyText);
          mLineDivider.setVisibility(View.GONE);
          mAllMyDevices.setVisibility(View.GONE);
          Log.d(TAG, "mAllowedDevicesList no devices found, device[] -" + device);
        }
        mAllowedDeivesListView.setVisibility(View.VISIBLE);
        mAllMyDevices.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick.mAllMyDevices()");
                sendDeviceSwitch(null, null, null);
                showProgressDialog(mContext, mAllMyDevices.getText().toString());
            }
        });
        DeviceSwitchScreenController.getInstance().setDeviceSwitchDialog(this);
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_DEVICE_SWITCH_DISMISS);
        mContext.registerReceiver(sReceiver, filter);
    }

    private void createDeviceListDialog() {
        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = getString(R.string.device_switch_title);
        p.mView = getLayoutInflater().inflate(R.layout.mtk_device_switch_screen, null);
        p.mNegativeButtonText = getString(R.string.device_switch_cancel);
        p.mNegativeButtonListener = this;
        p.mCancelable = true;
        setupAlert();
    }

    private View getViewByPosition(int pos, ListView listView) {
        final int firstListItemPosition = listView.getFirstVisiblePosition();
        final int lastListItemPosition = firstListItemPosition + listView.getChildCount() - 1;
        if (pos < firstListItemPosition || pos > lastListItemPosition) {
            Log.d(TAG, "view not visible");
            return null;
        } else {
            final int childIndex = pos - firstListItemPosition;
            Log.d(TAG, "index:" + childIndex);
            return listView.getChildAt(childIndex);
        }
    }

    public static void refreshDeviceList() {
            mAdapter.notifyDataSetChanged();
        }

    public View createView() {
        View view = getLayoutInflater().inflate(R.layout.mtk_device_switch_screen, null);
        return view;
    }

    @Override
    protected void onDestroy() {
        DeviceSwitchScreenController.getInstance().clearDeviceSwitchDialog(this);
        super.onDestroy();
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        // TODO Auto-generated method stub
        if (DialogInterface.BUTTON_NEGATIVE == which) {
            Log.d(TAG, "onClick->entry , canceleld Clicked ");
            cancelDeviceSwitch();
            DeviceSwitchScreenController.getInstance().dismissDialog();
        }
    }

    public void sendDeviceSwitch(String number, String deviceId, String deviceName) {
        /// M: Just remove code, the original code use broadcast to send security information.
        /// So remove these code.
    }

    public void cancelDeviceSwitch() {
        Log.d(TAG, "cancelDeviceSwitch->mCallId = " + mCallId);
        if (mCallId != null) {
            Intent broadcastIntent = new Intent(OP08InCallExt.EVENT_DEVICE_SWITCH_CANCEL);
            broadcastIntent.putExtra("callId", mCallId);
            getApplicationContext().sendBroadcast(broadcastIntent);
        }
    }

    public static void showProgressDialog(Context context, String selectedNumber) {
        Log.d(TAG, "showProgressDialog->entry , selectedNumber= " + selectedNumber);
        mAllowedDeivesListView.setVisibility(View.GONE);
        mAllMyDevices.setVisibility(View.GONE);
        mLineDivider.setVisibility(View.GONE);
        mDeviceSwitchProgressBar.setVisibility(View.VISIBLE);
        mProgressTextView.setText(context.getResources()
                .getString(R.string.device_switch_progress) + " " + selectedNumber);
    }

    public BroadcastReceiver sReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (intent.getAction().equals(ACTION_DEVICE_SWITCH_DISMISS)) {
                boolean switchStatus = intent.getBooleanExtra("switchStatus", false);
                Log.d(TAG, "onReceive->entry , ACTION_DEVICE_SWITCH_DISMISS= " + switchStatus);
                if(switchStatus) {
                    String successString =
                            mContext.getResources().getString(R.string.device_switch_successfull);
                    String deviceSwitchTime = DateFormat.getDateTimeInstance().format(new Date());
                    String deviceName = mSelectedDeviceName;
                    if(deviceName == null) {
                        deviceName = mContext.getResources().getString(R.string.all_my_devices);
                    }
                    successString = successString + mSelectedDeviceName +
                            " at "+ deviceSwitchTime;
                    Toast.makeText(mContext,successString,Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(mContext, R.string.device_switch_failed,
                            Toast.LENGTH_LONG).show();
                }
                DeviceSwitchScreen.this.finish();
            }
        }
    };
}
