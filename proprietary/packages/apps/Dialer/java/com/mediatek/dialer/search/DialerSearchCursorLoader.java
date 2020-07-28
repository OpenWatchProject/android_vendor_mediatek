/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.dialer.search;

import android.app.LoaderManager;
import android.content.AsyncTaskLoader;
import android.content.Context;
import android.content.Loader;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.MergeCursor;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import com.android.dialer.app.R;
import com.android.dialer.common.LogUtil;
import com.android.dialer.searchfragment.common.Projections;
import com.android.dialer.searchfragment.common.SearchCursor;
import com.mediatek.dialer.database.DialerDatabaseHelperEx;
import com.mediatek.dialer.search.DialerSearchNameMatcher;
import com.mediatek.dialer.util.DialerSearchUtils;

/**
 * M: [MTK Dialer Search]
 * Implements a Loader<Cursor> class to asynchronously load SmartDial search results.
 */
public class DialerSearchCursorLoader extends AsyncTaskLoader<Cursor> {

    private final String TAG = "DialerSearchCursorLoader";
    private final Context mContext;
    private Cursor mCursor;
    private String mQuery;
    // Set true if it can not apply MTK dialer search, and go to default flow.
    private ForceLoadContentObserver mObserver;
    private DialerSearchNameMatcher mDsNameMatcher;
    private int mLoaderId;
    private LoaderManager mLoaderManager;

    public DialerSearchCursorLoader(Context context, int id, LoaderManager loaderManager) {
        super(context);
        mContext = context;
        mLoaderId = id;
        mLoaderManager = loaderManager;
    }

    /**
     * Configures the query string to be used to find SmartDial matches.
     * @param query The query string user typed.
     */
    public void configureQuery(String query, boolean isSmartQuery) {

        LogUtil.d(TAG, "MTK-DialerSearch, Configure new query to be " + LogUtil.sanitizePii(query));

        mQuery = query;
        if (!isSmartQuery) {
            mQuery = DialerSearchUtils.stripTeleSeparators(query);
        }
        /** Constructs a name matcher object for matching names. */
        mDsNameMatcher = new DialerSearchNameMatcher();
    }

    /**
     * Queries the Contacts database and loads results in background.
     * @return Cursor of contacts that matches the SmartDial query.
     */
    @Override
    public Cursor loadInBackground() {

        LogUtil.i(TAG, "YF: MTK-DialerSearch, LoadInBackground.mQuery: " + LogUtil.sanitizePii(mQuery));

        final DialerSearchHelper dialerSearchHelper = DialerSearchHelper.getInstance(mContext);
        /// M: [MTK-Dialer-Search] @{
        long start = System.currentTimeMillis();
        Cursor cursor = dialerSearchHelper.getSmartDialerSearchResults(mQuery, mDsNameMatcher);
        LogUtil.i(TAG, String.format("***DSTime:%4dms, Matched: %-3d, Query: %s",
                    (System.currentTimeMillis() - start),
                    (null != cursor ?  cursor.getCount() : 0),
                    LogUtil.sanitizePii(mQuery)));
        /// @}
        if (cursor != null) {
            LogUtil.i(TAG, "YF:  MTK-DialerSearch, loadInBackground, result.getCount: "
                    + cursor.getCount());
            return MtkSearchCursor.newInstance(mContext,cursor);
        } else {
            LogUtil.i(TAG, "YF: MTK-DialerSearch, ----cursor is null----");
            return null;
        }
    }


     /// M: @ {
     static class MtkSearchCursor extends MergeCursor implements SearchCursor {

      static MtkSearchCursor newInstance(Context context, Cursor searchCursor) {
        if (searchCursor == null || searchCursor.getCount() == 0) {
          LogUtil.i("RegularSearchCursor.newInstance", "YF: Cursor was null or empty");
          return new MtkSearchCursor(new Cursor[] {new MatrixCursor(Projections.CP2_PROJECTION)});
        }

        MatrixCursor headerCursor = new MatrixCursor(HEADER_PROJECTION);
        headerCursor.addRow(new String[] {context.getString(R.string.all_contacts)});
        LogUtil.i("RegularSearchCursor.newInstance", "YF: return MtkSearchCursor");
        return new MtkSearchCursor(new Cursor[] {headerCursor, searchCursor});
      }

      public MtkSearchCursor(Cursor[] cursors) {
        super(cursors);
      }

      @Override
      public boolean isHeader() {
        return isFirst();
      }

      @Override
      public boolean updateQuery(@NonNull String query) {
        return false; // no-op
      }

      @Override
      public long getDirectoryId() {
        return 0; // no-op
      }
    }
    /// @}

    @Override
    public void deliverResult(Cursor cursor) {
        if (isReset()) {
            LogUtil.d(TAG, "MTK-DialerSearch, deliverResult releaseResources " + this);
            /** The Loader has been reset; ignore the result and invalidate the data. */
            releaseResources(cursor);
            return;
        }

        /** Hold a reference to the old data so it doesn't get garbage collected. */
        Cursor oldCursor = mCursor;
        mCursor = cursor;
        /** Get current active loader, register observer only if current loader not changed.
         * this is a workaround of LoaderManager's bug.
         * When user restartLoader frequently, some loader would not called cancel or reset.
         * And this observer leakage happened. */
        Loader currentLoader =  mLoaderManager.getLoader(mLoaderId);

        LogUtil.d(TAG, "MTK-DialerSearch, deliverResult isStarted = " + isStarted()
                + " isAbandon = " +isAbandoned() + "  currentActiveLoader = " + currentLoader
                + " " + this);
        if (null == mObserver && currentLoader == this) {
            mObserver = new ForceLoadContentObserver();
            /*mContext.getContentResolver().registerContentObserver(
                    DialerDatabaseHelperEx.SMART_DIAL_UPDATED_URI, true, mObserver);*/
        }

        if (isStarted()) {
            /** If the Loader is in a started state, deliver the results to the client. */
            super.deliverResult(cursor);
        }

        /** Invalidate the old data as we don't need it any more. */
        if (oldCursor != null && oldCursor != cursor) {
            releaseResources(oldCursor);
        }
    }

    @Override
    protected void onStartLoading() {
        if (mCursor != null) {
            /** Deliver any previously loaded data immediately. */
            deliverResult(mCursor);
        }
        if (mCursor == null) {
            /** Force loads every time as our results change with queries. */
            forceLoad();
        }
    }

    @Override
    protected void onStopLoading() {
        /** The Loader is in a stopped state, so we should attempt to cancel the current load. */
        cancelLoad();
    }

    @Override
    protected void onReset() {
        LogUtil.d(TAG, "MTK-DialerSearch, onReset() "  + this);
        /** Ensure the loader has been stopped. */
        onStopLoading();

        if (null != mObserver) {
            mContext.getContentResolver().unregisterContentObserver(mObserver);
            mObserver = null;
        }

        /** Release all previously saved query results. */
        if (mCursor != null) {
            LogUtil.d(TAG, "MTK-DialerSearch, onReset() releaseResources "  + this);
            releaseResources(mCursor);
            mCursor = null;
        }
    }

    @Override
    public void onCanceled(Cursor cursor) {
        super.onCanceled(cursor);

        LogUtil.d(TAG, "MTK-DialerSearch, onCanceled() " + this);

        if (null != mObserver) {
            mContext.getContentResolver().unregisterContentObserver(mObserver);
            mObserver = null;
        }

        /** The load has been canceled, so we should release the resources associated with 'data'.*/
        releaseResources(cursor);
    }

    private void releaseResources(Cursor cursor) {
        if (cursor != null) {
            LogUtil.w(TAG, "MTK-DialerSearch, releaseResources close cursor " + this);
            cursor.close();
            cursor = null;
        }
    }
}
