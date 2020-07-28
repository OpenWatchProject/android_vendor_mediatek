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
package com.gsma.services.rcs;

/**
 *
 *
 * @author MTK
 */
public class Permissions {

    /**
     * This is a only permission that governs access to the
     * all services API, and is required to use all RCS Services
     */
    public final static String RCS_USE_SERVICES = "com.gsma.services.permission.RCS";

    /**
     * This is a new permission that governs access to the
     * chat API, and is required both to receive and to send over an RCS
     * chat session
     */
    public final static String RCS_USE_CHAT = "com.gsma.services.rcs.RCS_USE_CHAT";

    /**
     * This is a new permission that that is required by a
     * client in order to read the chat history from the content provider
     */
    public final static String RCS_READ_CHAT = "com.gsma.services.rcs.RCS_READ_CHAT";

    /**
     * This is a new permission that is
     * required by a client in order to handle the receipt of a file transferred from a remote
     * party
     */
    public final static String RCS_FILETRANSFER_RECEIVE = "com.gsma.services.rcs.RCS_FILETRANSFER_RECEIVE";

    /**
     * This is a new permission that is
     * required by a client in order to initiate the transfer of a file transferred to a remote
     * party
     */
    public final static String RCS_FILETRANSFER_SEND = "com.gsma.services.rcs.RCS_FILETRANSFER_SEND";

    /**
     * This is a new permission that is
     * required by a client in order to read the file transfer history from the content provider
     */
    public final static String RCS_FILETRANSFER_READ = "com.gsma.services.rcs.RCS_FILETRANSFER_READ";

    /**
     * This is a new permission that is
     * required by a client in order to handle the receipt of a image shared by a remote
     */
    public final static String RCS_IMAGESHARE_RECEIVE = "com.gsma.services.rcs.RCS_IMAGESHARE_RECEIVE";

    /**
     * This is a new permission that is
     * required by a client in order to initiate the sharing of an image with a remote party
     */
    public final static String RCS_IMAGESHARE_SEND = "com.gsma.services.rcs.RCS_IMAGESHARE_SEND";

    /**
     * This is a new permission that is
     * required by a client in order to read the image share history from the content provider
     */
    public final static String RCS_IMAGESHARE_READ = "com.gsma.services.rcs.RCS_FILETRANSFER_READ";

    /**
     * This is a new permission that is
     * required by a client in order to receipt of a video shared by a remote party
     */
    public final static String RCS_VIDEOSHARE_RECEIVE = "com.gsma.services.rcs.RCS_VIDEOSHARE_RECEIVE";

    /**
     * This is a new permission that is
     * required by a client in order to initiate the sharing of an video with a remote party
     */
    public final static String RCS_VIDEOSHARE_SEND = "com.gsma.services.rcs.RCS_VIDEOSHARE_SEND";

    /**
     * This is a new permission that is
     * required by a client in order to read the video share history from the content provider
     */
    public final static String RCS_VIDEOSHARE_READ = "com.gsma.services.rcs.RCS_VIDEOSHARE_READ";

    /**
     * This is a new permission this is a new permission that governs
     * access to capability information
     */
    public final static String RCS_READ_CAPABILITIES = "com.gsma.services.rcs.RCS_READ_CAPABILITIES";

    /**
     * This is a new permission that governs access to the IP Call API, and is required to initiate,
     * receive and to manage RCS IP Call sessions.
     */
    public final static String RCS_USE_IPCALL = "com.gsma.services.rcs.RCS_USE_IPCALL";

    /**
     * This is a new permission that that is required by client in order to read the IP Call
     * history from the content provider..
     */
    public final static String RCS_READ_IPCALL = "com.gsma.services.rcs.RCS_READ_IPCALL";

    /**
     * this is a new permission that is required to
     * send Geolocation data over an RCS chat session
     */
    public final static String RCS_LOCATION_SEND = "com.gsma.services.rcs.RCS_LOCATION_SEND";

    /**
     * This is a new permission that that is required by client in order to read the using the
     * capabilities service, since use of the API implicitly reveals information about
     * past and current contacts for the device.
     */
    public final static String ANDROID_READ_CONTACTS = "android.permission.READ_CONTACTS";

    /**
     * This is a new permission that that is required by client analogous to READ_PHONE_STATE,
     * covering general access to the RCS stack state.
     */
    public final static String READ_RCS_STATE = "com.gsma.services.rcs.READ_RCS_STATE";

}
