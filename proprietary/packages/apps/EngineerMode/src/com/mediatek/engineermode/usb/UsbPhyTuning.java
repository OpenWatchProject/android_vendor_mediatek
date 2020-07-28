/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.usb;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

/**
 * USB PHY Tuning Activity.
 *
 * @author mtk81238
 *
 */
public class UsbPhyTuning extends Activity implements android.view.View.OnClickListener {
    /**
     * action data.
     *
     * @author mtk81238
     *
     */
    private class ActionData {
        public Spinner spinner;
        public String[] spinnerItems;
        public Button button;
        public String type;
        public String path;

        public ActionData(String actionType, String actionPath, Spinner sp,
                String[] spItems, Button btn) {
            spinner = sp;
            spinnerItems = spItems;
            button = btn;
            type = actionType;
            path = actionPath;
        }
    }

    private static final String TAG = "Usb/PhyTuning";
    private static final String TYPE_USB_DRIVING_CAPABILITY = "USB_DRIVING_CAPABILITY";
    private static final String TYPE_USB_HSTX_SRCTRL = "RG_USB20_HSTX_SRCTRL";
    private static final String TYPE_USB_INTR_EN = "RG_USB20_INTR_EN";
    private static final String TYPE_USB_REV6 = "RG_USB20_PHY_REV6";
    private static final String PATH_USB_PHY = "/sys/kernel/debug/usb20_phy";
    private static final String PATH_USB_DRIVING_CAPABILITY = PATH_USB_PHY + "/"
            + TYPE_USB_DRIVING_CAPABILITY;
    private static final String PATH_USB_HSTX_SRCTRLL = PATH_USB_PHY + "/"
            + TYPE_USB_HSTX_SRCTRL;
    private static final String PATH_USB_INTR_EN = PATH_USB_PHY + "/" + TYPE_USB_INTR_EN;
    private static final String PATH_USB_REV6 = PATH_USB_PHY + "/" + TYPE_USB_REV6;
    private static final int MSG_CHECK_SUBMIT_OPERATION = 10;
    private Spinner mSpDrivingCapability = null;
    private Spinner mSpHstxSrctrl = null;
    private Spinner mSpIntrEn = null;
    private Spinner mSpRev6En = null;
    private Button mBtnDrivingCapability = null;
    private Button mBtnHstxSrctrl = null;
    private Button mBtnIntrEn = null;
    private Button mBtnRev6En = null;
    private String[] mArrBValStr07 = null;
    private String[] mArrDValStr01 = null;
    private String[] mArrDValStr014 = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
            "10", "11", "12", "13", "14" };
    private String[] mArrDValStr003 = { "00", "01", "10", "11"};
    private ActionData[] mActionData = null;

    /**
     * tell whether usb phy path exist.
     *
     * @return if existed, return true, or false
     */
    public static boolean isUsbPhyExist() {
        return new File(PATH_USB_PHY).exists();
    }

    private Handler mMainHandler = new Handler() {
        @Override
        public void handleMessage(android.os.Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
            case MSG_CHECK_SUBMIT_OPERATION:
                handleCheckMsg(msg);
                break;
            default:
                Elog.d(TAG, "Unhandled msg:" + msg.what);
                break;
            }
        }
    };

    private void handleCheckMsg(Message msg) {
        ActionData actionData = (ActionData) msg.obj;
        boolean ret = checkSubmitResult(actionData);
        if (ret) {
            Toast.makeText(this, actionData.type + " execute success", Toast.LENGTH_SHORT)
                    .show();
        } else {
            Toast.makeText(this, actionData.type + " execute fail", Toast.LENGTH_SHORT)
                    .show();
        }
        actionData.button.setEnabled(true);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.usb_phy_tuning);
        mArrBValStr07 = getResources().getStringArray(R.array.usb_phy_tuning_val_0_7b);
        mArrDValStr01 = getResources().getStringArray(R.array.usb_phy_tuning_val_0_1d);
        mSpDrivingCapability = (Spinner) findViewById(R.id.usb_phy_driving_capability_sp);
        mSpHstxSrctrl = (Spinner) findViewById(R.id.usb_phy_hstx_srctrl_sp);
        mSpIntrEn = (Spinner) findViewById(R.id.usb_phy_intr_en_sp);
        mSpRev6En = (Spinner) findViewById(R.id.usb_phy_enhancement_sp);

        mBtnDrivingCapability = (Button) findViewById(R.id.usb_phy_driving_capability_btn);
        mBtnDrivingCapability.setOnClickListener(this);
        mBtnHstxSrctrl = (Button) findViewById(R.id.usb_phy_hstx_srctrl_btn);
        mBtnHstxSrctrl.setOnClickListener(this);
        mBtnIntrEn = (Button) findViewById(R.id.usb_phy_intr_en_btn);
        mBtnIntrEn.setOnClickListener(this);
        mBtnRev6En = (Button) findViewById(R.id.usb_phy_enhancement_btn);
        mBtnRev6En.setOnClickListener(this);

        mActionData = new ActionData[4];
        mActionData[0] = new ActionData(TYPE_USB_DRIVING_CAPABILITY,
                PATH_USB_DRIVING_CAPABILITY, mSpDrivingCapability, mArrDValStr014,
                mBtnDrivingCapability);
        mActionData[1] = new ActionData(TYPE_USB_HSTX_SRCTRL, PATH_USB_HSTX_SRCTRLL,
                mSpHstxSrctrl, mArrBValStr07, mBtnHstxSrctrl);

        mActionData[2] = new ActionData(TYPE_USB_INTR_EN, PATH_USB_INTR_EN, mSpIntrEn,
                mArrDValStr01, mBtnIntrEn);

        mActionData[3] = new ActionData(TYPE_USB_REV6, PATH_USB_REV6, mSpRev6En,
                    mArrDValStr003, mBtnRev6En);

        for (int i = 0; i < mActionData.length; i++) {
            fillSelectSpinner(mActionData[i]);
        }
    }

    private void fillSpinnerItems(Spinner spinner, String[] itemArr) {
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, itemArr);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
    }

    private void fillSelectSpinner(ActionData actionData) {
        Spinner spinner = actionData.spinner;
        String[] spItems = actionData.spinnerItems;
        String path = actionData.path;
        fillSpinnerItems(spinner, spItems);
        String content = readFileContent(path);
        if (content != null) {
            content = content.trim();
        }
        int index = getIdxInStrArr(spItems, content);
        if (index >= 0) {
            spinner.setSelection(index);
        } else {
            Toast.makeText(this,
                    "unknown value " + content + ", when read " + actionData.type,
                    Toast.LENGTH_LONG).show();
        }
    }

    private String readFileContent(String path) {
        StringBuilder builder = new StringBuilder();
        BufferedReader reader = null;
        char[] buffer = new char[1024];
        try {
            reader = new BufferedReader(new FileReader(path));
            while (true) {
                int ret = reader.read(buffer, 0, buffer.length);
                if (ret <= 0) {
                    reader.close();
                    break;
                }
                builder.append(buffer, 0, ret);
            }

        } catch (IOException e) {
            builder.append("Error:").append(e.getMessage());
            Elog.e(TAG, "readFileContent IOException:" + e.getMessage());
        }
        return builder.toString();
    }

    private int getIdxInStrArr(String[] array, String str) {
        int index = -1;
        for (int i = 0; i < array.length; i++) {
            if (array[i].equals(str)) {
                index = i;
                break;
            }
        }
        return index;
    }

    private void onClickButton(Button button) {
        ActionData actionData = null;
        for (int i = 0; i < mActionData.length; i++) {
            if (button == mActionData[i].button) {
                actionData = mActionData[i];
                break;
            }
        }
        if (actionData == null) {
            Elog.e(TAG, "onClickButton Invalid actionData: null");
            return;
        }
        submitSetting(actionData);
    }

    private void submitSetting(ActionData actionData) {
        Button btn = actionData.button;
        btn.setEnabled(false);

        try {
            String type = actionData.type;
            String value = actionData.spinner.getSelectedItem().toString();
            EmUtils.getEmHidlService().setEmUsbValue(value);
            EmUtils.getEmHidlService().setEmUsbType(type);
            Elog.d(TAG, "set property ok ...");
        } catch (Exception e) {
            Elog.e(TAG, "set property failed ...");
            e.printStackTrace();
        }
        Message msg = Message.obtain();
        msg.what = MSG_CHECK_SUBMIT_OPERATION;
        msg.obj = actionData;
        mMainHandler.sendMessageDelayed(msg, 100);
    }

    private boolean checkSubmitResult(ActionData actionData) {
        boolean result = false;
        Spinner spinner = null;
        String path = null;
        String[] array = null;
        if (actionData != null) {
            spinner = actionData.spinner;
            path = actionData.path;
            array = actionData.spinnerItems;
        }
        if (spinner != null && path != null && array != null) {
            String content = readFileContent(path);
            if (content != null) {
                content = content.trim();
            }
            String selectVal = spinner.getSelectedItem().toString();
            if (selectVal.equals(content)) {
                result = true;
            } else {
                Elog.d(TAG, "submit fail, submited value:" + selectVal + ", read value:"
                        + content);
                // execute fail, reset spinner consistent with system value
                int index = getIdxInStrArr(array, content);
                if (index >= 0) {
                    spinner.setSelection(index);
                }
            }
        }
        return result;
    }

    @Override
    public void onClick(View view) {
        if (view instanceof Button) {
            Button btn = (Button) view;
            onClickButton(btn);
        }
    }
}
