package com.mediatek.engineermode.ims;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.InputType;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class imsSettingPageActivity extends Activity implements OnClickListener {

    private static final String TAG = "Ims/imsSettingPage";


    private static final int TYPE_NUMBER = 0;
    private static final int TYPE_TEXT = 1;
    private static final int TYPE_SINGLE = 2;
    private static final int TYPE_MULTI = 3;
    private static final int TYPE_SPINNER = 4;
    private static final int MSG_QUERY = 0;
    private static final int MSG_SET = 1;
    private static final int DIALOG_MANUAL = 1;
    private static final String PROP_IMS_PCT_CONFIG = "vendor.ril.volte.mal.pctid";
    private ArrayList<Setting> mSetting = new ArrayList<Setting>();
    private ArrayList<SettingView> mSettingView = new ArrayList<SettingView>();
    private ViewGroup mList;
    private int mSimType;
    private String mSettingRule =
            "Setting Rule:<digit of list num><list num><mnc_len><MNC><mcc_len><MCC>...";

    private Toast mToast;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == MSG_QUERY) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    String[] data = (String[]) ar.result;
                    if ((data != null) && (data.length > 0) && (data[0] != null)) {
                        ((SettingView) ar.userObj).setValue(parseCommandResponse(data[0]));
                        if (((SettingView) ar.userObj).setting.label.equals("mncmcc check")) {
                            if (parseCommandResponse(data[0]).equals("1")) {
                                sendCommand("mncmcc_whitelist", getSettingView("white list"));
                            }
                        }
                        if (((SettingView) ar.userObj).setting.label.equals("white list")) {
                            ((SettingView) ar.userObj).label.setText("white list: \n"
                                    + parseCommandResponse(data[0]));
                        }
                    }
                } else {
                    Toast.makeText(imsSettingPageActivity.this,
                            "Query failed for " + ((SettingView) ar.userObj).setting.label,
                            Toast.LENGTH_SHORT).show();
                }
            } else if (msg.what == MSG_SET) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    showToast("Set successful.");
                    Elog.d(TAG, "Set successful.");
                } else {
                    showToast("Set failed.");
                    Elog.d(TAG, "Set failed.");
                }
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ims_setting);
        mList = (ViewGroup) findViewById(R.id.ims_setting_view);
        mSetting = (ArrayList<Setting>) getIntent().getSerializableExtra("mSettingDisplay");
        mSimType = getIntent().getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        Elog.d(TAG, "mSimType " + mSimType);
        initializeViews();
    }

    private void initializeViews() {
        mList.removeAllViews();
        for (Setting setting : mSetting) {
            SettingView view = null;
            switch (setting.getType()) {
                case TYPE_NUMBER:
                case TYPE_TEXT:
                    view = new SettingEditTextView(this, setting, setting.getType());
                    break;
                case TYPE_SINGLE:
                    view = new SettingSingleSelectView(this, setting);
                    break;
                case TYPE_MULTI:
                    view = new SettingMultiSelectView(this, setting);
                    break;
                case TYPE_SPINNER:
                    view = new SettingSpinnerSelectView(this, setting);
                    break;
                default:
                    break;
            }
            if (view != null) {
                if (!(setting.getLabel().equals("manual_impi")
                        || setting.getLabel().equals("manual_impu")
                        || setting.getLabel().equals("manual_domain_name"))) {
                    mList.addView(view);
                }
                mSettingView.add(view);
            }
        }
    }

    @Override
    public void onClick(final View arg0) {
        Elog.d(TAG, "onClick");

    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {
            case DIALOG_MANUAL:
                LayoutInflater inflater = getLayoutInflater();
                View dialogView = inflater.inflate(R.layout.ims_dialog_config, null);
                ViewGroup list = (ViewGroup) dialogView.findViewById(R.id.ims_item_list);
                final ArrayList<SettingView> originalViews = new ArrayList<SettingView>();
                final ArrayList<SettingView> views = new ArrayList<SettingView>();
                for (SettingView s : mSettingView) {
                    if (s.setting.getLabel().equals("manual_impi")
                            || s.setting.getLabel().equals("manual_impu")
                            || s.setting.getLabel().equals("manual_domain_name")) {
                        SettingView v = new SettingEditTextView(this, s.setting, TYPE_TEXT);
                        v.setValue(s.getValue());
                        originalViews.add(s);
                        views.add(v);
                        v.findViewById(R.id.ims_config_set).setVisibility(View.GONE);
                        list.addView(v);
                    }
                }
                return new AlertDialog.Builder(this)
                        .setTitle("Manual Settings")
                        .setView(dialogView)
                        .setPositiveButton("OK",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int whichButton) {
                                        for (int i = 0; i < views.size(); i++) {
                                            views.get(i).send();
                                            originalViews.get(i).setValue(views.get(i).getValue());
                                        }
                                        originalViews.get(0).requestFocus();
                                        sendCommand("force_user_account_by_manual", "1");
                                    }
                                })
                        .setNegativeButton("Cancel", null).create();
            default:
                return super.onCreateDialog(id);
        }
    }

    private void sendCommandWansOpId(String name, String value) {
        Message msg = mHandler.obtainMessage(MSG_SET);
        Elog.d(TAG, "AT+EIWLCFGSET=\"" + name + "\"," + "\"" + value + "\"");
        EmUtils.invokeOemRilRequestStringsEm(mSimType,
                new String[]{"AT+EIWLCFGSET=\"" + name + "\"," + "\"" + value + "\"", ""}, msg);
    }

    private void sendCommand(String name, String value) {
        Message msg = mHandler.obtainMessage(MSG_SET);
        Elog.d(TAG, "AT+ECFGSET=\"" + name + "\"," + "\"" + value + "\"");
        EmUtils.invokeOemRilRequestStringsEm(mSimType,
                new String[]{"AT+ECFGSET=\"" + name + "\"," + "\"" + value + "\"", ""}, msg);
    }
    private void sendCommand(String name) {
        Message msg = mHandler.obtainMessage(MSG_SET);
        Elog.d(TAG, "AT+ECFGSET=\"" + name + "\"");
        EmUtils.invokeOemRilRequestStringsEm(mSimType,
                new String[]{"AT+ECFGSET=\"" + name + "\"", ""}, msg);
    }

    private void sendCommand(String name, View obj) {
        Message msg = mHandler.obtainMessage(MSG_QUERY, obj);
        Elog.d(TAG, "AT+ECFGGET=\"" + name + "\"");
        EmUtils.invokeOemRilRequestStringsEm(mSimType,
                new String[]{"AT+ECFGGET=\"" + name + "\"", "+ECFGGET:"}, msg);
    }

    private String parseCommandResponse(String data) {
        Elog.d(TAG, "raw data: " + data);
        Pattern p = Pattern.compile("\\+ECFGGET:\\s*\".*\"\\s*,\\s*\"(.*)\"");
        Matcher m = p.matcher(data);
        while (m.find()) {
            String value = m.group(1);
            Elog.d(TAG, "value: " + value);
            return value;
        }
        Elog.d(TAG, "wrong format: " + data);
        showToast("wrong format: " + data);
        return "";
    }

    private int parseInt(String s) {
        try {
            return Integer.parseInt(s);
        } catch (NumberFormatException e) {
            showToast("Wrong integer format: " + s);
            return -1;
        }
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    private SettingView getSettingView(String label) {
        for (SettingView settingView : mSettingView) {
            if (settingView.setting.label.equals(label)) {
                return settingView;
            }
        }
        return null;
    }

    /**
     * SettingView class.
     */
    private abstract class SettingView extends FrameLayout {
        public Setting setting;
        public Button button;
        public TextView label;

        public SettingView(Context context, Setting setting) {
            super(context);
            LayoutInflater inflater = getLayoutInflater();
            View convertView = inflater.inflate(R.layout.ims_config_view, null);
            addView(convertView);
            this.setting = setting;
            label = (TextView) findViewById(R.id.ims_config_label);
            TextView suffix = (TextView) findViewById(R.id.ims_config_suffix);
            button = (Button) findViewById(R.id.ims_config_set);
            label.setText(setting.label + ":");
            suffix.setText(setting.suffix);

            if (setting.label.equals("white list")) {
                Elog.d(TAG, "setting.label" + setting.label);
                button.setVisibility(View.GONE);
            }
            button.setOnClickListener(new OnClickListener() {
                public void onClick(View view) {
                    send();
                }
            });
            if (setting.label.equals("Authentication")) {
                sendCommand("UA_reg_http_digest", this);
            } else if (setting.label.equals("Security")) {
                sendCommand("UA_net_ipsec", this);
            } else if (setting.label.equals("USSD Support")) {
                sendCommand("ussd_support", this);
            } else if (setting.label.equals("USSD NW Timeout")) {
                sendCommand("ussd_nw_timeout_timer", this);
            } else if (setting.label.equals("USSD Setup Timeout")) {
                sendCommand("ussd_setup_timeout_timer", this);
            } else if (setting.label.equals("mncmcc check")) {
                sendCommand("mncmcc_pass_flag", this);
            } else if (setting.label.equals("white list")) {
                Elog.d(TAG, "Don't do anything!");
            } else if (setting.label.equals(mSettingRule)) {
                Elog.d(TAG, "Don't do anything!");
            } else if (setting.label.equals("operator_code_textview")) {
                sendCommand("operator_code", this);
            }  else if (setting.label.equals("reset_ims_to_default")) {
                Elog.d(TAG, "Don't do anything!");
            }
            else {
                sendCommand(setting.label, this);
            }
        }

        public void send() {

            if (setting.label.equals("Authentication")) {
                sendCommand("UA_reg_http_digest", getValue());
            } else if (setting.label.equals("force_user_account_by_manual")
                    && getValue().equals("1")) {
                showDialog(DIALOG_MANUAL);
            } else if (setting.label.equals("Security")) {
                sendCommand("UA_net_ipsec", getValue());
            } else if (setting.label.equals("USSD Support")) {
                sendCommand("ussd_support", getValue());
            } else if (setting.label.equals("USSD NW Timeout")) {
                sendCommand("ussd_nw_timeout_timer", getValue());
            } else if (setting.label.equals("USSD Setup Timeout")) {
                sendCommand("ussd_setup_timeout_timer", getValue());
            } else if (setting.label.equals("mncmcc check")) {
                Elog.d(TAG, "button.getText().toString()" + button.getText().toString());
                sendCommand("mncmcc_pass_flag", getValue());
            } else if (setting.label.equals("white list")) {
                sendCommand("mncmcc_whitelist", this);
            } else if (setting.label.equals(mSettingRule)) {
                sendCommand("mncmcc_whitelist", getValue());
            } else if (setting.label.equals("ch_send")) {
                int chsend = parseInt(getValue());
                if (chsend < 1 || chsend > 10) {
                    showToast("The ch_send should be 1~10, please reset it");
                } else {
                    sendCommand(setting.label, getValue());
                }
            } else if (setting.label.equals("ch_recv")) {
                int chrecv = parseInt(getValue());
                if (chrecv < 1 || chrecv > 10) {
                    showToast("The ch_recv should be 1~10, please reset it");
                } else {
                    sendCommand(setting.label, getValue());
                }
            }else if(setting.label.equals("reset_ims_to_default")){
                sendCommand(setting.label);
            }
            else if (setting.label.equals("operator_code_textview")) {
                sendCommand("operator_code", getValue());
            } else {
                sendCommand(setting.label, getValue());
                if (setting.label.equals("operator_code")) {
                    String info = getValue();
                    if (getValue().equals("16387")) {
                        info = "OP16387";
                        sendCommandWansOpId("wans_op_id",getValue());
                    }
                    try {
                        EmUtils.getEmHidlService().setVolteMalPctid(info);
                    } catch (Exception e) {
                        e.printStackTrace();
                        Elog.e(TAG, "set property failed ...");
                    }
                }
            }
        }

        abstract protected String getValue();

        abstract protected void setValue(String value);
    }

    /**
     * SettingEditTextView class.
     */
    private class SettingEditTextView extends SettingView {
        public EditText mEditText;

        public SettingEditTextView(Context context, Setting setting, int type) {
            super(context, setting);
            mEditText = (EditText) findViewById(R.id.ims_config_edit_text);
            if (type == TYPE_NUMBER) {
                mEditText.setInputType(InputType.TYPE_CLASS_NUMBER);
            }
            findViewById(R.id.ims_config_edit_layout).setVisibility(View.VISIBLE);
            mEditText.setText(setting.defaultValue);
            if (setting.label.equals("white list")) {
                mEditText.setVisibility(View.GONE);
            }
        }

        protected String getValue() {
            return mEditText.getText().toString();
        }

        protected void setValue(String value) {
            mEditText.setText(value);
        }
    }

    /**
     * SettingSingleSelectView class.
     */
    private class SettingSingleSelectView extends SettingView {
        private RadioButton[] mRadios;
        private RadioGroup mRadioGroup;

        public SettingSingleSelectView(Context context, Setting setting) {
            super(context, setting);
            mRadioGroup = (RadioGroup) findViewById(R.id.ims_config_radio_group);
            mRadioGroup.setVisibility(View.VISIBLE);
            mRadioGroup.removeAllViews();
            mRadios = new RadioButton[setting.entries.size()];
            for (int i = 0; i < setting.entries.size(); i++) {
                RadioButton radio = new RadioButton(imsSettingPageActivity.this);
                radio.setText(setting.entries.get(i));
                radio.setTag(setting.values.get(i));
                mRadioGroup.addView(radio);
                mRadios[i] = radio;
                if (setting.values.get(i) == parseInt(setting.defaultValue)) {
                    mRadioGroup.check(radio.getId());
                }
            }
        }

        protected String getValue() {
            for (int i = 0; i < mRadios.length; i++) {
                if (mRadios[i].isChecked()) {
                    return String.valueOf((Integer) mRadios[i].getTag());
                }
            }
            return "";
        }

        protected void setValue(String value) {
            Elog.i(TAG, "setValue " + setting.label + ", " + value);
            int integerValue = parseInt(value);
            for (int i = 0; i < mRadios.length; i++) {
                if (integerValue == (Integer) mRadios[i].getTag()) {
                    mRadioGroup.check(mRadios[i].getId());
                }
            }
        }
    }

    /**
     * SettingSpinnerSelectView class.
     */
    private class SettingSpinnerSelectView extends SettingView {
        public Spinner mSpinner;
        public ArrayList<String> mListData;
        public int[] mAdapterValues;

        public SettingSpinnerSelectView(Context context, Setting setting) {
            super(context, setting);
            mSpinner = (Spinner) findViewById(R.id.ims_config_spinner);
            findViewById(R.id.ims_config_spinner_layout).setVisibility(View.VISIBLE);
            mListData = new ArrayList<String>();
            mAdapterValues = new int[setting.entries.size()];
            for (int i = 0; i < setting.entries.size(); i++) {
                mListData.add(setting.entries.get(i));
                mAdapterValues[i] = setting.values.get(i);
            }
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(imsSettingPageActivity.this,
                    android.R.layout.simple_spinner_item, mListData);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mSpinner.setAdapter(adapter);
            for (int j = 0; j < setting.entries.size(); j++) {
                if (mAdapterValues[j] == parseInt(setting.defaultValue)) {
                    mSpinner.setSelection(j, true);
                }
            }
        }

        protected String getValue() {
            int id = mSpinner.getSelectedItemPosition();
            if (id >= 0 && id < mAdapterValues.length) {
                return String.valueOf(mAdapterValues[id]);
            }
            return "";

        }

        protected void setValue(String value) {
            Elog.d(TAG, "setValue " + setting.label + ", " + value);
            int integerValue = parseInt(value);
            for (int i = 0; i < mAdapterValues.length; i++) {
                if (integerValue == mAdapterValues[i]) {
                    mSpinner.setSelection(i, true);
                }
            }
        }
    }

    /**
     * SettingMultiSelectView class.
     */
    private class SettingMultiSelectView extends SettingView {
        private CheckBox[] mCheckboxes;

        public SettingMultiSelectView(Context context, Setting setting) {
            super(context, setting);
            ViewGroup checkboxList = (ViewGroup) findViewById(R.id.ims_config_checkbox_list);
            checkboxList.setVisibility(View.VISIBLE);
            checkboxList.removeAllViews();
            mCheckboxes = new CheckBox[setting.entries.size()];
            for (int i = 0; i < setting.entries.size(); i++) {
                CheckBox checkbox = new CheckBox(imsSettingPageActivity.this);
                checkbox.setText(setting.entries.get(i));
                checkbox.setTag(setting.values.get(i));
                if ((setting.values.get(i) & parseInt(setting.defaultValue)) > 0) {
                    checkbox.setChecked(true);
                }
                checkboxList.addView(checkbox);
                mCheckboxes[i] = checkbox;
            }
        }

        protected String getValue() {
            int checked = 0;
            for (int i = 0; i < mCheckboxes.length; i++) {
                if (mCheckboxes[i].isChecked()) {
                    checked |= (Integer) mCheckboxes[i].getTag();
                }
            }
            return String.valueOf(checked);
        }

        protected void setValue(String value) {
            int integerValue = parseInt(value);
            for (int i = 0; i < mCheckboxes.length; i++) {
                if (integerValue > 0 && (integerValue & ((Integer) mCheckboxes[i].getTag())) > 0) {
                    mCheckboxes[i].setChecked(true);
                } else {
                    mCheckboxes[i].setChecked(false);
                }
            }
        }
    }

}
