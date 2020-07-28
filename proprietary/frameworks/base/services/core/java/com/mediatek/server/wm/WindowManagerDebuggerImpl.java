/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.server.wm;

//import static android.app.ActivityManager.StackId;
import com.android.server.wm.WindowManagerService;
import com.android.server.wm.WindowManagerDebugConfig;
import com.android.server.policy.PhoneWindowManager;
//import com.android.server.wm.PolicyControl;
import com.android.server.policy.WindowManagerPolicy.WindowState;

import android.os.Build;
import android.util.Slog;
import android.view.View;
import android.view.WindowManager;


import java.lang.reflect.Field;
import java.io.PrintWriter;


public class WindowManagerDebuggerImpl extends WindowManagerDebugger {
    private static final String TAG = "WindowManagerDebuggerImpl";

    public WindowManagerDebuggerImpl() {
        WMS_DEBUG_ENG = "eng".equals(Build.TYPE);
        WMS_DEBUG_USER = true;
    }

    /**
     * -d help : list the debug zone status
     * -d enable  <zone zone ...> : enable the debug zone
     * -d disable  <zone zone ...> : disable the debug zone
     * example:
     *    -d enable all
     *    -d disable DEBUG_RESIZE DEBUG_LAYOUT
     */
    @Override
    public void runDebug(PrintWriter pw, String[] args, int opti) {
        int mode = -1;
        String cmd = "help";
        if (opti < args.length) {
            cmd = args[opti];
            opti++;
        }

        if ("help".equals(cmd)) {
            mode = 0;
            pw.println("Window manager debug options:");
            pw.println("  -d enable <zone zone ...> : enable the debug zone");
            pw.println("  -d disable <zone zone ...> : disable the debug zone");
            pw.println("zone may be some of:");
            pw.println("  a[all]");
        } else if ("enable".equals(cmd)) {
            mode = 1;
        } else if ("disable".equals(cmd)) {
            mode = 2;
        } else {
            pw.println("Unknown debug argument: " + cmd + "; use \"-d help\" for help");
            return;
        }

        boolean setAll = false;
        Field[] fields = WindowManagerDebugConfig.class.getDeclaredFields();
        Field[] fieldsPolicy = PhoneWindowManager.class.getDeclaredFields();
        while (!setAll && (mode == 0 || opti < args.length)) {
            if (opti < args.length) {
                cmd = args[opti];
                opti++;
            }

            setAll = mode == 0 || "all".equals(cmd) || "a".equals(cmd);
            for (int i = 0; i < fields.length; ++i) {
                String name = fields[i].getName();
                if (name != null && (name.contains("DEBUG") || name.contains("SHOW") ||
                        name.equals("localLOGV"))) {
                    try {
                        if (setAll || name.equals(cmd)) {
                            if (mode != 0) { // enable or disable
                                fields[i].setAccessible(true);
                                fields[i].setBoolean(null, mode == 1);

                                if (name.equals("localLOGV")){
                                    WindowManagerService.localLOGV = (mode == 1);
                                }

                                for (int j = 0; j < fieldsPolicy.length; ++j) {
                                    String pname = fieldsPolicy[j].getName();
                                    if (pname.equals(name)) {
                                        fieldsPolicy[j].setAccessible(true);
                                        fieldsPolicy[j].setBoolean(null, mode == 1);
                                        break;
                                    }
                                }
                            }
                            pw.println(String.format("  %s = %b",
                                name, fields[i].getBoolean(null)));
                        }
                    } catch (IllegalAccessException  e) {
                        Slog.e(TAG, name + " setBoolean failed", e);
                    }
                }
            }
        }
    }

    @Override
    public void debugInterceptKeyBeforeQueueing(String tag, int keycode, boolean interactive,
            boolean keyguardActive, int policyFlags, boolean down, boolean canceled,
            boolean isWakeKey, boolean screenshotChordVolumeDownKeyTriggered, int result,
            boolean useHapticFeedback, boolean isInjected) {
        Slog.d(tag, "interceptKeyTq keycode=" + keycode
                + " interactive=" + interactive + " keyguardActive=" + keyguardActive
                + " policyFlags=" + Integer.toHexString(policyFlags)
                + " down =" + down + " canceled = " + canceled
                + " isWakeKey=" + isWakeKey
                + " mVolumeDownKeyTriggered =" + screenshotChordVolumeDownKeyTriggered
                + " result = " + result
                + " useHapticFeedback = " + useHapticFeedback
                + " isInjected = " + isInjected);
    }

    @Override
    public void debugApplyPostLayoutPolicyLw(String tag, WindowState win,
            WindowManager.LayoutParams attrs, WindowState mTopFullscreenOpaqueWindowState,
            WindowState attached, WindowState imeTarget,
            boolean dreamingLockscreen, boolean showingDream){
        Slog.i(tag, "applyPostLayoutPolicyLw Win " + win +
                ": win.isVisibleLw()=" + win.isVisibleLw() +
                ", win.hasDrawnLw()=" + win.hasDrawnLw() +
                ", win.isDrawnLw()=" + win.isDrawnLw() +
                ", attrs.type=" + attrs.type +
                ", attrs.privateFlags=#" + Integer.toHexString(attrs.privateFlags) +
                //", fl=#" + Integer.toHexString(PolicyControl.getWindowFlags(win, attrs)) +
                //", stackId=" + win.getStackId() +
                ", mTopFullscreenOpaqueWindowState=" + mTopFullscreenOpaqueWindowState +
                ", win.isGoneForLayoutLw()=" + win.isGoneForLayoutLw() +
                ", attached=" + attached +
                ", imeTarget=" + imeTarget +
                ", isFullscreen=" + attrs.isFullscreen() +
                ", normallyFullscreenWindows=" +
                //StackId.normallyFullscreenWindows(win.getStackId()) +
                ", mDreamingLockscreen=" + dreamingLockscreen +
                ", mShowingDream=" + showingDream);
    }

    @Override
    public void debugLayoutWindowLw(String tag, int adjust, int type, int fl,
                    boolean canHideNavigationBar, int sysUiFl) {
        Slog.v(tag, "layoutWindowLw " + ": sim=#" + Integer.toHexString(adjust)
            + ", type=" + type
            + ", flag=" + fl
            + ", canHideNavigationBar=" + canHideNavigationBar
            + ", sysUiFl=" + sysUiFl);
    }

    @Override
    public void debugGetOrientation(String tag, boolean displayFrozen,
                    int lastWindowForcedOrientation, int lastKeyguardForcedOrientation) {
        Slog.v(tag, "Checking window orientation: mDisplayFrozen=" + displayFrozen
                + ", mLastWindowForcedOrientation=" + lastWindowForcedOrientation
                + ", mLastKeyguardForcedOrientation=" + lastKeyguardForcedOrientation);
    }

    @Override
    public void debugGetOrientingWindow(String tag, WindowState w,
                    WindowManager.LayoutParams attrs,
                    boolean isVisible, boolean policyVisibilityAfterAnim,
                    int policyVisibility, boolean destroying){
        Slog.v(tag, w + " screenOrientation=" + attrs.screenOrientation
                + ", visibility=" + isVisible
                + ", mPolicyVisibilityAfterAnim=" + policyVisibilityAfterAnim
                + ", mPolicyVisibility=" + policyVisibility
                + ", destroying=" + destroying);
    }

    @Override
    public void debugPrepareSurfaceLocked(String tag, boolean isWallpaper, WindowState win,
            boolean wallpaperVisible, boolean isOnScreen, int policyVisibility,
            boolean hasSurface, boolean destroying, boolean lastHidden){
        Slog.v(tag, win + " prepareSurfaceLocked " +
                ", mIsWallpaper=" + isWallpaper +
                ", mWin.mWallpaperVisible=" + wallpaperVisible +
                ", w.isOnScreen=" + isOnScreen +
                ", w.mPolicyVisibility=" + policyVisibility +
                ", w.mHasSurface=" + hasSurface +
                ", w.mDestroying=" + destroying +
                ", mLastHidden=" + lastHidden);
    }

    @Override
    public void debugRelayoutWindow(String tag, WindowState win,
            int originType, int changeType) {
        Slog.e(tag, "Window : " + win + "changes the window type!!"
                + "\nOriginal type : " + originType
                + "\nChanged type : " + changeType);
    }

    @Override
    public void debugInputAttr(String tag, WindowManager.LayoutParams attrs) {
        Slog.v(tag, "Input attr :" + attrs);
    }

    @Override
    public void debugViewVisibility(String tag, WindowState win, int viewVisibility,
            int oldVisibility, boolean focusMayChange) {
        if (viewVisibility == View.VISIBLE && oldVisibility != View.VISIBLE) {
            Slog.i(tag, "Relayout " + win + ": oldVis=" + oldVisibility
                    + " newVis=" + viewVisibility
                    + " focusMayChange = " + focusMayChange);
        }
    }
}
