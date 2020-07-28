package com.mediatek.engineermode.desenseat;

import com.mediatek.engineermode.ModemCategory;

import java.util.HashMap;
/**
 * Band item information.
 *
 */
/**
 * @author mtk80357
 *
 */
public class BandItem {
    private String mBandName;
    private int mPower = -1;
    private int[] mChannleList = {-1 , -1 , -1};
    private int mBandValue = -1;
    private static HashMap<String, Integer[]> sDefaultData;
    private boolean mSelected = false;
     private boolean mIsULFreq = false;
    private BandType mBandType = BandType.BAND_NULL;

    /**
     * Telephony band type.
     *
     */
    enum BandType {
        BAND_NULL,
        BAND_GSM,
        BAND_WCDMA,
        BAND_TD,
        BAND_LTE_FDD,
        BAND_LTE_TDD,
        BAND_CDMA
    };

    /**
     * Constructor function.
     * @param bandName name for user to understand.
     * @param bandvalue value for real use with modem.
     * @param bandtype telephony band type.
     */
    public BandItem(String bandName, int bandvalue, BandType bandtype) {
        // TODO Auto-generated constructor stub
        mBandName = bandName;
        mBandValue = bandvalue;
        mBandType = bandtype;
        addDefaultValue(mBandName);
    }

    public BandItem(String bandName, int bandvalue, BandType bandtype,int channel,int power) {
        // TODO Auto-generated constructor stub
        mBandName = bandName;
        mBandValue = bandvalue;
        mBandType = bandtype;
        mChannleList[0] = channel;
        mPower = power;
        mSelected = true;

    }

    public BandItem(String bandName, int bandvalue, BandType bandtype,int channel,int power,
                    boolean isULFreq) {
        // TODO Auto-generated constructor stub
        this(bandName, bandvalue, bandtype,channel,power);
        mIsULFreq = isULFreq;
    }

    /**
     * Constructor function.
     * @param bandName name for user to understand
     * @param bandvalue value for real use with modem
     * @param bandtype telephony band type
     * @param isULFreq special param for Lte
     */
    public BandItem(String bandName, int bandvalue, BandType bandtype, boolean isULFreq) {
        // TODO Auto-generated constructor stub
        this(bandName, bandvalue, bandtype);
        mIsULFreq = isULFreq;
    }

    public boolean isUlFreq() {
        return mIsULFreq;
    }
    public BandType getType() {
        return mBandType;
    }
    public String getBandName() {
        return mBandName;
    }

    public void setSelected(boolean selected) {
        mSelected = selected;
    }

    public int getPower() {
        return mPower;
    }

    public void setPower(int power) {
        mPower = power;
    }

    public int getBandValue() {
        return mBandValue;
    }

    public void setBandValue(int bandValue) {
        mBandValue = bandValue;
    }

    /**
     * @param ch1 channel 1
     * @param ch2 channel 2
     * @param ch3 channel 3
     */
    public void setChannel(int ch1, int ch2, int ch3) {
        mChannleList[0] = ch1;
        mChannleList[1] = ch2;
        mChannleList[2] = ch3;
    }

    public int[] getChannel() {
        return mChannleList;
    }

    /**
     * @return get information summary to show
     */
    public String getSummary() {
        StringBuilder summary = new StringBuilder(mIsULFreq ? "ULFreq: " : "Channel: ");
        boolean notSet = true;
        for (int k = 0 ; k < 3 ; k++) {
            if (mChannleList[k] != -1) {
                summary.append(mChannleList[k]).append("   ");
                notSet = false;
            }
        }
        if (notSet) {
            summary.append("NA    ");
        }

        summary.append((mPower == -1) ? "PWR:NA" : ("PWR: " + mPower));

        return summary.toString();
    }

    public boolean isSelected() {
        return mSelected;
    }

    static void initDefaultData() {
        sDefaultData = new HashMap<String, Integer[]>();
        sDefaultData.put("GSM 850", new Integer[]{189, -1, -1, 5, 128});
        sDefaultData.put("DCS 1800", new Integer[]{698, -1, -1, 0, 8});
        if (ModemCategory.getModemType() == ModemCategory.MODEM_FDD) {
            sDefaultData.put("WCDMA Band 2", new Integer[]{9400, -1, -1, 24, 2});
            sDefaultData.put("WCDMA Band 5", new Integer[]{4182, -1, -1, 24, 5});
        } else if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
            sDefaultData.put("Band A", new Integer[]{5587, -1, -1, 24, 1});
            sDefaultData.put("Band F", new Integer[]{9500, -1, -1, 24, 6});
        }
        if (ModemCategory.isLteSupport()) {
            sDefaultData.put("LTE-FDD Band 3", new Integer[]{17475, -1, -1, 23, 2});
            sDefaultData.put("LTE-FDD Band 5", new Integer[]{8365, -1, -1, 23, 4});
            sDefaultData.put("LTE-FDD Band 17", new Integer[]{7100, -1, -1, 23, 16});
            sDefaultData.put("LTE-FDD Band 20", new Integer[]{8470, -1, -1, 23, 19});
            sDefaultData.put("LTE-TDD Band 38", new Integer[]{25950, -1, -1, 23, 37});
            sDefaultData.put("LTE-TDD Band 39", new Integer[]{19000, -1, -1, 23, 38});
            sDefaultData.put("LTE-TDD Band 40", new Integer[]{23500, -1, -1, 23, 39});
            sDefaultData.put("LTE-TDD Band 41", new Integer[]{25930, -1, -1, 23, 40});
        }
    }

    private void addDefaultValue(String bandName) {
        Integer[] data = sDefaultData.get(bandName);
        if (data != null) {
            mChannleList[0] = data[0];
            mChannleList[1] = data[1];
            mChannleList[2] = data[2];
            mPower = data[3];
            mBandValue = data[4];
            mSelected = true;
        }
    }

    @Override
    public String toString() {
        // TODO Auto-generated method stub
        return "mBandName:" + mBandName + " Summary: " + getSummary();
    }

}
