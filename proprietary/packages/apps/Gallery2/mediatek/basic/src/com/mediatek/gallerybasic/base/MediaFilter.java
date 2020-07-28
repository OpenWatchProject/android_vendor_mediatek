package com.mediatek.gallerybasic.base;

import android.content.Intent;

import com.mediatek.gallerybasic.util.Log;

import java.util.ArrayList;

/**
 * Save current filter info and provide kinds of condition.
 */
public class MediaFilter {
    private static final String TAG = "MtkGallery2/MediaFilter";
    public static final int INVALID_BUCKET_ID = 0xFFFFFFFF;
    private static int sCurrentFlag = 1;

    private static ArrayList<IFilter> sFilterArray = new ArrayList<IFilter>();
    private int mFlag;

    /**
     * Add new filter to MediaFilter.
     * @param filter
     *            The filter for one feature
     */
    public static void registerFilter(IFilter filter) {
        sFilterArray.add(filter);
        Log.d(TAG, "<registerFilter> filter = " + filter);
    }

    /**
     * Request a flag bit. The function always call in {@link IFilter}. {@link IFilter} request a
     * flag bit for itself, and set enable/disable to this flag.
     * @return The flag bit
     */
    public static synchronized int requestFlagBit() {
        int flag = sCurrentFlag;
        sCurrentFlag = sCurrentFlag << 1;
        return flag;
    }

    /**
     * Constructor, it will set flag as default.
     */
    public MediaFilter() {
        for (IFilter filter : sFilterArray) {
            filter.setDefaultFlag(this);
        }
    }

    public int getFlag() {
        return mFlag;
    }

    public int hashCode() {
        return mFlag;
    }

    public boolean equals(MediaFilter filter) {
        if (filter == null) {
            return false;
        }
        return mFlag == filter.getFlag();
    }

    public void setFlagEnable(int flag) {
        mFlag |= flag;
    }

    public void setFlagDisable(int flag) {
        mFlag &= ~flag;
    }

    /**
     * Set the flag of MediaFilter according to the intent info.
     * @param intent
     *            The intent to launch gallery
     */
    public void setFlagFromIntent(Intent intent) {
        if (intent == null) {
            return;
        }
        for (IFilter filter : sFilterArray) {
            filter.setFlagFromIntent(intent, this);
        }
    }

    /**
     * Get the query condition for image.
     * @param whereClause
     *            The current query condition
     * @param bucketID
     *            The bucket id of current query folder
     * @return The new query condition
     */
    public String getExtWhereClauseForImage(String whereClause, int bucketID) {
        for (IFilter filter : sFilterArray) {
            whereClause = AND(whereClause, filter.getWhereClauseForImage(mFlag, bucketID));
        }
        return whereClause;
    }

    /**
     * Get the query condition for video.
     * @param whereClause
     *            The current query condition
     * @param bucketID
     *            The bucket id of current query folder
     * @return The new query condition
     */
    public String getExtWhereClauseForVideo(String whereClause, int bucketID) {
        for (IFilter filter : sFilterArray) {
            whereClause = AND(whereClause, filter.getWhereClauseForVideo(mFlag, bucketID));
        }
        return whereClause;
    }

    /**
     * Get the common query condition, not special for image or video
     * @param flag
     *            The flag of current MediaFilter
     * @param bucketID
     *            The bucket id of current folder
     * @returnThe query condition
     */
    public String getExtWhereClause(String whereClause, int bucketID) {
        for (IFilter filter : sFilterArray) {
            whereClause = AND(whereClause, filter.getWhereClause(mFlag, bucketID));
        }
        return whereClause;
    }

    /**
     * Get the delete condition for image.
     * @param whereClause
     *            The current delete condition
     * @param bucketID
     *            The bucket id of current delete folder
     * @return The new delete condition
     */
    public String getExtDeleteWhereClauseForImage(String whereClause, int bucketID) {
        for (IFilter filter : sFilterArray) {
            whereClause =
                    AND(whereClause, filter.getDeleteWhereClauseForImage(mFlag, bucketID));
        }
        return whereClause;
    }

    /**
     * Get the delete condition for video.
     * @param whereClause
     *            The current delete condition
     * @param bucketID
     *            The bucket id of current delete folder
     * @return The new delete condition
     */
    public String getExtDeleteWhereClauseForVideo(String whereClause, int bucketID) {
        for (IFilter filter : sFilterArray) {
            whereClause =
                    AND(whereClause, filter.getDeleteWhereClauseForVideo(mFlag, bucketID));
        }
        return whereClause;
    }

    /**
     * Combine two conditions in AND relationship.
     * @param add1
     *            Condition 1
     * @param add2
     *            Condition 2
     * @return The new condition
     */
    public static String AND(String add1, String add2) {
        if ((add1 == null || add1.equals("")) && (add2 == null || add2.equals(""))) {
            return "";
        }
        if (add1 == null || add1.equals("")) {
            return add2;
        }
        if (add2 == null || add2.equals("")) {
            return add1;
        }
        return "(" + add1 + ") AND (" + add2 + ")";
    }

    /**
     * Combine two conditions in OR relationship.
     * @param or1
     *            Condition1
     * @param or2
     *            Condition2
     * @return The new condition
     */
    public static String OR(String or1, String or2) {
        if ((or1 == null || or1.equals("")) && (or2 == null || or2.equals(""))) {
            return "";
        }
        if (or1 == null || or1.equals("")) {
            return or2;
        }
        if (or2 == null || or2.equals("")) {
            return or1;
        }
        return "(" + or1 + ") OR (" + or2 + ")";
    }
}
