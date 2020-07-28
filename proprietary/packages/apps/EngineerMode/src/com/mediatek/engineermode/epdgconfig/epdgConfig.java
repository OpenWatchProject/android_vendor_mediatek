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

package com.mediatek.engineermode.epdgconfig;

import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.PagerTitleStrip;
import android.support.v4.view.ViewPager;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

public class epdgConfig extends FragmentActivity {

    private static final int MSG_SET = 1;
    private static final int MSG_QUERY = 2;
    private static final int MAX_TIME = 400;
    private static final int PASS = 1;
    private static final int FAIL = 0;
    private static final int UNKNOWN = -1;
    public static int mPhoneId = 0;
    private static String TAG = "epdgConfig";
    private static String mAtCmd = "";
    private static Object mObject = new Object();
    private static int mResult = -1;
    private static String mResultMsg = "";
    private static Handler handler;
    private ViewPager mViewPager;
    private PagerTitleStrip pagerTitleStrip;
    private List<Fragment> mFragments;
    private List<String> titleList;

    public static boolean setCfgValue(String cfg_value, String set_value) {
        boolean ret = false;
        if (FeatureSupport.is93Modem()) {
            ret = setCfgValue93andlater(cfg_value, set_value);
        } else {
            ret = setCfgValue93before(cfg_value, set_value);
        }
        return ret;
    }

    public static boolean setCfgValue93andlater(String cfg_value, String set_value) {
        boolean ret = false;

        if (set_value == null) {
            mAtCmd = "AT+EWOCFGSET=" + "\"" + cfg_value + "\"";
        } else {
            mAtCmd = "AT+EWOCFGSET=" + "\"" + cfg_value + "\"" + "," + "\"" + set_value + "\"";
        }

        Elog.d(TAG, "set at Cmd = " + mAtCmd);
        synchronized (mObject) {
            mResult = UNKNOWN;
        }
        String[] cmd = new String[2];
        cmd[0] = mAtCmd;
        cmd[1] = "";

        EmUtils.invokeOemRilRequestStringsEm(mPhoneId, cmd, handler.obtainMessage(MSG_SET,
                cfg_value));
        ret = waitForResult(cfg_value);
        return ret;
    }

    public static boolean setCfgValue93before(String cfg_value, String set_value) {
        //phase out
        return false;
    }

    public static String getCfgValue(String cfg_value) {
        String ret = "";
        if (FeatureSupport.is93Modem()) {
            ret = getCfgValue93later(cfg_value);
        } else {
            ret = getCfgValue93before(cfg_value);
        }
        return ret;

    }

    public static String getCfgValue93later(String cfg_value) {
        String respValue = null;
        mResultMsg = null;

        synchronized (mObject) {
            mResult = UNKNOWN;
        }
        mAtCmd = "AT+EWOCFGGET=" + "\"" + cfg_value + "\"";
        Elog.d(TAG, "At Cmd = " + mAtCmd);

        String[] cmd = new String[2];
        cmd[0] = mAtCmd;
        cmd[1] = "+EWOCFGGET:";

        EmUtils.invokeOemRilRequestStringsEm(mPhoneId, cmd,
                handler.obtainMessage(MSG_QUERY, cfg_value));

        waitForResult(cfg_value);

        Elog.d(TAG, "mResultMsg1 = " + mResultMsg);
        respValue = mResultMsg;
        return respValue;
    }

    public static String getCfgValue93before(String cfg_value) {
        //phase out
        return "";
    }

    private static boolean waitForResult(String info) {
        Elog.d(TAG, "waitForResult");
        boolean ret = false;
        long startTime = System.currentTimeMillis();
        synchronized (mObject) {
            try {
                mObject.wait(MAX_TIME);
            } catch (Exception e) {
                Elog.d(TAG, "Exception");
            }

            if (System.currentTimeMillis() - startTime >= MAX_TIME) {
                Elog.d(TAG, info + ": time out");
                ret = false;
                mResultMsg = null;
            } else if (mResult == PASS) {
                Elog.d(TAG, info + ": succeed");
                ret = true;
            } else if (mResult == FAIL) {
                Elog.d(TAG, info + ": fail");
                ret = false;
                mResultMsg = null;
            }

        }
        return ret;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.epdg_config);

        Elog.d(TAG, "epdgConfig onCreate ");

        if (FeatureSupport.is93Modem()) {
            mPhoneId = getIntent().getIntExtra("phoneType", PhoneConstants.SIM_ID_1);
            Elog.d(TAG, "it is 93mdoem,mPhoneId = " + mPhoneId);
            WorkerThread wt = new WorkerThread();
            wt.start();
        }
        mViewPager = (ViewPager) findViewById(R.id.viewpager);
        pagerTitleStrip = (PagerTitleStrip) findViewById(R.id.pagertitle);
        pagerTitleStrip.setTextSpacing(100);
        mFragments = new ArrayList<Fragment>();
        mFragments.add(new epdgConfigGeneralFragment());
        mFragments.add(new epdgConfigTimerFragment());
        mFragments.add(new epdgConfigCertificateFragment());
        mFragments.add(new epdgConfigEmergencyFragment());
        mFragments.add(new epdgConfigOtherFragment());
        mFragments.add(new epdgConfigResetFragment());
        titleList = new ArrayList<String>();
        titleList.add("General");
        titleList.add("Timer");
        titleList.add("Certificate");
        titleList.add("Emergency");
        titleList.add("Other");
        titleList.add("reset");
        FragAdapter adapter = new FragAdapter(getSupportFragmentManager(), mFragments,
                titleList);
        mViewPager.setAdapter(adapter);

    }

    @Override
    public void onDestroy() {
        Elog.d(TAG, "epdgConfig onDestroy");
        super.onDestroy();
    }

    public class FragAdapter extends FragmentPagerAdapter {

        private List<Fragment> mFragments;
        private List<String> mTitle;

        public FragAdapter(FragmentManager fm, List<Fragment> fragments, List<String> title) {
            super(fm);
            // TODO Auto-generated constructor stub
            mFragments = fragments;
            mTitle = title;
        }

        @Override
        public Fragment getItem(int arg0) {
            // TODO Auto-generated method stub
            return mFragments.get(arg0);
        }

        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return mFragments.size();
        }

        @Override
        public CharSequence getPageTitle(int position) {
            // TODO Auto-generated method stub
            return mTitle.get(position);
        }
    }

    class WorkerThread extends Thread {
        @Override
        public void run() {
            Looper.prepare();
            handler = new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    switch (msg.what) {
                        case MSG_SET:
                            synchronized (mObject) {
                                AsyncResult asyncResult = (AsyncResult) msg.obj;
                                String expected = (String) asyncResult.userObj;
                                if (asyncResult != null && asyncResult.exception == null) {
                                    mResult = PASS;
                                    // Elog.d(TAG, expected + " set succeed");
                                } else {
                                    mResult = FAIL;
                                    // Elog.d(TAG, expected + " set failed");
                                }
                                mObject.notifyAll();
                            }
                            break;
                        case MSG_QUERY:
                            synchronized (mObject) {
                                AsyncResult asyncResult = (AsyncResult) msg.obj;
                                String expected = (String) asyncResult.userObj;
                                if (asyncResult != null && asyncResult.exception == null) {
                                    final String[] result = (String[]) asyncResult.result;
                                    if ((result.length > 0) && (result[0] != null)) {
                                        Elog.i(TAG, "Get info " + result[0]);

                                        String mResult1 = result[0].substring(
                                                result[0].indexOf(",") + 1);
                                        Elog.i(TAG, "mResult1 " + mResult1);

                                        String mResult = mResult1
                                                .substring(mResult1.indexOf("\"") + 1,
                                                        mResult1.lastIndexOf("\""));
                                        Elog.i(TAG, "mResult " + mResult);
                                        if (mResult == null) {
                                            mResultMsg = "";
                                        } else {
                                            mResultMsg = mResult;
                                        }
                                    }
                                    mResult = PASS;
                                } else {
                                    mResult = FAIL;
                                }
                                mObject.notifyAll();
                            }
                            break;
                        default:
                            break;
                    }
                }
            };
            Looper.loop();
        }
    }
}
