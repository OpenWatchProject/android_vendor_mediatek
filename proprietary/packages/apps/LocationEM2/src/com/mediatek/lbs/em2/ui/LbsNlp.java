package com.mediatek.lbs.em2.ui;

import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Locale;

import android.app.Activity;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ToggleButton;
import android.widget.TextView;
import android.widget.Toast;
import android.location.ILocationManager;

import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;

import java.io.IOException;
import java.util.Date;
import java.util.List;


public class LbsNlp extends Activity {

    private final static int UPDATE_GEO_LOCATION = 6;
    private final static int UPDATE_NLP_PACKAGE_NAME = 7;
    private static final String MTK_NLP_PACKAGE = "com.mediatek.location.mtknlp";
    private static final String GMS_NLP_PACKAGE = "com.google.android.gms";
    private static final String BAIDU_NLP_PACKAGE = "com.baidu.map.location";
    private static final String PERMISSION_NLP_PKG = "com.mediatek.permission.REQUEST_NLP_PACKAGE";

    protected ToggleButton mToggleButtonNlp;
    protected Button mButtonClearScreen;
    private Button mButtonGetGeolocation;

    private   TextView mTextViewNlpInstalledValue;
    private   TextView mTextViewNlpEnabledValue;
    private   TextView mTextViewGeocoderInstalledValue;
    private   TextView mTextViewMtkNlpBoundValue;
    private   TextView mTextViewNlpPackageNameValue;
    private   TextView mTextViewNlpResult;
    private   TextView mTextViewGeolocation;

    private LocationManager mLocationManager = null;
    private StringList mStringList = new StringList(12);
    private Location mLastLocation;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.nlp);
        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);

        initUI();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mLocationManager.removeUpdates(mLocationListener);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        mToggleButtonNlp.setChecked(false);
    }

    @Override
    public void onResume() {
        super.onResume();

        if (null != mLocationManager.getProvider(LocationManager.NETWORK_PROVIDER)) {
            mTextViewNlpInstalledValue.setText("Yes");
            mTextViewNlpInstalledValue.setTextColor(Color.GREEN);
        } else {
            mTextViewNlpInstalledValue.setText("No");
            mTextViewNlpInstalledValue.setTextColor(Color.RED);
        }

        if (mLocationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
            mTextViewNlpEnabledValue.setText("Yes");
            mTextViewNlpEnabledValue.setTextColor(Color.GREEN);
        } else {
            mTextViewNlpEnabledValue.setText("No");
            mTextViewNlpEnabledValue.setTextColor(Color.RED);
        }

        if(Geocoder.isPresent()) {
            mTextViewGeocoderInstalledValue.setText("Yes");
            mTextViewGeocoderInstalledValue.setTextColor(Color.GREEN);
        } else {
            mTextViewGeocoderInstalledValue.setText("No");
            mTextViewGeocoderInstalledValue.setTextColor(Color.RED);
        }

        if (mLocationManager.isProviderPackage(MTK_NLP_PACKAGE)) {
            mTextViewMtkNlpBoundValue.setText("Yes");
            mTextViewMtkNlpBoundValue.setTextColor(Color.GREEN);
        } else {
            mTextViewMtkNlpBoundValue.setText("No");
            mTextViewMtkNlpBoundValue.setTextColor(Color.RED);
        }

        if (mLocationManager.isProviderPackage(BAIDU_NLP_PACKAGE)) {
            mTextViewNlpPackageNameValue.setText(BAIDU_NLP_PACKAGE);
        } else if (mLocationManager.isProviderPackage(GMS_NLP_PACKAGE)) {
            mTextViewNlpPackageNameValue.setText(GMS_NLP_PACKAGE);
        }
    }

    protected void initUI() {
        mTextViewNlpInstalledValue = (TextView) findViewById(R.id.TextNlp_InstalledValue);
        mTextViewNlpEnabledValue = (TextView) findViewById(R.id.TextNlp_EnabledValue);
        mTextViewGeocoderInstalledValue = (TextView) findViewById(R.id.TextGeocoder_InstalledValue);
        mTextViewMtkNlpBoundValue = (TextView) findViewById(R.id.TextNlp_MtkNlpBound);
        mTextViewNlpPackageNameValue = (TextView) findViewById(R.id.TextNlp_PackageNameValue);
        mToggleButtonNlp = (ToggleButton) findViewById(R.id.toggleButton_Nlp);
        mButtonClearScreen = (Button) findViewById(R.id.Button_ClearScreen);
        mButtonGetGeolocation = (Button) findViewById(R.id.Button_GetGeoLocation);
        mTextViewNlpResult     = (TextView) findViewById(R.id.TextView_NlpResult);
        mTextViewGeolocation   = (TextView) findViewById(R.id.TextView_Geolocation);

        mTextViewNlpInstalledValue.setText("");
        mTextViewNlpEnabledValue.setText("");
        mTextViewGeocoderInstalledValue.setText("");
        mTextViewMtkNlpBoundValue.setText("");
        mTextViewNlpPackageNameValue.setText("");
        mTextViewGeolocation.setText("");

        mToggleButtonNlp.setChecked(false);
        mToggleButtonNlp.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mToggleButtonNlp.isChecked()) {
                    /// directly request location updates even the location settings OFF
                    try {
                        LocationRequest request = LocationRequest.createFromDeprecatedProvider(
                                LocationManager.NETWORK_PROVIDER,
                                1000 /*minTime*/, 0 /*minDistance*/, false /*oneShot*/);
                        request.setLocationSettingsIgnored(true);
                        mLocationManager.requestLocationUpdates(
                                request,
                                mLocationListener,
                                mHandler.getLooper());
                    } catch (IllegalArgumentException e) {
                        log("requestLocationUpdates to unexisted provider");
                    }
                } else {
                    mLocationManager.removeUpdates(mLocationListener);
                }
            }
        });

        mButtonClearScreen.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mStringList.clear();
                mTextViewNlpResult.setText(mStringList.get());
                mTextViewGeolocation.setText("");
            }
        });

        mButtonGetGeolocation.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mLastLocation == null) {
                    Toast.makeText(getApplicationContext(),
                            "Before using Geolocation, you need to have NLP position. " +
                            "Please turn on NLP for getting a NLP location.",
                            Toast.LENGTH_LONG).show();
                } else {
                    GeolocationThread geoThread = new GeolocationThread();
                    geoThread.getGeolocation();
                    mTextViewGeolocation.setText("Waiting...");
                }
            }
        });
    }

    private LocationListener mLocationListener = new LocationListener() {

        @Override
        public void onLocationChanged(Location location) {
            if (location == null) {
                log("ERR: onLocationChanged get null");
            } else {
                log("onLocationChanged");
                mLastLocation = location;
                String str = getTimeString(location.getTime()) + " " + location.toString() + "\n";
                mStringList.add(str);
                mTextViewNlpResult.setText(mStringList.get());
            }
        }

        @Override
        public void onProviderDisabled(String provider) {
        }

        @Override
        public void onProviderEnabled(String provider) {
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
        }

    };

    private boolean isNlpProviderEnable() {
        if (mLocationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER) == false) {
            new Builder(this)
                .setMessage("NLP is disabled now, Do you want to enable it?")
                //.setCancelable(false)
                .setPositiveButton("No", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                })
                .setNegativeButton("Yes", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        startActivity(new Intent(android.provider.Settings.ACTION_LOCATION_SOURCE_SETTINGS));
                    }
                }).show();

            return false;
        }
        return true;
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
            case UPDATE_GEO_LOCATION:
                if (msg.obj == null) {
                    mTextViewGeolocation.setText("Get Address failed!");
                } else {
                    Address addr = (Address)msg.obj;
                    String str = "";
                    str += "CountryNmae: " + addr.getCountryName();
                    str += "\nCountryCode: " + addr.getCountryCode();
                    str += "\nAdminArea: " + addr.getAdminArea();
                    str += "\nSubAdminArea: " + addr.getSubAdminArea();
                    str += "\nLocality: " + addr.getLocality();
                    str += "\nSubLocality: " + addr.getSubLocality();
                    str += "\nPremises: " + addr.getPremises();
                    str += "\nPostalCode: " + addr.getPostalCode();
                    str += "\nFeatureName: " + addr.getFeatureName();
                    str += "\nAddressLine: " + addr.getAddressLine(0);
                    mTextViewGeolocation.setText(str);
                    log("GeoLocation successfully received from geocoder");
                }
                break;
            case UPDATE_NLP_PACKAGE_NAME:
                String pkgName = (String)msg.obj;
                if (pkgName != null) {
                    mTextViewNlpPackageNameValue.setText(pkgName);
                }
                break;
            default:
                log("WARNING: unknown handle event recv!!");
                break;
            }
        }
    };

    class GeolocationThread extends Thread {
        public void run() {
            Geocoder geocoder = new Geocoder(getApplicationContext());
            List<Address> addresses = null;
            try {
                log("Query geoLocation");
                addresses = geocoder.getFromLocation(
                        mLastLocation.getLatitude(), mLastLocation.getLongitude(), 5);
            } catch (IOException e) {
                log("geoLocation exception: " + (String) e.getMessage());
                sendMessage(UPDATE_GEO_LOCATION, 0, 0, null);
            }
            if (addresses != null && addresses.size() > 0) {
                log("geoLocation updated size: " + addresses.size());
                sendMessage(UPDATE_GEO_LOCATION, 0, 0, addresses.get(0));
            } else {
                log("geoLocation ret null address");
                sendMessage(UPDATE_GEO_LOCATION, 0, 0, null);
            }
        }
        public void getGeolocation() {
            this.start();
        }
    }

    private String getTimeString(long milliseconds) {
        Date date = new Date(milliseconds);
        String str = String.format("%04d/%02d/%02d %02d:%02d:%02d",
                (date.getYear() + 1900),
                (date.getMonth() + 1),
                date.getDate(),
                date.getHours(),
                date.getMinutes(),
                date.getSeconds());
        return str;
    }

    protected static void log(Object msg) {
        Log.d("LocationEM_NLP", "" + msg);
    }

}
