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

package com.mediatek.omacp.tests;

import java.lang.reflect.Field;

import com.mediatek.omacp.message.OmacpMessageList;
import com.mediatek.omacp.provider.OmacpProviderDatabase;
import com.mediatek.omacp.message.OmacpApplicationCapability;

import android.app.Activity;
import android.app.Dialog;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.content.SharedPreferences;
import android.content.IntentFilter.MalformedMimeTypeException;
import android.content.SharedPreferences.Editor;
import android.net.Uri;
import android.os.SystemClock;
import android.test.ActivityInstrumentationTestCase2;
import android.test.TouchUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.ListView;
import android.database.Cursor;
import android.content.ContentUris;
import android.app.NotificationManager;
import android.content.Context;

public class MessageListTest extends ActivityInstrumentationTestCase2<OmacpMessageList> {
    private final static String TAG = "OmacpFunctionalTest";

    private OmacpMessageList mActivity = null;

    private Instrumentation mInst;

    public MessageListTest() {
        super(OmacpMessageList.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Log.i(TAG, "MessageListTest");

        mActivity = getActivity();
        mInst = getInstrumentation();
        mInst.waitForIdleSync();

        Utils utils = new Utils(mActivity);
        utils.registerResultReceiver();
    }

    @Override
    protected void tearDown() throws Exception {
        Utils utils = new Utils(mActivity);
        utils.registerResultReceiver();

        if (mActivity != null) {
            mActivity.finish();
        }

        super.tearDown();
    }

    @InternalApiAnnotation
    public void testcase01_checkMessageExist() throws MalformedMimeTypeException,
            InterruptedException {
        SharedPreferences sh = mActivity.getSharedPreferences("omacp",
                mActivity.MODE_PRIVATE);
        boolean exist = sh.getBoolean("configuration_msg_exist", false);
        assertTrue(exist);
    }

    @InternalApiAnnotation
    public void testcase02_CheckMessageCount() throws InterruptedException {
        ListView listView = mActivity.getListView();
        assertTrue(listView.getCount() == 10);
    }

    @InternalApiAnnotation
    public void testcase03_ClickItem() {
        ActivityMonitor amSettingsDetail = mInst.addMonitor(
                "com.mediatek.omacp.message.OmacpMessageSettingsDetail", null, false);

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();

        ListView listView = mActivity.getListView();
        TouchUtils.clickView(this, listView.getSelectedView());

        Activity settingsDetailActivity = (Activity) amSettingsDetail.waitForActivityWithTimeout(5000);
        assertNotNull(settingsDetailActivity);
        settingsDetailActivity.finish();
        mInst.removeMonitor(amSettingsDetail);
        mInst.waitForIdleSync();
    }

    @InternalApiAnnotation
    public void testcase04_ContextViewMenu() {
        ActivityMonitor amSettingsDetail = mInst.addMonitor(
                "com.mediatek.omacp.message.OmacpMessageSettingsDetail", null, false);

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();

        ListView listView = mActivity.getListView();
        TouchUtils.longClickView(MessageListTest.this, listView.getSelectedView());

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        Activity settingsDetailActivity = (Activity) amSettingsDetail.waitForActivityWithTimeout(5000);
        assertNotNull(settingsDetailActivity);
        settingsDetailActivity.finish();
        mInst.removeMonitor(amSettingsDetail);
        mInst.waitForIdleSync();
    }

    @InternalApiAnnotation
    public void testcase04_BrowserCustomInstall() throws MalformedMimeTypeException, InterruptedException {
        clickCustomInstall();
        /** M: modify waitting time from 1s to 3s */
        SystemClock.sleep(3000);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        mInst.waitForIdleSync();
    }

    @InternalApiAnnotation
    public void testcase05_ContextDelete() {
        ListView listView = mActivity.getListView();
        int preCount = listView.getCount();

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();
        TouchUtils.longClickView(MessageListTest.this, listView.getSelectedView());

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        /** M: modify keyevent */
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_RIGHT);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();
        /** M: modify waitting time from 2s to 4s */
        SystemClock.sleep(4000);

        int nowCount = listView.getCount();
        assertTrue(nowCount == preCount - 1);
    }

    private Dialog getDetailDialog() {
        try {
            Class clazz;
            Field field;
            clazz = Class.forName("com.mediatek.omacp.message.OmacpMessageList");
            field = clazz.getDeclaredField("mDetailDialog");
            field.setAccessible(true);
            Dialog dialog = (Dialog) field.get(mActivity);
            assertNotNull(dialog);
            return dialog;
        } catch (Exception e) {
            e.printStackTrace();
            // ignore
        }
        return null;
    }

    @InternalApiAnnotation
    public void testcase06_ContextViewDetails() {
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();

        ListView listView = mActivity.getListView();
        TouchUtils.longClickView(MessageListTest.this, listView.getSelectedView());

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        Dialog detailDialog = getDetailDialog();
        assertTrue(detailDialog.isShowing());

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        mInst.waitForIdleSync();
    }

    // ***************************************OmacpMessageDetailsTest******************************//
    private void clickFullInstall(int focusDown) {
        ActivityMonitor amSettingsDetail = mInst.addMonitor(
                "com.mediatek.omacp.message.OmacpMessageSettingsDetail", null, false);

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();

        for (int i = 0; i < focusDown; i++) {
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
            mInst.waitForIdleSync();
        }

        ListView listView = mActivity.getListView();
        TouchUtils.clickView(this, listView.getSelectedView());

        Activity settingsDetailActivity = (Activity) amSettingsDetail.waitForActivityWithTimeout(5000);
        assertNotNull(settingsDetailActivity);
        mInst.removeMonitor(amSettingsDetail);
        mInst.waitForIdleSync();

        // focus and click full install
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();
    }

    private void clickCustomInstall() {
        ActivityMonitor amSettingsDetail = mInst.addMonitor(
                "com.mediatek.omacp.message.OmacpMessageSettingsDetail", null, false);

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();

        ListView listView = mActivity.getListView();
        TouchUtils.clickView(this, listView.getSelectedView());
        Activity settingsDetailActivity = (Activity) amSettingsDetail
                .waitForActivityWithTimeout(5000);
        assertNotNull(settingsDetailActivity);
        mInst.removeMonitor(amSettingsDetail);
        mInst.waitForIdleSync();

        // focus and click full install
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_RIGHT);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();
        //select application
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();
        //install application
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();
    }

    public void testcase07_BrowserInstall() throws MalformedMimeTypeException, InterruptedException {
        /*
        if (!OmacpApplicationCapability.sBrowser) {
            clickFullInstall(0);
            SystemClock.sleep(3000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();
            SystemClock.sleep(3000);
            Uri uri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI, new Long(2));
            String[] projection = {"installed"};
            Cursor c = mActivity.getContentResolver().query(uri, projection, null, null, null);
            assertNotNull(c.moveToFirst());
            int installed = c.getInt(0);
            c.close();
            assertTrue(installed == 1);
        } else {
            Utils utils = new Utils(mActivity);
            // utils.registerResultReceiver();
            clickFullInstall(0);
            utils.InstallOneSetting("w2", mInst);
            // utils.unregisterResultReceiver();

            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            assertTrue(utils.mResultMap.size() == 2);
        }
        */
    }

    @InternalApiAnnotation
    public void testcase08_MmsInstall() throws MalformedMimeTypeException, InterruptedException {
        if (!OmacpApplicationCapability.sMms) {
            clickFullInstall(1);
            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();
            /** M: modify waitting time from 1s to 3s */
            SystemClock.sleep(3000);
            Uri uri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI, new Long(3));
            String[] projection = {
                "installed"
            };
            Cursor c = mActivity.getContentResolver().query(uri, projection, null, null, null);
            assertNotNull(c.moveToFirst());
            int installed = c.getInt(0);
            c.close();
            assertTrue(installed == 1);
        } else {
            Utils utils = new Utils(mActivity);
            // utils.registerResultReceiver();
            clickFullInstall(1);
            utils.InstallOneSetting("w4", mInst);
            // utils.unregisterResultReceiver();

            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            assertTrue(utils.mResultMap.size() == 2);
        }
    }

    public void testcase09_RtspInstall() throws MalformedMimeTypeException, InterruptedException {
        /*
        if (!OmacpApplicationCapability.sRtsp) {
            clickFullInstall(2);
            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            SystemClock.sleep(1000);
            Uri uri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI, new Long(4));
            String[] projection = {
                "installed"
            };
            Cursor c = mActivity.getContentResolver().query(uri, projection, null, null, null);
            assertNotNull(c.moveToFirst());
            int installed = c.getInt(0);
            c.close();
            assertTrue(installed == 1);
        } else {
            Utils utils = new Utils(mActivity);
            // utils.registerResultReceiver();
            clickFullInstall(2);
            utils.InstallOneSetting("554", mInst);
            // utils.unregisterResultReceiver();

            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            assertTrue(utils.mResultMap.size() == 2);
            //assertTrue(((String) utils.mResultMap.get("appId")).equalsIgnoreCase("554"));
            //assertTrue(((Boolean) utils.mResultMap.get("result")) == true);
        }
        */
    }

    @InternalApiAnnotation
    public void testcase10_SuplInstall() throws MalformedMimeTypeException, InterruptedException {
        if (!OmacpApplicationCapability.sSupl) {
            clickFullInstall(3);
            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            SystemClock.sleep(1000);
            Uri uri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI, new Long(5));
            String[] projection = {
                "installed"
            };
            Cursor c = mActivity.getContentResolver().query(uri, projection, null, null, null);
            assertNotNull(c.moveToFirst());
            int installed = c.getInt(0);
            c.close();
            assertTrue(installed == 1);
        } else {
            Utils utils = new Utils(mActivity);
            // utils.registerResultReceiver();
            clickFullInstall(3);
            utils.InstallOneSetting("ap0004", mInst);
            // utils.unregisterResultReceiver();

            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            assertTrue(utils.mResultMap.size() == 2);
        }
    }

    @InternalApiAnnotation
    public void testcase11_DmInstall() throws MalformedMimeTypeException, InterruptedException {
        if (!OmacpApplicationCapability.sDm) {
            clickFullInstall(4);
            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            SystemClock.sleep(1000);
            Uri uri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI, new Long(6));
            String[] projection = {
                "installed"
            };
            Cursor c = mActivity.getContentResolver().query(uri, projection, null, null, null);
            assertNotNull(c.moveToFirst());
            int installed = c.getInt(0);
            c.close();
            //assertTrue(installed == 1);
        } else {
            Utils utils = new Utils(mActivity);
            // utils.registerResultReceiver();
            clickFullInstall(4);
            utils.InstallOneSetting("w7", mInst);
            // utils.unregisterResultReceiver();

            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            assertTrue(utils.mResultMap.size() == 2);
        }
    }

    public void testcase12_MMSAPNInstall() throws MalformedMimeTypeException,
            InterruptedException {
        /*
        Utils utils = new Utils(mActivity);
        clickFullInstall(5);
        utils.InstallOneSetting("apn", mInst);

        SystemClock.sleep(40000);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        mInst.waitForIdleSync();
        */
    }

    @InternalApiAnnotation
    public void testcase13_EmailSinaInstall() throws MalformedMimeTypeException,
            InterruptedException {
        if (!OmacpApplicationCapability.sEmail) {
            clickFullInstall(6);
            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            SystemClock.sleep(1000);
            Uri uri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI, new Long(8));
            String[] projection = {
                "installed"
            };
            Cursor c = mActivity.getContentResolver().query(uri, projection, null, null, null);
            assertNotNull(c.moveToFirst());
            int installed = c.getInt(0);
            c.close();
            //assertTrue(installed == 1);
        } else {
            Utils utils = new Utils(mActivity);
            // utils.registerResultReceiver();
            clickFullInstall(6);
            utils.InstallOneSetting("25", mInst);
            // utils.unregisterResultReceiver();

            SystemClock.sleep(1000);
            mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            mInst.waitForIdleSync();

            assertTrue(utils.mResultMap.size() == 2);
        }
    }

    @InternalApiAnnotation
    public void testcase14_ApnInstall() throws MalformedMimeTypeException, InterruptedException {
        Utils utils = new Utils(mActivity);
        // utils.registerResultReceiver();
        clickFullInstall(7);
        utils.InstallOneSetting("apn", mInst);
        // utils.unregisterResultReceiver();

        SystemClock.sleep(1000);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        mInst.waitForIdleSync();

        assertTrue(utils.mResultMap.size() == 2);
    }

    @InternalApiAnnotation
    public void testcase15_IMPSInstall() {
        clickFullInstall(8);
        SystemClock.sleep(1000);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        mInst.waitForIdleSync();

        SystemClock.sleep(1000);
        Uri uri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI, new Long(8));
        String[] projection = {
            "installed"
        };
        Cursor c = mActivity.getContentResolver().query(uri, projection, null, null, null);
        assertNotNull(c.moveToFirst());
        int installed = c.getInt(0);
        c.close();
        assertTrue(installed == 1);
    }

    @InternalApiAnnotation
    public void testcase16_OptionDeleteAll() {
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_MENU);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        /** M: modify keyevent */
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_RIGHT);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        SystemClock.sleep(2000);
        SharedPreferences sh = mActivity.getSharedPreferences("omacp",
                mActivity.MODE_PRIVATE);
        boolean exist = sh.getBoolean("configuration_msg_exist", true);
        assertFalse(exist);
    }

    @InternalApiAnnotation
    public void testcase17_checkMessageNetWorkPin() {
        try {
            Utils utils = new Utils(mActivity);
            utils.CreateWbXmlMessage(0, SendOmacpMessage.WEBXMLAPNDATA);
            mInst.waitForIdleSync();

            SystemClock.sleep(1000);

            SharedPreferences sh = mActivity.getSharedPreferences("omacp",
                    mActivity.MODE_PRIVATE);
            boolean exist = sh.getBoolean("configuration_msg_exist", false);
            //assertFalse(exist);

            Editor editor = sh.edit();
            editor.putBoolean("configuration_msg_exist", false);
            editor.commit();
        } catch (MalformedMimeTypeException malException) {
            malException.printStackTrace();
            fail();
        } catch (InterruptedException inteException) {
            inteException.printStackTrace();
            fail();
        }
    }

    @InternalApiAnnotation
    public void testcase18_receiveWbXmlMessage() {
        try {
            Utils utils = new Utils(mActivity);
            utils.CreateWbXmlMessage(1, SendOmacpMessage.WEBXMLAPNDATA);
            mInst.waitForIdleSync();

            SystemClock.sleep(1000);

            SharedPreferences sh = mActivity.getSharedPreferences("omacp",
                    mActivity.MODE_PRIVATE);
            boolean exist = sh.getBoolean("configuration_msg_exist", false);
            //assertTrue(exist);

            Editor editor = sh.edit();
            editor.putBoolean("configuration_msg_exist", false);
            editor.commit();
        } catch (MalformedMimeTypeException malException) {
            malException.printStackTrace();
            fail();
        } catch (InterruptedException inteException) {
            inteException.printStackTrace();
            fail();
        }
    }

    @InternalApiAnnotation
    public void testcase19_ReceiveMessage() {
        try {
            Utils utils = new Utils(mActivity);
            utils.CreateOneOmacpMessage(SendOmacpMessage.BROWSER_DATA);
            mInst.waitForIdleSync();

            SystemClock.sleep(1000);

            SharedPreferences sh = mActivity.getSharedPreferences("omacp",
                    mActivity.MODE_PRIVATE);
            boolean exist = sh.getBoolean("configuration_msg_exist", false);
            //assertTrue(exist);

            utils.DeleteAllMessages();

            NotificationManager nm = (NotificationManager) mActivity
                    .getSystemService(Context.NOTIFICATION_SERVICE);

            nm.cancel(126);

            Editor editor = sh.edit();
            editor.putBoolean("configuration_msg_exist", false);
            editor.commit();
        } catch (MalformedMimeTypeException malException) {
            malException.printStackTrace();
            fail();
        } catch (InterruptedException inteException) {
            inteException.printStackTrace();
            fail();
        }
    }
}
