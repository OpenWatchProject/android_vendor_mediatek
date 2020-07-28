package com.mediatek.settings.ext;

import com.android.mtksettingslib.wifi.AccessPoint;

import android.content.ContentResolver;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiConfiguration;

import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import androidx.preference.PreferenceCategory;

import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;

public interface IWifiSettingsExt {

    /**
     * Called when register priority observer
     * @param contentResolver The parent contentResolver
     * @internal
     */
    void registerPriorityObserver(ContentResolver contentResolver);
    /**
     * Called when unregister priority observer
     * @param contentResolver The parent contentResolver
     * @internal
     */
    void unregisterPriorityObserver(ContentResolver contentResolver);
    /**
     * Remember the configration of last connected access point
     * @param config The configration of last connected access point
     * @internal
     */
    void setLastConnectedConfig(WifiConfiguration config);
    /**
     * Update priority for access point
     * @internal
     */
    void updatePriority();
    /**
     * update the context menu for the current selected access point
     * @internal
     */
    void updateContextMenu(ContextMenu menu, int menuId, Preference preference);
    /**
     * Remove all prefereces in every catogory
     * @param screen The parent screen
     * @internal
     */
    void emptyCategory(PreferenceScreen screen);
    /**
     * Remove all prefereces in the screen
     * @param screen The parent screen
     * @internal
     */
    void emptyScreen(PreferenceScreen screen);
    /**
     * Refresh the category
     * @param screen The parent screen
     * @internal
     */
    void refreshCategory(PreferenceScreen screen);
    /**
     * update priority of access points
     * @param WifiConfiguration The wifiConfiguration
     * @internal
     */
    void recordPriority(WifiConfiguration config);

    /**
     * update priority of access points
     * @param config The configuration of the latest connect access point
     * @internal
     */

    void setNewPriority(WifiConfiguration config);
    /**
     * update priority of access points after click submit button
     * @param config The configuration of the wifi dialog
     * @internal
     */
    void updatePriorityAfterSubmit(WifiConfiguration config);
    /**
     * Disconnect current connected access point
     * @param item The MenuItem
     * @param networkId The network id of the access point
     * @return true if CMCC item id is MENU_ID_DISCONNECT
     * @internal
     */
    boolean disconnect(MenuItem item, WifiConfiguration wifiConfig);

    /**
     * add all accessPoints to screen
     * @param screen The current screen
     * @param preferenceCategory google default preference Category
     * @param preference the current AccessPoint
     * @param isConfiged:true or false; false:  newCategory; true: add to common screen
     * @internal
     */
    void addPreference(PreferenceScreen screen,
            PreferenceCategory preferenceCategory, Preference preference, boolean isConfiged);

    /**
     * add all Category to screen
     * @param screen The current screen
     * @internal
     */
    void init(PreferenceScreen screen);

    /**
     * remove google default connection wifi.
     * default return false.
     * cmcc return true.
     */
    boolean removeConnectedAccessPointPreference();

    /**
     * remove CMCC connection category when wifi off.
     */
    void emptyConneCategory(PreferenceScreen screen);

    /**
     * update CMCC proprity.
     * @param config The WifiConfiguration which ssid submit
     * @param selectedAccessPoint The ssid's AccessPoint
     * @param detailedState The ssid's DetailedState
     */
    void submit(WifiConfiguration config, AccessPoint selectedAccessPoint,
            DetailedState detailedState);

    /**
     * customer add refresh button.
     * @param screen for find the additional_settings PreferenceCategory.
     * @param wifiTracker , WifiTracker is on or off.
     * @param isUiRestricted, If the user is not allowed to configure wifi,
     *  do not show the menu.
     */
    void addRefreshPreference(PreferenceScreen screen,
            Object wifiTracker,
            boolean isUiRestricted);

    /**
     * customer refresh button click.
     * CMCC and CT will be return true,
     * other return false.
     * @param preference refreshbutton.
     * @return true if CMCC or CT opeator.
     */
    boolean customRefreshButtonClick(Preference preference);

    /**
     * customer refresh button status
     * @param checkStatus checked or not.
     */
    void customRefreshButtonStatus(boolean checkStatus);

    /**
     * add modify menu on connect wifi preference
     * @param isSaved
     * @return isSaved if CMCC, return false if OM load
     */
    boolean addModifyMenu(boolean isSaved);
}
