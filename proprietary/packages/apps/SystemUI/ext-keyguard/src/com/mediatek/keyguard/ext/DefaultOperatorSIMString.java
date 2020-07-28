package com.mediatek.keyguard.ext;

import android.content.Context;


/**
 * Default plugin implementation.
 */
public class DefaultOperatorSIMString implements IOperatorSIMString {
    @Override
    public String getOperatorSIMString(String sourceStr, int slotId, SIMChangedTag simChangedTag,
            Context context) {
        return sourceStr;
    }
}
