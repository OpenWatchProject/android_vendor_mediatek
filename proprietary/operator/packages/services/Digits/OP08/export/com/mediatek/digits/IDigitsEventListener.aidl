package com.mediatek.digits;

import android.os.Bundle;

interface IDigitsEventListener {

    /**
     * callback when any events happened.
     *
     * @param event, generally could be:
     *         "not-entitled", "entitled", "failed", "pending"
     * @param extras pass extra information in a bundle.
     */
    oneway void onDigitsEvent(int event, int result, in Bundle extras);
}
