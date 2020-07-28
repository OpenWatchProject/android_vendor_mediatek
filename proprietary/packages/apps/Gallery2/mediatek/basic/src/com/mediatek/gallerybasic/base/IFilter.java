package com.mediatek.gallerybasic.base;

import android.content.Intent;

/**
 * Filter the record of image and video view in some rules.
 */
public interface IFilter {
    /**
     * Set the flag of MediaFilter in default case.
     * @param filter
     *            The MediaFilter which will be set flag
     */
    public void setDefaultFlag(MediaFilter filter);

    /**
     * Set the flag of MediaFilter according to the intent info.
     * @param intent
     *            The intent to launch gallery
     * @param filter
     *            The MediaFilter which will be set flag
     */
    public void setFlagFromIntent(Intent intent, MediaFilter filter);

    /**
     * Get the common query condition, not special for image or video
     * @param flag
     *            The flag of current MediaFilter
     * @param bucketID
     *            The bucket id of current folder
     * @returnThe query condition
     */
    public String getWhereClause(int flag, int bucketID);

    /**
     * Get the query condition for image.
     * @param flag
     *            The flag of current MediaFilter
     * @param bucketID
     *            The bucket id of current query folder
     * @return The query condition
     */
    public String getWhereClauseForImage(int flag, int bucketID);

    /**
     * Get the query condition for video.
     * @param flag
     *            The flag of current MediaFilter
     * @param bucketID
     *            The bucket id of current query folder
     * @return The query condition
     */
    public String getWhereClauseForVideo(int flag, int bucketID);

    /**
     * Get the delete condition for image.
     * @param flag
     *            The flag of current MediaFilter
     * @param bucketID
     *            The bucket id of current delete folder
     * @return The delete condition
     */
    public String getDeleteWhereClauseForImage(int flag, int bucketID);

    /**
     * Get the delete condition for video.
     * @param flag
     *            The flag of current MediaFilter
     * @param bucketID
     *            The bucket id of current delete folder
     * @return The delete condition
     */
    public String getDeleteWhereClauseForVideo(int flag, int bucketID);
}
