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

import com.gd.mobicore.pa.ifc.CommandResult;

/**
* RootPAOemIfc is intended to be used by OEM only. It provides means to initiate unregistering root container.
*
* The service is started by sending intent com.gd.mobicore.pa.service.OEM_SERVICE in binding. The service is
* protected by permission com.gd.mobicore.pa.permission.OEM_PERMISSION with protectionLevel "signatureOrSystem"
*/
interface RootPAOemIfc{

    /**
     * For OEM testing purposes only. Contacts Service Enabler and requests it to unregister the root container,
     * so the device has to be connected to network for the call to succeed. The command returns almost immediately
     * and executes in a separate thread, the same status Intents and values as are returned with
     * @ref RootPAServiceIfc#doProvisioning can be returned when calling this methods. With the addition of value:
     * <ul>
     *     <li>UNREGISTERING_ROOT_CONTAINER (id 3000)</li>
     * </ul>
     *
     * There are constants related to the intents in @ref RootPAProvisioningIntents
     *
     * @return indication of successful start of provisioning thread (ROOTPA_OK) or an error code
     */
    CommandResult unregisterRootContainer();
}
/**@}*/

