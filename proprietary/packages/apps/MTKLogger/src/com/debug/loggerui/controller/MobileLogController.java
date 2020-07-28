package com.debug.loggerui.controller;

import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandler;
import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.LogHandlerUtils.MobileLogSubLog;

import java.util.HashMap;
import java.util.Map;

/**
 * @author MTK81255
 *
 */
public class MobileLogController extends AbstractLogController {

    private static final String TAG = Utils.TAG + "/MobileLogController";

    /**
     * Command prefix to configure sub log value.
     */
    private static final String ADB_COMMAND_SET_SUBLOG_PREFIX = "set_sublog_";

    public static MobileLogController sInstance = new MobileLogController(LogType.MOBILE_LOG);

    private MobileLogController(LogType logType) {
        super(logType);
    }

    public static MobileLogController getInstance() {
        return sInstance;
    }

    /**
     * @param enable
     *            boolean
     * @param subMobileLogInt
     *            int
     * @return boolean
     */
    public boolean setSubLogEnable(boolean enable, int subMobileLogInt) {
        MobileLogSubLog mobileLogSubLog = convertToSubMobileLogObject(subMobileLogInt);
        if (mobileLogSubLog != null) {
            return setSubLogEnable(enable, mobileLogSubLog);
        } else {
            Utils.loge(TAG, "When setSubLogEnable(), fail to get MobileLogSubLog"
                    + " of subMobileLogInt = " + subMobileLogInt);
            return false;
        }
    }

    /**
     * @param enable
     *            boolean
     * @param subMobileLog
     *            MobileLogSubLog
     * @return boolean
     */
    public boolean setSubLogEnable(boolean enable, MobileLogSubLog subMobileLog) {
        return LogHandler.getInstance().setSubMobileLogEnable(subMobileLog, enable);
    }

    private MobileLogSubLog convertToSubMobileLogObject(int subMobileLogInt) {
        switch (subMobileLogInt) {
        case 1:
            return MobileLogSubLog.AndroidLog;
        case 2:
            return MobileLogSubLog.KernelLog;
        case 3:
            return MobileLogSubLog.SCPLog;
        case 4:
            return MobileLogSubLog.ATFLog;
        case 5:
            return MobileLogSubLog.BSPLog;
        case 6:
            return MobileLogSubLog.MmediaLog;
        case 7:
            return MobileLogSubLog.SSPMLog;
        case 8:
            return MobileLogSubLog.ADSPLog;
        case 9:
            return MobileLogSubLog.MCUPMLog;
        default:
            return null;
        }
    }

    public static final Map<MobileLogSubLog, String> SUB_LOG_SETTINGS_ID_MAP =
            new HashMap<MobileLogSubLog, String>();
    static {
        SUB_LOG_SETTINGS_ID_MAP.put(MobileLogSubLog.AndroidLog, "mobilelog_androidlog");
        SUB_LOG_SETTINGS_ID_MAP.put(MobileLogSubLog.KernelLog, "mobilelog_kernellog");
        SUB_LOG_SETTINGS_ID_MAP.put(MobileLogSubLog.SCPLog, "mobilelog_scplog");
        SUB_LOG_SETTINGS_ID_MAP.put(MobileLogSubLog.ATFLog, "mobilelog_atflog");
        SUB_LOG_SETTINGS_ID_MAP.put(MobileLogSubLog.BSPLog, "mobilelog_bsplog");
        SUB_LOG_SETTINGS_ID_MAP.put(MobileLogSubLog.MmediaLog, "mobilelog_mmedialog");
        SUB_LOG_SETTINGS_ID_MAP.put(MobileLogSubLog.SSPMLog, "mobilelog_sspmlog");
        SUB_LOG_SETTINGS_ID_MAP.put(MobileLogSubLog.ADSPLog, "mobilelog_adsp_log");
        SUB_LOG_SETTINGS_ID_MAP.put(MobileLogSubLog.MCUPMLog, "mobilelog_mcupm_log");
    }

    public static final Map<MobileLogSubLog, String> SUB_LOG_CONFIG_STRING_MAP =
            new HashMap<MobileLogSubLog, String>();
    static {
        SUB_LOG_CONFIG_STRING_MAP.put(MobileLogSubLog.AndroidLog,
                "com.mediatek.log.mobile.AndroidLog");
        SUB_LOG_CONFIG_STRING_MAP.put(MobileLogSubLog.KernelLog,
                "com.mediatek.log.mobile.KernelLog");
        SUB_LOG_CONFIG_STRING_MAP.put(MobileLogSubLog.SCPLog, "com.mediatek.log.mobile.SCPLog");
        SUB_LOG_CONFIG_STRING_MAP.put(MobileLogSubLog.ATFLog, "com.mediatek.log.mobile.ATFLog");
        SUB_LOG_CONFIG_STRING_MAP.put(MobileLogSubLog.BSPLog, "com.mediatek.log.mobile.BSPLog");
        SUB_LOG_CONFIG_STRING_MAP.put(MobileLogSubLog.MmediaLog,
                "com.mediatek.log.mobile.MmediaLog");
        SUB_LOG_CONFIG_STRING_MAP.put(MobileLogSubLog.SSPMLog,
                "com.mediatek.log.mobile.SSPMLog");
        SUB_LOG_CONFIG_STRING_MAP.put(MobileLogSubLog.ADSPLog,
                "com.mediatek.log.mobile.ADSPLog");
        SUB_LOG_CONFIG_STRING_MAP.put(MobileLogSubLog.MCUPMLog,
                "com.mediatek.log.mobile.MCUPMLog");
    }

    /**
     * @param size
     *            int
     * @return boolean
     */
    public boolean setMobileLogTotalRecycleSize(int size) {
        return LogHandler.getInstance().setMobileLogTotalRecycleSize(size);
    }

    @Override
    public boolean dealWithADBCommand(String command) {
        if (command != null && command.startsWith(ADB_COMMAND_SET_SUBLOG_PREFIX)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name set_sublog_1_2_..._6_0 --ei cmd_target 1
            String newValue = command.substring(command.length() - 1);
            if (newValue.equals("0") || newValue.equals("1")) {
                String subTypeStr = command.substring(ADB_COMMAND_SET_SUBLOG_PREFIX.length(),
                        command.length() - 2);
                String[] subTypes = subTypeStr.split("_");
                if (subTypes == null || subTypes.length == 0) {
                    Utils.logw(TAG, "Unsupported set sublog value");
                    return false;
                }
                for (String subType : subTypes) {
                    int subTypeInt = 1;
                    try {
                        subTypeInt = Integer.parseInt(subType);
                    } catch (NumberFormatException e) {
                        Utils.loge(TAG, "Invalid sub log type parameter: " + subType);
                        continue;
                    }
                    setSubLogEnable(newValue.equals("1"), subTypeInt);
                }
            } else {
                Utils.logw(TAG, "Unsupported set sublog value");
            }
        }
        return super.dealWithADBCommand(command);
    }

}
