/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.anttunerdebug;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Spinner;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

public class AntTunerDebugBPI extends Activity implements OnClickListener,
        OnCheckedChangeListener {
    public static final String TAG = "AntTunerDebugBPI";

    public static final int OP_BPI_READ = 0;
    public static final int OP_BPI_WRITE = 1;
    private static final int OP_BPI_SIGNALING = 2;
    private static final int OP_BPI_NON_SIGNALING = 3;
    private static final String RESPONSE_CMD = "+ERFTX: ";

    private EditText mEdBpiData1;
    private EditText mEdBpiData2;

    private Button mBtnBpiRead;
    private Button mBtnBpiWrite;

    private String mBpiData1 = null;
    private String mBpiData2 = null;
    private String mBpiMode = null;

    ArrayAdapter<String> adapterPattern = null;
    private Spinner mSpQuantityType;
    private RadioGroup groupBpiCode;
    private Button btnNonSignaling;
    private Button btnSignaling;
    private TableRow bpiTableRowData1;
    private TableRow bpiTableRowData2;
    private TableRow bpiTableRowData3;
    private ListView bpiListView;
    private BpiDataAdapter bpiAdapter;
    private boolean showBinary = false;
    private int bpiQuantity = 0;
    private final static int BPI_QUANTITY_BIT = 32;

    private String[] bpiQuantityType = new String[BPI_QUANTITY_BIT];
    private String[] bpiDatas = new String[] { null, null };
    List<BpiBinaryData> bpiBinaryList = new ArrayList<BpiBinaryData>();

    private void handleQuery(String[] result) {
        if (result != null && result.length > 0) {
            Elog.v(TAG, "Modem return: " + result[0]);
            String[] values = result[0]
                    .substring(RESPONSE_CMD.length(), result[0].length())
                    .trim().split(",");
            if (values != null && values.length > 0) {
                if (values[2] == null && values[3] == null) {
                    return;
                }
                bpiDatas[0] = values[2];
                bpiDatas[1] = values[3];
                updateBpiData();
            }
        } else {
            Elog.e(TAG, "Modem return null");
        }
    }
    public void updateBpiData() {
        if (showBinary) {
            bpiTableRowData1.setVisibility(View.GONE);
            bpiTableRowData2.setVisibility(View.GONE);
            bpiTableRowData3.setVisibility(View.VISIBLE);
            bpiListView.setVisibility(View.VISIBLE);
            this.findViewById(R.id.BpiBinarySpiner).setVisibility(View.VISIBLE);
            this.findViewById(R.id.BpiHexSpiner).setVisibility(View.VISIBLE);
        } else {
            bpiTableRowData1.setVisibility(View.VISIBLE);
            bpiTableRowData2.setVisibility(View.VISIBLE);
            bpiTableRowData3.setVisibility(View.GONE);
            bpiListView.setVisibility(View.GONE);
            this.findViewById(R.id.BpiBinarySpiner).setVisibility(View.GONE);
            this.findViewById(R.id.BpiHexSpiner).setVisibility(View.VISIBLE);
        }
        if (bpiDatas[0] == null && bpiDatas[1] == null) {
            return;
        }
        String[] bpiHexDatas = new String[] { "", "" };
        String[] bpiBinaryDatas = new String[] { "", "" };
        List<BpiBinaryData> bpiList = bpiAdapter.getList();
        if (bpiDatas[0] != null) {
            bpiHexDatas[0] = Long.toHexString(Long.parseLong(bpiDatas[0]));
            bpiBinaryDatas[0] = Long
                    .toBinaryString(Long.parseLong(bpiDatas[0]));
            bpiBinaryDatas[0] = new StringBuffer(bpiBinaryDatas[0]).reverse()
                    .toString();
            Elog.d(TAG, "bpiBinaryDatas[0]" + bpiBinaryDatas[0]
                    + ",bpiHexDatas[0]" + bpiHexDatas[0]);
            mEdBpiData1.setText(bpiHexDatas[0]);
            for (int i = 0; i < bpiList.size(); i++) {
                int pin = bpiList.get(i).getValue().equals("") ? -1 : Integer
                        .valueOf(bpiList.get(i).getValue());
                if (pin != -1 && pin < bpiBinaryDatas[0].length()) {
                    BpiBinaryData bpiBinaryData;
                    if (bpiBinaryDatas[0].charAt(pin) == '1') {
                        bpiBinaryData = new BpiBinaryData("BPI Pin#", pin, true);
                    } else {
                        bpiBinaryData = new BpiBinaryData("BPI Pin#", pin,
                                false);
                    }
                    bpiList.set(i, bpiBinaryData);
                }
            }
        }
        if (bpiDatas[1] != null && !bpiDatas[1].equals("0")
                && !bpiDatas[1].replace("", " ").equals("")) {
            bpiHexDatas[1] = Long.toHexString(Long.parseLong(bpiDatas[1]));
            bpiBinaryDatas[1] = Long
                    .toBinaryString(Long.parseLong(bpiDatas[1]));
            bpiBinaryDatas[1] = new StringBuffer(bpiBinaryDatas[1]).reverse()
                    .toString();
            mEdBpiData2.setText(bpiHexDatas[1]);
            for (int i = 0; i < bpiBinaryDatas[1].length(); i++) {
                BpiBinaryData bpiBinaryData;
                if (bpiBinaryDatas[1].charAt(i) == '1') {
                    bpiBinaryData = new BpiBinaryData("BPI Pin#", i + 32, true);
                } else {
                    bpiBinaryData = new BpiBinaryData("BPI Pin#", i + 32);
                }
                bpiBinaryList.add(i + 32, bpiBinaryData);
            }
            Elog.d(TAG, "bpiBinaryDatas[1]" + bpiBinaryDatas[1]
                    + ",bpiHexDatas[1]" + bpiHexDatas[1]);
        } else if (bpiDatas[1] != null && !showBinary) {
            bpiHexDatas[1] = Long.toHexString(Long.parseLong(bpiDatas[1]));
            mEdBpiData2.setText(bpiHexDatas[1]);
        }
        updateListView(bpiList);
        for (BpiBinaryData bpiData : bpiList) {
            Elog.d(TAG, "bpiList:" + bpiData.toString());
        }
        for (BpiBinaryData bpiData : bpiBinaryList) {
            Elog.d(TAG, "bpiBinaryList:" + bpiData.toString());
        }
    }

    private final Handler mATHandler = new Handler() {
        private String[] mReturnData = new String[2];

        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            if (msg.what == OP_BPI_READ) {
                if (ar.exception == null) {
                    Elog.i(TAG, "BPI read successful.");
                    mReturnData = (String[]) ar.result;
                    Elog.d(TAG, "mReturnData = " + mReturnData[0]);
                    EmUtils.showToast(mReturnData[0], Toast.LENGTH_SHORT);
                    handleQuery(mReturnData);
                } else {
                    EmUtils.showToast("BPI read failed.", Toast.LENGTH_SHORT);
                    Elog.e(TAG, "BPI read failed.");
                }
            } else if (msg.what == OP_BPI_WRITE) {
                if (ar.exception == null) {
                    Elog.d(TAG, "BPI write successful.");
                } else {
                    EmUtils.showToast("BPI write failed.", Toast.LENGTH_SHORT);
                    Elog.e(TAG, "BPI write failed.");
                }
            } else if (msg.what == OP_BPI_SIGNALING) {
                if (ar.exception == null) {
                    Elog.d(TAG, "BPI Signaling successful.");
                } else {
                    EmUtils.showToast("BPI Signaling failed.", Toast.LENGTH_SHORT);
                    Elog.e(TAG, "BPI Signaling failed.");
                }
            } else if (msg.what == OP_BPI_NON_SIGNALING) {
                if (ar.exception == null) {
                    Elog.d(TAG, "BPI Non-Signaling successful.");
                } else {
                    EmUtils.showToast("BPI Non-Signaling failed.", Toast.LENGTH_SHORT);
                    Elog.e(TAG, "BPI Non-Signaling failed.");
                }
            }

        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ant_tuner_debug_bpi);

        mEdBpiData1 = (EditText) findViewById(R.id.ant_tuner_debug_bpi_data1);
        mEdBpiData2 = (EditText) findViewById(R.id.ant_tuner_debug_bpi_data2);

        mBtnBpiRead = (Button) findViewById(R.id.ant_tuner_debug_bpi_read);
        mBtnBpiWrite = (Button) findViewById(R.id.ant_tuner_debug_bpi_write);
        mBtnBpiRead.setOnClickListener(this);
        mBtnBpiWrite.setOnClickListener(this);

        mSpQuantityType = (Spinner) findViewById(R.id.ant_tuner_debug_bpi_quantity);
        adapterPattern = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item,
                initQuantityTypeArray(BPI_QUANTITY_BIT));
        adapterPattern
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpQuantityType.setAdapter(adapterPattern);
        bpiQuantity = BPI_QUANTITY_BIT / 2;
        mSpQuantityType.setSelection(bpiQuantity - 1);
        mSpQuantityType
                .setOnItemSelectedListener(new SpinnerSelectedListener());

        bpiListView = (ListView) findViewById(R.id.ant_tuner_debug_bpi_list);
        bpiBinaryList = initBpiBinaryList(BPI_QUANTITY_BIT);
        bpiAdapter = new BpiDataAdapter(this, bpiBinaryList);
        bpiListView.setAdapter(bpiAdapter);

        groupBpiCode = (RadioGroup) findViewById(R.id.bpi_code_rgroup);
        groupBpiCode.setOnCheckedChangeListener(this);

        btnSignaling = (Button) findViewById(R.id.bpi_signaling);
        btnSignaling.setOnClickListener(this);
        btnNonSignaling = (Button) findViewById(R.id.bpi_non_signaling);
        btnNonSignaling.setOnClickListener(this);

        bpiTableRowData1 = (TableRow) findViewById(R.id.BpiTableRow01);
        bpiTableRowData2 = (TableRow) findViewById(R.id.BpiTableRow02);
        bpiTableRowData3 = (TableRow) findViewById(R.id.BpiTableRow03);
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        // TODO Auto-generated method stub
        switch (checkedId) {
        case R.id.bpi_binary:
            showBinary = true;
            break;
        case R.id.bpi_hex:
            showBinary = false;
            break;
        default:
            break;
        }
        updateBpiData();
    }

    class SpinnerSelectedListener implements OnItemSelectedListener {

        public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
                long arg3) {
            bpiQuantity = arg2 + 1;
            List<BpiBinaryData> bpiList = initBpiBinaryList(bpiQuantity);
            updateListView(bpiList);
        }

        public void onNothingSelected(AdapterView<?> arg0) {
        }
    }

    String[] initQuantityTypeArray(int quantity) {
        String[] bpiQuantityArray = new String[quantity];
        for (int i = 0; i < quantity; i++) {
            bpiQuantityType[i] = String.valueOf(i + 1);
        }
        return bpiQuantityType;
    }

    List<BpiBinaryData> initBpiBinaryList(Object quantity) {
        List<BpiBinaryData> bpiList = new ArrayList<BpiBinaryData>();
        for (int i = 0; i < Integer.valueOf(quantity.toString()); i++) {
            BpiBinaryData bpiBinaryData = new BpiBinaryData("BPI Pin#");
            bpiList.add(bpiBinaryData);
        }
        return bpiList;
    }

    public void updateListView(List<BpiBinaryData> bpiList) {
        bpiAdapter.clear();
        bpiAdapter.addAll(bpiList);
        bpiAdapter.refresh(bpiList);
        bpiAdapter.notifyDataSetChanged();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    public void onClick(View arg0) {
        switch (arg0.getId()) {
        case R.id.ant_tuner_debug_bpi_read:
            if (valueGetAndCheck(0) == true) {
                String[] cmd = new String[2];
                cmd[0] = "AT+ERFTX=12,1,0";
                cmd[1] = "+ERFTX:";
                sendAtCommand(cmd, OP_BPI_READ);
            }
            break;
        case R.id.ant_tuner_debug_bpi_write:
            if (!showBinary && valueGetAndCheck(1)) {
                String[] cmd = new String[2];
                mBpiData1 = Long.toString(Long.parseLong(mBpiData1, 16));
                mBpiData2 = Long.toString(Long.parseLong(mBpiData2, 16));
                cmd[0] = "AT+ERFTX=12,1,1," + mBpiData1 + "," + mBpiData2;
                cmd[1] = "";
                sendAtCommand(cmd, OP_BPI_WRITE);
            } else if (showBinary && binaryValueGetAndCheck(1)) {
                String[] cmd = new String[2];
                cmd[0] = "AT+ERFTX=12,1,1," + bpiDatas[0] + "," + bpiDatas[1];
                cmd[1] = "";
                sendAtCommand(cmd, OP_BPI_WRITE);
            }
            break;
        case R.id.bpi_signaling:
            String[] cmdSignaling = new String[2];
            cmdSignaling[0] = "AT+CFUN=1";
            cmdSignaling[1] = "";
            sendAtCommand(cmdSignaling, OP_BPI_SIGNALING);
            break;
        case R.id.bpi_non_signaling:
            String[] cmdNonSignaling1 = new String[2];
            cmdNonSignaling1[0] = "AT+CFUN=0";
            cmdNonSignaling1[1] = "";
            sendAtCommand(cmdNonSignaling1, OP_BPI_NON_SIGNALING);

            String[] cmdNonSignaling2 = new String[2];
            cmdNonSignaling2[0] = "AT+EGCMD=53";
            cmdNonSignaling2[1] = "";
            sendAtCommand(cmdNonSignaling2, OP_BPI_NON_SIGNALING);
            break;
        default:
            break;
        }
    }

    private void sendAtCommand(String[] command, int msg) {
        Elog.d(TAG, "sendAtCommand() " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(command, mATHandler.obtainMessage(msg));
    }

    private boolean binaryValueGetAndCheck(int flag) {
        List<BpiBinaryData> bpiList = bpiAdapter.getList();
        char bpiData1[] = new char[BPI_QUANTITY_BIT];
        for (int i = 0; i < BPI_QUANTITY_BIT; i++) {
            bpiData1[i] = '0';
        }
        int max = 0;
        for (int i = 0; i < bpiList.size(); i++) {
            int pin = bpiList.get(i).getValue().equals("") ? -1 : Integer
                    .valueOf(bpiList.get(i).getValue());
            max = (pin > max && pin < 32) ? pin : max;
            if (pin < 0 || pin >= BPI_QUANTITY_BIT) {
                EmUtils.showToast("BPI Pin# should be (0-31) ", Toast.LENGTH_SHORT);
                return false;
            }
            if (pin != -1) {
                bpiData1[pin] = bpiList.get(i).isSelected() ? '1' : '0';
            }
            Elog.d(TAG, "pin: " + pin + ", bpiData1[pin]: " + bpiData1[pin]);
        }
        String bpiData = new String(bpiData1).substring(0, max + 1);
        Elog.d(TAG, "max: " + max + ", bpiData: " + bpiData);
        bpiData = new StringBuffer(bpiData).reverse().toString();
        try {
            bpiDatas[0] = Long.toString(Long.parseLong(bpiData, 2));
            bpiDatas[1] = "0";
        } catch (NumberFormatException e) {
            EmUtils.showToast("BPI Pin# should be (0-31)", Toast.LENGTH_SHORT);
            return false;
        }
        return true;
    }

    private boolean valueGetAndCheck(int flag) {
        mBpiData1 = mEdBpiData1.getText().toString();
        mBpiData2 = mEdBpiData2.getText().toString();

        mBpiMode = (flag == 0) ? "0" : "1"; // 0:read 1:write

        if (mBpiMode.equals("1")) {
            if (mBpiData1.equals("")) {
                EmUtils.showToast("Data1 should not be empty", Toast.LENGTH_SHORT);
                return false;
            }

            try {
                Long.parseLong(mBpiData1, 16);
            } catch (NumberFormatException e) {
                EmUtils.showToast("Data1 should be 16 HEX", Toast.LENGTH_SHORT);
                return false;
            }

            if (!mBpiData2.equals("")) {
                try {
                    Long.parseLong(mBpiData2, 16);
                } catch (NumberFormatException e) {
                    EmUtils.showToast("Data2 should be 16 HEX", Toast.LENGTH_SHORT);
                    return false;
                }
            } else {
                mBpiData2 = "0";
            }
        }

        return true;
    }
}

class BpiBinaryData {
    private String name;
    private boolean selected = false;
    private String value = "";

    public String getValue() {
        return this.value;
    }

    public void setValue(String value) {
        this.value = value;
    }

    public void setName(String name) {
        this.name = name;
    }

    public BpiBinaryData(String name, Object value) {
        this.name = name;
        if (value != null) {
            this.value = value.toString();
        }
    }

    public BpiBinaryData(String name, Object value, boolean selected) {
        this.name = name;
        if (value != null) {
            this.value = value.toString();
        }
        this.selected = selected;
    }

    public BpiBinaryData(String name) {
        this.name = name;
        this.value = "";
    }

    public String getName() {
        return name;
    }

    public boolean isSelected() {
        return selected;
    }

    public void setSelected(boolean selected) {
        this.selected = selected;
    }

    public String toString() {
        return this.getName() + "," + this.getValue() + "," + this.isSelected();
    }

}

class BpiDataAdapter extends ArrayAdapter<BpiBinaryData> {

    private List<BpiBinaryData> list;
    private final Activity context;

    public BpiDataAdapter(Activity context, List<BpiBinaryData> list) {
        super(context, R.layout.ant_tuner_debug_bpi_row, list);
        this.context = context;
        this.list = list;
    }

    public void refresh(List<BpiBinaryData> list) {
        this.list = list;
        notifyDataSetChanged();
    }

    public List<BpiBinaryData> getList() {
        return this.list;
    }

    static class ViewHolder {
        protected TextView text;
        protected EditText value;
        protected Spinner type;
    }

    class EditTextWatcher implements TextWatcher {

        protected ViewHolder viewHolder;

        EditTextWatcher(ViewHolder viewHolder) {
            this.viewHolder = viewHolder;
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count,
                int after) {
            // TODO Auto-generated method stub

        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before,
                int count) {
            // TODO Auto-generated method stub

        }

        @Override
        public void afterTextChanged(Editable s) {
            // TODO Auto-generated method stub
            list.get((Integer) viewHolder.value.getTag())
                    .setValue(s.toString());
        }

    }

    class ItemSelectedListener implements OnItemSelectedListener {

        protected ViewHolder viewHolder;

        ItemSelectedListener(ViewHolder viewHolder) {
            this.viewHolder = viewHolder;
        }

        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int pos,
                long id) {
            // TODO Auto-generated method stub
            list.get((Integer) viewHolder.type.getTag()).setSelected(
                    pos == 0 ? false : true);
        }

        @Override
        public void onNothingSelected(AdapterView<?> parent) {
            // TODO Auto-generated method stub
        }

    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder viewHolder = null;
        if (convertView == null) {
            String[] typeArray = new String[] { "Low", "Hign" };
            LayoutInflater inflator = context.getLayoutInflater();
            convertView = inflator.inflate(R.layout.ant_tuner_debug_bpi_row,
                    null);
            viewHolder = new ViewHolder();
            viewHolder.text = (TextView) convertView
                    .findViewById(R.id.bpi_binary_label);
            viewHolder.value = (EditText) convertView
                    .findViewById(R.id.bpi_binary_value);
            viewHolder.type = (Spinner) convertView
                    .findViewById(R.id.bpi_binary_type);
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(context,
                    android.R.layout.simple_spinner_item, typeArray);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            viewHolder.type.setAdapter(adapter);
            viewHolder.value.setTag(position);
            viewHolder.type.setTag(position);
            viewHolder.value.addTextChangedListener(new EditTextWatcher(
                    viewHolder));

            viewHolder.type.setOnItemSelectedListener(new ItemSelectedListener(
                    viewHolder));
            convertView.setTag(viewHolder);
            convertView.setTag(R.id.bpi_binary_label, viewHolder.text);
            convertView.setTag(R.id.bpi_binary_value, viewHolder.value);
            convertView.setTag(R.id.bpi_binary_type, viewHolder.type);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
            viewHolder.value.setTag(position);
            viewHolder.type.setTag(position);
        }

        viewHolder.text.setText((list.get(position) != null) ? list.get(
                position).getName() : "");
        viewHolder.value.setText((list.get(position) != null) ? list.get(
                position).getValue() : "");
        viewHolder.type.setSelection((list.get(position) != null && list.get(
                position).isSelected()) ? 1 : 0);
        return convertView;
    }
}
