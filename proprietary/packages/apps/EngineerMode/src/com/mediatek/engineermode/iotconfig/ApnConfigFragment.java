package com.mediatek.engineermode.iotconfig;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.database.Cursor;
import android.os.Bundle;
import android.provider.Telephony;
import android.support.v4.app.Fragment;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.InputFilter;
import android.text.method.NumberKeyListener;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

public class ApnConfigFragment extends Fragment implements OnItemClickListener {

    private static final int DIALOG_MCCMNC_READFAIL = 0;
    private static final int DIALOG_MCCMNC_INVALID = 1;
    private static final int MCC_LENGTH = 3;
    private static final String TAG = "Iot/ApnConfigFragment";
    private static final int SIM_CARD_INVALID = -1;
    private static final String[] IOT_APN_DEFAULT_LABLES = new String[] {
            "mcc", "mnc", "apn", "type", "protocol", "roaming_protocol" };
    private Spinner mSpinner;
    private EditText mEtMccMnc;
    private Button btnConfirm;
    private boolean mMccMncReadSim = false;
    private Button btnCancel;
    private String mSimMccMnc;
    private String mMccMncText;
    private int mSimType;
    private int mSubId;

    private ArrayList<String> items;
    private FrameLayout mInfoFrameLayout;
    private TableInfoAdapter mAdapter;
    private ListView mListView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        View view = inflater.inflate(R.layout.iot_apn_config, container, false);
        initView(view);
        return view;
    }

    public void initView(View view) {
        mSpinner = (Spinner) view.findViewById(R.id.spinnernp1);
        mEtMccMnc = (EditText) view.findViewById(R.id.input_mccmnc);
        limitEditText(mEtMccMnc, 6);
        mInfoFrameLayout = (FrameLayout) view
                .findViewById(R.id.detail_frame_apn);

        btnConfirm = (Button) view.findViewById(R.id.btn_confirm);
        btnCancel = (Button) view.findViewById(R.id.btn_cancel);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(
                getActivity(), R.array.input_mode,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinner.setAdapter(adapter);
        mMccMncReadSim = true;

        AdapterView.OnItemSelectedListener l = new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                // set the regulation of EditText
                if (arg2 == 1) {
                    mMccMncReadSim = false;
                    mEtMccMnc.setText("");
                } else {
                    mMccMncReadSim = true;
                    if (mSimType != SIM_CARD_INVALID) {
                        mEtMccMnc.setText(mSimMccMnc);
                    }
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        };
        mSpinner.setOnItemSelectedListener(l);

        if (mAdapter == null) {
            mAdapter = new TableInfoAdapter(getActivity());
        }
        btnCancel.setOnClickListener(new Button.OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                mEtMccMnc.setText("");
            }

        });
        btnConfirm.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View view) {
                hideInputMethod(mEtMccMnc);
                queryAndShowApn();
            }
        });
        mEtMccMnc.setOnEditorActionListener(new OnEditorActionListener() {

            @Override
            public boolean onEditorAction(TextView v, int actionId,
                    KeyEvent event) {
                // TODO Auto-generated method stub
                if ((event != null
                        && (event.getKeyCode() == KeyEvent.KEYCODE_ENTER))
                        || (actionId == EditorInfo.IME_ACTION_DONE)) {
                    hideInputMethod(mEtMccMnc);
                }
                return false;
            }
        });
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

    public void queryAndShowApn() {
        mMccMncText = mEtMccMnc.getText().toString().replace(" ", "");
        if (!checkValidMCCMNC(mMccMncText)) {
            showDialog(DIALOG_MCCMNC_INVALID, getString(R.string.invalidfail)
                    + mMccMncText);
        } else {
            mAdapter.clear();
            String mcc = mMccMncText.substring(0, MCC_LENGTH);
            String mnc = mMccMncText
                    .substring(MCC_LENGTH, mMccMncText.length());
            Elog.d(TAG, "mcc: " + mcc + "mnc: " + mnc);
            String queryAPn = "numeric=\"" + mMccMncText + "\"";
            Elog.d(TAG, "queryAPn: " + queryAPn);
            String order = Telephony.Carriers.DEFAULT_SORT_ORDER;
            Cursor apns = getActivity().getContentResolver()
                    .query(Telephony.Carriers.CONTENT_URI, null, queryAPn,
                            null, order);
            try {
                if (apns != null) {
                    if (apns.moveToFirst() && (apns.getCount() > 0)) {
                        Elog.d(TAG, "[queryAndShowApn]apns count is: " + apns.getCount());
                        int apnNum = 0;
                        if (mSpinner.getSelectedItemPosition() == 0
                                && mMccMncText.equals(mSimMccMnc)) {
                            mAdapter.add(new String[] { "Read from  Sim"
                                    + (mSimType + 1) });
                        }
                        do {
                            apnNum += 1;
                            mAdapter.add(new String[] { "APN " + apnNum });
                            for (int i = 0; i < IOT_APN_DEFAULT_LABLES.length; i++) {
                                int index = apns
                                        .getColumnIndex(IOT_APN_DEFAULT_LABLES[i]);
                                if (index != -1) {
                                    mAdapter.add(new String[] {
                                            apns.getColumnNames()[index],
                                            apns.getString(index) });
                                }
                            }
                            // get the apn data if want to display on UI
                        } while (apns.moveToNext());
                    }
                }
            } finally {
                apns.close();
                updateUI();
            }

        }
    }

    public void updateUI() {
        mInfoFrameLayout.removeAllViews();
        View listView = getListView();
        if (mInfoFrameLayout.getChildCount() <= 0) {
            if (listView.getParent() != null) {
                ((ViewGroup) listView.getParent()).removeView(listView);
            }
            mInfoFrameLayout.addView(listView);
        }
    }

    public View getListView() {
        if (mAdapter == null) {
            mAdapter = new TableInfoAdapter(getActivity());
        }
        if (mListView == null) {
            mListView = new ListView(getActivity());
        }
        if (mAdapter.getCount() == 0) {
            // Show placeholder to avoid showing empty list
            for (int i = 0; i < IOT_APN_DEFAULT_LABLES.length; i++) {
                mAdapter.add(new String[] { IOT_APN_DEFAULT_LABLES[i], "" });
            }
        }
        mListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
        return mListView;
    }

    private boolean checkValidMCCMNC(String text) {
        if ((5 > text.length()) || (6 < text.length())) {
            return false;
        }
        return true;

    }

    public void limitEditText(EditText et, int limitLength) {
        et.setFilters(new InputFilter[] { new InputFilter.LengthFilter(
                limitLength) });
        et.setKeyListener(new NumberKeyListener() {
            @Override
            protected char[] getAcceptedChars() {
                // return null;
                return new char[] { '0', '1', '2', '3', '4', '5', '6', '7',
                        '8', '9' };
            }

            public int getInputType() {
                return android.text.InputType.TYPE_CLASS_NUMBER;
            }

        });
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        if (!isVisibleToUser) {
            hideInputMethod(null);
        }
    }

    @Override
    public void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        initMccMncValue();
        if (mMccMncReadSim && mSimType != SIM_CARD_INVALID
                && mSimMccMnc != null
                && !mSimMccMnc.replace(" ", "").equals("")) {
            mEtMccMnc.setText(mSimMccMnc);
            queryAndShowApn();
        }

    }

    public void initMccMncValue() {
        if (TelephonyManager.getDefault().getPhoneCount() >= 1
                && isSimReady(PhoneConstants.SIM_ID_1)) {
            mSimType = PhoneConstants.SIM_ID_1;
        } else if (TelephonyManager.getDefault().getPhoneCount() > 1
                && isSimReady(PhoneConstants.SIM_ID_2)) {
            mSimType = PhoneConstants.SIM_ID_2;
        } else {
            mSimType = SIM_CARD_INVALID;
        }
        if (mSimType != SIM_CARD_INVALID) {
            int[] SubId = SubscriptionManager.getSubId(mSimType);
            if (SubId != null && SubId.length > 0) {
                mSubId = SubId[0];
            }
            TelephonyManager telephonyManager = (TelephonyManager) getActivity()
                    .getSystemService(getActivity().TELEPHONY_SERVICE);
            mSimMccMnc = telephonyManager.getSimOperator(mSubId);
        }
        if (mSimMccMnc == null) {
            Elog.d(TAG, "Fail to read SIM MCC+MNC!");
        } else {
            Elog.d(TAG, "Read SIM MCC+MNC: " + mSimMccMnc);
        }
    }

    protected boolean isSimReady(int mSlotId) {
        TelephonyManager telephonyManager = (TelephonyManager) getActivity()
                .getSystemService(getActivity().TELEPHONY_SERVICE);
        if (TelephonyManager.SIM_STATE_ABSENT == telephonyManager
                .getSimState(mSlotId)) {
            return false;
        }
        return true;
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        // TODO Auto-generated method stub

    }

    protected Dialog showDialog(int id, String info) {
        switch (id) {
        case DIALOG_MCCMNC_READFAIL: // Fail
            return new AlertDialog.Builder(getActivity())
                    .setMessage(info)
                    .setPositiveButton("OK",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog,
                                        int id) {
                                    mEtMccMnc.setText("");
                                    dialog.cancel();
                                }
                            }).show();
        case DIALOG_MCCMNC_INVALID: // Fail
            return new AlertDialog.Builder(getActivity())
                    .setMessage(info)
                    .setPositiveButton("OK",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog,
                                        int id) {
                                    dialog.cancel();
                                }
                            }).show();
        default:
            return null;
        }
    }

}

class TableInfoAdapter extends ArrayAdapter<String[]> {
    private Activity mActivity;

    public TableInfoAdapter(Activity activity) {
        super(activity, 0);
        mActivity = activity;
    }

    private class ViewHolder {
        public TextView[] texts;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;
        LayoutInflater inflater = mActivity.getLayoutInflater();
        if (convertView == null) {
            convertView = inflater.inflate(R.layout.em_info_entry, null);
            holder = new ViewHolder();
            holder.texts = new TextView[10];
            holder.texts[0] = (TextView) convertView.findViewById(R.id.info1);
            holder.texts[1] = (TextView) convertView.findViewById(R.id.info2);
            holder.texts[2] = (TextView) convertView.findViewById(R.id.info3);
            holder.texts[3] = (TextView) convertView.findViewById(R.id.info4);
            holder.texts[4] = (TextView) convertView.findViewById(R.id.info5);
            holder.texts[5] = (TextView) convertView.findViewById(R.id.info6);
            holder.texts[6] = (TextView) convertView.findViewById(R.id.info7);
            holder.texts[7] = (TextView) convertView.findViewById(R.id.info8);
            holder.texts[8] = (TextView) convertView.findViewById(R.id.info9);
            holder.texts[9] = (TextView) convertView.findViewById(R.id.info10);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        String[] text = getItem(position);
        for (int i = 0; i < text.length; i++) {
            holder.texts[i].setText(text[i]);
            holder.texts[i].setVisibility(View.VISIBLE);
        }
        for (int i = text.length; i < holder.texts.length; i++) {
            holder.texts[i].setVisibility(View.GONE);
        }
        return convertView;
    }
}
