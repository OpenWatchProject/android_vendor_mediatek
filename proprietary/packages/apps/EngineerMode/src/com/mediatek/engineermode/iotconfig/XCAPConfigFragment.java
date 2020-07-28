package com.mediatek.engineermode.iotconfig;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.support.v4.app.Fragment;
import android.telephony.SubscriptionManager;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmApplication;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class XCAPConfigFragment extends Fragment implements OnItemClickListener {
    protected static final String TAG = "Iot/XCAPConfigFragment";
    public static final int UNKNOW = -1;
    protected static final int DEFAULT_VALUE_XCAP_READY = 0;
    protected static final int SET_VALUE_XCAP_OK = 1;
    protected static final int SET_VALUE_XCAP_FAIL = 2;
    protected static final int RESET_VALUE_XCAP_OK = 3;
    protected static final int SET_SS_PROPERTY = 4;
    protected static final int XCAP_ACTION_GET = 0;
    protected static final int XCAP_ACTION_SET = 1;
    protected static final int XCAP_ACTION_RESET = 2;
    protected static final String DEFAULT_VALUE_XCAP_INTEGER = "-1";
    protected static final String DEFAULT_VALUE_XCAP_BOOLEAN = "0";
    protected static final String DEFAULT_VALUE_XCAP_STRING = "";

    // For 95
    protected static final String DEFAULT_DISABLE_VALUE = "0";
    protected static final String DEFAULT_ENABLE_VALUE = "1";

    protected static final String mATCMD = "SET_SS_PROP";

    private ListView listView;
    private Button btnSave;
    private XcapSettingAdapter adapter;
    private String[] mXcapBoolCfg;
    private LinkedHashMap<String, String> XcapItems;
    private HashMap<String, String> XcapItemTypes;
    private Button btnCancel;

    public int mPhoneId = 0;

    private boolean mIs95Modem = false;

    private CountDownLatch mCallbackLatch;
    private static final int OPERATION_TIME_OUT_MILLIS = 3000;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case DEFAULT_VALUE_XCAP_READY:
                updateUI((List<XCAPModel>) msg.obj);
                break;
            case SET_VALUE_XCAP_OK:
                updateUI((List<XCAPModel>) msg.obj);
                EmUtils.showToast("Set values done!");
                break;
            case RESET_VALUE_XCAP_OK:
                updateUI((List<XCAPModel>) msg.obj);
                EmUtils.showToast("Reset values done!");
                break;
            case SET_VALUE_XCAP_FAIL:
                EmUtils.showToast("Set values failed!");
                new AlertDialog.Builder(EmApplication.getContext())
                        .setMessage("Set values failed!")
                        .setPositiveButton("OK",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,
                                            int id) {
                                        dialog.cancel();
                                    }
                                }).show();
             case SET_SS_PROPERTY:
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Elog.e(TAG, "SET_SS_PROPERTY: ar.exception=" + ar.exception);
                    EmUtils.showToast("Set values to modem failed!");
                }
                if (mCallbackLatch != null) {
                    mCallbackLatch.countDown();
                }
                Elog.d(TAG, "SET_SS_PROPERTY done");
                break;
            default:
                break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        mPhoneId = SubscriptionManager.getPhoneId(SubscriptionManager
                .getDefaultDataSubscriptionId());
        mIs95Modem =
            SystemProperties.get("persist.vendor.ss.modem_version").equals("1") ? true : false;
        Elog.d(TAG, "mIs95Modem = " + mIs95Modem);
        if (FeatureSupport.is93Modem() && !mIs95Modem) {
            mXcapBoolCfg = IotConfigConstant.mXcapBoolCfgFor93;
            XcapItems = IotConfigConstant.mXcapItemsFor93;
            XcapItemTypes = IotConfigConstant.mXcapItemTypesFor93;
        } else if (mIs95Modem) {
            mXcapBoolCfg = IotConfigConstant.mXcapBoolCfgBeyond93;
            XcapItems = IotConfigConstant.mXcapItemsBeyond93;
            XcapItemTypes = IotConfigConstant.mXcapItemTypesBeyond93;
        } else {
            mXcapBoolCfg = IotConfigConstant.mXcapBoolCfgBelow93;
            XcapItems = IotConfigConstant.mXcapItemsBelow93;
            XcapItemTypes = IotConfigConstant.mXcapItemTypesBelow93;
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        View view = inflater.inflate(R.layout.iot_xcap_config, null);
        listView = (ListView) view.findViewById(R.id.xcap_list);
        btnSave = (Button) view.findViewById(R.id.btnSave);
        btnCancel = (Button) view.findViewById(R.id.btnCancel);
        adapter = new XcapSettingAdapter(getActivity(), getEmptyModel());
        listView.setAdapter(adapter);
        btnSave.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                new LoadModelThread(XCAP_ACTION_SET).start();
            }
        });
        btnCancel.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                new LoadModelThread(XCAP_ACTION_RESET).start();
                ;
            }
        });
        return view;
    }

    public void hideInputMethod(View view) {
        if (getActivity() != null && view == null) {
            view = getActivity().getCurrentFocus();
        }
        if (view != null) {
            view.clearFocus();
            InputMethodManager imm = (InputMethodManager)getActivity()
                    .getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm.isActive()) {
                imm.hideSoftInputFromWindow(view.getWindowToken(),  0);
            }
        }
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        if (!isVisibleToUser) {
            hideInputMethod(null);
        }
    }

    public class LoadModelThread extends Thread {
        private int action;

        public LoadModelThread(int action) {
            this.action = action;
        }

        public void run() {
            Message msg = new Message();
            switch (action) {
            case XCAP_ACTION_GET:
                msg.obj = getModel();
                msg.what = DEFAULT_VALUE_XCAP_READY;
                break;
            case XCAP_ACTION_SET:
                if (setModel()) {
                    try {
                        Thread.sleep(2000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    msg.obj = getModel();
                    msg.what = SET_VALUE_XCAP_OK;
                } else {
                    msg.what = SET_VALUE_XCAP_FAIL;
                }
                break;
            case XCAP_ACTION_RESET:
                msg.obj = resetModel();
                msg.what = RESET_VALUE_XCAP_OK;
                break;
            default:
                msg.what = UNKNOW;
                break;
            }
            mHandler.sendMessage(msg);
        }

    }

    public void updateUI(List<XCAPModel> updateModel) {
        adapter.clear();
        adapter.addAll(updateModel);
        adapter.notifyDataSetChanged();
    }

    public void sendATcmdToRIL(int phoneid, String[] cmd, Message response) {
        Elog.d(TAG, "sendATcmdToRIL : " + Arrays.toString(cmd));
        mCallbackLatch = new CountDownLatch(1);
        EmUtils.invokeOemRilRequestStringsEm(phoneid, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
        if (!isCallbackDone()) {
            // do error handling
            Elog.e(TAG, "waitForCallback: callback is not done!");
        }
    }

    private boolean isCallbackDone() {
        boolean isDone = false;

        try {
            isDone = mCallbackLatch.await(OPERATION_TIME_OUT_MILLIS,
                    TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            isDone = false;
        }

        Elog.d(TAG, "waitForCallback: isDone=" + isDone);
        return isDone;
    }

    private boolean setModel() {
        int mBoolConfig = 0;
        int mBoolValue = 0;
        List<XCAPModel> mList = adapter.getList();
        Elog.d(TAG, "[setModel]listSize : " + mList.size());
        String[] cmd = new String[2];
        for (int i = 0; i < mList.size(); i++) {
            if (i < mXcapBoolCfg.length) {
                if (mIs95Modem) {
                    if (mList.get(i).isConfiged()) {
                        cmd[0] = mATCMD;
                        cmd[1] = IotConfigConstant.SS_PREFIX + IotConfigConstant.mXcapBoolPropertyBeyond93[i] +
                                 "," + (mList.get(i).isSelected() ? DEFAULT_ENABLE_VALUE : DEFAULT_DISABLE_VALUE);
                        sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
                    }
                } else {
                    if (mList.get(i).isConfiged()) {
                        if (mList.get(i).isSelected()) {
                            mBoolValue |= (1 << i);
                        } else {
                            mBoolValue &= ~(1 << i);
                        }
                        mBoolConfig |= (1 << i);
                    } else {
                        mBoolConfig &= ~(1 << i);
                    }
                }
            } else {
                if (!"".equals(mList.get(i).getValue())) {
                    cmd[0] = mATCMD;
                    cmd[1] = XcapItems.get(mList.get(i).getName()) + "," + mList.get(i).getValue();
                    sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
                }
            }
        }
        if (!mIs95Modem) {
            Elog.d(TAG, "[setModel]BoolValue: " + mBoolValue
            + " BoolConfig: " + mBoolConfig);
            String[] cmdValue = new String[2];
            cmdValue[0] = mATCMD;
            cmdValue[1] = IotConfigConstant.FK_SS_BOOLVALUE + "," + mBoolValue;
            sendATcmdToRIL(mPhoneId, cmdValue, mHandler.obtainMessage(SET_SS_PROPERTY));

            String[] cmdConfig = new String[2];
            cmdConfig[0] = mATCMD;
            cmdConfig[1] = IotConfigConstant.FK_SS_BOOLCONFIG + "," + mBoolConfig;
            sendATcmdToRIL(mPhoneId, cmdConfig, mHandler.obtainMessage(SET_SS_PROPERTY));
        }
        return true;
    }

    private List<XCAPModel> resetModel() {
        ArrayList<XCAPModel> mList = new ArrayList<XCAPModel>();
        String[] cmd = new String[2];
        if (!mIs95Modem) {
            String[] cmdValue = new String[2];
            cmdValue[0] = mATCMD;
            cmdValue[1] =  IotConfigConstant.FK_SS_BOOLVALUE + ",0";
            sendATcmdToRIL(mPhoneId, cmdValue, mHandler.obtainMessage(SET_SS_PROPERTY));

            String[] cmdConfig = new String[2];
            cmdConfig[0] = mATCMD;
            cmdConfig[1] = IotConfigConstant.FK_SS_BOOLCONFIG + ",0";
            sendATcmdToRIL(mPhoneId, cmdConfig, mHandler.obtainMessage(SET_SS_PROPERTY));
        } else {
            // Start 95 modem reset
            cmd[0] = mATCMD;
            cmd[1] = IotConfigConstant.FK_SS_CONFIG_RESET_95;
            sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
        }

        for (int i = 0; i < (mXcapBoolCfg.length); i++) {
            mList.add(new XCAPModel(mXcapBoolCfg[i],
                    IotConfigConstant.BOOLEANTYPE));
            if (mIs95Modem) {
                cmd[0] = mATCMD;
                cmd[1] = IotConfigConstant.SS_PREFIX + IotConfigConstant.mXcapBoolPropertyBeyond93[i] +
                              "," + "";
                sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
            }
        }

        for (String str : XcapItems.keySet()) {
            if (!mIs95Modem) {
                if (XcapItemTypes.get(str).equals(IotConfigConstant.STRINGTYPE)) {
                    cmd[0] = mATCMD;
                    cmd[1] = XcapItems.get(str) + "," + DEFAULT_VALUE_XCAP_STRING;
                    sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
                    mList.add(new XCAPModel(str, SystemProperties.get(XcapItems
                            .get(str)), XcapItemTypes.get(str)));
                } else if (XcapItemTypes.get(str).equals(IotConfigConstant.INTEGERTYPE)) {
                    cmd[0] = mATCMD;
                    cmd[1] = XcapItems.get(str) + "," + String.valueOf(DEFAULT_VALUE_XCAP_INTEGER);
                    sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
                    mList.add(new XCAPModel(str, SystemProperties.get(XcapItems
                            .get(str)), XcapItemTypes.get(str)));
                } else {
                    cmd[0] = mATCMD;
                    cmd[1] = XcapItems.get(str) + "," + DEFAULT_VALUE_XCAP_BOOLEAN;
                    sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
                    mList.add(new XCAPModel(str, SystemProperties.get(
                            XcapItems.get(str)).equals("1") ? true : false,
                            XcapItemTypes.get(str)));
                }
            } else {
                if (XcapItemTypes.get(str).equals(IotConfigConstant.STRINGTYPE)) {
                    cmd[0] = mATCMD;
                    cmd[1] = XcapItems.get(str) + "," + "";
                    sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
                    mList.add(new XCAPModel(str, SystemProperties.get(XcapItems
                            .get(str)), XcapItemTypes.get(str)));
                } else if (XcapItemTypes.get(str).equals(
                        IotConfigConstant.INTEGERTYPE)) {
                    cmd[0] = mATCMD;
                    cmd[1] = XcapItems.get(str) + "," + "";
                    sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
                    mList.add(new XCAPModel(str, SystemProperties.get(XcapItems
                            .get(str)), XcapItemTypes.get(str)));
                } else {
                    cmd[0] = mATCMD;
                    cmd[1] = XcapItems.get(str) + "," + "";
                    sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
                    mList.add(new XCAPModel(str, SystemProperties.get(
                            XcapItems.get(str)).equals("1") ? true : false,
                            XcapItemTypes.get(str)));
                }
            }
        }

        if (mIs95Modem) {
            // 95 modem config reset done
            cmd[0] = mATCMD;
            cmd[1] = IotConfigConstant.FK_SS_CONFIG_RESET_DONE_95;
            sendATcmdToRIL(mPhoneId, cmd, mHandler.obtainMessage(SET_SS_PROPERTY));
        }

        return mList;
    }

    private List<XCAPModel> getEmptyModel() {
        ArrayList<XCAPModel> mList = new ArrayList<XCAPModel>();
        for (int i = 0; i < (mXcapBoolCfg.length); i++) {
            mList.add(new XCAPModel(mXcapBoolCfg[i],
                    IotConfigConstant.BOOLEANTYPE));
        }
        for (String str : XcapItems.keySet()) {
            mList.add(new XCAPModel(str, XcapItemTypes.get(str)));
        }
        return mList;
    }

    private List<XCAPModel> getModel() {
        ArrayList<XCAPModel> mList = new ArrayList<XCAPModel>();
        if (!mIs95Modem) {
            String boolValue = Integer.toBinaryString(Integer
                .valueOf(SystemProperties.get(
                        IotConfigConstant.FK_SS_BOOLVALUE, "0")));
            String boolConfig = Integer.toBinaryString(Integer
                    .valueOf(SystemProperties.get(
                            IotConfigConstant.FK_SS_BOOLCONFIG, "0")));
            Elog.d(TAG, "[getModel]BoolValue: " + boolValue
                    + " BoolConfig: " + boolConfig);
            char[] results = new char[boolValue.length()];
            char[] configs = new char[boolConfig.length()];
            results = boolValue.toCharArray();
            configs = boolConfig.toCharArray();
            for (int i = 0; i < (mXcapBoolCfg.length); i++) {
                boolean itemConfiged = false;
                if (i < configs.length && configs[configs.length - 1 - i] == ('1')) {
                    itemConfiged = true;
                }
                boolean itemSelected = false;
                if (i < results.length && results[results.length - 1 - i] == ('1')) {
                    itemSelected = true;
                }
                mList.add(new XCAPModel(mXcapBoolCfg[i], itemConfiged,
                        itemSelected, IotConfigConstant.BOOLEANTYPE));
            }
        } else {
            for (int i = 0; i < (mXcapBoolCfg.length); i++) {
                boolean result =
                    SystemProperties.get(IotConfigConstant.SS_PREFIX +
                        IotConfigConstant.mXcapBoolPropertyBeyond93[i]).equals("1") ? true : false;
                mList.add(new XCAPModel(mXcapBoolCfg[i], result,
                    result, IotConfigConstant.BOOLEANTYPE));
            }
        }

        for (String str : XcapItems.keySet()) {
            if (XcapItemTypes.get(str).equals(IotConfigConstant.BOOLEANTYPE)) {
                boolean itemSelected = false;
                if (SystemProperties.get(XcapItems.get(str)).equals("1")) {
                    itemSelected = true;
                }
                mList.add(new XCAPModel(str, itemSelected, XcapItemTypes
                        .get(str)));
            } else {
                mList.add(new XCAPModel(str, SystemProperties.get(
                        XcapItems.get(str), ""), XcapItemTypes.get(str)));
            }
        }
        return mList;
    }

    @Override
    public void onResume() {
        // TODO Auto-generated method stub
        new LoadModelThread(XCAP_ACTION_GET).start();
        super.onResume();
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        // TODO Auto-generated method stub

    }

}

class XcapSettingAdapter extends ArrayAdapter<XCAPModel> {

    private List<XCAPModel> list;
    private final Activity context;
    int listPosititon;

    public XcapSettingAdapter(Activity context, List<XCAPModel> list) {
        super(context, R.layout.iot_xcap_row, list);
        this.context = context;
        this.list = list;
    }

    public void refresh(List<XCAPModel> list) {
        this.list = list;
        notifyDataSetChanged();
    }

    public List<XCAPModel> getList() {
        return this.list;
    }

    static class ViewHolder {
        protected TextView text;
        protected CheckBox checkbox;
        protected EditText value;
        protected Button btnSet;
    }

    private OnClickListener onClickLinstener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            int getPosition = (Integer) v.getTag();
            showConfirmDialog(v, getPosition);
        }
    };

    public void showConfirmDialog(final View view, final int getPosition) {

        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                context);
        alertDialogBuilder
                .setCancelable(false)
                .setMessage(
                        context.getResources().getString(
                                R.string.set_xcap_dialog_message))
                .setTitle(
                        context.getResources().getString(
                                R.string.set_xcap_dialog_title))
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        list.get(getPosition).setConfiged(true);
                        list.get(getPosition).setSelected(false);
                        notifyDataSetChanged();
                    }
                })
                .setNegativeButton("Cancel",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                dialog.cancel();
                                list.get(getPosition).setConfiged(false);
                            }
                        });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        listPosititon = position;
        ViewHolder viewHolder = null;
        if (convertView == null) {
            LayoutInflater inflator = context.getLayoutInflater();
            convertView = inflator.inflate(R.layout.iot_xcap_row, null);
            viewHolder = new ViewHolder();
            viewHolder.text = (TextView) convertView
                    .findViewById(R.id.xcap_label);
            viewHolder.checkbox = (CheckBox) convertView
                    .findViewById(R.id.xcap_check);
            viewHolder.value = (EditText) convertView
                    .findViewById(R.id.xcap_value);
            viewHolder.btnSet = (Button) convertView
                    .findViewById(R.id.set_xcap);

            viewHolder.btnSet.setOnClickListener(onClickLinstener);

            viewHolder.checkbox
                    .setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

                        @Override
                        public void onCheckedChanged(CompoundButton buttonView,
                                boolean isChecked) {
                            int getPosition = (Integer) buttonView.getTag();
                            list.get(getPosition).setSelected(
                                    buttonView.isChecked());
                        }
                    });
            viewHolder.value.addTextChangedListener(new TextWatcher() {

                @Override
                public void onTextChanged(CharSequence s, int start,
                        int before, int count) {
                    // TODO Auto-generated method stub

                }

                @Override
                public void beforeTextChanged(CharSequence s, int start,
                        int count, int after) {
                    // TODO Auto-generated method stub

                }

                @Override
                public void afterTextChanged(Editable s) {
                    // TODO Auto-generated method stub
                    list.get(position).setValue(s.toString());
                }
            });

            viewHolder.value.setImeOptions(EditorInfo.IME_ACTION_DONE);
            viewHolder.value.setRawInputType(InputType.TYPE_CLASS_TEXT);
            viewHolder.value.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    if (v.getId() == R.id.xcap_value) {
                        v.getParent().requestDisallowInterceptTouchEvent(true);
                        switch (event.getAction() & MotionEvent.ACTION_MASK) {
                            case MotionEvent.ACTION_UP:
                                v.getParent().requestDisallowInterceptTouchEvent(false);
                                break;
                        }
                    }
                    return false;
                }
            });
            convertView.setTag(viewHolder);
            convertView.setTag(R.id.xcap_label, viewHolder.text);
            convertView.setTag(R.id.xcap_value, viewHolder.value);
            convertView.setTag(R.id.xcap_check, viewHolder.checkbox);
            convertView.setTag(R.id.set_xcap, viewHolder.btnSet);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }
        viewHolder.checkbox.setTag(position); // This line is important.
        viewHolder.btnSet.setTag(position);
        viewHolder.value.setTag(position);
        viewHolder.text.setTag(position);

        viewHolder.text.setText((list.get(position).getName() + "("
                + list.get(position).getType() + ")").replace("("
                + IotConfigConstant.BOOLEANTYPE + ")", "?"));
        LinearLayout booleanLayout = (LinearLayout) convertView
                .findViewById(R.id.xcap_check_layout);
        if (list.get(position).getValue() != null
                && !list.get(position).getValue()
                        .equals(XCAPConfigFragment.DEFAULT_VALUE_XCAP_INTEGER)) {
            viewHolder.value.setText(list.get(position).getValue() + "");
        } else {
            viewHolder.value.setText("");
        }
        if (list.get(position).getType().equals(IotConfigConstant.BOOLEANTYPE)) {

            booleanLayout.setVisibility(View.VISIBLE);
            viewHolder.value.setVisibility(View.GONE);
            if (list.get(position).isConfiged()) {
                viewHolder.checkbox.setVisibility(View.VISIBLE);
                viewHolder.btnSet.setVisibility(View.INVISIBLE);
                viewHolder.checkbox.setChecked(list.get(position).isSelected());
            } else {
                viewHolder.checkbox.setVisibility(View.INVISIBLE);
                viewHolder.btnSet.setVisibility(View.VISIBLE);
            }
        } else {
            booleanLayout.setVisibility(View.GONE);
            viewHolder.value.setVisibility(View.VISIBLE);
        }
        return convertView;
    }
}
