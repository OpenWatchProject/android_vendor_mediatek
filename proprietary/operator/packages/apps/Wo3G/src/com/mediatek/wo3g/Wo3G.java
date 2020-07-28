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

package com.mediatek.wo3g;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.KeyEvent;

import com.mediatek.telephony.MtkTelephonyManagerEx;

/** main activity of Wo3G app. */
public class Wo3G extends Activity {
    private static final String TAG = "Wo3G";
    private static final String url = "http://www.wo.com.cn";
    private static final int REQUEST_SETTINGS_CODE = 1;
    private static final String[] MCCMNC_TABLE_TYPE_CU = {
        "46001", "46006", "46009", "45407", "46005"};
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d("@M_" + TAG, "Start Wo3G!");
        NetworkInfo info = ((ConnectivityManager) getSystemService(
            Context.CONNECTIVITY_SERVICE)).getActiveNetworkInfo();
        if (info != null && info.isConnected()) {
            Log.d(TAG, "typename=" + info.getTypeName());
            if (info.getTypeName().equalsIgnoreCase("mobile")) {
                int curConSubId = SubscriptionManager.getDefaultDataSubscriptionId();
                int slotId = SubscriptionManager.getSlotIndex(curConSubId);
                int anotherSubId = getSubIdBySlot(1 - slotId);
                if (isCuSimCard(curConSubId) && is4GSimCard(curConSubId)) {
                    launchBrowser();
                } else {
                    if (isCuSimCard(anotherSubId) && is4GSimCard(anotherSubId)) {
                        showSimSwitchDialog(curConSubId, anotherSubId);
                    } else {
                        launchBrowser();
                    }
                }
            } else {    //use wifi network.
                //showContinueOrQuitDialog(getString(R.string.requireCUMsg));
                /// M: Launch Browser when WiFi is connect for CU spec 4.0 @{
                launchBrowser();
                /// @}
            }
        } else {
            showDialog(getString(R.string.noNetworkMsg));
        }
    }

    private boolean isCuSimCard(int subId) {
        Log.d("@M_" + TAG, "isCuSimCard, subId = " + subId);
        String simOperator = null;
        simOperator = getSimOperator(subId);
        if (simOperator != null) {
            Log.d("@M_" + TAG, "isCuSimCard, simOperator =" + simOperator);
            for (String mccmnc : MCCMNC_TABLE_TYPE_CU) {
                if (simOperator.equals(mccmnc)) {
                    return true;
                }
            }
        }
        return false;
    }

    private String getSimOperator(int subId) {
        if (subId < 0) {
            return null;
        }
        String simOperator = null;
        int status = TelephonyManager.SIM_STATE_UNKNOWN;
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            status = TelephonyManager.from(this).getSimState(slotId);
        }
        if (status == TelephonyManager.SIM_STATE_READY) {
            simOperator = TelephonyManager.from(this).getSimOperator(subId);
        }
        Log.d("@M_" + TAG, "getSimOperator, simOperator = " + simOperator + " subId = " + subId);
        return simOperator;
    }

    private boolean is4GSimCard(int subId) {
        String type = null;
        if (subId > 0) {
            type = MtkTelephonyManagerEx.getDefault().getIccCardType(subId);
        }
        Log.d(TAG, "getSIMType type : " + type);
        return "USIM".equals(type);
    }

    private int getSubIdBySlot(int slotId) {
        if (slotId < 0 || slotId > 1) {
            return -1;
        }
        int[] subids = SubscriptionManager.getSubId(slotId);
        int subid = -1;
        if (subids != null && subids.length >= 1) {
            subid = subids[0];
        }
        Log.d("@M_" + TAG, "GetSimIdBySlot: sub id = " + subid
                + "sim Slot = " + slotId);
        return subid;
    }

    private void showContinueOrQuitDialog(String msg) {
        new AlertDialog.Builder(this)
            .setTitle(R.string.alertTitle)
            .setMessage(msg)
            .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    launchBrowser();
                }
            })
        .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        })
        .setOnKeyListener(new DialogInterface.OnKeyListener() {
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                switch (keyCode) {
                    case KeyEvent.KEYCODE_BACK:
                        finish();
                        break;
                    default:
                        break;
                }
                return true;
            }
        }).show();
    }

    private void launchBrowser() {
        Uri uri = Uri.parse(url);
        Intent i = new Intent(Intent.ACTION_VIEW, uri);
        i.setClassName("com.android.browser", "com.android.browser.BrowserActivity");
        if (getPackageManager().resolveActivity(i, 0) == null) {
            i = new Intent(Intent.ACTION_VIEW, uri);
        }
        try {
            startActivity(i);
        } catch (ActivityNotFoundException e) {
            // Some third party app can disable Browser, so we should catch it. And since
            // user disable Browser, we should do nothing.
            Log.d(TAG, "Ooops, no activity found to handle uri", e);
        }
        finish();
    }

    private void showDialog(String msg) {
        new AlertDialog.Builder(this)
            .setTitle(R.string.alertTitle)
            .setMessage(msg)
            .setPositiveButton(R.string.ok1, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
            })
            .setOnKeyListener(new DialogInterface.OnKeyListener() {
                public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                    switch (keyCode) {
                        case KeyEvent.KEYCODE_ENTER:
                            if (event.getAction() != KeyEvent.ACTION_DOWN) {
                                break;
                            }
                        case KeyEvent.KEYCODE_BACK:
                            finish();
                            break;
                        default:
                            break;
                    }
                    return true;
                }
            })
            .show();
    }

    private void showSimSwitchDialog(final int curSub, final int anotherSub) {
        SubscriptionInfo sir = SubscriptionManager.from(this)
            .getActiveSubscriptionInfo(anotherSub);
        if (sir == null) {
            Log.d(TAG, "invalid sub id!");
            return;
        }
        CharSequence currentSimName = sir.getDisplayName();
        if (currentSimName == null) {
            currentSimName = "SIM " + (sir.getSimSlotIndex() + 1);
        }
        String msg = getResources().getString(R.string.simswitch, currentSimName);

        new AlertDialog.Builder(this)
            .setTitle(R.string.alertTitle)
            .setMessage(msg)
            .setPositiveButton(R.string.ok1, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    SubscriptionManager subscriptionManager = SubscriptionManager.from(Wo3G.this);
                    TelephonyManager tm = TelephonyManager.from(Wo3G.this);
                    subscriptionManager.setDefaultDataSubId(anotherSub);
                    tm.setDataEnabled(curSub, false);
                    tm.setDataEnabled(anotherSub, true);
                    launchBrowser();
                }
            })
        .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        })
        .setOnKeyListener(new DialogInterface.OnKeyListener() {
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                switch (keyCode) {
                    case KeyEvent.KEYCODE_BACK:
                        finish();
                        break;
                    default:
                        break;
                }
                return true;
            }
        }).show();
    }

}
