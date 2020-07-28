package com.mediatek.keyguard.ext;

import android.content.Context;

/**
 * Carrier text related interface
 */
public interface ICarrierTextExt {

    /**
     * Convert the carrier string to upper case.
     *
     * @param CarrierText The carrier String.
     * @return to upper case Carrier String.
     *
     * @internal
     */
    CharSequence customizeCarrierTextCapital(CharSequence CarrierText);

    /**
     * For CU, display "No SIM CARD" without "NO SERVICE" when
     * there is no sim card in device and carrier's service is
     * ready.
     *
     * @param simMessage
     *          the first part of common carrier text
     * @param original
     *          common carrier text
     * @param simId
     *          current sim id
     *
     * @return ture if sim is in service
     *
     * @internal
     */
    CharSequence customizeCarrierText(CharSequence CarrierText, CharSequence simMessage,
            int simId);

    /**
     * The carrier texts are displayed or not depending on the SIM card inserted status.
     * The default rule is to hide the carrier text if the SIM card is missing.
     * Operator plugins may override this function to customize their rules.
     *
     * For CT customization.
     * Don't hide carrier text even there is no sim card inserted.
     *
     * @param isMissing true if there is no sim card inserted.
     * @param simId  sim card id.
     *
     * @return whether hide carrier text
     *
     *
     * For CU 5.0 feature when simNum==2,one Servicestate is not in service,
     * just show sim one infomation
     *
     * @param isSimMissing: simMissing(not insert simcard)
     * return ture if one sim is out of service && no sim card is pin locked && two
     * sim is not missing
     *
     * @internal
     */
    boolean showCarrierTextWhenSimMissing(boolean isSimMissing, int simId);

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
     *
     * @internal
     */
    CharSequence customizeCarrierTextWhenCardTypeLocked(
            CharSequence carrierText, Context context, int phoneId, boolean isCardLocked);

    /**
     * The customized carrier text when SIM is missing.
     *
     * @param carrierText the current carrier text string.
     *
     * @return the customized the carrier text.
     *
     * @internal
     */
    CharSequence customizeCarrierTextWhenSimMissing(CharSequence carrierText);

    /**
     * The customized divider of carrier text.
     *
     * @param divider the current carrier text divider string.
     *
     * @return the customized carrier text divider string.
     *
     * @internal
     */
    String customizeCarrierTextDivider(String divider);
}