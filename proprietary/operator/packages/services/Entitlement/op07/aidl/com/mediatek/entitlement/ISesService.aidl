package com.mediatek.entitlement;

import com.mediatek.entitlement.ISesServiceListener;

interface ISesService {

    /**
     * Called to start Entilement Check. Device activation is also done for first time.
     * The service will decide what to do by current state.
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     * @param retryTimer caller can indicate the timer for service to retry.
     *         This is meaningless if retryTimes equal to 0.
     * @param retryTimes caller can indicate the retry times for service to retry.
     *         0 means no need to retry
     */
    void startEntitlementCheck(in int slotId, in int retryTimer, in int retryTimes);

    /**
     * Called to cancel Entilement Check.
     * The service will return to the original state. (before startEntitlementCheck)
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     */
    void stopEntitlementCheck(in int slotId);

    /**
     * Called to update Location and T&C information.
     * The service will query latest URL/post-data and callback onWebsheetPost again.
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     */
    void updateLocationAndTc(in int slotId);

    /**
     * Called to get the entitlement state currently.
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     */
    String getCurrentEntitlementState(in int slotId);

    /**
     * Get the last one error code.
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     * @return the last one error code.
     */
    int getLastErrorCode(in int slotId);

    /**
     * Register listener for any SES event.
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     * @param listener the instance of ISesServiceStateListener
     */
    void registerListener(in int slotId, in ISesServiceListener listener);

    /**
     * Un-register listener from service.
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     * @param listener the instance of ISesServiceStateListener
     */
    void unregisterListener(in int slotId, in ISesServiceListener listener);

    /**
     * Deactivate a service. For Wi-Fi calling, the managePushToken REMOVE
     * procedure will be performed.
     *
     * @param slotId the sim slot id, e.g. 0 is for SIM 1 and 1 is for SIM 2
     */
    void deactivateService(in int slotId);
}
