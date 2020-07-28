/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony;

import android.net.LinkProperties;
import android.net.NetworkStats;

import android.os.Bundle;
import android.os.Message;
import android.telephony.CellInfo;
import android.telephony.ServiceState;
import android.telephony.RadioAccessFamily;

import com.mediatek.internal.telephony.FemtoCellInfo;
import com.mediatek.internal.telephony.PseudoCellInfo;

/**
 * Interface used to interact with the phone.  Mostly this is used by the
 * TelephonyManager class.  A few places are still using this directly.
 * Please clean them up if possible and use TelephonyManager insteadl.
 *
 */
@ProductApi
interface IMtkTelephonyEx {
    /// M: [Network][C2K] Add isInHomeNetwork interface. @{
    /**
     * Return whether in home area for the specific subscription id.
     *
     * @param subId the id of the subscription to be queried.
     * @return true if in home network
     */
    boolean isInHomeNetwork(int subId);

    /// @}

    // MTK-START: SIM
    /**
     * Get icc app family by slot id.
     * @param slotId slot id
     * @return the family type
     * @hide
     */
    int getIccAppFamily(in int slotId);

    /**
     * Returns Icc card type for the given sub id.
     * @param subId - sub id.
     * @return string for the Icc card type.
     */
    String getIccCardType(int subId);

    /**
     * Returns the result of isAppTypeSupported.
     * @param slotId - slot id.
     * @param appType - appType.
     * @return true or false.
     */
    boolean isAppTypeSupported(int slotId, int appType);

    /**
     * Returns the result of isTestIccCard.
     * @param slotId - slot id.
     * @return true or false.
     */
    boolean isTestIccCard(int slotId);

    /**
     * Returns the raw data of Icc ATR result.
     * @param subId - sub id.
     * @return string for Icc ATR.
     */
    String getIccAtr(int subId);

    /**
     * Returns the byte array for the result of SIM exchange IO.
     * @param subId - sub id.
     * @param fileID - file id.
     * @param command - command.
     * @param p1 - p1.
     * @param p2 - p2.
     * @param p3 - p3.
     * @param filePath - filePath.
     * @param data - data.
     * @param pin2 - pin2.
     * @return byte array.
     */
    byte[] iccExchangeSimIOEx(int subId, int fileID, int command,
            int p1, int p2, int p3, String filePath, String data, String pin2);

    /**
     * Returns the byte array for the result of SIM transparent IO.
     * @param slotId - slot id.
     * @param family - family.
     * @param fileID - fileID.
     * @param filePath - filePath.
     * @return byte array.
     */
    byte[] loadEFTransparent(int slotId, int family, int fileID, String filePath);

    /**
     * Returns the byte array for the result of SIM linear fixed IO.
     * @param slotId - slot id.
     * @param family - family.
     * @param fileID - fileID.
     * @param filePath - filePath.
     * @return byte array.
     */
    List<String> loadEFLinearFixedAll(int slotId, int family, int fileID, String filePath);

    /**
     * Set SIM power state.
     *
     * @param slotId SIM slot id
     * @param state SIM_POWER_STATE_SIM_OFF: SIM off, SIM_POWER_STATE_SIM_OFF: SIM on.
     * @return -1: SET_SIM_POWER_ERROR_NOT_SUPPORT, 0: SET_SIM_POWER_SUCCESS,
     * 54: SET_SIM_POWER_ERROR_NOT_ALLOWED, 11: SET_SIM_POWER_ERROR_SIM_ABSENT.
     * <p>Requires Permission:
     *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
     * Or the calling app has carrier privileges. @see #hasCarrierPrivileges.
     **/
    int setSimPower(int slotId, int state);

    /**
     * Get SIM on/off state.
     * @param slotId SIM slot id
     * @return -1: unknown, SIM_POWER_STATE_SIM_OFF: SIM off, SIM_POWER_STATE_SIM_OFF: SIM on.
     */
    int getSimOnOffState(int slotId);

    /**
     * Check whether SIM is in process of on/off state.
     * @param slotId SIM slot id
     * @return -1: not in executing, SIM_POWER_STATE_EXECUTING_SIM_ON: in process of on state,
     *         SIM_POWER_STATE_EXECUTING_SIM_OFF: in process of off state
     */
    int getSimOnOffExecutingState(int slotId);
    // MTK-END

    //MTK-START: SIM ME LOCK
    Bundle queryNetworkLock(int subId, int category);

    int supplyNetworkDepersonalization(int subId, String strPasswd);
    // MTK-END

    // MTK-START: CMCC DUAL SIM DEPENDENCY LOCK
    /**
     * Modem SML change feature.
     * This function will query the SIM state of the given slot. And broadcast
     * ACTION_UNLOCK_SIM_LOCK if the SIM state is in network lock.
     *
     * @param subId: Indicate which sub to query
     * @param needIntent: The caller can deside to broadcast ACTION_UNLOCK_SIM_LOCK or not
     *                    in this time, because some APs will receive this intent (eg. Keyguard).
     *                    That can avoid this intent to effect other AP.
     */
    void repollIccStateForNetworkLock(int subId, boolean needIntent);
    // MTK-END

    // MTK-START: MVNO
    /**
     * Returns the MVNO type for the given sub id.
     * @param subId - sub id.
     * @return string for the type.
     */
    String getMvnoMatchType(int subId);

    /**
     * Returns the MVNO pattern for the given sub id and type.
     * @param subId - sub id.
     * @param type - MVNO type.
     * @return string for the type.
     */
    String getMvnoPattern(int subId, String type);

    // MTK-START: SIM GBA
    /**
     * Request to run AKA authenitcation on UICC card by indicated family.
     *
     * @param slotId indicated sim id
     * @param family indiacted family category
     *        UiccController.APP_FAM_3GPP =  1; //SIM/USIM
     *        UiccController.APP_FAM_3GPP2 = 2; //RUIM/CSIM
     *        UiccController.APP_FAM_IMS   = 3; //ISIM
     * @param byteRand random challenge in byte array
     * @param byteAutn authenication token in byte array
     *
     * @return reponse paramenters/data from UICC
     *
     */
    byte[] simAkaAuthentication(int slotId, int family, in byte[] byteRand, in byte[] byteAutn);

    /**
     * Request to run GBA authenitcation (Bootstrapping Mode)on UICC card
     * by indicated family.
     *
     * @param slotId indicated sim id
     * @param family indiacted family category
     *        UiccController.APP_FAM_3GPP =  1; //SIM/USIM
     *        UiccController.APP_FAM_3GPP2 = 2; //RUIM/CSIM
     *        UiccController.APP_FAM_IMS   = 3; //ISIM
     * @param byteRand random challenge in byte array
     * @param byteAutn authenication token in byte array
     *
     * @return reponse paramenters/data from UICC
     *
     */
    byte[] simGbaAuthBootStrapMode(int slotId, int family, in byte[] byteRand, in byte[] byteAutn);

    /**
     * Request to run GBA authenitcation (NAF Derivation Mode)on UICC card
     * by indicated family.
     *
     * @param slotId indicated sim id
     * @param family indiacted family category
     *        UiccController.APP_FAM_3GPP =  1; //SIM/USIM
     *        UiccController.APP_FAM_3GPP2 = 2; //RUIM/CSIM
     *        UiccController.APP_FAM_IMS   = 3; //ISIM
     * @param byteNafId network application function id in byte array
     * @param byteImpi IMS private user identity in byte array
     *
     * @return reponse paramenters/data from UICC
     *
     */
    byte[] simGbaAuthNafMode(int slotId, int family, in byte[] byteNafId, in byte[] byteImpi);
    // MTK-END

    /**
     * Query if the radio is turned off by user.
     *
     * @param subId inidicated subscription
     *
     * @return true radio is turned off by user.
     *         false radio isn't turned off by user.
     *
     */
    boolean isRadioOffBySimManagement(int subId);

    /**
     * Query if FDN is enabled.
     *
     * @param subId inidicated subscription
     *
     * @return true FDN is enabled.
     *         false FDN is disabled.
     *
     */
    boolean isFdnEnabled(int subId);

    /**
     * Returns the observed cell information of the device using slotId.
     * @param slotId - slot id.
     * @return Bundle of cell location information.
     */
    Bundle getCellLocationUsingSlotId(int slotId);

    /// [SIM-C2K] @{
    /**
     * Get uim imsi by sub id.
     * @param callingPackage The package get UIM subscriber id.
     * @param subId subscriber id
     * @return uim imsi
     */
     String getUimSubscriberId(String callingPackage, int subId);

    /**
     * Get IccId by slotId.
     * @param callingPackage The package get SIM serial number.
     * @param slotId
     * @return Iccid
     * @hide
     */
    String getSimSerialNumber(String callingPackage, int slotId);
    /// [SIM-C2K] @}

    /**
     * Returns the MCC+MNC (mobile country code + mobile network code) of the
     * provider of the SIM for a particular subscription. 5 or 6 decimal digits
     * for GSM and CDMA applications.
     *
     * @param phoneId for which SimOperator is returned
     * @return MCCMNC array. array[0]: GSM MCCMNC array[1]: CDMA MCCMNC
     * If there is no GSM or CDMA MCCMNC, it is set "". It returns null for invalid
     * phoneId and returns "" when card is not in ready state.
     */
    String[] getSimOperatorNumericForPhoneEx(int phoneId);

    /**
     * Set phone radio type and access technology.
     *
     * @param rafs an RadioAccessFamily array to indicate all phone's
     *        new radio access family. The length of RadioAccessFamily
     *        must equal to phone count.
     * @return true if start setPhoneRat successfully.
     */
    boolean setRadioCapability(in RadioAccessFamily[] rafs);

    /**
     * Check if under capability switching.
     *
     * @return true if switching
     */
    boolean isCapabilitySwitching();

    /**
     * Get main capability phone id.
     * @return The phone id with highest capability.
     */
    int getMainCapabilityPhoneId();

    /**
     * Get IMS registration state by given sub-id.
     * @param subId The subId for query
     * @return true if IMS is registered, or false
     * @hide
     */
    boolean isImsRegistered(in int subId);

    /**
     * Get Volte registration state by given sub-id.
     * @param subId The subId for query
     * @return true if volte is registered, or false
     * @hide
     */
    boolean isVolteEnabled(in int subId);

    /**
     * Get WFC registration state by given sub-id.
     * @param subId The subId for query
     * @return true if wfc is registered, or false
     * @hide
     */
    boolean isWifiCallingEnabled(in int subId);

    // M: [LTE][Low Power][UL traffic shaping] @{
    /**
     * Set LTE access stratum urc report
     * @param enabled the LTE AS state URC report is enable or disable
     * @return true if enabled/disable urc report successfully.
     */
    boolean setLteAccessStratumReport(boolean enabled);

    /**
     * Set LTE uplink data transfer
     * @param isOn the LTE uplink data transfer is on or off
     * @param timeMillis the close timer
     * @return true if enabled/disable uplink data transfer successfully.
     */
    boolean setLteUplinkDataTransfer(boolean isOn, int timeMillis);

    /**
     * Get LTE access stratum state
     * @return unknown/idle/connected if abnormal mode/power saving mode candidate/normal power mode.
     */
    String getLteAccessStratumState();

    /**
     * Get if shared default type apn
     * @return true if is shared default type apn occurred.
     */
    boolean isSharedDefaultApn();
    // M: [LTE][Low Power][UL traffic shaping] @}

    // PHB START
    /**
     * This function is used to get SIM phonebook storage information
     * by sim id.
     *
     * @param simId Indicate which sim(slot) to query
     * @return int[] which incated the storage info
     *         int[0]; // # of remaining entries
     *         int[1]; // # of total entries
     *         int[2]; // # max length of number
     *         int[3]; // # max length of alpha id
     *
     */
    int[] getAdnStorageInfo(int subId);

    /**
     * This function is used to check if the SIM phonebook is ready
     * by sim id.
     *
     * @param simId Indicate which sim(slot) to query
     * @return true if phone book is ready.
     */
    boolean isPhbReady(int subId);
    // PHB END

    /**
     * Set RF test Configuration to defautl phone
     * @param phoneId The target phone id
     * @param config The configuration
     *  0: signal information is not available on all Rx chains
     *  1: Rx diversity bitmask for chain 0(primary antenna)
     *  2: Rx diversity bitmask for chain 1(secondary antenna) is available
     *  3: Signal information on both Rx chains is available
     */
    int[] setRxTestConfig(int phoneId, int config);

    /**
     * Query RF Test Result
     */
    int[] getRxTestResult(int phoneId);

    /**
     * Trigger actions for self activation scenario
     * @param action The action to take, defined in SaUtils.
     *  ACTION_ADD_DATA_SERVICE (0)
     *  ACTION_MO_CALL (1)
     * @param param The paremeters for the action
     * @param subId The subscription id for the action
     *
     * @return Return nagtive value if failed
     */
    int selfActivationAction(int action, in Bundle param, int subId);

    /**
     * Query the activation state
     * @param subId The subscription id to query
     *
     * @return Return the state defined in SaUtils.
     *  0: STATE_NONE (no activation required)
     *  1: STATE_ACTIVATED
     *  2: STATE_NOT_ACTIVATED
     *  -1: STATE_UNKNOWN
     */
    int getSelfActivateState(int subId);

    /**
     * Query the PCO 5 to 0 state
     * @param subId The subscription id to query
     *
     * @return Return the state defined in SaUtils.
     *  0: FIVETOZERO_NONE (no pco change form 5 to 0 required)
     *  1: FIVETOZERO_ACTIVATED
     *  -1: FIVETOZERO_UNKNOWN
     */
    int getPCO520State(int subId);

    /**
     * Request to exit emergency call back mode.
     *
     * @param subId the subscription ID
     * @return true if exist the emeregency call back mode sucessfully
     */
    boolean exitEmergencyCallbackMode(int subId);

    /**
     * Set anti pseudo cell information mode of the device using slotId.
     * @param slotId - slot id.
     * @param mode - Apc feature mode.
     *                0: apc off;
     *                1: apc on, not attach to the pseudo cell;
     *                2: apc on, attach to the pseudo cell
     * @param reportOn - if allow apc urc report,
     *               true: report URC, false: not report URC
     * @param reportInterval - URC report interval if observed  pseudo cell.
     */
    void setApcModeUsingSlotId(int slotId, int mode,
                    boolean reportOn, int reportInterval);


    /**
     * Returns the observed  pseudo cell information of the device using slotId.
     * @param slotId - slot id.
     * @return Bundle of pseudo cell information.
     */
    PseudoCellInfo getApcInfoUsingSlotId(int slotId);

    /**
     * Get CDMA subscription active status
     *
     * @return 1 if active, 0 if deactive
     */
    int getCdmaSubscriptionActStatus(in int subId);

    /*
     * VzW E911 requirement, set / get is last Ecc via IMS
     */
    void setIsLastEccIms(boolean val);
    boolean getIsLastEccIms();

    /**
     * Returns the result and response from RIL for oem request
     *
     * @param oemReq the data is sent to ril.
     * @param oemResp the respose data from RIL.
     * @return negative value request was not handled or get error
     *         0 request was handled succesfully, but no response data
     *         positive value success, data length of response
     * @deprecated, use invokeOemRilRequestRawBySlot for instead
     */
    int invokeOemRilRequestRaw(in byte[] oemReq, out byte[] oemResp);

    /**
     * Returns the result and response from RIL for oem request
     *
     * @param slotId slot id.
     * @param oemReq the data is sent to ril.
     * @param oemResp the respose data from RIL.
     * @return negative value request was not handled or get error
     *         0 request was handled succesfully, but no response data
     *         positive value success, data length of response
     */
    int invokeOemRilRequestRawBySlot(in int slotId, in byte[] oemReq, out byte[] oemResp);

    /*
     * For CDMA system UI display requirement. Check whether in CS call.
     */
    boolean isInCsCall(int phoneId);

    /**
     * Returns the all observed cell information of the device for specific slot id.
     *
     * @param slotId the id of the slot.
     * @param callingPackage The package get all cell info.
     * @return Returns the all observed cell information of the device for specific slot id.
     */
    List<CellInfo> getAllCellInfo(int phoneId, String callingPackage);

    /**
     * Get current located plmn.
     * @param phoneId phoneId
     * @return located plmn
     */
    String getLocatedPlmn(int phoneId);

    /**
     * Set Disable 2G.
     * @param phoneId the id of the phone.
     * @param mode enable or disable 2G.
     * @return false if exception occurs or true if the request is successfully sent.
     */
     boolean setDisable2G(int phoneId, boolean mode);

    /**
     * Get Disable 2G.
     * @param phoneId the id of the phone.
     * @return disable 2G state
     */
    int getDisable2G(int phoneId);

    /**
     * get femtocell list.
     * @param phoneId the id of the phone.
     * @return List of {FemtoCellInfo}; null if femtocell information is unavailable.
     */
    List<FemtoCellInfo> getFemtoCellList(int phoneId);

    /**
     * abort femtocell list.
     * @param phoneId the id of the phone.
     * @return false if exception occurs or true if the request is successfully sent.
     */
    boolean abortFemtoCellList(int phoneId);

    /**
     * Select femtocell.
     * @param phoneId the id of the phone.
     * @param femtocell information.
     * @return false if exception occurs or true if the request is successfully sent.
     */
    boolean selectFemtoCell(int phoneId, in FemtoCellInfo femtocell);

    /**
     * Query femto cell system selection mode
     * @param phoneId the id of the phone.
     * @return femtocell systen selection mode
     */
    int queryFemtoCellSystemSelectionMode(int phoneId);

    /**
     * Set femto cell system selection mode
     * @param phoneId the id of the phone.
     * @param mode specifies the preferred system selection mode
     * @return false if exception occurs or true if the request is successfully sent.
     */
    boolean setFemtoCellSystemSelectionMode(int phoneId, in int mode);

    /**
     * Cancel available networke
     * @param phoneId the id of the phone.
     * @return false if exception occurs or true if the request is successfully sent.
     */
    boolean cancelAvailableNetworks(int phoneId);

    /**
     * Unlock the device.
     * @param pwd The password to unlock the device
     * @return error code and remaining retries
     */
    int[] supplyDeviceNetworkDepersonalization(String pwd);

    /**
     * To get RSU lock state. This state is from Blob. this API is expected to be used by all RSU.
     * @param phoneId Which slot to query
     * @return RSU lock state in blob or error
     * @hide
     */
    int getSimLockStateForRSU(int phoneId);

    /**
     * Try to clean up the PDN connection of specific type
     *
     * @param phoneId the phone the request is sent to.
     * @param type the PDN type of the connection to be cleaned up.
     * @return false if exception occurs or true if the request is successfully sent.
     */
    boolean tearDownPdnByType(int phoneId, String type);

    /**
     * Try to establish the PDN connection of specific type
     *
     * @param phoneId the phone the request is sent to.
     * @param type the PDN type of the connection to be cleaned up.
     * @return false if exception occurs or true if the request is successfully sent.
     */
    boolean setupPdnByType(int phoneId, String type);

    /**
     * @param phoneId the phone the request is sent to.
     * @param callingPackage package name
     * Returns the service state information on specified phoneId.
     */
    ServiceState getServiceStateByPhoneId(int phoneId, String callingPackage);

    /// M: [Network][C2K] Sprint roaming control @{
    /**
     * Set the roaming enabling.
     *
     * @param phoneId the id of the phone.
     * @param config the configuration for roaming parameter:
     *            config[0]: phone id
     *            config[1]: international_voice_text_roaming (0,1)
     *            config[2]: international_data_roaming (0,1)
     *            config[3]: domestic_voice_text_roaming (0,1)
     *            config[4]: domestic_data_roaming (0,1)
     *            config[5]: domestic_LTE_data_roaming (1)
     * @return {@code true} if successed.
     */
    boolean setRoamingEnable(int phoneId, in int[] config);

    /**
     * Get the roaming enabling.
     *
     * @param phoneId the id of the phone.
     * @return the roaming enable configuration.
     */
    int[] getRoamingEnable(int phoneId);
    /// @}

    /**
     * Get suggested PLMN list for VSIM
     *
     * @param phoneId the id of the phone.
     * @return the string array which contains PLMNs.
     */
    String[] getSuggestedPlmnList(int phoneId, int rat, int num, int timer, String callingPackage);

   /**
    * Data usage update for network latency optimization.
    * @return NetworkStats
    */
   NetworkStats getMobileDataUsage(int phoneId);

   /**
    * Data usage update for network latency optimization.
    * @param phoneId phoneId
    * @param txBytes previous txBytesSum
    * @param txPkts previous txPktsSum
    * @param rxBytes previous rxBytesSum
    * @param rxPkts previous rxPktsSum
    */
   void setMobileDataUsageSum(int phoneId, long txBytes, long txPkts,
                long rxBytes, long rxPkts);

    /**
     * Identifies if the supplied phone number is an emergency number that matches a known
     * emergency number based on current locale, SIM card(s), Android database, modem, network,
     * or defaults for specific phone.
     *
     * <p>This method assumes that only dialable phone numbers are passed in; non-dialable
     * numbers are not considered emergency numbers. A dialable phone number consists only
     * of characters/digits identified by {@link PhoneNumberUtils#isDialable(char)}.
     *
     * <p>The subscriptions which the identification would be based on, are all the active
     * subscriptions, no matter which subscription could be used to create TelephonyManager.
     *
     * @param phoneId  - the phone to look up
     * @param number   - the number to look up
     * @return {@code true} if the given number is an emergency number based on current locale,
     * SIM card(s), Android database, modem, network or defaults; {@code false} otherwise.
     */
     boolean isEmergencyNumber(int phoneId, String number);
}
