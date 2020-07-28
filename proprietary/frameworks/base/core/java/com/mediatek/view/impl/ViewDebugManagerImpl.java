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
package com.mediatek.view.impl;

import java.util.Date;
import java.util.HashMap;

import com.mediatek.view.ViewDebugManager;

import android.icu.text.SimpleDateFormat;
import android.os.SystemProperties;
import android.util.Log;
import android.view.InputEvent;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.MeasureSpec;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.ViewRootImpl;

public class ViewDebugManagerImpl extends ViewDebugManager{
    private static final String VIEW_LOG_TAG = "View";
    private static final String VIEWROOT_LOG_TAG = "ViewRoot";

    /** M: Add debug flags to partial enable logs. { */
    private static final String LOG_PROPERTY_NAME = "mtk_d.viewroot.enable";
    private static final int DEBUG_ENABLE_ALL_FLAG = 0x00000001;
    private static final int DEBUG_DRAW_FLAG = 0x00000002;
    private static final int DEBUG_LAYOUT_FLAG = 0x00000004;
    private static final int DEBUG_DIALOG_FLAG = 0x00000008;
    private static final int DEBUG_INPUT_RESIZE_FLAG = 0x00000010;
    private static final int DEBUG_ORIENTATION_FLAG = 0x00000020;
    private static final int DEBUG_TRACKBALL_FLAG = 0x00000040;
    private static final int DEBUG_IMF_FLAG = 0x00000080;
    private static final int DEBUG_CONFIGURATION_FLAG = 0x00000100;
    private static final int DEBUG_DEFAULT_FLAG = 0x00000200;
    private static final int DEBUG_FPS_FLAG = 0x00000400;
    private static final int DEBUG_HWUI_FLAG = 0x00000800;
    private static final int DEBUG_INPUT_FLAG = 0x00001000;
    private static final int DEBUG_KEY_FLAG = 0x00002000;
    private static final int DEBUG_MOTION_FLAG = 0x00004000;
    private static final int DEBUG_IME_ANR_FLAG = 0x00008000;
    private static final int DEBUG_LIFECYCLE_FLAG = 0x00010000;
    private static final int DEBUG_REQUESTLAYOUT_FLAG = 0x00020000;
    private static final int DEBUG_INVALIDATE_FLAG = 0x00040000;
    private static final int DEBUG_SCHEDULETRAVERSALS_FLAG = 0x00080000;
    private static final int DEBUG_TOUCHMODE_FLAG = 0x00100000;
    private static final int DEBUG_TOUCH_FLAG = 0x00200000;
    private static final int DEBUG_INPUT_STAGES_FLAG = 0x00400000;
    private static final int DEBUG_KEEP_SCREEN_ON_FLAG = 0x00800000;
    private static final int DEBUG_FOCUS_FLAG = 0x01000000;
    private static final int DEBUG_SYSTRACE_DRAW_FLAG = 0x02000000;
    private static final int DEBUG_SYSTRACE_MEASURE_FLAG = 0x04000000;
    private static final int DEBUG_SYSTRACE_LAYOUT_FLAG = 0x08000000;
    private static final int DEBUG_CHOREOGRAPHER_JANK_FLAG = 0x10000000;
    private static final int DEBUG_CHOREOGRAPHER_FRAMES_FLAG = 0x20000000;
    private static final int DEBUG_MET_TRACE_FLAG = 0x40000000;
    /** } */
    private static final int DBG_APP_DRAWING_MODE
                = SystemProperties.getInt("mtk_ro.mtk_dbg_app_drawing_mode", 0);
    private static final int DBG_APP_DRAWING_MODE_NOT_SET = 0x00000000;
    private static final int DBG_APP_DRAWING_MODE_FORCE_HWUI = 0x00000001;
    private static final int DBG_APP_DRAWING_MODE_FORCE_SW = 0x00000002;
    private static final int DBG_APP_DRAWING_MODE_FORCE_HWLAYER = 0x00000004;
    private static final int DBG_APP_DRAWING_MODE_FORCE_SWLAYER = 0x00000008;

    private static final boolean USE_RENDER_THREAD = false;
    /// M: for debug transparent region. use the same property as view
    private static final boolean DBG_TRANSP = SystemProperties.getBoolean(
             "mtk_d.view.transparentRegion", false);

    /// M: [APP launch time enhancenment feature] use property to control this feature. {
    static final boolean DBG_APP_LAUNCH_ENHANCE = true;

    static final boolean DBG_APP_FAST_LAUNCH_ENHANCE
              = (1 == SystemProperties.getInt("mtk_ro.mtk_perf_fast_start_win", 0)) ? true : false;
    /// }
    /// M: add constant value for log property.
    private static final int LOG_DISABLED = 0;
    private static final int LOG_ENABLED = 1;
    /// M: instance id used to identify each ViewRootImpl instance.
    private static long sIdent = 0;
    private long mIdent;

    /// M: dump file path, name and format of the dump image.
    private static final String DUMP_IMAGE_PTAH = "/data/dump/";
    private static final String DATE_FORMAT_STRING = "yyyyMMdd_hhmmss";
    private static final String DUMP_IMAGE_FORMAT = ".png";

    private HashMap<Object, Long> mInputStageRecored = new HashMap<Object, Long>();

    public boolean debugForceHWDraw(boolean hwDraw) {
        if ((DBG_APP_DRAWING_MODE & DBG_APP_DRAWING_MODE_FORCE_HWUI)
                      == DBG_APP_DRAWING_MODE_FORCE_HWUI) {
            return true;
        } else if ((DBG_APP_DRAWING_MODE & DBG_APP_DRAWING_MODE_FORCE_SW)
                      == DBG_APP_DRAWING_MODE_FORCE_SW) {
            return false;
        } else {
           return hwDraw;
        }
    }

    public int debugForceHWLayer(int hwLayer) {
        if ((DBG_APP_DRAWING_MODE & DBG_APP_DRAWING_MODE_FORCE_HWLAYER)
                      == DBG_APP_DRAWING_MODE_FORCE_HWLAYER) {
            return View.LAYER_TYPE_HARDWARE;
        } else if ((DBG_APP_DRAWING_MODE & DBG_APP_DRAWING_MODE_FORCE_SWLAYER)
                      == DBG_APP_DRAWING_MODE_FORCE_SWLAYER) {
            return View.LAYER_TYPE_SOFTWARE;
        } else {
           return hwLayer;
        }
    }

    public void debugKeyDispatch(View v, KeyEvent event){
        /// M : add log to help debugging and swith by system property
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            Log.i(VIEW_LOG_TAG, "Key down dispatch to " + v + ", event = " + event);
        } else if (event.getAction() == KeyEvent.ACTION_UP) {
            Log.i(VIEW_LOG_TAG, "Key up dispatch to " + v + ", event = " + event);
        }
    }

    public void debugEventHandled(View v, InputEvent event, String handler){
        Log.i(VIEW_LOG_TAG, "Event handle in " + v + ", event = " + event
                + ", handler = " + handler);
    }

    public void debugTouchDispatched(View v, MotionEvent event){
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            Log.i(VIEW_LOG_TAG, "Touch down dispatch to " + v + ", event x = "
                + event.getX() + ",y = " + event.getY());
        } else if (event.getAction() == MotionEvent.ACTION_UP) {
            Log.i(VIEW_LOG_TAG, "Touch up dispatch to " + v + ", event x = "
                + event.getX() + ",y = " + event.getY());
        } else {
            Log.d(VIEW_LOG_TAG, "(View)dispatchTouchEvent: event action = "
                + MotionEvent.actionToString(event.getAction())+ ",x = "
                + event.getX() + ",y = " + event.getY() + ",this = " + v);
        }
    }

    public void warningParentToNull(View v) {
        if (ViewDebugManager.DEBUG_MOTION) {
            Log.d(VIEW_LOG_TAG, "assignParent to null: this = " + v
                             + ", callstack = ", new Throwable());
        }
        Log.d(VIEW_LOG_TAG, "[Warning] assignParent to null: this = " + v);
    }

    public void debugOnDrawDone(View v, long start){
        long nowTime = System.currentTimeMillis();
        if (nowTime - start > DBG_TIMEOUT_VALUE) {
            Log.d(VIEW_LOG_TAG, "[ANR Warning]onDraw time too long, this =" + v
                    + "time =" + (nowTime - start) + " ms");
        }
        if (ViewDebugManager.DEBUG_DRAW) {
            Log.d(VIEW_LOG_TAG, "onDraw done, this =" + v
                    + "time =" + (nowTime - start) + " ms");
        }
    }

    public long debugOnMeasureStart(View v, int widthMeasureSpec, int heightMeasureSpec,
            int oldWidthMeasureSpec, int oldHeightMeasureSpec){
        /// M: Monitor onLayout time if longer than 3s print log.
        if (ViewDebugManager.DEBUG_LAYOUT) {
            Log.d(VIEW_LOG_TAG, "view measure start, this = " + v + ", widthMeasureSpec = "
                    + MeasureSpec.toString(widthMeasureSpec) + ", heightMeasureSpec = "
                    + MeasureSpec.toString(heightMeasureSpec) + ", mOldWidthMeasureSpec = "
                    + MeasureSpec.toString(oldWidthMeasureSpec) + ", mOldHeightMeasureSpec = "
                    + MeasureSpec.toString(oldHeightMeasureSpec) + getViewLayoutProperties(v));
        }
        long logTime = System.currentTimeMillis();
        return logTime;
    }

    public void debugOnMeasureEnd(View v, long logTime) {
        long nowTime = System.currentTimeMillis();
        if (nowTime - logTime > ViewDebugManager.DBG_TIMEOUT_VALUE) {
            Log.d(VIEW_LOG_TAG, "[ANR Warning]onMeasure time too long, this =" + v
                    + "time =" + (nowTime - logTime) + " ms");
        }
        if (ViewDebugManager.DEBUG_LAYOUT) {
            Log.d(VIEW_LOG_TAG, "view onMeasure end (measure cache), this =" + v
                    + ", mMeasuredWidth = " + v.getMeasuredWidth() + ", mMeasuredHeight = "
                    + v.getMeasuredHeight() + ", time =" + (nowTime - logTime) + " ms");
        }
    }

    public void debugOnLayoutEnd(View v, long logTime) {
        long nowTime = System.currentTimeMillis();
        if (nowTime - logTime > ViewDebugManager.DBG_TIMEOUT_VALUE) {
            Log.d(VIEW_LOG_TAG, "[ANR Warning]onLayout time too long, this =" + v
                    + "time =" + (nowTime - logTime) + " ms");
        }
        if (ViewDebugManager.DEBUG_LAYOUT) {
            Log.d(VIEW_LOG_TAG, "view layout end, this =" + v
                    + ", time =" + (nowTime - logTime) + " ms");
        }
    }

    private String getViewLayoutProperties(View v) {
        StringBuilder out = new StringBuilder(128);
        out.append(", Padding = {" + v.getPaddingLeft() + ", " + v.getPaddingTop()
                + ", " + v.getPaddingRight() + ", " + v.getPaddingBottom() + "}");

        if (v.getLayoutParams() == null) {
            out.append(", BAD! no layout params");
        } else {
            out.append(", " + v.getLayoutParams().debug(""));
        }

        return out.toString();
    }

    private int getCurrentLevel(View view) {
        int level = 0;
        ViewParent parent = view.getParent();
        while (parent != null && parent instanceof View) {
            level++;
            View v = (View) parent;
            parent = v.getParent();
        }
        return level;
    }

    private String sizeToString(int size) {
        if (size == ViewGroup.LayoutParams.WRAP_CONTENT) {
            return "wrap-content";
        }
        if (size == ViewGroup.LayoutParams.MATCH_PARENT) {
            return "match-parent";
        }
        return String.valueOf(size);
    }

    public String debug(String output, ViewGroup.MarginLayoutParams params) {
        return output + "ViewGroup.MarginLayoutParams={ width=" + sizeToString(params.width)
                + ", height=" + sizeToString(params.height) + ", leftMargin=" + params.leftMargin
                + ", rightMargin=" + params.rightMargin + ", topMargin=" + params.topMargin
                + ", bottomMargin=" + params.bottomMargin + " }";
    }

    public void debugViewRemoved(View child, ViewGroup parent, Thread rootThread) {
        if (parent.getViewRootImpl() != null
                && rootThread != Thread.currentThread()) {
            Log.e(VIEW_LOG_TAG, "[Warning] remove view from parent not in UIThread: parent = "
                       + parent + " view == " + child);
        }
        if (ViewDebugManager.DEBUG_LIFECYCLE) {
            Log.e(VIEW_LOG_TAG, "will remove view from parent " + parent
                     + " view == " + child, new Throwable());
        }
    }

    public void debugViewGroupChildMeasure(View child, View parent,
            ViewGroup.MarginLayoutParams lp, int widthUsed,  int heightUsed) {
        int level = getCurrentLevel(parent);
        Log.d(VIEW_LOG_TAG, "[ViewGroup][measureChildWithMargins] +" + level + " , child = "
                + child + ", child margin (L,R,T,B) = " + lp.leftMargin + "," + lp.rightMargin
                + "," + lp.topMargin + "," + lp.bottomMargin + ", widthUsed = " + widthUsed
                + ", heightUsed = " + heightUsed + ", parent padding (L,R,T,B) = "
                + parent.getPaddingLeft() + "," + parent.getPaddingRight() + ","
                + parent.getPaddingTop() + "," + parent.getPaddingBottom()
                + ", this = " + this);
    }

    public void debugViewGroupChildMeasure(View child, View parent,
            ViewGroup.LayoutParams lp, int widthUsed,  int heightUsed) {
        int level = getCurrentLevel(parent);
        Log.d(VIEW_LOG_TAG, "[ViewGroup][measureChildWithMargins] +" + level + " , child = "
                + child + ", child params (width, height) = " + lp.width + "," + lp.height
                + ", widthUsed = " + widthUsed
                + ", heightUsed = " + heightUsed + ", parent padding (L,R,T,B) = "
                + parent.getPaddingLeft() + "," + parent.getPaddingRight() + ","
                + parent.getPaddingTop() + "," + parent.getPaddingBottom()
                + ", this = " + this);
    }

    public void debugViewRootConstruct(String logTag, Object context, Object thread,
            Object chorgrapher, Object traversal, ViewRootImpl root) {
        /**
         * M: increase instance count and check log property to determine
         * whether to enable/disable log system. @{
         */
        root.mIdent = sIdent++;
        checkViewRootImplLogProperty();
        if (LOCAL_LOGV) {
            enableLog(logTag, true);
        }

        if (DEBUG_LIFECYCLE) {
            Log.v(logTag, "ViewRootImpl construct: context = " + context + ", mThread = " + thread
                    + ", mChoreographer = " + chorgrapher + ", mTraversalRunnable = "
                    + traversal + ", this = " + root);
        }
        /** @} */
    }

    /**
     * M: API used to switch on/off debug flags.
     *
     * @param enable enable or disable log.
     *
     * @hide
     */
    void enableLog(String logTag, boolean enable) {
        Log.v(logTag, "enableLog: enable = " + enable);
        LOCAL_LOGV = enable;
        DEBUG_DRAW = enable;
        DEBUG_LAYOUT = enable;
        DEBUG_DIALOG = enable;
        DEBUG_INPUT_RESIZE = enable;
        DEBUG_ORIENTATION = enable;
        DEBUG_TRACKBALL = enable;
        DEBUG_IMF = enable;
        DEBUG_CONFIGURATION = enable;
        DEBUG_FPS = enable;
        DEBUG_INPUT = enable;
        DEBUG_IME_ANR = enable;
        DEBUG_LIFECYCLE = enable;
        DEBUG_REQUESTLAYOUT = enable;
        DEBUG_INVALIDATE = enable;
        DEBUG_SCHEDULETRAVERSALS = enable;
    }

    /**
     * M: Get ViewRoot logs property and partial enable logs if debug flags is
     * set, you must recreate the activity to put it into effect.
     *
     * For example, you can use this function like
     * "adb shell setprop debug.viewroot.enable 1" to enable all logs, you can
     * also use "adb shell setprop debug.viewroot.enable 0" to disable all logs.
     */
    static void checkViewRootImplLogProperty() {
        final String propString = SystemProperties.get(LOG_PROPERTY_NAME);
        ViewDebugManager.DEBUG_USER = true;
        ViewDebugManager.DBG_TRANSP = DBG_TRANSP;
        if (propString != null && propString.length() > 0) {
            int logFilter = 0;
            try {
                logFilter = Integer.parseInt(propString, 16);
            } catch (NumberFormatException e) {
                Log.w(VIEWROOT_LOG_TAG, "Invalid format of propery string: " + propString);
            }

            Log.d(VIEWROOT_LOG_TAG, "checkViewRootImplLogProperty: propString = " + propString
                    + ",logFilter = #" + Integer.toHexString(logFilter));
            ViewRootImpl.LOCAL_LOGV = ViewDebugManager.LOCAL_LOGV =
                    (logFilter & DEBUG_ENABLE_ALL_FLAG) == DEBUG_ENABLE_ALL_FLAG;
            ViewRootImpl.DEBUG_DRAW = ViewDebugManager.DEBUG_DRAW =
                    (logFilter & DEBUG_DRAW_FLAG) == DEBUG_DRAW_FLAG;
            ViewRootImpl.DEBUG_LAYOUT = ViewDebugManager.DEBUG_LAYOUT =
                    (logFilter & DEBUG_LAYOUT_FLAG) == DEBUG_LAYOUT_FLAG;
            ViewRootImpl.DEBUG_DIALOG = ViewDebugManager.DEBUG_DIALOG =
                    (logFilter & DEBUG_DIALOG_FLAG) == DEBUG_DIALOG_FLAG;
            ViewRootImpl.DEBUG_INPUT_RESIZE = ViewDebugManager.DEBUG_INPUT_RESIZE =
                    (logFilter & DEBUG_INPUT_RESIZE_FLAG) == DEBUG_INPUT_RESIZE_FLAG;
            ViewRootImpl.DEBUG_ORIENTATION =  ViewDebugManager.DEBUG_ORIENTATION =
                    (logFilter & DEBUG_ORIENTATION_FLAG) == DEBUG_ORIENTATION_FLAG;
            ViewRootImpl.DEBUG_TRACKBALL = ViewDebugManager.DEBUG_TRACKBALL =
                    (logFilter & DEBUG_TRACKBALL_FLAG) == DEBUG_TRACKBALL_FLAG;
            ViewRootImpl.DEBUG_IMF = ViewDebugManager.DEBUG_IMF =
                    (logFilter & DEBUG_IMF_FLAG) == DEBUG_IMF_FLAG;
            ViewRootImpl.DEBUG_CONFIGURATION = ViewDebugManager.DEBUG_CONFIGURATION =
                    (logFilter & DEBUG_CONFIGURATION_FLAG) == DEBUG_CONFIGURATION_FLAG;
            ViewRootImpl.DBG = ViewDebugManager.DBG =
                    (logFilter & DEBUG_DEFAULT_FLAG) == DEBUG_DEFAULT_FLAG;
            ViewRootImpl.DEBUG_FPS = ViewDebugManager.DEBUG_FPS =
                    (logFilter & DEBUG_FPS_FLAG) == DEBUG_FPS_FLAG;
            ViewRootImpl.DEBUG_INPUT_STAGES = ViewDebugManager.DEBUG_INPUT_STAGES =
                    (logFilter & DEBUG_INPUT_STAGES_FLAG) == DEBUG_INPUT_STAGES_FLAG;
            ViewRootImpl.DEBUG_KEEP_SCREEN_ON = ViewDebugManager.DEBUG_KEEP_SCREEN_ON =
                    (logFilter & DEBUG_KEEP_SCREEN_ON_FLAG) == DEBUG_KEEP_SCREEN_ON_FLAG;
            ViewDebugManager.DEBUG_HWUI = (logFilter & DEBUG_HWUI_FLAG) == DEBUG_HWUI_FLAG;
            ViewDebugManager.DEBUG_INPUT = (logFilter & DEBUG_INPUT_FLAG) == DEBUG_INPUT_FLAG;
            ViewDebugManager.DEBUG_KEY = DEBUG_INPUT ||
                    (logFilter & DEBUG_KEY_FLAG) == DEBUG_KEY_FLAG;
            ViewDebugManager.DEBUG_MOTION = DEBUG_INPUT ||
                    (logFilter & DEBUG_MOTION_FLAG) == DEBUG_MOTION_FLAG;
            ViewDebugManager.DEBUG_IME_ANR =
                    (logFilter & DEBUG_IME_ANR_FLAG) == DEBUG_IME_ANR_FLAG;
            ViewDebugManager.DEBUG_LIFECYCLE =
                    (logFilter & DEBUG_LIFECYCLE_FLAG) == DEBUG_LIFECYCLE_FLAG;
            ViewDebugManager.DEBUG_REQUESTLAYOUT =
                    (logFilter & DEBUG_REQUESTLAYOUT_FLAG) == DEBUG_REQUESTLAYOUT_FLAG;
            ViewDebugManager.DEBUG_INVALIDATE =
                    (logFilter & DEBUG_INVALIDATE_FLAG) == DEBUG_INVALIDATE_FLAG;
            ViewDebugManager.DEBUG_SCHEDULETRAVERSALS =
                    (logFilter & DEBUG_SCHEDULETRAVERSALS_FLAG) == DEBUG_SCHEDULETRAVERSALS_FLAG;
            ViewDebugManager.DEBUG_TOUCHMODE =
                    (logFilter & DEBUG_TOUCHMODE_FLAG) == DEBUG_TOUCHMODE_FLAG;
            ViewDebugManager.DEBUG_TOUCH = (logFilter & DEBUG_TOUCH_FLAG) == DEBUG_TOUCH_FLAG;
            ViewDebugManager.DEBUG_FOCUS = (logFilter & DEBUG_FOCUS_FLAG) == DEBUG_FOCUS_FLAG;
            ViewDebugManager.DEBUG_SYSTRACE_MEASURE =
                    (logFilter & DEBUG_SYSTRACE_MEASURE_FLAG) == DEBUG_SYSTRACE_MEASURE_FLAG;
            ViewDebugManager.DEBUG_SYSTRACE_LAYOUT =
                    (logFilter & DEBUG_SYSTRACE_LAYOUT_FLAG) == DEBUG_SYSTRACE_LAYOUT_FLAG;
            ViewDebugManager.DEBUG_SYSTRACE_DRAW =
                    (logFilter & DEBUG_SYSTRACE_DRAW_FLAG) == DEBUG_SYSTRACE_DRAW_FLAG;
            ViewDebugManager.DEBUG_CHOREOGRAPHER_JANK =
                    (logFilter & DEBUG_CHOREOGRAPHER_JANK_FLAG) == DEBUG_CHOREOGRAPHER_JANK_FLAG;
            ViewDebugManager.DEBUG_CHOREOGRAPHER_FRAMES =
                (logFilter & DEBUG_CHOREOGRAPHER_FRAMES_FLAG) == DEBUG_CHOREOGRAPHER_FRAMES_FLAG;
            ViewDebugManager.DEBUG_MET_TRACE =
                    (logFilter & DEBUG_MET_TRACE_FLAG) == DEBUG_MET_TRACE_FLAG;
        }
    }
    /**
     * M: Check whether to enable app launch time logs.
     *
     * @return true if the app launch time property is 1, else return false.
     */
    private static boolean checkAppLaunchTimeProperty() {
        return (LOG_ENABLED == SystemProperties.getInt(
                "persist.applaunchtime.enable", LOG_DISABLED));
    }

    /**
     * Dump the current key and motion event processing state, for ANR analysis.
     *
     * {
     * @hide
     */
    public void dumpInputDispatchingStatus(String logTag) {
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
        Date date = new Date(mKeyEventStartTime);
        Date date2 = new Date(mPreviousKeyEventFinishTime);
        long dispatchTime = System.currentTimeMillis() - mKeyEventStartTime;
        if (mCurrentKeyEvent == null) {
            Log.i(logTag, "ANR Key Analyze: No Key event currently.");
            Log.i(logTag, "ANR Key Analyze: Previeous Event " + mPreviousKeyEvent +  ",finish at "
                    + simpleDateFormat.format(date2));
        } else {
            Log.i(logTag, "Dispatch " + mCurrentKeyEvent + " status is " + mKeyEventStatus
                    + ",start at " + simpleDateFormat.format(date)
                    + ", spent " + dispatchTime + "ms.");
        }

        if (mCurrentMotion == null) {
            date2.setTime(mPreviousMotionEventFinishTime);
            Log.i(logTag, "ANR Motion Analyze: No motion event currently.");
            Log.i(logTag, "ANR Motion Analyze: Previeous Event " + mPreviousMotion
                    + ",finish at " + simpleDateFormat.format(date2));
        } else {
            date.setTime(mMotionEventStartTime);
            dispatchTime = System.currentTimeMillis() - mMotionEventStartTime;
            Log.i(logTag, "Dispatch " + mCurrentMotion + " status is " + mMotionEventStatus
                    + ",start" + " at " + simpleDateFormat.format(date)
                    + ", spent " + dispatchTime + "ms.");
        }
        /// M: [ANR] Add for monitoring stage status. {
        dumpInputStageInfo(logTag, simpleDateFormat);
        clearInputStageInfo();
        /// }
    }

    /// M: [ANR] Add for monitoring input stage status{
    public void debugInputStageDeliverd(Object stage, long time){
        mInputStageRecored.put(stage, time);
    }

    void clearInputStageInfo() {
        mInputStageRecored.clear();
    }

    void dumpInputStageInfo(String logTag, SimpleDateFormat sdf) {
        if (!mInputStageRecored.isEmpty()) {
            for (Object obj: mInputStageRecored.keySet()) {
                long dt = mInputStageRecored.get(obj);
                Date deliveredTime = new Date(dt);
                if (dt != 0) {
                    Log.v(logTag, "Input event delivered to " + obj + " at "
                            + sdf.format(deliveredTime));
                }
            }
        }
    }
    /// }

    public void debugInputEventStart(InputEvent event) {
        /// M: record current key event and motion event to dump input event info for
        /// ANR analysis. {
        if (event instanceof KeyEvent) {
            mCurrentKeyEvent = (KeyEvent) event;
            mKeyEventStartTime = System.currentTimeMillis();
            mKeyEventStatus = INPUT_DISPATCH_STATE_STARTED;
        } else {
            mCurrentMotion = (MotionEvent) event;
            mMotionEventStartTime = System.currentTimeMillis();
            mMotionEventStatus = INPUT_DISPATCH_STATE_STARTED;
        }
        /// }
    }

    public void debugInputEventFinished(String logTag, boolean handled,
            InputEvent event, ViewRootImpl root) {
        /// M: record current key event and motion event to dump input event info for
        /// ANR analysis. {
        String stage;
        long inputElapseTime;
        long currentTime = System.currentTimeMillis();
        if (event instanceof KeyEvent) {
            mPreviousKeyEvent = mCurrentKeyEvent;
            mPreviousKeyEventFinishTime = System.currentTimeMillis();
            mCurrentKeyEvent = null;
            stage = mKeyEventStatus;
            mKeyEventStatus = INPUT_DISPATCH_STATE_FINISHED;
            inputElapseTime = currentTime - mKeyEventStartTime;
        } else {
            mPreviousMotion = mCurrentMotion;
            mPreviousMotionEventFinishTime = System.currentTimeMillis();
            mCurrentMotion = null;
            stage = mMotionEventStatus;
            mMotionEventStatus = INPUT_DISPATCH_STATE_FINISHED;
            inputElapseTime = currentTime - mMotionEventStartTime;
        }
        if (inputElapseTime >= INPUT_TIMEOUT) {
            SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
            Date enqueueTime = new Date(currentTime - inputElapseTime);
            Log.v(logTag, "[ANR Warning]Input routeing takes more than 6000ms since "
                    + simpleDateFormat.format(enqueueTime) + ", this = " + this);
            dumpInputStageInfo(logTag, simpleDateFormat);
        }
        clearInputStageInfo();
        /// }

        if (DEBUG_ENG || DEBUG_INPUT || DEBUG_KEY || DEBUG_MOTION) {
            if (event instanceof MotionEvent) {
                Log.v(logTag, "finishInputEvent: handled = " + handled + ",event action = "
                    + MotionEvent.actionToString(((MotionEvent)event).getAction())+ ",x = "
                    + ((MotionEvent)event).getX() + ",y = " + ((MotionEvent)event).getY()
                    + ", stage = " + stage);
             } else {
                Log.v(logTag, "finishInputEvent: handled = " + handled + ",event = " + event
                    + ", stage = " + stage);
             }
        }
    }

    public void debugInputDispatchState(InputEvent event, String state){
        if (event instanceof KeyEvent) {
            setKeyDispatchState(state);
        } else {
            setMotionDispatchState(state);
        }
    }

    void setKeyDispatchState(String state){
        mKeyEventStatus = state;
    }

    void setMotionDispatchState(String state){
        mMotionEventStatus = state;
    }

    public void debugTraveralDone(Object attachInfo, Object threadRender, boolean hwuiEnabled,
        ViewRootImpl root, boolean visable, boolean cancelDraw, String logTag){
        long frameCount = -999;
        String rendererAddr = "0x0";
        if (attachInfo != null && threadRender != null &&
            hwuiEnabled) {
            frameCount = root.mFrame;
        }
        Log.v(logTag, "ViewRoot performTraversals and draw- :"
                + " frame#" + frameCount + ", isViewVisible = " + visable
                + " (cancelDraw = " + cancelDraw + ")");
        root.mFrame++;
    }

    private String mKeyEventStatus = INPUT_DISPATCH_STATE_FINISHED;
    private String mMotionEventStatus = INPUT_DISPATCH_STATE_FINISHED;

    private KeyEvent mCurrentKeyEvent;
    private KeyEvent mPreviousKeyEvent;
    private MotionEvent mCurrentMotion;
    private MotionEvent mPreviousMotion;

    private long mKeyEventStartTime;
    private long mPreviousKeyEventFinishTime;
    private long mMotionEventStartTime;
    private long mPreviousMotionEventFinishTime;
    /// M: [ANR] Add for monitoring stage status. {
    public static final int INPUT_TIMEOUT = 6000;
    /// }

}
