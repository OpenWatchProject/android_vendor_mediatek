package com.mediatek.galleryfeature.pq.dcfilter;

import com.mediatek.gallerybasic.util.Log;
import com.mediatek.galleryfeature.pq.filter.FilterInterface;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * Native function declaration filter adjust bar interface for video.
 */
public class DCFilter implements FilterInterface {
    public static final String TAG = "MtkGallery2/DCFilter";
    public static final String MIN_VALUE = "textViewMinValue";
    public static final String RANGE = "textViewMaxValue";
    public static final String CURRRENT_INDEX = "textViewCurrentIndex";
    public static final String SEEKBAR_PROGRESS = "seekbarProgress";
    protected int mDefaultIndex;
    protected int mRange;
    protected int mCurrentIndex;

    protected String mName;
    static {
        System.loadLibrary("PQDCjni");
    }

    public Map<String, String> map = new HashMap<String, String>();

    public Map<String, String> getDate() {
        return map;
    }

    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilter(String name) {
        mName = name;
        initFilter();
    }

    /**
     * Constructor.
     */
    public DCFilter() {
    }

    protected void initFilter() {
        init();
        map.put(MIN_VALUE, getMinValue());
        map.put(RANGE, getMaxValue());
        map.put(CURRRENT_INDEX, getCurrentValue());
        map.put(SEEKBAR_PROGRESS, getSeekbarProgressValue() + "");
        Log.d(TAG, "<initFilter>Create [" + this.getClass().getName()
                + " ]: MIN_VALUE=" + getMinValue() + " RANGE=" + getMaxValue()
                + " CURRRENT_INDEX=" + getCurrentValue()
                + "  SEEKBAR_PROGRESS=" + getSeekbarProgressValue());
    }

    protected boolean addToList(ArrayList<FilterInterface> list) {
        Log.d(
                TAG,
                "<addToList>this " + this.getClass().getName()
                        + " Integer.parseInt(getMaxValue()="
                        + Integer.parseInt(getMaxValue()));
        if (Integer.parseInt(getMaxValue()) > 0) {
            list.add((FilterInterface) this);
            Log.d(TAG, "<addToList>:::" + this.getClass().getName()
                    + " has alread addToList! ");
            return true;
        }
        return false;
    }

    @Override
    public void onResume() {
        Log.d(TAG, "<onResume>: nativeSetTuningMode(1)");
        nativeSetTuningMode(1);
    }

    @Override
    public void onDestroy() {
    }

    @Override
    public ArrayList<FilterInterface> getFilterList() {
        ArrayList<FilterInterface> list = new ArrayList<FilterInterface>();
        (new DCFilterBlackEffectEnable("BlackEffectEnable")).addToList(list); // 1
        (new DCFilterWhiteEffectEnable("WhiteEffectEnable")).addToList(list); // 2
        (new DCFilterStrongBlackEffect("StrongBlackEffect")).addToList(list); // 3
        (new DCFilterStrongWhiteEffect("StrongWhiteEffect")).addToList(list); // 4
        (new DCFilterAdaptiveBlackEffect("AdaptiveBlackEffect"))
                .addToList(list); // 5
        (new DCFilterAdaptiveWhiteEffect("AdaptiveWhiteEffect"))
                .addToList(list); // 6
        (new DCFilterScenceChangeOnceEn("ScenceChangeOnceEn")).addToList(list); // 7
        (new DCFilterScenceChangeControlEn("ScenceChangeControlEn"))
                .addToList(list); // 8
        (new DCFilterScenceChangeControl("ScenceChangeControl"))
                .addToList(list); // 9
        (new DCFilterScenceChangeTh1("ScenceChangeTh1")).addToList(list); // 10
        (new DCFilterScenceChangeTh2("ScenceChangeTh2")).addToList(list); // 11
        (new DCFilterScenceChangeTh3("ScenceChangeTh3")).addToList(list); // 12
        (new DCFilterContentSmooth1("ContentSmooth1")).addToList(list); // 13
        (new DCFilterContentSmooth2("ContentSmooth2")).addToList(list); // 14
        (new DCFilterContentSmooth3("ContentSmooth3")).addToList(list); // 15
        (new DCFilterMiddleRegionGain1("MiddleRegionGain1")).addToList(list); // 16
        (new DCFilterMiddleRegionGain2("MiddleRegionGain")).addToList(list); // 17
        (new DCFilterBlackRegionGain1("BlackRegionGain1")).addToList(list); // 18
        (new DCFilterBlackRegionGain2("BlackRegionGain")).addToList(list); // 19
        (new DCFilterBlackRegionRange("BlackRegionRange")).addToList(list); // 20
        (new DCFilterBlackEffectLevel("BlackEffectLevel")).addToList(list); // 21
        (new DCFilterBlackEffectParam1("BlackEffectParam1")).addToList(list); // 22
        (new DCFilterBlackEffectParam2("BlackEffectParam2")).addToList(list); // 23
        (new DCFilterBlackEffectParam3("BlackEffectParam3")).addToList(list); // 24
        (new DCFilterBlackEffectParam4("BlackEffectParam4")).addToList(list); // 25
        (new DCFilterWhiteRegionGain1("WhiteRegionGain1")).addToList(list); // 26
        (new DCFilterWhiteRegionGain2("WhiteRegionGain")).addToList(list); // 27
        (new DCFilterWhiteRegionRange("WhiteRegionRange")).addToList(list); // 28
        (new DCFilterWhiteEffectLevel("WhiteEffectLevel")).addToList(list); // 29
        (new DCFilterWhiteEffectParam1("WhiteEffectParam1")).addToList(list); // 30
        (new DCFilterWhiteEffectParam2("WhiteEffectParam2")).addToList(list); // 31
        (new DCFilterWhiteEffectParam3("WhiteEffectParam3")).addToList(list); // 32
        (new DCFilterWhiteEffectParam4("WhiteEffectParam4")).addToList(list); // 33
        (new DCFilterContrastAdjust1("ContrastAdjust1")).addToList(list); // 34
        (new DCFilterContrastAdjust2("ContrastAdjust2")).addToList(list); // 35
        (new DCFilterDCChangeSpeedLevel("DCChangeSpeedLevel")).addToList(list); // 36
        (new DCFilterProtectRegionEffect("ProtectRegionEffect"))
                .addToList(list); // 37
        (new DCFilterDCChangeSpeedLevel2("DCChangeSpeedLevel2"))
                .addToList(list); // 38
        (new DCFilterProtectRegionWeight("ProtectRegionWeight"))
                .addToList(list); // 39

        return list;
    }

    protected static native boolean nativeSetTuningMode(int mode);

    // /1
    protected native int nativeGetBlackEffectEnableRange();

    protected native int nativeGetBlackEffectEnableIndex();

    protected native boolean nativeSetBlackEffectEnableIndex(int index);

    // /2
    protected native int nativeGetWhiteEffectEnableRange();

    protected native int nativeGetWhiteEffectEnableIndex();

    protected native boolean nativeSetWhiteEffectEnableIndex(int index);

    // /3
    protected native int nativeGetStrongBlackEffectRange();

    protected native int nativeGetStrongBlackEffectIndex();

    protected native boolean nativeSetStrongBlackEffectIndex(int index);

    // /4
    protected native int nativeGetStrongWhiteEffectRange();

    protected native int nativeGetStrongWhiteEffectIndex();

    protected native boolean nativeSetStrongWhiteEffectIndex(int index);

    // 5
    protected native int nativeGetAdaptiveBlackEffectRange();

    protected native int nativeGetAdaptiveBlackEffectIndex();

    protected native boolean nativeSetAdaptiveBlackEffectIndex(int index);

    // 6
    protected native int nativeGetAdaptiveWhiteEffectRange();

    protected native int nativeGetAdaptiveWhiteEffectIndex();

    protected native boolean nativeSetAdaptiveWhiteEffectIndex(int index);

    // 7
    protected native int nativeGetScenceChangeOnceEnRange();

    protected native int nativeGetScenceChangeOnceEnIndex();

    protected native boolean nativeSetScenceChangeOnceEnIndex(int index);

    // 8
    protected native int nativeGetScenceChangeControlEnRange();

    protected native int nativeGetScenceChangeControlEnIndex();

    protected native boolean nativeSetScenceChangeControlEnIndex(int index);

    // 9
    protected native int nativeGetScenceChangeControlRange();

    protected native int nativeGetScenceChangeControlIndex();

    protected native boolean nativeSetScenceChangeControlIndex(int index);

    // 10
    protected native int nativeGetScenceChangeTh1Range();

    protected native int nativeGetScenceChangeTh1Index();

    protected native boolean nativeSetScenceChangeTh1Index(int index);

    // 11
    protected native int nativeGetScenceChangeTh2Range();

    protected native int nativeGetScenceChangeTh2Index();

    protected native boolean nativeSetScenceChangeTh2Index(int index);

    // 12
    protected native int nativeGetScenceChangeTh3Range();

    protected native int nativeGetScenceChangeTh3Index();

    protected native boolean nativeSetScenceChangeTh3Index(int index);

    // 13
    protected native int nativeGetContentSmooth1Range();

    protected native int nativeGetContentSmooth1Index();

    protected native boolean nativeSetContentSmooth1Index(int index);

    // 14
    protected native int nativeGetContentSmooth2Range();

    protected native int nativeGetContentSmooth2Index();

    protected native boolean nativeSetContentSmooth2Index(int index);

    // 15
    protected native int nativeGetContentSmooth3Range();

    protected native int nativeGetContentSmooth3Index();

    protected native boolean nativeSetContentSmooth3Index(int index);

    // 16
    protected native int nativeGetMiddleRegionGain1Range();

    protected native int nativeGetMiddleRegionGain1Index();

    protected native boolean nativeSetMiddleRegionGain1Index(int index);

    // 17
    protected native int nativeGetMiddleRegionGain2Range();

    protected native int nativeGetMiddleRegionGain2Index();

    protected native boolean nativeSetMiddleRegionGain2Index(int index);

    // 18
    protected native int nativeGetBlackRegionGain1Range();

    protected native int nativeGetBlackRegionGain1Index();

    protected native boolean nativeSetBlackRegionGain1Index(int index);

    // 19
    protected native int nativeGetBlackRegionGain2Range();

    protected native int nativeGetBlackRegionGain2Index();

    protected native boolean nativeSetBlackRegionGain2Index(int index);

    // 20
    protected native int nativeGetBlackRegionRangeRange();

    protected native int nativeGetBlackRegionRangeIndex();

    protected native boolean nativeSetBlackRegionRangeIndex(int index);

    // 21
    protected native int nativeGetBlackEffectLevelRange();

    protected native int nativeGetBlackEffectLevelIndex();

    protected native boolean nativeSetBlackEffectLevelIndex(int index);

    // 22
    protected native int nativeGetBlackEffectParam1Range();

    protected native int nativeGetBlackEffectParam1Index();

    protected native boolean nativeSetBlackEffectParam1Index(int index);

    // 23
    protected native int nativeGetBlackEffectParam2Range();

    protected native int nativeGetBlackEffectParam2Index();

    protected native boolean nativeSetBlackEffectParam2Index(int index);

    // 24
    protected native int nativeGetBlackEffectParam3Range();

    protected native int nativeGetBlackEffectParam3Index();

    protected native boolean nativeSetBlackEffectParam3Index(int index);

    // 25
    protected native int nativeGetBlackEffectParam4Range();

    protected native int nativeGetBlackEffectParam4Index();

    protected native boolean nativeSetBlackEffectParam4Index(int index);

    // 26
    protected native int nativeGetWhiteRegionGain1Range();

    protected native int nativeGetWhiteRegionGain1Index();

    protected native boolean nativeSetWhiteRegionGain1Index(int index);

    // 27
    protected native int nativeGetWhiteRegionGain2Range();

    protected native int nativeGetWhiteRegionGain2Index();

    protected native boolean nativeSetWhiteRegionGain2Index(int index);

    // 28
    protected native int nativeGetWhiteRegionRangeRange();

    protected native int nativeGetWhiteRegionRangeIndex();

    protected native boolean nativeSetWhiteRegionRangeIndex(int index);

    // 29
    protected native int nativeGetWhiteEffectLevelRange();

    protected native int nativeGetWhiteEffectLevelIndex();

    protected native boolean nativeSetWhiteEffectLevelIndex(int index);

    // 30
    protected native int nativeGetWhiteEffectParam1Range();

    protected native int nativeGetWhiteEffectParam1Index();

    protected native boolean nativeSetWhiteEffectParam1Index(int index);

    // 31
    protected native int nativeGetWhiteEffectParam2Range();

    protected native int nativeGetWhiteEffectParam2Index();

    protected native boolean nativeSetWhiteEffectParam2Index(int index);

    // 32
    protected native int nativeGetWhiteEffectParam3Range();

    protected native int nativeGetWhiteEffectParam3Index();

    protected native boolean nativeSetWhiteEffectParam3Index(int index);

    // 33
    protected native int nativeGetWhiteEffectParam4Range();

    protected native int nativeGetWhiteEffectParam4Index();

    protected native boolean nativeSetWhiteEffectParam4Index(int index);

    // 34
    protected native int nativeGetContrastAdjust1Range();

    protected native int nativeGetContrastAdjust1Index();

    protected native boolean nativeSetContrastAdjust1Index(int index);

    // 35
    protected native int nativeGetContrastAdjust2Range();

    protected native int nativeGetContrastAdjust2Index();

    protected native boolean nativeSetContrastAdjust2Index(int index);

    // 36
    protected native int nativeGetDCChangeSpeedLevelRange();

    protected native int nativeGetDCChangeSpeedLevelIndex();

    protected native boolean nativeSetDCChangeSpeedLevelIndex(int index);

    // 37
    protected native int nativeGetProtectRegionEffectRange();

    protected native int nativeGetProtectRegionEffectIndex();

    protected native boolean nativeSetProtectRegionEffectIndex(int index);

    // 38
    protected native int nativeGetDCChangeSpeedLevel2Range();

    protected native int nativeGetDCChangeSpeedLevel2Index();

    protected native boolean nativeSetDCChangeSpeedLevel2Index(int index);

    // 39
    protected native int nativeGetProtectRegionWeightRange();

    protected native int nativeGetProtectRegionWeightIndex();

    protected native boolean nativeSetProtectRegionWeightIndex(int index);

    @Override
    public String getCurrentValue() {
        return getName() + Integer.toString(mCurrentIndex);
    }

    @Override
    public String getMaxValue() {
        return Integer.toString(mRange - 1);
    }

    @Override
    public String getMinValue() {
        return "0";
    }

    @Override
    public String getSeekbarProgressValue() {
        return Integer.toString(mCurrentIndex);
    }

    @Override
    public void init() {
        // TODO Auto-generated method stub
    }

    @Override
    public void setCurrentIndex(int progress) {
        mCurrentIndex = progress;
    }

    @Override
    public int getDefaultIndex() {
        return mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {

    }

    @Override
    public int getCurrentIndex() {
        return mCurrentIndex;
    }

    @Override
    public int getRange() {
        return mRange;
    }

    protected String getName() {
        return mName + ":  ";
    }
}
