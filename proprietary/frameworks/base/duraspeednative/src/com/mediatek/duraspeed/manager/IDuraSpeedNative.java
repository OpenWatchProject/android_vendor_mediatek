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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
package com.mediatek.duraspeed.manager;

import android.content.Context;
import android.content.Intent;

import com.android.server.wm.ActivityRecord;

public interface IDuraSpeedNative {
    /**
     * Be notified when system ready.
     */
    public void onSystemReady();

    /**
     * Activity switch will call this event twice, once is before the last
     * activity pause, the twice is after the last activity pause.
     *
     * @param lastResumedActivity
     *            the need pause activity record.
     * @param nextResumedActivity
     *            the need resume activity record.
     * @param pausing
     *            the need pause activity pause done
     * @param nextResumedActivityType
     *            the need resume activity type
     */
    public void onBeforeActivitySwitch(ActivityRecord lastResumedActivity,
            ActivityRecord nextResumedActivity, boolean pausing, int nextResumedActivityType);

    /**
     * Wakeful change will call this event, WAKEFULNESS_AWAKE is screen on,
     * WAKEFULNESS_ASLEEP is screen off.
     *
     * @param wakefulness screen on type
     */
    public void onWakefulnessChanged(int wakefulness);

    /**
     * Init handler, receiver, and thread of duraspeed.
     *
     * @param context context of system server.
     */
    public void startDuraSpeedService(Context context);

    /**
     * Add for lmkd to inform systemserver, and it will trigger duraSpeed work.
     * If minFree value is -1, it means trigger by memory pressure. If memoryPressure
     * value is -1, it means trigger by min free level.
     * @param minFree current memory min free level.
     * @param memoryPressure
     *            the memory pressure of current system.
     */
    public void triggerMemory(int minFree, int memoryPressure);

    /**
     * Notify an app process is died.
     *
     * @param processName The died process name.
     * @param packageName The app package name which the died process belong to.
     */
    public void onAppProcessDied(String processName, String packageName);

    /**
     * When activity is idle, the idle means the activity is executed onResume(),
     * will trigger this.
     *
     * @param context current activity context.
     * @param idleIntent the intent.
     */
    public void onActivityIdle(Context context, Intent idleIntent);

    /**
     * Judge duraspeed is enabled or not.
     *
     * @return if duraspeed is enabled, return true, else return false.
     */
    public boolean isDuraSpeedEnabled();
}
