package com.mediatek.digits;

import android.os.Bundle;
import com.mediatek.digits.DigitsDevice;
import com.mediatek.digits.DigitsLine;
import com.mediatek.digits.DigitsProfile;
import com.mediatek.digits.IDigitsEventListener;
import com.mediatek.digits.IDigitsActionListener;

interface IDigitsService {

    /**
     * Register listener for any digits event. It's an synchronous API.
     *
     * @param listener the instance of IDigitsServiceListener
     */
    void registerListener(in IDigitsEventListener listener);

    /**
     * Un-register listener for any digits event. It's an synchronous API.
     *
     * @param listener the instance of IDigitsServiceListener
     */
    void unregisterListener(in IDigitsEventListener listener);

    /****************************************************************
     * The following API is used to init Jansky service
     *
     ****************************************************************/

    /**
     * Get device config
     *
     */
    void getDeviceConfig(in IDigitsActionListener listener);

    /**
     * Update Location and T&C information to Get the latest URL/post-data
     *
     */
    void updateLocationAndTc(in IDigitsActionListener listener);

    /**
     * Notify to continue initing Jansky service. It's an asynchronous API.
     *
     */
    void notifyAddressOnFile();

    /****************************************************************
     * The following API is used to log in & log out
     *
     ****************************************************************/

    /**
     * Get TMOID login url for setting. It's an synchronous API.
     * [Authentication_APIv0.15.pdf]
     * 8. Initiate TMOID Login with websheet
     *
     */
    String getTmoidLoginURL();

    /**
     * Create TMO ID
     *
     */
    String getTmoidSignupURL();

    /**
     * Get MSISDN Auth url for setting. It's an synchronous API.
     * [Authentication_APIv0.15.pdf]
     * 9. Initiate MSISDN Auth with websheet
     *
     */
    String getMsisdnAuthURL();

    /**
     * Set auth code after login through websheet from setting. It's an synchronous API.
     * @param authCode auth code
     */
    void subscribe(in String authCode, in IDigitsActionListener listener);

    /**
     * Logout. It's an asynchronous API.
     */
    void logout(in IDigitsActionListener listener);

    /****************************************************************
     * The following API is used after log in.
     *
     ****************************************************************/

    /**
     * Set device name. It's an asynchronous API.
     *
     * @param name a friendly device name
     */
    void setDeviceName(in String name, in IDigitsActionListener listener);

    /**
     * Set line name. It's an asynchronous API.
     *
     * @param msisdn the line number
     * @param name the line name
     */
    void setLineName(in String msisdn, in String name, in IDigitsActionListener listener);

    /**
     * Set line color. It's an asynchronous API.
     *
     * @param msisdn the line number
     * @param color the line color
     */
    void setLineColor(in String msisdn, int color, in IDigitsActionListener listener);

    /**
     * Get the available line list after service is in SUBSCRIBED state. It's an synchronous API.
     *
     * @return the list of DigitsLine
     */
    DigitsLine[] getRegisteredLine();

    /**
     * Get available devices after service is in SUBSCRIBED state. It's an synchronous API.
     *
     * @return the list of DigitsDevice
     */
    DigitsDevice[] getRegisteredDevice();

    /**
     * Get consumer profile after service is in SUBSCRIBED state. It's an synchronous API.
     *
     * @return the list of DigitsProfile
     */
    DigitsProfile getDigitsProfile();

    /**
     * Activate a line. It's an asynchronous API.
     *
     * @param msisdn the line number
     */
    void activateLine(in String msisdn, in IDigitsActionListener listener);

    /**
     * Deactivate a line. It's an asynchronous API.
     *
     * @param msisdn the line number
     */
    void deactivateLine(in String msisdn, in IDigitsActionListener listener);

    /**
     * Activate multiple lines. It's an asynchronous API.
     *
     * @param msisdn the line number
     */
    void activateMultiLines(in String[] msisdns, in IDigitsActionListener listener);

    /**
     * Deactivate multiple lines. It's an asynchronous API.
     *
     * @param msisdn the line number
     */
    void deactivateMultiLines(in String[] msisdns, in IDigitsActionListener listener);

    /**
     * Add a new line. It's an asynchronous API.
     *
     * @param msisdn the line number
     */
    void addNewLine(in String msisdn, in IDigitsActionListener listener);

    /**
     * Query service state. It's an synchronous API.
     *
     * @return service state
     */
    int getServiceState();

    /**
     * Set refresh token for AT. It's an synchronous API.
     *
     * @return service state
     */
    void setRefreshToken(String refreshToken);

}
