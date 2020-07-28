package com.mediatek.gnssdebugreport;

import android.os.Parcel;
import android.os.Parcelable;

public class DebugDataReport implements Parcelable {
    public static final String DATA_KEY = "DebugDataReport";
    public static final String DATA_KEY_TYPE1 = "data_type1";
    public static final String JSON_TYPE = "type";
    private double mCB;
    private double mCompCB;
    private double mClkTemp;
    private int mSaturation;
    private int mPga;
    private long mTtff;
    private int mSvnum;
    private long mTT4SV;
    private float mTop4CNR;
    private double mInitLlhLongi;
    private double mInitLlhLati;
    private double mInitLlhHeight;
    private int mInitSrc;
    private float mInitPacc;
    private int mHaveEPO;
    private int mEPOage;
    private float mSensorHACC;
    private int mMPEvalid;
    private int mLsvalid;

    public DebugDataReport(double CB, double CompCB, double ClkTemp, int Saturation, int Pga,
            long Ttff, int Svnum, long TT4SV, float Top4CNR, double InitLlhLongi,
            double InitLlhLati, double InitLlhHeight, int InitSrc, float InitPacc, int HaveEPO,
            int EPOage, float SensorHACC, int MPEvalid, int Lsvalid) {
        // TODO Auto-generated constructor stub
        mCB = CB;
        mCompCB = CompCB;
        mClkTemp = ClkTemp;
        mSaturation = Saturation;
        mPga = Pga;
        mTtff = Ttff;
        mSvnum = Svnum;
        mTT4SV = TT4SV;
        mTop4CNR = Top4CNR;
        mInitLlhLongi = InitLlhLongi;
        mInitLlhLati = InitLlhLati;
        mInitLlhHeight = InitLlhHeight;
        mInitSrc = InitSrc;
        mInitPacc = InitPacc;
        mHaveEPO = HaveEPO;
        mEPOage = EPOage;
        mSensorHACC = SensorHACC;
        mMPEvalid = MPEvalid;
        mLsvalid = Lsvalid;
    };

    public DebugDataReport(Parcel source){
        mCB = source.readDouble();
        mCompCB = source.readDouble();
        mClkTemp = source.readDouble();
        mSaturation = source.readInt();
        mPga = source.readInt();
        mTtff = source.readLong();
        mSvnum = source.readInt();
        mTT4SV = source.readLong();
        mTop4CNR = source.readFloat();
        mInitLlhLongi = source.readDouble();
        mInitLlhLati = source.readDouble();
        mInitLlhHeight = source.readDouble();
        mInitSrc = source.readInt();
        mInitPacc = source.readFloat();
        mHaveEPO = source.readInt();
        mEPOage = source.readInt();
        mSensorHACC = source.readFloat();
        mMPEvalid = source.readInt();
        mLsvalid = source.readInt();
    }

    public double getCB() {
        return mCB;
    }

    public double getmCompCB() {
        return mCompCB;
    }

    public double getClkTemp() {
        return mClkTemp;
    }

    public int getSaturation() {
        return mSaturation;
    }

    public int getPga() {
        return mPga;
    }

    public long getTtff() {
        return mTtff;
    }

    public int getSvnum() {
        return mSvnum;
    }

    public long getTT4SV() {
        return mTT4SV;
    }

    public float getTop4CNR() {
        return mTop4CNR;
    }

    public double getInitLlhLongi() {
        return mInitLlhLongi;
    }

    public double getInitLlhLati() {
        return mInitLlhLati;
    }

    public double getInitLlhHeight() {
        return mInitLlhHeight;
    }

    public int getInitSrc() {
        return mInitSrc;
    }

    public float getInitPacc() {
        return mInitPacc;
    }

    public int getHaveEPO() {
        return mHaveEPO;
    }

    public int getEPOage() {
        return mEPOage;
    }

    public float getSensorHACC() {
        return mSensorHACC;
    }

    public int getMPEvalid() {
        return mMPEvalid;
    }

    public int getLsvalid() {
        return mLsvalid;
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        // TODO Auto-generated method stub
        dest.writeDouble(mCB);
        dest.writeDouble(mCompCB);
        dest.writeDouble(mClkTemp);
        dest.writeInt(mSaturation);
        dest.writeInt(mPga);
        dest.writeLong(mTtff);
        dest.writeInt(mSvnum);
        dest.writeLong(mTT4SV);
        dest.writeFloat(mTop4CNR);
        dest.writeDouble(mInitLlhLongi);
        dest.writeDouble(mInitLlhLati);
        dest.writeDouble(mInitLlhHeight);
        dest.writeInt(mInitSrc);
        dest.writeFloat(mInitPacc);
        dest.writeInt(mHaveEPO);
        dest.writeInt(mEPOage);
        dest.writeFloat(mSensorHACC);
        dest.writeInt(mMPEvalid);
        dest.writeInt(mLsvalid);
    }

    public static final Parcelable.Creator<DebugDataReport> CREATOR = new Parcelable.Creator<DebugDataReport>() {
        public DebugDataReport createFromParcel(Parcel in) {
            return new DebugDataReport(in);
        }

        public DebugDataReport[] newArray(int size) {
            return new DebugDataReport[size];
        }
    };

    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("[").append(mCB).append(", ");
        builder.append(mCompCB).append(", ");
        builder.append(mClkTemp).append(", ");
        builder.append(mSaturation).append(", ");
        builder.append(mPga).append(", ");
        builder.append(mTtff).append(", ");
        builder.append(mSvnum).append(", ");
        builder.append(mTT4SV).append(", ");
        builder.append(mTop4CNR).append(", ");
        builder.append(mInitLlhLongi).append(", ");
        builder.append(mInitLlhLati).append(", ");
        builder.append(mInitLlhHeight).append(", ");
        builder.append(mInitSrc).append(", ");
        builder.append(mInitPacc).append(", ");
        builder.append(mHaveEPO).append(", ");
        builder.append(mEPOage).append(", ");
        builder.append(mSensorHACC).append(", ");
        builder.append(mMPEvalid).append(", ");
        builder.append(mLsvalid).append("]");
        return builder.toString();
    }

    public static class DebugData840 {
        public static final String KEY_VER = "ver";
        public static final String KEY_SUPL_INJECT = "supl_inject";
        public static final String KEY_EPO = "epo";
        public static final String KEY_EPO_AGE = "epo_age";
        public static final String KEY_QEPO = "qepo";
        public static final String KEY_NLP = "nlp";
        public static final String KEY_AID_LAT = "aiding_lat";
        public static final String KEY_AID_LON = "aiding_lon";
        public static final String KEY_AID_HEIGHT = "aiding_height";
        public static final String KEY_NV = "nv";
        public static final String KEY_AID_SUMMARY = "aiding_summary";
    }

    public static class DebugData841 {
        public static final String KEY_VER = "ver";
        public static final String KEY_CLKD = "clk_d";
        public static final String KEY_XO_TEMPER = "xo_temper";
        public static final String KEY_PGA_GAIN = "pga_gain";
        public static final String KEY_NOISE_FLOOR = "noise_floor";
        public static final String KEY_DIGI_I = "digi_i";
        public static final String KEY_DIGI_Q = "digi_q";
        public static final String KEY_SENSOR = "sensor";
        public static final String KEY_CHIP_SUMMARY = "chip_summary";
        public static final String KEY_BLANKING = "blanking";
    }

}
