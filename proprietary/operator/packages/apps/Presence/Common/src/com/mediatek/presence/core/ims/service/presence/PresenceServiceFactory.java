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

package com.mediatek.presence.core.ims.service.presence;

import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.utils.SimUtils;

public class PresenceServiceFactory {
    public static PresenceService newInstance(ImsModule parent) throws CoreException {
        int slotId = parent.getSlotId();
        if (SimUtils.isAttSimCard(slotId)) {
            return new PresenceServiceEx(parent);
        }
        else if (SimUtils.isVzwSimCard(slotId)) {
            return new VzwPresenceService(parent);
        } else {
            return new PresenceService(parent);
        }
    }
}
