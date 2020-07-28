/*
 * Copyright (c) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.ims.internal;

import android.telephony.ims.ImsReasonInfo;
import android.net.Uri;

/**
 * {@hide}
 */
interface IMtkImsRegistrationListener {
    /**
     * Hook listener for IMS registration state report indication
     * @param state MtkImsConstants.IMS_REGISTERING/IMS_REGISTERED/IMS_REGISTER_FAIL
     * @param uris The profile URIs
     * @param expireTime The expire time of the IMS registration connection. 0 denote registering
     * @param imsReasonInfo Reason for the failure
     */
    void onRegistrationImsStateChanged(int state, in Uri[] uris, int expireTime, in ImsReasonInfo imsReasonInfo);

    /**
     * Redirect incoming call for other listener
     * @param phoneId incoming call from which phone
     * @param ar other informations
     */
    void onRedirectIncomingCallIndication(int phoneId, in String[] info);
}