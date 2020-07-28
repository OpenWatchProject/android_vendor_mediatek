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

package com.mediatek.rcse.settings;

import android.net.Uri;

/**
 * RCS settings data constants.
 */
public class AppSettingsData {
    /**
     * Database URI.
     */
    static final Uri CONTENT_URI = Uri
            .parse("content://com.mediatek.rcs.appsettings/appsettings");

    /**
     * Column name.
     */
    static final String KEY_ID = "_id";

    /**
     * Column name.
     */
    static final String KEY_KEY = "key";

    /**
     * Column name.
     */
    static final String KEY_VALUE = "value";

    // ---------------------------------------------------------------------------
    // Constants
    // ---------------------------------------------------------------------------

    /**
     * Boolean value "true".
     */
    public static final String TRUE = Boolean.toString(true);

    /**
     * Boolean value "false".
     */
    public static final String FALSE = Boolean.toString(false);

    // ---------------------------------------------------------------------------
    // UI settings
    // ---------------------------------------------------------------------------

    /**
     * Ringtone which is played when a social presence sharing invitation is
     * received.
     */
    public static final String PRESENCE_INVITATION_RINGTONE = "PresenceInvitationRingtone";

    /**
     * M: Add to achieve the RCS-e set compressing image feature. @{
     */
    /**
     * RCS-e compressing image.
     */
    public static final String RCSE_COMPRESSING_IMAGE = "RcseCompressingImage";
    public static final String RCSE_COMPRESSING_VIDEO = "RcseCompressingVideo";
    public static final String COMPRESS_IMAGE_HINT = "compress_image_hint";
    /**
     * @}
     */
    /**
     * M: Add to achieve the RCS-e Warning on Large image feature. @{
     */
    /**
     * RCS-e Large Size image.
     */
    public static final String WARNING_LARGE_IMAGE_HINT = "Warning_large_image_hint";
    
    /**
     * Display a warning if Store & Forward service is activated
     */
    public static final String WARN_SF_SERVICE = "StoreForwardServiceWarning";
    
    /**
     * Instant messaging is always on (Store & Forward server)
     */
    public static final String IM_CAPABILITY_ALWAYS_ON = "ImAlwaysOn";
    /**
     * @}
     */
    /** M: ftAutAccept @{ */
    /**
     * enable or disable auto-accept ft when roaming.
     */
    public static final String ENABLE_AUTO_ACCEPT_FT_ROMING = "ftAutAcceptWhenRoaming";

    /**
     * enable or disable auto-accept ft when no roaming.
     */
    public static final String ENABLE_AUTO_ACCEPT_FT_NOROMING = "ftAutAcceptWhenNoRoaming";

    /**
     * RCS-e chat wall paper.
     */
    public static final String RCSE_CHAT_WALLPAPER = "RcseChatWallpaper";

    /**
     * Ringtone which is played when a content sharing invitation is received.
     */
    public static final String CSH_INVITATION_RINGTONE = "CShInvitationRingtone";

    /**
     * Vibrate or not when a content sharing invitation is received.
     */
    public static final String CSH_INVITATION_VIBRATE = "CShInvitationVibrate";

    /**
     * Ringtone which is played when a file transfer invitation is received.
     */
    public static final String FILETRANSFER_INVITATION_RINGTONE = "FileTransferInvitationRingtone";

    /**
     * Vibrate or not when a file transfer invitation is received.
     */
    public static final String FILETRANSFER_INVITATION_VIBRATE = "FileTransferInvitationVibrate";

    /**
     * Ringtone which is played when a chat invitation is received.
     */
    public static final String CHAT_INVITATION_RINGTONE = "ChatInvitationRingtone";

    /**
     * Compress quality of image
     */
    public static final String IMAGE_COMPRESS_SIZE = "ImageCompressSize";

    /**
     * Vibrate or not when a chat invitation is received.
     */
    public static final String CHAT_INVITATION_VIBRATE = "ChatInvitationVibrate";

    /**
     * Satus of joyn master switch.
     */
    public static final String JOYN_DISABLE_STATUS = "JoynDisableStatus";

    public static final String JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED =
            "JoynMessagingDisabledFullyIntegrated";

}
