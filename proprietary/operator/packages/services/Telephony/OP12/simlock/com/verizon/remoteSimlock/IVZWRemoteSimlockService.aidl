package com.verizon.remoteSimlock;

import com.verizon.remoteSimlock.IVZWRemoteSimlockServiceCallback;

interface IVZWRemoteSimlockService {
    /**
    * Client calls this API to registerCallback, so it can receive asynchronous responses
    *
    * @param cb
    * Instance of callback
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully registered callback with SimLock service;
    * SIMLOCK_ERROR  = 1. Failed to register call back;
    */
    int registerCallback(in IVZWRemoteSimlockServiceCallback cb);

    /**
    * This API de-registers the callback
    *
    * @param cb
    * Instance of callback
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully deregistered callback with SimLock service;
    * SIMLOCK_ERROR  =  1. Failed to deregister call back;
    */
    int deregisterCallback(in IVZWRemoteSimlockServiceCallback cb);

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
    int remoteSimlockProcessSimlockData(in int token, in byte[] simlockData);

    /**
    * This API gets highest major/minor version of incoming blob (Response message) that is
    * supported by Network Lock Module
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    int remoteSimlockGetVersion(in int token);

    /**
    * This API gets Sim lock status of the device
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    int remoteSimlockGetSimlockStatus(in int token);

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
    int remoteSimlockGenerateRequest(in int token, in int requestType);

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
    int remoteSimlockUnlockTimer(in int token, in int requestType);
}
