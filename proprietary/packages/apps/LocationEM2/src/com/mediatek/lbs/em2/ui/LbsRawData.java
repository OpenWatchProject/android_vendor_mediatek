package com.mediatek.lbs.em2.ui;

import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Locale;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ToggleButton;
import android.widget.TextView;
import android.widget.LinearLayout;

import android.location.GnssMeasurementsEvent;
import android.location.LocationManager;
import android.location.GnssMeasurement;
import android.location.GnssMeasurementCorrections;
import android.location.GnssNavigationMessage;
import android.location.GnssReflectingPlane;
import android.location.GnssSingleSatCorrection;
import android.location.GnssStatus;


import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.Properties;


public class LbsRawData extends Activity {

    private final static int UPDATE_MEA_MESSAGE = 1;

    protected ToggleButton mToggleButtonMeasurement;
    protected ToggleButton mToggleButtonNavigation;
    protected ToggleButton mToggleButtonRegress;
    protected Button mButtonMeasurementCorrection;
    protected TextView mTextViewMessage;
    protected TextView mTextViewLogNavi;
    protected TextView mTextViewLogMeas;
    protected LinearLayout mLayoutDebug;
    private LocationManager mLocationManager = null;
    private int mMeasuNum = 0;
    private int mNaviNum = 0;
    private String mMeasFileName = "";
    private String mNaviFileName = "";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.raw);
        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);

        initUI();
    }

    @Override
    public void onDestroy() {
        mLocationManager.unregisterGnssMeasurementsCallback(mMeasurementCallback);
        super.onDestroy();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    private GnssMeasurementsEvent.Callback mMeasurementCallback =
            new GnssMeasurementsEvent.Callback() {
        @Override
        public void onGnssMeasurementsReceived(GnssMeasurementsEvent eventArgs) {
            mMeasuNum++;

            Collection<GnssMeasurement>  mReadOnlyMeasurements = eventArgs.getMeasurements();

            //output file
            int mSatNum = 0;
            String strout = new String();
            String strUI = new String();
            strout = getTimeString2(new Date().getTime()) +
                    ",getClock: " + eventArgs.getClock() + "\n";
            for (GnssMeasurement measurement : mReadOnlyMeasurements) {
                strout +=  "meas:[" + (mSatNum) +  "] :" + measurement + "\n";
                strUI += " Svid=" + measurement.getConstellationType()+"_"
                      + measurement.getSvid() + " STA=" + measurement.getState()
                      + " CType=" +measurement.getCodeType() + "\n";
                mSatNum++;
            }
            write2File("MEAS", "MEAS_" + mMeasFileName  + ".txt", strout, true, false);

            //logout
            strUI = "Measurement Count=" + mMeasuNum + "\n" +
                    "Satellite Num=" + mSatNum + "\n" + strUI;
            log(strUI);
            sendMessage(UPDATE_MEA_MESSAGE, 0, 0, strUI);
        }

        public void onStatusChanged(int status) {}
    };

    private GnssNavigationMessage.Callback mNavigationCallback =
            new GnssNavigationMessage.Callback() {
        @Override
        public void onGnssNavigationMessageReceived(
                GnssNavigationMessage naviMsg) {
            mNaviNum++;

            //output file
            String strout = new String();
            strout = getTimeString2(new Date().getTime()) + ":" + naviMsg + "\n";
            write2File("NAVI", "NAVI_" + mNaviFileName  + ".txt", strout, true, false);

            String str = new String();
            //logout
            str += "Navigation Count=" + mNaviNum + " Svid=" + naviMsg.getSvid() +
                    " Type=" + naviMsg.getType() + "\n";
            log("" + str);
            //mTextViewLogNavi.setText(str); //can't be accessed by non UI thread

        }

        public void onStatusChanged(int status) {}
    };

    protected void initUI() {
        mToggleButtonMeasurement = (ToggleButton) findViewById(R.id.toggleButton_measurement);
        mToggleButtonNavigation = (ToggleButton) findViewById(R.id.toggleButton_navigation);
        mToggleButtonRegress = (ToggleButton) findViewById(R.id.toggleButton_regress);
        mButtonMeasurementCorrection = (Button) findViewById(R.id.Button_MeasurementCorrection);
        mTextViewLogNavi       = (TextView) findViewById(R.id.TextView_navi);
        mTextViewLogMeas       = (TextView) findViewById(R.id.TextView_meas);
        mLayoutDebug = (LinearLayout) findViewById(R.id.LinearLayout_Debug);

        mLayoutDebug.setVisibility(View.GONE);

        mToggleButtonRegress.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                boolean result = true;
                if (mToggleButtonRegress.isChecked()) {
                    mToggleButtonMeasurement.performClick();
                    mToggleButtonNavigation.performClick();
                    mToggleButtonMeasurement.setEnabled(false);
                    mToggleButtonNavigation.setEnabled(false);
                } else {
                    mToggleButtonMeasurement.performClick();
                    mToggleButtonNavigation.performClick();
                    mToggleButtonMeasurement.setEnabled(true);
                    mToggleButtonNavigation.setEnabled(true);
                }
                String str = "regress Opt:" + mToggleButtonRegress.isChecked();
                log(str + " result:" + result);
            }
        });

        mToggleButtonMeasurement.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                boolean result = true;
                if (mToggleButtonMeasurement.isChecked()) {
                    mMeasuNum = 0;
                    result = mLocationManager
                            .registerGnssMeasurementsCallback(mMeasurementCallback);
                    mMeasFileName = getTimeString2(new Date().getTime());

                } else {
                    mLocationManager.unregisterGnssMeasurementsCallback(mMeasurementCallback);
                }
                String str = "Measurement Opt:" + mToggleButtonMeasurement.isChecked();
                mTextViewLogMeas.setText(str);
                log(str + " result:" + result);
            }
        });

        mToggleButtonNavigation.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                boolean result = true;
                if (mToggleButtonNavigation.isChecked()) {
                    mNaviNum = 0;
                    result = mLocationManager
                            .registerGnssNavigationMessageCallback(mNavigationCallback);
                    mNaviFileName = getTimeString2(new Date().getTime());
                } else {
                    mLocationManager.unregisterGnssNavigationMessageCallback(mNavigationCallback);
                }
                String str = "Navigation Opt:" + mToggleButtonNavigation.isChecked();
                mTextViewLogNavi.setText(str);
                log(str + " result:" + result);
            }
        });
        mButtonMeasurementCorrection.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                GnssReflectingPlane reflectingPlane = new GnssReflectingPlane.Builder()
                        .setLatitudeDegrees(37.4220039)
                        .setLongitudeDegrees(-122.0840991)
                        .setAltitudeMeters(250.35)
                        .setAzimuthDegrees(203.0)
                        .build();

                GnssSingleSatCorrection singleSatCorrection1 = new GnssSingleSatCorrection.Builder()
                        .setConstellationType(GnssStatus.CONSTELLATION_GPS)
                        .setSatelliteId(12)
                        .setCarrierFrequencyHz((float)1.59975e+09)
                        .setProbabilityLineOfSight((float)0.50001)
                        .setExcessPathLengthMeters((float)137.4802)
                        .setExcessPathLengthUncertaintyMeters((float)25.5)
                        .setReflectingPlane(reflectingPlane)
                        .build();

                GnssSingleSatCorrection singleSatCorrection2 = new GnssSingleSatCorrection.Builder()
                        .setConstellationType(GnssStatus.CONSTELLATION_GPS)
                        .setSatelliteId(9)
                        .setCarrierFrequencyHz((float)1.59975e+09)
                        .setProbabilityLineOfSight((float)0.873)
                        .setExcessPathLengthMeters((float)26.294)
                        .setExcessPathLengthUncertaintyMeters((float)10.0)
                        .build();

                ArrayList<GnssSingleSatCorrection> singleSatCorrections =
                        new ArrayList<GnssSingleSatCorrection>(2);
                singleSatCorrections.add(singleSatCorrection1);
                singleSatCorrections.add(singleSatCorrection2);

                GnssMeasurementCorrections meaCorrections = new GnssMeasurementCorrections.Builder()
                        .setLatitudeDegrees((float)37.4219999)
                        .setLongitudeDegrees((float)-122.0840575)
                        .setAltitudeMeters((float)30.60062531)
                        .setHorizontalPositionUncertaintyMeters((float)9.23542)
                        .setVerticalPositionUncertaintyMeters((float)15.02341)
                        .setToaGpsNanosecondsOfWeek(2935633453L)
                        .setSingleSatelliteCorrectionList(singleSatCorrections)
                        .build();
                mLocationManager.injectGnssMeasurementCorrections(meaCorrections);
            }
        });
    }

    private String getTimeString2(long milliseconds) {
        Date date = new Date(milliseconds);
        String str = String.format("%04d%02d%02d_%02d%02d%02d",
                (date.getYear() + 1900),
                (date.getMonth() + 1),
                date.getDate(),
                date.getHours(),
                date.getMinutes(),
                date.getSeconds());
        return str;
    }

    //true: success
    public boolean write2File(String folder, String fileName,
            String data, boolean isAppendMode, boolean toSdcard) {
        DataOutputStream dos;
        String fullFileName;
        if (toSdcard) {
            fullFileName = "/sdcard/" + folder;
        } else {
            fullFileName = "/data/data/" + this.getPackageName() + "/" + folder;
        }
        File f = new File(fullFileName);
        f.mkdir();
        fullFileName += "/" + fileName;
        f = new File(fullFileName);
        try {
            if (isAppendMode) {
                dos = new DataOutputStream(new FileOutputStream(fullFileName, true));
            } else {
                dos = new DataOutputStream(new FileOutputStream(f));
            }
            dos.writeBytes(data);
            dos.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    private static final String PROPERTIES_FILE = "/data/misc/gps.conf";

    private static String getGPSConfig(String key) {
        String result = null;
        Properties mProperties = new Properties();
        try {
            File file = new File(PROPERTIES_FILE);
            FileInputStream stream = new FileInputStream(file);
            mProperties.load(stream);
            stream.close();

            result = mProperties.getProperty(key);
            log("getConfig key:" + key + " val:" + result);
        } catch (IOException e) {
            log("getConfig fail key:" + key + " ex:" + e);
        }

        return result;
    }

    private void sendMessage(int what, int arg1) {
        sendMessage(what, arg1, 0, null);
    }

    private void sendDelayedMessage(int what, long delayMillis) {
        mHandler.sendEmptyMessageDelayed(what, delayMillis);
    }

    private void sendMessage(int what, int arg1, int arg2, Object obj) {
        mHandler.obtainMessage(what, arg1, arg2, obj).sendToTarget();
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch(msg.what) {
            case UPDATE_MEA_MESSAGE:
                if (msg.obj != null) {
                    String str = (String)msg.obj;
                    mTextViewLogMeas.setText(str);
                }
                break;
            }
        }
    };

    protected static void log(Object msg) {
        Log.d("LocationEM", "" + msg);
    }

    protected static void logw(Object msg) {
        Log.d("LocationEM", "[agps] WARNING: " + msg);
    }
}
