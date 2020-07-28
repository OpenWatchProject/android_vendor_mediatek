/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.rcse.plugin.phone;

import java.io.File;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.telecom.Call;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.Toast;
import android.content.Intent;

import com.android.internal.telephony.CallManager;

import com.mediatek.incallui.ext.DefaultRCSeInCallExt;
import com.mediatek.incallui.ext.IInCallScreenExt;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;

/**
 * The Class RCSeInCallUIExtension.
 */
public class RCSeInCallUIExtension extends DefaultRCSeInCallExt {
    /**
     * The Constant LOG_TAG.
     */
    private static final String LOG_TAG = "RCSeInCallUIExtension";
    /**
     * The Constant DBG.
     */
    private static final boolean DBG = true;
    /**
     * The m plugin context.
     */
    private Context mPluginContext;
    /**
     * The m rc se phone plugin.
     */
    private RCSePhonePlugin mRCSePhonePlugin;
    /**
     * The m in call screen host.
     */
    private IInCallScreenExt mInCallScreenHost;
    /**
     * The m activity.
     */
    public Activity mActivity;
    /**
     * The m cm.
     */
    private CallManager mCM;
    /**
     * The Constant ID_LARGE_AREA_SHARING.
     */
    private static final int ID_LARGE_AREA_SHARING = 1234562;
    /**
     * The Constant ID_CENTER_AREA_SHARING.
     */
    private static final int ID_CENTER_AREA_SHARING = 1234563;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_RECEIVE_FILE = 909;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE = 910;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_VIDEO = 911;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_CAMERA = 912;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_GALLERY = 914;
    /**
     * The m share file host.
     */
    private ShareFileCallScreenHost mShareFileHost;
    /**
     * The m share video host.
     */
    private ShareVideoCallScreenHost mShareVideoHost;
    /**
     * The m share file plug in.
     */
    private ICallScreenPlugIn mShareFilePlugIn;

    /**
     * Gets the m share file plug in.
     *
     * @return the m share file plug in
     */
    public ICallScreenPlugIn getmShareFilePlugIn() {
        return mShareFilePlugIn;
    }
    /**
     * Sets the m share file plug in.
     *
     * @param mShareFilePlugIn the new m share file plug in
     */
    public void setmShareFilePlugIn(ICallScreenPlugIn mShareFilePlugIn) {
        this.mShareFilePlugIn = mShareFilePlugIn;
    }
    /**
     * Gets the video share plug in.
     *
     * @return the video share plug in
     */
    public ICallScreenPlugIn getVideoSharePlugIn() {
        if (DBG) {
            log("getVideoSharePlugIn(), mShareVideoPlugIn is "
                    + mShareVideoPlugIn);
        }
        return mShareVideoPlugIn;
    }

    /**
     * The m whole area.
     */
    public static RelativeLayout sWholeArea = null;
    /**
     * The m center area.
     */
    public static ViewGroup sCenterArea = null;
    /**
     * The m share video plug in.
     */
    private ICallScreenPlugIn mShareVideoPlugIn;
    /**
     * The s instance.
     */
    private static RCSeInCallUIExtension sInstance = null;

    private static final String RCS_SETTINGS_PATH = "data/data/com.orangelabs.rcs/databases/rcs_settings.db";

    /**
     * Initialize.
     *
     * @param context the context
     * @param rcsePhonePlugin the rcse phone plugin
     */
    public static void initialize(Context context,
            RCSePhonePlugin rcsePhonePlugin) {
        MediatekFactory.setApplicationContext(context);
        sInstance = new RCSeInCallUIExtension(context);
    }
    /**
     * Gets the single instance of RCSeInCallUIExtension.
     *
     * @return single instance of RCSeInCallUIExtension
     */
    public static RCSeInCallUIExtension getInstance() {
        return sInstance;
    }
    /**
     * Reset display area.
     */
    public static void resetDisplayArea() {
        if (DBG) {
            log("resetDisplayArea");
        }
        if (sWholeArea != null) {
            sWholeArea.setVisibility(View.INVISIBLE);
        }
        if (sCenterArea != null) {
            sCenterArea.setVisibility(View.INVISIBLE);
        }
        if (DBG) {
            log("ShareFileScreenHost::resetDisplayArea() exit"
                    + sWholeArea + "center area" + sCenterArea);
        }
    }
    /**
     * Instantiates a new RC se in call ui extension.
     *
     * @param context the context
     */
    public RCSeInCallUIExtension(Context context) {
        MediatekFactory.setApplicationContext(context);
        mPluginContext = context;
        RCSePhonePlugin.initialize(context);
        mRCSePhonePlugin = RCSePhonePlugin.getInstance();
        sInstance = this;
    }
    /**
     * Gets the host resources.
     *
     * @return the host resources
     */
    protected Resources getHostResources() {
        return mActivity.getResources();
    }
    /**
     * Gets the host package name.
     *
     * @return the host package name
     */
    protected String getHostPackageName() {
        return mActivity.getPackageName();
    }
    /**
     * On create.
     *
     * @param icicle the icicle
     * @param activity the activity
     * @param inCallScreenHost the in call screen host
     */
    public void onCreate(Bundle icicle, Activity activity,
            IInCallScreenExt inCallScreenHost) {
    	/*File dbFile = new File(RCS_SETTINGS_PATH);        
        if (!dbFile.exists()) {
            log("RCS profile not exists ");
            RCSeUtils.setEnabled(false);
            //return;
        }*/
        if (RcsSettings.getInstance() == null)
            RcsSettings.createInstance(activity);
        RCSeUtils.setEnabled(RcsSettings.getInstance().isServiceRegistered());       
    	if (DBG) {
            log("onCreate(), RCSe Call is  " + RCSeUtils.isRCSCall);
        }
        if(!RCSeUtils.isEnabled() || !RcsSettings.getInstance().isRichCallSupported()) 
            return;
        CallManager cm = CallManager.getInstance();
        if (DBG) {
            log("onCreate(), inCallScreen is " + activity);
        }
        mActivity = activity;
        mRCSePhonePlugin.setInCallScreenActivity(activity);
        mInCallScreenHost = inCallScreenHost;
        mCM = cm;
        mRCSePhonePlugin.setCallManager(cm);
        mShareFilePlugIn = new ImageSharingPlugin(mPluginContext);
        mShareVideoPlugIn = new VideoSharingPlugin(mPluginContext);
        mShareFileHost = new ShareFileCallScreenHost();
        mShareVideoHost = new ShareVideoCallScreenHost();
        if (DBG) {
            log("onCreate(), plugins are image " + mShareFilePlugIn
                    + "video" + mShareVideoPlugIn);
        }
        // set host to plug-in
        if (null != mShareFilePlugIn) {
            mShareFilePlugIn.setCallScreenHost(mShareFileHost);
        }
        if (null != mShareVideoPlugIn) {
            mShareVideoPlugIn.setCallScreenHost(mShareVideoHost);
        }
    }
    /**
     * On destroy.
     *
     * @param activity the activity
     */
    public void onDestroy(Activity activity) {
    	if(!RCSeUtils.isEnabled() || !RcsSettings.getInstance().isRichCallSupported()) 
            return;
        if (DBG) {
            log("onDestroy(), inCallScreen is " + activity);
        }
        if (mActivity == activity) {
            mActivity = null;
        }
        if (mShareFilePlugIn != null) {
            if (mShareFileHost == mShareFilePlugIn
                    .getCallScreenHost()) {
                mShareFilePlugIn.setCallScreenHost(null);
            }
            mShareFilePlugIn.clearSavedDialogs();
        }
        if (mShareVideoPlugIn != null) {
            if (mShareVideoHost == mShareVideoPlugIn
                    .getCallScreenHost()) {
                mShareVideoPlugIn.setCallScreenHost(null);
            }
            mShareVideoPlugIn.clearSavedDialogs();
        }
    }
    /**
     * On pause.
     *
     * @param activity the activity
     */
    public void onPause(Activity activity) {
        if (DBG) {
            log("onPauseRcse()");
        }
        if (mShareFilePlugIn != null) {
            mShareFilePlugIn.saveAlertDialogs();
        }
        if (mShareVideoPlugIn != null) {
            mShareVideoPlugIn.saveAlertDialogs();
        }
        if(Utils.isInVideoSharing()){
            if (DBG) {
                log("onResumeRcse video in progress()");
            }
            mShareVideoPlugIn.pauseSharing();
        }
    }
    /**
     * On resume.
     *
     * @param activity the activity
     */
    public void onResume(Activity activity) {
        if (DBG) {
            log("onResumeRcse()");
        }
        if (mShareFilePlugIn != null) {
            mShareFilePlugIn.showAlertDialogs();
        }
        if (mShareVideoPlugIn != null) {
            mShareVideoPlugIn.showAlertDialogs();
        }
        if(Utils.isInVideoSharing()){
            if (DBG) {
                log("onResumeRcse video in progress()");
            }
            mShareVideoPlugIn.restartSharing();
        }
    }
    // The function called by onPrepareOptionsMenu() to set visibility of menu
    // items
    /**
     * Setup menu items.
     *
     * @param menu the menu
     * @param call the call
     */
    public void setupMenuItems(Menu menu, Call call) {/*
        RCSeUtils.setmFgCall(call);
        int state = call.getState();
        boolean canHold = call.can(Call.Capabilities.HOLD);
        boolean canAddCall = call.can(Call.Capabilities.ADD_CALL);
        if (DBG) {
            log("setupMenuItems()");
        }
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        final MenuItem addMenu = menu.findItem(resource
                .getIdentifier("menu_add_call", "id", packageName));
        final MenuItem holdMenu = menu.findItem(resource
                .getIdentifier("menu_hold_voice", "id", packageName));
        if (RCSeUtils.canShare(mCM)) {
            if (DBG) {
                log("setupMenuItems(), can share");
            }
            if (isSharingVideo()) {
                if (DBG) {
                    log("setupMenuItems(), is sharing video");
                }
                // share video
                if (state == Call.State.IDLE) {
                    int size = menu.size();
                    for (int i = 0; i < size; ++i) {
                        menu.getItem(i).setVisible(false);
                    }
                    holdMenu.setVisible(canHold);
                    if (canHold) {
                        String title = null;
                        if (state == Call.State.ONHOLD) {
                            title = resource.getString(resource
                                    .getIdentifier(
                                            "incall_toast_unhold",
                                            "string", packageName));
                        } else {
                            title = "Hold";
                            //"string", packageName));
                        }
                        holdMenu.setTitle(title);
                    }
                    if (!ViewConfiguration.get(mActivity)
                            .hasPermanentMenuKey()) {
                        if (canAddCall) {
                            if (addMenu != null) {
                                addMenu.setVisible(true);
                            }
                        }
                    }
                }
            } else {
                if (DBG) {
                    log("setupMenuItems(), not share video");
                }
                holdMenu.setVisible(canHold);
                String title = null;
                if (state == Call.State.ONHOLD) {
                    title = resource.getString(resource
                            .getIdentifier("incall_toast_unhold",
                                    "string", packageName));
                } else {
                    title = "Hold";
                    //"string", packageName));
                }
                holdMenu.setTitle(title);
            }
        }
    */}
    /**
     * On phone state changed.
     *
     * @param cm the cm
     * @return true, if successful
     */
    public boolean onPhoneStateChanged(CallManager cm) {
        if (DBG) {
            log("onPhoneStateChanged(), cm = ");
        }
        if (RCSeUtils.canShareFromCallState(cm)) {
            String number = RCSeUtils.getRCSePhoneNumber(cm);
            if (null != number) {
                if (null != mShareFilePlugIn) {
                    mShareFilePlugIn
                            .registerForCapabilityChange(number);
                }
                if (null != mShareVideoPlugIn) {
                    mShareVideoPlugIn
                            .registerForCapabilityChange(number);
                }
            }
        }
        if (RCSeUtils.shouldStop(cm)) {
            if ((isTransferingFile())) {
                mShareFilePlugIn.stop();
            } else if (isSharingVideo()) {
                mShareVideoPlugIn.stop();
            }
        }
        if (null != mShareFilePlugIn) {
            mShareFilePlugIn.onPhoneStateChange(cm);
        }
        if (null != mShareVideoPlugIn) {
            mShareVideoPlugIn.onPhoneStateChange(cm);
        }
        return false;
    }
    /**
     * Dismiss dialogs.
     *
     * @return true, if successful
     */
    public boolean dismissDialogs() {
        if (null != mShareFilePlugIn) {
            mShareFilePlugIn.dismissDialog();
        }
        if (null != mShareVideoPlugIn) {
            mShareVideoPlugIn.dismissDialog();
        }
        return false;
    }
    /**
     * Gets the share file plug in.
     *
     * @return the share file plug in
     */
    public static ICallScreenPlugIn getShareFilePlugIn() {
        return getInstance().mShareFilePlugIn;
    }
    /**
     * Gets the share video plug in.
     *
     * @return the share video plug in
     */
    public static ICallScreenPlugIn getShareVideoPlugIn() {
        if (DBG) {
            log("getShareVideoPlugIn(), mShareVideoPlugIn is "
                    + getInstance().mShareVideoPlugIn);
        }
        return getInstance().mShareVideoPlugIn;
    }
    /**
     * Checks if is capability to share.
     *
     * @param number the number
     * @return true, if is capability to share
     */
    public static boolean isCapabilityToShare(String number) {
        if (DBG) {
            log("isCapabilityToShare(), number = " + number);
        }
        ICallScreenPlugIn filePlugin = getShareFilePlugIn();
        ICallScreenPlugIn videoPlugin = getShareVideoPlugIn();
        if (null == filePlugin && null == videoPlugin) {
            if (DBG) {
                log("both plug-in are null, no capability");
            }
            return false;
        } else if (null != filePlugin
                && filePlugin.getCapability(number)) {
            if (DBG) {
                log("Share file plugIn has capability");
            }
            return true;
        } else if (null != videoPlugin
                && videoPlugin.getCapability(number)) {
            if (DBG) {
                log("Share video plugIn has capability");
            }
            return true;
        } else {
            if (DBG) {
                log("Neither plug-ins have capability");
            }
            return false;
        }
    }
    /**
     * Checks if is capability to share image.
     *
     * @param number the number
     * @return true, if is capability to share image
     */
    public static boolean isCapabilityToShareImage(String number) {
        if (DBG) {
            log("sharing isCapabilityToShareImage(), number = "
                    + number);
        }
        ICallScreenPlugIn filePlugin = getShareFilePlugIn();
        if (null == filePlugin) {
            if (DBG) {
                log("sharing file plug-in is null, no file share capability");
            }
            return false;
        } else if (filePlugin.isImageShareSupported(number)) {
            if (DBG) {
                log("sharing Share file plugIn has capability");
            }
            return true;
        } else {
            if (DBG) {
                log("sharing file plug-in has no capability");
            }
            return false;
        }
    }
    /**
     * Checks if is capability to share video.
     *
     * @param number the number
     * @return true, if is capability to share video
     */
    public static boolean isCapabilityToShareVideo(String number) {
        if (DBG) {
            log("isCapabilityToShareVideo(), number = " + number);
        }
        ICallScreenPlugIn videoPlugin = getShareVideoPlugIn();
        if (null == videoPlugin) {
            if (DBG) {
                log("Video sharing plug-in is null, no Video Sharing capability");
            }
            return false;
        } else if (videoPlugin.isVideoShareSupported(number)) {
            if (DBG) {
                log("Share video plugIn has capability");
            }
            return true;
        } else {
            if (DBG) {
                log("Video plug-in has no capability");
            }
            return false;
        }
    }
    /**
     * Checks if is transfering file.
     *
     * @return true, if is transfering file
     */
    public static boolean isTransferingFile() {
        ICallScreenPlugIn plugin = getShareFilePlugIn();
        if (null == plugin) {
            return false;
        }
        return Constants.SHARE_FILE_STATE_TRANSFERING == plugin
                .getState();
    }
    /**
     * Checks if is displaying file.
     *
     * @return true, if is displaying file
     */
    public static boolean isDisplayingFile() {
        ICallScreenPlugIn plugin = getShareFilePlugIn();
        if (null == plugin) {
            return false;
        }
        return Constants.SHARE_FILE_STATE_DISPLAYING == plugin
                .getState();
    }
    /**
     * Checks if is sharing video.
     *
     * @return true, if is sharing video
     */
    public static boolean isSharingVideo() {
        ICallScreenPlugIn plugin = getShareVideoPlugIn();
        if (null == plugin) {
            return false;
        }
        return Constants.SHARE_VIDEO_STATE_SHARING == plugin
                .getState();
    }
    /**
     * On disconnect.
     *
     * @param number the number
     * @return true, if successful
     */
    public boolean onDisconnect(String number) {
        if (DBG) {
            log("onDisconnect(), number =" + number);
        }
        dismissDialogs();
        if (null != mShareFilePlugIn) {
            //mShareFilePlugIn.stop();
            mShareFilePlugIn.unregisterForCapabilityChange(number);
            mShareFilePlugIn.clearSavedDialogs();
        }
        if (null != mShareVideoPlugIn) {
            //mShareVideoPlugIn.stop();
            mShareVideoPlugIn.unregisterForCapabilityChange(number);
            mShareVideoPlugIn.clearSavedDialogs();
        }
        return false;
    }
    /**
     * Update screen.
     *
     * @param callManager the call manager
     * @param isForegroundActivity the is foreground activity
     * @return true, if successful
     */
    public boolean updateScreen(CallManager callManager,
            boolean isForegroundActivity) {
        if (RCSeUtils.canShare(callManager)) {
            if (isSharingVideo() || isDisplayingFile()) {
                return false;
            }
        }
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        View inCallTouchUi = (View) mActivity.findViewById(resource
                .getIdentifier("bottomButtons", "id", packageName));
        if (null != inCallTouchUi) {
            inCallTouchUi.setVisibility(View.VISIBLE);
        }
        return false;
    }

    /*
     * public boolean handleOnscreenButtonClick(int id) { switch (id) { case
     * R.id.endSharingVideo: if (DBG)
     * log("end sharing video button is clicked"); if (null !=
     * mShareVideoPlugIn) { mShareVideoPlugIn.stop(); } return true; case
     * R.id.shareFileButton: if (DBG) log("share file button is clicked"); if
     * (null != mShareFilePlugIn) { String phoneNumber =
     * RCSeUtils.getRCSePhoneNumber(mCM); if (null != phoneNumber) {
     * mShareFilePlugIn.start(phoneNumber); } } return true; case
     * R.id.shareVideoButton: if (DBG) log("share video button is clicked"); if
     * (null != mShareVideoPlugIn) { String phoneNumber =
     * RCSeUtils.getRCSePhoneNumber(mCM); if (null != phoneNumber) {
     * mShareVideoPlugIn.start(phoneNumber); } } return true; } return false; }
     */
    /**
     * The Class ShareFileCallScreenHost.
     */
    public class ShareFileCallScreenHost implements ICallScreenHost {
        /**
         * Instantiates a new share file call screen host.
         */
        public ShareFileCallScreenHost() {
        }
        @Override
        public ViewGroup requestAreaForDisplay() {
            if (DBG) {
                log("ShareFileCallScreenHost::requestAreaForDisplay()");
            }
            sCenterArea = (ViewGroup) mActivity
                    .findViewById(ID_CENTER_AREA_SHARING);
            sCenterArea.setVisibility(View.VISIBLE);
            if (DBG) {
                log("ShareFileScreenHost::requestAreaForDisplay() exit"
                        + sCenterArea);
            }
            return sCenterArea;
        }
        @Override
        public void onStateChange(final int state) {
            if (DBG) {
                log("ShareFileCallScreenHost::onStateChange(), state = "
                        + state);
            }
            if (null != mInCallScreenHost) {
                mInCallScreenHost.requestUpdateScreen();
            }
        }
        @Override
        public void onCapabilityChange(String number,
                boolean isSupport) {
            final String contact = number;
            Logger.w(LOG_TAG, "onCapabilitiesChanged  contact0: "
                    + contact);
            if (DBG) {
                log("options ShareFileCallScreenHost::onCapabilityChange(), number = "
                        + number + ", isSupport = " + isSupport);
            }
            if (null != mInCallScreenHost) {
                Handler refresh = new Handler(Looper.getMainLooper());
                refresh.post(new Runnable() {
                    public void run() {
                        Logger.w(LOG_TAG,
                                "onCapabilitiesChanged  contact: "
                                        + contact);
                        mInCallScreenHost.requestUpdateScreen();
                    }
                });
                // mInCallScreenHost.requestUpdateScreen();
            }
        }
        @Override
        public Activity getCallScreenActivity() {
            return mActivity;
        }
    }

    /**
     * The Class ShareVideoCallScreenHost.
     */
    public class ShareVideoCallScreenHost implements ICallScreenHost {
        /**
         * Instantiates a new share video call screen host.
         */
        public ShareVideoCallScreenHost() {
        }
        @Override
        public ViewGroup requestAreaForDisplay() {
            if (DBG) {
                log("ShareVideoCallScreenHost::requestAreaForDisplay()");
            }
            Resources resource = getHostResources();
            String packageName = getHostPackageName();
            sWholeArea = (RelativeLayout) mActivity
                    .findViewById(ID_LARGE_AREA_SHARING);
            sWholeArea.setVisibility(View.INVISIBLE);
            if (DBG) {
                log("ShareVideoCallScreenHost::requestAreaForDisplay() exit"
                        + sWholeArea);
            }
            return sWholeArea;
        }
        @Override
        public void onStateChange(final int state) {
            if (DBG) {
                log("ShareVideoCallScreenHost::onStateChange(), state = "
                        + state);
            }
            if (null != mInCallScreenHost) {
                mInCallScreenHost.requestUpdateScreen();
            }
        }
        @Override
        public void onCapabilityChange(String number,
                boolean isSupport) {
            final String contact = number;
            Logger.w(LOG_TAG, "onCapabilitiesChanged  contact0: "
                    + contact);
            if (DBG) {
                log("options ShareVideoCallScreenHost::onCapabilityChange(), number = "
                        + number + ", isSupport = " + isSupport);
            }
            if (null != mInCallScreenHost) {
                Handler refresh = new Handler(Looper.getMainLooper());
                refresh.post(new Runnable() {
                    public void run() {
                        Logger.w(LOG_TAG,
                                "onCapabilitiesChanged  contact: "
                                        + contact);
                        mInCallScreenHost.requestUpdateScreen();
                    }
                });
                // mInCallScreenHost.requestUpdateScreen();
            }
        }
        @Override
        public Activity getCallScreenActivity() {
            return mActivity;
        }
    }

    public boolean isNeverGrantedPermission(String permission) {
        return !mActivity.shouldShowRequestPermissionRationale(permission);
    }

    public void onRCSeRequestPermissionsResult(
            final int requestCode, final String permissions[], final int[] grantResults) {
        Logger.d(LOG_TAG, "onRCSeRequestPermissionsResult requestCode:" + requestCode + "permissions:" + permissions);
        final long currentTimeMillis = SystemClock.elapsedRealtime();
        
        try {
        if (grantResults.length <= 0
                || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
            Logger.d(LOG_TAG, "onRCSeRequestPermissionsResult not granted");
            if(isNeverGrantedPermission(permissions[0])) {
                    Toast.makeText(
                            mActivity,
                            "Permission denied.You can change them in Settings->Apps.",
                            Toast.LENGTH_LONG).show();
            }
            if(requestCode == PERMISSION_REQUEST_CODE_IPMSG_RECEIVE_FILE){
                RCSeUtils.isFileReadPermissionsFirstTime = false;
            }
            return;
        } else {
            Logger.d(LOG_TAG, "onRCSeRequestPermissionsResult granted");
        }
        } catch (Exception e) {
            // TODO: handle exception
        }
        String phoneNumber = RCSeUtils
                .getRCSePhoneNumber(mRCSePhonePlugin
                        .getCallManager());
        // Now permissions are granted, start the feature
        if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_VIDEO ) {             
            getShareVideoPlugIn().start(phoneNumber);              
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE ) {
            getShareFilePlugIn().start(phoneNumber);
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_RECEIVE_FILE) {
            RCSeUtils.isFileReadPermissionsFirstTime = false;
        }
    
    }

    /**
     * Log.
     *
     * @param msg the msg
     */
    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
