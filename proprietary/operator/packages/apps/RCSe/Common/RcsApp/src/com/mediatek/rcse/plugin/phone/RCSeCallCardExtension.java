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
package com.mediatek.rcse.plugin.phone;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Point;
import android.graphics.drawable.Drawable;
import android.telecom.Call;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.android.internal.telephony.CallManager;

import com.mediatek.incallui.ext.DefaultRCSeCallCardExt;

import com.mediatek.rcs.R;
import com.mediatek.rcse.settings.RcsSettings;

/**
 * The Class RCSeCallCardExtension.
 */
public class RCSeCallCardExtension extends DefaultRCSeCallCardExt
        implements View.OnClickListener {
    /**
     * The Constant LOG_TAG.
     */
    private static final String LOG_TAG = "RCSeCallCardExtension";
    /**
     * The Constant DBG.
     */
    private static final boolean DBG = true;
    /**
     * The Constant ID_LARGE_AREA_SHARING.
     */
    private static final int ID_LARGE_AREA_SHARING = 1234562;
    /**
     * The Constant ID_CENTER_AREA_SHARING.
     */
    private static final int ID_CENTER_AREA_SHARING = 1234563;
    /**
     * The m rc se phone plugin.
     */
    private RCSePhonePlugin mRCSePhonePlugin;
    /**
     * The m plugin context.
     */
    private Context mPluginContext;
    /**
     * The m call card.
     */
    private View mCallCard;
    /**
     * The m call state label.
     */
    private TextView mCallStateLabel;
    /**
     * The m photo.
     */
    private ImageView mPhoto;
    /**
     * The m primary call banner.
     */
    private ViewGroup mPrimaryCallBanner;
    /**
     * The m center area.
     */
    private ViewGroup mCenterArea;
    /**
     * The m whole area.
     */
    private ViewGroup mWholeArea;
    /**
     * The m primary call info.
     */
    private View mPrimaryCallInfo;
    /**
     * The m phone number geo description.
     */
    private TextView mPhoneNumberGeoDescription;
    /**
     * The m density.
     */
    protected float mDensity;
    /**
     * The m is center area full screen.
     */
    private boolean mIsCenterAreaFullScreen;

    public static View sCallCard = null;

    public static int sWidth = 0;

    /**
     * Instantiates a new RC se call card extension.
     *
     * @param pluginContext the plugin context
     * @param rcsePhonePlugin the rcse phone plugin
     */
    public RCSeCallCardExtension(Context pluginContext) {
        mPluginContext = pluginContext;
    	RCSePhonePlugin.initialize(pluginContext);
        mRCSePhonePlugin = RCSePhonePlugin.getInstance();
    }
    /**
     * Gets the host resources.
     *
     * @return the host resources
     */
    protected Resources getHostResources() {
        return mRCSePhonePlugin.getInCallScreenActivity()
                .getResources();
    }
    /**
     * Gets the host package name.
     *
     * @return the host package name
     */
    protected String getHostPackageName() {
        return mRCSePhonePlugin.getInCallScreenActivity()
                .getPackageName();
    }
    /**
     * On view created.
     *
     * @param context the context
     * @param callCard the call card
     */
    public void onViewCreated(Context context, View callCard) {
        try {
        if (DBG) {
            log("onViewCreated()");
        }
    	 if (RcsSettings.getInstance() == null)
             RcsSettings.createInstance(mPluginContext);
        RCSeUtils.setEnabled(RcsSettings.getInstance().isServiceRegistered());       
     	if (DBG) {
             log("onCreate(), RCSe Call is  " + RCSeUtils.isRCSCall);
         }
    	if(!RCSeUtils.isEnabled() || !RcsSettings.getInstance().isRichCallSupported()) 
            return;       
        mCallCard = callCard;
        sCallCard = callCard;
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        FrameLayout hostCallCardArea = null; 
        hostCallCardArea = (FrameLayout) callCard.findViewById(resource
                .getIdentifier("answer_and_dialpad_container", "id", packageName));
        RelativeLayout mWholeArea = new RelativeLayout(context);
        mWholeArea.setId(ID_LARGE_AREA_SHARING);
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        sWidth = size.x;
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
        		FrameLayout.LayoutParams.MATCH_PARENT,
        		FrameLayout.LayoutParams.MATCH_PARENT);
        /*FrameLayout.LayoutParams paramsHost = new FrameLayout.LayoutParams(
        		FrameLayout.LayoutParams.MATCH_PARENT,
        		FrameLayout.LayoutParams.WRAP_CONTENT);*/
        if (DBG) {
            log("onViewCreated() PackageName = " + packageName
                    + "resource =" + resource + "callCrad ="
                    + callCard + "width=" + sWidth);
        }
        //hostCallCardArea.setLayoutParams(paramsHost);
        mWholeArea.setLayoutParams(params);
        ((ViewGroup) hostCallCardArea).addView(mWholeArea,params);
        mWholeArea.setVisibility(View.GONE);
        mWholeArea.setOnClickListener(this);
        RelativeLayout mCenterArea = new RelativeLayout(context);
        mCenterArea.setId(ID_CENTER_AREA_SHARING);
        RelativeLayout.LayoutParams centerParams = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        centerParams.addRule(RelativeLayout.CENTER_IN_PARENT);
        mCenterArea.setLayoutParams(centerParams);
        ((ViewGroup) hostCallCardArea).addView(mCenterArea, centerParams);
        mCenterArea.setVisibility(View.INVISIBLE);        
        mCallStateLabel = (TextView) callCard.findViewById(resource
                .getIdentifier("callStateLabel", "id", packageName));
        mPhoto = (ImageView) callCard.findViewById(resource
                .getIdentifier("photo", "id", packageName));
        mPrimaryCallBanner = (ViewGroup) callCard
                .findViewById(resource.getIdentifier("call_banner_1",
                        "id", packageName));
        mPrimaryCallInfo = callCard
                .findViewById(resource.getIdentifier(
                        "primary_call_banner", "id", packageName));		
			mPhoneNumberGeoDescription = (TextView) callCard
					.findViewById(resource.getIdentifier("location", "id",
							packageName));        	
        mDensity = resource.getDisplayMetrics().density;
        if (DBG) {
            log("onViewCreated() mcallstatelabel = "
                    + mCallStateLabel + "photo= " + mPhoto
                    + "primaryCallinfo =" + mPrimaryCallInfo
                    + " geoDesc =" + mPhoneNumberGeoDescription);
        }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    /*public boolean updateCallInfoLayout(PhoneConstants.State state) {
        if (DBG) {
            log("updateCallInfoLayout(), state = " + state);
        }
        if (shouldResetLayoutMargin()) {
            ViewGroup.MarginLayoutParams callInfoLp =
                    (ViewGroup.MarginLayoutParams) mCallCard.getLayoutParams();
            callInfoLp.bottomMargin = 0; // Equivalent to setting
            // android:layout_marginBottom in XML
            if (DBG) {
                log("  ==> callInfoLp.bottomMargin: 0");
            }
            mCallCard.setLayoutParams(callInfoLp);
            return true;
        } else {
            return false;
        }
    }*/
    /**
     * Updates the state of all UI elements on the CallCard, based on the
     * current state of the phone.
     *
     * @param call the call
     */
    public void onStateChange(Call call) {
    	if(!RCSeUtils.isEnabled() || !RcsSettings.getInstance().isRichCallSupported()) 
            return;
        CallManager cm = CallManager.getInstance();
        RCSeUtils.setmFgCall(call);
        if (RCSeUtils.canShare(cm)) {
            if (DBG) {
                log("updateState(), can share");
            }
            // have capability to share
            Drawable drawable = mPluginContext.getResources()
                    .getDrawable(R.drawable.ic_rcse_indicaton);
            if (null != drawable) {
                drawable.setBounds(0, 0,
                        drawable.getIntrinsicWidth(),
                        drawable.getIntrinsicHeight());
            } else {
                if (DBG) {
                    log("rcse indication icon drawable is null");
                }
            }
            if (null != mPhoneNumberGeoDescription
                    && View.VISIBLE == mPhoneNumberGeoDescription
                            .getVisibility()) {
            	 if (DBG) {
                     log("mPhoneNumberGeoDescription");
                 }
                mPhoneNumberGeoDescription.setCompoundDrawables(
                        drawable, null, null, null);
                mPhoneNumberGeoDescription
                        .setCompoundDrawablePadding((int) (mDensity * 5));
                mPhoneNumberGeoDescription.setVisibility(View.VISIBLE);
            } else {
                if (null != mCallStateLabel) {
                    mCallStateLabel.setCompoundDrawables(drawable,
                            null, null, null);
                    mCallStateLabel
                            .setCompoundDrawablePadding((int) (mDensity * 5));
                    mCallStateLabel.setVisibility(View.VISIBLE);
                    if (DBG) {
                        log("mCallStateLabel");
                    }
                }
            }
            if (RCSeInCallUIExtension.isTransferingFile()) {
                if (DBG) {
                    log("updateState(), is transfering file");
                }
                // share file
                if (null != mPhoto) {
                   // mPhoto.setVisibility(View.INVISIBLE);
                }
                if (null != mCenterArea) {
                    mCenterArea.setVisibility(View.VISIBLE);
                }
                if (mIsCenterAreaFullScreen) {
                    fullDisplayCenterArea(false);
                }
            } else if (RCSeInCallUIExtension.isDisplayingFile()) {
                if (DBG) {
                    log("updateState(), is displaying file");
                }
                if (null != mPhoto) {
                   // mPhoto.setVisibility(View.INVISIBLE);
                }
                if (null != mCenterArea) {
                    mCenterArea.setVisibility(View.VISIBLE);
                }
            } else {
                if (DBG) {
                    log("updateState(), not sharing file");
                }
                // not share file
                if (null != mPhoto) {
                    mPhoto.setVisibility(View.VISIBLE);
                }
                if (null != mCenterArea) {
                    mCenterArea.setVisibility(View.INVISIBLE);
                }
                if (mIsCenterAreaFullScreen) {
                    fullDisplayCenterArea(false);
                }
            }
            if (RCSeInCallUIExtension.isSharingVideo()) {
                if (DBG) {
                    log("updateState(), is sharing video");
                }
                // share video
                if (null != mWholeArea) {
                    mWholeArea.setVisibility(View.VISIBLE);
                }
                if (null != mPrimaryCallInfo) {
                    //mPrimaryCallInfo.setVisibility(View.INVISIBLE);
                }
            } else {
                if (DBG) {
                    log("updateState(), not sharing video");
                }
                if (null != mWholeArea) {
                    mWholeArea.setVisibility(View.INVISIBLE);
                }
                if (null != mPrimaryCallInfo) {
                    mPrimaryCallInfo.setVisibility(View.VISIBLE);
                }
                hideStatusBar(false);
            }
            if (RCSeInCallUIExtension.isSharingVideo()
                    || (RCSeInCallUIExtension.isDisplayingFile() && mIsCenterAreaFullScreen)) {
                hideStatusBar(true);
            } else {
                hideStatusBar(false);
            }
        } else {
            if (DBG) {
                log("updateState(), can not share");
            }
            // Clear out any icons
            if (null != mPhoneNumberGeoDescription
                    && View.VISIBLE == mPhoneNumberGeoDescription
                            .getVisibility()) {
                mPhoneNumberGeoDescription
                        .setCompoundDrawablesWithIntrinsicBounds(0,
                                0, 0, 0);
            } else {
                if (null != mCallStateLabel) {
                    mCallStateLabel
                            .setCompoundDrawablesWithIntrinsicBounds(
                                    0, 0, 0, 0);
                }
            }
            if (null != mCenterArea) {
                mCenterArea.setVisibility(View.INVISIBLE);
            }
            if (null != mWholeArea) {
                mWholeArea.setVisibility(View.INVISIBLE);
            }
            hideStatusBar(false);
            if (mIsCenterAreaFullScreen) {
                fullDisplayCenterArea(false);
            }
        }
        RCSeInCallUIExtension.getInstance().onPhoneStateChanged(cm);
    }
    @Override
    public void onClick(View view) {
        int id = view.getId();
        if (DBG) {
            log("onClick(View " + view + ", id " + id + ")...");
        }
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        View inCallTouchUi = mRCSePhonePlugin
                .getInCallScreenActivity().findViewById(
                        resource.getIdentifier("inCallTouchUi", "id",
                                packageName));
        if (id == resource.getIdentifier("largeAreaForSharing", "id",
                packageName)) {
            int visibility = inCallTouchUi.getVisibility();
            if (DBG) {
                log("large area for sharing is clicked, visibility is "
                        + visibility);
            }
            inCallTouchUi
                    .setVisibility(visibility == View.VISIBLE ? View.INVISIBLE
                            : View.VISIBLE);
            if (RCSeInCallUIExtension.isSharingVideo()) {
                if (DBG) {
                    log("is sharing video, so need to set stored video ui");
                }
            } else {
                if (DBG) {
                    log("is sharing image, so no need to set stored video ui");
                }
            }
        } else if (id == resource.getIdentifier(
                "centerAreaForSharing", "id", packageName)) {
            if (DBG) {
                log("center area for sharing is clicked");
            }
            if (RCSeInCallUIExtension.isDisplayingFile()) {
                hideStatusBar(!mIsCenterAreaFullScreen);
                inCallTouchUi
                        .setVisibility(mIsCenterAreaFullScreen ? View.INVISIBLE
                                : View.VISIBLE);
                fullDisplayCenterArea(!mIsCenterAreaFullScreen);
            }
        }
    }
    /**
     * Hide status bar.
     *
     * @param isHide the is hide
     */
    private void hideStatusBar(final boolean isHide) {
        if (DBG) {
            log("hideStatusBar(), isHide = " + isHide);
        }
        WindowManager.LayoutParams attrs = mRCSePhonePlugin
                .getInCallScreenActivity().getWindow()
                .getAttributes();
        if (isHide) {
            attrs.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
        } else {
            attrs.flags &= (~WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        mRCSePhonePlugin.getInCallScreenActivity().getWindow()
                .setAttributes(attrs);
    }
    /**
     * Full display center area.
     *
     * @param isFullDisplay the is full display
     */
    private void fullDisplayCenterArea(final boolean isFullDisplay) {
        if (DBG) {
            log("fullDisplayCenterArea(), isFullDisplay = "
                    + isFullDisplay);
        }
        if (isFullDisplay) {
            mIsCenterAreaFullScreen = true;
            //mPrimaryCallBanner.setVisibility(View.GONE);
            mCallStateLabel.setVisibility(View.GONE);
        } else {
            mIsCenterAreaFullScreen = false;
            //mPrimaryCallBanner.setVisibility(View.VISIBLE);
            mCallStateLabel.setVisibility(View.VISIBLE);
        }
    }
    /**
     * Should reset layout margin.
     *
     * @return true, if successful
     */
    private boolean shouldResetLayoutMargin() {
        if (DBG) {
            log("shouldResetLayoutMargin()");
        }
        if (!RCSeUtils.canShare(mRCSePhonePlugin.getCallManager())) {
            if (DBG) {
                log("Can not share, so no need reset layout margin");
            }
            return false;
        }
        if (RCSeInCallUIExtension.isSharingVideo()) {
            if (DBG) {
                log("is sharing video, so need reset layout margin");
            }
            return true;
        }
        if (RCSeInCallUIExtension.isDisplayingFile()
                && mIsCenterAreaFullScreen) {
            if (DBG) {
                log("is displaying file and full screen, so need reset layout margin");
            }
            return true;
        }
        return false;
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
