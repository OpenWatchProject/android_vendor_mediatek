package com.kddi.remoteSimlock;

import com.kddi.remoteSimlock.IKDDIRemoteSimlockServiceCallback;

interface IKDDIRemoteSimlockService {
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
    int registerCallback(in IKDDIRemoteSimlockServiceCallback cb);

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
    int deregisterCallback(in IKDDIRemoteSimlockServiceCallback cb);

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
    * @param soltId
    * Identifier the SIM that the request to be sent. soltId = 0 (SIM1), slotId = 1 (SIM2)
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    int setSimLockPolicyData(in int token, in byte[] simlockData, in int soltId);

    /**
    * This API to get SIM lock data of white list of the device.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param soltId
    * Identifier the SIM that the request to be sent. soltId = 0 (SIM1), slotId = 1 (SIM2)
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    int getAllowedSimLockData(int token, int slotId);

    /**
    * This API to get SIM lock data of excluded list of the device.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param soltId
    * Identifier the SIM that the request to be sent. soltId = 0 (SIM1), slotId = 1 (SIM2)
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    int getExcludedSimLockData(int token, int slotId);

}
