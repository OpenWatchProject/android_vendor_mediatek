package com.mediatek.engineermode.videolog;


import android.os.RemoteException;
import android.os.SystemProperties;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;

/**
 * Class for all video log items.
 *
 */
public abstract class VideoLogItem {
    protected static final String LOG_PROP_1 = "test";
    protected static final String TAG = "VideoLog";
    protected static final String PROP_ENABLE = "1";
    protected static final String PROP_DISABLE = "0";
    private static final String LOG_MUCH_PROP = "persist.vendor.logmuch";
    private static final String LOG_MUCH_ENABLE = "true";
    private static final String LOG_MUCH_DISABLE = "false";
    private static final String LOG_INTERNAL_PROP = "ro.vendor.mtklog_internal";
//    private static boolean sLogInternal = PROP_ENABLE.equals(
//                                              SystemProperties.get(LOG_INTERNAL_PROP));
    protected boolean mCurStatus;

    boolean switchStatus() {
        Boolean result = switchLog(!mCurStatus);
        if (result) {
            mCurStatus = !mCurStatus;
        }
        return result;
    }

    boolean getStatus() {
        String strProp = getStatusProp();
        Elog.d(TAG, "getStatus: " + strProp);
        mCurStatus = PROP_ENABLE.equals(SystemProperties.get(strProp));
        Elog.d(TAG, "queryCurrent: " + mCurStatus);
        return mCurStatus;
    }

    static boolean switchLogMuchDetect(boolean on) {
        Elog.d(TAG, "switchLogMuchDetect " + on);
//        if (!sLogInternal) {
//            return true;
//        }
        String logMuchProTar = on ? LOG_MUCH_ENABLE : LOG_MUCH_DISABLE;
        SystemProperties.set(LOG_MUCH_PROP, logMuchProTar);

        return logMuchProTar.equals(SystemProperties.get(LOG_MUCH_PROP));
    }

    protected abstract String getStatusProp();
    protected abstract boolean switchLog(boolean enable);

}

/**
 * OMX Venc Log Item.
 *
 */
class OmxVenc extends VideoLogItem {

    protected static final String LOG_PROP_1 = "vendor.mtk.omx.enable.venc.log";

    @Override
    protected String getStatusProp() {
        // TODO Auto-generated method stub
        return LOG_PROP_1;
    }

    @Override
    protected boolean switchLog(boolean enable) {
        String proTar = enable ? PROP_ENABLE : PROP_DISABLE;
        try {
            EmUtils.getEmHidlService().setOmxVencLogEnable(proTar);
        } catch (RemoteException e) {
            e.printStackTrace();
            Elog.e(TAG, "set property failed ...");
        }
        return proTar.equals(SystemProperties.get(LOG_PROP_1));
    }
}

/**
 * OMX Vdec Log Item.
 *
 */
class OmxVdec extends VideoLogItem {

    protected static final String LOG_PROP_1 = "vendor.mtk.omx.enable.mvamgr.log";
    private static final String LOG_PROP_2 = "vendor.mtk.omx.vdec.log";
    private static final String LOG_PROP_3 = "vendor.mtk.omx.vdec.perf.log";


    @Override
    protected String getStatusProp() {
        // TODO Auto-generated method stub
        return LOG_PROP_1;
    }

    @Override
    protected boolean switchLog(boolean enable) {
        String proTar = enable ? PROP_ENABLE : PROP_DISABLE;
        try {
            EmUtils.getEmHidlService().setOmxVdecLogEnable(proTar);
        } catch (RemoteException e) {
            e.printStackTrace();
            Elog.e(TAG, "set property failed ...");
        }
        return (proTar.equals(SystemProperties.get(LOG_PROP_1))
                && proTar.equals(SystemProperties.get(LOG_PROP_2))
                 && proTar.equals(SystemProperties.get(LOG_PROP_3)));
    }

}

/**
 * Vdec driver Log Item.
 *
 */
class VdecDriver extends VideoLogItem {

    private static final String LOG_PROP_1 = "vendor.mtk.vdec.log";

    @Override
    protected String getStatusProp() {
        // TODO Auto-generated method stub
        return LOG_PROP_1;
    }

    @Override
    protected boolean switchLog(boolean enable) {
        String proTar = enable ? PROP_ENABLE : PROP_DISABLE;
        try {
            EmUtils.getEmHidlService().setVdecDriverLogEnable(proTar);
        } catch (RemoteException e) {
            e.printStackTrace();
            Elog.e(TAG, "set property failed ...");
        }
        return proTar.equals(SystemProperties.get(LOG_PROP_1));
    }
}

/**
 * SVP Log Item.
 *
 */
class Svp extends VideoLogItem {

    private static final String LOG_PROP_1 = "vendor.mtk.vdectlc.log";

    @Override
    protected String getStatusProp() {
        // TODO Auto-generated method stub
        return LOG_PROP_1;
    }

    @Override
    protected boolean switchLog(boolean enable) {
        String proTar = enable ? PROP_ENABLE : PROP_DISABLE;
        try {
            EmUtils.getEmHidlService().setSvpLogEnable(proTar);
        } catch (RemoteException e) {
            e.printStackTrace();
            Elog.e(TAG, "set property failed ...");
        }
        return proTar.equals(SystemProperties.get(LOG_PROP_1));
    }
}

/**
 * OMX Core Log Item.
 *
 */
class OmxCore extends VideoLogItem {

    private static final String LOG_PROP_1 = "vendor.mtk.omx.core.log";

    @Override
    protected String getStatusProp() {
        // TODO Auto-generated method stub
        return LOG_PROP_1;
    }

    @Override
    protected boolean switchLog(boolean enable) {
        String proTar = enable ? PROP_ENABLE : PROP_DISABLE;
        try {
            EmUtils.getEmHidlService().setOmxCoreLogEnable(proTar);
        } catch (RemoteException e) {
            e.printStackTrace();
            Elog.e(TAG, "set property failed ...");
        }
        return proTar.equals(SystemProperties.get(LOG_PROP_1));
    }
}


/**
 * H264 Venc driver Log Item.
 *
 */
class VencDriver extends VideoLogItem {

    private static final String LOG_PROP_1 = "vendor.mtk.venc.h264.showlog";

    @Override
    protected String getStatusProp() {
        // TODO Auto-generated method stub
        return LOG_PROP_1;
    }

    @Override
    protected boolean switchLog(boolean enable) {
        String proTar = enable ? PROP_ENABLE : PROP_DISABLE;
        try {
            EmUtils.getEmHidlService().setVencDriverLogEnable(proTar);
        } catch (RemoteException e) {
            e.printStackTrace();
            Elog.e(TAG, "set property failed ...");
        }
        return proTar.equals(SystemProperties.get(LOG_PROP_1));
    }
}