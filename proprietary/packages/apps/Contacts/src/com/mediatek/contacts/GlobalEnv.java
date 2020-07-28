/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.contacts;

import android.app.Application;
import android.content.Context;
import android.os.SystemProperties;

import com.mediatek.contacts.aassne.SimAasEditor;
import com.mediatek.contacts.aassne.SimSneEditor;
import com.mediatek.contacts.util.Log;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class GlobalEnv {
    private static final String TAG = "GlobalEnv";

    private static Context sContext = null;
    private static SimAasEditor sSimAasEditor = null;
    private static SimSneEditor sSimSneEditor = null;

    /**
     * Should be single thread, as we don't want to simultaneously handle
     * contacts copy-delete-import-export request.
     */
    //private static ExecutorService sSingleTaskService = Executors.newSingleThreadExecutor();

    private static final boolean IS_TABLET =
            ("tablet".equals(SystemProperties.get("ro.build.characteristics")));

    /**
     * set the application context here, in order to use ContactsCommon more
     * easily
     * @param context
     *            only ApplicationContext is acceptable, otherwise throw
     *            Exceptions
     */
    public static void setApplicationContext(Context context) {
        if (sContext == null) {
            if (context != null && context instanceof Application) {
                sContext = context;
                Log.i(TAG, "[setApplicationContext]sContext: " + sContext);
            } else {
                throw new IllegalArgumentException("Only Application context can be set: "
                        + context);
            }
        } else {
            throw new IllegalStateException("application context could be set only once");
        }
    }

    public static Context getApplicationContext() {
        if (sContext != null) {
            return sContext;
        } else {
            throw new IllegalStateException("context not set yet");
        }
    }

    public static boolean isUsingTwoPanes() {
        return IS_TABLET;
    }

    public static SimAasEditor getSimAasEditor() {
        return sSimAasEditor;
    }

    public static SimAasEditor setSimAasEditor() {
        if (sSimAasEditor == null) {
            sSimAasEditor = new SimAasEditor(sContext);
        } else {
            throw new IllegalStateException("sAasExtension context could be set only once");
        }
        return sSimAasEditor;
    }

    public static SimSneEditor getSimSneEditor() {
        return sSimSneEditor;
    }

    public static SimSneEditor setSimSneEditor() {
        if (sSimSneEditor == null) {
            sSimSneEditor = new SimSneEditor(sContext);
        } else {
            throw new IllegalStateException("sSneExtension context could be set only once");
        }
        return sSimSneEditor;
    }

}
