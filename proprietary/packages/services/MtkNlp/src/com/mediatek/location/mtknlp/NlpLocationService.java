/*
* Copyright (C) 2015 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
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
package com.mediatek.location.mtknlp;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;


public class NlpLocationService extends Service {
    private static final String TAG = "NlpLocationService";
    private static final String GMS_PACKAGE_NAME = "com.google.android.gms";
    private NlpLocationProvider mNetworkProvider;
    private NlpGeocoder mGeocoder;
    protected MtkAgpsXmlParser mXml;
    private String mVendorPackageName = null;
    private NlpSwitcher mNlpSwitcher = null;
    private Context mContext;

    private static final int MSG_INIT_NLP = 1;
    private static final int MSG_INIT_GEOCODER = 2;

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind intent: " + intent);
        String action = intent.getAction();
        mContext = getApplicationContext();
        if (NlpLocationProvider.NETWORK_LOCATION_SERVICE_ACTION.equals(action)) {
            if (mNetworkProvider == null) {
                mNetworkProvider = new NlpLocationProvider(mContext);
                mHandler.sendEmptyMessage(MSG_INIT_NLP);
            }
            if (mNetworkProvider != null) {
                return mNetworkProvider.getBinder();
            }
        } else if (NlpGeocoder.GEOCODER_SERVICE_ACTION.equals(action)) {
            if (mGeocoder == null) {
                mGeocoder = new NlpGeocoder(mContext);
                mHandler.sendEmptyMessage(MSG_INIT_GEOCODER);
            }
            if (mGeocoder != null) {
                return mGeocoder.getBinder();
            }
        }
        return null;
    }


    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_INIT_NLP:
                    initNLP();
                    break;
                case MSG_INIT_GEOCODER:
                    initGeocoder();
                    break;
            }
        }
    };

    private void initSwitcher() {
        Log.d(TAG, "initSwitcher mNlpSwitcher=" + mNlpSwitcher);
        if (mNlpSwitcher == null) {
            loadVendorPackageName();

            if (mVendorPackageName == null) {
                Log.d(TAG, "mVendorPackageName = " + mVendorPackageName);
            }
            String[] vendorPkgs = new String[1];
            vendorPkgs[0] = mVendorPackageName;
            mNlpSwitcher = new NlpSwitcher(mContext, GMS_PACKAGE_NAME, vendorPkgs);
            if (mNlpSwitcher != null) {
                mNlpSwitcher.startMonitor();
            }
        }
    }

    private void initNLP() {
        Log.d(TAG, "initNLP mVendorPackageName = " + mVendorPackageName);
        if (mNlpSwitcher == null) {
            initSwitcher();
        }
        if (mNlpSwitcher != null) {
            mNetworkProvider.init(mVendorPackageName);
            mNlpSwitcher.setNlp(mNetworkProvider);
        }
    }

    private void initGeocoder() {
        Log.d(TAG, "initGeocoder mVendorPackageName = " + mVendorPackageName);
        if (mNlpSwitcher == null) {
            initSwitcher();
        }
        if (mNlpSwitcher != null) {
            mGeocoder.init(mVendorPackageName);
            mNlpSwitcher.setGeocoder(mGeocoder);
        }
    }

    @Override
    public boolean onUnbind(Intent intent) {
      Log.d(TAG, "onUnbind intent: " + intent);
      return false;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        if (mNlpSwitcher != null) {
            mNlpSwitcher.stopMonitor();
            mNlpSwitcher = null;
        }
        if (mNetworkProvider != null) {
            mNetworkProvider.onStop();
            mNetworkProvider = null;
        }
        if (mGeocoder!= null) {
            mGeocoder.onStop();
            mGeocoder = null;
        }
    }

    private void loadVendorPackageName() {
        loadXML();
        if (mXml != null) {
            ArrayList<NlpProfile> nlpProfiles = mXml.getNlpProfiles();
            if (nlpProfiles != null) {
                for (int i = 0; i < nlpProfiles.size(); i++) {
                    NlpProfile p = nlpProfiles.get(i);
                    Log.d(TAG, "Parsed nlp vendor package: "
                            +p.getNlpName() + ": " +p.getPackageName());
                    mVendorPackageName = new String(p.getPackageName());
                    break;  // currently only add first one
                }
            }
        }
    }

    protected void loadXML() {
        try {
            try {
                mXml = new MtkAgpsXmlParser();
            } catch (FileNotFoundException e) {
                mXml = new MtkAgpsXmlParser("/vendor/etc/agps_profiles_conf2.xml");
            }
            if (mXml != null) {
                Log.d(TAG, mXml.toString());
            }
        } catch (IOException e) {
            Log.e(TAG, "cannot load the XML file");
            e.printStackTrace();
        }
    }
}
