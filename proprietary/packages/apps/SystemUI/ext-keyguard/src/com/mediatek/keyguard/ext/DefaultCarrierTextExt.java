package com.mediatek.keyguard.ext;

import android.content.Context;

/**
 * Default plugin implementation.
 */

public class DefaultCarrierTextExt implements ICarrierTextExt {

    @Override
    public CharSequence customizeCarrierTextCapital(CharSequence carrierText) {
        if (carrierText != null) {
            return carrierText.toString().toUpperCase();
        }
        return null;
    }

    @Override
    public CharSequence customizeCarrierText(CharSequence carrierText, CharSequence simMessage,
            int simId) {
        return carrierText;
    }

    @Override
    public boolean showCarrierTextWhenSimMissing(boolean isSimMissing, int simId) {
        return isSimMissing;
    }

    /**
     * For CT, display "No SERVICE" when CDMA card type is locked.
     *
     * @param carrierText
     *          the carrier text before customize.
     *
     * @param context
     *          the context of the application.
     *
     * @param phoneId
     *          the phone ID of the customized carrier text.
     *
     * @param isCardLocked
     *          whether is the card is locked.
     *
     * @return the right carrier text when card is locked.
     */
    @Override
    public CharSequence customizeCarrierTextWhenCardTypeLocked(
            CharSequence carrierText, Context context, int phoneId, boolean isCardLocked) {
        return carrierText;
    }

    /**
     * The customized carrier text when SIM is missing.
     *
     * @param carrierText the current carrier text string.
     *
     * @return the customized the carrier text.
     */
    @Override
    public CharSequence customizeCarrierTextWhenSimMissing(CharSequence carrierText) {
        return carrierText;
    }

    /**
     * The customized divider of carrier text.
     *
     * @param divider the current carrier text divider string.
     *
     * @return the customized carrier text divider string.
     */
    @Override
    public String customizeCarrierTextDivider(String divider) {
        return divider;
    }
}
