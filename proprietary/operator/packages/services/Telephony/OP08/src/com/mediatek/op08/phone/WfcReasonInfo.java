package com.mediatek.op08.phone;

/**
 * This class enables an application to get details on why a WFC.
 * registration/call failed.
 * @hide
 */
public class WfcReasonInfo {

    /**
     * Specific code of each types.
     */
    public static final int CODE_UNSPECIFIED = 999;

    /* Registration Successful, No error code. */
    public static final int CODE_WFC_SUCCESS = 99;

    /* Code for WFC OFF & default errors, if any */
    public static final int CODE_WFC_DEFAULT = 100;

    // Incorrect SIM card used (ex: no UICC with GBA support being used)
    // The error will be displayed when the SIM card inserted is not GBA capable
    // or is not a TMO SIM card
    public static final int CODE_WFC_INCORRECT_SIM_CARD_ERROR = 1301;

    /**
     * Returns status code corresponding to WFC status.
     * @param status indicate WFC status
     * @return WFC error code
     */
    public static int getImsStatusCodeString(int status) {
        // WFC status string resounce id
        int resId = 0;
        switch (status) {
            case CODE_WFC_INCORRECT_SIM_CARD_ERROR:
                resId = R.string.wfc_incorrect_sim_card_error;
            default:
                break;
        }
        return resId;
    }

}
