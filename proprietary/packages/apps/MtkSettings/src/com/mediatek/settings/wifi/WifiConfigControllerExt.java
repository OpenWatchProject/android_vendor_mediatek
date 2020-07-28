package com.mediatek.settings.wifi;

import java.util.ArrayList;
import java.util.Arrays;

import android.app.AlertDialog;
import android.content.Context;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiConfiguration.GroupCipher;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.net.wifi.WifiConfiguration.PairwiseCipher;
import android.net.wifi.WifiConfiguration.Protocol;
import android.net.wifi.WifiManager;
import android.os.SystemProperties;
import android.security.Credentials;
import android.security.KeyStore;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import com.android.settings.R;
import com.android.mtksettingslib.wifi.AccessPoint;
import com.android.settings.utils.ManagedServiceSettings.Config;
import com.android.settings.wifi.WifiConfigController;
import com.android.settings.wifi.WifiConfigUiBase;
import com.mediatek.settings.FeatureOption;
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.IWifiExt;

public class WifiConfigControllerExt {
    private static final String TAG = "WifiConfigControllerExt";
    private final WifiManager mWifiManager;  // Add for WAPI @{

    // EAP SIM/AKA SIM slot selection @{
    private static final String SIM_STRING = "SIM";
    private static final String AKA_STRING = "AKA";
    private static final String AKA_PLUS_STRING = "AKA\'";
    private static final int WIFI_EAP_METHOD_DUAL_SIM = 2;
    private Spinner mSimSlot;
    // @}

    // Add for WAPI @{
    private Spinner mWapiCert;
    private static final String WLAN_PROP_KEY = "persist.vendor.sys.wlan";
    private static final String WIFI = "wifi";
    private static final String WAPI = "wapi";
    private static final String WIFI_WAPI = "wifi-wapi";
    private static final String DEFAULT_WLAN_PROP = WIFI_WAPI;
    // Updated WAPI_PSK and WAPI_CERT security value as two more
    // security types are added by Google
    public static final int SECURITY_WAPI_PSK = 9;
    public static final int SECURITY_WAPI_CERT = 10;
    // @}

    // Add for plug in
    private IWifiExt mExt;
    private Context mContext;
    private View mView;
    private WifiConfigUiBase mConfigUi;
    private WifiConfigController mController;

    public WifiConfigControllerExt(WifiConfigController controller, WifiConfigUiBase configUi,
            View view) {
        mController = controller;
        mConfigUi = configUi;
        mContext = mConfigUi.getContext();
        mView = view;
        mExt = UtilsExt.getWifiExt(mContext);
        /// Init Wi-Fi manager
        mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
    }

    public void addViews(WifiConfigUiBase configUi, String security) {
        ViewGroup group = (ViewGroup) mView.findViewById(R.id.info);
        // add security information
        View row = configUi.getLayoutInflater().inflate(R.layout.wifi_dialog_row, group, false);
        ((TextView) row.findViewById(R.id.name)).setText(configUi.getContext().getString(
                R.string.wifi_security));
        // mExt.setSecurityText((TextView) row.findViewById(R.id.name));
        ((TextView) row.findViewById(R.id.value)).setText(security);
        group.addView(row);
    }

    public void setConfig(WifiConfiguration config, int accessPointSecurity, TextView passwordView,
            Spinner eapMethodSpinner) {
        // get priority of configuration TODO 2019 Check below code
        config.wapiCertSelMode = 0;
        switch (accessPointSecurity) {
        // EAP SIM/AKA sim slot config @{
        case AccessPoint.SECURITY_EAP:
            config.enterpriseConfig.setSimNum(0);
            String eapMethodStr = (String) eapMethodSpinner.getSelectedItem();
            Log.d(TAG, "selected eap method:" + eapMethodStr);
            if (AKA_STRING.equals(eapMethodStr) || SIM_STRING.equals(eapMethodStr)
                    || AKA_PLUS_STRING.equals(eapMethodStr)) {
                if (mSimSlot == null) {
                    mSimSlot = (Spinner) mView.findViewById(R.id.sim_slot);
                }
                if (TelephonyManager.getDefault().getPhoneCount() == WIFI_EAP_METHOD_DUAL_SIM) {
                    int simSlot = mSimSlot.getSelectedItemPosition();
                    if (simSlot > -1) {
                        config.enterpriseConfig.setSimNum(simSlot);
                    }
                }
                Log.d(TAG, "EAP SIM/AKA config: " + config.toString());
            }
            break;
        // @}
        // Add WAPI_PSK & WAPI_CERT @{
        case SECURITY_WAPI_PSK:
            config.allowedKeyManagement.set(KeyMgmt.WAPI_PSK);
            config.allowedProtocols.set(Protocol.WAPI);
            if (passwordView.length() != 0) {
                String password = passwordView.getText().toString();
                config.preSharedKey = '"' + password + '"';
            }
            break;

        case SECURITY_WAPI_CERT:
            config.allowedKeyManagement.set(KeyMgmt.WAPI_CERT);
            config.allowedProtocols.set(Protocol.WAPI);
            if (mWapiCert.getSelectedItemPosition() != 0) {
                config.wapiCertSel = (String) mWapiCert.getSelectedItem();
                config.wapiCertSelMode = 1;
            }
            break;
        // @}
        default:
            break;
        }
    }

    private static String addQuote(int s) {
        return "\"" + s + "\"";
    }
    /// M: for china mobile wlan feature start @{
    public void setEapmethodSpinnerAdapter() {
        // set array for eap method spinner. CMCC will show only eap and sim
        Context context = mConfigUi.getContext();
        String[] eapString = context.getResources().getStringArray(R.array.wifi_eap_method);
        ArrayList<String> eapList = new ArrayList<String>(Arrays.asList(eapString));
        final ArrayAdapter<String> adapter = new ArrayAdapter<String>(context,
                android.R.layout.simple_spinner_item, eapList);
        if (mController.getAccessPoint() != null) {
            mExt.setEapMethodArray(adapter, getAccessPointSsid(), getAccessPointSecurity());
        }
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        // Add for triggering onItemSelected
        Spinner eapMethodSpinner = (Spinner) mView.findViewById(R.id.method);
        eapMethodSpinner.setAdapter(adapter);
    }

    public void setEapMethodFields(boolean edit) {
        Spinner eapMethodSpinner = (Spinner) mView.findViewById(R.id.method);
        int eapMethod = eapMethodSpinner.getSelectedItemPosition();
        // for CMCC-AUTO eap Method config information
        if (mController.getAccessPoint() != null) {
            eapMethod = mExt.getEapMethodbySpinnerPos(eapMethod, getAccessPointSsid(),
                    getAccessPointSecurity());
        }
        Log.d(TAG, "showSecurityFields modify method = " + eapMethod);
        // for CMCC ignore some config information
        mExt.hideWifiConfigInfo(new IWifiExt.Builder().setAccessPoint(mController.getAccessPoint())
                .setEdit(edit).setViews(mView), mConfigUi.getContext());
    }
    /// M }@

    /**
     * Show EAP SIM/AKA sim slot by method.
     *
     * @param eapMethod
     *            The EAP method of AP.
     */
    public void showEapSimSlotByMethod(int eapMethod) {
        // for CMCC-AUTO eap Method config information
        if (mController.getAccessPoint() != null) {
            eapMethod = mExt.getEapMethodbySpinnerPos(eapMethod, getAccessPointSsid(),
                    getAccessPointSecurity());
        }

        if (eapMethod == WifiConfigController.WIFI_EAP_METHOD_SIM
                || eapMethod == WifiConfigController.WIFI_EAP_METHOD_AKA
                || eapMethod == WifiConfigController.WIFI_EAP_METHOD_AKA_PRIME) {
            if (TelephonyManager.getDefault().getPhoneCount() == WIFI_EAP_METHOD_DUAL_SIM) {
                mView.findViewById(R.id.sim_slot_fields).setVisibility(View.VISIBLE);
                mSimSlot = (Spinner) mView.findViewById(R.id.sim_slot);
                Context context = mConfigUi.getContext();
                String[] tempSimAkaMethods = context.getResources()
                        .getStringArray(R.array.sim_slot);
                TelephonyManager telephonyManager = (TelephonyManager) mContext
                        .getSystemService(Context.TELEPHONY_SERVICE);
                int sum = telephonyManager.getSimCount();
                Log.d(TAG, "the num of sim slot is :" + sum);
                String[] simAkaMethods = new String[sum];
                for (int i = 0; i < (sum); i++) {
                    if (i < tempSimAkaMethods.length) {
                        simAkaMethods[i] = tempSimAkaMethods[i];
                    } else {
                        simAkaMethods[i] = tempSimAkaMethods[1].replaceAll("1", "" + i);
                    }
                }
                final ArrayAdapter<String> adapter = new ArrayAdapter<String>(context,
                        android.R.layout.simple_spinner_item, simAkaMethods);
                adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                mSimSlot.setAdapter(adapter);

                if (mController.getAccessPoint() != null
                        && mController.getAccessPoint().isSaved()) {
                    WifiConfiguration config = getAccessPointConfig();
                    if (config != null && config.enterpriseConfig != null) {
                        mSimSlot.setSelection(config.enterpriseConfig.getSimNum());
                    }
                }
            }
        } else {
            if (TelephonyManager.getDefault().getPhoneCount() == WIFI_EAP_METHOD_DUAL_SIM) {
                mView.findViewById(R.id.sim_slot_fields).setVisibility(View.GONE);
            }
        }
    }

    /**
     * add for EAP & WAPI
     */
    public boolean showSecurityFields(int accessPointSecurity, boolean edit) {
        Log.d(TAG, "showSecurityFields, accessPointSecurity = " + accessPointSecurity);
        Log.d(TAG, "showSecurityFields, edit = " + edit);

        // show WAPI_CERT field
        if (accessPointSecurity == SECURITY_WAPI_CERT) {
            mView.findViewById(R.id.security_fields).setVisibility(View.GONE);
            mView.findViewById(R.id.wapi_cert_fields).setVisibility(View.VISIBLE);
            mWapiCert = (Spinner) mView.findViewById(R.id.wapi_cert);
            mWapiCert.setOnItemSelectedListener(mController);
            loadCertificates(mWapiCert);

            if (mController.getAccessPoint() != null && mController.getAccessPoint().isSaved()) {
                WifiConfiguration config = getAccessPointConfig();
                setCertificate(mWapiCert, config.wapiCertSel);
            }
            return true;
        } else {
            mView.findViewById(R.id.wapi_cert_fields).setVisibility(View.GONE);
        }

        // for CMCC ignore some config information
        mExt.hideWifiConfigInfo(new IWifiExt.Builder().setAccessPoint(mController.getAccessPoint())
                .setEdit(edit).setViews(mView), mConfigUi.getContext());
        return false;
    }

    private void setCertificate(Spinner spinner, String cert) {
        Log.d(TAG, "setSelection, cert = " + cert);
        if (cert != null) {
            @SuppressWarnings("unchecked")
            ArrayAdapter<String> adapter = (ArrayAdapter<String>) spinner.getAdapter();
            for (int i = adapter.getCount() - 1; i >= 0; --i) {
                if (cert.equals(adapter.getItem(i))) {
                    spinner.setSelection(i);
                    break;
                }
            }
        }
    }

    public boolean enableSubmitIfAppropriate(TextView passwordView, int accessPointSecurity,
            boolean pwInvalid) {
        boolean passwordInvalid = pwInvalid;
        if (passwordView != null
                && ((accessPointSecurity == AccessPoint.SECURITY_WEP && !isWepKeyValid(passwordView
                        .getText().toString())) ||
                        ((accessPointSecurity == AccessPoint.SECURITY_PSK && passwordView
                        .length() < 8) || (accessPointSecurity == SECURITY_WAPI_PSK && (passwordView
                        .length() < 8
                        || 64 < passwordView.length()))))) {
            passwordInvalid = true;
        }
        return passwordInvalid;
    }

    public int getEapMethod(int eapMethod) {
        Log.d(TAG, "getEapMethod, eapMethod = " + eapMethod);
        int result = eapMethod;
        if (mController.getAccessPoint() != null) {
            result = mExt.getEapMethodbySpinnerPos(eapMethod, getAccessPointSsid(),
                    getAccessPointSecurity());
        }
        Log.d(TAG, "getEapMethod, result = " + result);
        return result;
    }

    public void setEapMethodSelection(Spinner eapMethodSpinner, int eapMethod) {
        int eapMethodPos = eapMethod;
        if (mController.getAccessPoint() != null) {
            eapMethodPos = mExt.getPosByEapMethod(eapMethod, getAccessPointSsid(),
                    getAccessPointSecurity());
        }
        eapMethodSpinner.setSelection(eapMethodPos);
        Log.d(TAG, "[skyfyx]showSecurityFields modify pos = " + eapMethodPos);
        Log.d(TAG, "[skyfyx]showSecurityFields modify method = " + eapMethod);

    }

    /**
     * 1.Add some more security spinners to support WAPI 2.Switch spinner
     * according to WIFI & WAPI config
     */
    public void addWifiConfigView(boolean edit) {
        // set security text
        //TextView securityText = (TextView) mView.findViewById(R.id.security_text);
        //mExt.setSecurityText(securityText);
        if (mController.getAccessPoint() == null) {
            // set array for wifi security
            Spinner mSecuritySpinner = ((Spinner) mView.findViewById(R.id.security));
            ArrayAdapter<String> spinnerAdapter = new ArrayAdapter<String>(mContext,
                            android.R.layout.simple_spinner_item, android.R.id.text1);
            spinnerAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mSecuritySpinner.setAdapter(spinnerAdapter);
            int idx = 0;

            if (FeatureOption.MTK_WAPI_SUPPORT) {
                String type = SystemProperties.get(WLAN_PROP_KEY, DEFAULT_WLAN_PROP);
                Log.d(TAG, "addWifiConfigView, type = " + type);
                if (type.equals(WIFI_WAPI)) {
                    idx = addWifiItems(spinnerAdapter, idx);
                    spinnerAdapter.add(mContext.getString(R.string.wapi_psk));
                    mController.mSecurityInPosition[idx++] = SECURITY_WAPI_PSK;
                    spinnerAdapter.add(mContext.getString(R.string.wifi_security_wapi_certificate));
                    mController.mSecurityInPosition[idx++] = SECURITY_WAPI_CERT;

                } else if (type.equals(WIFI)) {
                    idx = addWifiItems(spinnerAdapter, idx); // WIFI only, AOSP
                } else if (type.equals(WAPI)) {
                    // WAPI only
                    spinnerAdapter.add(mContext.getString(R.string.wapi_psk));
                    mController.mSecurityInPosition[idx++] = SECURITY_WAPI_PSK;
                    spinnerAdapter.add(mContext.getString(R.string.wifi_security_wapi_certificate));
                    mController.mSecurityInPosition[idx++] = SECURITY_WAPI_CERT;
                }
            } else {
                idx = addWifiItems(spinnerAdapter, idx); // WIFI only, AOSP
            }

            spinnerAdapter.notifyDataSetChanged();
        } else {
            WifiConfiguration config = getAccessPointConfig();
            Log.d(TAG, "addWifiConfigView, config = " + config);
            // Whether to show access point priority select spinner.
            if (mController.getAccessPoint().isSaved() && config != null) {
                Log.d(TAG, "priority=" + config.priority);
            }
        }

        // for CMCC ignore some config information
        mExt.hideWifiConfigInfo(new IWifiExt.Builder().setAccessPoint(mController.getAccessPoint())
                .setEdit(edit).setViews(mView), mConfigUi.getContext());
    }

    private int addWifiItems(ArrayAdapter<String> spinnerAdapter, int idx) {
        // Populate the Wi-Fi security spinner with the various supported key management types
        spinnerAdapter.add(mContext.getString(R.string.wifi_security_none));
        mController.mSecurityInPosition[idx++] = AccessPoint.SECURITY_NONE;
        if (mWifiManager.isEnhancedOpenSupported()) {
            spinnerAdapter.add(mContext.getString(R.string.wifi_security_owe));
            mController.mSecurityInPosition[idx++] = AccessPoint.SECURITY_OWE;
        }
                   spinnerAdapter.add(mContext.getString(R.string.wifi_security_wep));
                   mController.mSecurityInPosition[idx++] = AccessPoint.SECURITY_WEP;
                   spinnerAdapter.add(mContext.getString(R.string.wifi_security_wpa_wpa2));
                   mController.mSecurityInPosition[idx++] = AccessPoint.SECURITY_PSK;
                   if (mWifiManager.isWpa3SaeSupported()) {
                       spinnerAdapter.add(mContext.getString(R.string.wifi_security_sae));
                       mController.mSecurityInPosition[idx++] = AccessPoint.SECURITY_SAE;
                   }
                   spinnerAdapter.add(mContext.getString(R.string.wifi_security_eap));
                   mController.mSecurityInPosition[idx++] = AccessPoint.SECURITY_EAP;
                   if (mWifiManager.isWpa3SuiteBSupported()) {
                       spinnerAdapter.add(mContext.getString(R.string.wifi_security_eap_suiteb));
                       mController.mSecurityInPosition[idx++] = AccessPoint.SECURITY_EAP_SUITE_B;
                   }
        return idx;
    }

    private boolean isWepKeyValid(String password) {
        if (password == null || password.length() == 0) {
            return false;
        }
        int keyLength = password.length();
        /// M: ALPS03001760, confirmed by wap_supplicant, we don't support WEP-152 AP
        if (((keyLength == 10 || keyLength == 26) && password
                .matches("[0-9A-Fa-f]*"))
                || (keyLength == 5 || keyLength == 13)) {
            return true;
        }
        return false;
    }

    private void loadCertificates(Spinner spinner) {
        final Context context = mConfigUi.getContext();
        String autoSelectCert = context.getString(R.string.wapi_auto_sel_cert);
        String[] certs = KeyStore.getInstance().list("WAPI_CACERT_", android.os.Process.WIFI_UID);
        if (certs == null || certs.length == 0) {
            certs = new String[] { autoSelectCert };
        } else {
            final String[] array = new String[certs.length + 1];
            array[0] = autoSelectCert;
            System.arraycopy(certs, 0, array, 1, certs.length);
            certs = array;
        }
        final ArrayAdapter<String> adapter = new ArrayAdapter<String>(context,
                android.R.layout.simple_spinner_item, certs);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
    }

    /**
     * get the security to its corresponding security spinner position
     */
    public int getSecurity(int accessPointSecurity) {
        Log.d(TAG, "getSecurity, accessPointSecurity = " + accessPointSecurity);
        // Only WPAI supported
        if (FeatureOption.MTK_WAPI_SUPPORT) {
            String type = SystemProperties.get(WLAN_PROP_KEY, DEFAULT_WLAN_PROP);
            if (type.equals(WAPI) && accessPointSecurity > 0) {
                accessPointSecurity += SECURITY_WAPI_PSK - AccessPoint.SECURITY_WEP;
            }
        }
        Log.d(TAG, "getSecurity, accessPointSecurity = " + accessPointSecurity);
        return accessPointSecurity;
    }

    private WifiConfiguration getAccessPointConfig() {
        if (mController.getAccessPoint() != null) {
            return mController.getAccessPoint().getConfig();
        }
        return null;
    }

    private String getAccessPointSsid() {
        if (mController.getAccessPoint() != null) {
            return mController.getAccessPoint().getSsidStr();
        }
        return null;
    }

    private int getAccessPointSecurity() {
        if (mController.getAccessPoint() != null) {
            return mController.getAccessPoint().getSecurity();
        }
        return 0;
    }

    private DetailedState getAccessPointState() {
        if (mController.getAccessPoint() != null) {
            return (mController.getAccessPoint().getNetworkInfo() != null ? mController
                    .getAccessPoint().getNetworkInfo().getDetailedState() : null);
        }
        return null;
    }
}

