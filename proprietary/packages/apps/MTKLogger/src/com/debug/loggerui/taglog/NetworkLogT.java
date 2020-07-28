package com.debug.loggerui.taglog;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.utils.Utils;

/**
 * @author MTK81255
 *
 */
public class NetworkLogT extends LogInstanceForTaglog {

    /**
     * @param logType int
     * @param tagLogInformation TagLogInformation
     */
    public NetworkLogT(int logType, TagLogInformation tagLogInformation) {
        super(logType, tagLogInformation);
    }

    @Override
    public boolean isNeedDoTag() {
        String currentMode =
                MyApplication.getInstance().getDefaultSharedPreferences()
                        .getString(Utils.KEY_MD_MODE_1, Utils.MODEM_MODE_SD);
        if (Utils.MODEM_MODE_PLS.equals(currentMode)) {
            Utils.logi(TAG, "isNeedZipNetworkLog ? false. For the modem is in PLS mode.");
            Utils.logd(TAG, "isNeedDoTag ? false. mLogType = " + mLogType);
            return false;
        }
        return super.isNeedDoTag();
    }

}
