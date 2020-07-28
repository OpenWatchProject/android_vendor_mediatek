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

/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.connectivity;

import android.app.Activity;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkPolicy;
import android.net.NetworkPolicyManager;
import android.net.NetworkQuotaInfo;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

/**
 * Class for network framework service information.
 *
 */
public class CdsFrameworkSrvActivity extends Activity {
    private static final String TAG = "CdsFrameworkSrvActivity";

    private static final String NULL_INFO = "N/A";

    private TextView mActiveNetworkInfo;
    private TextView mActiveNetworkLinkProperties;
    private TextView mNetworkInfo;

    private Context mContext;
    private ConnectivityManager mConnMgr;
    private NetworkPolicyManager mNetPolicyMgr;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cds_framework_info);
        mContext = getBaseContext();

        if (mContext == null) {
            Log.e(TAG, "Could not get Conext of this activity");
        }

        mConnMgr = (ConnectivityManager) mContext.getSystemService(
                                Context.CONNECTIVITY_SERVICE);

        if (mConnMgr == null) {
            Log.e(TAG, "Could not get Connectivity Manager");
            return;
        }

        mNetPolicyMgr = NetworkPolicyManager.from(mContext);

        if (mNetPolicyMgr == null) {
            Log.e(TAG, "Could not get Network policy manager");
            return;
        }

        mActiveNetworkInfo            = (TextView) findViewById(R.id.active_network_info);
        mActiveNetworkLinkProperties  = (TextView) findViewById(R.id.active_link_properties);
        mNetworkInfo                  = (TextView) findViewById(R.id.network_infos);

        Log.i(TAG, "CdsFrameworkSrvActivity is started");
    }

    @Override
    public void onResume() {
        super.onResume();
        updateFrameworkSrvInfo();
    }

    private void updateFrameworkSrvInfo() {
        int i = 0;
        NetworkInfo networkInfo;
        NetworkPolicy[] networkPolicy;
        LinkProperties linkProperties;
        Network network;
        NetworkCapabilities networkcapabilities;
        String infoString = "";

        try {
            networkInfo = mConnMgr.getActiveNetworkInfo();
            if (networkInfo != null) {
                mActiveNetworkInfo.setText(networkInfo.toString().replace(',', '\n'));
            } else {
                mActiveNetworkInfo.setText(NULL_INFO);
            }
            linkProperties = mConnMgr.getActiveLinkProperties();
            if (linkProperties != null) {
                String link = linkProperties.toString();
                Log.i(TAG, link);
                mActiveNetworkLinkProperties.setText(link.replace("]", "]\n"));
            } else {
                mActiveNetworkLinkProperties.setText(NULL_INFO);
            }

            Network[] networks = mConnMgr.getAllNetworks();
            NetworkInfo[] networkinfos = mConnMgr.getAllNetworkInfo();
            infoString = "Total network number is " + networks.length
                    + "/" + networkinfos.length + "\r\n";

            if (networks != null) {
                for (i = 0; i < networks.length; i++) {
                    network = networks[i];
                    networkInfo = mConnMgr.getNetworkInfo(network);
                    if (networkInfo == null) {
                        continue;
                    }
                    Log.i(TAG, "[" + i + "]" + networkInfo.getTypeName());

                    if (!networkInfo.getTypeName().equals("Unknown")) {
                        infoString = infoString + "[" + i + "]"
                                + networkInfo.toString().replace(',', '\n')
                                + "\r\n----------------------\r\n";
                        infoString = infoString.replace(',', '\n');

                        if (networkInfo.isConnected()) {
                            linkProperties = mConnMgr.getLinkProperties(network);

                            if (linkProperties != null) {
                                infoString = infoString + "[" + i + "]"
                                + linkProperties.toString() + "\r\n----------------------\r\n";
                            }
                            networkcapabilities = mConnMgr.getNetworkCapabilities(network);
                            if (networkcapabilities != null) {
                                infoString = infoString + "[" + i + "]"
                                + networkcapabilities.toString() + "\r\n----------------------\r\n";
                            }
                        }
                    }
                }
                mNetworkInfo.setText(infoString);
            }
            Log.i(TAG, "updateFrameworkSrvInfo Done");
        } catch (NullPointerException e) {
            e.printStackTrace();
        }
    }
}
