/*
 * Copyright (C) 2011 The Android Open Source Project
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


import android.content.ContentProvider;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.database.sqlite.SQLiteFullException;
import android.database.sqlite.SQLiteDiskIOException;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.text.TextUtils;
import android.util.Log;

import com.android.browser.Extensions;
import com.android.browser.R;
import com.android.browser.provider.BrowserProvider2;
import com.mediatek.browser.ext.IBrowserSiteNavigationExt;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

/**
 * M: Site navigation
 */
public class SiteNavigationProvider extends ContentProvider {

    private static final String TAG = "browser/SiteNavigationProvider";

    private static final String TABLE_WEB_SITES = "websites";

    private static final int WEB_SITES_ALL = 0;
    private static final int WEB_SITES_ID = 1;

    private static final UriMatcher S_URI_MATCHER = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        S_URI_MATCHER.addURI(SiteNavigation.AUTHORITY, "websites" , WEB_SITES_ALL);
        S_URI_MATCHER.addURI(SiteNavigation.AUTHORITY, "websites/#" , WEB_SITES_ID);
    }

    private static final Uri NOTIFICATION_URI = SiteNavigation.SITE_NAVIGATION_URI;

    private SiteNavigationDatabaseHelper mOpenHelper;
    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        //Current not used, just return 0
        return 0;
    }

    @Override
    public String getType(Uri uri) {
        //Current not used, just return null
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        //Current not used, just return null
        return null;
    }

    @Override
    public boolean onCreate() {
        mOpenHelper =  new SiteNavigationDatabaseHelper(this.getContext());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(TABLE_WEB_SITES);

        switch (S_URI_MATCHER.match(uri)) {
        case WEB_SITES_ALL:
            break;
        case WEB_SITES_ID:
            qb.appendWhere(SiteNavigation.ID + "=" + uri.getPathSegments().get(0));
            break;
        default:
            Log.e("@M_" + TAG, "SiteNavigationProvider query Unknown URI: " + uri);
            return null;
        }

        String orderBy;
        if (TextUtils.isEmpty(sortOrder)) {
            orderBy = null;
        } else {
            orderBy = sortOrder;
        }

        //Get the database and run the query
        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, orderBy);
        if (c != null) {
            //Tell the cursor what uri to watch, so it knows when its source data changes
            c.setNotificationUri(getContext().getContentResolver(), NOTIFICATION_URI);
        }
        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection,
            String[] selectionArgs) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int count = 0;

        switch (S_URI_MATCHER.match(uri)) {
        case WEB_SITES_ALL:
            count = db.update(TABLE_WEB_SITES, values, selection, selectionArgs);
            break;
        case WEB_SITES_ID:
            String newIdSelection = SiteNavigation.ID + "=" + uri.getLastPathSegment()
            + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : "");
            try {
                count = db.update(TABLE_WEB_SITES, values, newIdSelection, selectionArgs);
            } catch (SQLiteFullException e) {
                Log.e(TAG, "Here happened SQLiteFullException");
            } catch (SQLiteDiskIOException e) {
                Log.e(TAG, "Here happened SQLiteDiskIOException");
            }
            break;
        default:
            Log.e("@M_" + TAG, "SiteNavigationProvider update Unknown URI: " + uri);
            return count;
        }

        if (count > 0) {
            ContentResolver cr = getContext().getContentResolver();
            cr.notifyChange(uri, null);
        }
        return count;
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) {
        try {
            ParcelFileDescriptor[] pipes = ParcelFileDescriptor.createPipe();
            final ParcelFileDescriptor write = pipes[1];
            AssetFileDescriptor afd = new AssetFileDescriptor(write, 0, -1);
            new RequestHandlerSiteNavigation(getContext(), uri, afd.createOutputStream())
            .start();
            return pipes[0];
        } catch (IOException e) {
            Log.e(TAG, "Failed to handle request: " + uri, e);
            return null;
        }
    }

    private class SiteNavigationDatabaseHelper extends SQLiteOpenHelper {

        private Context mContext;
        static final String DATABASE_NAME = "websites.db";

        public SiteNavigationDatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, 1); //"1" is the db version here
            // TODO Auto-generated constructor stub
            mContext = context;
        }

        public SiteNavigationDatabaseHelper(Context context, String name,
                CursorFactory factory, int version) {
            super(context, name, factory, version);
            // TODO Auto-generated constructor stub
            mContext = context;
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            // TODO Auto-generated method stub

            // create table for site navigation
            createTable(db);
            // initial table , insert websites to table websites
            initTable(db);
        }

        @Override
        public void onUpgrade(SQLiteDatabase arg0, int arg1, int arg2) {
            // TODO Auto-generated method stub
        }

        private void createTable(SQLiteDatabase db) {
            db.execSQL("CREATE TABLE websites (" +
                  SiteNavigation.ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                  SiteNavigation.URL + " TEXT," +
                  SiteNavigation.TITLE + " TEXT," +
                  SiteNavigation.DATE_CREATED + " LONG," +
                  SiteNavigation.WEBSITE + " INTEGER," +
                  SiteNavigation.THUMBNAIL + " BLOB DEFAULT NULL," +
                  SiteNavigation.FAVICON + " BLOB DEFAULT NULL," +
                  SiteNavigation.DEFAULT_THUMB + " TEXT" +
                  ");");

        }

        private IBrowserSiteNavigationExt mBrowserSiteNavigationExt = null;
        //initial table , insert websites to table websites
        private void initTable(SQLiteDatabase db) {
            CharSequence[] websites = null;

            mBrowserSiteNavigationExt = Extensions.getSiteNavigationPlugin(mContext);
            if (null == (websites = mBrowserSiteNavigationExt.getPredefinedWebsites())) {
                websites = mContext.getResources()
                                   .getTextArray(R.array.predefined_websites_default_optr);
            }

            if (websites == null) {
                return;
            }
            int websiteSize = websites.length;
            int siteNavigationWebsiteNum = 0;

            if (!mContext.getResources().getBoolean(R.bool.isTablet)) {
                siteNavigationWebsiteNum = mBrowserSiteNavigationExt.getSiteNavigationCount();
                if (siteNavigationWebsiteNum == 0) {
                    siteNavigationWebsiteNum = SiteNavigation.WEBSITE_NUMBER;
                }
            } else {
                siteNavigationWebsiteNum = SiteNavigation.WEBSITE_NUMBER_FOR_TABLET;
            }

            if (websiteSize > siteNavigationWebsiteNum * 3) {
                websiteSize = siteNavigationWebsiteNum * 3;
            }

            ByteArrayOutputStream os = null;
            Bitmap bm = null;
            ContentValues values = null;
            String fileName = null;
            try {
                for (int i = 0; i < websiteSize; i = i + 3) {
                    os = new ByteArrayOutputStream();
                    fileName = websites[i + 2].toString();
                    try {
                        if (fileName != null && fileName.length() != 0) {
                            int id = mContext.getResources()
                                     .getIdentifier(fileName, "raw", mContext.getPackageName());
                            bm = BitmapFactory.decodeResource(mContext.getResources(), id);
                        } else {
                            bm = BitmapFactory.decodeResource(mContext.getResources(),
                                    R.raw.sitenavigation_thumbnail_default);
                        }
                    } finally {
                        if (bm == null) {
                            bm = BitmapFactory.decodeResource(mContext.getResources(),
                                    R.raw.sitenavigation_thumbnail_default);
                        }
                    }

                    bm.compress(Bitmap.CompressFormat.PNG, 100, os);
                    values = new ContentValues();
                    CharSequence websiteDestination
                            = BrowserProvider2.replaceSystemPropertyInString(
                                               mContext, websites[i + 1]);
                    values.put(SiteNavigation.URL, websiteDestination.toString());
                    values.put(SiteNavigation.TITLE, websites[i].toString());
                    values.put(SiteNavigation.DATE_CREATED, 0 + "");
                    values.put(SiteNavigation.WEBSITE, 1 + "");
                    values.put(SiteNavigation.THUMBNAIL, os.toByteArray());
                    db.insertOrThrow(TABLE_WEB_SITES, SiteNavigation.URL,
                            values);
                    }
                    // Insert some empty rows if predefined websites is less than
                    // designated number: about:blank
                    for (int i = websiteSize / 3; i < siteNavigationWebsiteNum; i++) {
                        os = new ByteArrayOutputStream();
                        bm = BitmapFactory.decodeResource(mContext.getResources(),
                                R.raw.sitenavigation_thumbnail_default);
                        bm.compress(Bitmap.CompressFormat.PNG, 100, os);
                        values = new ContentValues();
                        // Url must be unique, because we use this column to query and update
                        // The number after about:blank is just the id in the database
                        values.put(SiteNavigation.URL, "about:blank" + (i + 1));
                        values.put(SiteNavigation.TITLE, "about:blank");
                        values.put(SiteNavigation.DATE_CREATED, 0 + "");
                        values.put(SiteNavigation.WEBSITE, 1 + "");
                        values.put(SiteNavigation.THUMBNAIL, os.toByteArray());
                        db.insertOrThrow(TABLE_WEB_SITES, SiteNavigation.URL, values);

                    }
            } catch (ArrayIndexOutOfBoundsException e) {
                Log.e("@M_" + TAG, "initTable: ArrayIndexOutOfBoundsException: " + e);
            }
        }
    }

}
