/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.android.browser.sitenavigation;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.ParseException;
import android.net.Uri;
import android.net.WebAddress;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.android.browser.R;
import com.android.browser.UrlUtils;

import java.io.ByteArrayOutputStream;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

/**
 *  This class provides UI and methods that allow user to
 *  add new website or edit existing website.
 */
public class SiteNavigationAddDialog extends Activity {

    private static final String XLOGTAG = "browser/AddSiteNavigationPage";

    private EditText    mName;
    private EditText    mAddress;
    private Button      mButtonOK;
    private Button      mButtonCancel;
    private Bundle      mMap;
    // The original url that is editting
    private String      mItemUrl;
    private String      mItemName;
    private boolean     mIsAdding;
    private TextView    mDialogText;

    // Message IDs
    private static final int SAVE_SITE_NAVIGATION = 100;

    private Handler mHandler;

    private View.OnClickListener mOKListener = new View.OnClickListener() {
        public void onClick(View v) {
            if (save()) {
                SiteNavigationAddDialog.this
                .setResult(Activity.RESULT_OK, (new Intent()).putExtra("need_refresh", true));
                finish();
            }
        }
    };

    private View.OnClickListener mCancelListener = new View.OnClickListener() {
        public void onClick(View v) {
            finish();
        }
    };

    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.site_navigation_add);

        String name = null;
        String url = null;

        mMap = getIntent().getExtras();
        Log.d("@M_" + XLOGTAG, "onCreate mMap is : " + mMap);
        if (mMap != null) {
            Bundle b = mMap.getBundle("websites");
            if (b != null) {
                mMap = b;
            }
            name = mMap.getString("name");
            url = mMap.getString("url");
            mIsAdding = mMap.getBoolean("isAdding");
        }

        //The original url that is editting
        mItemUrl = url;
        mItemName = name;

        mName = (EditText) findViewById(R.id.title);
        mName.setText(name);
        mAddress = (EditText) findViewById(R.id.address);
        // Do not show about:blank + number
        if (url.startsWith("about:blank")) {
            mAddress.setText("about:blank");
        } else {
            mAddress.setText(url);
        }
        mDialogText = (TextView) findViewById(R.id.dialog_title);
        if (mIsAdding) {
            mDialogText.setText(R.string.add);
        }

        mButtonOK = (Button) findViewById(R.id.OK);
        mButtonOK.setOnClickListener(mOKListener);

        mButtonCancel = (Button) findViewById(R.id.cancel);
        mButtonCancel.setOnClickListener(mCancelListener);

        if (!getWindow().getDecorView().isInTouchMode()) {
            mButtonOK.requestFocus();
        }
    }

    /**
     * Runnable to save a website, so it can be performed in its own thread.
     */
   private class SaveSiteNavigationRunnable implements Runnable {
        private Message mMessage;
        public SaveSiteNavigationRunnable(Message msg) {
            mMessage = msg;
        }
        public void run() {
            // Unbundle website data.
            Bundle bundle = mMessage.getData();
            String title = bundle.getString("title");
            String url = bundle.getString("url");
            String itemUrl = bundle.getString("itemUrl");
            Boolean toDefaultThumbnail = bundle.getBoolean("toDefaultThumbnail");
            // Save to the site navigation DB.
            ContentResolver cr = SiteNavigationAddDialog.this.getContentResolver();
            Cursor cursor = null;
            try {
                cursor = cr.query(SiteNavigation.SITE_NAVIGATION_URI,
                        new String[] {SiteNavigation.ID}, "url = ? COLLATE NOCASE",
                        new String[] {itemUrl}, null);
                if (cursor != null && cursor.moveToFirst()) {
                    ContentValues values = new ContentValues();
                    values.put(SiteNavigation.TITLE, title);
                    values.put(SiteNavigation.URL, url);
                    values.put(SiteNavigation.WEBSITE, 1 + "");
                    if (toDefaultThumbnail) {
                        ByteArrayOutputStream os = new ByteArrayOutputStream();
                        Bitmap bm = BitmapFactory.decodeResource(
                                SiteNavigationAddDialog.this.getResources(),
                                R.raw.sitenavigation_thumbnail_default);
                        bm.compress(Bitmap.CompressFormat.PNG, 100, os);
                        values.put(SiteNavigation.THUMBNAIL, os.toByteArray());
                    }
                    Uri uri = ContentUris.withAppendedId(SiteNavigation.SITE_NAVIGATION_URI,
                                                         cursor.getLong(0));
                    Log.d("@M_" + XLOGTAG, "SaveSiteNavigationRunnable uri is : " + uri);
                    cr.update(uri, values, null, null);
                } else {
                    Log.e("@M_" + XLOGTAG, "saveSiteNavigationItem the item does not exist!");
                }
            } catch (IllegalStateException e) {
                Log.e("@M_" + XLOGTAG, "saveSiteNavigationItem", e);
            } finally {
                if (null != cursor) {
                    cursor.close();
                }
            }
        }
    }

    /**
     * Parse the data entered in the dialog and post a message
     * to update the Site Navigation database.
     */
    boolean save() {

        String name = mName.getText().toString().trim();
        String unfilteredUrl = UrlUtils.fixUrl(mAddress.getText().toString());
        boolean emptyTitle = name.length() == 0;
        boolean emptyUrl = unfilteredUrl.trim().length() == 0;
        Resources r = getResources();
        if (emptyTitle || emptyUrl) {
            if (emptyTitle) {
                mName.setError(r.getText(R.string.website_needs_title));
            }
            if (emptyUrl) {
                mAddress.setError(r.getText(R.string.website_needs_url));
            }
            return false;
        }

        if (!name.equals(mItemName) && isSiteNavigationTitle(this, name)) {
            mName.setError(r.getText(R.string.duplicate_site_navigation_title));
            return false;
        }
        String url = unfilteredUrl.trim();
        try {
            // We allow website with a javascript: scheme, but these will in most cases
            // fail URI parsing, so don't try it if that's the kind of bookmark we have.

            if (!url.toLowerCase().startsWith("javascript:")) {
                URI uriObj = new URI(url);
                String scheme = uriObj.getScheme();
                if (!urlHasAcceptableScheme(url)) {
                    // If the scheme was non-null, let the user know that we
                    // can't save their website. If it was null, we'll assume
                    // they meant http when we parse it in the WebAddress class.
                    if (scheme != null) {
                        mAddress.setError(r.getText(R.string.site_navigation_cannot_save_url));
                        return false;
                    }
                    WebAddress address;
                    try {
                        address = new WebAddress(unfilteredUrl);
                    } catch (ParseException e) {
                        throw new URISyntaxException("", "");
                    }
                    if (address.getHost().length() == 0) {
                        throw new URISyntaxException("", "");
                    }
                    url = address.toString();
                } else {
                    String mark = "://";
                    int iRet = -1;
                    if (null != url) {
                        iRet = url.indexOf(mark);
                    }
                    if (iRet > 0 && url.indexOf("/", iRet + mark.length()) < 0) {
                        url = url + "/";
                        Log.d("@M_" + XLOGTAG, "URL=" + url);
                    }
                }

                try {
                    byte[] bytes = url.getBytes("UTF-8");
                    if (url.length() != bytes.length) {
                        throw new URISyntaxException("", "");
                    }
                } catch (UnsupportedEncodingException e) {
                    throw new URISyntaxException("", "");
                }
            }
        } catch (URISyntaxException e) {
            mAddress.setError(r.getText(R.string.bookmark_url_not_valid));
            return false;
        }
        /// M: workaround for conflict between java.net.URL and Chrome GURL. @ {
        try {
            URL unModifyUrl = new URL(url);
            String path = unModifyUrl.getPath();
            if ((path.equals("/") && url.endsWith("."))
                    || (path.equals("") && url.endsWith(".."))) {
                mAddress.setError(r.getText(R.string.bookmark_url_not_valid));
                return false;
            }
        } catch (MalformedURLException e) {
            mAddress.setError(r.getText(R.string.bookmark_url_not_valid));
            return false;
        }
        /// @ }
        // When it is adding, avoid duplicate url that already existing in the database
        if (!mItemUrl.equals(url)) {
            boolean exist = isSiteNavigationUrl(this, url, url);
            if (exist) {
                mAddress.setError(r.getText(R.string.duplicate_site_navigation_url));
                return false;
            }
        }

        // Process the about:blank url, because we should ensure the url is unique
        if (url.startsWith("about:blank")) {
            url = mItemUrl;
        }

        // Post a message to write to the DB.
        Bundle bundle = new Bundle();
        bundle.putString("title", name);
        bundle.putString("url", url);
        bundle.putString("itemUrl", mItemUrl);
        if (!mItemUrl.equals(url)) {
            bundle.putBoolean("toDefaultThumbnail", true);
        } else {
            bundle.putBoolean("toDefaultThumbnail", false);
        }
        Message msg = Message.obtain(mHandler, SAVE_SITE_NAVIGATION);
        msg.setData(bundle);
        // Start a new thread so as to not slow down the UI
        Thread t = new Thread(new SaveSiteNavigationRunnable(msg));
        t.start();

        return true;
    }
    /** check both url and original url.
    *   @param context context of activity
    *   @param itemUrl url of current webview
    *   @param originalUrl original url of current webview
    *   @return if this is a site navigation web
    */
    public static boolean isSiteNavigationUrl(Context context,
                                              String itemUrl,
                                              String originalUrl) {
        ContentResolver cr = context.getContentResolver();
        Cursor cursor = null;
        try {
            cursor = cr.query(SiteNavigation.SITE_NAVIGATION_URI,
                    new String[] {SiteNavigation.TITLE},
                    "url = ? COLLATE NOCASE OR url = ? COLLATE NOCASE",
                    new String[] {itemUrl, originalUrl}, null);
            if (null != cursor && cursor.moveToFirst()) {
                Log.d("@M_" + XLOGTAG, "isSiteNavigationUrl will return true.");
                return true;
            }
        } catch (IllegalStateException e) {
            Log.e("@M_" + XLOGTAG, "isSiteNavigationUrl", e);
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }

        return false;
    }

    public static boolean isSiteNavigationTitle(Context context, String itemTitle) {
        ContentResolver cr = context.getContentResolver();
        Cursor cursor = null;
        try {
            cursor = cr.query(SiteNavigation.SITE_NAVIGATION_URI,
                    new String[] {SiteNavigation.TITLE}, "title = ?",
                                  new String[] {itemTitle}, null);
            if (null != cursor && cursor.moveToFirst()) {
                Log.d("@M_" + XLOGTAG, "isSiteNavigationTitle will return true.");
                return true;
            }
        } catch (IllegalStateException e) {
            Log.e("@M_" + XLOGTAG, "isSiteNavigationTitle", e);
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        return false;
    }

    private static final String ACCEPTABLE_WEBSITE_SCHEMES[] = {
        "http:",
        "https:",
        "about:",
        "data:",
        "javascript:",
        "file:",
        "content:",
        "rtsp:"
    };
    private static boolean urlHasAcceptableScheme(String url) {
        if (url == null) {
            return false;
        }

        for (int i = 0; i < ACCEPTABLE_WEBSITE_SCHEMES.length; i++) {
            if (url.startsWith(ACCEPTABLE_WEBSITE_SCHEMES[i])) {
                return true;
            }
        }
        return false;
    }
}
