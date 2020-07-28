package com.mediatek.mdmlsample;

import com.mediatek.mdml.TRAP_TYPE;

/**
 * Created by MTK03685 on 2015/9/14.
 */
public enum CommandProxyActionType {
    ACTION_TYPE_CREATE_SESSION,
    ACTION_TYPE_CLOSE_SESSION,
    ACTION_TYPE_SET_TRAP_RECEIVER,
    ACTION_TYPE_CREATE_SESSION_AND_SET_TRAP_RECEIVER,
    ACTION_TYPE_ENABLE_TRAP,
    ACTION_TYPE_DISABLE_TRAP,
    ACTION_TYPE_SUBSCRIBE_TRAP,
    ACTION_TYPE_UNSUBSCRIBE_TRAP,
    ACTION_TYPE_ENABLE_MDM,
    /* Add new type below */

    /* ------------------ */
    ACTION_TYPE_END;

}
