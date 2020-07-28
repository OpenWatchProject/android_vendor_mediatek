package com.mediatek.apmonitor.kpi;


public class LocationMessage extends KpiObjBaseRoot {
    private static final int VERSION = 1;

    public static final int NETWORK_PROVIDER = 1;
    public static final int GPS_PROVIDER = 2;

    public static final int HAS_ALTITUDE_MASK = 1;
    public static final int HAS_SPEED_MASK = 2;
    public static final int HAS_BEARING_MASK = 4;
    public static final int HAS_HORIZONTAL_ACCURACY_MASK = 8;
    public static final int HAS_MOCK_PROVIDER_MASK = 16;
    public static final int HAS_VERTICAL_ACCURACY_MASK = 32;
    public static final int HAS_SPEED_ACCURACY_MASK = 64;
    public static final int HAS_BEARING_ACCURACY_MASK = 128;

    public Integer mProvider;
    public Long mTime;
    public Double mLatitude;
    public Double mLongitude;
    public Double mAltitude;
    public Float mSpeed;
    public Float mBearing;
    public Float mHorizontalAccuracyMeters;
    public Float mVerticalAccuracyMeters;
    public Float mSpeedAccuracyMetersPerSecond;
    public Float mBearingAccuracyDegrees;
    public Integer mFieldsMask;

    public LocationMessage() {
        super(VERSION);
    }

    @Override
    public void serialize() {
        writeInteger(mProvider);
        writeLong(mTime);
        writeDouble(mLatitude);
        writeDouble(mLongitude);
        writeDouble(mAltitude);
        writeFloat(mSpeed);
        writeFloat(mBearing);
        writeFloat(mHorizontalAccuracyMeters);
        writeFloat(mVerticalAccuracyMeters);
        writeFloat(mSpeedAccuracyMetersPerSecond);
        writeFloat(mBearingAccuracyDegrees);
        writeInteger(mFieldsMask);
    }

    // No need to implement this currently.
    @Override
    public void deserialize() {
        // Not implemented
    }
}
