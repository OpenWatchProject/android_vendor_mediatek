package com.mediatek.engineermode.npt;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;


public class NoiseProfilingActivity extends Activity implements View.OnClickListener,
        AdapterView.OnItemClickListener, CompoundButton.OnCheckedChangeListener {
    public static final String SCAN_CMD_PRE_REF_DEFAULT = "input keyevent 26";
    public static final String SCAN_CMD_PRE_NPT_DEFAULT = "input keyevent 26";
    public static final String SCAN_CMD_POST_NPT_DEFAULT = "input keyevent 26";
    private static final String TAG = "NPT/NoiseProfilingMain";
    private static final String KEY_RSSI0_THRESHOLD = "RssiThreshold0";
    private static final String KEY_RSSI0_DELTA = "RssiDelta0";
    private static final String KEY_RSSI1_THRESHOLD = "RssiThreshold1";
    private static final String KEY_RSSI1_DELTA = "RssiDelta1";
    private static final String KEY_DELAY_TIME = "DelayTime";
    private static final String KEY_NPT_PRE_SCAN_TIME = "NPT_pre_scan_time";
    private static final String KEY_SCAN_MODE = "scanMode";
    private static final String KEY_SCAN_CMD_PRE_REF = "scanCmdPreRef";
    private static final String KEY_SCAN_CMD_PRE_NPT = "scanCmdPreNpt";
    private static final String KEY_SCAN_CMD_POST_NPT = "scanCmdPostNpt";

    private static final String KEY_ANT_MODE = "antMode";
    private static final String KEY_ANT_STATUS = "antStatus";

    private static final int RSSI_THRESHOLD_DEFAULT = -100;
    private static final int RSSI_DELTA_DEFAULT = 8;
    private static final int RSSI_DELAY_TIME_DEFAULT = 10;
    private static final int NPT_PRE_SCAN_TIME_DEFAULT = 5;
    private static final int ANT_STATUS = 0;
    private static final int ANT_MODE = 0;
    private static final int SCAN_MODE_DEFAULT = 0;
    private static final int MSG_NPT_START_NOISE_PROFILING = 1;
    private static final int MSG_NW_RF_OFF = 2;
    private static final int MSG_NW_RF_ON = 3;
    private static final int MSG_NPT_UI_UPDATE = 4;
    private static final int MSG_NPT_START_TEST = 5;
    private static final int MSG_NPT_SWITCH_ANT_STATUS_DONE = 6;
    private static final int MSG_NPT_START_NOISE_PROFILING_DELAY = 7;
    private static final int MSG_NPT_RADIO_STATE_CHANGED = 8;

    private static final int DIG_WAIT_FOR_TEST = 2000;
    private static final int DIG_FINISH_TEST = 2001;
    private static final int DIG_RESULT_NAME = 2002;
    private static final int MENU_SETTINGS = 1000;
    private static final int MENU_LOADING_SETTINGS_MORE = 1002;
    private static final int REQUEST_SELECT_BIN = 1;
    private static final String[] mRatName = {"GSM", "TDSCDMA", "WCDMA", "LTE",
            "CDMA(1X)", "CDMA(EVDO)"};
    private static final String[] mLteBw = {"1.4M", "3M", "5M", "10M", "15M", "20M", "-"};
    private static final String PREF_FILE = "NoiseProfilingActivity";
    private static final String titleRef =
            "Mode,Rat," + "channel," + "DL Freq," + "UL Freq," + "Rx Bw," + "Tx Bw," +
                    "RSSI0," + "RSSIO Result," + "Threshold_RSSI0," +
                    "RSSI1," + "RSSI1 Result," + "Threshold_RSSI1," + "Times\n";
    private static final String titleNPT =
            "Mode,Rat," + "channel," + "DL Freq," + "UL Freq," + "Rx Bw," + "Tx Bw," +
                    "RSSI0," + "RSSIO result," + "Threshold_RSSI0," + "Delta_RSSI0," +
                    "Delta_RSSI0 Result," + "Threshold_delta_RSSI0," +
                    "RSSI1," + "RSSI1 result," + "Threshold_RSSI1," + "Delta_RSSI1," +
                    "Delta_RSSI1 Result," + "Threshold_delta_RSSI1," + "Times\n";
    public static int mRssiThreshold0;
    public static int mRssiDelta0;
    public static int mRssiThreshold1;
    public static int mRssiDelta1;
    public static int mDelayTime;
    public static int mNptPreScanTime;
    public static int mScanMode;

    public static int mAntMode;
    public static int mAntStatus;

    public static String mScanCmdPreRef;
    public static String mScanCmdPreNpt;
    public static String mScanCmdPostNpt;
    public static boolean mIsScanModeRef = true;
    private static int mDelayTimeCount;
    private static List<BandItem> sSelectedBandItems = new ArrayList<BandItem>();

    private static float mCurrentRssi[] = {-100, -200};
    private static BandItem mCurtItem = null;
    private static int mCurtItemIndex = -1;
    private static boolean mTestStartedStatus = false;
    private static int testItemsCount = 0;

    private static NoiseProfilingBandAdapter mRetAdapter = null;
    private static ArrayList<NoiseProfilingBandAdapter.RatSelectInfo> mRatList = new
            ArrayList<NoiseProfilingBandAdapter.RatSelectInfo>();
    private static ArrayList<NoiseProfilingResultAdapter.ResultInfo> mResultListScan = new
            ArrayList<NoiseProfilingResultAdapter.ResultInfo>();
    private static NoiseProfilingResultAdapter mResultAdapter_ref;
    private static Menu mMenu = null;
    private EditText mResultFileEt = null;
    private EditText mRssi0_threshold;
    private EditText mRssi0_delta;
    private EditText mRssi1_threshold;
    private EditText mRssi1_delta;
    private EditText mDelay_time;
    private EditText mNptPre_Scan_Time;
    private CheckBox mNptAntMode;
    private EditText mNptAntStatus;
    private Button mStart_listen;
    private Button mStop_listen;
    private Button mExit_listen;
    private ListView mRatListView;
    private ListView mResultListView;
    private RadioButton mNpt_scan_mode_ref;
    private RadioButton mNpt_scan_mode_npt;
    private EditText mNpt_scan_cmd_pre_ref;
    private EditText mNpt_scan_cmd_pre_npt;
    private EditText mNpt_scan_cmd_post_npt;
    private LinearLayout mNPTSettingsForm = null;
    private CountDownTimer timer;
    private String mBandNameLast = "";
    private String mBandNameCur = "";
    private String mSwitchAntcmd = "";
    private String mCurrentcmd = "";
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MSG_NW_RF_ON:
                    Elog.d(TAG, "RF is on");
                    Elog.d(TAG, "turn off RF...");
                    mStart_listen.setEnabled(false);
                    EmUtils.setAirplaneModeEnabled(true);
                    break;
                case MSG_NW_RF_OFF:
                    Elog.d(TAG, "RF is off");
                    mStart_listen.setEnabled(true);
                    BandConfigActivity.initBandItems(NoiseProfilingActivity.this);
                    //  load_default_configure_fime(NoiseProfilingActivity.this);
                    if (mMenu != null)
                        mMenu.setGroupEnabled(0, true);
                    break;
                case MSG_NPT_UI_UPDATE:
                    if ((mCurtItem.getmChannleValue() + mCurtItem.getmChannleScope()[1])
                            <= mCurtItem.getmChannleScope()[2]) {
                        int channel = mCurtItem.getmChannleValue() + mCurtItem
                                .getmChannleScope()[1];
                        start_noise_profiling(mCurtItem, channel);
                    } else {
                        mCurtItemIndex++;
                        if (mCurtItemIndex < sSelectedBandItems.size()) {
                            mCurtItem = sSelectedBandItems.get(mCurtItemIndex);
                            int channel = mCurtItem.getmChannleScope()[0];
                            start_noise_profiling(mCurtItem, channel);
                        } else {
                            Elog.d(TAG, "test finished");
                            if (mScanMode == 1 && mIsScanModeRef == true) {
                                mIsScanModeRef = false;
                                testItemsCount = mResultListScan.size();
                                start_test_mode();
                                break;
                            } else if (mScanMode == 1 && mIsScanModeRef == false) {
                                exec_pre_pose_npt(mScanCmdPostNpt);
                            }
                            save_to_file();
                            mTestStartedStatus = false;
                            mStart_listen.setEnabled(true);
                            mStop_listen.setEnabled(false);
                            Utils.playMediaPlayer();
                            showDialog(DIG_FINISH_TEST);
                        }
                    }
                    break;
                case MSG_NPT_START_NOISE_PROFILING:
                    if (ar.exception != null) {
                        Elog.d(TAG, "start test failed ");
                    } else {
                        Elog.d(TAG, "start test succeed");
                    }

                    final String[] result = (String[]) ar.result;
                    //+ERFSCAN: 1,-473
                    String rssi_value[] = null;
                    try {
                        Elog.d(TAG, "result --.>" + result[0]);
                        rssi_value = result[0].substring("+ERFSCAN: ".length()).split(",");
                        if (rssi_value[1].equals("1")) {
                            mCurrentRssi[0] = -150.0f;
                        } else {
                            mCurrentRssi[0] = Integer.parseInt(rssi_value[1]) / 8.0f;
                        }
                        if (rssi_value.length > 2) {
                            if (rssi_value[2].equals("1")) {
                                mCurrentRssi[1] = -150.0f;
                            } else {
                                mCurrentRssi[1] = Integer.parseInt(rssi_value[2]) / 8.0f;
                            }
                        } else {
                            mCurrentRssi[1] = 0.0f;
                        }
                    } catch (Exception e) {
                        mCurrentRssi[0] = 0.0f;
                        mCurrentRssi[1] = 0.0f;
                        Elog.e(TAG, "rssi not received");
                    }

                    updateUI();

                    if (mTestStartedStatus == false) {
                        Elog.d(TAG, "test stoped");
                        save_to_file();
                        return;
                    }
                    mHandler.sendEmptyMessageDelayed(MSG_NPT_UI_UPDATE, 200);
                    break;
                case MSG_NPT_START_TEST:
                    start_test();
                    break;
                case MSG_NPT_START_NOISE_PROFILING_DELAY:
                    sendATCommand(mCurrentcmd, MSG_NPT_START_NOISE_PROFILING);
                    break;
                case MSG_NPT_SWITCH_ANT_STATUS_DONE:
                    if (ar.exception != null) {
                        Elog.d(TAG, "switch ant status failed ");
                        EmUtils.showToast("switch ant status failed");
                    } else {
                        Elog.d(TAG, "switch ant status succeed");
                        mHandler.sendEmptyMessageDelayed(MSG_NPT_START_NOISE_PROFILING_DELAY, 200);
                    }
                    break;
                case MSG_NPT_RADIO_STATE_CHANGED:
                    int[] data = (int[]) ar.result;
                    if (data[0] == TelephonyManager.RADIO_POWER_ON) {
                        Elog.d(TAG, "RADIO_POWER_ON");
                        mHandler.sendEmptyMessage(MSG_NW_RF_ON);
                    } else if (data[0] == TelephonyManager.RADIO_POWER_OFF) {
                        Elog.d(TAG, "RADIO_POWER_OFF");
                        mHandler.sendEmptyMessage(MSG_NW_RF_OFF);
                    } else if (data[0] == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                        Elog.d(TAG, "RADIO_POWER_UNAVAILABLE");
                    }
                    break;
                default:
                    break;
            }
        }
    };
    private String mResultFilePath = "";

    public static void load_default_configure_fime(Context context) {
        String path = Environment.getExternalStorageDirectory().getPath() +
                NoiseProfilingFileSave.mNptConfigLoadPath + "/npt_input.csv";
        Elog.i(TAG, "path:" + path);
        NoiseProfilingFileSave.setNptInputFileName(path);
        int result = BandConfigActivity.initItemsFromInput();
        if (result == -1) {
            EmUtils.showToast("Default Config file not found.");
            Elog.d(TAG, "Default Config file not found");
        } else if (result == -2) {
            EmUtils.showToast("Default config file Read error");
            Elog.d(TAG, "Default config file Read error");
        } else {
            EmUtils.showToast("Default Read config file succeed");
            Elog.d(TAG, "Default config file succeed");
        }
    }

    void calcData() {
        Elog.d(TAG, "mResultListNpt.size() = " + mResultListScan.size());
        for (NoiseProfilingResultAdapter.ResultInfo testItem : mResultListScan) {
            if (testItem.getChannel().equals("-")) {
                continue;
            }
            float rssi0_delta_Threshold = Float.parseFloat(testItem.getRssi0()) -
                    NoiseProfilingActivity.mRssiThreshold0;
            float rssi1_delta_Threshold = Float.parseFloat(testItem.getRssi1()) -
                    NoiseProfilingActivity.mRssiThreshold1;
            if (rssi0_delta_Threshold <= 0) {
                testItem.setRssi0_result("pass");
            } else {
                testItem.setRssi0_result("failed");
            }
            if (rssi1_delta_Threshold <= 0) {
                testItem.setRssi1_result("pass");
            } else {
                testItem.setRssi1_result("failed");
            }
        }
    }

    void updateUIInfo(String info) {
        NoiseProfilingResultAdapter.ResultInfo mResultInfo = new
                NoiseProfilingResultAdapter.ResultInfo();
        mResultInfo.setRat(info);
        mResultInfo.setChannel("-");
        mResultInfo.setRssi0("-");
        mResultInfo.setRssi1("-");
        mResultInfo.setDl_freq("-");
        mResultInfo.setUl_freq("-");
        mResultListScan.add(mResultInfo);
        mResultListView.requestFocusFromTouch();
        // mResultListView.setSelection(mResultListScan.size() - 1);
        mResultAdapter_ref.notifyDataSetChanged();
    }

    void updateUIDelta(NoiseProfilingResultAdapter.ResultInfo item) {
        NoiseProfilingResultAdapter.ResultInfo mResultInfo = new
                NoiseProfilingResultAdapter.ResultInfo();
        mResultInfo.setRat(item.getRat());
        mResultInfo.setChannel(item.getChannel());
        mResultInfo.setRssi0(item.getRssi0_delta());
        mResultInfo.setRssi1(item.getRssi1_delta());
        mResultInfo.setDl_freq(item.getRssi0_delta_result());
        mResultInfo.setUl_freq(item.getRssi1_delta_result());
        mResultListScan.add(mResultInfo);
        mResultListView.requestFocusFromTouch();
        //  mResultListView.setSelection(mResultListScan.size() - 1);
        mResultAdapter_ref.notifyDataSetChanged();
    }

    void save_to_file() {
        calcData();
        int count = 0;
        float rssi_npt;
        float rssi_ref;
        float rssi_delat0;
        float rssi_delat1;

        NoiseProfilingFileSave.saveRatTestResult(this, titleRef, false);
        Elog.d(TAG, "save_to_file");
        if (mScanMode == 0) {
            for (NoiseProfilingResultAdapter.ResultInfo item : mResultListScan) {
                NoiseProfilingFileSave.saveRatTestResult(this, item.getSummary(), true);
            }
        } else {
            int resultListScanSize = mResultListScan.size();
            Elog.d(TAG, "resultListScanSize = " + resultListScanSize + ",testItemsCount = " +
                    testItemsCount);
            updateUIInfo("Result");
            for (int i = 0; i < resultListScanSize - 1; i++) {
                NoiseProfilingResultAdapter.ResultInfo item = mResultListScan.get(i + 1);
                if (count < testItemsCount - 1) {
                    NoiseProfilingFileSave.saveRatTestResult(this, item.getSummary(), true);
                } else {
                    if (count == (testItemsCount - 1)) {
                        NoiseProfilingFileSave.saveRatTestResult(this, "\n", true);
                        NoiseProfilingFileSave.saveRatTestResult(this, titleNPT, true);
                        count++;
                        continue;
                    }
                    rssi_npt = Float.parseFloat(item.getRssi0());
                    rssi_ref = Float.parseFloat(mResultListScan.get(count - testItemsCount + 1)
                            .getRssi0());
                    rssi_delat0 = rssi_npt - rssi_ref;

                    item.setRssi0_delta(String.valueOf(rssi_delat0));
                    item.setRssi0_delta_result(Math.abs(rssi_delat0) < mRssiDelta0 ?
                            "pass" : "failed");

                    rssi_npt = Float.parseFloat(item.getRssi1());
                    rssi_ref = Float.parseFloat(mResultListScan.get(count - testItemsCount + 1)
                            .getRssi1());
                    rssi_delat1 = rssi_npt - rssi_ref;

                    item.setRssi1_delta(String.valueOf(rssi_delat1));
                    item.setRssi1_delta_result(Math.abs(rssi_delat1) < mRssiDelta1
                            ? "pass" : "failed");
                    if (Math.abs(rssi_delat0) > mRssiDelta0 ||
                            Math.abs(rssi_delat1) > mRssiDelta1) {
                        updateUIDelta(item);
                    }
                    NoiseProfilingFileSave.saveRatTestResult(this, item.getSummaryNPT(), true);
                }
                count++;
            }
        }
    }

    void updateUI() {
        NoiseProfilingResultAdapter.ResultInfo mResultInfo = new
                NoiseProfilingResultAdapter.ResultInfo();

        if (mCurrentRssi[0] == 0.0f) {
            mResultInfo.setRat(mCurtItem.getmBandName() + "-error");
        } else {
            mResultInfo.setRat(mCurtItem.getmBandName());
        }

        mResultInfo.setChannel(mCurtItem.getmChannleValue() + "");

        mResultInfo.setRssi0(mCurrentRssi[0] + "");
        mResultInfo.setRssi1(mCurrentRssi[1] + "");

        float dl_freqp = -1;
        float up_freq = -1;
        int channel = mCurtItem.getmChannleValue();
        if (mCurtItem.getmBandType() == BandItem.BandType.BAND_LTE) {
            dl_freqp = (mCurtItem.getmDownlinkMinFreqValue() +
                    (channel - mCurtItem.getmDownlinkMinChannelValue())) / 10.0f;
            up_freq = (mCurtItem.getmUPlinkMinFreqValue() +
                    (channel - mCurtItem.getmDownlinkMinChannelValue())) / 10.0f;
        } else if (mCurtItem.getmBandType() == BandItem.BandType.BAND_GSM) {
            if ((mCurtItem.getmBandValue() == 2) && channel >= 955 && channel <= 1023) {
                dl_freqp = (9212 + (channel - 955) * 2) / 10.0f;
                up_freq = (8762 + (channel - 955) * 2) / 10.0f;
            } else {
                dl_freqp = (mCurtItem.getmDownlinkMinFreqValue()
                        + (channel - mCurtItem.getmDownlinkMinChannelValue()) * 2) / 10.0f;
                up_freq = (mCurtItem.getmUPlinkMinFreqValue()
                        + (channel - mCurtItem.getmDownlinkMinChannelValue()) * 2) / 10.0f;
            }

        } else if (mCurtItem.getmBandType() == BandItem.BandType.BAND_WCDMA ||
                mCurtItem.getmBandType() == BandItem.BandType.BAND_TD) {
            dl_freqp = (mCurtItem.getmDownlinkMinFreqValue() +
                    (channel - mCurtItem.getmDownlinkMinChannelValue()) * 2) / 10.0f;
            up_freq = (mCurtItem.getmUPlinkMinFreqValue() +
                    (channel - mCurtItem.getmDownlinkMinChannelValue()) * 2) / 10.0f;
        } else if (mCurtItem.getmBandType() == BandItem.BandType.BAND_CDMA ||
                mCurtItem.getmBandType() == BandItem.BandType.BAND_EVDO) {

            if (mCurtItem.getmBandValue() == 0) {       //class 0
                if (channel >= 1 && channel <= 799) {
                    up_freq = channel * 30 + 825000;
                    dl_freqp = channel * 30 + 870000;
                } else if (channel >= 991 && channel <= 1023) {
                    up_freq = (channel - 1023) * 30 + 825000;
                    dl_freqp = (channel - 1023) * 30 + 870000;
                } else if (channel >= 1024 && channel <= 1323) {
                    up_freq = (channel - 1024) * 30 + 815040;
                    dl_freqp = (channel - 1024) * 30 + 860040;
                }
            } else if (mCurtItem.getmBandValue() == 1) {       //class 1
                up_freq = channel * 50 + 1850000;
                dl_freqp = channel * 50 + 1930000;
            } else if (mCurtItem.getmBandValue() == 10) {
                if (channel >= 0 && channel <= 719) {
                    up_freq = channel * 25 + 806000;
                    dl_freqp = channel * 25 + 851000;
                } else if (channel >= 720 && channel <= 919) {
                    up_freq = (channel - 720) * 25 + 896000;
                    dl_freqp = (channel - 720) * 25 + 935000;
                } else {
                    up_freq = -1;
                    dl_freqp = -1;
                }
            }
            up_freq = up_freq / 1000.0f;
            dl_freqp = dl_freqp / 1000.0f;
        }
        mResultInfo.setDl_freq(dl_freqp + "");
        mResultInfo.setUl_freq(up_freq + "");

        if (mCurtItem.getmBandType() == BandItem.BandType.BAND_LTE) {
            mResultInfo.setRx_bw(mLteBw[mCurtItem.getmRxBwValue()]);
            if (mCurtItem.getmTxOnflagValueRef() == 1 && (mScanMode == 0 || (mScanMode == 1 &&
                    mIsScanModeRef == true))) {
                mResultInfo.setTx_bw(mLteBw[mCurtItem.getmTxBwValue()]);
            } else if (mCurtItem.getmTxOnflagValueNpt() == 1 && (mScanMode == 0 || (mScanMode ==
                    1 && mIsScanModeRef == false))) {
                mResultInfo.setTx_bw(mLteBw[mCurtItem.getmTxBwValue()]);
            } else {
                mResultInfo.setTx_bw(mLteBw[6]);
            }
        } else {
            mResultInfo.setRx_bw(mLteBw[6]);
            mResultInfo.setTx_bw(mLteBw[6]);
        }
        mResultInfo.setTimes(mCurtItem.getmRepeatTimesValue() + "");
        mResultInfo.setRssi0_Threshold(mRssiThreshold0 + "");
        mResultInfo.setRssi1_Threshold(mRssiThreshold1 + "");
        mResultInfo.setRssi0_delta_Threshold(mRssiDelta0 + "");
        mResultInfo.setRssi1_delta_Threshold(mRssiDelta1 + "");

        if (mScanMode == 0 || (mScanMode == 1 && mIsScanModeRef == true)) {
            mResultInfo.setScan_mode("RefScan");
        } else if (mScanMode == 1 && mIsScanModeRef == false) {
            mResultInfo.setScan_mode("NPTScan");
        }
        mResultListScan.add(mResultInfo);
        mResultAdapter_ref.notifyDataSetChanged();
        mResultListView.setSelection(mResultListScan.size() - 1);
        mResultListView.requestFocusFromTouch();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.noise_profiling);
        if (ModemCategory.isSimReady(0) || ModemCategory.isSimReady(1)) {
            EmUtils.showToast("This test item should not insert simCard!", Toast.LENGTH_LONG);
            Elog.w(TAG, "This test item should not insert simCard!");
            finish();
        }
        bindViews();
        Utils.powerInit(this);
        restoreSettingState();
        NoiseProfilingFileSave.initNptDirPATH();
        mRatList.clear();
        int modemType = ModemCategory.getModemType();
        for (int i = 0; i < mRatName.length; i++) {
            NoiseProfilingBandAdapter.RatSelectInfo mRatInfo = new NoiseProfilingBandAdapter
                    .RatSelectInfo();
            mRatInfo.setRatName(mRatName[i]);
            mRatInfo.setRatCheckState(false);
            if ((modemType == ModemCategory.MODEM_TD) && (i == 2))
                continue;
            else if ((modemType == ModemCategory.MODEM_FDD) && (i == 1))
                continue;
            mRatList.add(mRatInfo);
        }
        mRetAdapter = new NoiseProfilingBandAdapter(this, mRatList);
        mRatListView.setAdapter(mRetAdapter);
        mRatListView.setOnItemClickListener(this);

        mResultAdapter_ref = new NoiseProfilingResultAdapter(this, mResultListScan);

        mResultListView.setAdapter(mResultAdapter_ref);

        EmUtils.registerForradioStateChanged(0, mHandler, MSG_NPT_RADIO_STATE_CHANGED);

        if (EmUtils.ifAirplaneModeEnabled()) {
            Elog.d(TAG, "it is in AirplaneMode");
            EmUtils.setAirplaneModeEnabled(false);
        } else {
            Elog.d(TAG, "it is not in AirplaneMode");
            EmUtils.setAirplaneModeEnabled(true);
        }

        Utils.initMediaPlayer(this);
        Utils.mMediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer arg0) {
                Utils.mMediaPlayer.start();
                Utils.mMediaPlayer.setLooping(true);
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, MENU_SETTINGS, 0, "Settings");
        menu.add(0, MENU_LOADING_SETTINGS_MORE, 0, "Loading Configs from Browse..");
        menu.setGroupEnabled(0, false);
        mMenu = menu;
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case MENU_SETTINGS:
                Elog.d(TAG, "settings click ");
                showSettingState();
                npt_settings_show();
                break;
            case MENU_LOADING_SETTINGS_MORE:
                Elog.d(TAG, "Browses click ");
                Intent it = new Intent(Intent.ACTION_GET_CONTENT);
                it.setType("*/*");
                startActivityForResult(it, REQUEST_SELECT_BIN);
                break;
            default:
                break;
        }
        return super.onOptionsItemSelected(aMenuItem);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub
        if (requestCode == REQUEST_SELECT_BIN) {
            if (resultCode == Activity.RESULT_OK) {
                Uri uri = data.getData();
                String path = NoiseProfilingFileSave.getPathFromUri(uri);
                if (path == null) {
                    return;
                }
                Elog.i(TAG, "path:" + path);
                NoiseProfilingFileSave.setNptInputFileName(path);
                int result = BandConfigActivity.initItemsFromInput();
                if (result == -1) {
                    EmUtils.showToast("Config file not found.");
                    Elog.d(TAG, "Config file not found.");
                } else if (result == -2) {
                    Elog.d(TAG, "Read config file error");
                    EmUtils.showToast("Read config file error");
                } else {
                    Elog.d(TAG, "Read config file succeed");
                    EmUtils.showToast("Read config file succeed");
                }
                showSettingState();
            }
        }
    }

    @Override
    public void onDestroy() {
        if (timer != null) {
            timer.cancel();
            timer = null;
        }
        mTestStartedStatus = false;

        EmUtils.unregisterradioStateChanged(0);
        if (EmUtils.ifAirplaneModeEnabled()) {
            EmUtils.setAirplaneModeEnabled(false);
        }

        Utils.releaseMediaPlayer();
        //Utils.holdonWakelock(false);
        Elog.d(TAG, "onDestroy");
        super.onDestroy();
    }

    private void bindViews() {
        mStart_listen = (Button) findViewById(R.id.button_start);
        mStop_listen = (Button) findViewById(R.id.button_stop);
        mExit_listen = (Button) findViewById(R.id.button_exit_total);
        mStart_listen.setEnabled(false);
        mStop_listen.setEnabled(false);
        mRatListView = (ListView) findViewById(R.id.list);
        mResultListView = (ListView) findViewById(R.id.npt_test_result);
        mStart_listen.setOnClickListener(this);
        mStop_listen.setOnClickListener(this);
        mExit_listen.setOnClickListener(this);

        mNPTSettingsForm = (LinearLayout) getLayoutInflater().inflate(R.layout
                .noise_profiling_settings, null);

        mRssi0_threshold = (EditText) mNPTSettingsForm.findViewById(R.id.rssi0_threshold);
        mRssi0_delta = (EditText) mNPTSettingsForm.findViewById(R.id.rssi0_delta);
        mRssi1_threshold = (EditText) mNPTSettingsForm.findViewById(R.id.rssi1_threshold);
        mRssi1_delta = (EditText) mNPTSettingsForm.findViewById(R.id.rssi1_delta);
        mDelay_time = (EditText) mNPTSettingsForm.findViewById(R.id.delay_time);
        mNptPre_Scan_Time = (EditText) mNPTSettingsForm.findViewById(R.id.npt_pre_scan_time);

        mNptAntMode = (CheckBox) mNPTSettingsForm.findViewById(R.id.npt_ant_mode);
        mNptAntStatus = (EditText) mNPTSettingsForm.findViewById(R.id.npt_ant_status);

        mNpt_scan_mode_ref = (RadioButton) mNPTSettingsForm.findViewById(R.id.npt_scan_mode_ref);
        mNpt_scan_mode_npt = (RadioButton) mNPTSettingsForm.findViewById(R.id.npt_scan_mode_npt);

        mNpt_scan_cmd_pre_ref = (EditText) mNPTSettingsForm.findViewById(R.id.npt_scan_cmd_pre_ref);
        mNpt_scan_cmd_pre_npt = (EditText) mNPTSettingsForm.findViewById(R.id.npt_scan_cmd_pre_npt);
        mNpt_scan_cmd_post_npt = (EditText) mNPTSettingsForm.findViewById(R.id
                .npt_scan_cmd_post_npt);

        mNptAntMode.setOnCheckedChangeListener(this);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.button_start:
                Elog.i(TAG, "start test button click");
                init_test_items();
                //   if (settings_init() == false) {
                //    Elog.e(TAG, "settings_init failed");
                //     break;
                //  }
                if (BandConfigActivity.sGsmItems.size() == 0) {
                    EmUtils.showToast("The MD status is unknow,please reboot phone");
                    break;
                }
                if (sSelectedBandItems.size() == 0) {
                    EmUtils.showToast("You must select at least one item");
                    break;
                }

                mDelayTimeCount = mDelayTime;

                String serial = EmUtils.systemPropertyGet("gsm.serial", "");

                if (!serial.equals(""))
                    mResultFilePath = serial + "_";
                else {
                    mResultFilePath = "";
                }
                mResultFilePath += NoiseProfilingFileSave.getCurrectTime() + "_"
                        + ((mScanMode == 0) ? "ref" : "npt") + ".csv";
                Elog.i(TAG, "mResultFilePath before = " + mResultFilePath);

                if (mResultFileEt != null)
                    mResultFileEt.setText(mResultFilePath);
                showDialog(DIG_RESULT_NAME);
                break;
            case R.id.button_stop:
                if (timer != null) {
                    timer.cancel();
                    timer = null;
                }
                mTestStartedStatus = false;
                mStart_listen.setEnabled(true);
                mStop_listen.setEnabled(false);
                Elog.i(TAG, "stop test button click");
                Utils.stopMediaPlayer();
                break;
            case R.id.button_exit_total:
                Elog.i(TAG, "exit test button click");
                finish();
                break;
        }
    }

    public void startTimer() {
        timer = new CountDownTimer(mDelayTime * 1000, 1000) {
            @Override
            public void onTick(long millisUntilFinishied) {
                EmUtils.showToast("Wait for " + (mDelayTimeCount--));
            }

            @Override
            public void onFinish() {
                timer.cancel();
                timer = null;
                mIsScanModeRef = true;
                saveSettingsState();
                Elog.i(TAG, "timer finish");
                start_test_mode();
            }
        };
        timer.start();
        Elog.d(TAG, "delay_time = " + mDelayTime);
    }

    private boolean settings_init() {
        String rssi0_threshold = mRssi0_threshold.getText().toString();
        String rssi0_delta = mRssi0_delta.getText().toString();
        String rssi1_threshold = mRssi1_threshold.getText().toString();
        String rssi1_delta = mRssi1_delta.getText().toString();
        String delay_time = mDelay_time.getText().toString();
        String npt_pre_scan_time = mNptPre_Scan_Time.getText().toString();

        String ant_status = mNptAntStatus.getText().toString();

        if (rssi0_threshold.equals("") || rssi1_threshold.equals("")) {
            mRssi0_threshold.setText(RSSI_THRESHOLD_DEFAULT + "");
            EmUtils.showToast("threshold value should not be null");
            return false;
        }
        if (rssi0_delta.equals("") || rssi1_delta.equals("")) {
            mRssi0_delta.setText(RSSI_DELTA_DEFAULT + "");
            EmUtils.showToast("delta value should not be null");
            return false;
        }
        if (delay_time.equals("")) {
            mDelay_time.setText(RSSI_DELAY_TIME_DEFAULT + "");
            EmUtils.showToast("delay time should not be null");
            return false;
        }
        if (npt_pre_scan_time.equals("")) {
            mNptPre_Scan_Time.setText(NPT_PRE_SCAN_TIME_DEFAULT + "");
            EmUtils.showToast("npt_pre_scan_time should not be null");
            return false;
        }
        if (mAntMode == 1 && ant_status.equals("")) {
            mNptPre_Scan_Time.setText(ANT_STATUS + "");
            EmUtils.showToast("ant_status should not be null");
            return false;
        }

        mRssiThreshold0 = Integer.parseInt(rssi0_threshold);
        mRssiDelta0 = Integer.parseInt(rssi0_delta);
        mRssiThreshold1 = Integer.parseInt(rssi1_threshold);
        mRssiDelta1 = Integer.parseInt(rssi1_delta);
        mDelayTime = Integer.parseInt(delay_time);
        mNptPreScanTime = Integer.parseInt(npt_pre_scan_time);
        mAntStatus = Integer.parseInt(ant_status);

        mScanMode = mNpt_scan_mode_npt.isChecked() ? 1 : 0;
        mScanCmdPreRef = mNpt_scan_cmd_pre_ref.getText().toString();
        mScanCmdPreNpt = mNpt_scan_cmd_pre_npt.getText().toString();
        mScanCmdPostNpt = mNpt_scan_cmd_post_npt.getText().toString();
        return true;
    }

    void exec_pre_pose_npt(String mScanCmd) {
        try {
            String[] cmds = mScanCmd.split(",");
            for (String cmd : cmds) {
                Elog.d(TAG, "cmd = " + cmd);
                if (cmd != null && !cmd.equals("")) {
                    String value = "";
                    if (cmd.startsWith("adb shell")) {
                        value = cmd.substring(9);
                    } else {
                        value = cmd;
                    }
                    if (Utils.execCmd(value, true) == Utils.RETURN_SUCCESS) {
                        Elog.d(TAG, "exec_pre_pose_npt " + cmd + " succeed!");
                    } else {
                        Elog.d(TAG, "exec_pre_pose_npt " + cmd + " failed!");
                    }
                }
            }
        } catch (Exception e) {
            Elog.e(TAG, "exec_pre_npt error!");
        }
    }

    private void start_test_mode() {
        Elog.d(TAG, "start_test_mode,mScanMode = " + mScanMode
                + ",mIsScanModeRef = " + mIsScanModeRef);
        mBandNameLast = "";
        if (mScanMode == 1 && mIsScanModeRef == true) {
            // Utils.switchScreanOnOrOff(false);
            Utils.holdonWakelock(true);
            exec_pre_pose_npt(mScanCmdPreRef);
            updateUIInfo("REF Scan");
            mHandler.sendEmptyMessageDelayed(MSG_NPT_START_TEST, mNptPreScanTime * 1000);
        } else if (mScanMode == 1 && mIsScanModeRef == false) {
            //   Utils.switchScreanOnOrOff(true);
            exec_pre_pose_npt(mScanCmdPreNpt);
            updateUIInfo("NPT Scan");
            mHandler.sendEmptyMessageDelayed(MSG_NPT_START_TEST, mNptPreScanTime * 1000);
        } else {     //ref mode
            start_test();
        }
    }

    private void start_test() {
        mCurtItemIndex = 0;
        mCurtItem = sSelectedBandItems.get(mCurtItemIndex);
        int channel = mCurtItem.getmChannleScope()[0];
        start_noise_profiling(mCurtItem, channel);
    }

    void init_test_items() {
        sSelectedBandItems.clear();
        mResultListScan.clear();
        mResultAdapter_ref.notifyDataSetChanged();
        Elog.d(TAG, "init_test_items: ");
        for (int i = 0; i < mRatList.size(); i++) {
            if (mRatList.get(i).getRatCheckState() == true) {
                String rat_name = mRatList.get(i).getRatName();
                Elog.d(TAG, rat_name + " checked");
                if (rat_name.equals(mRatName[0])) { //GSM
                    for (BandItem item : BandConfigActivity.sGsmItems) {
                        if (item.ismSelected())
                            sSelectedBandItems.add(item);
                    }
                } else if (rat_name.equals(mRatName[1])) { //TDSCDMA
                    for (BandItem item : BandConfigActivity.sTddItems) {
                        if (item.ismSelected())
                            sSelectedBandItems.add(item);
                    }
                } else if (rat_name.equals(mRatName[2])) {//WCDMA
                    for (BandItem item : BandConfigActivity.sFddItems) {
                        if (item.ismSelected())
                            sSelectedBandItems.add(item);
                    }
                } else if (rat_name.equals(mRatName[3])) { //LTE
                    for (BandItem item : BandConfigActivity.sLteItems) {
                        if (item.ismSelected())
                            sSelectedBandItems.add(item);
                    }
                } else if (rat_name.equals(mRatName[4])) { //CDMA(1X)
                    for (BandItem item : BandConfigActivity.sCdmaItems) {
                        if (item.ismSelected())
                            sSelectedBandItems.add(item);
                    }
                } else if (rat_name.equals(mRatName[5])) { //EVDO
                    for (BandItem item : BandConfigActivity.sEvdoItems) {
                        if (item.ismSelected())
                            sSelectedBandItems.add(item);
                    }
                }
            }
        }
    }

    private void start_noise_profiling(BandItem item, int channel) {
        Elog.d(TAG, item.getmBandName() + ",channel = " + channel);
        String txFlag = "0";
        int AntStatus = 0;
        int ratValue = item.getmRatValue();
        if (ratValue == BandItem.BandType.BAND_EVDO.ordinal()) {
            ratValue = BandItem.BandType.BAND_CDMA.ordinal();
        }
        AntStatus = item.getmAntennaStateValue();
        if (mAntMode == 1) {
            AntStatus = Integer.valueOf(mNptAntStatus.getText().toString());
        }
        if (mScanMode == 0 || (mScanMode == 1 && mIsScanModeRef == true)) {
            txFlag = String.valueOf(item.getmTxOnflagValueRef());
        } else if (mScanMode == 1 && mIsScanModeRef == false) {
            txFlag = String.valueOf(item.getmTxOnflagValueNpt());
        }
        item.setmChannleValue(channel);
        mCurrentcmd = "AT+ERFSCAN="
                + item.getmRatValue() + "," + item.getmBandValue() + "," + channel + ","
                + item.getmRxBwValue() + "," + item.getmTxBwValue() + ","
                + txFlag + "," + item.getmTxPowerValue() + ","
                + item.getmRepeatTimesValue() + ","
                + item.getmRbStartValue() + "," + item.getmRblengthValue();
        mBandNameCur = item.getmBandName();
        if (item.getmRatValue() == BandItem.BandType.BAND_LTE.ordinal()) {  //add for tinna
            mCurrentcmd += ",1," + AntStatus;
            sendATCommand(mCurrentcmd, MSG_NPT_START_NOISE_PROFILING);
        } else if (!mBandNameCur.equals(mBandNameLast)) {
            mSwitchAntcmd = "AT+ETXANT=1," + ratValue + "," + AntStatus;
            sendATCommand(mSwitchAntcmd, MSG_NPT_SWITCH_ANT_STATUS_DONE);
            mBandNameLast = mBandNameCur;
        } else {
            sendATCommand(mCurrentcmd, MSG_NPT_START_NOISE_PROFILING);
        }
    }

    private void sendATCommand(String atCommand, int msg) {
        String[] cmd = new String[2];
        cmd[0] = atCommand;
        cmd[1] = "+ERFSCAN:";
        Elog.d(TAG, "atCommand = " + atCommand);
        EmUtils.invokeOemRilRequestStringsEm(cmd, mHandler.obtainMessage(msg));
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        Intent intent = new Intent();
        String item = mRatList.get(position).getRatName();
        if (item.equals(mRatName[0])) { //GSM
            intent.putExtra("mRatType", BandConfigActivity.RAT_TYPE_GSM);
        } else if (item.equals(mRatName[1])) {  //TDSCDMA
            intent.putExtra("mRatType", BandConfigActivity.RAT_TYPE_TDSCDMA);
        } else if (item.equals(mRatName[2])) { //WCDMA
            intent.putExtra("mRatType", BandConfigActivity.RAT_TYPE_WCDMA);
        } else if (item.equals(mRatName[3])) {//LTE
            intent.putExtra("mRatType", BandConfigActivity.RAT_TYPE_LTE);
        } else if (item.equals(mRatName[4])) {
            intent.putExtra("mRatType", BandConfigActivity.RAT_TYPE_CDMD_1X);
        } else if (item.equals(mRatName[5])) {
            intent.putExtra("mRatType", BandConfigActivity.RAT_TYPE_CDMD_EVDO);
        }
        intent.setClass(this, BandConfigActivity.class);
        this.startActivity(intent);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case DIG_WAIT_FOR_TEST:
                return new AlertDialog.Builder(this).setTitle("Waiting")
                        .setMessage("Waiting for start test?\n")
                        .setPositiveButton("Confirm", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                mStart_listen.setEnabled(false);
                                mStop_listen.setEnabled(true);
                                mTestStartedStatus = true;
                                startTimer();
                            }
                        })
                        .setNegativeButton("Cancel", null)
                        .create();
            case DIG_FINISH_TEST:
                return new AlertDialog.Builder(this).setTitle(
                        "The test is finish").setMessage("Press ok to stop music play!")
                        .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Utils.stopMediaPlayer();
                                if (mScanMode == 1) {
                                    Utils.holdonWakelock(false);
                                }
                                dialog.dismiss();
                            }
                        })
                        .create();
            case DIG_RESULT_NAME:
                mResultFileEt = new EditText(this);
                mResultFileEt.setText(mResultFilePath);
                DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (mResultFileEt.getText() != null && (!mResultFileEt.getText().toString
                                ().equals(""))) {
                            mResultFilePath = mResultFileEt.getText().toString().trim();
                            Elog.d(TAG, "mResultFilePath = " + mResultFilePath);
                            NoiseProfilingFileSave.setNptFiletName(mResultFilePath);
                            showDialog(DIG_WAIT_FOR_TEST);
                        }
                    }
                };

                return new AlertDialog.Builder(this)
                        .setTitle("OutputFile name:")
                        .setView(mResultFileEt)
                        .setPositiveButton("OK", listener)
                        .setNegativeButton("Cancel", null).create();
        }
        return super.onCreateDialog(id);
    }

    private void restoreSettingState() {
        SharedPreferences pref = getSharedPreferences(PREF_FILE, MODE_PRIVATE);
        mRssiThreshold0 = pref.getInt(KEY_RSSI0_THRESHOLD, RSSI_THRESHOLD_DEFAULT);
        mRssiDelta0 = pref.getInt(KEY_RSSI0_DELTA, RSSI_DELTA_DEFAULT);
        mRssiThreshold1 = pref.getInt(KEY_RSSI1_THRESHOLD, RSSI_THRESHOLD_DEFAULT);
        mRssiDelta1 = pref.getInt(KEY_RSSI1_DELTA, RSSI_DELTA_DEFAULT);
        mDelayTime = pref.getInt(KEY_DELAY_TIME, RSSI_DELAY_TIME_DEFAULT);
        mNptPreScanTime = pref.getInt(KEY_NPT_PRE_SCAN_TIME, NPT_PRE_SCAN_TIME_DEFAULT);
        mScanMode = pref.getInt(KEY_SCAN_MODE, SCAN_MODE_DEFAULT);
        mScanCmdPreRef = pref.getString(KEY_SCAN_CMD_PRE_REF, SCAN_CMD_PRE_REF_DEFAULT);
        mScanCmdPreNpt = pref.getString(KEY_SCAN_CMD_PRE_NPT, SCAN_CMD_PRE_NPT_DEFAULT);
        mScanCmdPostNpt = pref.getString(KEY_SCAN_CMD_POST_NPT, SCAN_CMD_POST_NPT_DEFAULT);

        mAntMode = pref.getInt(KEY_ANT_MODE, ANT_MODE);
        mAntStatus = pref.getInt(KEY_ANT_STATUS, ANT_STATUS);
        showSettingState();
    }

    void showSettingState() {
        mRssi0_threshold.setText(String.valueOf(mRssiThreshold0));
        mRssi0_delta.setText(String.valueOf(mRssiDelta0));

        mRssi1_threshold.setText(String.valueOf(mRssiThreshold1));
        mRssi1_delta.setText(String.valueOf(mRssiDelta1));
        mDelay_time.setText(String.valueOf(mDelayTime));
        mNptPre_Scan_Time.setText(String.valueOf(mNptPreScanTime));

        mNpt_scan_mode_ref.setChecked(mScanMode == 0);
        mNpt_scan_mode_npt.setChecked(mScanMode != 0);

        mNptAntMode.setChecked(mAntMode == 1);
        mNptAntStatus.setText(String.valueOf(mAntStatus));

        mNpt_scan_cmd_pre_ref.setText(mScanCmdPreRef);
        mNpt_scan_cmd_pre_npt.setText(mScanCmdPreNpt);
        mNpt_scan_cmd_post_npt.setText(mScanCmdPostNpt);
    }

    private void saveSettingsState() {
        SharedPreferences.Editor editor = getSharedPreferences(PREF_FILE, MODE_PRIVATE).edit();
        editor.putInt(KEY_RSSI0_THRESHOLD, mRssiThreshold0);
        editor.putInt(KEY_RSSI0_DELTA, mRssiDelta0);
        editor.putInt(KEY_RSSI1_THRESHOLD, mRssiThreshold1);
        editor.putInt(KEY_RSSI1_DELTA, mRssiDelta1);
        editor.putInt(KEY_DELAY_TIME, mDelayTime);
        editor.putInt(KEY_NPT_PRE_SCAN_TIME, mNptPreScanTime);
        editor.putInt(KEY_SCAN_MODE, mScanMode);
        editor.putInt(KEY_ANT_MODE, mAntMode);
        editor.putInt(KEY_ANT_STATUS, mAntStatus);
        editor.putString(KEY_SCAN_CMD_PRE_REF, mScanCmdPreRef);
        editor.putString(KEY_SCAN_CMD_PRE_NPT, mScanCmdPreNpt);
        editor.putString(KEY_SCAN_CMD_POST_NPT, mScanCmdPostNpt);
        editor.apply();
    }

    private void npt_settings_show() {
        DialogInterface.OnClickListener dialogOnclicListener = new DialogInterface
                .OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                ((ViewGroup) mNPTSettingsForm.getParent()).removeView(mNPTSettingsForm);
                switch (which) {
                    case Dialog.BUTTON_NEGATIVE:
                        break;
                    case Dialog.BUTTON_POSITIVE:
                        settings_init();
                        break;
                }
            }
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("NPT Setting:");
        builder.setNegativeButton("Cancel", dialogOnclicListener);
        builder.setPositiveButton("yes", dialogOnclicListener);
        builder.setCancelable(false);
        builder.setView(mNPTSettingsForm);
        builder.create().show();
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (isChecked) {
            mNptAntStatus.setEnabled(true);
            mAntMode = 1;
        } else {
            mNptAntStatus.setEnabled(false);
            mAntMode = 0;
        }
    }
}