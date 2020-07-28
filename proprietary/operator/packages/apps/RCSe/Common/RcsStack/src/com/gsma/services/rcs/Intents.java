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
 * Intents related to joyn service activities
 *
 * @author Jean-Marc AUFFRET
 */
public class Intents {
    /**
     * Intents for joyn client
     */
    public static class Client {
        /**
         * Intent to load the settings activity to enable or disable the client
         */
        public static final String ACTION_VIEW_SETTINGS = "com.gsma.services.rcs.action.VIEW_SETTINGS";

        /**
         * Intent to request the client status. The result is received via an Intent
         * having the following extras:
         * <ul>
         * <li> {@link #EXTRA_CLIENT} containing the client package name.
         * <li> {@link #EXTRA_STATUS} containing the boolean status of the client. True
         *  means that the client is activated, else the client is not activated.
         */
        public static final String ACTION_CLIENT_GET_STATUS = ".client.action.GET_STATUS";

        /**
         * Client package name
         */
        public final static String EXTRA_CLIENT = "client";

        /**
         * Client status
         */
        public final static String EXTRA_STATUS = "status";

        public final static String SERVICE_UP = "com.gsma.services.rcs.action.SERVICE_UP";

        private Client() {
        }
    }

 /**
     * Intents for RCS service
     */
    public static class Service {
        /**
         * Intent to check if stack deactivation/activation is allowed by the client
         */
        public static final String ACTION_GET_ACTIVATION_MODE_CHANGEABLE = "com.gsma.services.rcs.action.GET_ACTIVATION_MODE_CHANGEABLE";

        /**
         * Used as an boolean extra field in ACTION_GET_ACTIVATION_MODE_CHANGEABLE intent to request
         * the activation mode changeable.
         */
        public static final String EXTRA_GET_ACTIVATION_MODE_CHANGEABLE = "get_activation_mode_changeable";

        /**
         * Intent to check if RCS stack is activated
         */
        public static final String ACTION_GET_ACTIVATION_MODE = "com.gsma.services.rcs.action.GET_ACTIVATION_MODE";

        /**
         * Used as an boolean extra field in ACTION_GET_ACTIVATION_MODE intent to request the
         * activation mode.
         */
        public static final String EXTRA_GET_ACTIVATION_MODE = "get_activation_mode";

        /**
         * Intent to set the activation mode of the RCS stack
         */
        public static final String ACTION_SET_ACTIVATION_MODE = "com.gsma.services.rcs.action.SET_ACTIVATION_MODE";

        /**
         * Used as an boolean extra field in ACTION_SET_ACTIVATION_MODE intent to set the activation
         * mode.
         */
        public static final String EXTRA_SET_ACTIVATION_MODE = "set_activation_mode";

        /**
         * Intent to check if RCS stack is compatible with RCS API
         */
        public static final String ACTION_GET_COMPATIBILITY = "com.gsma.services.rcs.action.GET_COMPATIBILITY";

        /**
         * Used as a string extra field in ACTION_GET_COMPATIBILITY intent to convey the codename
         */
        public static final String EXTRA_GET_COMPATIBILITY_CODENAME = "get_compatibility_codename";

        /**
         * Used as an integer extra field in ACTION_GET_COMPATIBILITY intent to convey the version
         */
        public static final String EXTRA_GET_COMPATIBILITY_VERSION = "get_compatibility_version";

        /**
         * Used as an integer extra field in ACTION_GET_COMPATIBILITY intent to convey the increment
         */
        public static final String EXTRA_GET_COMPATIBILITY_INCREMENT = "get_compatibility_increment";

        /**
         * Used as an boolean extra field in ACTION_GET_COMPATIBILITY intent to convey the response
         */
        public static final String EXTRA_GET_COMPATIBILITY_RESPONSE = "get_compatibility_response";

        /**
         * Used as an string extra field in ACTION_GET_COMPATIBILITY intent to convey the service
         * class name
         */
        public static final String EXTRA_GET_COMPATIBILITY_SERVICE = "get_compatibility_service";

        /**
         * Intent to get the RCS service starting state.
         */
        public static final String ACTION_GET_SERVICE_STARTING_STATE = "com.gsma.services.rcs.action.GET_SERVICE_STARTING_STATE";

        /**
         * Used as a boolean extra field in ACTION_GET_SERVICE_STARTING_STATE intent to convey the
         * response.
         */
        public static final String EXTRA_GET_SERVICE_STARTING_STATE = "get_service_starting_state";

        private Service() {
        }
    }

    /**
     * Intents for chat service
     */
    public static class Chat {
        /**
         * Load the chat application to view a chat conversation. This
         * Intent takes into parameter an URI on the chat conversation
         * (i.e. content://chats/chat_ID). If no parameter found the main
         * entry of the chat application is displayed.
         */
        public static final String ACTION_VIEW_CHAT = "com.gsma.services.rcs.action.VIEW_CHAT";

        /**
         * Load the chat application to start a new conversation with a
         * given contact. This Intent takes into parameter a contact URI
         * (i.e. content://contacts/people/contact_ID). If no parameter the
         * main entry of the chat application is displayed.
         */
        public static final String ACTION_INITIATE_CHAT = "com.gsma.services.rcs.action.INITIATE_CHAT";

        /**
         * Load the group chat application. This Intent takes into parameter an
         * URI on the group chat conversation (i.e. content://chats/chat_ID). If
         * no parameter found the main entry of the group chat application is displayed.
         */
        public static final String ACTION_VIEW_GROUP_CHAT = "com.gsma.services.rcs.action.VIEW_GROUP_CHAT";

        /**
         * Load the group chat application to start a new conversation with a
         * group of contacts. This Intent takes into parameter a list of contact
         * URIs. If no parameter the main entry of the group chat application is displayed.
         */
        public static final String ACTION_INITIATE_GROUP_CHAT = "com.gsma.services.rcs.action.INITIATE_GROUP_CHAT";

        private Chat() {
        }
    }

    /**
     * Intents for file transfer service
     */
    public static class FileTransfer {
        /**
         * Load the file transfer application to view a file transfer. This Intent
         * takes into parameter an URI on the file transfer (i.e. content://filetransfers/ft_ID).
         * If no parameter found the main entry of the file transfer application is displayed.
         */
        public static final String ACTION_VIEW_FT = "com.gsma.services.rcs.action.VIEW_FT";

        /**
         * Load the file transfer application to start a new file transfer to a given
         * contact. This Intent takes into parameter a contact URI (i.e. content://contacts/people/contact_ID).
         * If no parameter the main entry of the file transfer application is displayed.
         */
        public static final String ACTION_INITIATE_FT = "com.gsma.services.rcs.action.INITIATE_FT";

        private FileTransfer() {
        }
    }

 
}

