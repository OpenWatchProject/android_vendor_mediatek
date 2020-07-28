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

/**
 * User profile interface
 * 
 * @author jexa7410
 */
public abstract class UserProfileInterface {
    protected int mSlotId = 0;
    /**
     * Constructor
     */
    public UserProfileInterface(int slotId) {
        mSlotId = slotId;
    }
    
    /**
     * Read the user profile
     * 
     * @return User profile
     */
    public abstract UserProfile read();
}
