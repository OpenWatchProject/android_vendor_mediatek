package com.mediatek.engineermode.npt;

public class BandItem {
    private String mBandName;
    private int mRatValue;
    private int mBandValue = -1;
    private int mChannleValue;
    private int[] mChannleScope = {-1, -1, -1};
    private int mRxBwValue;
    private int mTxBwValue;
    private int mTxOnflagValueRef;
    private int mTxOnflagValueNpt;
    private int mTxPowerValue;
    private int mAntennaStateValue;
    private int mRepeatTimesValue;
    private int mRbStartValue;
    private int mRblengthValue;
    private BandType mBandType = BandType.BAND_NULL;
    private int mUPlinkMinFreqValue;
    private int mDownlinkMinFreqValue;

    public int getmDownlinkMinChannelValue() {
        return mDownlinkMinChannelValue;
    }

    public void setmDownlinkMinChannelValue(int mDownlinkMinChannelValue) {
        this.mDownlinkMinChannelValue = mDownlinkMinChannelValue;
    }

    public int getmDownlinkMaxChannelValue() {
        return mDownlinkMaxChannelValue;
    }

    public void setmDownlinkMaxChannelValue(int mDownlinkMaxChannelValue) {
        this.mDownlinkMaxChannelValue = mDownlinkMaxChannelValue;
    }

    private int mDownlinkMinChannelValue;
    private int mDownlinkMaxChannelValue;


    private boolean mSelected = false;

    public BandItem(String bandName, int band_value, int channel_start, int channel_step,
                    int channel_end, int power, int tx_flag_ref,int tx_flag_npt, int repeat_time, 
                    BandType bandtype,
                    int rx_bw, int tx_bw, int rb_start, int rb_length,
                    int uplink_min_freq, int downlink_min_freq,int ant_status) {
        mBandName = bandName;
        mBandValue = band_value;
        mChannleScope[0] = channel_start;
        mChannleScope[1] = channel_step;
        mChannleScope[2] = channel_end;
        mDownlinkMinChannelValue = channel_start;
        mDownlinkMaxChannelValue = channel_end;
        mTxPowerValue = power;
        mTxOnflagValueRef = tx_flag_ref;
        mTxOnflagValueNpt = tx_flag_npt;
        mRepeatTimesValue = repeat_time;
        mBandType = bandtype;

        mRxBwValue = rx_bw;
        mTxBwValue = tx_bw;
        mRbStartValue = rb_start;
        mRblengthValue = rb_length;

        mUPlinkMinFreqValue = uplink_min_freq;
        mDownlinkMinFreqValue = downlink_min_freq;
        mAntennaStateValue = ant_status;
        if (BandType.BAND_GSM == mBandType) {
            mRatValue = 1;
        } else if ((BandType.BAND_WCDMA == mBandType) ||
                (BandType.BAND_TD == mBandType)) {
            mRatValue = 2;
        } else if (BandType.BAND_LTE == mBandType) {
            mRatValue = 3;
        } else if (BandType.BAND_CDMA == mBandType) {
            mRatValue = 4;
        } else if (BandType.BAND_EVDO == mBandType) {
            mRatValue = 5;
        } else {
            mRatValue = -1;
        }

        mSelected = true;
    }

    public BandItem(String bandName, int band_value, int channel_start, int channel_step, int
            channel_end, int power, int tx_flag_ref,int tx_flag_npt, int repeat_time, BandType 
            bandtype, int uplink_min_freq, int downlink_min_freq,int ant_status) {

        this(bandName, band_value, channel_start, channel_step, channel_end, power, tx_flag_ref,
                tx_flag_npt, repeat_time, bandtype, 0, 0, 0, 0, uplink_min_freq,
                downlink_min_freq,ant_status);
    }

    public int getmUPlinkMinFreqValue() {
        return mUPlinkMinFreqValue;
    }

    public void setmUPlinkMinFreqValue(int mUPlinkMinFreqValue) {
        this.mUPlinkMinFreqValue = mUPlinkMinFreqValue;
    }

    public int getmDownlinkMinFreqValue() {
        return mDownlinkMinFreqValue;
    }

    public void setmDownlinkMinFreqValue(int mDownlinkMinFreqValue) {
        this.mDownlinkMinFreqValue = mDownlinkMinFreqValue;
    }
    public int getmAntennaStateValue() {
        return mAntennaStateValue;
    }

    public void setmAntennaStateValue(int mAntennaStateValue) {
        this.mAntennaStateValue = mAntennaStateValue;
    }

    public String getmBandName() {
        return mBandName;
    }

    public void setmBandName(String mBandName) {
        this.mBandName = mBandName;
    }

    public int getmRatValue() {
        return mRatValue;
    }

    public void setmRatValue(int mRatValue) {
        this.mRatValue = mRatValue;
    }

    public int getmBandValue() {
        return mBandValue;
    }

    public void setmBandValue(int mBandValue) {
        this.mBandValue = mBandValue;
    }

    public int getmChannleValue() {
        return mChannleValue;
    }

    public void setmChannleValue(int mChannleValue) {
        this.mChannleValue = mChannleValue;
    }

    public int[] getmChannleScope() {
        return mChannleScope;
    }

    public void setmChannleScope(int[] channleScope) {
        mChannleScope[0] = channleScope[0];
        mChannleScope[1] = channleScope[1];
        mChannleScope[2] = channleScope[2];
    }

    public int getmRxBwValue() {
        return mRxBwValue;
    }

    public void setmRxBwValue(int mRxBwValue) {
        this.mRxBwValue = mRxBwValue;
    }

    public int getmTxBwValue() {
        return mTxBwValue;
    }

    public void setmTxBwValue(int mTxBwValue) {
        this.mTxBwValue = mTxBwValue;
    }

    public int getmTxOnflagValueRef() {
        return mTxOnflagValueRef;
    }

    public void setmTxOnflagValueRef(int mTxOnflagValueRef) {
        this.mTxOnflagValueRef = mTxOnflagValueRef;
    }

    public int getmTxOnflagValueNpt() {
        return mTxOnflagValueNpt;
    }

    public void setmTxOnflagValueNpt(int mTxOnflagValueNpt) {
        this.mTxOnflagValueNpt = mTxOnflagValueNpt;
    }

    public int getmTxPowerValue() {
        return mTxPowerValue;
    }

    public void setmTxPowerValue(int mTxPowerValue) {
        this.mTxPowerValue = mTxPowerValue;
    }

    public int getmRepeatTimesValue() {
        return mRepeatTimesValue;
    }

    public void setmRepeatTimesValue(int mRepeatTimesValue) {
        this.mRepeatTimesValue = mRepeatTimesValue;
    }

    public int getmRbStartValue() {
        return mRbStartValue;
    }

    public void setmRbStartValue(int mRbStartValue) {
        this.mRbStartValue = mRbStartValue;
    }

    public int getmRblengthValue() {
        return mRblengthValue;
    }

    public void setmRblengthValue(int mRblengthValue) {
        this.mRblengthValue = mRblengthValue;
    }

    public BandType getmBandType() {
        return mBandType;
    }

    public void setmBandType(BandType mBandType) {
        this.mBandType = mBandType;
    }

    public boolean ismSelected() {
        return mSelected;
    }

    public void setmSelected(boolean mSelected) {
        this.mSelected = mSelected;
    }


    public String getSummary1() {
        StringBuilder summary = new StringBuilder();
        summary.append("Channel Start: " + this.getmChannleScope()[0]);
        summary.append("  Step: " + this.getmChannleScope()[1]);
        summary.append("  End: " + this.getmChannleScope()[2]);
        return summary.toString();
    }
    public String getSummary2() {
        StringBuilder summary = new StringBuilder();
        summary.append("Tx flag ref: " + this.getmTxOnflagValueRef());
        summary.append("  Tx flag npt: " + this.getmTxOnflagValueNpt());
        return summary.toString();
    }
    public String getSummary3() {
        StringBuilder summary = new StringBuilder();
        summary.append("Tx Power: " + this.getmTxPowerValue());
        summary.append("  Repeat times: " + this.getmRepeatTimesValue());
        summary.append("  Ant : " + this.getmAntennaStateValue());
        return summary.toString();
    }

    public boolean isSelected() {
        return mSelected;
    }

    public void setSelected(boolean selected) {
        mSelected = selected;
    }

    @Override
    public String toString() {
        // TODO Auto-generated method stub
        return "mBandName:" + mBandName + " Summary: " + getSummary1() + getSummary2()
                +getSummary3();
    }

    /**
     * Telephony band type.
     */
    enum BandType {
        BAND_GSM,
        BAND_WCDMA,
        BAND_TD,
        BAND_LTE,
        BAND_CDMA,
        BAND_EVDO,
        BAND_NULL,
    }

}
