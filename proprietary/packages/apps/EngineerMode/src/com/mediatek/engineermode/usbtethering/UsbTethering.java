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

package com.mediatek.engineermode.usbtethering;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

public class UsbTethering extends Activity implements OnClickListener {

    private static final String TAG = "UsbTethering";
    private static final String USB_TETHERING_PROPERTY = "persist.vendor.net.auto.tethering";
    private Button mSetButton;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.usb_tethering);
        mSetButton = (Button) findViewById(R.id.usbtethering_switch);
        mSetButton.setOnClickListener(this);

        final String buttonFlag = SystemProperties.get(USB_TETHERING_PROPERTY, "false");

        Elog.v(TAG, "buttonFlag is:" + buttonFlag);

        if (buttonFlag.equals("true")) {
            mSetButton.setText(R.string.UsbTethering_disable);
        } else {
            mSetButton.setText(R.string.UsbTethering_enable);
        }
    }

    @Override
    public void onClick(final View arg0) {
        if (arg0 == mSetButton) {
            if ((getString(R.string.UsbTethering_enable)).equals(mSetButton.getText())) {
                boolean status = writeSystemProperties("true");
                if (status) {
                    mSetButton.setText(R.string.UsbTethering_disable);
                    Elog.v(TAG, "set succeed");
                    EmUtils.showToast("set tethering to true succeed");
                } else {
                    Elog.v(TAG, "set failed");
                    EmUtils.showToast("set tethering to true failed");
                }
            } else {

                boolean status = writeSystemProperties("false");
                if (status) {
                    mSetButton.setText(R.string.UsbTethering_enable);
                    Elog.v(TAG, "set succeed");
                    EmUtils.showToast("set tethering to false succeed");
                } else {
                    Elog.v(TAG, "set failed");
                    EmUtils.showToast("set tethering to false failed");
                }

            }
        }
    }

    private boolean writeSystemProperties(final String flag) {
        boolean statue = false;
        SystemProperties.set(USB_TETHERING_PROPERTY, flag);
        Elog.v(TAG, USB_TETHERING_PROPERTY + ",set to: " + flag);
        String value = SystemProperties.get(USB_TETHERING_PROPERTY);
        Elog.v(TAG, USB_TETHERING_PROPERTY + ",read from system is: " + value);
        if (value.equals(flag))
            statue = true;
        else
            statue = false;
        return statue;
    }
}
