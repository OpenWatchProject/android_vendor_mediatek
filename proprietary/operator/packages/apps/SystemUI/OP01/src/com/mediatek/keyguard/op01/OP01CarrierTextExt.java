package com.mediatek.keyguard.op01;

import com.mediatek.keyguard.ext.DefaultCarrierTextExt;

/**
 * Customize the carrier text for OP01.
 */
public class OP01CarrierTextExt extends DefaultCarrierTextExt {

    /**
     * The customized divider of carrier text.
     *
     * @param divider the current carrier text divider string.
     *
     * @return the customized carrier text divider string.
     */
    @Override
    public String customizeCarrierTextDivider(String divider) {
        String carrierDivider = " | ";
        return carrierDivider;
    }
}
