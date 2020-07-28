package com.android.emailcommon.mail;

public class ServerCommandInfo {

    /**
     * Exchange command "Oof" information
     * @author MTK
     *
     */
    public class OofInfo {
        /**
         * MTK define:OofStatus:
         * use 0 point to server not support Oof;
         * use -1 point to IOException or network unavailable.
         * use -2 point to the account may be not initialized for sync oof.
         * */
        public static final int SERVER_NOT_SUPPORT_OOF = 0;
        public static final int NETWORK_SHUT_DOWN = -1;
        public static final int SYNC_OOF_UNINITIALIZED = -2;

        /**
         * OofStatus:
         * use 1 point to set or get Oof successful, protocol define;
         * use 2 point to set or get Oof fail, protocol define;
         */
        public static final int SET_OR_SAVE_SUCCESS = 1;
        public static final int SET_OR_SAVE_FAIL = 2;
        /**
         * OofState:
         * use 0 point to Oof is disabled of this account, protocol define;
         * use 1 point to Oof is global of this account, protocol define;
         * use 2 point to Oof is time based of this account, protocol define;
         */
        public static final int OOF_IS_DISABLED = 0;
        public static final int OOF_IS_GLOBAL = 1;
        public static final int OOF_IS_TIME_BASED = 2;
    }

}
