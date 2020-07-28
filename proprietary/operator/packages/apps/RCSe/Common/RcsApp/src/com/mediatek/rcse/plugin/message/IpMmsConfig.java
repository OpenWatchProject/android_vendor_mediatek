/*
 * Copyright (C) 2009 The Android Open Source Project
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
 */

package com.mediatek.rcse.plugin.message;

import java.io.File;

import android.content.Context;
import android.text.TextUtils;

import com.mediatek.mms.ipmessage.DefaultIpConfigExt;

public class IpMmsConfig extends DefaultIpConfigExt {
    private static final String TAG = "IpMmsConfig";

    // / M: add for ipmessage
    private static String sPicTempPath = "";
    private static String sAudioTempPath = "";
    private static String sVideoTempPath = "";
    private static String sVcardTempPath = "";
    private static String sCalendarTempPath = "";

    private static void initializeIpMessageFilePath(Context context) {
        if (IpMessageUtils.getSDCardStatus()) {
            sPicTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "picture";
            File picturePath = new File(sPicTempPath);
            if (!picturePath.exists()) {
                picturePath.mkdirs();
            }

            sAudioTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "audio";
            File audioPath = new File(sAudioTempPath);
            if (!audioPath.exists()) {
                audioPath.mkdirs();
            }

            sVideoTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "video";
            File videoPath = new File(sVideoTempPath);
            if (!videoPath.exists()) {
                videoPath.mkdirs();
            }

            sVcardTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "vcard";
            File vcardPath = new File(sVcardTempPath);
            if (!vcardPath.exists()) {
                vcardPath.mkdirs();
            }

            sCalendarTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "calendar";
            File calendarPath = new File(sCalendarTempPath);
            if (!calendarPath.exists()) {
                calendarPath.mkdirs();
            }

            String cachePath = IpMessageUtils.getCachePath(context);
            if (cachePath != null) {
                File f = new File(cachePath);
                if (!f.exists()) {
                    f.mkdirs();
                }
            }
        }
    }

    public static String getPicTempPath(Context context) {
        if (TextUtils.isEmpty(sPicTempPath)) {
            sPicTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "picture";
            File picturePath = new File(sPicTempPath);
            if (!picturePath.exists()) {
                picturePath.mkdirs();
            }
        }
        return sPicTempPath;
    }

    public static String getAudioTempPath(Context context) {
        if (TextUtils.isEmpty(sAudioTempPath)) {
            sAudioTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "audio";
            File audioPath = new File(sAudioTempPath);
            if (!audioPath.exists()) {
                audioPath.mkdirs();
            }
        }
        return sAudioTempPath;
    }

    public static String getVideoTempPath(Context context) {
        if (TextUtils.isEmpty(sVideoTempPath)) {
            sVideoTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "video";
            File videoPath = new File(sVideoTempPath);
            if (!videoPath.exists()) {
                videoPath.mkdirs();
            }
        }
        return sVideoTempPath;
    }

    public static String getVcardTempPath(Context context) {
        if (TextUtils.isEmpty(sVcardTempPath)) {
            sVcardTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "vcard";
            File vcardPath = new File(sVcardTempPath);
            if (!vcardPath.exists()) {
                vcardPath.mkdirs();
            }
        }
        return sVcardTempPath;
    }

    public static String getVcalendarTempPath(Context context) {
        if (TextUtils.isEmpty(sCalendarTempPath)) {
            sCalendarTempPath = IpMessageUtils.getSDCardPath(context)
                    + IpMessageUtils.IP_MESSAGE_FILE_PATH + "calendar";
            File calendarPath = new File(sCalendarTempPath);
            if (!calendarPath.exists()) {
                calendarPath.mkdirs();
            }
        }
        return sCalendarTempPath;
    }

    public static boolean isActivated(Context context) {
        if (!IpMessageServiceMananger.getInstance(context).serviceIsReady()) {
            return false;
        }

        return true;
    }

    public static boolean isServiceEnabled(Context context) {
        if (!IpMessageServiceMananger.getInstance(context).serviceIsReady()) {
            return false;
        }
        return true;
    }

    @Override
    public int getMaxTextLimit(Context context) {
        return IpMessageManager.getInstance(context).getMaxTextLimit();
    }

    @Override
    public boolean onIpInit(Context context) {
        initializeIpMessageFilePath(context);
        return true;
    }
}
