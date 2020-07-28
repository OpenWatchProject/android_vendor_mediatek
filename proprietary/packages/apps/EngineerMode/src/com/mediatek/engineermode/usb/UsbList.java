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

package com.mediatek.engineermode.usb;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.io.File;
import java.util.ArrayList;

/**
 * Test USB OTG and ex function
 *
 * @author mtk54040
 *
 */
public class UsbList extends Activity implements OnItemClickListener {
    public static final String IF_TEST = "if_test";
    public static final String IF_OTG20_TEST = "if_otg20_test";
    private static final String FILE_USB_OTG = "/sys/devices/platform/otg_iddig/otg_mode";
    private static final  int INDEX_0 = 0;
    private static final  int INDEX_1 = 1;
    private static final  int INDEX_2 = 2;
    private static final String TAG = "Usb/List";
    private ArrayList<String> mItemList = new ArrayList<String>();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.usb);

        ListView listView = (ListView) findViewById(R.id.ListView_USB);

        mItemList.add(getString(R.string.USB_IF_TEST));
        mItemList.add(getString(R.string.USB_EX_TEST));
        mItemList.add(getString(R.string.USB_IF_OTG20_TEST));

        if (UsbPhyTuning.isUsbPhyExist()) {
            mItemList.add(getString(R.string.usb_phy_tuning));
        }

        if ( new File(FILE_USB_OTG).exists()) {
            mItemList.add(getString(R.string.usb_otg_switch));
        }

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mItemList);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(this);
    }

    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        Elog.d(TAG, "-->onItemClick + arg2 " + arg2);
        Intent intent = null;
        if (getString(R.string.usb_phy_tuning).equals(mItemList.get(arg2))) {
            intent = new Intent(this, UsbPhyTuning.class);
        } else if (getString(R.string.usb_otg_switch).equals(mItemList.get(arg2))) {
            intent = new Intent(this, UsbOtgSwitch.class);
        }
        else {
            intent = new Intent(UsbList.this, UsbTest.class);
            switch (arg2) {
            case INDEX_0:
                intent.putExtra(UsbList.IF_TEST, true);
                intent.putExtra(UsbList.IF_OTG20_TEST, false);
                break;
            case INDEX_1:
                intent.putExtra(UsbList.IF_TEST, false);
                intent.putExtra(UsbList.IF_OTG20_TEST, false);
                break;
            case INDEX_2:
                intent.putExtra(UsbList.IF_TEST, false);
                intent.putExtra(UsbList.IF_OTG20_TEST, true);
                break;
            default:
                break;
            }
        }
        if (intent == null) {
            Elog.d(TAG, "Invalid intent: null");
            return;
        }
        startActivity(intent);
    }

}
