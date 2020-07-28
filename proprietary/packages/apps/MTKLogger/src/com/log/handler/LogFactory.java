package com.log.handler;

import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.connection.LbsHidlConnection;
import com.log.handler.connection.LogHidlConnection;
import com.log.handler.connection.LogSocketConnection;
import com.log.handler.instance.AbstractLogInstance;
import com.log.handler.instance.BTHostLog;
import com.log.handler.instance.ConnsysFWLog;
import com.log.handler.instance.GPSHostLog;
import com.log.handler.instance.METLog;
import com.log.handler.instance.MobileLog;
import com.log.handler.instance.ModemLog;
import com.log.handler.instance.NetworkLog;

/**
 * @author MTK81255
 *
 */
public class LogFactory {

    /**
     * @param logType
     *            LogType
     * @return AbstractLogInstance
     */
    public static AbstractLogInstance getTypeLogInstance(LogType logType) {
        switch (logType) {
        case MOBILE_LOG:
            return getMobileLogInstance();
        case MODEM_LOG:
            return getModemLogInstance();
        case NETWORK_LOG:
            return getNetworkLogInstance();
        case CONNSYSFW_LOG:
            return getConnsysFWLogInstance();
        case GPSHOST_LOG:
            return getGPSHostLogInstance();
        case BTHOST_LOG:
            return getBTHostLogInstance();
        case MET_LOG:
            return getMETLogInstance();
        default:
            return null;
        }
    }

    private static MobileLog sMobileLog;

    /**
     * @return MobileLog
     */
    public static MobileLog getMobileLogInstance() {
        if (sMobileLog == null) {
            synchronized (LogFactory.class) {
                if (sMobileLog == null) {
                    sMobileLog = new MobileLog(new LogSocketConnection("mobilelogd"),
                            LogType.MOBILE_LOG);
                }
            }
        }
        return sMobileLog;
    }

    private static ModemLog sModemLog;

    /**
     * @return ModemLog
     */
    public static ModemLog getModemLogInstance() {
        if (sModemLog == null) {
            synchronized (LogFactory.class) {
                if (sModemLog == null) {
                    sModemLog =
                            new ModemLog(new LogSocketConnection("com.mediatek.mdlogger.socket1"),
                                    LogType.MODEM_LOG);
                }
            }
        }
        return sModemLog;
    }

    private static NetworkLog sNetworkLog;

    /**
     * @return NetworkLog
     */
    public static NetworkLog getNetworkLogInstance() {
        if (sNetworkLog == null) {
            synchronized (LogFactory.class) {
                if (sNetworkLog == null) {
                    sNetworkLog =
                            new NetworkLog(new LogSocketConnection("netdiag"), LogType.NETWORK_LOG);
                }
            }
        }
        return sNetworkLog;
    }

    private static BTHostLog sBTHostLog;

    /**
     * @return BTHostLog
     */
    public static BTHostLog getBTHostLogInstance() {
        if (sBTHostLog == null) {
            synchronized (LogFactory.class) {
                if (sBTHostLog == null) {
                    sBTHostLog = new BTHostLog(new LogSocketConnection("bthostlogd"),
                            LogType.BTHOST_LOG);
                }
            }
        }
        return sBTHostLog;
    }

    private static METLog sMETLog;

    /**
     * @return METLog
     */
    public static METLog getMETLogInstance() {
        if (sMETLog == null) {
            synchronized (LogFactory.class) {
                if (sMETLog == null) {
                    sMETLog =
                            new METLog(new LogHidlConnection("METLogHidlServer"), LogType.MET_LOG);
                }
            }
        }
        return sMETLog;
    }

    private static ConnsysFWLog sConnsysFWLog;

    /**
     * @return ConnsysFWLog
     */
    public static ConnsysFWLog getConnsysFWLogInstance() {
        if (sConnsysFWLog == null) {
            synchronized (LogFactory.class) {
                if (sConnsysFWLog == null) {
                    sConnsysFWLog = new ConnsysFWLog(new LogSocketConnection("connsysfwlogd"),
                            LogType.CONNSYSFW_LOG);
                }
            }
        }
        return sConnsysFWLog;
    }

    private static GPSHostLog sGPSHostLog;

    /**
     * @return GPSHostLog
     */
    public static GPSHostLog getGPSHostLogInstance() {
        if (sGPSHostLog == null) {
            synchronized (LogFactory.class) {
                if (sGPSHostLog == null) {
                    sGPSHostLog = new GPSHostLog(new LbsHidlConnection("mtk_mtklogger2mnld"),
                            LogType.GPSHOST_LOG);
                }
            }
        }
        return sGPSHostLog;
    }

}
