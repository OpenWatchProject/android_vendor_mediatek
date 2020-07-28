/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \addtogroup ROOTPA AIDL Interfaces
 * @{
 */
package com.gd.mobicore.pa.ifc;

import java.util.List;
import com.gd.mobicore.pa.ifc.CmpCommand;
import com.gd.mobicore.pa.ifc.CmpResponse;
import com.gd.mobicore.pa.ifc.CommandResult;
import com.gd.mobicore.pa.ifc.BooleanResult;
import com.gd.mobicore.pa.ifc.SPID;
import com.gd.mobicore.pa.ifc.Version;
import com.gd.mobicore.pa.ifc.SUID;
import com.gd.mobicore.pa.ifc.SPContainerStructure;
import com.gd.mobicore.pa.ifc.SPContainerStateParcel;

/**
* RootPAServiceIfc is intended for SP.PA use at the time of installing new TA. It provides means
* to communicate with content management TA (using CMP version 3), request SE to perform provisioning
* of root container and TA container and means to obtain some information on the MobiCore and its registry.
*
* The service is started by sending intent com.gd.mobicore.pa.service.PROVISIONING_SERVICE in binding.
*/
interface RootPAServiceIfc {

    /**
     * Checks if a root container is registered.
     * Lock must not be acquired before executing this method and it can not be
     * acquired while this method runs.
     *
     * @param result true if root container exists.
     * @return indication of successful completion
     */
    CommandResult isRootContainerRegistered(out BooleanResult result);

    /**
     * Checks if a specific SP container is registered.
     * Lock must not be acquired before executing this method and it can not be
     * acquired while this method runs.
     *
     * @param spid SPID of the Service Provider
     * @param result is true if the SP container exists.
     * @return indication of successful completion
     */
    CommandResult isSPContainerRegistered(in SPID spid, out BooleanResult result);

    /**
     * Returns the version of various components of the MobiCore OS and surrounding components.
     * Lock must not be acquired before executing this method and it can not be
     * acquired while this method runs.
     *
     * @param version the result code of the call is saved in this parameter.
     * @return indication of successful completion
     */
    CommandResult getVersion(out Version version);

    /**
     * Returns the SUID of the device.
     * Lock must not be acquired before executing this method and it can not be
     * acquired while this method runs.
     *
     * @param suid the result of the SUID is saved in this parameter.
     * @return indication of successful completion
     */
    CommandResult getDeviceId(out SUID suid);

    /**
     * Acquires an exclusive communication lock with the MobiCore CMTL and registers the UID as the owner of the lock.
     * Subsequent calls to acquire a lock by the same application with the same UID will succeed, but requests for a lock
     * by other applications with different UIDs will fail until the lock is released. A lock is released when
     * {@link #releaseLock(int)} is called or the lock times out 60 seconds after being acquired. Each subsequent
     * call to acquire a lock with the same UID will reset the lock timeout counter, so that it expires 60 seconds after
     * the most recent call.
     * @param uid a unique value for the application that wishes to acquire a lock (the user id of the process satisfies this)
     * @return indication of successful completion
     */
    CommandResult acquireLock(int uid);

    /**
     * Explicitly frees a lock.
     *
     * @param uid the user id of the process which is accessing.
     * @return indication of successful completion
     */
    CommandResult releaseLock(int uid);

    /**
     * Executes a list of CMP commands and returns the corresponding CMP responses.
     * If one of the commands result in an error then the following commands are
     * not executed anymore, unless the command object has "ignoreError" set true.
     * A lock is required to execute this method.
     *
     * Note that this causes only excution of the CMP command(s) and reads/stores the
     * secure objects when needed. It does not initiate any discussion with Servce Enabler
     * or any other network component with any of CMP commands.
     *
     * @param uid the user id of the process which is accessing.
     * @param commands the CMP commands to be executed
     * @param responses the CMP command responses
     * @return result code of the call
     */
     CommandResult executeCmpCommands(int uid, in List<CmpCommand> commands, out List<CmpResponse> responses);

    /**
     * Starts provisioning. What actually happens after calling this depends on the state of the system and the commands SE sends.
     * In normal situation, SE sends commands to create root container and SP container (indicated by spid) if they do not already
     * exist. If given spid is 0, only root container is created (if it does not already exist).
     * Tasks are performed asynchronously. Method returns immediately.
     * Intents are broadcast to indicate the progress of the provisioning. The result is also
     * sent via broadcast.
     *
     * Cannot be executed if the acquireLock is called. Release any lock before calling this
     * method. Also, this command acquires lock internally before executing and releases lock
     * when error occurs or provisioning is finished (just before sending FINISHED_ROOT_PROVISIONING intent) or after 1 minute timeout.
     *
     * The following intents are broadcast after calling doProvisioning:
     * <ul>
     *     <li>com.gd.mobicore.pa.service.PROVISIONING_PROGRESS_UPDATE: Sent when the progress is changing, status can be one of the following.
     *         <ul>
     *             <li>CONNECTING_SERVICE_ENABLER (id 100)</li>
     *             <li>AUTHENTICATING_SOC (id 200)</li>
     *             <li>CREATING_ROOT_CONTAINER (id 300)</li>
     *             <li>AUTHENTICATING_ROOT (id 400)</li>
     *             <li>CREATING_SP_CONTAINER (id 500)</li>
     *             <li>FINISHED_PROVISIONING (id 1000)</li>
     *         </ul>
     *     </li>
     *     <li>com.gd.mobicore.pa.service.FINISHED_ROOT_PROVISIONING: Sent when the provisioning is finished.</li>
     *     <li>com.gd.mobicore.pa.service.PROVISIONING_ERROR: Sent when an error has occured, also contains an error code.</li>
     * </ul>
     * Note that depending on the nature of th errors it is possible that more than one PROVISIONING_ERROR intents are sent
     * before the excution of provisioning is fully stopped. This depends a lot on whether SE can still continue execution.
     *
     * There are constants related to the intents in @ref RootPAProvisioningIntents
     *
     * Service Enabler is contacted and asked to perform the tasks, so the device has to be connected to network
     * in order for this to succeed.
     *
     * @param uid the user id of the process which is accessing.
     * @param spid the service provider id for which a SP container should be created
     * @return indication of successful start of provisioning thread (ROOTPA_OK) or an error code
     */

    CommandResult doProvisioning(int uid, in SPID spid);

    /**
     * Interrogates the SP container structure.
     * Lock must not be acquired before executing this method and it can not be
     * acquired while this method runs.
     *
     * @param spid provides [in] the id of the SP (SPCont)
     * @param cs [out] state of the sp container and a list of installed TA containers for the given SP
     * @return indication of successful completion
     */
    CommandResult getSPContainerStructure(in SPID spid, out SPContainerStructure cs);

    /**
     * Interrogates the state of an SP container.
     * Lock must not be acquired before executing this method and it can not be
     * acquired while this method runs.
     *
     * @param spid [in] service provider id to query
     * @param state [out] the state of the SP container
     * @return indication of successful completion
     */
    CommandResult getSPContainerState(in SPID spid, out SPContainerStateParcel state);


    /**
     * Stores the actual TA binary to registry.
     *
     * @param spid [in] service provider id
     * @param uuid [in] unique UUID of the TA
     * @param taBinary [in] the actual TA to be stored
     * @return indication of successful completion
     */
    CommandResult storeTA(in SPID spid, in byte[] uuid, in byte[] taBinary);
}

/**@}*/
