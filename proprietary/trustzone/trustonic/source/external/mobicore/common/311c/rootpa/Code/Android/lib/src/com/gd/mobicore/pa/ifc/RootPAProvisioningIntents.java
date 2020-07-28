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

package com.gd.mobicore.pa.ifc;

/**
 * A list of intent actions that the root PA can broadcast.
 */
public class RootPAProvisioningIntents {

//
/** Intents for starting up and connecting to services  */
//

    public static final String PROVISIONING_SERVICE="com.gd.mobicore.pa.service.PROVISIONING_SERVICE";
    public static final String DEVELOPER_SERVICE="com.gd.mobicore.pa.service.DEVELOPER_SERVICE";
    public static final String OEM_SERVICE="com.gd.mobicore.pa.service.OEM_SERVICE";

//
/** Execution status reporting Intents   */
//

/** Provisioning is ongoing. The intent contained additional data in the integer field pointed by STATUS */
    public static final String PROVISIONING_PROGRESS_UPDATE = "com.gd.mobicore.pa.service.PROVISIONING_PROGRESS_UPDATE";
/** root provisioning was attempted but failed - the intent will contain an error code */
    public static final String PROVISIONING_ERROR = "com.gd.mobicore.pa.service.PROVISIONING_ERROR";
/** root provisioning has completed, root and SP containers are available for use */
    public static final String FINISHED_ROOT_PROVISIONING = "com.gd.mobicore.pa.service.PROVISIONING_FINISHED";


/** this intent contains developer trustlet in it's extra data. The trustlet has been signed by SE */
    public static final String INSTALL_TRUSTLET = "com.gd.mobicore.pa.service.INSTALL_TRUSTLET";

//
/** Names of extra data field's for intents */
//

/** Additional state information in PROVISIONING_PROGRESS_UPDATE intent */
    public static final String STATE ="com.gd.mobicore.pa.ifc.State";
/** Error code field in PROVISIONING_ERROR intent */
    public static final String ERROR ="com.gd.mobicore.pa.ifc.Error";
/** Error code field in INSTALL_TRUSTLET intent */
    public static final String TRUSTLET ="com.gd.mobicore.pa.ifc.Trustlet";

//
/** possible values for STATE field */
//

/** root provisioning has started and a connection to SE is being established */
    public static final int CONNECTING_SERVICE_ENABLER=100;
    public static final int AUTHENTICATING_SOC=200;
/** connection to SE has been established during provisioning and the root container is being created */
    public static final int CREATING_ROOT_CONTAINER=300;
    public static final int AUTHENTICATING_ROOT=400;
/** connection to SE has been established during provisioning and the SP container is being created */
    public static final int CREATING_SP_CONTAINER=500;
    public static final int FINISHED_PROVISIONING=1000;
/** unregistering root container will be sent by SE after OemService.unregisterRootContainer is used */
    public static final int UNREGISTERING_ROOT_CONTAINER=3000;

	private RootPAProvisioningIntents() { }
}
