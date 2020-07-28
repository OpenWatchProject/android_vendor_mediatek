package com.mediatek.camera.tests.v3.observer;

import android.content.Context;
import android.database.Cursor;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Observer;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

public class MediaLocationObserver extends Observer implements LocationListener {
    public static final int INDEX_PHOTO = 0;
    public static final int INDEX_VIDEO = 1;
    public static final int INDEX_PHOTO_OR_VIDEO = 2;

    private static final LogUtil.Tag TAG = Utils.getTestTag(MediaLocationObserver.class
            .getSimpleName());

    private static final String[] PROJECTION_IMAGE = new String[]{
            MediaStore.Images.ImageColumns._ID, MediaStore.Images.ImageColumns.LATITUDE,
            MediaStore.Images.ImageColumns.LONGITUDE
    };
    private static final String[] PROJECTION_VIDEO = new String[]{
            MediaStore.Video.VideoColumns._ID, MediaStore.Video.VideoColumns.LATITUDE,
            MediaStore.Video.VideoColumns.LONGITUDE
    };
    private static final int INDEX_LAT = 1;
    private static final int INDEX_LON = 2;
    private static final String WHERE_CLAUSE_IMAGE = MediaStore.Images.ImageColumns.DATA +
            " = ?";
    private static final String WHERE_CLAUSE_VIDEO = MediaStore.Video.VideoColumns.DATA +
            " = ?";

    private static final int MSG_START_RECORD_LOCATION = 0;
    private static final int MSG_STOP_RECORD_LOCATION = 1;

    private static HandlerThread mRecordLocationThread;
    private static Handler mHandler;
    private LocationManager mLocationManager;
    private List<Location> mLocationGPS = new ArrayList<>();
    private List<Location> mLocationNetwork = new ArrayList<>();

    @Override
    public int getObserveCount() {
        return 3;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_PHOTO:
                return "Observe location info of captured photo in media database is same as test" +
                        " case get";
            case INDEX_VIDEO:
                return "Observe location info of captured video in media database is same as test" +
                        " case get";
            case INDEX_PHOTO_OR_VIDEO:
                return "Observe location info of captured photo/video in media database is same " +
                        "as test case get";
            default:
                return null;
        }
    }

    @Override
    protected void doBeginObserve(int index) {
        mLocationGPS.clear();
        mLocationNetwork.clear();
        if (mRecordLocationThread == null) {
            mRecordLocationThread = new HandlerThread("CamAp_AT_RecordLocationThread");
            mRecordLocationThread.start();
        }
        if (mHandler == null) {
            mHandler = new LocationHandler(mRecordLocationThread.getLooper());
        }
        mHandler.sendEmptyMessage(MSG_START_RECORD_LOCATION);
        Utils.waitSafely(10000);
    }

    @Override
    protected void doEndObserve(int index) {
        mHandler.sendEmptyMessage(MSG_STOP_RECORD_LOCATION);

        Cursor cursorImage = null;
        Cursor cursorVideo = null;
        switch (index) {
            case INDEX_PHOTO:
                if (TestContext.mLatestPhotoPath != null) {
                    LogHelper.d(TAG, "[doEndObserve] TestContext.mLatestPhotoPath = "
                            + TestContext.mLatestPhotoPath);
                    cursorImage = Utils.getTargetContext().getContentResolver().query(
                            MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                            PROJECTION_IMAGE,
                            WHERE_CLAUSE_IMAGE,
                            new String[]{String.valueOf(TestContext.mLatestPhotoPath)},
                            null);
                    Utils.assertRightNow(cursorImage != null
                            && cursorImage.moveToFirst()
                            && cursorImage.getCount() == 1);
                } else {
                    LogHelper.d(TAG, "[doEndObserve] TestContext.mLatestPhotoPath is null, return");
                    return;
                }
                break;
            case INDEX_VIDEO:
                if (TestContext.mLatestVideoPath != null) {
                    LogHelper.d(TAG, "[doEndObserve] TestContext.mLatestVideoPath = "
                            + TestContext.mLatestVideoPath);
                    cursorVideo = Utils.getTargetContext().getContentResolver().query(
                            MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                            PROJECTION_VIDEO,
                            WHERE_CLAUSE_VIDEO,
                            new String[]{String.valueOf(TestContext.mLatestVideoPath)},
                            null);
                    Utils.assertRightNow(cursorVideo != null
                            && cursorVideo.moveToFirst()
                            && cursorVideo.getCount() == 1);
                } else {
                    LogHelper.d(TAG, "[doEndObserve] TestContext.mLatestVideoPath is null, return");
                    return;
                }
                break;
            case INDEX_PHOTO_OR_VIDEO:
                if (TestContext.mLatestPhotoPath != null) {
                    LogHelper.d(TAG, "[doEndObserve] TestContext.mLatestPhotoPath = "
                            + TestContext.mLatestPhotoPath);
                    cursorImage = Utils.getTargetContext().getContentResolver().query(
                            MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                            PROJECTION_IMAGE,
                            WHERE_CLAUSE_IMAGE,
                            new String[]{String.valueOf(TestContext.mLatestPhotoPath)},
                            null);
                    Utils.assertRightNow(cursorImage != null
                            && cursorImage.moveToFirst()
                            && cursorImage.getCount() == 1);
                }

                if (TestContext.mLatestVideoPath != null) {
                    LogHelper.d(TAG, "[doEndObserve] TestContext.mLatestVideoPath = "
                            + TestContext.mLatestVideoPath);
                    cursorVideo = Utils.getTargetContext().getContentResolver().query(
                            MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                            PROJECTION_VIDEO,
                            WHERE_CLAUSE_VIDEO,
                            new String[]{String.valueOf(TestContext.mLatestVideoPath)},
                            null);
                    Utils.assertRightNow(cursorVideo != null
                            && cursorVideo.moveToFirst()
                            && cursorVideo.getCount() == 1);
                }
        }

        if (cursorImage != null) {
            double lat = cursorImage.getDouble(INDEX_LAT);
            double lon = cursorImage.getDouble(INDEX_LON);
            cursorImage.close();
            LogHelper.d(TAG, "[doEndObserve] read from image media db, latitude = " + lat
                    + ", longitude = " + lon);

            checkLocation(lat, lon);
        }

        if (cursorVideo != null) {
            double lat = cursorVideo.getDouble(INDEX_LAT);
            double lon = cursorVideo.getDouble(INDEX_LON);
            cursorVideo.close();
            LogHelper.d(TAG, "[doEndObserve] read from video media db, latitude = " + lat
                    + ", longitude = " + lon);

            checkLocation(lat, lon);
        }


    }

    private void startRecordLocation() {
        mLocationManager = (android.location.LocationManager)
                Utils.getTargetContext().getSystemService(Context.LOCATION_SERVICE);
        List<String> locationProviders = mLocationManager.getAllProviders();

        for (String provider : locationProviders) {
            try {
                LogHelper.d(TAG, "[startRecordLocation] requestLocationUpdates " + provider);
                mLocationManager.requestLocationUpdates(
                        provider, Utils.TIME_OUT_SHORT_SHORT, 0F, this);
            } catch (SecurityException e) {
                LogHelper.d(TAG, "[startRecordLocation] SecurityException");
            }
        }
    }

    private void stopRecordLocation() {
        try {
            mLocationManager.removeUpdates(this);
        } catch (SecurityException e) {
            LogHelper.d(TAG, "[stopRecordLocation] SecurityException");
        }
    }

    private void checkLocation(double lat, double lon) {
        for (Location location : mLocationGPS) {
            if (location.getLatitude() == lat && location.getLongitude() == lon) {
                LogHelper.d(TAG, "[checkLocation] find mapping in GPS provider");
                return;
            }
        }
        for (Location location : mLocationNetwork) {
            if (location.getLatitude() == lat && location.getLongitude() == lon) {
                LogHelper.d(TAG, "[checkLocation] find mapping in NETWORK provider");
                return;
            }
        }

        if (mLocationGPS.size() == 0 && mLocationNetwork.size() == 0) {
            Utils.assertRightNow(lat == 0.0 && lon == 0.0, "(0.0, 0.0) excepted, but find (" +
                    lat + ", " + lon + ")");
            LogHelper.d(TAG, "[checkLocation] (0.0, 0.0) mapping no location");
        } else {
            Utils.assertRightNow(false, "(" + lat + ", " + lon + ") not mapping with any location");
        }

    }

    @Override
    public void onLocationChanged(Location location) {
        LogHelper.d(TAG, "[onLocationChanged] " + location.getProvider()
                + ", lat = " + location.getLatitude()
                + ", lon = " + location.getLongitude());

        if (location != null && location.getProvider().equals(LocationManager.GPS_PROVIDER)) {
            mLocationGPS.add(location);
        } else if (location != null
                && location.getProvider().equals(LocationManager.NETWORK_PROVIDER)) {
            mLocationNetwork.add(location);
        }
    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
        LogHelper.d(TAG, "[onStatusChanged] " + provider);
        switch (status) {
            case android.location.LocationProvider.OUT_OF_SERVICE:
            case android.location.LocationProvider.TEMPORARILY_UNAVAILABLE: {
                if (provider.equals(LocationManager.GPS_PROVIDER)) {
                    LogHelper.d(TAG, "[onStatusChanged] clear location from GPS_PROVIDER");
                    mLocationGPS.clear();
                } else if (provider.equals(LocationManager.NETWORK_PROVIDER)) {
                    LogHelper.d(TAG, "[onStatusChanged] clear location from NETWORK_PROVIDER");
                    mLocationNetwork.clear();
                }
                break;
            }
        }
    }

    @Override
    public void onProviderEnabled(String provider) {
        LogHelper.d(TAG, "[onProviderEnabled] " + provider);
    }

    @Override
    public void onProviderDisabled(String provider) {
        LogHelper.d(TAG, "[onProviderDisabled] " + provider);
    }

    class LocationHandler extends Handler {

        public LocationHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_START_RECORD_LOCATION:
                    startRecordLocation();
                    break;
                case MSG_STOP_RECORD_LOCATION:
                    stopRecordLocation();
                    break;
            }
        }
    }
}
