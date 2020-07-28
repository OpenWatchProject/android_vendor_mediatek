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

package com.mediatek.engineermode.debugtool;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;

/**
 * @author @author Aee owner
 *
 */
public class AEEControlService extends Service {
    private final static String TAG = "AEEControlService/Debugutils";

    protected Map<String, String> mCommandMap;

    protected static final String COMMAND_CLEAR_DAL = "aee -c dal";

    protected static final String COMMAND_CLEAN_DATA = "rm -r /data/core/ /data/anr";

    // This is the object that receives interactions from clients.
    private final IBinder mBinder = new LocalBinder();

    /**
     * Class for clients to access. Because we know this service always runs in
     * the same process as its clients, we don't need to deal with IPC.
     */
    public class LocalBinder extends Binder {
        AEEControlService getService() {
            return AEEControlService.this;
        }
    }

    @Override
    public void onCreate() {
        mCommandMap =
                buildCommandList("AEECleanData#rm -r /data/core /data/anr /data/tombstones",
                        "AEEClearDAL#aee -c dal", "MediatekEngineer#aee -m 1",
                        "MediatekUser#aee -m 2", "CustomerEngineer#aee -m 3",
                        "CustomerUser#aee -m 4", "EnableDAL#aee -s on", "DisableDAL#aee -s off");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Elog.i("AEEControlService", "Received start id " + startId + ": " + intent);
        // We want this service to continue running until it is explicitly
        // stopped, so return sticky.
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        // Tell the user we stopped.
        // Toast.makeText(this, R.string.aee_service_stopped,
        // Toast.LENGTH_SHORT).show();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    /**
     * @param modevalue String
     */
    public void changeAEEMode(String modevalue) {
        String command = mCommandMap.get(modevalue);
        if (command != null) {
            systemexec(command);
        }
    }

    /**
     * @param dalOption String
     */
    public void dalSetting(String dalOption) {
        String command = mCommandMap.get(dalOption);
        if (command != null) {
            systemexec(command);
        }
    }

    /**
     * void return.
     */
    public void clearDAL() {
        String command = mCommandMap.get("AEEClearDAL");
        if (command != null) {
            systemexec(command);
        }
        Elog.d(TAG, "Device AEE red screen cleared.");
        // Toast.makeText(this, "DAL cleared", Toast.LENGTH_SHORT).show();
    }

    /**
     * void return.
     */
    public void cleanData() {
        String command = mCommandMap.get("AEECleanData");
        if (command != null) {
            systemexec(command);
        }
        Elog.i(TAG, "Device /data partition cleaned up.");
        Toast.makeText(this, "Device /data partition cleaned up.", Toast.LENGTH_SHORT).show();
    }

    private static StringBuffer systemexec(String command) {
        StringBuffer output = new StringBuffer();
        try {
            Process process = Runtime.getRuntime().exec(command);
            BufferedReader reader =
                    new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line = new String();
            while ((line = reader.readLine()) != null) {
                output.append(line + "\n");
            }
            Elog.d(TAG, output.toString());
            process.waitFor();
            reader.close();
        } catch (IOException ioe) {
            ioe.printStackTrace();
            Elog.e(TAG, "Operation failed.");
        } catch (InterruptedException ie) {
            ie.printStackTrace();
            Elog.e(TAG, "Operation failed.");
        }
        return output;
    }

    private static Map<String, String> buildCommandList(String... cmdentries) {
        Map<String, String> commandMap = new HashMap<String, String>();
        for (String cmdentry : cmdentries) {
            String[] kv = cmdentry.split("#");
            commandMap.put(kv[0], kv[1]);
        }
        return commandMap;
    }
}
