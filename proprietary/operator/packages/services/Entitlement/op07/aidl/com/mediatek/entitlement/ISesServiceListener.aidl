package com.mediatek.entitlement;

import android.os.Bundle;

interface ISesServiceListener {

    /**
     * callback when service has any events happened.
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     * @param event, generally could be:
     *         "not-entitled", "entitled", "failed", "pending"
     * @param extras pass extra information in a bundle.
     *         (e.g. {"tc-accepted": false, "emergency-address-filled": false} for 'pending',
     *               {"error-code": '1111'} for 'failed').
     */
    void onEntitlementEvent(int slotId, String event, in Bundle extras);

    /**
     * callback if needing to show a Websheet for entitlement procedure.
     *
     * Note: Entitlement service will always allow calling this method. UI needs to ensure
     *        to show no more than one Websheet.
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     * @param url string of the Websheet server url.
     * @param postData string of the post-data for websheet.
     */
    void onWebsheetPost(int slotId, String url, String postData);
}
