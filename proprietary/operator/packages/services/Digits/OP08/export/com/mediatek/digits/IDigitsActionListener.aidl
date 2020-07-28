package com.mediatek.digits;

import android.os.Bundle;

interface IDigitsActionListener {

    /** The operation succeeded */
    oneway void onSuccess(in Bundle extras);

    /**
     * The operation failed
     * @param reason The reason for failure
     * @param extras pass extra information in a bundle.
     */
    oneway void onFailure(int reason, in Bundle extras);
}
