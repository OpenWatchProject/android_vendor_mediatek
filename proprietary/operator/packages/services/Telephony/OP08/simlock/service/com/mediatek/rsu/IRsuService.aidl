package com.mediatek.rsu;

/**
* MTK RSU service API.
*/
interface IRsuService {
    /**
    * Request SLE Vendor Service Adapter for the shared symmetric key data that will be
    * sent to the Unlock Server for registration.
    *
    * @param data the input data that contains the IMEI
    *
    * @return the formatted shared symmetric key data or error
    */
    byte[] getRegisterRequest(in byte[] data);

    /**
    * Request Vendor Service Adapter to generate and return the formatted unlock
    * request that will be sent to the Unlock Server.
    *
    * @param data the input data that contains the unlock request type and the IMSI
    *
    * @return the formatted unlock request or error
    */
    byte[] createUnlockRequest(in byte[] data);

    /**
    * Request Vendor Service Adapter to process the input message that was received
    * from the Unlock Server.
    *
    * @param msg the message that needs to be processed by the SLE service
    * (e.g.: an unlock request)
    *
    * @param imei the device IMEI as bytes
    *
    * @return the formatted response that needs to be sent back to the RSU server or error
    */
    byte[] processMessage(in byte[] msg, in byte[] imei);

    /**
    * Retrieve the current SIM-Lock status from the SLE.
    *
    * @param data the input data is always null (used for future usages and compatibility)
    *
    * @return the lock state data (lock status and the remaining unlock time
    * in case it is temporarily unlocked) or error
    */
    byte[] getSimlockStatus(in byte[] data);

    /**
    * Retrieve the SLE vendor adapter configuration (Unlock Server URL and other specific
    * information for delivering the request messages to the server).
    *
    * @param data the input data that contains the caller version code
    *
    * @return the SLE vendor adapter configuration data
    */
    byte[] getAdapterConfig(in byte[] data);
}