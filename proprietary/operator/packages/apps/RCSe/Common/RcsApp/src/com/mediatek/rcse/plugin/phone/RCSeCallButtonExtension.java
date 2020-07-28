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

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.PorterDuff.Mode;
import android.graphics.drawable.Drawable;
import android.telecom.Call;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.ImageButton;
import android.widget.ToggleButton;

import com.android.internal.telephony.CallManager;

import com.mediatek.incallui.ext.DefaultRCSeCallButtonExt;

import com.mediatek.rcs.R;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;

import java.util.HashMap;

public class RCSeCallButtonExtension extends DefaultRCSeCallButtonExt
        implements OnClickListener {
    /**
     * The Constant LOG_TAG.
     */
    private static final String LOG_TAG = "RCSeCallButtonExtension";
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_RECEIVE_FILE = 909;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE = 910;
    
    private static final int PERMISSION_REQUEST_CODE_IPMSG_VIDEO = 911;
    /**
     * The Constant DBG.
     */
    private static final boolean DBG = true;
    /**
     * The m rc se phone plugin.
     */
    private RCSePhonePlugin mRCSePhonePlugin;
    // private ViewGroup mEndSharingVideoButtonWrapper;
    /**
     * The m end sharing video button.
     */
    private ImageButton mEndSharingVideoButton;
    /**
     * The m share file button.
     */
    private ImageButton mShareFileButton;
    /**
     * The m share video button.
     */
    private ImageButton mShareVideoButton;
    /**
     * The m in call control area.
     */
    private ViewGroup mInCallControlArea;
    /**
     * The m dialpad button.
     */
    private ToggleButton mDialpadButton;
    /**
     * The m plugin context.
     */
    private Context mPluginContext;
    /**
     * The Constant ID_FT_BUTTON.
     */
    private static final int ID_FT_BUTTON = 15432345;
    /**
     * The Constant ID_VIDEO_BUTTON.
     */
    private static final int ID_VIDEO_BUTTON = 18432120;
    /**
     * The Constant VERTICAl_SPACER_WIDTH.
     */
    private static final int VERTICAl_SPACER_WIDTH = 20;
    /**
     * The Constant VERTICAl_SPACER_WIDTH_2.
     */
    private static final int VERTICAl_SPACER_WIDTH_2 = 35;

    private View mVerticalSpacerFile;
    
    private View mVerticalSpacerBeforeFile;
    
    private View mVerticalSpacerAfterVideo;

    /**
     * Instantiates a new RC se call button extension.
     *
     * @param context the context
     * @param rcsePhonePlugin the rcse phone plugin
     */
    public RCSeCallButtonExtension(Context context) {
        mPluginContext = context;
        RCSePhonePlugin.initialize(context);
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
     * @param inCallTouchUi the in call touch ui
     */
    public void onViewCreated(Context context, View inCallTouchUi) {
    	try {
    	if(!RCSeUtils.isEnabled() || !RcsSettings.getInstance().isRichCallSupported()) 
            return;
        if (DBG) {
            log("onFinishInflate()...");
        }
        
        if(!hasPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {           
        RCSeInCallUIExtension.getInstance().mActivity.requestPermissions(new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE },
                    PERMISSION_REQUEST_CODE_IPMSG_RECEIVE_FILE); 
        }  
   
        // !!!! Todo: add end sharing button, share file, share video button
        // dynamically
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        mInCallControlArea = (ViewGroup) inCallTouchUi
                .findViewById(resource.getIdentifier(
                        "callButtonContainer", "id", packageName));
        /*mEndSharingVideoButton =
                (ImageButton) inCallTouchUi.findViewById(resource.getIdentifier("endSharingVideo",
                        "id", packageName));
        mEndSharingVideoButton.setOnClickListener(this);*/
        // mEndSharingVideoButtonWrapper = (ViewGroup)
        // inCallTouchUi.findViewById(resource.getIdentifier("endSharingVideoWrapper",
        // "id", packageName));
        //mShareFileButton = (ImageButton) inCallTouchUi.findViewById(resource.getIdentifier(
        // "shareFileButton", "id", packageName));
        
        mVerticalSpacerFile = new View(context);
        mVerticalSpacerFile.setLayoutParams(new LayoutParams(VERTICAl_SPACER_WIDTH_2 , LayoutParams.FILL_PARENT));
        
        mVerticalSpacerBeforeFile = new View(context);
        mVerticalSpacerBeforeFile.setLayoutParams(new LayoutParams(VERTICAl_SPACER_WIDTH , LayoutParams.FILL_PARENT));
        
        mVerticalSpacerAfterVideo = new View(context);
        mVerticalSpacerAfterVideo.setLayoutParams(new LayoutParams(VERTICAl_SPACER_WIDTH , LayoutParams.FILL_PARENT));
        
        mShareFileButton = new ImageButton(context);
        mShareFileButton.setLayoutParams(new LayoutParams(
                LayoutParams.WRAP_CONTENT, LayoutParams.FILL_PARENT));
        Drawable shareFileDrawable = mPluginContext.getResources()
                .getDrawable(R.drawable.btn_share_file);
        mShareFileButton.setEnabled(true);
        mShareFileButton.setClickable(true);
        if (shareFileDrawable != null) {
            if (DBG) {
                log("onFinishInflate()2-image share drawable is not null");
            }
            mShareFileButton.setBackgroundDrawable(shareFileDrawable);
        } else {
            if (DBG) {
                log("onFinishInflate()-image share drawable is null");
            }
        }
        mInCallControlArea.addView(mVerticalSpacerBeforeFile, 4);
        mVerticalSpacerBeforeFile.setVisibility(View.GONE);
        mInCallControlArea.addView(mShareFileButton, 5);
        mShareFileButton.setOnClickListener(this);
        mShareFileButton.setVisibility(View.GONE);
        mShareFileButton.setId(ID_FT_BUTTON);
        mShareVideoButton = new ImageButton(context);
        mShareVideoButton.setLayoutParams(new LayoutParams(
                LayoutParams.WRAP_CONTENT, LayoutParams.FILL_PARENT));
        mShareVideoButton.setEnabled(true);
        mShareVideoButton.setClickable(true);
        mShareVideoButton.setVisibility(View.GONE);
        mShareVideoButton.setId(ID_VIDEO_BUTTON);
        Drawable shareVideoDrawable = mPluginContext.getResources()
                .getDrawable(R.drawable.btn_share_video);
        if (shareVideoDrawable != null) {
            if (DBG) {
                log("onFinishInflate()-video2 share drawable is not null");
            }
            mShareVideoButton
                    .setBackgroundDrawable(shareVideoDrawable);
        } else {
            if (DBG) {
                log("onFinishInflate()-video share drawable is null");
            }
        }
        mInCallControlArea.addView(mVerticalSpacerFile,6);
        mVerticalSpacerFile.setVisibility(View.GONE);
        mInCallControlArea.addView(mShareVideoButton, 7);
        mInCallControlArea.addView(mVerticalSpacerAfterVideo, 8);
        mVerticalSpacerAfterVideo.setVisibility(View.GONE);
        mShareVideoButton.setOnClickListener(this);
        mDialpadButton = (ToggleButton) inCallTouchUi
                .findViewById(resource.getIdentifier("dialpadButton",
                        "id", packageName));
        if (DBG) {
            log("onFinishInflating() values buttons are -file2 "
                    + mShareFileButton + "InCallControlarea "
                    + mInCallControlArea + "mvideobutton"
                    + mShareVideoButton + "dialpad" + mDialpadButton);
        }
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }
    /**
     * Sets the compound button background transparency.
     *
     * @param button the button
     * @param transparency the transparency
     */
    private void setCompoundButtonBackgroundTransparency(
            ImageButton button, int transparency) {
    }
    /**
     * Sets the image button gray.
     *
     * @param button the button
     * @param isImageShare the is image share
     * @param cm the cm
     */
    private void setImageButtonGray(ImageButton button,
            boolean isImageShare, CallManager cm) {
        Drawable originalIcon;
        Drawable grayScaleIcon;
        if (isImageShare) {
            originalIcon = mPluginContext.getResources().getDrawable(
                    R.drawable.richcall_image_share);
            if (originalIcon != null) {
                grayScaleIcon = RCSeUtils.canShareImage(cm) ? originalIcon
                        : convertDrawableToGrayScale(originalIcon);
                button.setImageDrawable(grayScaleIcon);
            }
        } else {
            originalIcon = mPluginContext.getResources().getDrawable(
                    R.drawable.richcall_video_share);
            if (originalIcon != null) {
                grayScaleIcon = RCSeUtils.canShareVideo(cm) ? originalIcon
                        : convertDrawableToGrayScale(originalIcon);
                button.setImageDrawable(grayScaleIcon);
            }
        }
    }
    /**
     * Handle menu item click.
     *
     * @param menuItem the menu item
     * @return true, if successful
     */
    public boolean handleMenuItemClick(MenuItem menuItem) {
    	if(!RCSeUtils.isEnabled() || !RcsSettings.getInstance().isRichCallSupported()) 
            return false;
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        if (menuItem.getItemId() == resource.getIdentifier(
                "menu_hold_voice", "id", packageName)) {
            if (DBG) {
                log("hold voice menu item is clicked");
            }
            if ((RCSeInCallUIExtension.isTransferingFile())) {
                RCSeInCallUIExtension.getShareFilePlugIn().stop();
            } else if (RCSeInCallUIExtension.isSharingVideo()) {
                RCSeInCallUIExtension.getShareVideoPlugIn().stop();
            }
        }
        return false;
    }
    /**
     * Convert drawable to gray scale.
     *
     * @param drawable the drawable
     * @return the drawable
     */
    private static Drawable convertDrawableToGrayScale(
            Drawable drawable) {
        if (drawable == null) {
            return null;
        }
        Drawable res = drawable.mutate();
        res.setColorFilter(Color.GRAY, Mode.SRC_IN);
        return res;
    }
    /**
     * Update bottom buttons.
     *
     * @param cm the cm
     */
    private void updateBottomButtons(CallManager cm) {
        if (RCSeUtils.canShare(cm)) {
            if(RCSeCallCardExtension.sWidth <= 480)
            {
                if (DBG) {
                    log("dialpad updateBottomButtons() gone + swidth" + RCSeCallCardExtension.sWidth);
                }
                if (null != mDialpadButton) {
                    mDialpadButton.setVisibility(View.GONE);
                }
            }
            if (RCSeInCallUIExtension.isSharingVideo()) {
                if (DBG) {
                    log("updateBottomButtons(), is sharing video");
                }
                if (null != mEndSharingVideoButton) {
                    mEndSharingVideoButton
                            .setVisibility(View.VISIBLE);
                }
                if (null != mShareFileButton) {
                    mShareFileButton.setVisibility(View.VISIBLE);
                }
                if (null != mShareVideoButton) {
                    mShareVideoButton.setVisibility(View.VISIBLE);
                }
                if (null != mInCallControlArea) {
                    Drawable drawable = mInCallControlArea
                            .getBackground();
                    if (drawable != null) {
                        drawable.setAlpha(200);
                    }
                }
            } else {
                if (DBG) {
                    log("updateBottomButtons(), not sharing video");
                }
                if (null != mEndSharingVideoButton) {
                    mEndSharingVideoButton.setVisibility(View.GONE);
                }
                if (null != mShareFileButton) {
                    if (!RCSeUtils.canShareImage(cm)) {
                        if (DBG) {
                            log("sharing updateBottomButtons() can not share image");
                        }
                        mShareFileButton.setEnabled(false);
                        mShareFileButton.setClickable(false);
                    } else {
                        if (DBG) {
                            log("sharing updateBottomButtons() can share image");
                        }
                        if(hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)) {
                            if (DBG) {
                                log("updateState() hasPermission File True");
                            } 
                        } else {
                            if (DBG) {
                        }  
                        }  
                        mShareFileButton.setEnabled(true);
                        mShareFileButton.setClickable(true);
                    }
                    mShareFileButton.setVisibility(View.VISIBLE);
                }
                if (null != mShareVideoButton) {
                    if (!RCSeUtils.canShareVideo(cm)) {
                        mShareVideoButton.setEnabled(false);
                        mShareFileButton.setClickable(false);
                    } else {
                        mShareVideoButton.setEnabled(true);
                        mShareFileButton.setClickable(true);
                    }
                    mShareVideoButton.setVisibility(View.VISIBLE);
                }
                if (null != mInCallControlArea) {
                    Drawable drawable = mInCallControlArea
                            .getBackground();
                    if (drawable != null) {
                        //drawable.setAlpha(255);
                    }
                }
                if (null != mShareFileButton) {
                    setImageButtonGray(mShareFileButton, true, cm);
                }
                if (null != mShareVideoButton) {
                    setImageButtonGray(mShareVideoButton, false, cm);
                }
            }
            if (null != mVerticalSpacerFile) {
            	mVerticalSpacerFile.setVisibility(View.VISIBLE);
            }
            if (null != mVerticalSpacerBeforeFile) {
            	mVerticalSpacerFile.setVisibility(View.VISIBLE);
            }
            if (null != mVerticalSpacerAfterVideo) {
            	mVerticalSpacerFile.setVisibility(View.VISIBLE);
            }
           } 
        else {
            if (null != mShareFileButton) {
                if (DBG) {
                    log("sharing updateBottomButtons() visibility gone");
                }
                mShareFileButton.setVisibility(View.GONE);
            }
            if (null != mShareVideoButton) {
                mShareVideoButton.setVisibility(View.GONE);
            }
            if (null != mDialpadButton) {
                mDialpadButton.setVisibility(View.VISIBLE);
            }
            if (null != mEndSharingVideoButton) {
                mEndSharingVideoButton.setVisibility(View.GONE);
            }
            if (null != mVerticalSpacerFile) {
            	mVerticalSpacerFile.setVisibility(View.GONE);
            }
            if (null != mVerticalSpacerBeforeFile) {
            	mVerticalSpacerFile.setVisibility(View.GONE);
            }
            if (null != mVerticalSpacerAfterVideo) {
            	mVerticalSpacerFile.setVisibility(View.GONE);
            }
            if (null != mInCallControlArea) {
                Drawable drawable = mInCallControlArea
                        .getBackground();
                if (drawable != null) {
                    drawable.setAlpha(255);
                }
            }
        }
    }
    /**
     * On state change.
     *
     * @param call the call
     * @param mCallMap the m call map
     */
    public void onStateChange(Call call,
            HashMap<String, Call> mCallMap) {
    	if(!RCSeUtils.isEnabled() || !RcsSettings.getInstance().isRichCallSupported()) 
            return;
        if (DBG) {
            log("updateState()");
        }
        RCSeUtils.setmFgCall(call);
        CallManager cm = CallManager.getInstance();
        updateBottomButtons(cm);
    }
    /* (non-Javadoc).
     * @see android.view.View.OnClickListener#onClick(android.view.View)
     */
    @Override
    public void onClick(View view) {
        int id = view.getId();
        if (DBG) {
            log("onClick(View " + view + ", id " + id + ")...");
        }
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        if (id == resource.getIdentifier("endSharingVideo", "id",
                packageName)) {
            if (DBG) {
                log("end sharing video button is clicked");
            }
            if (null != RCSeInCallUIExtension.getShareVideoPlugIn()) {
                RCSeInCallUIExtension.getShareVideoPlugIn().stop();
            }
        } else if (id == ID_FT_BUTTON) {
            if (DBG) {
                log("share file button is clicked");
            }
            if(hasPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                if (DBG) {
                    log("updateState() hasPermission File True");
                }               
                startFilePlugin();
            } else {
                if (DBG) {
                    log("updateState() hasPermission File False");
                } 
                RCSeInCallUIExtension.getInstance().mActivity.requestPermissions(new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE },
                        PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE);
            }  
            
        } else if (id == ID_VIDEO_BUTTON) {
            if (DBG) {
                log("share video button is clicked");
            }
            
            if(hasPermission(Manifest.permission.CAMERA)) {
                if (DBG) {
                    log("updateState() hasPermission Video True");
                }               
                startVideoPlugin();
            } else {
                if (DBG) {
                    log("updateState() hasPermission Video False");
                } 
                RCSeInCallUIExtension.getInstance().mActivity.requestPermissions(new String[] { Manifest.permission.CAMERA },
                        PERMISSION_REQUEST_CODE_IPMSG_VIDEO);
            } 
           
        }
    }
    public boolean hasPermission(final String permission) {
        final Context context = MediatekFactory.getApplicationContext();
        final int permissionState = mPluginContext.checkSelfPermission(permission);
        Logger.v("RCSeCallButtonExtension", "hasPermission() : permission = " + permission + " permissionState = " + permissionState);
        return permissionState == PackageManager.PERMISSION_GRANTED;

    }
    
    private void startFilePlugin() {
            if (null != RCSeInCallUIExtension.getShareFilePlugIn()) {
                String phoneNumber = RCSeUtils
                        .getRCSePhoneNumber(mRCSePhonePlugin
                                .getCallManager());
                if (null != phoneNumber) {
                    RCSeInCallUIExtension.getShareFilePlugIn().start(
                            phoneNumber);
                }
            }
            }
    
    private void startVideoPlugin() {
            if (null != RCSeInCallUIExtension.getInstance()
                    .getVideoSharePlugIn()) {
                String phoneNumber = RCSeUtils
                        .getRCSePhoneNumber(mRCSePhonePlugin
                                .getCallManager());
                log("phone number is" + phoneNumber);
                if (null != phoneNumber) {
                    RCSeInCallUIExtension.getShareVideoPlugIn()
                            .start(phoneNumber);
                }
            } else {
                log("share video plugin is null");
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
