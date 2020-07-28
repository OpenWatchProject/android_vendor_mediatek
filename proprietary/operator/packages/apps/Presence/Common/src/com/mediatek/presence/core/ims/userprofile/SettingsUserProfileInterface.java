/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.mediatek.presence.core.ims.userprofile;

import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;

/**
 * User profile read from RCS settings database
 * 
 * @author JM. Auffret
 */
public class SettingsUserProfileInterface extends UserProfileInterface {
    /**
     * Constructor
     */
    public SettingsUserProfileInterface(int slotId) {
        super(slotId);
    }
    
    /**
     * Read the user profile
     * 
     * @return User profile
     */
    public UserProfile read() {
        // Read profile info from the database settings
        String username = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getUserProfileImsUserName(); 
        String homeDomain = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getUserProfileImsDomain();
        String privateID = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getUserProfileImsPrivateId();
        String password = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getUserProfileImsPassword();
        String realm = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getUserProfileImsRealm();
        String xdmServer = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getXdmServer();
        String xdmLogin = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getXdmLogin();
        String xdmPassword = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getXdmPassword();
        String imConfUri = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getImConferenceUri();
        String username_full = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getUserProfileImsUserName_full();// @tct-stack wuquan add for full public user identity but not only number part 
        String imMultiConfUri = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getImMultiConferenceUri();
         

        return new UserProfile(username, homeDomain, privateID, password, realm,
                xdmServer, xdmLogin, xdmPassword, imConfUri, username_full, imMultiConfUri);
    }
}
