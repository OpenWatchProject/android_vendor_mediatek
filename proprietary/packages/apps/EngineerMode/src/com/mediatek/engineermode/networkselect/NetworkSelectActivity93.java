package com.mediatek.engineermode.networkselect;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.RadioAccessFamily;
import android.telephony.TelephonyManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Spinner;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;


public class NetworkSelectActivity93 extends Activity implements OnCheckedChangeListener {
    public static final String TAG = "NetworkMode93";
    public static final String FK_MTK_C2K_CAPABILITY = "persist.vendor.radio.disable_c2k_cap";
    public static final int WCDMA_PREFERRED = TelephonyManager.NETWORK_MODE_WCDMA_PREF; // 0
    public static final int GSM_ONLY = TelephonyManager.NETWORK_MODE_GSM_ONLY; // 1
    public static final int WCDMA_ONLY = TelephonyManager.NETWORK_MODE_TDSCDMA_WCDMA; // 14
    public static final int GSM_WCDMA_AUTO
            = TelephonyManager.NETWORK_MODE_TDSCDMA_GSM_WCDMA; // 18
    public static final int CDMA_EVDO = TelephonyManager.NETWORK_MODE_CDMA_EVDO; // 4
    public static final int CDMA_ONLY = TelephonyManager.NETWORK_MODE_CDMA_NO_EVDO; // 5
    public static final int EVDO_ONLY = TelephonyManager.NETWORK_MODE_EVDO_NO_CDMA; // 6
    public static final int CDMA_EVDO_GSM_WCDMA
            = TelephonyManager.NETWORK_MODE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;// 21
    public static final int LTE_CDMA_EVDO = TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO; // 8
    public static final int LTE_GSM_WCDMA
            = TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_GSM_WCDMA;// 20
    public static final int LTE_CDMA_EVDO_GSM_WCDMA
            = TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;// 22
    public static final int LTE_ONLY = TelephonyManager.NETWORK_MODE_LTE_ONLY; // 11
    public static final int LTE_WCDMA = TelephonyManager.NETWORK_MODE_LTE_TDSCDMA_WCDMA; // 20
    public static final int TDSCDMA_ONLY = TelephonyManager.NETWORK_MODE_TDSCDMA_WCDMA; // 14
    public static final int GSM_TDSCDMA_AUTO
            = TelephonyManager.NETWORK_MODE_TDSCDMA_GSM_WCDMA; //18

    public static final int NR_ONLY = TelephonyManager.NETWORK_MODE_NR_ONLY; //23
    public static final int NR_LTE = TelephonyManager.NETWORK_MODE_NR_LTE; //24
    public static final int NR_LTE_UTMS
            = TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_WCDMA; //31
    public static final int NR_LTE_GSM_UTMS
            = TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_GSM_WCDMA; //32
    public static final int NR_LTE_CDMA_EVDO_GSM_UTMS
            = TelephonyManager.NETWORK_MODE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;//33

    private static final int EVENT_QUERY_NETWORKMODE_DONE = 101;
    private static final int EVENT_SET_NETWORKMODE_DONE = 102;
    private static final int EVENT_QUERY_EHRPD_ENABLE_DONE = 103;
    private static final int EVENT_SET_EHRPD_ENABLE_DONE = 104;
    private static final int REBOOT_DIALOG = 2000;
    private static final int INDEX_WCDMA_PREFERRED = 0;
    private static final int INDEX_GSM_ONLY = 1;
    private static final int INDEX_WCDMA_ONLY = 2;
    private static final int INDEX_TDSCDMA_ONLY = 3;
    private static final int INDEX_GSM_WCDMA_AUTO = 4;
    private static final int INDEX_GSM_TDSCDMA_AUTO = 5;
    private static final int INDEX_LTE_ONLY = 6;
    private static final int INDEX_LTE_GSM_UMTS = 7;
    private static final int INDEX_LTE_UMTS = 8;
    private static final int INDEX_CDMA_EVDO = 9;
    private static final int INDEX_CDMA_ONLY = 10;
    private static final int INDEX_EVDO_ONLY = 11;
    private static final int INDEX_LTE_CDMA_EVDO_UMTS_GSM = 12;
    private static final int INDEX_LTE_CDMA_EVDO = 13;
    private static final int INDEX_CDMA_EVDO_UMTS_GSM = 14;
    private static final int INDEX_NR_ONLY = 15;
    private static final int INDEX_NR_LTE = 16;
    private static final int INDEX_NR_LTE_UTMS = 17;
    private static final int INDEX_NR_LTE_UTMS_GSM = 18;
    private static final int INDEX_NR_LTE_UTMS_GSM_CDMA = 19;
    private static final int INDEX_EM_UI_NOT_SUPPORT_TYPE = INDEX_NR_LTE_UTMS_GSM_CDMA + 1;
    private static final int CARD_TYPE_GSM_ONLY = 1;
    private static final int CARD_TYPE_GSM_CDMA = 2;
    private static final int CARD_TYPE_CDMA_ONLY = 3;
    private int mSimType;
    private TelephonyManager mTelephonyManager = null;
    private String[] mCardTypeValues = new String[]{
            "gsm_only_card", "gsm_cdma_card", "cdma_only_card"};
    private String[] mNetworkTypeLabel = new String[]{
            "GSM/WCDMA (WCDMA preferred)",
            "GSM only",
            "WCDMA only",
            "TD-SCDMA only",
            "GSM/WCDMA (auto)",
            "GSM/TD-SCDMA(auto)",
            "LTE only",
            "LTE/UMTS/GSM",
            "LTE/UMTS",
            "CDMA/EVDO",
            "CDMA only",
            "EVDO only",
            "LTE/CDMA/EVDO/UMTS/GSM",
            "LTE/CDMA/EVDO",
            "CDMA/EVDO/UMTS/GSM",
            "NR only",
            "NR/LTE",
            "NR/LTE/UMTS",
            "NR/LTE/UMTS/GSM",
            "NR/LTE/UMTS/GSM/C2K",
            "EM UI not support this type"
    };
    private int[] mNetworkTypeValues = new int[]{
            WCDMA_PREFERRED,
            GSM_ONLY,
            WCDMA_ONLY,
            TDSCDMA_ONLY,
            GSM_WCDMA_AUTO,
            GSM_TDSCDMA_AUTO,
            LTE_ONLY,
            LTE_GSM_WCDMA,
            LTE_WCDMA,
            CDMA_EVDO,
            CDMA_ONLY,
            EVDO_ONLY,
            LTE_CDMA_EVDO_GSM_WCDMA,
            LTE_CDMA_EVDO,
            CDMA_EVDO_GSM_WCDMA,
            NR_ONLY,
            NR_LTE,
            NR_LTE_UTMS,
            NR_LTE_GSM_UTMS,
            NR_LTE_CDMA_EVDO_GSM_UTMS
    };
    private int mModemType;
    private SimCardInfo mSimCard;
    private boolean mFirstEntry = true;
    private boolean mEHRPDFirstEnter = true;
    private String[] network_mode_labels;
    private int mSubId = 1;

    private int mCurrentSelected = 0;
    private Spinner mPreferredNetworkSpinner = null;
    private CheckBox mCbDisableC2kCapabilit = null;
    private CheckBox mDisableeHRPDCheckBox = null;
    private int selectNetworkMode;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_QUERY_NETWORKMODE_DONE:
                    int result = msg.arg1;
                    if (result > 0) {
                        Elog.d(TAG, "Get Preferred Type = " + msg.arg1);
                        updateNetworkType(msg.arg1);
                    } else {
                        Elog.d(TAG, "query_preferred_failed");
                        EmUtils.showToast("query_preferred_failed");
                    }
                    break;
                case EVENT_SET_NETWORKMODE_DONE:
                    String info = "";
                    int status = msg.arg1;
                    if (status == 0) {
                        info = "set the network to : " + selectNetworkMode + " succeed";
                    } else {
                        info = "set the network to : " + selectNetworkMode + " failed";
                    }
                    EmUtils.showToast(info);
                    Elog.d(TAG, info);
                    break;
                case EVENT_QUERY_EHRPD_ENABLE_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        if (ar.result != null && ar.result instanceof String[]) {
                            String data[] = (String[]) ar.result;

                            if ((data.length > 0) && (data[0] != null)) {
                                Elog.d(TAG, "data[0]:" + data[0]);
                                if (data[0].equals("+EHRPD:0")) {
                                    mEHRPDFirstEnter = true;
                                    mDisableeHRPDCheckBox.setChecked(true);
                                } else {
                                    mEHRPDFirstEnter = false;
                                    mDisableeHRPDCheckBox.setChecked(false);
                                }
                            }
                        }
                    } else {
                        EmUtils.showToast(R.string.query_eHRPD_state_fail);
                        Elog.d(TAG, getResources().getString(R.string.query_eHRPD_state_fail));
                    }
                    break;
                case EVENT_SET_EHRPD_ENABLE_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        EmUtils.showToast(R.string.set_eHRPD_state_fail);
                        Elog.d(TAG, getResources().getString(R.string.set_eHRPD_state_fail));
                    } else {
                        EmUtils.showToast(R.string.set_eHRPD_state_succeed);
                        Elog.d(TAG, getResources().getString(R.string.set_eHRPD_state_succeed));
                    }
                    break;
                default:
                    break;
            }
        }
    };
    private OnItemSelectedListener mPreferredNetworkListener = new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView parent, View v, int pos, long id) {
            Elog.d(TAG, "onItemSelected = " + pos + ",mCurrentSelected = " + mCurrentSelected
                    + " " + mNetworkTypeLabel[pos]);

            if (mCurrentSelected == pos) {
                Elog.d(TAG, "listener being invoked by setSelection, return ");
                return; // avoid listener being invoked by setSelection()
            }

            mCurrentSelected = pos;
            selectNetworkMode = mNetworkTypeValues[pos];
            Elog.d(TAG, "selectNetworkMode to: " + selectNetworkMode);

            Settings.Global.putInt(getContentResolver(),
                    Settings.Global.PREFERRED_NETWORK_MODE + mSubId, selectNetworkMode);

            new Thread(new Runnable() {
                @Override
                public void run() {
                    boolean result = mTelephonyManager
                            .setPreferredNetworkType(mSubId, selectNetworkMode);
                    Message message = new Message();
                    message.what = EVENT_SET_NETWORKMODE_DONE;
                    message.arg1 = result ? 0 : -1;
                    mHandler.sendMessage(message);
                }
            }).start();
        }

        @Override
        public void onNothingSelected(AdapterView parent) {
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.networkmode_switching);
        mPreferredNetworkSpinner = (Spinner) findViewById(R.id.networkModeSwitching);
        findViewById(R.id.network_mode_set_hint).setVisibility(View.GONE);
        findViewById(R.id.disable_eHRPD).setVisibility(View.GONE);

        mSimType = getIntent().getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        Elog.i(TAG, "mSimType " + mSimType);

        boolean isCdma = ModemCategory.isCdma();
        if (isCdma) {
            mCbDisableC2kCapabilit = (CheckBox) findViewById(R.id.disable_c2k_capability);
            mCbDisableC2kCapabilit.setVisibility(View.VISIBLE);
            mCbDisableC2kCapabilit.setOnCheckedChangeListener(this);
            handleQueryCdmaCapability();
            mDisableeHRPDCheckBox = (CheckBox) findViewById(R.id.disable_eHRPD);
            mDisableeHRPDCheckBox.setOnCheckedChangeListener(this);
        } else {
            findViewById(R.id.disable_eHRPD).setVisibility(View.GONE);
        }

        CustomAdapter adapter =
                new CustomAdapter(this, android.R.layout.simple_spinner_item,
                        mNetworkTypeLabel);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mPreferredNetworkSpinner.setAdapter(adapter);
        mPreferredNetworkSpinner.setOnItemSelectedListener(mPreferredNetworkListener);
        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        if (isCdma) {
            queryeHRPDStatus();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        int mSimType = getIntent().getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        Elog.i(TAG, "mSimType " + mSimType);
        mSimCard = new SimCardInfo(mSimType);

        if (!ModemCategory.isSimReady(mSimCard.getSimType())) {
            EmUtils.showToast("The card is not ready,please check it");
            Elog.w(TAG, "The card is not ready,please check it");
            mPreferredNetworkSpinner.setEnabled(false);
            return;
        }

        mSubId = ModemCategory.getSubIdBySlot(mSimCard.getSimType());
        Elog.d(TAG, "mSubId = " + mSubId);
        if (mSubId < 0) {
            Elog.w(TAG, "Invalid sub id");
            return;
        } else {
            mPreferredNetworkSpinner.setEnabled(true);
        }

        mModemType = ModemCategory.getModemType(); // modem type: 1:FDD 2:TDD
        Elog.d(TAG, "mModemType = " + mModemType);

        new Thread(new Runnable() {
            @Override
            public void run() {
                if (mTelephonyManager != null) {
                    int result = mTelephonyManager.getPreferredNetworkType(mSubId);
                    Message message = new Message();
                    message.what = EVENT_QUERY_NETWORKMODE_DONE;
                    message.arg1 = result;
                    mHandler.sendMessage(message);
                } else {
                    Elog.e(TAG, "mTelephonyManager = null");
                }
            }
        }).start();

    }

    void updateNetworkType(int type) {
        int index = findSpinnerIndexByType(type);
        Elog.d(TAG, "Index = " + index);
        if (index >= 0 && index < mPreferredNetworkSpinner.getCount()) {
            mCurrentSelected = index;
            mPreferredNetworkSpinner.setSelection(index, true);
            Elog.d(TAG, "The NetworkSpinner show: " + index
                    + " " + mNetworkTypeLabel[index]);
            EmUtils.showToast("Netwok select type is: " + type);
        } else {
            Elog.d(TAG, "Netwok select not support the type: " + type);
            EmUtils.showToast("Netwok select not support the type: " + type);
            mCurrentSelected = INDEX_EM_UI_NOT_SUPPORT_TYPE;
            mPreferredNetworkSpinner.setSelection(INDEX_EM_UI_NOT_SUPPORT_TYPE,
                    true);
        }
    }


    private int findSpinnerIndexByType(int type) {
        // Not WCDMA preferred for TD
        // if (type == WCDMA_PREFERRED && mModemType == ModemCategory.MODEM_TD)
        // {
        // type = GSM_WCDMA_AUTO;
        // }
        // set tdscdma_GSM_auto ,framework return WCDMA_PREFERRED or
        // GSM_WCDMA_AUTO
        if ((type == WCDMA_PREFERRED || type == GSM_WCDMA_AUTO) &&
                (mModemType == ModemCategory.MODEM_TD) &&
                mSimCard.isCapabilitySim()) {
            type = GSM_TDSCDMA_AUTO;
        }
        // set tdscdma_only, return wcdma only mapping to tdscdma_only
        if (type == WCDMA_ONLY && mModemType == ModemCategory.MODEM_TD
                && mSimCard.isCapabilitySim()) {
            type = TDSCDMA_ONLY;
        }
        // Not support WCDMA preferred
        if (type == WCDMA_PREFERRED
                && !FeatureSupport.isSupported(FeatureSupport.FK_WCDMA_PREFERRED)) {
            type = GSM_WCDMA_AUTO;
        }

        for (int i = 0; i < mNetworkTypeValues.length; i++) {
            if (mNetworkTypeValues[i] == type && isAvailable(i)) {
                return i;
            }
        }
        return -1;
    }

    private boolean isAvailable(int index) {

        if (INDEX_EM_UI_NOT_SUPPORT_TYPE == index) {
            return false;
        }

        if (!mSimCard.isNRCapabilityViceSim()) {
            if (index == INDEX_NR_ONLY
                    || index == INDEX_NR_LTE
                    || index == INDEX_NR_LTE_UTMS
                    || index == INDEX_NR_LTE_UTMS_GSM
                    || index == INDEX_NR_LTE_UTMS_GSM_CDMA)
                return false;
        }

        // for main card
        if (mSimCard.isCapabilitySim()) {
            // for cdma only card, remove the other items except cdma items
            if (mSimCard.getCardType() == CARD_TYPE_CDMA_ONLY
                    && (index == INDEX_CDMA_ONLY
                    || index == INDEX_EVDO_ONLY
                    || index == INDEX_CDMA_EVDO)) {
                return true;
            } else if (mSimCard.getCardType() == CARD_TYPE_CDMA_ONLY) {
                return false;
            }
            // for GSM only card, remove the cdma items
            else if (mSimCard.getCardType() == CARD_TYPE_GSM_ONLY
                    && (index == INDEX_CDMA_ONLY
                    || index == INDEX_EVDO_ONLY
                    || index == INDEX_CDMA_EVDO
                    || index == INDEX_LTE_CDMA_EVDO
                    || index == INDEX_LTE_CDMA_EVDO_UMTS_GSM
                    || index == INDEX_CDMA_EVDO_UMTS_GSM)) {
                return false;
            }

            // for GSM_CDMA double card
            // for MODEM_TD(tdscdma), remove the WCDMA items
            if (mModemType == ModemCategory.MODEM_TD
                    && (index == INDEX_WCDMA_PREFERRED || index == INDEX_WCDMA_ONLY
                    || index == INDEX_GSM_WCDMA_AUTO)) {
                return false;
            }
            // for MODEM_FDD(Wcdma), remove the TDSCDMA items
            if (mModemType == ModemCategory.MODEM_FDD
                    && (index == INDEX_TDSCDMA_ONLY || index == INDEX_GSM_TDSCDMA_AUTO)) {
                return false;
            }

            // for None LTE project,remove all of lte items
            if (!ModemCategory.isLteSupport()
                    && (index == INDEX_LTE_ONLY || index == INDEX_LTE_GSM_UMTS
                    || index == INDEX_LTE_UMTS
                    || index == INDEX_LTE_CDMA_EVDO_UMTS_GSM
                    || index == INDEX_LTE_CDMA_EVDO)) {
                return false;
            }
            // for not support wcdma_preferred project,remove the
            // wcdma_preferred item
            if (!FeatureSupport.isSupported(FeatureSupport.FK_WCDMA_PREFERRED)
                    && index == INDEX_WCDMA_PREFERRED) {
                return false;
            }

            return true;
        } else { // for sub card

            // for cdma only card, remove all of items except cdma only item
            if (mSimCard.getCardType() == CARD_TYPE_CDMA_ONLY
                    && index == INDEX_CDMA_ONLY) {
                return true;
            } else if (mSimCard.getCardType() == CARD_TYPE_CDMA_ONLY) {
                return false;
            }
            // for GSM only card, remove the cdma items
            else if (mSimCard.getCardType() == CARD_TYPE_GSM_ONLY
                    && (index == INDEX_CDMA_ONLY
                    || index == INDEX_EVDO_ONLY
                    || index == INDEX_CDMA_EVDO
                    || index == INDEX_LTE_CDMA_EVDO
                    || index == INDEX_LTE_CDMA_EVDO_UMTS_GSM
                    || index == INDEX_CDMA_EVDO_UMTS_GSM)) {
                return false;
            }
            // for GSM_CDMA double card

            //support L+L LTE
            if (mSimCard.isLteCapabilityViceSim()) {
                if (index == INDEX_LTE_ONLY || index == INDEX_LTE_GSM_UMTS
                        || index == INDEX_LTE_UMTS || index == INDEX_LTE_CDMA_EVDO_UMTS_GSM
                        || index == INDEX_LTE_CDMA_EVDO) {
                    return true;
                }
            } else {
                if (index == INDEX_LTE_ONLY || index == INDEX_LTE_GSM_UMTS
                        || index == INDEX_LTE_UMTS || index == INDEX_LTE_CDMA_EVDO_UMTS_GSM
                        || index == INDEX_LTE_CDMA_EVDO) {
                    return false;
                }
            }
            //supoort W
            if (mSimCard.isWCapabilityViceSim()) {
                if (index == INDEX_TDSCDMA_ONLY || index == INDEX_GSM_TDSCDMA_AUTO) {
                    return false;
                }
                if (!FeatureSupport.isSupported(FeatureSupport.FK_WCDMA_PREFERRED)
                        && index == INDEX_WCDMA_PREFERRED) {
                    return false;
                }
                return true;
            } else {
                if (index == INDEX_GSM_ONLY || index == INDEX_CDMA_ONLY
                        || index == INDEX_CDMA_EVDO || index == INDEX_CDMA_EVDO_UMTS_GSM) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    }

    private void handleQueryCdmaCapability() {
        String enabled = SystemProperties.get(FK_MTK_C2K_CAPABILITY, "0");
        if ("1".equals(enabled)) {
            Elog.d(TAG, "QueryCdmaCapability set true");
            mFirstEntry = true;
            mCbDisableC2kCapabilit.setChecked(true);
        } else {
            Elog.d(TAG, "QueryCdmaCapability set false");
            mFirstEntry = false;
            mCbDisableC2kCapabilit.setChecked(false);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        switch (id) {
            case REBOOT_DIALOG:
                return new AlertDialog.Builder(this).setTitle(
                        "Disable c2k capability").setMessage("Please reboot the phone!")
                        .setPositiveButton("OK", null).create();
        }
        return dialog;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        // TODO Auto-generated method stub
        if (buttonView.getId() == R.id.disable_c2k_capability) {
            if (mFirstEntry == true) {
                mFirstEntry = false;
                return;
            }
            try {
                if (mCbDisableC2kCapabilit.isChecked()) {
                    EmUtils.getEmHidlService().setDisableC2kCap("1");
                } else {
                    EmUtils.getEmHidlService().setDisableC2kCap("0");
                }
            } catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
            showDialog(REBOOT_DIALOG);
        } else if (buttonView.getId() == R.id.disable_eHRPD) {
            if (mEHRPDFirstEnter == true) {
                mEHRPDFirstEnter = false;
                return;
            }
            mDisableeHRPDCheckBox.setChecked(isChecked);
            seteHRPDStatus(isChecked ? 0 : 1);
        }
    }

    private void queryeHRPDStatus() {
        String[] cmd = new String[3];
        cmd[0] = "AT+eHRPD?";
        cmd[1] = "+EHRPD:";
        cmd[2] = "DESTRILD:C2K";
        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);
        sendAtCommand(cmd_s, EVENT_QUERY_EHRPD_ENABLE_DONE);
    }

    private void seteHRPDStatus(int state) {
        String atCommand = null;
        switch (state) {
            case 0:
                atCommand = "AT+eHRPD=0";
                break;
            case 1:
                atCommand = "AT+eHRPD=1";
                break;
        }

        String[] cmd = new String[3];
        cmd[0] = atCommand;
        cmd[1] = "";
        cmd[2] = "DESTRILD:C2K";
        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);

        sendAtCommand(cmd_s, EVENT_SET_EHRPD_ENABLE_DONE);
    }

    private void sendAtCommand(String[] command, int msg) {
        Elog.d(TAG, "sendAtCommand: " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(mSimType, command, mHandler.obtainMessage(msg));
    }

    /**
     * Adapter.
     */
    public class CustomAdapter extends ArrayAdapter<String> {
        /**
         * Constructor.
         *
         * @param context            Context
         * @param textViewResourceId Resource id
         * @param objects            Objects
         */
        public CustomAdapter(Context context, int textViewResourceId, String[] objects) {
            super(context, textViewResourceId, objects);
        }

        @Override
        public View getDropDownView(int position, View convertView, ViewGroup parent) {
            View v = null;
            Elog.d(TAG, "isAvailable: " + position + " is " + isAvailable(position));
            if (!isAvailable(position)) {
                TextView tv = new TextView(getContext());
                tv.setVisibility(View.GONE);
                tv.setHeight(0);
                v = tv;
            } else {
                v = super.getDropDownView(position, null, parent);
            }
            return v;
        }
    }

    class SimCardInfo {
        private int mSimType;
        private boolean isCapabilitySim;
        private boolean isWCapabilityViceSim;
        private boolean isLteCapabilityViceSim;
        private boolean isNRCapabilityViceSim;
        private int mCardType;

        public SimCardInfo(int mSimType) {
            super();
            setSimType(mSimType);
            setCapabilitySim(ModemCategory.isCapabilitySim(mSimType));
            setWCapabilityViceSim(ModemCategory.CheckViceSimWCapability(mSimType));
            setLteCapabilityViceSim(ModemCategory.checkViceSimCapability(
                    mSimType, RadioAccessFamily.RAF_LTE));
            setNRCapabilityViceSim(ModemCategory.CheckViceSimNRCapability(mSimType));
            setCardType(getCardType(mSimType)); // card type:1: GSM only 2:GSM_CDMA 3:CDMA only
        }

        public int getCardType() {
            return mCardType;
        }

        private void setCardType(int mCardType) {
            this.mCardType = mCardType;
        }

        public boolean isNRCapabilityViceSim() {
            return isNRCapabilityViceSim;
        }

        public void setNRCapabilityViceSim(boolean NRCapabilityViceSim) {
            isNRCapabilityViceSim = NRCapabilityViceSim;
        }


        public boolean isLteCapabilityViceSim() {
            return isLteCapabilityViceSim;
        }

        private void setLteCapabilityViceSim(boolean isLteCapabilityViceSim) {
            this.isLteCapabilityViceSim = isLteCapabilityViceSim;
        }

        public boolean isWCapabilityViceSim() {
            return isWCapabilityViceSim;
        }

        private void setWCapabilityViceSim(boolean isWCapabilityViceSim) {
            this.isWCapabilityViceSim = isWCapabilityViceSim;
        }

        public int getSimType() {
            return mSimType;
        }

        public void setSimType(int mSimType) {
            this.mSimType = mSimType;

        }

        public boolean isCapabilitySim() {
            return this.isCapabilitySim;
        }

        private void setCapabilitySim(boolean isCapabilitySim) {
            this.isCapabilitySim = isCapabilitySim;
        }

        private int getCardType(int mSimType) {
            int type = CARD_TYPE_GSM_ONLY;
            String[] Cardtype = ModemCategory.getSupportCardType(mSimType);
            if (Cardtype != null) {
                boolean iscCt3gDualMode = ModemCategory.isCt3gDualMode(mSimType);
                String Cardtypes = "";
                for (int i = 0; i < Cardtype.length; i++) {
                    Cardtypes += Cardtype[i] + " ";
                }
                if ((Cardtypes.contains("USIM") && Cardtypes.contains("CSIM"))
                        || iscCt3gDualMode == true) {
                    type = CARD_TYPE_GSM_CDMA;
                } else if (Cardtypes.contains("USIM") && Cardtypes.contains("RUIM")) {
                    type = CARD_TYPE_GSM_CDMA;
                } else if (Cardtypes.contains("CSIM") && Cardtypes.contains("SIM")) {
                    type = CARD_TYPE_GSM_CDMA;
                } else if (!Cardtypes.contains("RUIM") && !Cardtypes.contains("CSIM")) {
                    type = CARD_TYPE_GSM_ONLY;
                } else if (!(Cardtypes.contains("SIM") && !Cardtypes.contains("USIM"))
                        && iscCt3gDualMode == false) {
                    type = CARD_TYPE_CDMA_ONLY;
                } else
                    Elog.w(TAG, "the card type is unknow!");
            } else
                Elog.w(TAG, "there has no card insert,default type is GSM_only");
            Elog.d(TAG, "card type = " + mCardTypeValues[type - 1]);
            return type;
        }
    }

}
