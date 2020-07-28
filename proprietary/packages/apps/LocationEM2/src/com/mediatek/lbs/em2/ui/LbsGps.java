package com.mediatek.lbs.em2.ui;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.location.GnssStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.location.OnNmeaMessageListener;
import android.net.LocalSocketAddress.Namespace;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.text.method.DigitsKeyListener;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.PopupMenu;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.mediatek.lbs.em2.mnldinterface.Debug2MnldInterface;
import com.mediatek.lbs.em2.mnldinterface.Debug2MnldInterface.Debug2MnldInterfaceSender;
import com.mediatek.socket.base.UdpClient;


public class LbsGps extends Activity {

    private final static int TIMER_UPDATE        = 0;
    private final static int STRESS_END          = 1;
    private final static int STRESS_LOOP_UPDATE  = 2;
    private final static int STRESS_START_GPS    = 3;
    private final static int STRESS_STOP_GPS     = 4;
    private final static int STRESS_DELETE_DATA  = 5;
    private final static int HAS_ALMANAC_UPDATE  = 6;

    private static final String CHANNEL_OUT = "mtk_debugService2mnld"; // send cmd to mnld
    private static final String GPS_ALMANAC = "vendor.gps.almanac";
    private static final String BLACKLIST_DELIMITER = ",";
    private static final int BLACKLIST_SV_COUNT = 41;

    private int mNumOfLoop = 100;
    private int mDelay1 = 20;
    private int mDelay2 = 2;
    private int mDelay3 = 600;
    private int mDelay4 = 0;
    private StressTest mStressTest = new StressTest(
            mNumOfLoop, mDelay1, mDelay2, mDelay3, mDelay4);

    private int mFixCount = 0;
    private int mSatUpdateCount = 0;
    private int mNmeaCount = 0;
    private int mTTFF = -1;
    private boolean mGotFix = false;
    private Location mFirstLocation;
    private float mFirstDistance = 0;
    private String mFirstSvNumStr = "";
    private String mFirstSvStr = "";
    private boolean mGotFirstSv = false;

    private float mDistance = 0;
    private double mRefLat = 0;
    private double mRefLng = 0;
    private int mGpsMode = 0;
    private boolean mLogToSdcard = false;
    private boolean mLowPowerMode = false;
    private boolean mSingleShot = false;

    private String mNmeaFileName = null;
    private String mNmeaFolderName = null;
    private String mBlackList = "";
    private Debug2MnldInterfaceSender mToMnldSender;
    private UdpClient mUdpClient;

    private ToggleButton    mButtonGps;
    private Button          mButtonDelete;
    private Button          mButtonGpsMode;
    private PopupMenu       mPopupDeleteAidingData;
    private Button          mButtonChangeRefLatLng;
    private Button          mButtonClearLog;
    private ToggleButton    mButtonStress;
    private CheckBox        mCheckBoxLogNmea;
    private CheckBox        mCheckBoxLogToSdcard;
    private CheckBox        mCheckBoxLowPowerMode;
    private CheckBox        mCheckBoxSingleShot;
    private CheckBox        mCheckBoxSwitchDelet2First;
    private TextView        mTextViewRefLatLng;
    private TextView        mTextViewTimer;
    private TextView        mTextViewLocation;
    private TextView        mTextViewSatellite;
    private TextView        mTextViewNmea;
    private TextView        mTextView5;
    private TextView        mTextViewLoop;
    private TextView        mTextViewDeleteAidingData;
    private TextView        mTextViewHasAlmanacValue;
    private TextView        mTextViewAlmanacDateValue;
    private TextView        mTextViewYearOfGnssHwValue;
    private TextView        mTextViewGnssHwNameValue;
    private EditText        mEditTextNumOfLoop;
    private EditText        mEditTextDelay1;
    private EditText        mEditTextDelay2;
    private EditText        mEditTextDelay3;
    private EditText        mEditTextDelay4;
    private LocationManager mLocationManager = null;
    private StringList mStringList = new StringList(16);
    private GpsTimer        mTimer = new GpsTimer();
    private CheckBox        mCheckBoxBlockGps;
    private CheckBox        mCheckBoxBlockGlonass;
    private CheckBox        mCheckBoxBlockBeidou;
    private CheckBox        mCheckBoxBlockGalileo;
    private TextView        mTextViewBlockGps;
    private TextView        mTextViewBlockGlonass;
    private TextView        mTextViewBlockBeidou;
    private TextView        mTextViewBlockGalileo;
    private CheckBox        mCheckBoxSvArr[] = new CheckBox[BLACKLIST_SV_COUNT];

    @Override
    public void onCreate(Bundle savedInstanceState) {
        log("onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.gps);

        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        if (mLocationManager == null) {
            log("ERR: mLocationManager is null");
        }

        mLocationManager.addNmeaListener(mNmeaListener);
        mLocationManager.registerGnssStatusCallback(mGnssStatusCallback);

        initWidget();
        updateRefLatLng();

        mNmeaFolderName = "single_" + getTimeString2(new Date().getTime());
        mNmeaFileName = null;

        mToMnldSender = new Debug2MnldInterfaceSender();
        mUdpClient = new UdpClient(CHANNEL_OUT, Namespace.ABSTRACT,
                Debug2MnldInterface.MAX_BUFF_SIZE);
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        mButtonGps.setChecked(false);
        mButtonStress.setChecked(false);
    }

    @Override
    public void onResume() {
        super.onResume();
        updateHasAlmanacValue();
        updateYearOfGnssHwValue();
        updateGnssHwNameValue();
        mToMnldSender.debugMnldNeMsg(mUdpClient, true);
    }

    private boolean isGPSProviderEnable() {
        if (mLocationManager.isProviderEnabled(LocationManager.GPS_PROVIDER) == false) {
            new Builder(this)
                .setMessage("GPS is disabled now, Do you want to enable it?")
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

    //=============== initial widget ============================\\
    private void initWidget() {
        mButtonGps         = (ToggleButton) findViewById(R.id.Button_StartGPS);
        mButtonDelete      = (Button) findViewById(R.id.Button_Once);
        mButtonGpsMode     = (Button) findViewById(R.id.Button_GpsMode);
        mButtonChangeRefLatLng = (Button) findViewById(R.id.Button_ChangeRefLatLng);
        mButtonClearLog      = (Button) findViewById(R.id.Button_ClearLog);
        mCheckBoxLogNmea   = (CheckBox) findViewById(R.id.CheckBox_LogNmea);
        mCheckBoxLogToSdcard = (CheckBox) findViewById(R.id.CheckBox_LogToSdcard);
        mCheckBoxLowPowerMode = (CheckBox) findViewById(R.id.CheckBox_LowPowerMode);
        mCheckBoxSingleShot = (CheckBox) findViewById(R.id.CheckBox_SingleShot);
        mCheckBoxSwitchDelet2First = (CheckBox) findViewById(R.id.checkBox_switchDeleteToFirst);
        mButtonStress      = (ToggleButton) findViewById(R.id.Button_Stress);
        mTextViewRefLatLng  = (TextView) findViewById(R.id.TextView_0);
        mTextViewTimer      = (TextView) findViewById(R.id.TextView_1);
        mTextViewLocation   = (TextView) findViewById(R.id.TextView_2);
        mTextViewSatellite  = (TextView) findViewById(R.id.TextView_3);
        mTextViewNmea       = (TextView) findViewById(R.id.TextView_4);
        mTextView5          = (TextView) findViewById(R.id.TextView_5);
        mTextViewLoop        = (TextView) findViewById(R.id.TextView_Loop);
        mTextViewDeleteAidingData = (TextView) findViewById(R.id.TextView_DeleteAidingData);
        mEditTextNumOfLoop    = (EditText) findViewById(R.id.EditText_NumOfLoop);
        mEditTextDelay1        = (EditText) findViewById(R.id.EditText_Delay1);
        mEditTextDelay2        = (EditText) findViewById(R.id.EditText_Delay2);
        mEditTextDelay3        = (EditText) findViewById(R.id.EditText_Delay3);
        mEditTextDelay4        = (EditText) findViewById(R.id.EditText_Delay4);
        mTextViewHasAlmanacValue = (TextView) findViewById(R.id.TextView_HasAlmanacValue);
        mTextViewAlmanacDateValue = (TextView) findViewById(R.id.TextView_AlmanacDateValue);
        mTextViewYearOfGnssHwValue = (TextView) findViewById(R.id.TextView_YearOfGnssHwValue);
        mTextViewGnssHwNameValue = (TextView) findViewById(R.id.TextView_GnssHwNameValue);
        mCheckBoxBlockGps       = (CheckBox) findViewById(R.id.checkBox_BlockGpsSatellites);
        mCheckBoxBlockGlonass   = (CheckBox) findViewById(R.id.checkBox_BlockGlonassSatellites);
        mCheckBoxBlockBeidou    = (CheckBox) findViewById(R.id.checkBox_BlockBeidouSatellites);
        mCheckBoxBlockGalileo   = (CheckBox) findViewById(R.id.checkBox_BlockGalileoSatellites);
        mTextViewBlockGps        = (TextView) findViewById(R.id.TextView_BlockGps);
        mTextViewBlockGlonass    = (TextView) findViewById(R.id.TextView_BlockGlonass);
        mTextViewBlockBeidou     = (TextView) findViewById(R.id.TextView_BlockBeidou);
        mTextViewBlockGalileo    = (TextView) findViewById(R.id.TextView_BlockGalileo);

        mTextView5.setVisibility(4); //4: disable
        mTextViewNmea.setTextSize(11.0f);
        mTextViewDeleteAidingData.setText("Hot Start");

        mEditTextNumOfLoop.setText(String.valueOf(mNumOfLoop));
        mEditTextDelay1.setText(String.valueOf(mDelay1));
        mEditTextDelay2.setText(String.valueOf(mDelay2));
        mEditTextDelay3.setText(String.valueOf(mDelay3));
        mEditTextDelay4.setText(String.valueOf(mDelay4));

        mCheckBoxLogToSdcard.setEnabled(mCheckBoxLogNmea.isChecked());

        mTextViewRefLatLng.setText(
                String.format("Reference Lat=%.06f Lng=%.06f", mRefLat, mRefLng));

        mTextViewHasAlmanacValue.setText("No");
        mTextViewAlmanacDateValue.setText("");
        mTextViewYearOfGnssHwValue.setText("");
        mTextViewGnssHwNameValue.setText("");

        mButtonGps.setChecked(false);
        mButtonGps.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {

                if (mButtonGps.isChecked()) {
                    /// direct request location updates, even in location settings OFF
                    mButtonStress.setEnabled(false);
                    startGps();
                } else {
                    mButtonStress.setEnabled(true);
                    stopGps();
                }
            }
        });

        mButtonGpsMode.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPopupDeleteAidingData.show();
            }
        });


        mPopupDeleteAidingData= new PopupMenu(this, mButtonGpsMode);
        mPopupDeleteAidingData.getMenu().add(0, 0, Menu.NONE, "Hot");
        mPopupDeleteAidingData.getMenu().add(0, 1, Menu.NONE, "Warm");
        mPopupDeleteAidingData.getMenu().add(0, 2, Menu.NONE, "Cold");
        mPopupDeleteAidingData.getMenu().add(0, 3, Menu.NONE, "Full");
        mPopupDeleteAidingData.getMenu().add(0, 4, Menu.NONE, "delete ALM, EPH");
        mPopupDeleteAidingData.getMenu().add(0, 5, Menu.NONE, "delete EPH");
        mPopupDeleteAidingData.getMenu().add(0, 6, Menu.NONE, "delete Time");
        mPopupDeleteAidingData.getMenu().add(0, 7, Menu.NONE, "delete all except Time, ALM");
        mPopupDeleteAidingData.getMenu().add(0, 8, Menu.NONE, "delete all except Time, ALM, EPO");

        mPopupDeleteAidingData
                .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        mGpsMode = item.getItemId();
                        if (mGpsMode == 0)
                            mTextViewDeleteAidingData.setText("Hot Start");
                        else if (mGpsMode == 1)
                            mTextViewDeleteAidingData.setText("Warm Start");
                        else if (mGpsMode == 2)
                            mTextViewDeleteAidingData.setText("Cold Start");
                        else if (mGpsMode == 3)
                            mTextViewDeleteAidingData.setText("Full Start");
                        else if (mGpsMode == 4)
                            mTextViewDeleteAidingData.setText("delete ALM, EPH");
                        else if (mGpsMode == 5)
                            mTextViewDeleteAidingData.setText("delete EPH");
                        else if (mGpsMode == 6)
                            mTextViewDeleteAidingData.setText("delete Time");
                        else if (mGpsMode == 7)
                            mTextViewDeleteAidingData.setText("delete all except Time, ALM");
                        else if (mGpsMode == 8)
                            mTextViewDeleteAidingData.setText("delete all except Time, ALM, EPO");
                        return false;
                    }
                });

        mButtonDelete.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                startDeletingAidingData();
                msleep(300);
                //clean text display
                resetVarialbe();
            }
        });
        mButtonClearLog.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                showClearLogConfirmDialog();
            }
        });

        mButtonChangeRefLatLng.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                openDialogLatlng(false);
            }
        });
        mCheckBoxLogNmea.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                showLogNmeaConfirmDialog();
            }
        });
        mCheckBoxLogToSdcard.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                mLogToSdcard = mCheckBoxLogToSdcard.isChecked();
            }
        });
        mCheckBoxLowPowerMode.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                mLowPowerMode = mCheckBoxLowPowerMode.isChecked();
            }
        });
        mCheckBoxSingleShot.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                mSingleShot = mCheckBoxSingleShot.isChecked();
            }
        });

        mCheckBoxBlockGps.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                CheckBox clickedCheckBox = (CheckBox) v;
                if (clickedCheckBox.isChecked()) {
                    openBlackSvSelectDialog(GnssStatus.CONSTELLATION_GPS,
                            mTextViewBlockGps.getText().toString());
                } else {
                    setBlacklistData();
                }
            }
        });
        mCheckBoxBlockGlonass.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                CheckBox clickedCheckBox = (CheckBox) v;
                if (clickedCheckBox.isChecked()) {
                    openBlackSvSelectDialog(GnssStatus.CONSTELLATION_GLONASS,
                            mTextViewBlockGlonass.getText().toString());
                } else {
                    setBlacklistData();
                }
            }
        });
        mCheckBoxBlockBeidou.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                CheckBox clickedCheckBox = (CheckBox) v;
                if (clickedCheckBox.isChecked()) {
                    openBlackSvSelectDialog(GnssStatus.CONSTELLATION_BEIDOU,
                            mTextViewBlockBeidou.getText().toString());
                } else {
                    setBlacklistData();
                }
            }
        });
        mCheckBoxBlockGalileo.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                CheckBox clickedCheckBox = (CheckBox) v;
                if (clickedCheckBox.isChecked()) {
                    openBlackSvSelectDialog(GnssStatus.CONSTELLATION_GALILEO,
                            mTextViewBlockGalileo.getText().toString());
                } else {
                    setBlacklistData();
                }
            }
        });

        mButtonStress.setChecked(false);
        mButtonStress.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (mButtonStress.isChecked()) {
                    if (isGPSProviderEnable() == false) {
                        mButtonStress.setChecked(false);
                    } else {
                        startStressGps();
                    }
                } else {
                    stopStressGps();
                }
            }
        });
    }

    //=============== Location framework related API ========================\\

    private void startGps() {
        log("startGps");

        mTimer = new GpsTimer();
        mTimer.startTimer();
        //change nmea file each time when GPS started
        mNmeaFileName = getTimeString2(new Date().getTime());
        LocationRequest request = LocationRequest.createFromDeprecatedProvider(
                LocationManager.GPS_PROVIDER,
                (mLowPowerMode ? 300000 : 1000) /*minTime*/, 0 /*minDistance*/, mSingleShot /*oneShot*/);
        request.setLowPowerMode(mLowPowerMode);
        request.setLocationSettingsIgnored(true);
        mLocationManager.requestLocationUpdates(
                request,
                mLocationListener,
                mHandler.getLooper());
        resetVarialbe();
    }

    private void stopGps() {
        log("stopGps");
        mTimer.stopTimer();
        mLocationManager.removeUpdates(mLocationListener);
    }

    private void resetVarialbe() {
        //reset the variables
        mFixCount = 0;
        mSatUpdateCount = 0;
        mNmeaCount = 0;
        mTTFF = -1;
        mGotFix = false;
        mFirstLocation = null;
        mFirstDistance = 0;
        mDistance = 0;
        mStringList.clear();
        mFirstSvNumStr = "";
        mFirstSvStr = "";
        mGotFirstSv = false;
        mTextViewTimer.setText("");
        mTextViewLocation.setText("");
        mTextViewSatellite.setText("");
        mTextViewNmea.setText("");
        mTimer.resetTimer();
    }

    private void setGpsMode(int mode) {
        Bundle extras = new Bundle();
        if (mode == 0) {
            log("Hot Start");
            extras.putBoolean("rti", true);
            mTextViewDeleteAidingData.setText("Hot Start: delete rti");
        } else if (mode == 1) {
            log("Warm Start");
            mTextViewDeleteAidingData.setText("Warm Start: delete eph");
            extras.putBoolean("ephemeris", true);
        } else if (mode == 2) {
            log("Cold Start");
            extras.putBoolean("ephemeris", true);
            extras.putBoolean("position", true);
            extras.putBoolean("time", true);
            extras.putBoolean("iono", true);
            extras.putBoolean("utc", true);
            extras.putBoolean("health", true);
            mTextViewDeleteAidingData.setText("Cold Start: delete eph, pos, time, iono, utc, hlth");
        } else if (mode == 3) {
            log("Full Start");
            extras.putBoolean("all", true);
            mTextViewDeleteAidingData.setText("Full Start: delete all");
        } else if (mode == 4) {
            log("VzW Delete Almanac, Ephemeris");
            extras.putBoolean("ephemeris", true);
            extras.putBoolean("almanac", true);
            mTextViewDeleteAidingData.setText("delete eph, alm");
        } else if (mode == 5) {
            log("VzW Delete Ephemeris");
            extras.putBoolean("ephemeris", true);
            mTextViewDeleteAidingData.setText("delete eph");
        } else if (mode == 6) {
            log("VzW Delete Reference Time");
            extras.putBoolean("time", true);
            mTextViewDeleteAidingData.setText("delete time");
        } else if (mode == 7) {
            log("Vzw Delete all except time, alm");
            extras.putBoolean("ephemeris", true);
            //extras.putBoolean("almanac", true);
            extras.putBoolean("position", true);
            //extras.putBoolean("time", true);
            extras.putBoolean("iono", true);
            extras.putBoolean("utc", true);
            extras.putBoolean("health", true);
            extras.putBoolean("svdir", true);
            extras.putBoolean("svsteer", true);
            extras.putBoolean("sadata", true);
            extras.putBoolean("rti", true);
            extras.putBoolean("hot-still", true);
            extras.putBoolean("epo", true);
            extras.putBoolean("celldb-info", true);
            mTextViewDeleteAidingData.setText("delete all except time, alm");
        } else if (mode == 8) {
            log("VzW Delete all except time, alm, epo");
            extras.putBoolean("ephemeris", true);
            //extras.putBoolean("almanac", true);
            extras.putBoolean("position", true);
            //extras.putBoolean("time", true);
            extras.putBoolean("iono", true);
            extras.putBoolean("utc", true);
            //extras.putBoolean("health", true);
            extras.putBoolean("svdir", true);
            extras.putBoolean("svsteer", true);
            extras.putBoolean("sadata", true);
            extras.putBoolean("rti", true);
            //extras.putBoolean("hot-still", true);
            //extras.putBoolean("epo", true);
            extras.putBoolean("celldb-info", true);
            mTextViewDeleteAidingData.setText("delete all except time, alm, epo");
        } else {
            log("WARNING: unknown reset type");
            return;
        }
        mLocationManager.sendExtraCommand(LocationManager.GPS_PROVIDER, "delete_aiding_data", extras);
    }

    private OnNmeaMessageListener mNmeaListener = new OnNmeaMessageListener() {
        public void onNmeaMessage(String nmea, long timestamp) {
            //Log.d("NMEA", nmea);
            mStringList.add(nmea);
            mNmeaCount++;
            mTextViewNmea.setText("NMEA Count=" + mNmeaCount + "\n" + mStringList.get());
            NmeaParser parser = NmeaParser.getNMEAParser();
            parser.parse(nmea);
            if (mCheckBoxLogNmea.isChecked() && mNmeaFileName != null) {
                write2File("nmea", mNmeaFolderName, "NMEA_" + mNmeaFileName + ".txt",
                        nmea, true, mLogToSdcard);
            }
        }
    };

    private GnssStatus.Callback mGnssStatusCallback = new GnssStatus.Callback() {
        @Override
        public void onStarted() {
            sendMessage(HAS_ALMANAC_UPDATE, 0);
        }

        @Override
        public void onSatelliteStatusChanged(GnssStatus status) {
            String str = new String();
            mSatUpdateCount++;
            int i = 0;
            int svUsedInFixCount = 0;
            for (i=0; i < status.getSatelliteCount(); i++) {
                str += "Id=" + LbsUtility.map2SvTypes(status.getConstellationType(i)) + "_"
                    + LbsUtility.getFrequencyBand(status.getCarrierFrequencyHz(i)) + "_"
                    + status.getSvid(i)
                    + " SNR=" + String.format("%.01f", status.getCn0DbHz(i))
                    + " AZI=" + String.format("%.01f", status.getAzimuthDegrees(i))
                    + " ELE=" + String.format("%.01f", status.getElevationDegrees(i))
                    + " UInFix=" + (status.usedInFix(i) ? "1" : "0") + "\n";
                if (status.usedInFix(i)) {
                    svUsedInFixCount++;
                }
            }
            mTextViewSatellite.setText("Satellite Status Update Count=" + mSatUpdateCount + "\n" +
                    "Satellite Num=" + i + "\n" + str);
            if (mTTFF > -1 && !mGotFirstSv && svUsedInFixCount > 0) {
                log("onSatelliteStatusChanged for first svUsedInFix count = " + svUsedInFixCount);
                updateFirstSvStr(status);
                mGotFirstSv = true;
            }
        }

        @Override
        public void onFirstFix(int ttffMillis) {
            log("onFirstFix ttffMillis:" + ttffMillis);
        }
    };

    private LocationListener mLocationListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {
            if (location == null) {
                log("ERR: onLocationChanged get null");
            } else {
                log("onLocationChanged");
                mFixCount++;

                float distance[] = new float[2];
                Location.distanceBetween(location.getLatitude(), location.getLongitude(),
                        mRefLat, mRefLng, distance);
                mDistance = distance[0];

                if (mGotFix == false) {
                    log("onLocationChanged for TTFF");
                    mGotFix = true;
                    mTTFF = mTimer.getTimer();
                    mFirstLocation = location;
                    mFirstDistance = mDistance;
                    mStressTest.gotFixNotify();
                }
                updateLocationInfo(location);
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

    //=============== Stress test functions ========================\\
    private void startStressGps() {
        try {
            if (Integer.valueOf(mEditTextDelay3.getText().toString()) < 1)
                throw new NumberFormatException();
            mStressTest = new StressTest(
                    Integer.valueOf(mEditTextNumOfLoop.getText().toString()),
                    Integer.valueOf(mEditTextDelay1.getText().toString()),
                    Integer.valueOf(mEditTextDelay2.getText().toString()),
                    Integer.valueOf(mEditTextDelay3.getText().toString()),
                    Integer.valueOf(mEditTextDelay4.getText().toString()));
            mStressTest.startStress();
        } catch (NumberFormatException e) {
            e.printStackTrace();
            Toast.makeText(getApplicationContext(),
                    "ERR: Parameters error!!", Toast.LENGTH_SHORT).show();
            mButtonStress.setChecked(false);
            return;
        }

        mButtonGps.setEnabled(false);
        mEditTextNumOfLoop.setEnabled(false);
        mEditTextDelay1.setEnabled(false);
        mEditTextDelay2.setEnabled(false);
        mEditTextDelay3.setEnabled(false);
        mEditTextDelay4.setEnabled(false);
        mButtonDelete.setEnabled(false);
        mButtonGpsMode.setEnabled(false);
        mCheckBoxSwitchDelet2First.setEnabled(false);

        mNmeaFolderName = "stress_" + getTimeString2(new Date().getTime());
        mNmeaFileName = null;
    }

    private void stopStressGps() {
        enableWidgetAfterStress();
        mNmeaFolderName = "single_" + getTimeString2(new Date().getTime());
        mNmeaFileName = null;
    }

    private void enableWidgetAfterStress() {
        mStressTest.stopStress();

        mButtonGps.setEnabled(true);
        mEditTextNumOfLoop.setEnabled(true);
        mEditTextDelay1.setEnabled(true);
        mEditTextDelay2.setEnabled(true);
        mEditTextDelay3.setEnabled(true);
        mEditTextDelay4.setEnabled(true);
        mButtonDelete.setEnabled(true);
        mButtonGpsMode.setEnabled(true);

        mButtonStress.setChecked(false);
        mCheckBoxSwitchDelet2First.setEnabled(true);
    }

    class StressTest extends Thread {
        private int numOfLoop;
        private int delay1; //before starting GPS
        private int delay2; //before deleting data
        private int delay3; //Timeout
        private int delay4; //before stopping GPS
        private boolean enable = true;
        public StressTest() {
            this(100, 20, 1, 600, 0);
        }
        public StressTest(int numOfLoop, int delay1,
                int delay2, int delay3, int delay4) {
            this.numOfLoop = numOfLoop;
            this.delay1 = delay1;
            this.delay2 = delay2;
            this.delay3 = delay3;
            this.delay4 = delay4;
            enable = true;
        }
        public void startStress() {
            this.start();
        }

        public void stopStress() {
            enable = false;
            this.interrupt();
        }

        public void gotFixNotify() {
            this.interrupt();
        }

        public String toString() {
            return "StressTest " + " numOfLoop=" + numOfLoop + " delay1=" + delay1 +
                    " delay2=" + delay2 + " delay3=" + delay3 + " delay4=" + delay4;
        }

        public void run() {
            log("numOfLoop=" + numOfLoop + " delay1=" + delay1 + " delay2=" + delay2 +
                    " delay3=" + delay3 + " delay4=" + delay4);
            for (int i = 0; i < numOfLoop; i++) {
                log("=== stress test loop=" + i + "===");
                sendMessage(STRESS_LOOP_UPDATE, i + 1);
                oneSession();
                sendMessage(STRESS_STOP_GPS, 0);
                if (enable == false)
                    break;
            }
            log("end of stress test");
            sendMessage(STRESS_END, 0);
        }

        private void oneSession() {

            if (mCheckBoxSwitchDelet2First.isChecked()) {
                try {
                    Thread.sleep(delay2 * 1000);
                } catch (InterruptedException e) {
                    return;
                }
                sendMessage(STRESS_DELETE_DATA, 0);
            }

            try {
                Thread.sleep(delay1 * 1000);
            } catch (InterruptedException e) {
                return;
            }

            sendMessage(STRESS_START_GPS, 0);

            try {
                Thread.sleep(delay3 * 1000);
                return;
            } catch (InterruptedException e) {
                if (enable == false) {
                    return;
                }
            }

            try {
                Thread.sleep(delay4 * 1000);
            } catch (InterruptedException e) {
                return;
            }
        }
    } //end of class StressTest

    //=================== basic utility ========================\\
    private void startDeletingAidingData() {
        log("startDeletingAidingData");
            if (mGpsMode == 0)
                setGpsMode(0);
            else if (mGpsMode == 1)
                setGpsMode(1);
            else if (mGpsMode == 2)
                setGpsMode(2);
            else if (mGpsMode == 3)
                setGpsMode(3);
            else if (mGpsMode == 4)
                setGpsMode(4);
            else if (mGpsMode == 5)
                setGpsMode(5);
            else if (mGpsMode == 6)
                setGpsMode(6);
            else if (mGpsMode == 7)
                setGpsMode(7);
            else if (mGpsMode == 8)
                setGpsMode(8);

    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch(msg.what) {
            case TIMER_UPDATE: //timer update
                mTextViewTimer.setText("Timer=" + msg.arg1);
                break;
            case STRESS_END: //Stress thread sent for ending the stress test session
                enableWidgetAfterStress();
                break;
            case STRESS_LOOP_UPDATE: //Stress thread sent for updating current number of loop
                mTextViewLoop.setText("Current loop=" + msg.arg1);
                break;
            case STRESS_START_GPS: //Stress thread sent for starting GPS
                startGps();
                break;
            case STRESS_STOP_GPS: //Stress thread sent for Stopping GPS
                stopGps();
                break;
            case STRESS_DELETE_DATA: //Stress thread sent for Deleting Aiding data
                startDeletingAidingData();
                resetVarialbe();
                break;
            case HAS_ALMANAC_UPDATE: // Update has almanac value
                updateHasAlmanacValue();
                break;
            default:
                log("WARNING: unknown handle event recv!!");
                break;
            }
        }
    };

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

    private void sendMessage(int what, int arg1) {
        Message m = new Message();
        m.what = what;
        m.arg1 = arg1;
        mHandler.sendMessage(m);
    }

    class GpsTimer extends Thread {
        private boolean enable = true;
        private int timeCount = 0;
        public void run() {
            while (enable) {
                try {
                    Thread.sleep(1000);
                    timeCount++;
                    sendMessage(TIMER_UPDATE, timeCount);
                    log("time=" + timeCount);
                } catch (InterruptedException e) {
                    //e.printStackTrace();
                }
            }
            log("GpsTimer stopped");
        }
        public int getTimer() {
            return timeCount;
        }
        public void startTimer() {
            this.start();
        }
        public void stopTimer() {
            enable = false;
            this.interrupt();
        }
        public void resetTimer() {
            timeCount = 0;
        }
    }

    @SuppressWarnings(value = {"unused" })
    private void msleep(long milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void openDialogLatlng(boolean warning) {
        final Dialog dialog = new Dialog(this);
        dialog.setTitle("Change Ref Position");
        dialog.setContentView(R.layout.gps_ref_latlng);

        final EditText editText_lat = (EditText) dialog.findViewById(R.id.EditText_lat);
        final EditText editText_lng = (EditText) dialog.findViewById(R.id.EditText_lng);
        Button button_ok            = (Button) dialog.findViewById(R.id.Button_ok);
        Button button_cancel        = (Button) dialog.findViewById(R.id.Button_cancel);
        Button button_apply        = (Button) dialog.findViewById(R.id.Button_apply);

        editText_lat.setText(String.valueOf(mRefLat));
        editText_lat.setKeyListener(DigitsKeyListener.getInstance("0123456789.-"));
        editText_lng.setText(String.valueOf(mRefLng));
        editText_lng.setKeyListener(DigitsKeyListener.getInstance("0123456789.-"));

        button_ok.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                try {
                    double lat = Double.valueOf(editText_lat.getText().toString());
                    double lng = Double.valueOf(editText_lng.getText().toString());
                    if (lat > 90.0 || lng > 180.0 || lat < -90.0 || lng < -180.0) {
                        openDialogLatlng(true);
                    } else {
                        mRefLat = lat;
                        mRefLng = lng;
                        mTextViewRefLatLng.setText(
                                String.format("Reference Lat=%.06f Lng=%.06f", mRefLat, mRefLng));
                    }
                } catch (NumberFormatException e) {
                    openDialogLatlng(true);
                }
                dialog.dismiss();
            }
        });
        button_cancel.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                dialog.dismiss();
            }
        });
        button_apply.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                updateRefLatLng();
                dialog.dismiss();
            }
        });

        dialog.show();

        if (warning == true) {
            Button button = new Button(this);
            final Dialog dialog1 = new Dialog(this);
            button.setText("OK");
            dialog1.setTitle("Input lat/lng are incorrect");
            dialog1.setContentView(button);
            dialog1.show();
            button.setOnClickListener(new OnClickListener() {
                public void onClick(View v) {
                    dialog1.dismiss();
                }
            });
        }
    }

    private void updateRefLatLng() {
        Location location = mLocationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
        if (location != null) {
            mRefLat = location.getLatitude();
            mRefLng = location.getLongitude();
            mTextViewRefLatLng.setText(
                    String.format("Reference Lat=%.06f Lng=%.06f", mRefLat, mRefLng));
        }
    }

    private String getTimeString(long milliseconds) {
        Date date = new Date(milliseconds);
        String str = (date.getYear() + 1900) + "/" + (date.getMonth() + 1) + "/" + date.getDate() +
                " " + date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds();
        return str;
    }

    private void updateLocationInfo(Location location) {
        if (location == null) {
            return;
        }
        String str;
        str = "Location Count=" + mFixCount + "\n";
        str += String.format("Lat=%.06f Lng=%.06f\n",
                location.getLatitude(), location.getLongitude());
        str += "Time tick=" + location.getTime() + "\n";
        str += "UTC time=" + getTimeString(location.getTime()) + "\n";
        str += "ElapsedtimeNs=" + location.getElapsedRealtimeNanos() + "\n";
        str += "ElapsedtimeUncertaintyNs=" + location.getElapsedRealtimeUncertaintyNanos() + "\n";
        str += "Altitude=" + location.getAltitude() +  " " + location.hasAltitude() + "\n";
        str += "H Accuracy=" + location.getAccuracy() +  " " + location.hasAccuracy() + "\n";
        str += "V Accuracy=" + location.getVerticalAccuracyMeters() +  " " + location.hasVerticalAccuracy() + "\n";
        str += "Bearing=" + location.getBearing() +  " " + location.hasBearing() + "\n";
        str += "Bearing Accuracy=" + location.getBearingAccuracyDegrees() +  " " + location.hasBearingAccuracy() + "\n";
        str += "Speed=" + location.getSpeed() +  " " + location.hasSpeed() + "\n";
        str += "Speed Accuracy=" + location.getSpeedAccuracyMetersPerSecond() +  " " + location.hasSpeedAccuracy() + "\n";

        str += "Distance=" + mDistance + "\n";
        str += "TTFF=" + ((mTTFF < 0) ? "" : mTTFF) + "\n";
        str += String.format("First Lat=%.06f Lng=%.06f\n", mFirstLocation.getLatitude()
                , mFirstLocation.getLongitude());
        str += "First distance=" + mFirstDistance + "\n";
        str += mFirstSvNumStr;
        str += mFirstSvStr;
        mTextViewLocation.setText(str);
    }

    private void updateFirstSvStr(GnssStatus gnssStatus) {
        int svNum = 0;
        mFirstSvStr = "";
        int i = 0;
        if (gnssStatus != null) {
            for (i=0; i < gnssStatus.getSatelliteCount(); i++) {
                if (gnssStatus.getCn0DbHz(i) > 0) {
                    svNum++;
                    mFirstSvStr += "SV" + gnssStatus.getSvid(i) + ":" +
                            gnssStatus.getCn0DbHz(i) + " ";
                }
            }
            mFirstSvStr += "\n";
        }
        mFirstSvNumStr = "First Satellite Num=" + svNum + "\n";
    }

    //true: success
    public boolean write2File(String folder, String subFolder, String fileName,
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

        if (subFolder != null) {
            fullFileName = fullFileName + "/" + subFolder;
            f = new File(fullFileName);
            f.mkdir();
        }

        fullFileName += "/" + fileName;
        f = new File(fullFileName);
        try {
            if (isAppendMode)
                dos = new DataOutputStream(new FileOutputStream(fullFileName, true));
            else
                dos = new DataOutputStream(new FileOutputStream(f));
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

    protected void onDestroy() {
        log("onDestroy()");
        super.onDestroy();
        mLocationManager.removeNmeaListener(mNmeaListener);
        mLocationManager.removeUpdates(mLocationListener);
        mLocationManager.unregisterGnssStatusCallback(mGnssStatusCallback);

        mTimer.stopTimer();
        mStressTest.stopStress();
        mToMnldSender.debugMnldNeMsg(mUdpClient, false);
    }

    private void log(String msg) {
        Log.d("LocationEM_Gps", msg);
    }

    /// update hasAlmanac value
    private void updateHasAlmanacValue() {
        String existence = "";
        final String almanacDate = SystemProperties.get(GPS_ALMANAC);
        if (almanacDate.equals("") || almanacDate.equals("0")) {
            existence = "No";
            mTextViewAlmanacDateValue.setText("");
        } else {
            existence = "Yes";
            mTextViewAlmanacDateValue.setText(almanacDate);
        }
        mTextViewHasAlmanacValue.setText(existence);
        log("updateHasAlmanacValue, existence = " + existence + " date=" + almanacDate);
    }

    private void updateYearOfGnssHwValue() {
        int year = mLocationManager.getGnssYearOfHardware();
        mTextViewYearOfGnssHwValue.setText("" + year);
        log("updateYearOfGnssHwValue, year = " + year);
    }

    private void updateGnssHwNameValue() {
        String hwName = mLocationManager.getGnssHardwareModelName();
        mTextViewGnssHwNameValue.setText(hwName);
        log("updateGnssHwNameValue, hwName = " + hwName);
    }

    private void setBlacklistData() {
        mBlackList = "";
        if (mCheckBoxBlockGps.isChecked() && !mTextViewBlockGps.getText().toString().isEmpty()) {
            log("Gps Blacklist: " + mTextViewBlockGps.getText().toString());
            List<Integer> parsed = parseSatelliteBlacklist(mTextViewBlockGps.getText().toString());
            int length = parsed.size();
            for (int i = 0; i < length; i++) {
                mBlackList += GnssStatus.CONSTELLATION_GPS + BLACKLIST_DELIMITER + parsed.get(i) + BLACKLIST_DELIMITER;
            }
        }
        if (mCheckBoxBlockGlonass.isChecked() && !mTextViewBlockGlonass.getText().toString().isEmpty()) {
            log("Glonass Blacklist: " + mTextViewBlockGlonass.getText().toString());
            List<Integer> parsed = parseSatelliteBlacklist(mTextViewBlockGlonass.getText().toString());
            int length = parsed.size();
            for (int i = 0; i < length; i++) {
                mBlackList += GnssStatus.CONSTELLATION_GLONASS + BLACKLIST_DELIMITER + parsed.get(i) + BLACKLIST_DELIMITER;
            }
        }
        if (mCheckBoxBlockBeidou.isChecked() && !mTextViewBlockBeidou.getText().toString().isEmpty()) {
            log("Beidou Blacklist: " + mTextViewBlockBeidou.getText().toString());
            List<Integer> parsed = parseSatelliteBlacklist(mTextViewBlockBeidou.getText().toString());
            int length = parsed.size();
            for (int i = 0; i < length; i++) {
                mBlackList += GnssStatus.CONSTELLATION_BEIDOU + BLACKLIST_DELIMITER + parsed.get(i) + BLACKLIST_DELIMITER;
            }
        }
        if (mCheckBoxBlockGalileo.isChecked() && !mTextViewBlockGalileo.getText().toString().isEmpty()) {
            log("Galileo Blacklist: " + mTextViewBlockGalileo.getText().toString());
            List<Integer> parsed = parseSatelliteBlacklist(mTextViewBlockGalileo.getText().toString());
            int length = parsed.size();
            for (int i = 0; i < length; i++) {
                mBlackList += GnssStatus.CONSTELLATION_GALILEO + BLACKLIST_DELIMITER + parsed.get(i) + BLACKLIST_DELIMITER;
            }
        }

        log("mBlacklist: " + mBlackList);
        Settings.Global.putString(this.getContentResolver(),
                Settings.Global.GNSS_SATELLITE_BLACKLIST,
                mBlackList);
    }

    private List<Integer> parseSatelliteBlacklist(String blacklist){
        String[] strings = blacklist.split(BLACKLIST_DELIMITER);
        List<Integer> parsed = new ArrayList<>(strings.length);
        for (String string : strings) {
            string = string.trim();
            if ("All".equals(string)) {
                parsed.add(0);
            } else if (!"".equals(string)) {
                int value = Integer.parseInt(string);
                if (value < 0) {
                    log("Negative value is invalid for Satellite Blacklist.");
                    break;
                }
                parsed.add(value);
            }
        }
        return parsed;
    }

    private void openBlackSvSelectDialog(int svType, String svStr) {
        final Dialog dialog = new Dialog(this);
        dialog.setTitle("Select black sv list");
        dialog.setContentView(R.layout.gps_blacksvdlg);

        List<Integer> svList = parseSatelliteBlacklist(svStr);

        Button button_ok    = (Button) dialog.findViewById(R.id.Button_BlacksvOk);
        mCheckBoxSvArr[0] = (CheckBox) dialog.findViewById(R.id.CheckBox_svAll);
        for (int i = 1; i < BLACKLIST_SV_COUNT; i++) {
            String checkBoxID = "CheckBox_sv" + i;
            int resID = getResources().getIdentifier(checkBoxID, "id", getPackageName());
            mCheckBoxSvArr[i] = (CheckBox) dialog.findViewById(resID);
        }

        boolean allItemChecked = (svList.indexOf(0) != -1);
        for (int i = 0; i < BLACKLIST_SV_COUNT; i++) {
            if (i != 0) {
                mCheckBoxSvArr[i].setEnabled(!allItemChecked);
            }
            if (svList.indexOf(i) != -1) {
                mCheckBoxSvArr[i].setChecked(true);
            }
        }
        dialog.show();
        button_ok.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                setSvOutCheckStr(svType);
                dialog.dismiss();
                setBlacklistData();
            }
        });
        mCheckBoxSvArr[0].setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                CheckBox clickedCheckBox = (CheckBox) v;
                boolean allItemChecked = clickedCheckBox.isChecked();
                for (int i = 1; i < BLACKLIST_SV_COUNT; i++) {
                    mCheckBoxSvArr[i].setEnabled(!allItemChecked);
                }
            }
        });
    }

    private void setSvOutCheckStr(int svType) {
        String svCheckStr = "";
        for (int i = 0; i < BLACKLIST_SV_COUNT; i++) {
            if (mCheckBoxSvArr[i].isChecked()) {
                svCheckStr += i + ",";
                if (i == 0) {
                    break;
                }
            }
        }

        if ("0,".equals(svCheckStr)) {
            svCheckStr = "All,";
        }

        switch (svType) {
            case GnssStatus.CONSTELLATION_GPS:
                mTextViewBlockGps.setText(svCheckStr);
                break;
            case GnssStatus.CONSTELLATION_GLONASS:
                mTextViewBlockGlonass.setText(svCheckStr);
                break;
            case GnssStatus.CONSTELLATION_BEIDOU:
                mTextViewBlockBeidou.setText(svCheckStr);
                break;
            case GnssStatus.CONSTELLATION_GALILEO:
                mTextViewBlockGalileo.setText(svCheckStr);
                break;
            }
    }

    private void showLogNmeaConfirmDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Data may be accessed by other APP. Are you sure to log nmea data?");
        builder.setCancelable(false);

        builder.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                mCheckBoxLogToSdcard.setEnabled(mCheckBoxLogNmea.isChecked());
            }
        });

        builder.setNegativeButton("No", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                mCheckBoxLogNmea.setChecked(false);
            }
        });

        AlertDialog alert = builder.create();
        alert.show();
    }

    private void showClearLogConfirmDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage("Are you sure to delete nmea log?");
        builder.setCancelable(false);

        builder.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                ClearNmeaLog();
            }
        });

        builder.setNegativeButton("No", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
            }
        });

        AlertDialog alert = builder.create();
        alert.show();
    }

    void ClearNmeaLog() {
        String fullFileName;

        fullFileName = "/sdcard/" + "nmea/";
        deleteFilePath(fullFileName);

        fullFileName = "/data/data/" + this.getPackageName() + "/" + "nmea/";
        deleteFilePath(fullFileName);
    }

    boolean deleteFilePath(String path) {
        File dirFile = new File(path);
        if (dirFile.isDirectory()) {
            for (File child : dirFile.listFiles()) {
                boolean success =  deleteFilePath(child.toString());
                if (!success) {
                   return false;
                }
            }
            log("The directory is deleted: " + path);
        }
        return dirFile.delete();
    }

}
