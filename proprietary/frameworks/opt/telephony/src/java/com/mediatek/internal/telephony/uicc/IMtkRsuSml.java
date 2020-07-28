package com.mediatek.internal.telephony.uicc;

import android.os.Message;
import android.telephony.Rlog;

/**
 * This class is responsible for providing the common exposed APIs.
 */
public abstract class IMtkRsuSml {
    private static final String TAG = "IMtkRsuSml";

    protected static final int REMOTE_SIM_UNLOCK_SUCCESS = 0;
    protected static final int REMOTE_SIM_UNLOCK_ERROR = 1;

    /**
    * Client calls this API to registerCallback, so it can receive asynchronous responses.
    *
    * @param cb
    * Instance of callback
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully registered callback with SimLock service;
    * SIMLOCK_ERROR  = 1. Failed to register call back;
    */
    public int registerCallback(Object cb) {
        loge("default registerCallback, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API de-registers the callback.
    *
    * @param cb
    * Instance of callback
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully deregistered callback with SimLock service;
    * SIMLOCK_ERROR  =  1. Failed to deregister call back;
    */
    public int deregisterCallback(Object cb) {
        loge("default deregisterCallback, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API gets Sim lock status of the device. Exposed to other modules without permission.
    * Common API for all RSU features.
    *
    * @param cb
    * message callback.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGetSimlockStatusEx(Object cb) {
        loge("default remoteSimlockGetSimlockStatusEx, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }

    // VzW RSU APIs - Start
    /**
    * This API sends lock/unlock blob (also known as Response Message) to Network Lock Module.
    * Note: Network Lock module use this blob to perform lock/unlock operation
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param simlockData
    * This contains simlock blob for lock and unlock operation
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockProcessSimlockData(int token, byte[] simlockData) {
        loge("default remoteSimlockProcessSimlockData, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API gets highest major/minor version of incoming blob (Response message) that is
    * supported by Network Lock Module.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGetVersion(int token) {
        loge("default remoteSimlockGetVersion, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API gets Sim lock status of the device.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGetSimlockStatus(int token) {
        loge("default remoteSimlockGetSimlockStatus, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API requests Network Lock Module to generate a Request Message.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param requestType
    * Indicates the type of request to generate.
    * Current supported value
    * SIMLOCK_REMOTE_GENERATE_BLOB_REQUEST_TYPE = 1;
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGenerateRequest(int token, int requestType) {
        loge("default remoteSimlockGenerateRequest, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API request Network Lock Module to start/stop unlock device timer to unlock the device
    * for the specific time. Temporary Unlocking allows device to have data connectivity to
    * download the blob.
    * The Network Lock Module unlocks the device only once per UICC power cycle.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param requestType
    * SIMLOCK_REMOTE_START_UNLOCK_TIMER = 1;
    * SIMLOCK_REMOTE_STOP_UNLOCK_TIMER = 2;
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockUnlockTimer(int token, int requestType) {
        loge("default remoteSimlockUnlockTimer, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }
    // VzW RSU APIs - End

    // KDDI RSU APIs - Start
    /**
    * This API sends lock/unlock blob (also known as Response Message) to Network Lock Module.
    * Note: Network Lock module use this blob to perform lock/unlock operation
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param data
    * This contains simlock blob for lock and unlock operation
    *
    * @param slotId
    * Identifier the SIM that the request to be sent: soltId = 0 (SIM1), slotId = 1 (SIM2)
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int setSimLockPolicyData(int token, byte[] data, int slotId) {
        loge("default setSimLockPolicyData, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API gets white list Sim lock data of the device.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param slotId
    * Identifier the SIM that the request to be sent: soltId = 0 (SIM1), slotId = 1 (SIM2)
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int getAllowedSimLockData(int token, int slotId) {
        loge("default getAllowedSimLockData, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API gets black list Sim lock data of the device.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param slotId
    * Identifier the SIM that the request to be sent: soltId = 0 (SIM1), slotId = 1 (SIM2)
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int getExcludedSimLockData(int token, int slotId) {
        loge("default getExcludedSimLockData, user should override this API");
        return REMOTE_SIM_UNLOCK_ERROR;
    }
    // KDDI RSU APIs - End

    // Tmobile RSU APIs - Start
    /**
    * Request SLE Vendor Service Adapter for the shared symmetric key data that will be
    * sent to the Unlock Server for registration.
    *
    * @param data the input data that contains the IMEI
    *
    * @param onComplete callback message
    */
    public void sleGetRegisterRequest(byte[] data, Message onComplete) {
        loge("default sleGetRegisterRequest, user should override this API");
    }

    /**
    * Request Vendor Service Adapter to generate and return the formatted unlock
    * request that will be sent to the Unlock Server.
    *
    * @param data the input data that contains the unlock request type and the IMSI
    *
    * @param onComplete callback message
    */
    public void sleCreateUnlockRequest(byte[] data, Message onComplete) {
        loge("default sleCreateUnlockRequest, user should override this API");
    }

    /**
    * Request Vendor Service Adapter to process the input message that was received
    * from the Unlock Server.
    *
    * @param msg the message that needs to be processed by the SLE service
    * (e.g.: an unlock request)
    *
    * @param imei the device IMEI as bytes
    *
    * @param onComplete callback message
    */
    public void sleProcessMessage(byte[] msg, byte[] imei, Message onComplete) {
        loge("default sleProcessMessage, user should override this API");
    }

    /**
    * Retrieve the current SIM-Lock status from the SLE.
    *
    * @param data the input data is always null (used for future usages and compatibility)
    *
    * @param onComplete callback message
    */
    public void sleGetSimlockStatus(byte[] data, Message onComplete) {
        loge("default sleGetSimlockStatus, user should override this API");
    }

    /**
    * Retrieve the SLE vendor adapter configuration (Unlock Server URL and other specific
    * information for delivering the request messages to the server).
    *
    * @param data the input data that contains the caller version code
    *
    * @param onComplete callback message
    */
    public void sleGetAdapterConfig(byte[] data, Message onComplete) {
        loge("default sleGetAdapterConfig, user should override this API");
    }
    // Tmobile RSU APIs - End

    private void loge(String s) {
        Rlog.e(TAG, "[RSU-SIMLOCK] " + s);
    }
}
