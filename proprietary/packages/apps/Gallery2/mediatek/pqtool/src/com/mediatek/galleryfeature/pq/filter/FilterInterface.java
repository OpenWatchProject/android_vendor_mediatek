package com.mediatek.galleryfeature.pq.filter;

import java.util.ArrayList;

/**
 * Interface with native.
 */
public interface FilterInterface {

    /**
     * Init the filter object, get default value.
     */
    public void init();

    /**
     * Get Min value of the filter.
     * @return the Min value.
     */
    public String getMinValue();

    /**
     * Get Max value of the filter.
     * @return the Max value.
     */
    public String getMaxValue();

    /**
     * Get current filter value.
     * @return the current value string of this filter.
     */
    public String getCurrentValue();

    /**
     * Get current filter value set seekBar default value.
     * @return the current filter value.
     */
    public String getSeekbarProgressValue();

    /**
     * Set current filter value to native.
     * @param index current filter value.
     */
    public void setIndex(int index);

    /**
     * Save current filter value.
     * @param progress current filter value.
     */
    public void setCurrentIndex(int progress);

    /**
     * Get all filter.
     * @return ArrayList of filter.
     */
    public ArrayList<FilterInterface> getFilterList();

    /**
     * On resume state while Activity onResume state.
     */
    public void onResume();

    /**
     * On destroy state while Activity onDestroy state.
     */
    public void onDestroy();

    /**
     * Get default value set to native.
     * @return the default value.
     */
    public int getDefaultIndex();

    /**
     * Get current value set to native.
     * @return the current value.
     */
    public int getCurrentIndex();

    /**
     * Get range of this filter.
     * @return the range value.
     */
    public int getRange();
}
