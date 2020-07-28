package com.mediatek.ims.ril;

import static com.android.internal.telephony.RILConstants.RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE;
import static com.android.internal.telephony.RILConstants.RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE;
import static com.android.internal.telephony.RILConstants.RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED;

import android.telephony.TelephonyManager;

public class RadioIndicationImpl extends RadioIndicationBase {

    RadioIndicationImpl(ImsRILAdapter ril, int phoneId) {
        mRil= ril;
    }

    // IMS RIL Instance
    private ImsRILAdapter mRil;

    /**
     * Indicates of enter emergency callback mode
     * URC: RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE
     * @param indicationType RadioIndicationType
     * @param radioState android.hardware.radio.V1_0.RadioState
     */
    @Override
    public void enterEmergencyCallbackMode(int indicationType) {

        mRil.processIndication(indicationType);
        if (ImsRILAdapter.IMS_RILA_LOGD)
            mRil.unsljLog(RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE);

        if (mRil.mEnterECBMRegistrants != null) {
            mRil.mEnterECBMRegistrants.notifyRegistrants();
        }
    }

    /**
     * Indicates of exit emergency callback mode
     * URC: RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE
     * @param indicationType RadioIndicationType
     * @param radioState android.hardware.radio.V1_0.RadioState
     */
    @Override
    public void exitEmergencyCallbackMode(int indicationType) {

        mRil.processIndication(indicationType);
        if (ImsRILAdapter.IMS_RILA_LOGD)
            mRil.unsljLog(RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE);

        if (mRil.mExitECBMRegistrants != null) {
            mRil.mExitECBMRegistrants.notifyRegistrants();
        }
    }

    /**
     * Indicates when radio state changes
     * URC: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
     * @param indicationType RadioIndicationType
     * @param radioState android.hardware.radio.V1_0.RadioState
     */
    @Override
    public void radioStateChanged(int type, int radioState)
    {

        mRil.processIndication(type);
        int newState = getRadioStateFromInt(radioState);

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogMore(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                    "radioStateChanged: " + newState);
        }

        mRil.setRadioState(newState);
        mRil.notifyRadioStateChanged(newState);
    }

    @Override
    protected void riljLoge(String msg) {
        mRil.riljLoge(msg);
    }

    /**
     * @param stateInt
     * @return {@link TelephonyManager.RadioPowerState RadioPowerState}
     */
    protected @TelephonyManager.RadioPowerState int getRadioStateFromInt(int stateInt) {
        int state;

        switch(stateInt) {
            case android.hardware.radio.V1_0.RadioState.OFF:
                state = TelephonyManager.RADIO_POWER_OFF;
                break;
            case android.hardware.radio.V1_0.RadioState.UNAVAILABLE:
                state = TelephonyManager.RADIO_POWER_UNAVAILABLE;
                break;
            case android.hardware.radio.V1_0.RadioState.ON:
                state = TelephonyManager.RADIO_POWER_ON;
                break;
            default:
                throw new RuntimeException("Unrecognized RadioState: " + stateInt);
        }
        return state;
    }
}
