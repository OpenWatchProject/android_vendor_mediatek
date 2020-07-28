package com.mediatek.lbs.em2.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.location.GnssStatus;
import android.location.GpsStatus;
import android.location.LocationManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.RadioButton;

import com.mediatek.lbs.em2.ui.NmeaParser.NmeaUpdateViewListener;

public class LbsView extends Activity {
    protected RadioButton mRadioButtonGNSS;
    protected RadioButton mRadioButtonGPS;
    protected RadioButton mRadioButtonNMEA;
    protected RadioButton mRadioButtonINFO;
    protected RadioButton mRadioButtonPage1;
    protected RadioButton mRadioButtonPage2;
    protected RadioButton mRadioButtonPage3;

    private static final int SHOWING_VIEW_GNSS = 1;
    private static final int SHOWING_VIEW_GPS = 2;
    private static final int SHOWING_VIEW_NMEA = 3;
    private ViewGnss mViewGnss;
    private ViewGps mViewGps;
    private ViewNmea mViewNmea;
    private LocationManager mLocationManager = null;

    private NmeaUpdateViewListener mNmeaListener = null;
    private NmeaParser nmeaInd = NmeaParser.getNMEAParser();
    private int mShowingView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.view);

        mViewGnss = (ViewGnss) findViewById(R.id.viewgnss);
        mViewGps = (ViewGps) findViewById(R.id.viewgps);
        mViewNmea = (ViewNmea) findViewById(R.id.viewnmea);
        mRadioButtonGNSS= (RadioButton) findViewById(R.id.RadioButton_GNSS);
        mRadioButtonGPS= (RadioButton) findViewById(R.id.RadioButton_GPS);
        mRadioButtonNMEA= (RadioButton) findViewById(R.id.RadioButton_NMEA);
        mRadioButtonINFO= (RadioButton) findViewById(R.id.RadioButton_INFO);
        mRadioButtonPage1= (RadioButton) findViewById(R.id.RadioButton_Page1);
        mRadioButtonPage2= (RadioButton) findViewById(R.id.RadioButton_Page2);
        mRadioButtonPage3= (RadioButton) findViewById(R.id.RadioButton_Page3);

        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        if (mLocationManager == null) {
            log("ERR: mLocationManager is null");
        }

        mRadioButtonGNSS.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mShowingView = SHOWING_VIEW_GNSS;
                showView();
            }
        });
        mRadioButtonGPS.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mShowingView = SHOWING_VIEW_GPS;
                showView();
            }
        });
        mRadioButtonNMEA.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mShowingView = SHOWING_VIEW_NMEA;
                showView();
            }
        });
        mRadioButtonINFO.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showInfoDialog("Constellation Type:\n",
                        "G: GPS\n"
                        + "S: SBAS\n"
                        + "R: GLONASS\n"
                        + "Q: QZSS\n"
                        + "B: BEIDOU\n"
                        + "E: GALILEO\n"
                        + "I: IRNSS\n\n"
                        + "Example:\n"
                        + "GL1: GPS Band L1\n"
                        + "RL5: GLONASS Band L5");
                mRadioButtonINFO.setChecked(false);
                if (mShowingView == SHOWING_VIEW_GNSS) {
                    mRadioButtonGNSS.setChecked(true);
                } else if (mShowingView == SHOWING_VIEW_NMEA) {
                    mRadioButtonNMEA.setChecked(true);
                }
            }
        });

        mRadioButtonPage1.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mViewGnss.setDisplayPage(0);
                mViewGps.setDisplayPage(0);
                mViewNmea.setDisplayPage(0);
            }
        });

        mRadioButtonPage2.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mViewGnss.setDisplayPage(1);
                mViewGps.setDisplayPage(1);
                mViewNmea.setDisplayPage(1);
            }
        });

        mRadioButtonPage3.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mViewGnss.setDisplayPage(2);
                mViewGps.setDisplayPage(2);
                mViewNmea.setDisplayPage(2);
            }
        });

        ///// GNSS status callback
        mLocationManager.registerGnssStatusCallback(mGnssStatusCallback);

        ///// GPS staus callback
        mLocationManager.addGpsStatusListener(mGpsStatusListener);

        ///// MNEA status callback
        mNmeaListener = new NmeaStatusListener();
        nmeaInd.addSVUpdateListener(mNmeaListener);

        mShowingView = SHOWING_VIEW_GNSS;
        mRadioButtonGNSS.setChecked(true);
        mRadioButtonPage1.setChecked(true);
    }

    /// GNSS
    private GnssStatus.Callback mGnssStatusCallback = new GnssStatus.Callback() {
        @Override
        public void onStarted() {
            mViewGnss.resetGnssView();
        }

        @Override
        public void onSatelliteStatusChanged(GnssStatus status) {
            try {
                mViewGnss.setGnssStatus(status);
            } catch (Exception e) {
                Log.e("LocationEM", "Error encountered on setGnssStatus() ", e);
            }
        }
    };

    /// GPS
    private GpsStatus.Listener mGpsStatusListener = new GpsStatus.Listener() {
        @Override
        public void onGpsStatusChanged(int event) {
            if (event == GpsStatus.GPS_EVENT_STARTED) {
                mViewGps.resetGpsView();
            } else if (event == GpsStatus.GPS_EVENT_SATELLITE_STATUS) {
                try {
                    mViewGps.setGpsStatus(mLocationManager.getGpsStatus(null));
                } catch (Exception e) {
                    Log.e("LocationEM", "Error encountered on getGpsStatus() ", e);
                }
            }
        }
    };

    /// NMEA
    private class NmeaStatusListener implements NmeaParser.NmeaUpdateViewListener {
        @Override
        public void onViewupdateNotify() {
            mViewNmea.setGpsStatus(nmeaInd.getSatelliteList());
        }
    }

    private void showView() {
        log("showView mShowingView=" + mShowingView);

        mViewGnss.setVisibility(View.GONE);
        mViewGps.setVisibility(View.GONE);
        mViewNmea.setVisibility(View.GONE);

        switch(mShowingView) {
            case SHOWING_VIEW_GNSS:
                mViewGnss.setVisibility(View.VISIBLE);
                break;
            case SHOWING_VIEW_GPS:
                mViewGps.setVisibility(View.VISIBLE);
                break;
            case SHOWING_VIEW_NMEA:
                mViewNmea.setVisibility(View.VISIBLE);
                break;
        }
    }


    //=================== basic utility ========================\\
    @Override
    public void onResume() {
        super.onResume();
        showView();
    }

    protected void onDestroy() {
        super.onDestroy();
        /// GNSS
        mLocationManager.unregisterGnssStatusCallback(mGnssStatusCallback);

        /// GPS
        mLocationManager.removeGpsStatusListener(mGpsStatusListener);

        /// NMEA
        nmeaInd.removeSVUpdateListener(mNmeaListener);
    }

    private void log(String msg) {
        Log.d("LocationEM", msg);
    }

    private void showInfoDialog (CharSequence title, CharSequence message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setMessage(message);
        builder.setCancelable(true);

        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
            }
        });

        AlertDialog alert = builder.create();
        alert.show();
    }
}
