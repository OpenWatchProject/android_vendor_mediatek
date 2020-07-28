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
package com.orangelabs.rcs.core.ims.service.presence;

/**
 * Presence utility functions
 * 
 * @author jexa7410
 */
public class PresenceUtils {
    /**
     * RCS 2.0 video share feature tag
     */
    public final static String FEATURE_RCS2_VIDEO_SHARE = "org.gsma.videoshare";
    
    /**
     * RCS 2.0 image share feature tag
     */
    public final static String FEATURE_RCS2_IMAGE_SHARE = "org.gsma.imageshare";

    /**
     * RCS 2.0 file transfer feature tag
     */
    public final static String FEATURE_RCS2_FT = "org.openmobilealliance:File-Transfer";

    /**
     * RCS 2.0 chat feature tag
     */
             public final static String FEATURE_RCS2_CHAT = "org.openmobilealliance:IM-ChatSession";

       //RCS 2.0 Second feature tag comes in NOTIFY     
       public final static String FEATURE_RCS2_CHAT_2 = "org.openmobilealliance:ChatSession";
     
    //public final static String FEATURE_RCS2_CHAT = "org.openmobilealliance:IM-session";
       public final static String FEATURE_RCS2_STANDALONEMSG = "org.openmobilealliance:StandaloneMsg";
    
    /**
     * RCS 2.0 CS video feature tag
     */
    public final static String FEATURE_RCS2_CS_VIDEO = "org.3gpp.cs-videotelephony";
    
    //DISCOVERY VIA PRESENCE
    public final static String FEATURE_RCS2_DISCOVERY_VIA_PRESENCE = "org.3gpp.urn:urn-7:3gpp-application.ims.iari.rcse.dp";
    
    public final static String FEATURE_RCS2_FT_HTTP = "org.openmobilealliance:File-Transfer-HTTP";
/*
 * IP Voice Call (IR.92)
Service-id: org.3gpp.urn:urn-7:3gpp-service.ims.icsi.mmtel
Version: 1.0
Media capabilities: audio, duplex
Contact address type: tel / SIP URI
 * 
 * AND
 * 
 * IP Video Call (IR.94)
Service-id: org.3gpp.urn:urn-7:3gpp-service.ims.icsi.mmtel
Version: 1.0
Media capabilities: audio, video, duplex
 */
    public final static String FEATURE_RCS2_IP_VOICE_CALL = "org.3gpp.urn:urn-7:3gpp-service.ims.icsi.mmtel";
    
    public final static String FEATURE_RCS2_FT_THUMBNAIL =  "org.openmobilealliance:File-Transfer-thumb";
    
    
}
