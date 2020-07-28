package com.mediatek.mdmlsample;

import com.mediatek.mdml.TRAP_TYPE;

/**
 * Created by MTK03685 on 2015/9/17.
 */
public class CommandProxyAction {  // Current use public for each member variable as structure object
    public CommandProxyActionType actionType;
    public long sessionID;
    public String serverName;
    public TRAP_TYPE trapType;
    public long msgID;  // single
    public long[] msgIDArray;  // multiple
}
