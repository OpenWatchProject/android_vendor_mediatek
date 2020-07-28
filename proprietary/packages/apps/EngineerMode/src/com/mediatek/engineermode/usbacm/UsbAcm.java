/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.usbacm;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class UsbAcm extends Activity implements OnClickListener {
    private static final String TAG = "UsbAcm";
    private static final String PROP_USB_ACM = "persist.vendor.radio.port_index";
    private static final String USB_ACM_DISABLE = "";
    private static final int BUTTON_NUM = 4;
    private Button[] mBtnList = new Button[BUTTON_NUM];
    private Button mBtnClose = null;
    private Toast mToast;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.usb_acm);

        mBtnList[0] = (Button) findViewById(R.id.button_acm_open_gs0);
        mBtnList[1] = (Button) findViewById(R.id.button_acm_open_gs1);
        mBtnList[2] = (Button) findViewById(R.id.button_acm_open_gs2);
        mBtnList[3] = (Button) findViewById(R.id.button_acm_open_gs3);
        mBtnClose = (Button) findViewById(R.id.button_acm_close);

        for (int i = 0; i < mBtnList.length; i++) {
            mBtnList[i].setOnClickListener(this);
        }
        mBtnClose.setOnClickListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        String tmp = SystemProperties.get(PROP_USB_ACM);
        Elog.i(TAG, "onresume getprop:" + tmp);
        if (!USB_ACM_DISABLE.equals(tmp)) {
            enableAllButton(false);
        }
    }

    public void onClick(View arg0) {
        for (int i = 0; i < mBtnList.length; i++) {
            if (arg0.getId() == mBtnList[i].getId()) {
                if (openPort(i + 1)) {
                    showToast(R.string.usbacm_open_succeed);
                    enableAllButton(false);
                } else {
                    showToast(R.string.usbacm_open_failed);
                }
            }
        }

        if (arg0.getId() == mBtnClose.getId()) {
            if (closePort()) {
                showToast(R.string.usbacm_close_succeed);
                enableAllButton(true);
            } else {
                showToast(R.string.usbacm_close_failed);
            }
        }
    }

    private boolean openPort(int index) {

        String tar = String.valueOf(index);
        SystemProperties.set(PROP_USB_ACM, tar);
        String ret = SystemProperties.get(PROP_USB_ACM);
        Elog.i(TAG, "open index:" + index + " getprop:" + ret);
        return tar.equals(ret);
    }

    private boolean closePort() {
        SystemProperties.set(PROP_USB_ACM, USB_ACM_DISABLE);
        String ret = SystemProperties.get(PROP_USB_ACM);
        Elog.i(TAG, "closeport getprop: " + ret);
        return USB_ACM_DISABLE.equals(ret);
    }

    private void enableAllButton(boolean enable) {
        for (Button btn : mBtnList) {
            btn.setEnabled(enable);
        }
    }

    private void showToast(int msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}