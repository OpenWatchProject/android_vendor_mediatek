package com.mediatek.hdmi;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.RemoteException;
import android.util.Log;
import android.os.ServiceManager;
import android.os.SystemProperties;
import java.util.ArrayList;
import java.util.Arrays;

public class HdmiNative {
    static HdmiNative mHdmi;


    private static final boolean DEBUG = true;

    private static final String TAG = "HdmiNative";

    static boolean sLibStatus = true;

    private int[] mEdid;
    private int[] mPreEdid;

    static {
        try {
            Log.d("JNI_HDMI", "loadLibrary");
            System.loadLibrary("jni_hdmi");
        } catch (UnsatisfiedLinkError e) {
            Log.e("JNI_HDMI", "UnsatisfiedLinkError");
            sLibStatus = false;
        }
    }
    public static boolean getLibStatus() {
        return sLibStatus;
    }


    // //////////////////////public APIS//////////////////////////

    public static HdmiNative getInstance() {
        if (mHdmi == null){
           mHdmi = new HdmiNative();
        }
        return mHdmi;
    }

    private HdmiNative() {
        Log.d(TAG, "HIDL GET HDMI NATIVE");

        //mHdmiObserver = HdmiObserver.getHdmiObserver();
        //mHdmiObserver.setHandler(mHandler);
        //mHdmiObserver.startObserve();
    }

    public int[] getSupportedResolutions() {
        return getSupportedResolutionsImpl();
    }

    private static class FeatureOption {

        public static final boolean MTK_DRM_KEY_MNG_SUPPORT = getValue("ro.vendor.mtk_drm_key_mng_support");
        //public static final boolean MTK_HDMI_HDCP_SUPPORT = getValue("ro.vendor.mtk_hdmi_hdcp_support");
        //public static final boolean MTK_MT8193_HDCP_SUPPORT = getValue("ro.vendor.mtk_mt8193_hdcp_support");
        //public static final boolean MTK_INTERNAL_MHL_SUPPORT = getValue("ro.vendor.mtk_internal_mhl_support");
        public static final boolean MTK_INTERNAL_HDMI_SUPPORT =
            getString("ro.vendor.mtk_tb_hdmi", "4k") || getString("ro.vendor.mtk_tb_hdmi", "internal");
        public static final boolean MTK_MT8193_HDMI_SUPPORT = getString("ro.vendor.mtk_tb_hdmi", "mt8193");
        public static final boolean MTK_HDMI_4K_SUPPORT = getString("ro.vendor.mtk_tb_hdmi", "4k");
        public static final boolean MTK_ITE66121_HDMI_SUPPORT = getString("ro.vendor.mtk_tb_hdmi", "ite66121");
        public static final boolean MTK_ENABLE_HDMI_MULTI_CHANNEL = true;
        //public static final boolean MTK_TB6582_HDMI_SUPPORT = getValue("ro.vendor.hdmi.1080p60.disable");

        private static boolean getValue(String key) {
            return SystemProperties.get(key).equals("1");
        }

        private static boolean getString(String key, String valueString) {
            return SystemProperties.get(key).contains(valueString);
        }
    }

    public int[] getResolutionMask() {
        Log.d(TAG, "getResolutionMask before");
        int[] edid = nativeGetResolutionMask();
        Log.d(TAG, "getResolutionMask end");
        Log.d(TAG, "getResolutionMask edid = " + edid[0] + " " + edid[1] + " " + edid[2] + " " + edid[3]);
        return edid;
    }

    private int[] getSupportedResolutionsImpl() {
        Log.d(TAG, "getSupportedResolutionsImpl");
        mEdid = getResolutionMask();
        /*if(mEdid == null){
            return HdmiDef.getDefaultResolutions(3);
        }
        if (0 == mEdid[0] && 0 == mEdid[1] && 0 == mEdid[2]) {
            return HdmiDef.getDefaultResolutions(3);
        }*/
        Log.d(TAG, "getSupportedResolutionsImpl, INTERNAL = " + FeatureOption.MTK_INTERNAL_HDMI_SUPPORT);
        Log.d(TAG, "getSupportedResolutionsImpl, 4k = " + FeatureOption.MTK_HDMI_4K_SUPPORT);
        Log.d(TAG, "getSupportedResolutionsImpl, 8193 = " + FeatureOption.MTK_MT8193_HDMI_SUPPORT);
        Log.d(TAG, "getSupportedResolutionsImpl, ite66121 = " + FeatureOption.MTK_ITE66121_HDMI_SUPPORT);
        int[] resolutions;
        if (FeatureOption.MTK_INTERNAL_HDMI_SUPPORT
                /*|| FeatureOption.MTK_INTERNAL_MHL_SUPPORT*/) {
                Log.d(TAG, "MTK_INTERNAL_HDMI_SUPPORT");
            if(FeatureOption.MTK_HDMI_4K_SUPPORT){
                return HdmiDef.getDefaultResolutions(1);
            } else {
                return HdmiDef.getDefaultResolutions(4);
            }
        } else if (FeatureOption.MTK_MT8193_HDMI_SUPPORT
            || FeatureOption.MTK_ITE66121_HDMI_SUPPORT){
            resolutions = HdmiDef.getDefaultResolutions(2);
        } else {
            resolutions = HdmiDef.getAllResolutions();
        }
        int edidTemp = mEdid[0] | mEdid[1];
        //add for 4k2k resolution
        int edidTemp_4k2k = mEdid[2];
        Log.d(TAG, "getSupportedResolutionsImpl edidTemp:" + edidTemp);
        Log.d(TAG, "getSupportedResolutionsImpl edidTemp_4k2k:" + edidTemp_4k2k);
        ArrayList<Integer> list = new ArrayList<Integer>();
        for (int res : resolutions) {
            try {
                if(res >= HdmiDef.RESOLUTION_3840X2160P23_976HZ){
                    int mask_4k2k = HdmiDef.sResolutionMask_4k2k[res - HdmiDef.RESOLUTION_3840X2160P23_976HZ];
                              if((edidTemp_4k2k & mask_4k2k) != 0){
                        if(!list.contains(res)){
                            list.add(res);
                        }
                    }
                }else if(res < HdmiDef.RESOLUTION_3840X2160P23_976HZ){
                int mask = HdmiDef.sResolutionMask[res];
                    if ((edidTemp & mask) != 0) {
                        if (!list.contains(res)) {
                            list.add(res);
                        }
                    }
                }
            } catch (ArrayIndexOutOfBoundsException e) {
                Log.w(TAG, e.getMessage());
            }
        }
        resolutions = new int[list.size()];
        for (int i = 0; i < list.size(); i++) {
            resolutions[i] = list.get(i);
            Log.d(TAG, "getSupportedResolutionsImpl i: " + i + ", " + resolutions[i]);
        }
        return resolutions;
    }

    private boolean needUpdate(int videoResolution) {
        Log.d(TAG, "needUpdate: " + videoResolution);
        boolean needUpdate = true;
        if (videoResolution >= HdmiDef.AUTO) {
            needUpdate = true;
        }
        if (mPreEdid != null && Arrays.equals(mEdid, mPreEdid)) {
            needUpdate = false;
        }
        Log.d(TAG, "needUpdate needUpdate: " + needUpdate);
        return needUpdate;
    }

    private int getSuitableResolution(int videoResolution) {
        Log.d(TAG, "getSuitableResolution videoResolution: " + videoResolution);
        int[] supportedResolutions = getSupportedResolutions();
        ArrayList<Integer> resolutionList = new ArrayList<Integer>();
        for (int res : supportedResolutions) {
            resolutionList.add(res);
        }
        if (needUpdate(videoResolution)) {
            if (mEdid[0] != 0 || mEdid[1]!= 0 || mEdid[2]!= 0) {
                int edidTemp = mEdid[0] | mEdid[1];
                int edidTemp_4k2k = mEdid[2];
                Log.d(TAG, "getSuitableResolution edidTemp:" + edidTemp);
                Log.d(TAG, "getSuitableResolution edidTemp_4k2k:" + edidTemp_4k2k);
                int index = 0;
                if (FeatureOption.MTK_INTERNAL_HDMI_SUPPORT
                        /*|| FeatureOption.MTK_INTERNAL_MHL_SUPPORT*/) {
                    index = 1;
                } else if (FeatureOption.MTK_MT8193_HDMI_SUPPORT
                    || FeatureOption.MTK_ITE66121_HDMI_SUPPORT) {
                    index = 2;
                } /*else if (FeatureOption.MTK_TB6582_HDMI_SUPPORT) {
                    index = 2;
                }*/else {
                    index = 3;
                }
                Log.d(TAG, "getSuitableResolution index:" + index);
                int[] prefered = HdmiDef.getPreferedResolutions(index);
                for (int res : prefered) {
                    Log.d(TAG, "getSuitableResolution res:" + res);
                    int act = res;
                    if (res >= HdmiDef.AUTO) {
                        act = res - HdmiDef.AUTO;
                    }
                    Log.d(TAG, "getSuitableResolution act:" + act);
                    if(act<HdmiDef.RESOLUTION_3840X2160P23_976HZ){
                        if (0 != (edidTemp & HdmiDef.sResolutionMask[act])
                              && resolutionList.contains(act)) {
                            videoResolution = res;
                            Log.d(TAG, "getSuitableResolution videoResolution 1:" + videoResolution);
                            break;
                        }
                    }else{
                        if (0 != (edidTemp_4k2k & HdmiDef.sResolutionMask_4k2k[act - HdmiDef.RESOLUTION_3840X2160P23_976HZ])
                              && resolutionList.contains(act)) {
                            videoResolution = res;
                            Log.d(TAG, "getSuitableResolution videoResolution 2:" + videoResolution);
                            break;
                        }
                    }
                }
            }
        }
        Log.d(TAG, "suiteable video resolution: " + videoResolution);
        return videoResolution;
    }

    public static boolean enableHdmi(boolean enable) {
        Log.d(TAG, "hidl enableHdmi: " + enable);
        boolean ret = false;
        ret = nativeEnableHdmi(enable);
        Log.d(TAG, "hidl enableHdmi ret:" + ret);
        return ret;

    }

    public static boolean setVideoResolutionImpl(int resolution) {
        boolean ret = false;
        Log.d(TAG, "hidl setVideoResolutionImpl: " + resolution);
        ret = nativeSetVideoResolution(resolution);
        Log.d(TAG, "hidl setVideoResolutionImpl ret:" + ret);
        return ret;
    }

    public boolean setVideoResolution(int resolution) {
        Log.d(TAG, "setVideoResolution: " + resolution);
        boolean ret = false;
        int suitableResolution = resolution;
        if (resolution >= HdmiDef.AUTO) {
            suitableResolution = getSuitableResolution(resolution);
        }
        int finalResolution = suitableResolution >= HdmiDef.AUTO ? (suitableResolution - HdmiDef.AUTO)
                : suitableResolution;
        Log.d(TAG, "final video resolution: " + finalResolution);
        ret = setVideoResolutionImpl(finalResolution);
        Log.d(TAG, "setVideoResolution ret:" + ret);
        return ret;
    }

    public static boolean setAutoMode(boolean enable) {
        Log.d(TAG, "hidl setAutoMode: " + enable);
        boolean ret = false;
        ret = nativeSetAutoMode(enable);
        Log.d(TAG, "hidl setAutoMode ret:" + ret);
        return ret;
    }

    public static boolean enableHDCP(boolean enable) {
        Log.d(TAG, "hidl enableHDCP: " + enable);
        boolean ret = false;
        ret = nativeEnableHDCP(enable);
        Log.d(TAG, "hidl enableHDCP ret:" + ret);
        return ret;
    }

    public static boolean enableHdmiHdr(boolean enable) {
        Log.d(TAG, "hidl enableHdmiHdr: " + enable);
        boolean ret = false;
        ret = nativeEnableHdmiHdr(enable);
        Log.d(TAG, "hidl enableHdmiHdr ret:" + ret);
        return ret;
    }

    public static boolean setColorFormat(int colorFormat) {
        Log.d(TAG, "hidl setColorFormat: " + colorFormat);
        boolean ret = false;
        ret = nativeSetColorFormat(colorFormat);
        Log.d(TAG, "hidl setColorFormat ret:" + ret);
        return ret;
    }

    public static boolean setColorDepth(int colorDepth) {
        Log.d(TAG, "hidl setColorDepth: " + colorDepth);
        boolean ret = false;
        ret = nativeSetColorDepth(colorDepth);
        Log.d(TAG, "hidl setColorDepth ret:" + ret);
        return ret;
    }

    private static native boolean nativeEnableHdmi(boolean isEnable);
    private static native boolean nativeEnableHDCP(boolean isEnable);
    private static native boolean nativeSetAutoMode(boolean isEnable);
    private static native boolean nativeEnableHdmiHdr(boolean isEnable);
    private static native int[] nativeGetResolutionMask();
    private static native boolean nativeSetVideoResolution(int resolution);
    private static native boolean nativeSetColorFormat(int colorFormat);
    private static native boolean nativeSetColorDepth(int colorDepth);

}
