package com.mediatek.settings.ext;

import android.content.Context;
import android.util.Log;

public class DefaultStatusExt implements IStatusExt {

    private static final String TAG = "DefaultStatusExt";

    /**
     * Customize phone number based on SIM.
     * @param currentNumber current mobile number shared.
     * @param slotId slot id
     * @param context Activity contxt
     * @return String to display formatted number
     * @internal
     */
    public String updatePhoneNumber(String currentNumber, int slotId, Context context) {
        return currentNumber;
    }
}
