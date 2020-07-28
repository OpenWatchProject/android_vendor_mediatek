package com.mediatek.engineermode.iotconfig;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;

import android.support.v4.app.Fragment;
import android.text.InputFilter;
import android.text.TextUtils;
import android.text.method.NumberKeyListener;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.TextView.OnEditorActionListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.WifiOffloadManager;

public class WfcConfigFragment extends Fragment implements OnClickListener {

    private static final String TAG = "Iot/WfcConfigFragment";
    public static final int WFC_ACTION_SET_MCCMNC = 0;
    public static final int WFC_ACTION_RESET_MCCMNC = 1;
    public static final int UNKNOW = 2;
    public static final int WFC_SET_MCCMNC_LENGTH_TOO_LONG = 1;
    public static final int WFC_SET_MCCMNC_INPUT_ILLEGAL = 2;
    public static final int WFC_SET_MCCMNC_OK = 3;
    public static final int WFC_SET_MCCMNC_FAILED = 4;
    public static final int WFC_SHOW_EXISTED_MCCMNC_LIST = 0;
    public static final int WFC_SHOW_ADDED_MCCMNC_LIST = 1;
    public static final int WFC_SHOW_ALL_MCCMNC_LIST = 2;
    private Spinner mccMncSpinner;
    private LinearLayout addMccMncLayout;
    private LinearLayout defaultAddMccMncLayout;
    private Button btnAddMccMnc;
    private Button btnResetMccMnc;
    private Button btnAddOk;
    private Button btnAddCancel;
    private MccMncAdapter mAdapter;
    private ListView mccMncListView;
    private IWifiOffloadService mWfoService;
    private EditText inputMccMnc;
    private int currentList = 0;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case WFC_SET_MCCMNC_LENGTH_TOO_LONG:
                break;
            case WFC_SET_MCCMNC_INPUT_ILLEGAL:
                showToast("MCC/MNC input illegal!");
                break;
            case WFC_SET_MCCMNC_OK:
                setAddMccMncLayoutVisibility(false);
                updateUI(currentList);
                showToast("Set/Reset MCC/MNC ok!");
                break;
            case WFC_SET_MCCMNC_FAILED:
                showToast("Set MCC/MNC failed!");
                break;
            default:
                break;
            }
        }
    };

    public void setAddMccMncLayoutVisibility(boolean visibility) {
        if (visibility) {
            String mccMncContent = "";
            if(getMccMncList(WFC_SHOW_ADDED_MCCMNC_LIST) != null){
                mccMncContent = TextUtils.join(",",
                    getMccMncList(WFC_SHOW_ADDED_MCCMNC_LIST));
            }
            inputMccMnc.setText(mccMncContent);
            addMccMncLayout.setVisibility(View.VISIBLE);
            defaultAddMccMncLayout.setVisibility(View.GONE);
            limitEditText(inputMccMnc, 92);
        } else {
            inputMccMnc.setText("");
            addMccMncLayout.setVisibility(View.GONE);
            defaultAddMccMncLayout.setVisibility(View.VISIBLE);
            hideInputMethod(inputMccMnc);
        }
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
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        View view = inflater.inflate(R.layout.iot_wfc_config, container, false);
        initView(view);
        return view;
    }

    private void initView(View view) {
        // TODO Auto-generated method stub
        mccMncSpinner = (Spinner) view.findViewById(R.id.mccmnc_list_spinner);
        btnAddMccMnc = (Button) view.findViewById(R.id.add_mccmnc);
        btnResetMccMnc = (Button) view.findViewById(R.id.reset_mccmnc);
        mccMncListView = (ListView) view.findViewById(R.id.mccmnc_list);
        addMccMncLayout = (LinearLayout) view
                .findViewById(R.id.add_mccmnc_layout);
        defaultAddMccMncLayout = (LinearLayout) view
                .findViewById(R.id.reset_mccmnc_layout);
        btnAddOk = (Button) view.findViewById(R.id.add_mccmnc_ok);
        btnAddCancel = (Button) view.findViewById(R.id.add_mccmnc_cancel);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(
                getActivity(), R.array.mccmnc_display_mode,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mccMncSpinner.setAdapter(adapter);

        AdapterView.OnItemSelectedListener l = new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view,
                    int position, long id) {
                // set the regulation of EditText
                switch (position) {
                case WFC_SHOW_ALL_MCCMNC_LIST:
                    updateUI(WFC_SHOW_ALL_MCCMNC_LIST);
                    break;
                case WFC_SHOW_ADDED_MCCMNC_LIST:
                    updateUI(WFC_SHOW_ADDED_MCCMNC_LIST);
                    break;
                case WFC_SHOW_EXISTED_MCCMNC_LIST:
                    updateUI(WFC_SHOW_EXISTED_MCCMNC_LIST);
                    break;
                default:
                    break;
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        };
        inputMccMnc = (EditText) view.findViewById(R.id.wfc_input_mccmnc);
        setAddMccMncLayoutVisibility(false);
        mccMncSpinner.setOnItemSelectedListener(l);
        IBinder b = ServiceManager.getService(WifiOffloadManager.WFO_SERVICE);
        mWfoService = IWifiOffloadService.Stub.asInterface(b);
        mAdapter = new MccMncAdapter(getActivity(), getMccMncList(currentList));
        mccMncListView.setAdapter(mAdapter);
        btnAddMccMnc.setOnClickListener(this);
        btnResetMccMnc.setOnClickListener(this);
        btnAddOk.setOnClickListener(this);
        btnAddCancel.setOnClickListener(this);
        inputMccMnc.setOnEditorActionListener(new OnEditorActionListener() {

            @Override
            public boolean onEditorAction(TextView v, int actionId,
                    KeyEvent event) {
                // TODO Auto-generated method stub
                if ((event != null
                        && (event.getKeyCode() == KeyEvent.KEYCODE_ENTER))
                        || (actionId == EditorInfo.IME_ACTION_DONE)) {
                    hideInputMethod(inputMccMnc);
                }
                return false;
            }
        });
    }

    public void limitEditText(EditText et, int limitLength) {
        et.setFilters(new InputFilter[] { new InputFilter.LengthFilter(
                limitLength) });
        et.setKeyListener(new NumberKeyListener() {
            @Override
            protected char[] getAcceptedChars() {
                return new char[] { '0', '1', '2', '3', '4', '5', '6', '7',
                        '8', '9', ',' };
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
    public void onClick(View v) {
        // TODO Auto-generated method stub
        switch (v.getId()) {
        case R.id.add_mccmnc_ok:
            String mccMncContent = inputMccMnc.getText().toString()
                    .replace(" ", "");
            if (mccMncContent != null && !mccMncContent.equals("")) {
                new SetMccMncThread(WFC_ACTION_SET_MCCMNC, mccMncContent)
                        .start();
            }
            break;
        case R.id.add_mccmnc_cancel:
            setAddMccMncLayoutVisibility(false);
            break;
        case R.id.add_mccmnc:
            setAddMccMncLayoutVisibility(true);
            break;
        case R.id.reset_mccmnc:
            mccMncContent = "";
            new SetMccMncThread(WFC_ACTION_RESET_MCCMNC, mccMncContent).start();
            break;
        }

    }

    public class SetMccMncThread extends Thread {
        private int action;
        private String mccMncContent;

        public SetMccMncThread(int action, String mccMncContent) {
            this.action = action;
            this.mccMncContent = mccMncContent;
        }

        public void run() {
            Message msg = new Message();
            switch (this.action) {
            case WFC_ACTION_SET_MCCMNC:
                if (mccMncContent.length() > 92) {
                    msg.what = WFC_SET_MCCMNC_LENGTH_TOO_LONG;
                } else {
                    String[] mccMncArray = this.mccMncContent.split(",");
                    if (checkInputMccMnc(mccMncArray)) {
                        boolean setMccMncOk = false;
                        try {
                            setMccMncOk = mWfoService
                                    .setMccMncAllowList(mccMncArray);
                        } catch (RemoteException e) {
                            e.printStackTrace();
                        }
                        if (setMccMncOk) {
                            msg.what = WFC_SET_MCCMNC_OK;
                        } else {
                            msg.what = WFC_SET_MCCMNC_FAILED;
                        }
                    } else {
                        msg.what = WFC_SET_MCCMNC_INPUT_ILLEGAL;
                    }
                }
                break;
            case WFC_ACTION_RESET_MCCMNC:
                boolean setMccMncOk = false;
                try {
                    setMccMncOk = mWfoService.setMccMncAllowList(null);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                if (setMccMncOk) {
                    msg.what = WFC_SET_MCCMNC_OK;
                } else {
                    msg.what = WFC_SET_MCCMNC_FAILED;
                }
                break;
            default:
                msg.what = UNKNOW;
                break;
            }
            mHandler.sendMessage(msg);
        }

    }

    public void showToast(String content) {
        Toast.makeText(getActivity(), content, Toast.LENGTH_SHORT).show();
    }

    public boolean checkInputMccMnc(String[] mccMncArray) {
        for (String mccMnc : mccMncArray) {
            if ((5 > mccMnc.length()) || (6 < mccMnc.length())) {
                return false;
            }
        }
        return true;
    }

    public ArrayList<String> getMccMncList(int type) {
        ArrayList<String> mMccMncList = new ArrayList<String>();
        if (mWfoService != null) {
            try {
                String[] mccMncResult = mWfoService.getMccMncAllowList(type);
                if(mccMncResult != null){
                    mMccMncList = new ArrayList<String>(Arrays.asList(mccMncResult));
                    Elog.d(TAG, "Read MccMncList : " + mMccMncList.toString());
                } else {
                    return null;
                }
            } catch (RemoteException e) {
                Elog.e(TAG, "RemoteException ImsCallSessionProxy()");
            }
        }
        return mMccMncList;

    }

    public void updateUI(int type) {
        currentList = type;
        if (mAdapter == null) {
            mAdapter = new MccMncAdapter(getActivity(), getMccMncList(type));
        } else {
            mAdapter.clear();
            if( getMccMncList(type) != null ){
                for (String mccMnc : getMccMncList(type)) {
                    mAdapter.add(mccMnc);
                }
            }
            mAdapter.refresh(getMccMncList(type));
        }
        mccMncListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
    }

    @Override
    public void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        updateUI(currentList);
    }

}

class MccMncAdapter extends ArrayAdapter<String> {

    private ArrayList<String> mccMncList;
    private final Activity context;
    int listPosititon;

    public MccMncAdapter(Activity context, ArrayList<String> mccMncList2) {
        super(context, R.layout.iot_wfc_row);
        this.context = context;
        this.mccMncList = mccMncList2;
    }

    public void refresh(ArrayList<String> mccMncList) {
        this.mccMncList = mccMncList;
        notifyDataSetChanged();
    }

    public ArrayList<String> getList() {
        return this.mccMncList;
    }

    static class ViewHolder {
        protected TextView text;
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        listPosititon = position;
        ViewHolder viewHolder = null;
        if (convertView == null) {
            LayoutInflater inflator = context.getLayoutInflater();
            convertView = inflator.inflate(R.layout.iot_wfc_row, null);
            viewHolder = new ViewHolder();
            viewHolder.text = (TextView) convertView
                    .findViewById(R.id.display_mccmnc);

            convertView.setTag(viewHolder);
            convertView.setTag(R.id.display_mccmnc, viewHolder.text);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        if(mccMncList != null) {
            viewHolder.text.setText(mccMncList.get(position));
        } else {
            viewHolder.text.setText("");
        }
        return convertView;
    }
}
