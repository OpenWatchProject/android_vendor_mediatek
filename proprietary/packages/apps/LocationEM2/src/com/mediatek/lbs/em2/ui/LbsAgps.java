package com.mediatek.lbs.em2.ui;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.graphics.Color;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.text.Html;
import android.text.InputFilter;
import android.text.InputType;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.PopupMenu;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.mediatek.lbs.em2.utils.AgpsConfig;
import com.mediatek.lbs.em2.utils.AgpsInterface;
import com.mediatek.lbs.em2.utils.AgpsProfilingInfo;
import com.mediatek.lbs.em2.utils.AgpsProfilingInfo.ProfileElement;
import com.mediatek.lbs.em2.utils.AgpsSetting;
import com.mediatek.lbs.em2.utils.CpSetting;
import com.mediatek.lbs.em2.utils.MtkAgpsXmlParser;
import com.mediatek.lbs.em2.utils.SuplProfile;
import com.mediatek.lbs.em2.utils.UpSetting;

public class LbsAgps extends Activity {

    private static final String LOCATION_PERMISSION_NAME =
            "android.permission.ACCESS_FINE_LOCATION";
    private static final String BACKGROUND_LOCATION_PERMISSION_NAME =
            "android.permission.ACCESS_BACKGROUND_LOCATION";
    private static final String TAG = "LocationEM";
    protected static Context sContext;
    protected boolean mIsKValueUnit = true;
    protected MtkAgpsXmlParser mXml;
    protected AgpsInterface mAgpsInterface;
    protected boolean mAgpsdExisted = false;
    private PackageManager mPackageManager;
    private String mGnssProxyPackageName;

    protected final static byte[] SUPL_INIT_1_MSA = { 0x0, 0x10, 0x1, 0x0, 0x0,
            0x40, 0x0, 0x0, 0x0, 0x46, 0x5, 0x40, 0x1, 0x18, 0x0, 0x0 };
    protected final static byte[] SUPL_INIT_2_MSA = { 0x0, 0x10, 0x2, 0x0, 0x0,
            0x40, 0x0, 0x0, 0x0, 0x46, 0x5, 0x40, 0x1, 0x18, 0x0, 0x0 };
    protected final static byte[] SUPL_INIT_2_E = { 0x0, 0x29, 0x2, 0x0, 0x0,
            0x40, 0x0, 0x0, 0x0, 0x46, 0x5, 0x40, 0x1, 0x18, 0x60, 0x68, 0x0,
            0x4, 0x5, 0x0, 0x2, 0x26, 0x10, 0x45, 0x2a, 0x3, (byte) 0x8c, 0x75,
            0x7, (byte) 0x8d, 0x6e, (byte) 0xde, (byte) 0xac, (byte) 0xf9, 0x3,
            0x1a, 0x38, 0x6, (byte) 0x8d, 0x32, 0x0 };
    protected final static byte[] SUPL_INIT_WRONG = { 1, 2, 3, 4, 5 };
    private final static int SHOW_PROXY_APP_PERMISSION_DIALOG        = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        try {
            initAgpsInterface();
            mAgpsdExisted = true;
        } catch (RuntimeException e) {
            log("Failed to initAgesInterface mAgpsdExisted= " +mAgpsdExisted);
            return;
        }

        setContentView(R.layout.agps);
        sContext = getApplicationContext();
        mPackageManager = sContext.getPackageManager();
        mGnssProxyPackageName = loadProxyNameFromCarrierConfig();
        log("mGnssProxyPackageName from carrierConfig: " + mGnssProxyPackageName);

        loadXML();
        initUI();
        initPopupMenu();
        addOnClickListener();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mAgpsdExisted) {
            applyAgpsConfigToUI();
            checkLocationProxyAppPermission();
        }
    }

    private void checkLocationProxyAppPermission() {
        if (isPackageInstalled(mGnssProxyPackageName)) {
            boolean proxyAppLocationGranted = hasLocationPermission(mGnssProxyPackageName);
            log("proxyApp = " + mGnssProxyPackageName
                    + " proxyAppLocationGranted = " + proxyAppLocationGranted);
            if (!proxyAppLocationGranted) {
                sendMessage(SHOW_PROXY_APP_PERMISSION_DIALOG, 0);
            }
        } else {
            log("Skip dialog, because package is not installed: " + mGnssProxyPackageName);
        }
    }

    private void showConfirmDialog (CharSequence title, CharSequence message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setMessage(message);
        builder.setCancelable(false);

        builder.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                try {
                    Intent launchIntent = mPackageManager.getLaunchIntentForPackage(
                            mGnssProxyPackageName);
                    startActivity(launchIntent);
                } catch (Exception e) {
                    Log.e(TAG, "Failed to launch package: " + mGnssProxyPackageName);
                    launchPackageSettings(mGnssProxyPackageName);
                }
            }
        });

        builder.setNegativeButton("No", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
            }
        });

        AlertDialog alert = builder.create();
        alert.show();
    }

    private void launchPackageSettings(String packageName) {
        try {
            Intent intent = new Intent();
            intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            Uri uri = Uri.fromParts("package", packageName, null);
            intent.setData(uri);
            startActivity(intent);
        } catch (Exception e) {
            Log.e(TAG, "Failed to launch package settings: " + mGnssProxyPackageName);
            Toast.makeText(getApplicationContext(), "ERR: start settings failed!!",
                    Toast.LENGTH_LONG).show();
        }
    }

    private boolean hasLocationPermission(String pkgName) {
        return (mPackageManager.checkPermission(LOCATION_PERMISSION_NAME, pkgName)
                == PackageManager.PERMISSION_GRANTED);
    }

    private void sendDelayedMessage(int what, long delayMillis) {
        mHandler.sendEmptyMessageDelayed(what, delayMillis);
    }

    private void sendMessage(int what, int arg1) {
        Message m = new Message();
        m.what = what;
        m.arg1 = arg1;
        mHandler.sendMessage(m);
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch(msg.what) {
                case SHOW_PROXY_APP_PERMISSION_DIALOG:
                    log("handle SHOW_PROXY_APP_PERMISSION_DIALOG");
                    showConfirmDialog("MTK Non-Framework LBS",
                            "Non-Framework LBS LOCATION permission is not granted. "
                            +"Some AGPS NI test cases may be failed if it is not granted. "
                            +"Would you like to launch the APP?");
                    break;
                default:
                    break;
            }
        }
    };

    // PopupMenu
    protected PopupMenu mPopupSuplUt2;
    protected PopupMenu mPopupSuplUt3;
    protected PopupMenu mPopupSlpTemplate;
    protected PopupMenu mPopupTriggerTest;
    protected PopupMenu mPopupTestButton;
    protected PopupMenu mPopupSuplVerMinor;
    protected PopupMenu mPopupSuplVerSerInd;

    // Debug
    protected LinearLayout mLayoutDebug;
    protected Button mButtonTriggerTest;
    protected Button mButtonTestButton;
    protected Button mButtonResetAgpsd;
    protected TextView mTextViewProperty;
    protected TextView mTextViewEmulatorMode;
    protected TextView mTextLppeServiceInstalled;
    protected TextView mTextViewUncaliPresureSupported;
    protected TextView mTextViewCpLppeSupported;
    protected TextView mTextViewGnssLppeSupported;

    // AGPS profiling info
    protected TextView mTextViewProfilingInfo;

    // AGPS on/off
    protected CheckBox mCheckBoxEnableAgps;

    // UP/CP switching
    protected RadioButton mRadioButtonUP;
    protected RadioButton mRadioButtonCP;

    // UP Settings
    protected LinearLayout mLayoutUpSetting;

    // UP Settings -> CDMA Settings
    protected LinearLayout mLayoutCdmaSetting;
    protected RadioButton mRadioButtonWCDMAPrefer;
    protected RadioButton mRadioButtonCDMAForce;

    // UP Settings -> SUPL Settings
    protected RadioButton mRadioButtonMSA;
    protected RadioButton mRadioButtonMSB;
    protected RadioButton mRadioButtonNoPref;

    protected CheckBox mCheckBoxMSA;
    protected CheckBox mCheckBoxMSB;
    protected CheckBox mCheckBoxECID;
    protected CheckBox mCheckBoxOTDOA;
    protected CheckBox mCheckBoxAuto;
    protected CheckBox mCheckBoxAFLT;

    protected CheckBox mCheckBoxLPP;

    protected RadioButton mRadioButtonSUPL1;
    protected RadioButton mRadioButtonSUPL2;

    protected RadioButton mRadioButtonTLS10;
    protected RadioButton mRadioButtonTLS11;
    protected RadioButton mRadioButtonTLS12;

    protected CheckBox mCheckBoxCertVerify;
    protected CheckBox mCheckBoxUdpEnable;

    protected Button mButtonSuplUt2;
    protected TextView mTextViewSuplUt2;
    protected Button mButtonSuplUt3;
    protected TextView mTextViewSuplUt3;

    protected Button mButtonSuplVerMinor;
    protected TextView mTextViewSuplVerMinor;
    protected Button mButtonSuplVerSerInd;
    protected TextView mTextViewSuplVerSerInd;

    protected Button mButtonSlpTemplate;

    protected TextView mTextViewSlpAddr;
    protected Button mButtonSlpAddr;
    protected TextView mTextViewSlpPort;
    protected Button mButtonSlpPort;
    protected CheckBox mCheckBoxTls;

    // UP Settings -> SUPL Settings -> SUPL2.0 Settings
    protected LinearLayout mLayoutSUPL2;

    protected Button mButtonPeriodicStart;
    protected Button mButtonPeriodicAbort;

    protected Button mButtonAreaStart;
    protected Button mButtonAreaAbort;

    protected RadioButton mRadioButtonEnter;
    protected RadioButton mRadioButtonInside;
    protected RadioButton mRadioButtonOutside;
    protected RadioButton mRadioButtonLeave;

    // UP Settings -> SUPL Settings -> QoP Settings
    protected ToggleButton mButtonShowQoP;
    protected LinearLayout mLayoutQoP;
    protected RadioButton mRadioButtonKValue;
    protected RadioButton mRadioButtonMeter;

    protected TextView mTextViewHAcc;
    protected Button mButtonHAcc;
    protected TextView mTextViewVAcc;
    protected Button mButtonVAcc;
    protected TextView mTextViewLocationAge;
    protected Button mButtonLocationAge;
    protected TextView mTextViewDelay;
    protected Button mButtonDelay;

    // CP Settings
    protected LinearLayout mLayoutCpSetting;

    protected RadioButton mRadioButtonLocationEstimate;
    protected RadioButton mRadioButtonAssistanceData;

    protected CheckBox mCheckBoxExternalAddr;
    protected TextView mTextViewExternalAddr;
    protected CheckBox mCheckBoxMlcNumber;
    protected TextView mTextViewMlcNumber;

    protected CheckBox mCheckBoxCpAutoReset;
    protected CheckBox mCheckBoxEpcMolrPduEnable;

    // Feature Enabler
    protected CheckBox mCheckBoxNiAllow;
    protected CheckBox mCheckBoxRoamingAllow;
    protected CheckBox mCheckBoxSupl2File;
    protected CheckBox mCheckBoxLogSensitiveData;
    protected CheckBox mCheckBoxCertFromSdcard;
    protected CheckBox mCheckBoxAutoProfileEnable;
    protected CheckBox mCheckBoxSuplApnEnable;
    protected CheckBox mCheckBoxSyncToSlp;
    protected CheckBox mCheckBoxImsiEnable;
    protected CheckBox mCheckBoxE911GpsIconEnable;
    protected CheckBox mCheckBoxLppeHideWifiBtStatus;
    protected CheckBox mCheckBoxLppeNetworkLocationDisable;
    protected CheckBox mCheckBoxLppeCpEnable;
    protected CheckBox mCheckBoxLppeUpEnable;
    protected CheckBox mCheckBoxAospProfileEnable;
    protected CheckBox mCheckBoxNlpSettings2SuplEnable;
    protected CheckBox mCheckBoxLbsLogEnable;

    protected CheckBox mCheckBoxIngoreSiForE911;
    protected CheckBox mCheckBoxLppeCpWlanEnable;
    protected CheckBox mCheckBoxLppeCpSrnEnable;
    protected CheckBox mCheckBoxLppeCpSensorEnable;
    protected CheckBox mCheckBoxLppeCpDbhEnable;
    protected CheckBox mCheckBoxLppeUpWlanEnable;
    protected CheckBox mCheckBoxLppeUpSrnEnable;
    protected CheckBox mCheckBoxLppeUpSensorEnable;
    protected CheckBox mCheckBoxLppeUpDbhEnable;
    protected CheckBox mCheckBoxUpLppIn2g3gDisable;
    protected CheckBox mCheckBoxUpRrlpIn4gDisable;
    protected CheckBox mCheckBoxUpSiDisable;
    protected CheckBox mCheckBoxDefaultNlpEnable;
    protected CheckBox mCheckBoxForceOtdoaAssistEnable;
    protected CheckBox mCheckBoxAospPosModeEnable;
    protected CheckBox mCheckBoxPrivacyOverrideEnable;
    protected CheckBox mCheckBoxAllowNiForGpsOffEnable;
    protected RadioButton mRadioButtonIpV6;
    protected RadioButton mRadioButtonIpV4;

    protected TextView mTextViewTcpKeepAliveInterval;
    protected Button   mButtonTcpKeepAlive;
    protected TextView mTextViewLppeCrowdSourceConfident;
    protected Button   mButtonLppeCrowdSourceConfident;

    // GNSS Settings
    protected CheckBox mCheckBoxSib8Sib16Enable;
    protected CheckBox mCheckBoxGpsSatelliteEnable;
    protected CheckBox mCheckBoxGlonassSatelliteEnable;
    protected CheckBox mCheckBoxBeidouSatelliteEnable;
    protected CheckBox mCheckBoxGalileoSatelliteEnable;
    protected CheckBox mCheckBoxAgpsEnable;
    protected CheckBox mCheckBoxAglonassEnable;
    protected CheckBox mCheckBoxAbeidouEnable;
    protected CheckBox mCheckBoxAgalileoEnable;

    protected Button mButtonReset2Default;

    protected void initPopupMenu() {
        mPopupSuplUt2 = new PopupMenu(this, mButtonSuplUt2);
        for (int i = 1; i <= 60; i++) {
            mPopupSuplUt2.getMenu().add(0, i, Menu.NONE, "" + i);
        }
        mPopupSuplUt2
                .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        byte value = (byte)item.getItemId();
                        mTextViewSuplUt2.setText("" + value);
                        mAgpsInterface.setSuplUt2(value);
                        return false;
                    }
                });

        mPopupSuplUt3 = new PopupMenu(this, mButtonSuplUt3);
        for (int i = 1; i <= 60; i++) {
            mPopupSuplUt3.getMenu().add(0, i, Menu.NONE, "" + i);
        }
        mPopupSuplUt3
                .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        byte value = (byte)item.getItemId();
                        mTextViewSuplUt3.setText("" + value);
                        mAgpsInterface.setSuplUt3(value);
                        return false;
                    }
                });

        mPopupSlpTemplate = new PopupMenu(this, mButtonSlpTemplate);
        if (mXml != null) {
            for (int i = 0; i < mXml.getAgpsConfig().getSuplProfiles().size(); i++) {
                SuplProfile p = mXml.getAgpsConfig().getSuplProfiles().get(i);
                mPopupSlpTemplate.getMenu().add(0, i, Menu.NONE,
                        "[" + p.name + "]");
            }

            mPopupSlpTemplate
                    .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                        @Override
                        public boolean onMenuItemClick(MenuItem item) {
                            SuplProfile p = mXml.getAgpsConfig()
                                    .getSuplProfiles().get(item.getItemId());
                            mTextViewSlpAddr.setText(p.addr);
                            mTextViewSlpPort.setText(String.valueOf(p.port));
                            mCheckBoxTls.setChecked(p.tls);

                            updateSuplProfile();
                            return false;
                        }
                    });
        }

        mPopupTriggerTest = new PopupMenu(this, mButtonTriggerTest);
        mPopupTriggerTest.getMenu().add(0, 0, Menu.NONE, "Coder Test");
        mPopupTriggerTest.getMenu().add(0, 1, Menu.NONE, "AgpsInterface Auto UT");
        mPopupTriggerTest.getMenu().add(0, 2, Menu.NONE, "NI request");
        mPopupTriggerTest.getMenu().add(0, 3, Menu.NONE, "Test 03");
        mPopupTriggerTest.getMenu().add(0, 4, Menu.NONE, "Test 04");
        mPopupTriggerTest.getMenu().add(0, 5, Menu.NONE, "Test 05");
        mPopupTriggerTest.getMenu().add(0, 6, Menu.NONE, "Test 06");
        mPopupTriggerTest.getMenu().add(0, 7, Menu.NONE, "Test 07");
        mPopupTriggerTest
                .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        switch (item.getItemId()) {
                        case 0: // Coder Test
                            try {
                                mAgpsInterface.coderTest();
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                            break;
                        case 1: // AgpsInterface IT
                            agpsInterfaceAutoUT();
                            break;
                        case 2: // NI request
                            mAgpsInterface.setNiRequest(12345, new byte[] { 1, 2, 3, 4, 5 });
                            break;
                        case 3:
                            break;
                        case 4:
                            break;
                        case 5:
                            break;
                        case 6:
                            break;
                        case 7:
                            break;
                        }
                        // TODO
                        return false;
                    }
                });

        mPopupTestButton = new PopupMenu(this, mButtonTestButton);
        mPopupTestButton.getMenu().add(0, 0, Menu.NONE, "agps_open_gps_req");
        mPopupTestButton.getMenu().add(0, 1, Menu.NONE, "agps_close_gps_req");
        mPopupTestButton.getMenu().add(0, 2, Menu.NONE, "agps_reset_gps_req");
        mPopupTestButton.getMenu().add(0, 3, Menu.NONE, "ni_notify1 notify");
        mPopupTestButton.getMenu().add(0, 4, Menu.NONE, "ni_notify1 verify");
        mPopupTestButton.getMenu().add(0, 5, Menu.NONE, "ni_notify2 notify");
        mPopupTestButton.getMenu().add(0, 6, Menu.NONE, "ni_notify2 verify");
        mPopupTestButton.getMenu().add(0, 7, Menu.NONE, "data_conn_req");
        mPopupTestButton.getMenu().add(0, 8, Menu.NONE, "data_conn_release");
        mPopupTestButton.getMenu().add(0, 9, Menu.NONE, "md_sim_info_req");
        mPopupTestButton.getMenu().add(0, 10, Menu.NONE, "md_data_conn_state_req");
        mPopupTestButton.getMenu().add(0, 11, Menu.NONE, "lppe_capability");
        mPopupTestButton.getMenu().add(0, 12, Menu.NONE, "lppe_start_meas");
        mPopupTestButton.getMenu().add(0, 13, Menu.NONE, "lppe_stop_meas");
        mPopupTestButton.getMenu().add(0, 14, Menu.NONE, "lppe_query_meas");
        mPopupTestButton.getMenu().add(0, 15, Menu.NONE, "agps_md_huge_data");
        mPopupTestButton.getMenu().add(0, 16, Menu.NONE, "agps_open_gps_rejected");
        mPopupTestButton.getMenu().add(0, 17, Menu.NONE, "ni_notify3 notify");
        mPopupTestButton.getMenu().add(0, 18, Menu.NONE, "Test 18");
        mPopupTestButton.getMenu().add(0, 19, Menu.NONE, "Test 19");
        mPopupTestButton.getMenu().add(0, 20, Menu.NONE, "Test 20");
        mPopupTestButton.getMenu().add(0, 21, Menu.NONE, "Test 21");
        mPopupTestButton.getMenu().add(0, 22, Menu.NONE, "Test 22");
        mPopupTestButton.getMenu().add(0, 23, Menu.NONE, "Test 23");
        mPopupTestButton.getMenu().add(0, 24, Menu.NONE, "Test 24");

        mPopupTestButton
                .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        mAgpsInterface.testButton(item.getItemId());
                        return false;
                    }
                });

        mPopupSuplVerMinor = new PopupMenu(this, mButtonSuplVerMinor);
        for (int i = 0; i <= 5; i++) {
            mPopupSuplVerMinor.getMenu().add(0, i, Menu.NONE, "" + i);
        }
        mPopupSuplVerMinor
                .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        byte value = (byte)item.getItemId();
                        mTextViewSuplVerMinor.setText("" + value);
                        mAgpsInterface.setSuplVerMinor(value);
                        return false;
                    }
                });

        mPopupSuplVerSerInd = new PopupMenu(this, mButtonSuplVerSerInd);
        for (int i = 0; i <= 5; i++) {
            mPopupSuplVerSerInd.getMenu().add(0, i, Menu.NONE, "" + i);
        }
        mPopupSuplVerSerInd
                .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        byte value = (byte)item.getItemId();
                        mTextViewSuplVerSerInd.setText("" + value);
                        mAgpsInterface.setSuplVerSerInd(value);
                        return false;
                    }
                });

    }

    protected void addOnClickListener() {
        // Debug
        mButtonTriggerTest.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPopupTriggerTest.show();
            }
        });
        mButtonTestButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPopupTestButton.show();
            }
        });
        mButtonResetAgpsd.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.resetAgpsd();
                applyAgpsConfigToUI();
            }
        });
        // AGPS on/off
        mCheckBoxEnableAgps.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAgpsEnabled(mCheckBoxEnableAgps.isChecked());
            }
        });
        // UP/CP switching
        mRadioButtonUP.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setProtocol(AgpsSetting.AGPS_PROTOCOL_UP);
                setLayoutDependency();
            }
        });
        mRadioButtonCP.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setProtocol(AgpsSetting.AGPS_PROTOCOL_CP);
                setLayoutDependency();
            }
        });
        // UP Settings
        // UP Settings -> CDMA Settings
        mRadioButtonWCDMAPrefer.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setCdmaPref(UpSetting.CDMA_PREFERRED_WCDMA);
                setLayoutDependency();
            }
        });
        mRadioButtonCDMAForce.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setCdmaPref(UpSetting.CDMA_PREFERRED_CDMA_FORCE);
                setLayoutDependency();
            }
        });
        // UP Settings -> SUPL Settings
        mRadioButtonMSA.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setUpPrefMethod(UpSetting.PREF_METHOD_MSA);
            }
        });
        mRadioButtonMSB.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setUpPrefMethod(UpSetting.PREF_METHOD_MSB);
            }
        });
        mRadioButtonNoPref.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setUpPrefMethod(UpSetting.PREF_METHOD_NO_PREF);
            }
        });
        mCheckBoxMSA.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setPosTechnologyMSA(mCheckBoxMSA.isChecked());
            }
        });
        mCheckBoxMSB.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setPosTechnologyMSB(mCheckBoxMSB.isChecked());
            }
        });
        mCheckBoxECID.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setPosTechnologyECID(mCheckBoxECID.isChecked());
            }
        });
        mCheckBoxOTDOA.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setPosTechnologyOTDOA(mCheckBoxOTDOA.isChecked());
            }
        });
        mCheckBoxAuto.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setPosTechnologyAutonomous(mCheckBoxAuto.isChecked());
            }
        });
        mCheckBoxAFLT.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setPosTechnologyAFLT(mCheckBoxAFLT.isChecked());
            }
        });

        mCheckBoxLPP.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppEnable(mCheckBoxLPP.isChecked());
            }
        });

        mRadioButtonSUPL1.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setSuplVersion(1);
                setLayoutDependency();
            }
        });
        mRadioButtonSUPL2.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setSuplVersion(2);
                setLayoutDependency();
            }
        });

        mRadioButtonTLS10.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setTlsVersion(0);
                setLayoutDependency();
            }
        });
        mRadioButtonTLS11.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setTlsVersion(1);
                setLayoutDependency();
            }
        });
        mRadioButtonTLS12.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setTlsVersion(2);
                setLayoutDependency();
            }
        });

        mCheckBoxCertVerify.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setCertVerify(mCheckBoxCertVerify.isChecked());
                setLayoutDependency();
            }
        });

        mCheckBoxUdpEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setUdpEnable(mCheckBoxUdpEnable.isChecked());
                setLayoutDependency();
                if(mCheckBoxUdpEnable.isChecked()) {
                    Toast.makeText(
                            getApplicationContext(),
                            "WARNING: after you enable UDP for SUPL, please do not run CTS or you may get the failure on netstat test case",
                            Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(
                            getApplicationContext(),
                            "WARNING: To cancel the UDP binding, you need to re-start your device",
                            Toast.LENGTH_LONG).show();
                }
            }
        });

        mButtonSuplUt2.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPopupSuplUt2.show();
            }
        });

        mButtonSuplUt3.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPopupSuplUt3.show();
            }
        });

        mButtonSlpTemplate.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPopupSlpTemplate.show();
            }
        });
        mButtonSlpAddr.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showEditDialog(LbsAgps.this, "Edit SLP Address", 127,
                        InputType.TYPE_CLASS_TEXT, mTextViewSlpAddr,
                        slpAddrListener);
            }
        });
        mButtonSlpPort.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showEditDialog(LbsAgps.this, "Edit SLP Port", 5,
                        InputType.TYPE_CLASS_NUMBER, mTextViewSlpPort,
                        slpPortListener);
            }
        });
        mButtonTcpKeepAlive.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showEditDialog(LbsAgps.this, "Edit TCP Keep Alive Interval", 3,
                        InputType.TYPE_CLASS_NUMBER, mTextViewTcpKeepAliveInterval,
                        tcpKeepAliveListener);
            }
        });
        mButtonLppeCrowdSourceConfident.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showEditDialog(LbsAgps.this, "Lppe Crowd Source Confident", 3,
                        InputType.TYPE_CLASS_NUMBER, mTextViewLppeCrowdSourceConfident,
                        lppeCrowdSourceConfidentListener);
            }
        });
        mCheckBoxTls.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                updateSuplProfile();
            }
        });
        mButtonSuplVerMinor.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPopupSuplVerMinor.show();
            }
        });
        mButtonSuplVerSerInd.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPopupSuplVerSerInd.show();
            }
        });
        // UP Settings -> SUPL Settings -> SUPL2.0 Settings
        mButtonPeriodicStart.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.startPeriodic();
            }
        });
        mButtonPeriodicAbort.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.stopPeriodic();
            }
        });
        mButtonAreaStart.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.startAreaEvent(getAreaEventType());
            }
        });
        mButtonAreaAbort.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.stopAreaEvent();
            }
        });
        mRadioButtonEnter.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                // do nothing
            }
        });
        mRadioButtonInside.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                // do nothing
            }
        });
        mRadioButtonOutside.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                // do nothing
            }
        });
        mRadioButtonLeave.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                // do nothing
            }
        });
        mButtonShowQoP.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                setLayoutDependency();
            }
        });
        mRadioButtonKValue.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mIsKValueUnit == false) {
                    mIsKValueUnit = true;
                    int ret = 0;
                    ret = calcKFromMeter(Integer.valueOf((mTextViewHAcc
                            .getText().toString())));
                    mTextViewHAcc.setText("" + ret);
                    ret = calcKFromMeter(Integer.valueOf((mTextViewVAcc
                            .getText().toString())));
                    mTextViewVAcc.setText("" + ret);
                }
            }
        });
        mRadioButtonMeter.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mIsKValueUnit) {
                    mIsKValueUnit = false;
                    int ret = 0;
                    ret = calcMeterFromK(Integer.valueOf((mTextViewHAcc
                            .getText().toString())));
                    mTextViewHAcc.setText("" + ret);
                    ret = calcMeterFromK(Integer.valueOf((mTextViewVAcc
                            .getText().toString())));
                    mTextViewVAcc.setText("" + ret);
                }
            }
        });
        mButtonHAcc.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showEditDialog(LbsAgps.this, "Edit Horizontal Acuuracy", 5,
                        InputType.TYPE_CLASS_NUMBER, mTextViewHAcc,
                        hAccListener);
            }
        });
        mButtonVAcc.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showEditDialog(LbsAgps.this, "Edit Vertical Acuuracy", 5,
                        InputType.TYPE_CLASS_NUMBER, mTextViewVAcc,
                        vAccListener);
            }
        });
        mButtonLocationAge.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showEditDialog(LbsAgps.this, "Edit Location Age", 5,
                        InputType.TYPE_CLASS_NUMBER, mTextViewLocationAge,
                        locationAgeListener);
            }
        });
        mButtonDelay.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                showEditDialog(LbsAgps.this, "Edit Delay", 5,
                        InputType.TYPE_CLASS_NUMBER, mTextViewDelay,
                        delayListener);
            }
        });
        // CP Settings
        mRadioButtonLocationEstimate.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface
                        .setMolrPosMethod(CpSetting.MOLR_POS_METHOD_LOC_EST);
            }
        });
        mRadioButtonAssistanceData.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface
                        .setMolrPosMethod(CpSetting.MOLR_POS_METHOD_ASSIST_DATA);
            }
        });
        mCheckBoxExternalAddr.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mCheckBoxExternalAddr.isChecked()) {
                    showEditDialog(LbsAgps.this, "Edit External Address", 19,
                            InputType.TYPE_CLASS_TEXT, mTextViewExternalAddr,
                            externalAddrListener);
                } else {
                    mAgpsInterface.setExternalAddr(false, mTextViewExternalAddr
                            .getText().toString());
                }
            }
        });
        mCheckBoxMlcNumber.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mCheckBoxMlcNumber.isChecked()) {
                    showEditDialog(LbsAgps.this, "Edit MLC Number", 19,
                            InputType.TYPE_CLASS_TEXT, mTextViewMlcNumber,
                            mlcNumberListener);
                } else {
                    mAgpsInterface.setMlcNumber(false, mTextViewMlcNumber
                            .getText().toString());
                }
            }
        });

        mCheckBoxCpAutoReset.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setCpAutoReset(mCheckBoxCpAutoReset.isChecked());
            }
        });
        mCheckBoxEpcMolrPduEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setEpcMolrPduEnable(mCheckBoxEpcMolrPduEnable
                        .isChecked());
            }
        });
        // Feature Enabler
        mCheckBoxNiAllow.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAllowNI(mCheckBoxNiAllow.isChecked());
            }
        });
        mCheckBoxRoamingAllow.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAllowRoaming(mCheckBoxRoamingAllow
                        .isChecked());
            }
        });
        mCheckBoxSupl2File.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setSupl2file(mCheckBoxSupl2File.isChecked());
            }
        });
        mCheckBoxLogSensitiveData.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                boolean noSensitiveLog = !mCheckBoxLogSensitiveData.isChecked();
                mAgpsInterface.setNoSensitiveLog(noSensitiveLog);
            }
        });
        mCheckBoxCertFromSdcard.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setCertFromSdcard(mCheckBoxCertFromSdcard.isChecked());
                if (!mCheckBoxCertFromSdcard.isChecked()) {
                    Toast.makeText(getApplicationContext(),
                            "To disable this feature, please reboot your phone",
                            Toast.LENGTH_SHORT).show();
                }
            }
        });
        mCheckBoxAutoProfileEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAutoProfileEnable(mCheckBoxAutoProfileEnable.isChecked());
            }
        });
        mCheckBoxSuplApnEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.
                    setSuplDedicatedApnEnable(mCheckBoxSuplApnEnable.isChecked());
            }
        });
        mCheckBoxSyncToSlp.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setSyncToSlpEnable(mCheckBoxSyncToSlp.isChecked());
            }
        });
        mCheckBoxImsiEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setImsiEnable(mCheckBoxImsiEnable.isChecked());
            }
        });
        mCheckBoxE911GpsIconEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setE911GpsIconEnable(mCheckBoxE911GpsIconEnable.isChecked());
            }
        });
        mCheckBoxLppeHideWifiBtStatus.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeHideWifiBtStatus(mCheckBoxLppeHideWifiBtStatus.isChecked());
            }
        });
        mCheckBoxLppeNetworkLocationDisable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeNetworkLocationDisable(mCheckBoxLppeNetworkLocationDisable.isChecked());
            }
        });
        mCheckBoxLppeCpEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeCpEnable(mCheckBoxLppeCpEnable.isChecked());
            }
        });
        mCheckBoxLppeUpEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeUpEnable(mCheckBoxLppeUpEnable.isChecked());
            }
        });
        mCheckBoxAospProfileEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAospProfileEnable(mCheckBoxAospProfileEnable.isChecked());
            }
        });
        mCheckBoxNlpSettings2SuplEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setBindNlpSettingToSupl(mCheckBoxNlpSettings2SuplEnable.isChecked());
            }
        });
        mCheckBoxLbsLogEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLbsLogEnable(mCheckBoxLbsLogEnable.isChecked());
            }
        });

        mCheckBoxIngoreSiForE911.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setIngoreSiForE911(mCheckBoxIngoreSiForE911.isChecked());
            }
        });

        mCheckBoxLppeCpWlanEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeCpWlanEnable(mCheckBoxLppeCpWlanEnable.isChecked());
            }
        });

        mCheckBoxLppeCpSrnEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeCpSrnEnable(mCheckBoxLppeCpSrnEnable.isChecked());
            }
        });

        mCheckBoxLppeCpSensorEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeCpSensorEnable(mCheckBoxLppeCpSensorEnable.isChecked());
            }
        });

        mCheckBoxLppeCpDbhEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeCpDbhEnable(mCheckBoxLppeCpDbhEnable.isChecked());
            }
        });

        mCheckBoxLppeUpWlanEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeUpWlanEnable(mCheckBoxLppeUpWlanEnable.isChecked());
            }
        });

        mCheckBoxLppeUpSrnEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeUpSrnEnable(mCheckBoxLppeUpSrnEnable.isChecked());
            }
        });

        mCheckBoxLppeUpSensorEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeUpSensorEnable(mCheckBoxLppeUpSensorEnable.isChecked());
            }
        });

        mCheckBoxLppeUpDbhEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setLppeUpDbhEnable(mCheckBoxLppeUpDbhEnable.isChecked());
            }
        });

        mCheckBoxUpLppIn2g3gDisable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setUpLppIn2g3gDisable(mCheckBoxUpLppIn2g3gDisable.isChecked());
            }
        });

        mCheckBoxUpRrlpIn4gDisable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setUpRrlpIn4gDisable(mCheckBoxUpRrlpIn4gDisable.isChecked());
            }
        });

        mCheckBoxUpSiDisable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setUpSiDisable(mCheckBoxUpSiDisable.isChecked());
            }
        });

        mCheckBoxDefaultNlpEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setDefaultNlpEnable(mCheckBoxDefaultNlpEnable.isChecked());
            }
         });

        mCheckBoxForceOtdoaAssistEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setForceOtdoaAssistReq(mCheckBoxForceOtdoaAssistEnable.isChecked());
            }
         });

        mCheckBoxAospPosModeEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAospPosModeEnable(mCheckBoxAospPosModeEnable.isChecked());
            }
         });

        mCheckBoxPrivacyOverrideEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                /// enable all bits for testing
                mAgpsInterface.setPrivacyOverrideMode(
                    mCheckBoxPrivacyOverrideEnable.isChecked() ? 0xFFFF : 0);
            }
         });

        mCheckBoxAllowNiForGpsOffEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAllowNiForGpsOff(mCheckBoxAllowNiForGpsOffEnable.isChecked());
            }
         });

        mRadioButtonIpV6.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setIpVersionPrefer(0);
            }
        });

        mRadioButtonIpV4.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setIpVersionPrefer(1);
            }
        });

        mCheckBoxSib8Sib16Enable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setSib8Sib16Enable(mCheckBoxSib8Sib16Enable.isChecked());
            }
        });
        mCheckBoxGpsSatelliteEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mCheckBoxAgpsEnable.setEnabled(mCheckBoxGpsSatelliteEnable.isChecked());
                mAgpsInterface.setGpsSatelliteEnable(mCheckBoxGpsSatelliteEnable.isChecked());
            }
        });
        mCheckBoxGlonassSatelliteEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mCheckBoxAglonassEnable.setEnabled(mCheckBoxGlonassSatelliteEnable.isChecked());
                mAgpsInterface.setGlonassSatelliteEnable(mCheckBoxGlonassSatelliteEnable.isChecked());
            }
        });
        mCheckBoxBeidouSatelliteEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mCheckBoxAbeidouEnable.setEnabled(mCheckBoxBeidouSatelliteEnable.isChecked());
                mAgpsInterface.setBeidouSatelliteEnable(mCheckBoxBeidouSatelliteEnable.isChecked());
            }
        });
        mCheckBoxGalileoSatelliteEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mCheckBoxAgalileoEnable.setEnabled(mCheckBoxGalileoSatelliteEnable.isChecked());
                mAgpsInterface.setGalileoSatelliteEnable(mCheckBoxGalileoSatelliteEnable.isChecked());
            }
        });
        mCheckBoxAgpsEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAgpsSatelliteEnable(mCheckBoxAgpsEnable.isChecked());
            }
        });
        mCheckBoxAglonassEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAglonassSatelliteEnable(mCheckBoxAglonassEnable.isChecked());
            }
        });
        mCheckBoxAbeidouEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAbeidouSatelliteEnable(mCheckBoxAbeidouEnable.isChecked());
            }
        });
        mCheckBoxAgalileoEnable.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setAgalileoSatelliteEnable(mCheckBoxAgalileoEnable.isChecked());
            }
        });
        mButtonReset2Default.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mAgpsInterface.setResetToDefault();
                applyAgpsConfigToUI();
            }
        });
    }

    protected static void showEditDialog(Context context, String title,
            int maxTextLength, int inputType, final TextView textView,
            final DialogListener listener) {

        final String originalString = textView.getText().toString();
        final Dialog dialog = new Dialog(context);
        dialog.setTitle(title);
        dialog.setContentView(R.layout.edit);
        dialog.setCancelable(false);
        dialog.show();
        final EditText text = (EditText) dialog
                .findViewById(R.id.EditText_EditButton);
        InputFilter[] filterArray = new InputFilter[1];
        filterArray[0] = new InputFilter.LengthFilter(maxTextLength);
        text.setFilters(filterArray);
        text.setInputType(inputType);
        text.setText(originalString);

        Button buttonOk = (Button) dialog
                .findViewById(R.id.Button_EditButtonOK);
        Button buttonCancel = (Button) dialog
                .findViewById(R.id.Button_EditButtonCancel);
        buttonOk.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                if (listener == null) {
                    textView.setText(text.getText().toString());
                    dialog.dismiss();
                } else {
                    listener.onOKClick(dialog, v, text.getText().toString());
                }
            }
        });
        buttonCancel.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                if (listener == null) {
                    dialog.dismiss();
                } else {
                    listener.onCancelClick(dialog, v, originalString);
                }
            }
        });
    }

    protected void initUI() {
        // Debug
        mLayoutDebug = (LinearLayout) findViewById(R.id.LinearLayout_Debug);
        mButtonTriggerTest = (Button) findViewById(R.id.Button_TriggerTest);
        mButtonTestButton = (Button) findViewById(R.id.Button_TestButton);
        mButtonResetAgpsd = (Button) findViewById(R.id.Button_ResetAgpsd);
        mTextViewProperty = (TextView) findViewById(R.id.TextView_Property);
        mTextViewEmulatorMode = (TextView) findViewById(R.id.TextView_EmulatorMode);
        // LPPe Service info
        mTextLppeServiceInstalled = (TextView) findViewById(R.id.TextView_LppeServiceInstalledValue);
        mTextViewUncaliPresureSupported = (TextView) findViewById(R.id.TextView_UncaliPressureSupportedValue);
        mTextViewCpLppeSupported = (TextView) findViewById(R.id.TextView_CpLppeSupportedValue);
        mTextViewGnssLppeSupported = (TextView) findViewById(R.id.TextView_GnssLppeSupportedValue);
        // AGPS profiling info
        mTextViewProfilingInfo = (TextView) findViewById(R.id.TextView_ProfilingInfo);
        // AGPS on/off
        mCheckBoxEnableAgps = (CheckBox) findViewById(R.id.CheckBox_EnableAgps);
        // UP/CP switching
        mRadioButtonUP = (RadioButton) findViewById(R.id.RadioButton_Up);
        mRadioButtonCP = (RadioButton) findViewById(R.id.RadioButton_Cp);
        // UP Settings
        mLayoutUpSetting = (LinearLayout) findViewById(R.id.LinearLayout_UpSettings);
        // UP Settings -> CDMA Settings
        mLayoutCdmaSetting = (LinearLayout) findViewById(R.id.LinearLayout_CdmaSetting);
        mRadioButtonWCDMAPrefer = (RadioButton) findViewById(R.id.RadioButton_WCDMAPrefer);
        mRadioButtonCDMAForce = (RadioButton) findViewById(R.id.RadioButton_CDMAForce);
        // UP Settings -> SUPL Settings
        mRadioButtonMSA = (RadioButton) findViewById(R.id.RadioButton_MSA);
        mRadioButtonMSB = (RadioButton) findViewById(R.id.RadioButton_MSB);
        mRadioButtonNoPref = (RadioButton) findViewById(R.id.RadioButton_NoPref);

        mCheckBoxMSA = (CheckBox) findViewById(R.id.CheckBox_MSA);
        mCheckBoxMSB = (CheckBox) findViewById(R.id.CheckBox_MSB);
        mCheckBoxECID = (CheckBox) findViewById(R.id.CheckBox_ECID);
        mCheckBoxOTDOA = (CheckBox) findViewById(R.id.CheckBox_OTDOA);
        mCheckBoxAuto = (CheckBox) findViewById(R.id.CheckBox_Auto);
        mCheckBoxAFLT = (CheckBox) findViewById(R.id.CheckBox_AFLT);
        mCheckBoxLPP = (CheckBox) findViewById(R.id.CheckBox_LPP);

        mRadioButtonSUPL1 = (RadioButton) findViewById(R.id.RadioButton_supl1);
        mRadioButtonSUPL2 = (RadioButton) findViewById(R.id.RadioButton_supl2);

        mRadioButtonTLS10 = (RadioButton) findViewById(R.id.RadioButton_tls1_0);
        mRadioButtonTLS11 = (RadioButton) findViewById(R.id.RadioButton_tls1_1);
        mRadioButtonTLS12 = (RadioButton) findViewById(R.id.RadioButton_tls1_2);

        mCheckBoxCertVerify = (CheckBox) findViewById(R.id.CheckBox_certVerfiy);
        mCheckBoxUdpEnable = (CheckBox) findViewById(R.id.CheckBox_UdpEnable);

        mButtonSuplUt2 = (Button) findViewById(R.id.Button_SuplUt2);
        mTextViewSuplUt2 = (TextView) findViewById(R.id.TextView_suplUt2);
        mButtonSuplUt3 = (Button) findViewById(R.id.Button_SuplUt3);
        mTextViewSuplUt3 = (TextView) findViewById(R.id.TextView_suplUt3);

        mButtonSlpTemplate = (Button) findViewById(R.id.Button_SlpTemplate);

        mTextViewSlpAddr = (TextView) findViewById(R.id.TextView_SlpAddr);
        mButtonSlpAddr = (Button) findViewById(R.id.Button_SlpAddr);
        mTextViewSlpPort = (TextView) findViewById(R.id.TextView_SlpPort);
        mButtonSlpPort = (Button) findViewById(R.id.Button_SlpPort);
        mCheckBoxTls = (CheckBox) findViewById(R.id.CheckBox_Tls);

        mButtonSuplVerMinor = (Button) findViewById(R.id.Button_SuplVerMinor);
        mTextViewSuplVerMinor = (TextView) findViewById(R.id.TextView_suplVerMinor);
        mButtonSuplVerSerInd = (Button) findViewById(R.id.Button_SuplVerSerInd);
        mTextViewSuplVerSerInd = (TextView) findViewById(R.id.TextView_suplVerSerInd);

        // UP Settings -> SUPL Settings -> SUPL2.0 Settings
        mLayoutSUPL2 = (LinearLayout) findViewById(R.id.LinearLayout_SUPL2);

        mButtonPeriodicStart = (Button) findViewById(R.id.Button_PeriodicStart);
        mButtonPeriodicAbort = (Button) findViewById(R.id.Button_PeriodicAbort);

        mButtonAreaStart = (Button) findViewById(R.id.Button_AreaStart);
        mButtonAreaAbort = (Button) findViewById(R.id.Button_AreaAbort);

        mRadioButtonEnter = (RadioButton) findViewById(R.id.RadioButton_Enter);
        mRadioButtonInside = (RadioButton) findViewById(R.id.RadioButton_Inside);
        mRadioButtonOutside = (RadioButton) findViewById(R.id.RadioButton_Outside);
        mRadioButtonLeave = (RadioButton) findViewById(R.id.RadioButton_Leave);
        // UP Settings -> SUPL Settings -> QoP Settings
        mButtonShowQoP = (ToggleButton) findViewById(R.id.Button_ShowQoP);
        mLayoutQoP = (LinearLayout) findViewById(R.id.LinearLayout_QoP);
        mRadioButtonKValue = (RadioButton) findViewById(R.id.RadioButton_KValue);
        mRadioButtonMeter = (RadioButton) findViewById(R.id.RadioButton_Meter);

        mTextViewHAcc = (TextView) findViewById(R.id.TextView_HAcc);
        mButtonHAcc = (Button) findViewById(R.id.Button_HAcc);
        mTextViewVAcc = (TextView) findViewById(R.id.TextView_VAcc);
        mButtonVAcc = (Button) findViewById(R.id.Button_VAcc);
        mTextViewLocationAge = (TextView) findViewById(R.id.TextView_LocationAge);
        mButtonLocationAge = (Button) findViewById(R.id.Button_LocationAge);
        mTextViewDelay = (TextView) findViewById(R.id.TextView_Delay);
        mButtonDelay = (Button) findViewById(R.id.Button_Delay);
        // CP Settings
        mLayoutCpSetting = (LinearLayout) findViewById(R.id.LinearLayout_CpSettings);

        mRadioButtonLocationEstimate = (RadioButton) findViewById(R.id.RadioButton_LocationEstimate);
        mRadioButtonAssistanceData = (RadioButton) findViewById(R.id.RadioButton_AssistanceData);

        mCheckBoxExternalAddr = (CheckBox) findViewById(R.id.CheckBox_ExternalAddr);
        mTextViewExternalAddr = (TextView) findViewById(R.id.TextView_ExternalAddr);
        mCheckBoxMlcNumber = (CheckBox) findViewById(R.id.CheckBox_MlcNumber);
        mTextViewMlcNumber = (TextView) findViewById(R.id.TextView_MlcNumber);

        mCheckBoxCpAutoReset = (CheckBox) findViewById(R.id.CheckBox_CPAutoReset);
        mCheckBoxEpcMolrPduEnable = (CheckBox) findViewById(R.id.CheckBox_EpcMolrPduEnable);
        // Feature Enabler
        mCheckBoxNiAllow = (CheckBox) findViewById(R.id.CheckBox_NiAllow);
        mCheckBoxRoamingAllow = (CheckBox) findViewById(R.id.CheckBox_RoamingAllow);
        mCheckBoxSupl2File = (CheckBox) findViewById(R.id.CheckBox_Supl2File);
        mCheckBoxLogSensitiveData = (CheckBox) findViewById(R.id.CheckBox_LogSensitiveData);
        mCheckBoxCertFromSdcard = (CheckBox) findViewById(R.id.CheckBox_CertFromSdcard);
        mCheckBoxAutoProfileEnable = (CheckBox) findViewById(R.id.CheckBox_AutoProfileEnable);
        mCheckBoxSuplApnEnable = (CheckBox) findViewById(R.id.CheckBox_SuplApnEnable);
        mCheckBoxSyncToSlp = (CheckBox) findViewById(R.id.CheckBox_SyncToSlp);
        mCheckBoxImsiEnable = (CheckBox) findViewById(R.id.CheckBox_ImsiEnable);
        mCheckBoxE911GpsIconEnable = (CheckBox) findViewById(R.id.CheckBox_E911GpsIconEnable);
        mCheckBoxLppeHideWifiBtStatus = (CheckBox) findViewById(R.id.CheckBox_LppeHideWifiBtStatus);
        mCheckBoxLppeNetworkLocationDisable = (CheckBox) findViewById(R.id.CheckBox_LppeNetworkLocationDisable);
        mCheckBoxLppeCpEnable = (CheckBox) findViewById(R.id.CheckBox_LppeCpEnable);
        mCheckBoxLppeUpEnable = (CheckBox) findViewById(R.id.CheckBox_LppeUpEnable);
        mCheckBoxAospProfileEnable = (CheckBox) findViewById(R.id.CheckBox_AospProfileEnable);
        mCheckBoxNlpSettings2SuplEnable = (CheckBox) findViewById(R.id.CheckBox_NlpSettings2SuplEnable);
        mCheckBoxLbsLogEnable = (CheckBox) findViewById(R.id.CheckBox_LbsLogEnable);

        mCheckBoxIngoreSiForE911 = (CheckBox) findViewById(R.id.CheckBox_IngoreSiForE911);
        mCheckBoxLppeCpWlanEnable = (CheckBox) findViewById(R.id.CheckBox_LppeCpWlanEnable);
        mCheckBoxLppeCpSrnEnable = (CheckBox) findViewById(R.id.CheckBox_LppeCpSrnEnable);
        mCheckBoxLppeCpSensorEnable = (CheckBox) findViewById(R.id.CheckBox_LppeCpSensorEnable);
        mCheckBoxLppeCpDbhEnable = (CheckBox) findViewById(R.id.CheckBox_LppeCpDbhEnable);
        mCheckBoxLppeUpWlanEnable = (CheckBox) findViewById(R.id.CheckBox_LppeUpWlanEnable);
        mCheckBoxLppeUpSrnEnable = (CheckBox) findViewById(R.id.CheckBox_LppeUpSrnEnable);
        mCheckBoxLppeUpSensorEnable = (CheckBox) findViewById(R.id.CheckBox_LppeUpSensorEnable);
        mCheckBoxLppeUpDbhEnable = (CheckBox) findViewById(R.id.CheckBox_LppeUpDbhEnable);
        mCheckBoxUpLppIn2g3gDisable = (CheckBox) findViewById(R.id.CheckBox_UpLppIn2g3gDisable);
        mCheckBoxUpRrlpIn4gDisable = (CheckBox) findViewById(R.id.CheckBox_UpRrlpIn4gDisable); 
        mCheckBoxUpSiDisable = (CheckBox) findViewById(R.id.CheckBox_UpSiDisable);
        mCheckBoxDefaultNlpEnable = (CheckBox) findViewById(R.id.CheckBox_DefaultNlpEnable);
        mCheckBoxForceOtdoaAssistEnable = (CheckBox) findViewById(R.id.CheckBox_ForceOtdoaAssistEnable);
        mCheckBoxAospPosModeEnable = (CheckBox) findViewById(R.id.CheckBox_AospPosModeEnable);
        mCheckBoxPrivacyOverrideEnable = (CheckBox) findViewById(R.id.CheckBox_PrivacyOverrideEnable);
        mCheckBoxAllowNiForGpsOffEnable = (CheckBox) findViewById(R.id.CheckBox_AllowNiForGpsOffEnable);

        mRadioButtonIpV6 = (RadioButton) findViewById(R.id.RadioButton_Ipv6);
        mRadioButtonIpV4 = (RadioButton) findViewById(R.id.RadioButton_Ipv4);

        mTextViewTcpKeepAliveInterval = (TextView) findViewById(R.id.TextView_TcpKeepAliveInterval);
        mButtonTcpKeepAlive = (Button) findViewById(R.id.Button_TcpKeepAlive);
        mTextViewLppeCrowdSourceConfident = (TextView) findViewById(R.id.TextView_LppeCrowdSourceConfident);
        mButtonLppeCrowdSourceConfident = (Button) findViewById(R.id.Button_LppeCrowdSourceConfident);

        // GNSS Settings
        mCheckBoxSib8Sib16Enable = (CheckBox) findViewById(R.id.CheckBox_Sib8_Sib16_Enable);
        mCheckBoxGpsSatelliteEnable = (CheckBox) findViewById(R.id.CheckBox_GpsSatelliteEnable);
        mCheckBoxGlonassSatelliteEnable = (CheckBox) findViewById(R.id.CheckBox_GlonassSatelliteEnable);
        mCheckBoxBeidouSatelliteEnable = (CheckBox) findViewById(R.id.CheckBox_BeidouSatelliteEnable);
        mCheckBoxGalileoSatelliteEnable = (CheckBox) findViewById(R.id.CheckBox_GalileoSatelliteEnable);
        mCheckBoxAgpsEnable = (CheckBox) findViewById(R.id.CheckBox_AgpsEnable);
        mCheckBoxAglonassEnable = (CheckBox) findViewById(R.id.CheckBox_AglonassEnable);
        mCheckBoxAbeidouEnable = (CheckBox) findViewById(R.id.CheckBox_AbeidouEnable);
        mCheckBoxAgalileoEnable = (CheckBox) findViewById(R.id.CheckBox_AgalileoEnable);

        mButtonReset2Default = (Button) findViewById(R.id.Button_Reset2Default);

        if (isPackageInstalled("com.mediatek.location.lppe.main")) {
            mTextLppeServiceInstalled.setText("Yes");
            mTextLppeServiceInstalled.setTextColor(Color.GREEN);
        } else {
            mTextLppeServiceInstalled.setText("No");
            mTextLppeServiceInstalled.setTextColor(Color.RED);
        }

        if (isUncaliPressureSupported()) {
            mTextViewUncaliPresureSupported.setText("Yes");
            mTextViewUncaliPresureSupported.setTextColor(Color.GREEN);
        } else {
            mTextViewUncaliPresureSupported.setText("No");
            mTextViewUncaliPresureSupported.setTextColor(Color.RED);
        }
    }

    protected void updateSuplProfile() {
        mAgpsInterface.setSuplProfile(mTextViewSlpAddr.getText().toString(),
                Integer.valueOf(mTextViewSlpPort.getText().toString()),
                mCheckBoxTls.isChecked());
    }

    protected void updateQoP() {
        int hacc = Integer.valueOf(mTextViewHAcc.getText().toString());
        int vacc = Integer.valueOf(mTextViewVAcc.getText().toString());
        int locationAge = Integer.valueOf(mTextViewLocationAge.getText()
                .toString());
        int delay = Integer.valueOf(mTextViewDelay.getText().toString());
        if (!mIsKValueUnit) {
            hacc = calcKFromMeter(hacc);
            vacc = calcKFromMeter(vacc);
        }
        mAgpsInterface.setQop(hacc, vacc, locationAge, delay);
    }

    protected DialogListener slpAddrListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            // textViewSlpAddr

            if (input
                    .matches("((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))") != true
                    && input.matches("^((?:[0-9A-Fa-f]{1,4}(?::[0-9A-Fa-f]{1,4})*)?)::((?:[0-9A-Fa-f]{1,4}(?::[0-9A-Fa-f]{1,4})*)?)$") != true
                    && input.matches("([a-zA-Z0-9_\\-]{1,63}\\.)+[a-zA-Z0-9_\\-]{1,63}") != true
                    && input.equals("") != true) {
                Toast.makeText(getApplicationContext(),
                        "ERR: The SLP address must be IPv4, IPv6 or FQDN!!",
                        Toast.LENGTH_LONG).show();
                return;
            }
            mTextViewSlpAddr.setText(input);
            dialog.dismiss();
            updateSuplProfile();
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
        }
    };

    protected DialogListener slpPortListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            try {
                int port = Integer.valueOf(input);
                if (port > 65535) {
                    Toast.makeText(
                            getApplicationContext(),
                            "ERR: The SLP port value must be less than 65536!!",
                            Toast.LENGTH_SHORT).show();
                } else {
                    mTextViewSlpPort.setText(String.valueOf(port));
                    dialog.dismiss();
                    updateSuplProfile();
                }
            } catch (NumberFormatException e) {
                Toast.makeText(getApplicationContext(),
                        "ERR: Parameters error!!", Toast.LENGTH_SHORT).show();
            }
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
        }
    };

    protected DialogListener tcpKeepAliveListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            try {
                int interval = Integer.valueOf(input);
                if (interval > 65535) {
                    Toast.makeText(
                            getApplicationContext(),
                            "ERR: The TCP keep alive interval value must be less than 65536!!",
                            Toast.LENGTH_SHORT).show();
                } else {
                    mTextViewTcpKeepAliveInterval.setText(String.valueOf(interval));
                    dialog.dismiss();
                    mAgpsInterface.setTcpKeepAlive(interval);
                }
            } catch (NumberFormatException e) {
                Toast.makeText(getApplicationContext(),
                        "ERR: Parameters error!!", Toast.LENGTH_SHORT).show();
            }
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
        }
    };

    protected DialogListener lppeCrowdSourceConfidentListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            try {
                int interval = Integer.valueOf(input);
                if (interval > 100) {
                    Toast.makeText(
                            getApplicationContext(),
                            "ERR: The Lppe Crowd Source Confident must be less than 100!!",
                            Toast.LENGTH_SHORT).show();
                } else {
                    mTextViewLppeCrowdSourceConfident.setText(String.valueOf(interval));
                    dialog.dismiss();
                    mAgpsInterface.setLppeCrowdSourceConfident(interval);
                }
            } catch (NumberFormatException e) {
                Toast.makeText(getApplicationContext(),
                        "ERR: Parameters error!!", Toast.LENGTH_SHORT).show();
            }
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
        }
    };

    protected DialogListener hAccListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            try {
                int hAcc = Integer.valueOf(input);
                int hAccK = hAcc;
                if (mIsKValueUnit == false) {
                    hAccK = calcKFromMeter(hAcc);
                }
                if (hAccK > 100) {
                    Toast.makeText(getApplicationContext(),
                            "ERR: The K-value must be less than 100!!",
                            Toast.LENGTH_SHORT).show();
                    return;
                }

                mTextViewHAcc.setText(String.valueOf(hAcc));
                dialog.dismiss();
                updateQoP();

            } catch (NumberFormatException e) {
                Toast.makeText(getApplicationContext(),
                        "ERR: Parameters error!!", Toast.LENGTH_SHORT).show();
            }
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
        }
    };

    protected DialogListener vAccListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            try {
                int vAcc = Integer.valueOf(input);
                int vAccK = vAcc;
                if (mIsKValueUnit == false) {
                    vAccK = calcKFromMeter(vAcc);
                }
                if (vAccK > 100) {
                    Toast.makeText(getApplicationContext(),
                            "ERR: The K-value must be less than 100!!",
                            Toast.LENGTH_SHORT).show();
                    return;
                }

                mTextViewVAcc.setText(String.valueOf(vAcc));
                dialog.dismiss();
                updateQoP();

            } catch (NumberFormatException e) {
                Toast.makeText(getApplicationContext(),
                        "ERR: Parameters error!!", Toast.LENGTH_SHORT).show();
            }
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
        }
    };

    protected DialogListener locationAgeListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            try {
                int locationAge = Integer.valueOf(input);
                mTextViewLocationAge.setText(String.valueOf(locationAge));
                dialog.dismiss();
                updateQoP();
            } catch (NumberFormatException e) {
                Toast.makeText(getApplicationContext(),
                        "ERR: Parameters error!!", Toast.LENGTH_SHORT).show();
            }
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
        }
    };

    protected DialogListener delayListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            try {
                int delay = Integer.valueOf(input);
                if (delay == 36466) {
                    enableDebugMode(!isDebugModeEnabled());
                    applyAgpsConfigToUI();
                }
                mTextViewDelay.setText(String.valueOf(delay));
                dialog.dismiss();
                updateQoP();
            } catch (NumberFormatException e) {
                Toast.makeText(getApplicationContext(),
                        "ERR: Parameters error!!", Toast.LENGTH_SHORT).show();
            }
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
        }
    };

    protected DialogListener externalAddrListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            mTextViewExternalAddr.setText(input);
            dialog.dismiss();
            mAgpsInterface.setExternalAddr(true, input);
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
            mAgpsInterface.setExternalAddr(true, input);
        }
    };
    protected DialogListener mlcNumberListener = new DialogListener() {
        @Override
        public void onOKClick(Dialog dialog, View view, String input) {
            mTextViewMlcNumber.setText(input);
            dialog.dismiss();
            mAgpsInterface.setMlcNumber(true, input);
        }

        @Override
        public void onCancelClick(Dialog dialog, View view, String input) {
            dialog.dismiss();
            mAgpsInterface.setMlcNumber(true, input);
        }
    };

    protected void initAgpsInterface() {
        try {
            mAgpsInterface = new AgpsInterface();
        } catch (IOException e) {
            loge("agps interface connection failure");
            e.printStackTrace();
        }
    }

    protected int getAreaEventType() {
        if (mRadioButtonEnter.isChecked()) {
            return 0;
        }
        if (mRadioButtonInside.isChecked()) {
            return 1;
        }
        if (mRadioButtonOutside.isChecked()) {
            return 2;
        }
        if (mRadioButtonLeave.isChecked()) {
            return 3;
        }
        return 0;
    }

    protected void applyAgpsConfigToUI() {
        if (mAgpsInterface == null) {
            loge("agpsInterface is null");
            return;
        }

        int emulatorMode = mAgpsInterface.getEmulatorMode();
        switch (emulatorMode) {
        case AgpsInterface.EMULATOR_MODE_OFF:
            if (isDebugModeEnabled()) {
                mTextViewEmulatorMode.setText("EmulatorMode=[Off]");
                mTextViewEmulatorMode.setTextColor(0xff00ff00);
            } else {
                mTextViewEmulatorMode.setText("");
            }
            break;
        case AgpsInterface.EMULATOR_MODE_GEMINI:
            mTextViewEmulatorMode.setText("EmulatorMode=[Gemini]");
            mTextViewEmulatorMode.setTextColor(0xffff0000);
            break;
        case AgpsInterface.EMULATOR_MODE_DT:
            mTextViewEmulatorMode.setText("EmulatorMode=[Dual Talk]");
            mTextViewEmulatorMode.setTextColor(0xffff0000);
            break;
        case AgpsInterface.EMULATOR_MODE_COMPORT:
            mTextViewEmulatorMode.setText("EmulatorMode=[COMPORT]");
            mTextViewEmulatorMode.setTextColor(0xffff0000);
            break;
        default:
            break;
        }

        AgpsConfig config = mAgpsInterface.getAgpsConfigV24();
        mCheckBoxEnableAgps.setChecked(config.getAgpsSetting().agpsEnable);
        if (config.getAgpsSetting().agpsProtocol == AgpsSetting.AGPS_PROTOCOL_UP) {
            mRadioButtonUP.setChecked(true);
        } else {
            mRadioButtonCP.setChecked(true);
        }

        if (config.getUpSetting().cdmaPreferred == UpSetting.CDMA_PREFERRED_WCDMA) {
            mRadioButtonWCDMAPrefer.setChecked(true);
        } else if (config.getUpSetting().cdmaPreferred == UpSetting.CDMA_PREFERRED_CDMA) {
            mRadioButtonWCDMAPrefer.setChecked(true);
        } else {
            mRadioButtonCDMAForce.setChecked(true);
        }

        if (config.getUpSetting().prefMethod == UpSetting.PREF_METHOD_MSA) {
            mRadioButtonMSA.setChecked(true);
        } else if (config.getUpSetting().prefMethod == UpSetting.PREF_METHOD_MSB) {
            mRadioButtonMSB.setChecked(true);
        } else {
            mRadioButtonNoPref.setChecked(true);
        }

        mCheckBoxMSA.setChecked(config.getUpSetting().msaEnable);
        mCheckBoxMSB.setChecked(config.getUpSetting().msbEnable);
        mCheckBoxECID.setChecked(config.getUpSetting().ecidEnable);
        mCheckBoxOTDOA.setChecked(config.getUpSetting().otdoaEnable);
        mCheckBoxAuto.setChecked(config.getUpSetting().autonomousEnable);
        mCheckBoxAFLT.setChecked(config.getUpSetting().afltEnable);
        mCheckBoxLPP.setChecked(config.getUpSetting().lppEnable);

        if (config.getUpSetting().suplVersion == 1) {
            mRadioButtonSUPL1.setChecked(true);
        } else {
            mRadioButtonSUPL2.setChecked(true);
        }

        if (config.getUpSetting().tlsVersion == 0) {
            mRadioButtonTLS10.setChecked(true);
        } else if (config.getUpSetting().tlsVersion == 1) {
            mRadioButtonTLS11.setChecked(true);
        } else {
            mRadioButtonTLS12.setChecked(true);
        }

        mCheckBoxCertVerify.setChecked(config.getUpSetting().caEnable);
        mCheckBoxUdpEnable.setChecked(config.getUpSetting().udpEnable);

        mTextViewSuplUt2.setText(String.valueOf(config.getUpSetting().ut2));
        mTextViewSuplUt3.setText(String.valueOf(config.getUpSetting().ut3));

        mTextViewSuplVerMinor.setText(String.valueOf(config.getUpSetting().suplVerMinor));
        mTextViewSuplVerSerInd.setText(String.valueOf(config.getUpSetting().suplVerSerInd));

        mTextViewSlpAddr.setText(config.getCurSuplProfile().addr);
        mTextViewSlpPort
                .setText(String.valueOf(config.getCurSuplProfile().port));
        mCheckBoxTls.setChecked(config.getCurSuplProfile().tls);

        int qopHacc = config.getUpSetting().qopHacc;
        int qopVacc = config.getUpSetting().qopVacc;
        if (mRadioButtonMeter.isChecked()) {
            qopHacc = calcMeterFromK(qopHacc);
            qopVacc = calcMeterFromK(qopVacc);
        }
        mTextViewHAcc.setText(String.valueOf(qopHacc));
        mTextViewVAcc.setText(String.valueOf(qopVacc));

        mTextViewLocationAge
                .setText(String.valueOf(config.getUpSetting().qopLocAge));
        mTextViewDelay.setText(String.valueOf(config.getUpSetting().qopDelay));

        if (config.getCpSetting().molrPosMethod == CpSetting.MOLR_POS_METHOD_LOC_EST) {
            mRadioButtonLocationEstimate.setChecked(true);
        } else {
            mRadioButtonAssistanceData.setChecked(true);
        }
        mCheckBoxExternalAddr
                .setChecked(config.getCpSetting().externalAddrEnable);
        mTextViewExternalAddr.setText(config.getCpSetting().externalAddr);
        mCheckBoxMlcNumber.setChecked(config.getCpSetting().mlcNumberEnable);
        mTextViewMlcNumber.setText(config.getCpSetting().mlcNumber);
        mCheckBoxCpAutoReset.setChecked(config.getCpSetting().cpAutoReset);
        mCheckBoxEpcMolrPduEnable
                .setChecked(config.getCpSetting().epcMolrLppPayloadEnable);

        mCheckBoxNiAllow.setChecked(config.getUpSetting().niRequest);
        mCheckBoxRoamingAllow.setChecked(config.getUpSetting().roaming);
        mCheckBoxSupl2File.setChecked(config.getUpSetting().suplLog);
        mCheckBoxLogSensitiveData.setChecked(!config.getUpSetting().noSensitiveLog);
        mCheckBoxCertFromSdcard.setChecked(config.getUpSetting().certFromSdcard);
        mCheckBoxAutoProfileEnable.setChecked(config.getUpSetting().autoProfileEnable);
        mCheckBoxSuplApnEnable.setChecked(config.getUpSetting().apnEnable);
        mCheckBoxSyncToSlp.setChecked(config.getUpSetting().syncToslp);
        mCheckBoxImsiEnable.setChecked(config.getUpSetting().imsiEnable);
        mCheckBoxE911GpsIconEnable.setChecked(config.getAgpsSetting().e911GpsIconEnable);
        mCheckBoxLppeHideWifiBtStatus.setChecked(config.getAgpsSetting().lppeHideWifiBtStatus);
        mCheckBoxLppeNetworkLocationDisable.setChecked(config.getAgpsSetting().lppeNetworkLocationDisable);
        mCheckBoxLppeCpEnable.setChecked(config.getCpSetting().cpLppeEnable);
        mCheckBoxLppeUpEnable.setChecked(config.getUpSetting().upLppeEnable);
        mCheckBoxAospProfileEnable.setChecked(config.getUpSetting().aospProfileEnable);
        mCheckBoxNlpSettings2SuplEnable.setChecked(config.getUpSetting().bindNlpSettingToSupl);
        mCheckBoxLbsLogEnable.setChecked(config.getAgpsSetting().lbsLogEnable);

        mCheckBoxIngoreSiForE911.setChecked(config.getAgpsSetting().ignoreSiForE911);
        mCheckBoxLppeCpWlanEnable.setChecked(config.getCpSetting().cpLppeWlanEnable);
        mCheckBoxLppeCpSrnEnable.setChecked(config.getCpSetting().cpLppeSrnEnable);
        mCheckBoxLppeCpSensorEnable.setChecked(config.getCpSetting().cpLppeSensorEnable);
        mCheckBoxLppeCpDbhEnable.setChecked(config.getCpSetting().cpLppeDbhEnable);
        mCheckBoxLppeUpWlanEnable.setChecked(config.getUpSetting().upLppeWlanEnable);
        mCheckBoxLppeUpSrnEnable.setChecked(config.getUpSetting().upLppeSrnEnable);
        mCheckBoxLppeUpSensorEnable.setChecked(config.getUpSetting().upLppeSensorEnable);
        mCheckBoxLppeUpDbhEnable.setChecked(config.getUpSetting().upLppeDbhEnable);
        mCheckBoxUpLppIn2g3gDisable.setChecked(config.getUpSetting().upLppIn2g3gDisable);
        mCheckBoxUpRrlpIn4gDisable.setChecked(config.getUpSetting().upRrlpIn4gDisable);
        mCheckBoxUpSiDisable.setChecked(config.getUpSetting().upSiDisable);
        mCheckBoxDefaultNlpEnable.setChecked(config.getAgpsSetting().defaultNlpEnable);
        mCheckBoxForceOtdoaAssistEnable.setChecked(config.getUpSetting().forceOtdoaAssistReq);
        mCheckBoxAospPosModeEnable.setChecked(config.getUpSetting().aospPosModeEnable);
        mCheckBoxPrivacyOverrideEnable.setChecked(config.getUpSetting().privacyOverrideMode > 0);
        mCheckBoxAllowNiForGpsOffEnable.setChecked(config.getUpSetting().allowNiForGpsOff);

        if (config.getUpSetting().ipVersionPrefer == 0) {
            mRadioButtonIpV6.setChecked(true);
        } else {
            mRadioButtonIpV4.setChecked(true);
        }

        mTextViewTcpKeepAliveInterval.setText(String.valueOf(config.getUpSetting().tcpKeepAlive));
        mTextViewLppeCrowdSourceConfident.setText(
                String.valueOf(config.getAgpsSetting().lppeCrowdSourceConfident));
        mCheckBoxSib8Sib16Enable.setChecked(config.getGnssSetting().sib8sib16Enable);

        if (!config.getGnssSetting().glonassSatelliteSupport) {
            mCheckBoxGlonassSatelliteEnable.setVisibility(View.GONE);
            mCheckBoxAglonassEnable.setVisibility(View.GONE);
        }
        if (!config.getGnssSetting().beidousSatelliteSupport) {
            mCheckBoxBeidouSatelliteEnable.setVisibility(View.GONE);
            mCheckBoxAbeidouEnable.setVisibility(View.GONE);
        }
        if (!config.getGnssSetting().galileoSatelliteSupport) {
            mCheckBoxGalileoSatelliteEnable.setVisibility(View.GONE);
            mCheckBoxAgalileoEnable.setVisibility(View.GONE);
        }

        mCheckBoxGpsSatelliteEnable.setChecked(config.getGnssSetting().gpsSatelliteEnable);
        mCheckBoxGlonassSatelliteEnable.setChecked(config.getGnssSetting().glonassSatelliteEnable);
        mCheckBoxBeidouSatelliteEnable.setChecked(config.getGnssSetting().beidouSatelliteEnable);
        mCheckBoxGalileoSatelliteEnable.setChecked(config.getGnssSetting().galileoSatelliteEnable);

        mCheckBoxAgpsEnable.setChecked(config.getGnssSetting().aGpsSatelliteEnable);
        mCheckBoxAgpsEnable.setEnabled(mCheckBoxGpsSatelliteEnable.isChecked());
        mCheckBoxAglonassEnable.setChecked(config.getGnssSetting().aGlonassSatelliteEnable);
        mCheckBoxAglonassEnable.setEnabled(mCheckBoxGlonassSatelliteEnable.isChecked());
        mCheckBoxAbeidouEnable.setChecked(config.getGnssSetting().aBeidouSatelliteEnable);
        mCheckBoxAbeidouEnable.setEnabled(mCheckBoxBeidouSatelliteEnable.isChecked());
        mCheckBoxAgalileoEnable.setChecked(config.getGnssSetting().aGalileoSatelliteEnable);
        mCheckBoxAgalileoEnable.setEnabled(mCheckBoxGalileoSatelliteEnable.isChecked());

        if (config.getCpSetting().cpLppeSupport) {
            mTextViewCpLppeSupported.setText("Yes");
            mTextViewCpLppeSupported.setTextColor(Color.GREEN);
        } else {
            mTextViewCpLppeSupported.setText("No");
            mTextViewCpLppeSupported.setTextColor(Color.RED);
        }

        if (config.getGnssSetting().lppeSupport) {
            mTextViewGnssLppeSupported.setText("Yes");
            mTextViewGnssLppeSupported.setTextColor(Color.GREEN);
        } else {
            mTextViewGnssLppeSupported.setText("No");
            mTextViewGnssLppeSupported.setTextColor(Color.RED);
        }

        setLayoutDependency();

        showAgpsProfilingInfo();
    }

    protected void showAgpsProfilingInfo() {
        AgpsProfilingInfo info = mAgpsInterface.getAgpsProfilingInfo();
        log(info);
        String output = "";
        for (ProfileElement element : info.getElements()) {
            String color;
            if (element.getType() == ProfileElement.PROFILING_TYPE_NORMAL) {
                color = "c0c0c0";
            } else if (element.getType() == ProfileElement.PROFILING_TYPE_WARNING) {
                color = "ffff00";
            } else if (element.getType() == ProfileElement.PROFILING_TYPE_ERROR) {
                color = "ff0000";
            } else {
                color = "0000ff";
            }
            output += "<font color='#" + color + "'>"
                    + getTimeString(element.getTimestamp()) + " "
                    + element.getMessage() + "</font><br>";
        }
        mTextViewProfilingInfo.setText(Html.fromHtml(output),
                TextView.BufferType.SPANNABLE);

    }

    protected void setLayoutDependency() {
        boolean debugModeEnabled = isDebugModeEnabled();

        setVisibility(mLayoutDebug, debugModeEnabled);
        setVisibility(mLayoutCdmaSetting, true);
        setVisibility(mLayoutUpSetting, mRadioButtonUP.isChecked());
        setVisibility(mLayoutCpSetting, mRadioButtonCP.isChecked());
        setVisibility(mLayoutSUPL2, mRadioButtonSUPL2.isChecked());
        setVisibility(mLayoutQoP, mButtonShowQoP.isChecked());

        setVisibility(mCheckBoxGpsSatelliteEnable, debugModeEnabled);
        setVisibility(mCheckBoxGlonassSatelliteEnable, debugModeEnabled);
        setVisibility(mCheckBoxBeidouSatelliteEnable, debugModeEnabled);
        setVisibility(mCheckBoxGalileoSatelliteEnable, debugModeEnabled);
        setVisibility(mCheckBoxLppeHideWifiBtStatus, debugModeEnabled);
    }

    protected void setVisibility(View layout, boolean visible) {
        if (visible) {
            layout.setVisibility(View.VISIBLE);
        } else {
            layout.setVisibility(View.GONE);
        }
    }

    protected void loadXML() {
        try {
            try {
                mXml = new MtkAgpsXmlParser();
            } catch (FileNotFoundException e) {
                mXml = new MtkAgpsXmlParser("/vendor/etc/agps_profiles_conf2.xml");
            }
        } catch (IOException e) {
            loge("cannot load the XML file");
            e.printStackTrace();
        }
        // Do not log the xml parser log, since it doesn't show full correct values
        // log(mXml);
    }

    protected void enableDebugMode(boolean enabled) {
        File file = new File(getFilesDir() + "/../show");
        if (enabled) {
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            file.delete();
        }
    }

    protected boolean isDebugModeEnabled() {
        File file = new File(getFilesDir() + "/../show");
        return file.exists();
    }

    protected boolean isNetworkAvailable() {
        ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = cm.getActiveNetworkInfo();
        if (networkInfo != null && networkInfo.isConnected()) {
            return true;
        }
        return false;
    }

    protected static int calcMeterFromK(int k) {
        double meter;
        if (k == 1)
            return 1;
        meter = (Math.pow(1.1, k) - 1) * 10;
        return (int) Math.ceil(meter);
    }

    protected static int calcKFromMeter(int meter) {
        double k;
        k = Math.log((double) meter / 10 + 1) / Math.log(1.1);
        return (int) Math.floor(k);
    }

    // Auto UI
    protected void agpsInterfaceAutoUT() {
        // test 1: setResetToDefault, getAgpsConfig
        mAgpsInterface.setResetToDefault();
        AgpsConfig c1 = mAgpsInterface.getAgpsConfig();
        AgpsConfig c2 = mXml.getAgpsConfig();
        compareAgpsConfig(c1, c2);

        // test 2: getSuplProfile
        compareSuplProfile(c1.curSuplProfile, mAgpsInterface.getSuplProfile());

        // test 3: setAgpsEnabled
        mAgpsInterface.setAgpsEnabled(false);
        c1.agpsSetting.agpsEnable = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setAgpsEnabled(true);
        c1.agpsSetting.agpsEnable = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 4: setAllowNI
        mAgpsInterface.setAllowNI(false);
        c1.upSetting.niRequest = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setAllowNI(true);
        c1.upSetting.niRequest = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 5: setAllowRoaming
        mAgpsInterface.setAllowRoaming(false);
        c1.upSetting.roaming = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setAllowRoaming(true);
        c1.upSetting.roaming = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 6: setCdmaPref
        mAgpsInterface.setCdmaPref(UpSetting.CDMA_PREFERRED_WCDMA);
        c1.upSetting.cdmaPreferred = UpSetting.CDMA_PREFERRED_WCDMA;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setCdmaPref(UpSetting.CDMA_PREFERRED_CDMA);
        c1.upSetting.cdmaPreferred = UpSetting.CDMA_PREFERRED_CDMA;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setCdmaPref(UpSetting.CDMA_PREFERRED_CDMA_FORCE);
        c1.upSetting.cdmaPreferred = UpSetting.CDMA_PREFERRED_CDMA_FORCE;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 7: setCpAutoReset
        mAgpsInterface.setCpAutoReset(false);
        c1.cpSetting.cpAutoReset = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setCpAutoReset(true);
        c1.cpSetting.cpAutoReset = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 8: setExternalAddr
        mAgpsInterface.setExternalAddr(false, "123");
        c1.cpSetting.externalAddrEnable = false;
        c1.cpSetting.externalAddr = "123";
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setExternalAddr(true, "abc");
        c1.cpSetting.externalAddrEnable = true;
        c1.cpSetting.externalAddr = "abc";
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 9: setMlcNumber
        mAgpsInterface.setMlcNumber(false, "123");
        c1.cpSetting.mlcNumberEnable = false;
        c1.cpSetting.mlcNumber = "123";
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setMlcNumber(true, "abc");
        c1.cpSetting.mlcNumberEnable = true;
        c1.cpSetting.mlcNumber = "abc";
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 10: setMolrPosMethdo
        mAgpsInterface.setMolrPosMethod(CpSetting.MOLR_POS_METHOD_LOC_EST);
        c1.cpSetting.molrPosMethod = CpSetting.MOLR_POS_METHOD_LOC_EST;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setMolrPosMethod(CpSetting.MOLR_POS_METHOD_ASSIST_DATA);
        c1.cpSetting.molrPosMethod = CpSetting.MOLR_POS_METHOD_ASSIST_DATA;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 11: setPosTechnologyECID
        mAgpsInterface.setPosTechnologyECID(false);
        c1.upSetting.ecidEnable = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setPosTechnologyECID(true);
        c1.upSetting.ecidEnable = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 12: setPosTechnologyMSA
        mAgpsInterface.setPosTechnologyMSA(false);
        c1.upSetting.msaEnable = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setPosTechnologyMSA(true);
        c1.upSetting.msaEnable = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 13: setPosTechnologyMSB
        mAgpsInterface.setPosTechnologyMSB(false);
        c1.upSetting.msbEnable = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setPosTechnologyMSB(true);
        c1.upSetting.msbEnable = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 14: setPosTechnologyOTDOA
        mAgpsInterface.setPosTechnologyOTDOA(false);
        c1.upSetting.otdoaEnable = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setPosTechnologyOTDOA(true);
        c1.upSetting.otdoaEnable = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 15: setProtocol
        mAgpsInterface.setProtocol(AgpsSetting.AGPS_PROTOCOL_CP);
        c1.agpsSetting.agpsProtocol = AgpsSetting.AGPS_PROTOCOL_CP;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setProtocol(AgpsSetting.AGPS_PROTOCOL_UP);
        c1.agpsSetting.agpsProtocol = AgpsSetting.AGPS_PROTOCOL_UP;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 16: setQop
        mAgpsInterface.setQop(1, 2, 3, 4);
        c1.upSetting.qopHacc = 1;
        c1.upSetting.qopVacc = 2;
        c1.upSetting.qopLocAge = 3;
        c1.upSetting.qopDelay = 4;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 17: setSupl2file
        mAgpsInterface.setSupl2file(false);
        c1.upSetting.suplLog = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setSupl2file(true);
        c1.upSetting.suplLog = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 18: setSuplProfile(profile)
        SuplProfile profile = new SuplProfile();
        profile.name = "abc";
        profile.addr = "www.hugo.com";
        profile.port = 12345;
        profile.tls = true;
        profile.mccMnc = "12345";
        profile.appId = "appId";
        profile.providerId = "providerID";
        profile.defaultApn = "default";
        profile.optionalApn = "opt1";
        profile.optionalApn2 = "opt2";
        profile.addressType = "addrType";

        mAgpsInterface.setSuplProfile(profile);
        c1.curSuplProfile = profile;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        profile.name = "abc1";
        profile.addr = "www.hugo.com1";
        profile.port = 1234;
        profile.tls = false;
        profile.mccMnc = "1234";
        profile.appId = "appId1";
        profile.providerId = "providerID1";
        profile.defaultApn = "default1";
        profile.optionalApn = "opt11";
        profile.optionalApn2 = "opt21";
        profile.addressType = null;

        mAgpsInterface.setSuplProfile(profile);
        c1.curSuplProfile = profile;
        c1.curSuplProfile.addressType = "";
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 19: setSuplProfile(String addr, int port, boolean tlsEnabled)
        mAgpsInterface.setSuplProfile("abc", 444, false);
        c1.curSuplProfile.addr = "abc";
        c1.curSuplProfile.port = 444;
        c1.curSuplProfile.tls = false;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setSuplProfile("123", 555, true);
        c1.curSuplProfile.addr = "123";
        c1.curSuplProfile.port = 555;
        c1.curSuplProfile.tls = true;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 20: setSuplVersion
        mAgpsInterface.setSuplVersion(1);
        c1.upSetting.suplVersion = 1;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setSuplVersion(2);
        c1.upSetting.suplVersion = 2;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 21: setUpPrefMethod
        mAgpsInterface.setUpPrefMethod(UpSetting.PREF_METHOD_MSA);
        c1.upSetting.prefMethod = UpSetting.PREF_METHOD_MSA;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setUpPrefMethod(UpSetting.PREF_METHOD_MSB);
        c1.upSetting.prefMethod = UpSetting.PREF_METHOD_MSB;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        mAgpsInterface.setUpPrefMethod(UpSetting.PREF_METHOD_NO_PREF);
        c1.upSetting.prefMethod = UpSetting.PREF_METHOD_NO_PREF;
        compareAgpsConfig(c1, mAgpsInterface.getAgpsConfig());

        // test 22: getAgpsProfilingInfo
        mAgpsInterface.getAgpsProfilingInfo();

        // test 23: getSystemProperty
        if (!stringEquals(mAgpsInterface.getSystemProperty("abc__", "def"),
                "def")) {
            throw new RuntimeException("getSystemProperty failed");
        }

        // test 24: coderTest
        try {
            mAgpsInterface.coderTest();
        } catch (IOException e) {
            e.printStackTrace();
        }

        // test 25: getEmulatorMode
        mAgpsInterface.getEmulatorMode();

        // reset all A-GPS settings to default
        mAgpsInterface.setResetToDefault();
        applyAgpsConfigToUI();

        showToast("agpsInterfaceAutoUT test done");
    }

    /**
     * If s1.equals(s2) then return true, otherwise return false
     * */
    protected boolean stringEquals(String s1, String s2) {
        if (s1 == null && s2 == null) {
            return true;
        }
        if (s1 == null && s2 != null) {
            return false;
        }
        if (s1 != null && s2 == null) {
            return false;
        }
        return s1.equals(s2);
    }

    protected void compareSuplProfile(SuplProfile p1, SuplProfile p2) {
        if (!stringEquals(p1.name, p2.name)) {
            throw new RuntimeException("name failed [" + p1.name + "] ["
                    + p2.name + "]");
        }
        if (!stringEquals(p1.addr, p2.addr)) {
            throw new RuntimeException("addr failed [" + p1.addr + "] ["
                    + p2.addr + "]");
        }
        if (p1.port != p2.port) {
            throw new RuntimeException("port failed " + p1.port + " " + p2.port);
        }
        if (p1.tls != p2.tls) {
            throw new RuntimeException("tls failed " + p1.tls + " " + p2.tls);
        }
        if (!stringEquals(p1.mccMnc, p2.mccMnc)) {
            throw new RuntimeException("mccMnc failed [" + p1.mccMnc + "] ["
                    + p2.mccMnc + "]");
        }
        if (!stringEquals(p1.appId, p2.appId)) {
            throw new RuntimeException("appId failed [" + p1.appId + "] ["
                    + p2.appId + "]");
        }
        if (!stringEquals(p1.providerId, p2.providerId)) {
            throw new RuntimeException("providerId failed [" + p1.providerId
                    + "] [" + p2.providerId + "]");
        }
        if (!stringEquals(p1.defaultApn, p2.defaultApn)) {
            throw new RuntimeException("defaultApn failed [" + p1.defaultApn
                    + "] [" + p2.defaultApn + "]");
        }
        if (!stringEquals(p1.optionalApn, p2.optionalApn)) {
            throw new RuntimeException("optionalApn failed [" + p1.optionalApn
                    + "] [" + p2.optionalApn + "]");
        }
        if (!stringEquals(p1.optionalApn2, p2.optionalApn2)) {
            throw new RuntimeException("optionalApn2 failed ["
                    + p1.optionalApn2 + "] [" + p2.optionalApn2 + "]");
        }
        if (!stringEquals(p1.addressType, p2.addressType)) {
            throw new RuntimeException("addressType failed [" + p1.addressType
                    + "] [" + p2.addressType + "]");
        }
    }

    protected void compareAgpsSetting(AgpsSetting c1, AgpsSetting c2) {
        if (c1.agpsEnable != c2.agpsEnable) {
            throw new RuntimeException("agpsEnable failed " + c1.agpsEnable
                    + " " + c2.agpsEnable);
        }
        if (c1.agpsProtocol != c2.agpsProtocol) {
            throw new RuntimeException("agpsProtocol failed " + c1.agpsProtocol
                    + " " + c2.agpsProtocol);
        }
        if (c1.gpevt != c2.gpevt) {
            throw new RuntimeException("gpevt failed " + c1.gpevt + " "
                    + c2.gpevt);
        }
    }

    protected void compareCpSetting(CpSetting c1, CpSetting c2) {
        if (c1.molrPosMethod != c2.molrPosMethod) {
            throw new RuntimeException("molrPosMethod failed "
                    + c1.molrPosMethod + " " + c2.molrPosMethod);
        }
        if (c1.externalAddrEnable != c2.externalAddrEnable) {
            throw new RuntimeException("externalAddrEnable failed "
                    + c1.externalAddrEnable + " " + c2.externalAddrEnable);
        }
        if (!stringEquals(c1.externalAddr, c2.externalAddr)) {
            throw new RuntimeException("externalAddr failed ["
                    + c1.externalAddr + "] [" + c2.externalAddr + "]");
        }
        if (c1.mlcNumberEnable != c2.mlcNumberEnable) {
            throw new RuntimeException("mlcNumberEnable failed "
                    + c1.mlcNumberEnable + " " + c2.mlcNumberEnable);
        }
        if (!stringEquals(c1.mlcNumber, c2.mlcNumber)) {
            throw new RuntimeException("mlcNumber failed [" + c1.mlcNumber
                    + "] [" + c2.mlcNumber + "]");
        }
        if (c1.cpAutoReset != c2.cpAutoReset) {
            throw new RuntimeException("cpAutoReset failed " + c1.cpAutoReset
                    + " " + c2.cpAutoReset);
        }
    }

    protected void compareUpSetting(UpSetting c1, UpSetting c2) {
        // upSetting
        if (c1.caEnable != c2.caEnable) {
            throw new RuntimeException("caEnable failed " + c1.caEnable + " "
                    + c2.caEnable);
        }
        if (c1.niRequest != c2.niRequest) {
            throw new RuntimeException("niRequest failed " + c1.niRequest + " "
                    + c2.niRequest);
        }
        if (c1.roaming != c2.roaming) {
            throw new RuntimeException("roaming failed " + c1.roaming + " "
                    + c2.roaming);
        }
        if (c1.cdmaPreferred != c2.cdmaPreferred) {
            throw new RuntimeException("cdmaPreferred failed "
                    + c1.cdmaPreferred + " " + c2.cdmaPreferred);
        }
        if (c1.prefMethod != c2.prefMethod) {
            throw new RuntimeException("prefMethod failed " + c1.prefMethod
                    + " " + c2.prefMethod);
        }
        if (c1.suplVersion != c2.suplVersion) {
            throw new RuntimeException("suplVersion failed " + c1.suplVersion
                    + " " + c2.suplVersion);
        }
        if (c1.suplLog != c2.suplLog) {
            throw new RuntimeException("suplLog failed " + c1.suplLog + " "
                    + c2.suplLog);
        }
        if (c1.msaEnable != c2.msaEnable) {
            throw new RuntimeException("msaEnable failed " + c1.msaEnable + " "
                    + c2.msaEnable);
        }
        if (c1.msbEnable != c2.msbEnable) {
            throw new RuntimeException("msbEnable failed " + c1.msbEnable + " "
                    + c2.msbEnable);
        }
        if (c1.ecidEnable != c2.ecidEnable) {
            throw new RuntimeException("ecidEnable failed " + c1.ecidEnable
                    + " " + c2.ecidEnable);
        }
        if (c1.otdoaEnable != c2.otdoaEnable) {
            throw new RuntimeException("otdoaEnable failed " + c1.otdoaEnable
                    + " " + c2.otdoaEnable);
        }
        if (c1.qopHacc != c2.qopHacc) {
            throw new RuntimeException("qopHacc failed " + c1.qopHacc + " "
                    + c2.qopHacc);
        }
        if (c1.qopVacc != c2.qopVacc) {
            throw new RuntimeException("qopVacc failed " + c1.qopVacc + " "
                    + c2.qopVacc);
        }
        if (c1.qopLocAge != c2.qopLocAge) {
            throw new RuntimeException("qopLocAge failed " + c1.qopLocAge + " "
                    + c2.qopLocAge);
        }
        if (c1.qopDelay != c2.qopDelay) {
            throw new RuntimeException("qopDelay failed " + c1.qopDelay + " "
                    + c2.qopDelay);
        }
    }

    protected void compareAgpsConfig(AgpsConfig c1, AgpsConfig c2) {
        compareAgpsSetting(c1.agpsSetting, c2.agpsSetting);
        compareCpSetting(c1.cpSetting, c2.cpSetting);
        compareUpSetting(c1.upSetting, c2.upSetting);
        compareSuplProfile(c1.curSuplProfile, c2.curSuplProfile);
    }

    protected int getGnssSupports() {
        int gnssSupports = 0;
        gnssSupports |= mCheckBoxGlonassSatelliteEnable.isEnabled()?0x02:0;
        gnssSupports |= mCheckBoxBeidouSatelliteEnable.isEnabled()? 0x04:0;
        gnssSupports |= mCheckBoxGalileoSatelliteEnable.isEnabled()?0x08:0;
        return gnssSupports;
    }

    protected int getGnssEnables() {
        int gnssSupports = getGnssSupports();
        int gnssEnables = 0;
        gnssEnables |= mCheckBoxGlonassSatelliteEnable.isChecked()?0x02:0;
        gnssEnables |= mCheckBoxBeidouSatelliteEnable.isChecked()? 0x04:0;
        gnssEnables |= mCheckBoxGalileoSatelliteEnable.isChecked()?0x08:0;
        gnssEnables &= gnssSupports;
        return gnssEnables;
    }

    // utility
    protected static String getTimeString(long timeInMilliseconds) {
        Calendar cal = new GregorianCalendar();
        cal.setTimeInMillis(timeInMilliseconds);
        // String date = String.format(Locale.US,
        // "%04d-%02d-%02d %02d:%02d:%02d.%03d", cal.get(Calendar.YEAR),
        // cal.get(Calendar.MONDAY) + 1, cal.get(Calendar.DAY_OF_MONTH),
        // cal.get(Calendar.HOUR_OF_DAY), cal.get(Calendar.MINUTE),
        // cal.get(Calendar.SECOND), cal.get(Calendar.MILLISECOND));
        String date = String.format(Locale.US, "%02d:%02d:%02d.%03d",
                cal.get(Calendar.HOUR_OF_DAY), cal.get(Calendar.MINUTE),
                cal.get(Calendar.SECOND), cal.get(Calendar.MILLISECOND));
        return date;
    }

    protected static void log(Object msg) {
        Log.d(TAG, "" + msg);
    }

    protected static void logw(Object msg) {
        Log.d(TAG, "[agps] WARNING: " + msg);
    }

    protected static void loge(Object msg) {
        Log.d(TAG, "[agps] ERR: " + msg);
        showToast("ERR: " + msg);
    }

    protected static void showToast(Object msg) {
        Toast t = Toast.makeText(sContext, "" + msg, Toast.LENGTH_LONG);
        t.setGravity(Gravity.CENTER, 0, 0);
        t.show();
    }

    protected static void msleep(long millisecond) {
        try {
            Thread.sleep(millisecond);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    protected interface DialogListener {
        public void onOKClick(Dialog dialog, View view, String input);

        public void onCancelClick(Dialog dialog, View view, String input);
    }

    private boolean isPackageInstalled(String packagename) {
        try {
            mPackageManager.getPackageInfo(packagename, 0);
            return true;
        } catch (NameNotFoundException e) {
            return false;
        }
    }

    private boolean isUncaliPressureSupported() {
        SensorManager sensorManager = (SensorManager) sContext
                .getSystemService(Context.SENSOR_SERVICE);
        if (sensorManager != null) {
            if(sensorManager.getDefaultSensor(Sensor.TYPE_PRESSURE) != null) {
                return true;
            }
        }
        return false;
    }

    String loadProxyNameFromCarrierConfig() {
        CarrierConfigManager configManager = (CarrierConfigManager)
                sContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        if (configManager == null) {
            return "";
        }

        int ddSubId = SubscriptionManager.getDefaultDataSubscriptionId();
        PersistableBundle configs = SubscriptionManager.isValidSubscriptionId(ddSubId)
                ? configManager.getConfigForSubId(ddSubId) : null;
        if (configs == null) {
            Log.d(TAG, "SIM not ready, use default carrier config.");
            configs = CarrierConfigManager.getDefaultConfig();
        }

        String value = (String) configs.get(CarrierConfigManager.Gps.KEY_NFW_PROXY_APPS_STRING);
        Log.d(TAG, CarrierConfigManager.Gps.KEY_NFW_PROXY_APPS_STRING + ": " + value);
        if (value != null) {
            String[] strings = value.trim().split(" ");
            /// use first one package as proxy package
            return strings[0];
        } else {
            return "";
        }
    }

}
