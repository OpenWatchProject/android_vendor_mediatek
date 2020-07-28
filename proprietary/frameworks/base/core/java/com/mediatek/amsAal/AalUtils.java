/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.amsAal;

import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.os.SystemProperties;
import android.util.Slog;
import android.util.Xml;
import java.io.PrintWriter;
import java.io.IOException;
import java.io.FileReader;
import java.io.File;
import java.util.HashMap;
import java.util.Map;
import java.util.ArrayList;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

/**
 * App-based AAL.
 */
public final class AalUtils {
    public static final int AAL_MODE_PERFORMANCE = 0;
    public static final int AAL_MODE_BALANCE = 1;
    public static final int AAL_MODE_LOWPOWER = 2;
    public static final int AAL_MODE_SIZE = AAL_MODE_LOWPOWER + 1;

    private static final String TAG = "AalUtils";
    private static boolean sDebug = false;

    private static boolean sIsAalSupported =
            SystemProperties.get("ro.vendor.mtk_aal_support").equals("1");
    private static boolean sEnabled = sIsAalSupported &&
            SystemProperties.get("persist.vendor.sys.mtk_app_aal_support").equals("1");

    private int mAalMode = AalUtils.AAL_MODE_BALANCE;

    private Map<AalIndex, Integer> mConfig = new HashMap<AalIndex, Integer>();

    private static AalUtils sInstance = null;

    private static String sAalConfigXMLPath = "/system/etc/ams_aal_config.xml";

    private static final int AAL_MIN_LEVEL = 0;
    private static final int AAL_MAX_LEVEL = 256;
    private static final int AAL_DEFAULT_LEVEL = 128;
    private static final int AAL_NULL_LEVEL = -1;
    private AalConfig mCurrentConfig = null;

    AalUtils() {
        if (!sIsAalSupported) {
            if (sDebug) {
                Slog.d(TAG, "AAL is not supported");
            }
            return;
        }

        try {
            parseXML();
        } catch (XmlPullParserException e) {
           Slog.d(TAG, "XmlPullParserException fail to parseXML, " + e);
        } catch (IOException e) {
           Slog.d(TAG, "IOException fail to parseXML, " + e);
        } catch (Exception e) {
           Slog.d(TAG, "fail to parseXML, " + e);
        }
    }

    /**
     * The device supports AAL or not.
     *
     * @return Support/Not support
     */
    public static boolean isSupported() {
        if (sDebug) {
            Slog.d(TAG, "isSupported = " + sIsAalSupported);
        }
        return sIsAalSupported;
    }

    /**
     * Get the instance of AalUtils.
     *
     * @param init True for initializing configurations
     * @return The instance of AalUtils
     */
    public static AalUtils getInstance() {
        if (sInstance == null) {
            sInstance = new AalUtils();
        }
        return sInstance;
    }

    /**
     * Set AAL mode.
     *
     * @param mode AAL mode
     */
    public void setAalMode(int mode) {
        if (!sIsAalSupported) {
            if (sDebug) {
                Slog.d(TAG, "AAL is not supported");
            }
            return;
        }

        setAalModeInternal(mode);
    }

    /**
     * Enable/Disable App-based AAL.
     *
     * @param enabled Enable/Disable
     */
    public void setEnabled(boolean enabled) {
        if (!sIsAalSupported) {
            if (sDebug) {
                Slog.d(TAG, "AAL is not supported");
            }
            return;
        }

        setEnabledInternal(enabled);
    }

    /**
     * Set AAL mode.
     *
     * @param mode AAL mode
     * @return Message for the operation result
     */
    synchronized String setAalModeInternal(int mode) {
        if (!sEnabled) {
            String msg = "AAL is not enabled";
            if (sDebug) {
                Slog.d(TAG, msg);
            }
            return msg;
        }

        String msg = null;
        if (mode >= 0 && mode < AAL_MODE_SIZE) {
            mAalMode = mode;
            msg = "setAalModeInternal " + mAalMode + "(" + modeToString(mAalMode) + ")";
        } else {
            msg = "unknown mode " + mode;
        }

        if (sDebug) {
            Slog.d(TAG, msg);
        }
        return msg;
    }

    /**
     * Enable/Disable App-based AAL.
     *
     * @param enabled Enable/Disable
     */
    synchronized public void setEnabledInternal(boolean enabled) {
        sEnabled = enabled;
        if (!sEnabled) {
            setDefaultSmartBacklightInternal("disabled");
            SystemProperties.set("persist.vendor.sys.mtk_app_aal_support", "0");
        } else {
            SystemProperties.set("persist.vendor.sys.mtk_app_aal_support", "1");
        }

        if (sDebug) {
            Slog.d(TAG, "setEnabledInternal(" + sEnabled + ")");
        }
    }

    /**
     * Set AAL level for the component.
     *
     * @param name Component name
     */
    synchronized public void setSmartBacklightInternal(ComponentName name) {
        setSmartBacklightInternal(name, mAalMode);
    }

    /**
     * Set AAL level for the component and AAL mode.
     *
     * @param name Component name
     * @param mode AAL mode
     */
    synchronized public void setSmartBacklightInternal(ComponentName name, int mode) {
        if (!sEnabled) {
            if (sDebug) {
                Slog.d(TAG, "AAL is not enabled");
            }
            return;
        }

        if (mode < 0 || mode >= AAL_MODE_SIZE) {
            if (sDebug) {
                Slog.d(TAG, "Unknown mode: " + mode);
            }
            return;
        }

        if (mCurrentConfig == null) {
            if (sDebug) {
                Slog.d(TAG, "mCurrentConfig == null");
            }
            mCurrentConfig = new AalConfig(null, AAL_DEFAULT_LEVEL);
        }

        // get level by activity or package
        AalIndex index = new AalIndex(mode, name.flattenToShortString());
        AalConfig config = getAalConfig(index);
        if (AAL_NULL_LEVEL == config.mLevel) {
            index = new AalIndex(mode, name.getPackageName());
            config = getAalConfig(index);
            if (AAL_NULL_LEVEL == config.mLevel) {
                config.mLevel = AAL_DEFAULT_LEVEL;
            }
        }

        int validLevel = ensureBacklightLevel(config.mLevel);
        if (sDebug) {
            Slog.d(TAG, "setSmartBacklight current level: " + mCurrentConfig.mLevel +
                " for " + index);
        }
        if (mCurrentConfig.mLevel != validLevel) {
            Slog.d(TAG, "setSmartBacklightStrength(" + validLevel + ") for " + index);
            mCurrentConfig.mLevel = validLevel;
            mCurrentConfig.mName = index.getIndexName();
            setSmartBacklightStrength(validLevel);
        }
    }

    /**
     * Set AAL level to default value.
     *
     * @param reason Reason
     */
    synchronized public void setDefaultSmartBacklightInternal(String reason) {
        if (!sEnabled) {
            if (sDebug) {
                Slog.d(TAG, "AAL is not enabled");
            }
            return;
        }

        if (mCurrentConfig != null && mCurrentConfig.mLevel != AAL_DEFAULT_LEVEL) {
            Slog.d(TAG, "setSmartBacklightStrength(" + AAL_DEFAULT_LEVEL + ") " + reason);
            mCurrentConfig.mLevel = AAL_DEFAULT_LEVEL;
            mCurrentConfig.mName = null;
            setSmartBacklightStrength(AAL_DEFAULT_LEVEL);
        }
    }

    /**
     * It is the activity status in the AMS records.
     * The ActivityThread resuming may not completed.
     *
     * @param data The data from the AMEventHook event.
     */
    public void onAfterActivityResumed(String packageName,String activityName) {
        setSmartBacklightInternal(new ComponentName(packageName, activityName));
    }

    public void onUpdateSleep(boolean wasSleeping, boolean isSleepingAfterUpdate) {
        if (sDebug) {
            Slog.d(TAG, "onUpdateSleep before=" + wasSleeping
                + " after=" + isSleepingAfterUpdate);
        }
        if (wasSleeping != isSleepingAfterUpdate && isSleepingAfterUpdate == true) {
            setDefaultSmartBacklightInternal("for sleep");
        }
    }

    private native void setSmartBacklightStrength(int level);

    private int ensureBacklightLevel(int level) {
        if (level < AAL_MIN_LEVEL) {
            if (sDebug) {
                Slog.e(TAG, "Invalid AAL backlight level: " + level);
            }
            return AAL_MIN_LEVEL;
        } else if (level > AAL_MAX_LEVEL) {
            if (sDebug) {
                Slog.e(TAG, "Invalid AAL backlight level: " + level);
            }
            return AAL_MAX_LEVEL;
        }

        return level;
    }

    private AalConfig getAalConfig(AalIndex index) {
        int level = AAL_NULL_LEVEL;
        if (mConfig.containsKey(index)) {
            level = mConfig.get(index);
        } else {
            if (sDebug) {
                Slog.d(TAG, "No config for " + index);
            }
        }
        return new AalConfig(index.getIndexName(), level);
    }

    private String modeToString(int mode) {
        switch (mode) {
        case AAL_MODE_PERFORMANCE:
            return "AAL_MODE_PERFORMANCE";
        case AAL_MODE_BALANCE:
            return "AAL_MODE_BALANCE";
        case AAL_MODE_LOWPOWER:
            return "AAL_MODE_LOWPOWER";
        default:
        }

        return "Unknown mode: " + mode;
    }

    /**
     * Class for AAL config.
     */
    private class AalConfig {
        public String mName = null;
        public int mLevel = AAL_NULL_LEVEL;

        public AalConfig(String name, int level) {
            mName = name;
            mLevel = level;
        }
    }

    /**
     * Class for mapping AAL settings.
     */
    private class AalIndex {
        private int mMode;
        private String mName;

        AalIndex(int mode, String name) {
            set(mode, name);
        }

        private void set(int mode, String name) {
            mMode = mode;
            mName = name;
        }

        public int getMode() {
            return mMode;
        }

        public String getIndexName() {
            return mName;
        }

        @Override
        public String toString() {
            return "(" + mMode + ": " + modeToString(mMode) + ", " + mName + ")";
        }

        @Override
        public boolean equals(Object obj) {
            if (obj == null) {
                return false;
            }
            if (obj == this) {
                return true;
            }
            if (!(obj instanceof AalIndex)) {
                return false;
            }

            AalIndex index = (AalIndex) obj;
            if (mName == null && index.mName == null) {
                return mMode == index.mMode;
            } else if (mName == null || index.mName == null) {
                return false;
            }
            return mMode == index.mMode && mName.equals(index.mName);
        }

        @Override
        public int hashCode() {
            String hashString = Integer.toString(mMode) + ":";
            if (mName != null) {
                hashString = hashString + Integer.toString(mName.hashCode());
            }
            return hashString.hashCode();
        }
    }

    /**
     * Debug commands.
     *
     * @param pw The output of the dumpsys command.
     * @param args The input arguments of  the dumpsys command.
     * @param opti The index of the arguments before handling.
     *
     * @return The index of the arguments after handling.
     */
    public int dump(PrintWriter pw, String[] args, int opti) {
        if (sIsAalSupported) {
            if (args.length <= 1) {
                pw.println(dumpDebugUsageInternal());
            } else {
                String option = args[opti];
                if ("dump".equals(option) && args.length == 2) {
                    pw.println(dumpInternal());
                } else if ("debugon".equals(option) && args.length == 2) {
                    pw.println(setDebugInternal(true));
                    pw.println("App-based AAL debug on");
                } else if ("debugoff".equals(option) && args.length == 2) {
                    pw.println(setDebugInternal(false));
                    pw.println("App-based AAL debug off");
                } else if ("on".equals(option) && args.length == 2) {
                    setEnabledInternal(true);
                    pw.println("App-based AAL on");
                } else if ("off".equals(option) && args.length == 2) {
                    setEnabledInternal(false);
                    pw.println("App-based AAL off");
                } else if ("mode".equals(option) && args.length == 3) {
                    opti++;
                    int mode = Integer.parseInt(args[opti]);
                    pw.println(setAalModeInternal(mode));
                    pw.println("Done");
                } else if ("set".equals(option) &&
                    (args.length == 4 || args.length == 5)) {
                    opti++;
                    String pkgName = args[opti];
                    opti++;
                    int value = Integer.parseInt(args[opti]);
                    if (args.length == 4) {
                        pw.println(setSmartBacklightTableInternal(pkgName, value));
                    } else {
                        opti++;
                        int mode = Integer.parseInt(args[opti]);
                        pw.println(setSmartBacklightTableInternal(pkgName, value, mode));
                    }
                    pw.println("Done");
                } else {
                    pw.println(dumpDebugUsageInternal());
                }
            }
        } else {
            pw.println("Not support App-based AAL");
        }

        return opti;
    }

    /**
     * Dump usage of debug commnads.
     *
     * @return Usage of debug commnads
     */
    private String dumpDebugUsageInternal() {
        StringBuilder sb = new StringBuilder();
        sb.append("\nUsage:\n");
        sb.append("1. App-based AAL help:\n");
        sb.append("    adb shell dumpsys activity aal\n");
        sb.append("2. Dump App-based AAL settings:\n");
        sb.append("    adb shell dumpsys activity aal dump\n");
        sb.append("1. App-based AAL debug on:\n");
        sb.append("    adb shell dumpsys activity aal debugon\n");
        sb.append("1. App-based AAL debug off:\n");
        sb.append("    adb shell dumpsys activity aal debugoff\n");
        sb.append("3. Enable App-based AAL:\n");
        sb.append("    adb shell dumpsys activity aal on\n");
        sb.append("4. Disable App-based AAL:\n");
        sb.append("    adb shell dumpsys activity aal off\n");
        sb.append("5. Set App-based AAL mode:\n");
        sb.append("    adb shell dumpsys activity aal mode <mode>\n");
        sb.append("6. Set App-based AAL config for current mode:\n");
        sb.append("    adb shell dumpsys activity aal set <component> <value>\n");
        sb.append("7. Set App-based AAL config for the mode:\n");
        sb.append("    adb shell dumpsys activity aal set <component> <value> <mode>\n");

        return sb.toString();
    }

    /**
     * Dump AAL settings.
     *
     * @return AAL settings
     */
    synchronized private String dumpInternal() {
        StringBuilder sb = new StringBuilder();
        sb.append("\nApp-based AAL Mode: " + mAalMode + "(" + modeToString(mAalMode) +
            "), Supported: " + sIsAalSupported + ", Enabled: " + sEnabled +
            ", Debug: " + sDebug + "\n");

        int i = 1;
        for (AalIndex index : mConfig.keySet()) {
            String level = Integer.toString(mConfig.get(index));
            sb.append("\n" + i + ". " + index + " - " + level);
            i++;
        }
        if (i == 1) {
            sb.append("\nThere is no App-based AAL configuration.\n");
            sb.append(dumpDebugUsageInternal());
        }
        if (sDebug) {
            Slog.d(TAG, "dump config: " + sb.toString());
        }

        return sb.toString();
    }

    /**
     * Enable/Disable debug log.
     *
     * @param debug Enable/Disable
     * @return Message for the operation result
     */
    synchronized private String setDebugInternal(boolean debug) {
        sDebug = debug;
        return "Set Debug: " + debug;
    }

    /**
     * Set AAL config for the component.
     *
     * @param name Component name
     * @param value AAL level
     * @return Message for the operation result
     */
    synchronized private String setSmartBacklightTableInternal(String name, int value) {
        return setSmartBacklightTableInternal(name, value, mAalMode);
    }

    /**
     * Set AAL config for the component and AAL mode.
     *
     * @param name Component name
     * @param value AAL level
     * @param mode AAL mode
     * @return Message for the operation result
     */
    synchronized private String setSmartBacklightTableInternal(String name, int value, int mode) {
        if (!sEnabled) {
            String msg = "AAL is not enabled";
            if (sDebug) {
                Slog.d(TAG, msg);
            }
            return msg;
        }

        if (mode < 0 || mode >= AAL_MODE_SIZE) {
            String msg = "Unknown mode: " + mode;
            if (sDebug) {
                Slog.d(TAG, msg);
            }
            return msg;
        }

        AalIndex index = new AalIndex(mode, name);
        if (sDebug) {
            Slog.d(TAG, "setSmartBacklightTable(" + value + ") for " + index);
        }
        mConfig.put(index, value);

        return "Set(" + value + ") for " + index;
    }

    private void parseXML() throws XmlPullParserException, IOException {
        if (!(new File(sAalConfigXMLPath).exists())) {
            if (sDebug) {
                Slog.d(TAG, "parseXML file not exists: " + sAalConfigXMLPath);
            }
            return;
        }
        FileReader fileReader = new FileReader(sAalConfigXMLPath);
        XmlPullParser parser = Xml.newPullParser();
        parser.setInput(fileReader);
        int eventType = parser.getEventType();
        while (eventType != XmlPullParser.END_DOCUMENT) {
            switch (eventType) {
                case XmlPullParser.START_DOCUMENT:
                    break;
                case XmlPullParser.START_TAG:
                    if (parser.getName().equals("config")) {
                        int mode = Integer.parseInt(parser.getAttributeValue(0));
                        String componentName = parser.getAttributeValue(1);
                        int backlight = Integer.parseInt(parser.getAttributeValue(2));
                        mConfig.put(new AalIndex(mode, componentName), backlight);
                    }
                    break;
                case XmlPullParser.END_TAG:
                    break;
            }
            eventType = parser.next();
        }
        if (fileReader != null) {
            fileReader.close();
        }
    }
}
