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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.op08.telephony;

import android.content.Context;
import android.os.SystemProperties;
import android.os.Handler;
import android.os.PowerManager;
import android.os.Message;
import android.os.AsyncResult;
import android.content.DialogInterface;
import android.app.AlertDialog;
import android.view.WindowManager;

import android.util.Log;
import com.android.internal.telephony.CommandsInterface;
import com.mediatek.internal.telephony.uicc.IMtkSimHandler;
import com.mediatek.internal.telephony.uicc.MtkSimHandler;
import com.mediatek.opcommon.telephony.MtkRilOp;
import com.mediatek.internal.telephony.MtkRIL;

public class Op08MtkSimHandler extends MtkSimHandler {

    private static final String TAG_OP08 = "Op08MtkSimHandler";
    private static boolean DBG = SystemProperties.get("ro.build.type").equals("eng");

    private static final int MTK_SIM_OP08_EVENT_BASE = 1000;
    private static final int EVENT_MELOCK_CHANGED = 1 + MTK_SIM_OP08_EVENT_BASE;
    private MtkRilOp mMtkRilOp = null;
    protected Context mContext = null;

    public Op08MtkSimHandler() {
        super();
        mtkLog(TAG_OP08, "Enter Op08MtkSimHandler");
    }
    public Op08MtkSimHandler(Context context, CommandsInterface ci) {
        super(context, ci);
        mtkLog(TAG_OP08, "Enter Op08MtkSimHandler context");
        mContext = context;
        registerForRemoteSimUnlockChanged(ci);
    }

    private void registerForRemoteSimUnlockChanged(CommandsInterface ci) {
        mtkLog(TAG_OP08, "registerForRemoteSimUnlockChanged");
        mMtkRilOp = (MtkRilOp)((MtkRIL)ci).getRilOp();
        mMtkRilOp.registerForMelockChanged(mHandler, EVENT_MELOCK_CHANGED, null);
    }

    private void unregisterForRemoteSimUnlockChanged() {
        if (mMtkRilOp == null) {
            mtkLoge(TAG_OP08, "unregisterForRemoteSimUnlockChanged null mCi");
            return;
        }
        mMtkRilOp.unregisterForMelockChanged(mHandler);
    }

    public void dispose() {
        mtkLog(TAG_OP08, "dispose Op08MtkSimHandler");
        unregisterForRemoteSimUnlockChanged();
    }
    private class RebootClickListener
            implements DialogInterface.OnClickListener {

        @Override
        public void onClick(DialogInterface dialog, int which) {
            mtkLog(TAG_OP08, "Unlock Phone onClick");
            PowerManager pm = (PowerManager) mContext
                    .getSystemService(Context.POWER_SERVICE);
            pm.reboot("Unlock state changed");
        }
    }

    protected Handler mHandler = new Handler() {
        AsyncResult ar = null;
        @Override
        public void handleMessage(Message msg){
            switch (msg.what) {
                case EVENT_MELOCK_CHANGED:
                    if (DBG) mtkLog(TAG_OP08, "handleMessage (EVENT_MELOCK_CHANGED)");
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception == null && ar.result != null) {
                        int[] simMelockEvent = (int []) ar.result;

                        if (DBG) mtkLog(TAG_OP08, "sim melock event = " + simMelockEvent[0]);

                        RebootClickListener listener = new RebootClickListener();

                        if (simMelockEvent[0] == 0) {
                            AlertDialog alertDialog = new AlertDialog.Builder(mContext)
                                    .setTitle("Unlock Phone")
                                    .setMessage(
                                    "Please restart the phone now since unlock setting has changed."
                                    )
                                    .setPositiveButton("OK", listener)
                                    .create();

                            alertDialog.setCancelable(false);
                            alertDialog.setCanceledOnTouchOutside(false);

                            alertDialog.getWindow().setType(
                                    WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
                            alertDialog.show();
                        }
                    }
                    break;
                default:
                    mtkLoge(TAG_OP08, "Unknown Event " + msg.what);
            }
        }
    };
}
