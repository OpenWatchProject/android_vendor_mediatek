package com.mediatek.settings.ext;

import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;

import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiConfiguration;
import android.support.v7.preference.ListPreference;
import android.support.v7.preference.PreferenceScreen;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.TextView;

import com.android.mtksettingslib.wifi.AccessPoint;

public interface IWifiExt {
    //WifiConfigController
    /**
     * set network id for access point.
     * @param apNetworkId New network id
     * @internal
     */
    void setAPNetworkId(WifiConfiguration wifiConfig);
    /**
     * Set priority for access point.
     * @param apPriority New priority
     * @internal
     */
    void setAPPriority(int apPriority);
    /**
     * Initiatlize priority UI.
     * @param priorityLayout The priority view layout
     * @param networkId The current access point network id
     * @param isEdit is the operation edit access point
     * @internal
     */
    void setPriorityView(LinearLayout priorityLayout, WifiConfiguration wifiConfig, boolean isEdit);
    /**
     * Set security text label.
     * @param context The parent context
     * @param view The view which contains security label
     * @internal
     */
    void setSecurityText(TextView view);
    /**
     * add disconnect button for wifi dialog.
     * @param dialog the wifi dialog
     * @param edit is current edit access point
     * @param state wifi detailed state
     * @param networkId network Id
     * @internal
     */
    void addDisconnectButton(AlertDialog dialog, boolean edit,
            DetailedState state, WifiConfiguration wifiConfig);
    /**
     * get priority for current select access point.
     * @param priority get the access point's new priority
     * @return new priority
     * @internal
     */
    int getPriority(int priority);
    /**
     * set proxy title
     * @internal
     */
    void setProxyText(TextView view);
    //advanced wifi settings
    /**
     * Initiatlize connect type in wifi advanced settings.
     * @param screen The screen of wifi advanced settings
     * @param connectType Connect type is manual or auto
     * @param connectApType data connection change type from Cell to Wifi, manual or auto
     * @param selectSsidType data connection change type among wifi access points
     * @param listner called when this preference has changed
     * @internal
     */
    void initConnectView(Activity activity, PreferenceScreen screen);
    /**
     * Initiatlize gatway & netmask info in wifi advanced settings.
     * @param screen The screen of wifi advanced settings
     * @internal
     */
    void initNetworkInfoView(PreferenceScreen screen);
    /**
     * refresh gatway & netmask info in wifi advanced settings.
     * @internal
     */
    void refreshNetworkInfoView();
    /**
     * Initiatlize preference for wifi advanced settings.
     * @param contentResolver The parent content resolver
     * @internal
     */
    void initPreference(ContentResolver contentResolver);
    /**
     * Set sleep policy preference entries and values.
     * @param sleepPolicyPref Wifi sleep policy setting preference
     * @param sleepPolicyEntries wifi sleep policy setting entries array
     * @param sleepPolicyValues wifi sleep policy setting values array
     * @internal
     */
    void setSleepPolicyPreference(ListPreference sleepPolicyPref,
            String[] sleepPolicyEntries, String[] sleepPolicyValues);
    /**
     * hide edit ap info
     * @param builder access point information
     * @internal
     */
    void hideWifiConfigInfo(Builder builder , Context context);

    public class Builder {
        private AccessPoint mAccessPoint;
        private boolean mEdit;
        private View mView;

        public Builder() {

        }

        public Builder setAccessPoint(AccessPoint accessPoint) {
            this.mAccessPoint = accessPoint;
            return this;
        }


        public AccessPoint getAccessPoint() {
            return this.mAccessPoint;
        }
        public Builder setEdit(boolean edit) {
            this.mEdit = edit;
            return this;
        }

        public boolean getEdit() {
            return this.mEdit;
        }
        public Builder setViews(View view) {
            this.mView = view;
            return this;
        }

        public View getViews() {
            return this.mView;
        }

    }

    /**
     * @internal
     */
    void setEapMethodArray(ArrayAdapter adapter, String ssid, int security);

    //wifi controller
    /**
     * get eap method by spinner position.
     * @param lists current edit access point information
     * @internal
     */
    int getEapMethodbySpinnerPos(int spinnerPos, String ssid, int security);

    /**
     * @internal
     */
    int getPosByEapMethod(int spinnerPos, String ssid, int security);

    /**
     * For create new PreferenceController.
     * @param context context
     * @param lifecycle LifeCycle
     * @return PreferenceController.
     */
    Object createWifiPreferenceController(Context context, Object lifecycle);

    /**
     * For CMCC WLAN feature.
     * add OP01 Wifi Preference Controller to OM part.
     * @param controllers OM List controller.
     * @param wifiPreferenceController OP01WifiPreferenceController.
     */
    void addPreferenceController(Object controllers,
            Object wifiPreferenceController);
}
