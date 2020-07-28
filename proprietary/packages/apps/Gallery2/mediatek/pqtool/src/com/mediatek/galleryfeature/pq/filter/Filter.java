package com.mediatek.galleryfeature.pq.filter;

import com.mediatek.gallerybasic.util.Log;

import com.mediatek.galleryportable.SystemPropertyUtils;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * Super class of All Filter.
 */
public class Filter implements FilterInterface {

    public static final String TAG = "MtkGallery2/Filter";
    public static final String MIN_VALUE = "textViewMinValue";
    public static final String RANGE = "textViewMaxValue";
    public static final String CURRRENT_INDEX = "textViewCurrentIndex";
    public static final String SEEKBAR_PROGRESS = "seekbarProgress";
    protected int mDefaultIndex;
    protected int mRange;
    protected int mCurrentIndex;

    /// M: [FEATURE.ADD] <Global PQ > @{
    public static final String GLOBAL_PQ_PROPERTY = "ro.globalpq.support";
    /// @}

    /// M: [FEATURE.ADD] <Global PQ > @{
    public static final boolean IS_GLOBALPQ_SUPPORT = SystemPropertyUtils.get(GLOBAL_PQ_PROPERTY)
                .equals("1");
    /// @}

    static {
        System.loadLibrary("PQjni");
    }

    public Map<String, String> map = new HashMap<String, String>();

    public Map<String, String> getDate() {
        return map;
    }

    /**
     * Constructor.
     */
    public Filter() {
        init();
        map.put(MIN_VALUE, getMinValue());
        map.put(RANGE, getMaxValue());
        map.put(CURRRENT_INDEX, getCurrentValue());
        map.put(SEEKBAR_PROGRESS, getSeekbarProgressValue());
        Log.d(TAG, "<Filter> Create [" + this.getClass().getName()
                + " ]: MIN_VALUE=" + getMinValue() + " RANGE=" + getMaxValue()
                + " CURRRENT_INDEX=" + getCurrentValue()
                + "  SEEKBAR_PROGRESS=" + getSeekbarProgressValue());
    }

    /**
     * Check if the filter should add to list.
     * @param list the list that filter add to.
     * @return whether add to list or not.
     */
    public boolean addToList(ArrayList<FilterInterface> list) {
        if (Integer.parseInt(getMaxValue()) > 0) {
            list.add((FilterInterface) this);
            Log.d(TAG, "<addToList>:::" + this.getClass().getName()
                    + " has alread addToList! ");
            return true;
        }
        return false;
    }

    @Override
    public ArrayList<FilterInterface> getFilterList() {
        ArrayList<FilterInterface> list = new ArrayList<FilterInterface>();

        /// M: [FEATURE.ADD] <Global PQ > @{
        if(!IS_GLOBALPQ_SUPPORT) {
            (new FilterSharpAdj()).addToList(list);
        }
        /// @}
        //(new FilterSharpAdj()).addToList(list);
        (new FilterSatAdj()).addToList(list);
        (new FilterHueAdj()).addToList(list);
        (new FilterSkinToneH()).addToList(list);
        (new FilterSkinToneS()).addToList(list);
        (new FilterSkyToneH()).addToList(list);
        (new FilterSkyToneS()).addToList(list);
        (new FilterGetXAxis()).addToList(list);
        (new FilterGetYAxis()).addToList(list);
        (new FilterGrassToneH()).addToList(list);
        (new FilterGrassToneS()).addToList(list);
        (new FilterContrastAdj()).addToList(list);
        return list;
    }

    protected native int nativeGetContrastAdjRange();

    protected native int nativeGetContrastAdjIndex();

    protected native boolean nativeSetContrastAdjIndex(int index);

    protected native int nativeGetXAxisRange();

    protected native int nativeGetXAxisIndex();

    protected native boolean nativeSetXAxisIndex(int index);

    protected native int nativeGetYAxisRange();

    protected native int nativeGetYAxisIndex();

    protected native boolean nativeSetYAxisIndex(int index);

    protected native int nativeGetGrassToneHRange();

    protected native int nativeGetGrassToneHIndex();

    protected native boolean nativeSetGrassToneHIndex(int index);

    protected native int nativeGetGrassToneSRange();

    protected native int nativeGetGrassToneSIndex();

    protected native boolean nativeSetGrassToneSIndex(int index);

    protected native int nativeGetHueAdjRange();

    protected native int nativeGetHueAdjIndex();

    protected native boolean nativeSetHueAdjIndex(int index);

    protected native int nativeGetSatAdjRange();

    protected native int nativeGetSatAdjIndex();

    protected native boolean nativeSetSatAdjIndex(int index);

    protected native int nativeGetSharpAdjRange();

    protected native int nativeGetSharpAdjIndex();

    protected native boolean nativeSetSharpAdjIndex(int index);

    protected native int nativeGetSkinToneHRange();

    protected native int nativeGetSkinToneHIndex();

    protected native boolean nativeSetSkinToneHIndex(int index);

    protected native int nativeGetSkinToneSRange();

    protected native int nativeGetSkinToneSIndex();

    protected native boolean nativeSetSkinToneSIndex(int index);

    protected native int nativeGetSkyToneHRange();

    protected native int nativeGetSkyToneHIndex();

    protected native boolean nativeSetSkyToneHIndex(int index);

    protected native int nativeGetSkyToneSRange();

    protected native int nativeGetSkyToneSIndex();

    protected native boolean nativeSetSkyToneSIndex(int index);

    public String getCurrentValue() {
        return Integer.toString(mCurrentIndex);
    }

    public String getMaxValue() {
        return Integer.toString(mRange - 1);
    }

    public String getMinValue() {
        return "0";
    }

    public String getSeekbarProgressValue() {
        return Integer.toString(mCurrentIndex);
    }

    @Override
    public void init() {

    }

    public void setCurrentIndex(int progress) {
        mCurrentIndex = progress;
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
    }

    @Override
    public void onResume() {
        // TODO Auto-generated method stub

    }

    public int getCurrentIndex() {
        return mCurrentIndex;
    }

    public int getRange() {
        return mRange;
    }

    public int getDefaultIndex() {
        return mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {

    }

}
