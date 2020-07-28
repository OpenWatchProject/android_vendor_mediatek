package com.mediatek.mms.appservice;

import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;

import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.ims.ImsManager;
import com.android.ims.ImsException;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkRILConstants;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;



import java.util.ArrayList;

/**
 * {@hide}
 *
 * Provide utility functionality to check flight mode, radio, locked and roaming state.
 * User cares about the mentioned state notification can use these APIs.
 *
 */
public class CellConnMgr {
    private static final String TAG = "CellConnMgr";

    /**
     * Bit mask: STATE_READY means the card is under ready state.
     *
     * @internal
     */
    public static final int STATE_READY = 0x00;

    /**
     * Bit mask: STATE_FLIGHT_MODE means under flight mode on.
     *
     * @internal
     */
    public static final int STATE_FLIGHT_MODE = 0x01;

    /**
     * Bit mask: STATE_RADIO_OFF means the card is under radio off state.
     *
     * @internal
     */
    public static final int STATE_RADIO_OFF = 0x02;

    /**
     * Bit mask: STATE_SIM_LOCKED means the card is under locked state.
     * @deprecated - keyguard dismiss has be defeature.
     */
    public static final int STATE_SIM_LOCKED = 0x04;

    /**
     * Bit mask: STATE_ROAMING means the card is under locked state.
     * @deprecated - not supported since no corresponding requirement.
     */
    public static final int STATE_ROAMING = 0x08;


     /**
     * Bit mask: STATE_NOIMSREG_FOR_CTVOLTE means the CT SIM card is under ims unavailable state
     * when ENHANCED_4G_MODE_ENABLED is enabled.
     */
    public static final int STATE_NOIMSREG_FOR_CTVOLTE = 0x10;

    private Context mContext;
    private static final String INTENT_SET_RADIO_POWER =
            "com.mediatek.internal.telephony.RadioManager.intent.action.FORCE_SET_RADIO_POWER";


    private static final String[] CT_NUMERIC = {"45502", "46003", "46011", "46012", "45507"};

    public static final boolean MTK_CTVOLTE_SUPPORT =
            SystemProperties.getInt("persist.vendor.mtk_ct_volte_support", 0) != 0;

    /**
     * To use the utility function, please create the object on your local side.
     *
     * @param context the indicated context
     *
     * @internal
     */
    public CellConnMgr(Context context) {
        mContext = context;

        if (mContext == null) {
            throw new RuntimeException(
                "CellConnMgr must be created by indicated context");
        }
    }

    /**
     * Query current state by indicated subscription and request type.
     *
     * @param subId indicated subscription
     * @param requestType the request type you cared
     *                    STATE_FLIGHT_MODE means that you would like to query if under flight mode.
     *                    STATE_RADIO_OFF means that you would like to query if this SIM radio off.
     *                    STATE_SIM_LOCKED will check flight mode and radio state first, and then
     *                                     check if under SIM locked state.
     *                    STATE_ROAMING will check flight mode and radio state first, and then
     *                                  check if under roaming.
     * @return a bit mask value composed by STATE_FLIGHT_MODE, STATE_RADIO_OFF, STATE_SIM_LOCKED and
     *         STATE_ROAMING.
     *
     * @internal
     */
    public int getCurrentState(int subId, int requestType) {
        int state = STATE_READY;

        // Query flight mode settings
        int flightMode = Settings.Global.getInt(
                mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, -1);

        // Query radio state (need to check if the radio off is set by users)
        boolean isRadioOff = !isRadioOn(subId) && isRadioOffBySimManagement(subId);

        // Query SIM state
        int slotId = SubscriptionManager.getSlotIndex(subId);
        TelephonyManager telephonyMgr = TelephonyManager.getDefault();
        boolean isLocked =
                (TelephonyManager.SIM_STATE_PIN_REQUIRED == telephonyMgr.getSimState(slotId)
                || TelephonyManager.SIM_STATE_PUK_REQUIRED == telephonyMgr.getSimState(slotId)
                || TelephonyManager.SIM_STATE_NETWORK_LOCKED == telephonyMgr.getSimState(slotId));

        // Query roaming state
        boolean isRoaming = false;

        Rlog.d(TAG, "[getCurrentState]subId: " + subId + ", requestType:" + requestType +
                "; (flight mode, radio off, locked, roaming) = ("
                + flightMode + "," + isRadioOff + "," + isLocked + "," + isRoaming + ")");

        switch (requestType) {
            case STATE_FLIGHT_MODE:
                state = ((flightMode == 1) ? STATE_FLIGHT_MODE : STATE_READY);
                break;

            case STATE_RADIO_OFF:
                state = ((isRadioOff) ? STATE_RADIO_OFF : STATE_READY);
                break;

            case STATE_SIM_LOCKED:
                state = (((flightMode == 1) ? STATE_FLIGHT_MODE : STATE_READY) |
                        ((isRadioOff) ? STATE_RADIO_OFF : STATE_READY) |
                        ((isLocked) ? STATE_SIM_LOCKED : STATE_READY));
                break;

            case STATE_ROAMING:
                // If fligt mode on/radio off and roaming state occurred continuously, means that
                // we need to show two dialog continously.
                // That is, need to indicate multiple state at the same time.
                state = (((flightMode == 1) ? STATE_FLIGHT_MODE : STATE_READY) |
                        ((isRadioOff) ? STATE_RADIO_OFF : STATE_READY) |
                        ((isRoaming) ? STATE_ROAMING : STATE_READY));

                break;

            default:
                state = ((flightMode == 1) ? STATE_FLIGHT_MODE : STATE_READY) |
                        ((isRadioOff) ? STATE_RADIO_OFF : STATE_READY) |
                        ((isLocked) ? STATE_SIM_LOCKED : STATE_READY) |
                        ((isRoaming) ? STATE_ROAMING : STATE_READY);
        }

        if (state == STATE_READY
            && (requestType & STATE_NOIMSREG_FOR_CTVOLTE) == STATE_NOIMSREG_FOR_CTVOLTE) {
             state = isImsUnavailableForCTVolte(subId) ? STATE_NOIMSREG_FOR_CTVOLTE : STATE_READY;
        }
        Rlog.d(TAG, "[getCurrentState] state:" + state);

        return state;
    }

    private boolean isRadioOffBySimManagement(int subId) {
        boolean result = true;
        try {
            IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));

            if (null == iTelEx) {
                Rlog.d(TAG, "[isRadioOffBySimManagement] iTelEx is null");
                return false;
            }

            result = iTelEx.isRadioOffBySimManagement(subId);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        Rlog.d(TAG, "[isRadioOffBySimManagement]  subId " + subId + ", result = " + result);
        return result;
    }


    private boolean isRadioOn(int subId) {
        Rlog.d(TAG, "isRadioOff verify subId " + subId);
        boolean radioOn = true;
        try {
            ITelephony iTel = ITelephony.Stub.asInterface(
                    ServiceManager.getService(Context.TELEPHONY_SERVICE));

            if (null == iTel) {
                Rlog.d(TAG, "isRadioOff iTel is null");
                return false;
            }

            radioOn = iTel.isRadioOnForSubscriber(subId, mContext.getOpPackageName());
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }

        Rlog.d(TAG, "isRadioOff subId " + subId + " radio on? " + radioOn);
        return radioOn;
    }

    private boolean isImsUnavailableForCTVolte(int subId) {
        if (MTK_CTVOLTE_SUPPORT == true) {
            int mainphoneId =
                    SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
            int phoneId = SubscriptionManager.getPhoneId(subId);
            boolean enable4G = true;
            boolean isCTClib = "OP09".equals(SystemProperties.get("persist.vendor.operator.optr"))
                           && "SEGC".equals(SystemProperties.get("persist.vendor.operator.seg"));

            if (isCTClib == false) {
                int settingsNetworkMode = Settings.Global.getInt(
                    mContext.getContentResolver(),
                    Settings.Global.PREFERRED_NETWORK_MODE + subId,
                    Phone.PREFERRED_NT_MODE);
                enable4G =
                    (settingsNetworkMode == TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA
                    || settingsNetworkMode == MtkRILConstants.NETWORK_MODE_LTE_TDD_ONLY);
                Rlog.d(TAG, "[isImsUnavailableForCTVolte] enable 4g = " + enable4G);
            }

            if (mainphoneId == phoneId && isCTCard(subId)
                && enable4G == true
                && (1 == Settings.Global.getInt(
                      mContext.getContentResolver(), Settings.Global.ENHANCED_4G_MODE_ENABLED, 0))
                && isImsReg(phoneId) == false) {
                Rlog.d(TAG, "isImsUnavailableForCTVolte ture");
                return true;
           }
        }
        return false;
    }

    private boolean isImsReg(int phoneId) {
        /* O_migration
        boolean isImsReg = false;
        try {
            ImsManager imsManager = ImsManager.getInstance(mContext, phoneId);
            isImsReg = imsManager.getImsRegInfo();
        } catch (ImsException e) {
            Rlog.d(TAG, "Get IMS register info fail.");
        }
        Rlog.d(TAG, "[isImsServiceAvailable] isImsReg = " + isImsReg);
        return isImsReg;*/
        return MtkTelephonyManagerEx.getDefault().isImsRegistered(phoneId);
    }

       /**
       * M: Return if the sim card is cmcc or not. @{
       * @param subId sub id identify the sim card
       * @return true if the sim card is cmcc
       */
       private static boolean isCTCard(int subId) {
          boolean result = false;
          String numeric = TelephonyManager.getDefault().getSimOperator(subId);
          for (String ct : CT_NUMERIC) {
              if (ct.equals(numeric)) {
                  result = true;
              }
          }
          Rlog.d(TAG, "isCTCard:" + result);
          return result;
      }

       
}
