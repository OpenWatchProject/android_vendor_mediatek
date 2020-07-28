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

import com.mediatek.omacp.provider.OmacpProviderDatabase;

import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.database.Cursor;
import android.test.AndroidTestCase;

public class MessageListProviderTest extends AndroidTestCase {

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    @InternalApiAnnotation
    public void testcase01_addBrowserMessage() {
        Utils utils = new Utils(this.mContext);
        utils.DeleteAllMessages();
        utils.insertOneMessage(SendOmacpMessage.BROWSER_DATA, "w2,");

        // Modify the shared preference value to indicate the mms that
        // configuration message exist
        SharedPreferences sh = this.mContext.getSharedPreferences("omacp",
                this.mContext.MODE_PRIVATE);
        Editor editor = sh.edit();
        editor.putBoolean("configuration_msg_exist", true);
        editor.commit();

        String[] project = {
            "_id"
        };
        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI,
                project, null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 1);
    }

    @InternalApiAnnotation
    public void testcase02_addBrowserData() {
        Utils utils = new Utils(this.mContext);
        utils.insertOneMessage(SendOmacpMessage.BROWSER_DATA, "w2,");

        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI, null,
                null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 2);
    }

    @InternalApiAnnotation
    public void testcase03_addMmsData() {
        Utils utils = new Utils(this.mContext);
        utils.insertOneMessage(SendOmacpMessage.MMS_DATA, "w4,");

        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI, null,
                null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 3);
    }

    @InternalApiAnnotation
    public void testcase04_addRtspData() {
        Utils utils = new Utils(this.mContext);
        utils.insertOneMessage(SendOmacpMessage.RTSP_DATA, "554,");

        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI, null,
                null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 4);
    }

    @InternalApiAnnotation
    public void testcase05_addSuplData() {
        Utils utils = new Utils(this.mContext);
        utils.insertOneMessage(SendOmacpMessage.SUPL_DATA, "ap0004,");

        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI, null,
                null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 5);
    }

    @InternalApiAnnotation
    public void testcase06_addDmData() {
        Utils utils = new Utils(this.mContext);
        utils.insertOneMessage(SendOmacpMessage.DM_DATA, "w7,");

        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI, null,
                null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 6);
    }

    @InternalApiAnnotation
    public void testcase07_addMMSAPNData() {
        Utils utils = new Utils(this.mContext);
        utils.insertOneMessage(SendOmacpMessage.MMSAPN_DATA, "w4,");

        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI, null,
                null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 7);
    }

    @InternalApiAnnotation
    public void testcase08_addEmailSinaData() {
        Utils utils = new Utils(this.mContext);
        utils.insertOneMessage(SendOmacpMessage.EMAIL_SINA_DATA, "25,");

        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI, null,
                null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 8);
    }

    @InternalApiAnnotation
    public void testcase09_addApnData() {
        Utils utils = new Utils(this.mContext);
        utils.insertOneMessage(SendOmacpMessage.APN_DATA, "apn,");

        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI, null,
                null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 9);
    }

    @InternalApiAnnotation
    public void testcase09_addIMPSData() {
        Utils utils = new Utils(this.mContext);
        utils.insertOneMessage(SendOmacpMessage.IMPS_DATA, "wA,");

        Cursor c = getContext().getContentResolver().query(OmacpProviderDatabase.CONTENT_URI, null,
                null, null, null);
        assertNotNull(c);
        int messageCount = c.getCount();
        c.close();
        assertTrue(messageCount == 10);
    }

}
