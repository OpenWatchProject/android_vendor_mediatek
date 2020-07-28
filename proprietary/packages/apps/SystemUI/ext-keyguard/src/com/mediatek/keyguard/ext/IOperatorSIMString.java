package com.mediatek.keyguard.ext;

import android.content.Context;

/**
 *  Interface to define changing the SIM name in string resources according to operator.
 */
public interface IOperatorSIMString {
    /**
     * To distinguish the processing SIM string situations.
     * SIMTOUIM tag replaces SIM to UIM
     * UIMSIM tag replaces SIM to UIM/SIM
     * DELSIM tag replaces SIM to "" or a white space.
     */
    public enum SIMChangedTag {
        SIMTOUIM, UIMSIM, DELSIM
    };

    /**
     * Get the string with SIM or UIM according to Operator
     * @param sourceStr the source string
     * @param slotId the slot Id
     * @param simChangedTag the needed changed Tag. If none, set the value to -1.
     *
     * @internal
     */
    String getOperatorSIMString(String sourceStr, int slotId, SIMChangedTag simChangedTag,
            Context context);
}