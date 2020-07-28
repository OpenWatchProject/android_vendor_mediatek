package com.mediatek.engineermode.networkselect;

import android.app.Activity;
import android.content.Context;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.telephony.RadioAccessFamily;
import android.telephony.TelephonyManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;


public class NetworkSelectActivity extends Activity {
    private static final String TAG = "NetworkMode";
    private static final int EVENT_QUERY_NETWORKMODE_DONE = 101;
    private static final int EVENT_SET_NETWORKMODE_DONE = 102;

    private static final int INDEX_WCDMA_PREFERRED = 0;
    private static final int INDEX_GSM_ONLY = 1;
    private static final int INDEX_WCDMA_ONLY = 2;
    private static final int INDEX_TDSCDMA_ONLY = 3;
    private static final int INDEX_GSM_WCDMA_AUTO = 4;
    private static final int INDEX_GSM_TDSCDMA_AUTO = 5;
    private static final int INDEX_LTE_ONLY = 6;
    private static final int INDEX_CDMA_ONLY = 7;
    private static final int INDEX_LTE_GSM_WCDMA = 8;
    private static final int INDEX_LTE_WCDMA = 9;
    private static final int INDEX_EM_UI_NOT_SUPPORT_TYPE = INDEX_LTE_WCDMA + 1;

    private static final int WCDMA_PREFERRED = NetworkSelectActivity93.WCDMA_PREFERRED;
    private static final int GSM_ONLY = NetworkSelectActivity93.GSM_ONLY;
    private static final int WCDMA_ONLY = NetworkSelectActivity93.WCDMA_ONLY;
  private static final int TDSCDMA_ONLY = NetworkSelectActivity93.TDSCDMA_ONLY;
    private static final int GSM_WCDMA_AUTO = NetworkSelectActivity93.GSM_WCDMA_AUTO;
    private static final int GSM_TDSCDMA_AUTO = NetworkSelectActivity93.GSM_TDSCDMA_AUTO;
    private static final int LTE_ONLY = NetworkSelectActivity93.LTE_ONLY;
    private static final int CDMA_ONLY = NetworkSelectActivity93.CDMA_ONLY;
    private static final int LTE_GSM_WCDMA = NetworkSelectActivity93.LTE_GSM_WCDMA;
    private static final int LTE_CDMA_EVDO_GSM_WCDMA =
            NetworkSelectActivity93.LTE_CDMA_EVDO_GSM_WCDMA;
    private static final int LTE_WCDMA = NetworkSelectActivity93.LTE_WCDMA;
    private TelephonyManager mTelephonyManager = null;
    private int mModemType;
    private SimCardInfo mSimCard;
    private int mSubId = 1;
    private int[] mNetworkTypeValues = new int[] {WCDMA_PREFERRED, GSM_ONLY, WCDMA_ONLY,
            TDSCDMA_ONLY, GSM_WCDMA_AUTO, GSM_TDSCDMA_AUTO,
            LTE_ONLY, CDMA_ONLY, LTE_GSM_WCDMA, LTE_WCDMA};
    private int mCurrentSelected = 0;
    private Spinner mPreferredNetworkSpinner = null;

    private OnItemSelectedListener mPreferredNetworkListener = new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView parent, View v, int pos, long id) {
            Elog.d(TAG, "onItemSelected " + pos + " mCurrentSelected: " + mCurrentSelected);
            if (mCurrentSelected == pos || pos == INDEX_EM_UI_NOT_SUPPORT_TYPE ) {
                return; // avoid listener being invoked by setSelection()
            }
            mCurrentSelected = pos;

            Message msg = mHandler.obtainMessage(EVENT_SET_NETWORKMODE_DONE);
            int selectNetworkMode = mNetworkTypeValues[pos];

            Elog.d(TAG, "selectNetworkMode " + selectNetworkMode);
            Settings.Global.putInt(getContentResolver(),
                    Settings.Global.PREFERRED_NETWORK_MODE + mSubId, selectNetworkMode);

            new Thread(new Runnable() {
                @Override
                public void run() {
                    boolean result = mTelephonyManager
                            .setPreferredNetworkType(mSubId, selectNetworkMode);
                    Message message = new Message();
                    message.what = EVENT_SET_NETWORKMODE_DONE;
                    message.arg1 =  result ? 0 : -1;
                    mHandler.sendMessage(message);
                }
            }).start();
        }

        @Override
        public void onNothingSelected(AdapterView parent) {
        }
    };

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
                    info = "set the network succeed";
                } else {
                    info = "set the network failed";
                }
                EmUtils.showToast(info);
                Elog.d(TAG, info);
                break;

            default:
                break;
            }
        }
    };

    public class SimCardInfo {
        private int mSimType = PhoneConstants.SIM_ID_1;
        private boolean isCapabilitySim = ModemCategory.isCapabilitySim(this.mSimType);
        private boolean isWCapabilityViceSim = ModemCategory.CheckViceSimWCapability(this.mSimType);
        private boolean isLteCapabilityViceSim =
            ModemCategory.checkViceSimCapability(mSimType, RadioAccessFamily.RAF_LTE);
        public boolean isLteCapabilityViceSim() {
            return isLteCapabilityViceSim;
        }
        private void setLteCapabilityViceSim(boolean isLteCapabilityViceSim) {
            this.isLteCapabilityViceSim = isLteCapabilityViceSim;
        }
        public SimCardInfo(int mSimType) {
            super();
            setSimType(mSimType);
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
            setCapabilitySim(ModemCategory.isCapabilitySim(mSimType));
            setWCapabilityViceSim(ModemCategory.CheckViceSimWCapability(this.mSimType));
            setLteCapabilityViceSim(ModemCategory.checkViceSimCapability(
                mSimType, RadioAccessFamily.RAF_LTE));
        }
        public boolean isCapabilitySim() {
            return this.isCapabilitySim;
        }
        private void setCapabilitySim(boolean isCapabilitySim) {
            this.isCapabilitySim = isCapabilitySim;
        }
    }
    /**
     * Adapter.
     */
    public class CustomAdapter extends ArrayAdapter<String> {
        /**
         * Constructor.
         *
         * @param context
         *          Context
         * @param textViewResourceId
         *          Resource id
         * @param objects
         *          Objects
         */
        public CustomAdapter(Context context, int textViewResourceId, String[] objects) {
            super(context, textViewResourceId, objects);
        }

        @Override
        public View getDropDownView(int position, View convertView, ViewGroup parent) {
            View v = null;
            Elog.d(TAG, "isAvailable: " + position + " is " + isAvailable(position));
            if (!isAvailable(position) || position == INDEX_EM_UI_NOT_SUPPORT_TYPE) {
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

    private boolean isAvailable(int index) {
        if(mSimCard.isCapabilitySim()) {
            if (mModemType == ModemCategory.MODEM_TD
                && (index == INDEX_WCDMA_PREFERRED || index == INDEX_WCDMA_ONLY
                    || index == INDEX_GSM_WCDMA_AUTO)) {
                return false;
            }
            if (mModemType == ModemCategory.MODEM_FDD
                && (index == INDEX_TDSCDMA_ONLY || index == INDEX_GSM_TDSCDMA_AUTO)) {
                return false;
            }
            if (!ModemCategory.isLteSupport()
                && (index == INDEX_LTE_ONLY || index == INDEX_LTE_GSM_WCDMA
                    || index == INDEX_LTE_WCDMA)) {
                return false;
            }
            if (!FeatureSupport.isSupported(FeatureSupport.FK_WCDMA_PREFERRED)
                && index == INDEX_WCDMA_PREFERRED) {
                return false;
            }
            if (index == INDEX_CDMA_ONLY) {
                return false;
            }
            return true;
        }else {
            //support L+L LTE
            if (mSimCard.isLteCapabilityViceSim()) {
                if (index == INDEX_LTE_ONLY || index == INDEX_LTE_GSM_WCDMA
                        || index == INDEX_LTE_WCDMA ) {
                    return true;
                }
            }else{
                if (index == INDEX_LTE_ONLY || index == INDEX_LTE_GSM_WCDMA
                        || index == INDEX_LTE_WCDMA) {
                    return false;
                }
            }

            //supoort W
            if (mSimCard.isWCapabilityViceSim()) {
                if (index == INDEX_TDSCDMA_ONLY || index == INDEX_GSM_TDSCDMA_AUTO
                        || index == INDEX_CDMA_ONLY ) {
                    return false;
                }
                if (!FeatureSupport.isSupported(FeatureSupport.FK_WCDMA_PREFERRED)
                        && index == INDEX_WCDMA_PREFERRED) {
                    return false;
                }
                return true;
            } else {
                if (index == INDEX_GSM_ONLY) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.networkmode_switching);
        mPreferredNetworkSpinner = (Spinner) findViewById(R.id.networkModeSwitching);
        if (!ModemCategory.isCdma()) {
            findViewById(R.id.network_mode_set_hint).setVisibility(View.GONE);
        }
        findViewById(R.id.disable_eHRPD).setVisibility(View.GONE);
        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
    }

    @Override
    protected void onResume() {
        super.onResume();
        int mSimType = getIntent().getIntExtra("mSimType", ModemCategory.getCapabilitySim());
        Elog.i(TAG, "mSimType " + mSimType);
        mSimCard = new SimCardInfo(mSimType);
        if (!ModemCategory.isSimReady(mSimCard.getSimType())) {
            EmUtils.showToast("The card is not ready,please check it");
            Elog.w(TAG, "The card is not ready,please check it");
            mPreferredNetworkSpinner.setEnabled(false);
            return;
        }
        mSubId = ModemCategory.getSubIdBySlot(mSimCard.getSimType());
        if (mSubId < 0) {
            Elog.w(TAG, "Invalid sub id");
            return;
        } else {
            mPreferredNetworkSpinner.setEnabled(true);
        }

        mModemType = ModemCategory.getModemType();
        String[] labels;
        if (mModemType != ModemCategory.MODEM_NO3G) {
            labels = getResources().getStringArray(R.array.network_mode_labels);
        } else {
            Elog.w(TAG, "Isn't TD/WCDMA modem: " + mModemType);
            labels = new String[] {"GSM only"};
            mNetworkTypeValues = new int[] {GSM_ONLY};
        }
        CustomAdapter adapter =
                new CustomAdapter(this, android.R.layout.simple_spinner_item, labels);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mPreferredNetworkSpinner.setAdapter(adapter);
        for (int i = 0; i < mNetworkTypeValues.length; i++){
              if(isAvailable(i)){
                  mCurrentSelected = i;
                  mPreferredNetworkSpinner.setSelection(i, true);
                  break;
              }
         }
        mPreferredNetworkSpinner.setOnItemSelectedListener(mPreferredNetworkListener);

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

    private int findSpinnerIndexByType(int type) {
        if ((type == WCDMA_PREFERRED || type == GSM_WCDMA_AUTO) &&
                mModemType == ModemCategory.MODEM_TD) {
            type = GSM_TDSCDMA_AUTO;
        }
        // Not support WCDMA preferred
        if (type == WCDMA_PREFERRED
                && !FeatureSupport.isSupported(FeatureSupport.FK_WCDMA_PREFERRED)) {
            type = GSM_WCDMA_AUTO;
        }
        //set tdscdma_only, return wcdma only mapping to tdscdma_only
        if (type == WCDMA_ONLY && mModemType == ModemCategory.MODEM_TD) {
            type = TDSCDMA_ONLY;
        }
        for (int i = 0; i < mNetworkTypeValues.length; i++) {
            if (mNetworkTypeValues[i] == type && isAvailable(i)) {
                return i;
            }
        }
        return -1;
    }
    void updateNetworkType(int type) {
        int index = findSpinnerIndexByType(type);
        Elog.d(TAG, "Index = " + index);
        if (index >= 0 && index < mPreferredNetworkSpinner.getCount()) {
            mCurrentSelected = index;
            mPreferredNetworkSpinner.setSelection(index, true);
            Elog.d(TAG, "Netwok select type is: " + type);
            EmUtils.showToast("Netwok select type is: " + type);
        } else {
            Elog.d(TAG, "Netwok select not support the type: " + type);
            EmUtils.showToast("Netwok select not support the type: " + type);
            mCurrentSelected = INDEX_EM_UI_NOT_SUPPORT_TYPE;
            mPreferredNetworkSpinner.setSelection(INDEX_EM_UI_NOT_SUPPORT_TYPE,
                    true);
        }
    }
}
