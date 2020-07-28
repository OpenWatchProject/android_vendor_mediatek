package com.mediatek.settings.ext;

import android.content.Context;

public interface IStatusExt {

    /**
     * Customize phone number based on SIM.
     * @param currentNumber current mobile number shared.
     * @param slotId slot id
     * @param context Activity contxt
     * @return String to display formatted number
     * @internal
     */
    String updatePhoneNumber(String currentNumber, int slotId, Context context);
}
