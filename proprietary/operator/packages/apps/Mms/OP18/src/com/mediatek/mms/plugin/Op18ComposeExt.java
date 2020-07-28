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

package com.mediatek.op18.mms;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import com.mediatek.mms.callback.IComposeActivityCallback;
import com.mediatek.mms.ext.DefaultOpComposeExt;
import com.mediatek.mms.ext.IOpAttachmentEditorExt;
import com.mediatek.mms.ext.IOpWorkingMessageExt;
import com.mediatek.op18.mms.R;
import android.widget.TextView;
import android.os.Message;
import com.mediatek.setting.SettingListActivity;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;
import java.util.List;
import android.telephony.TelephonyManager;


import com.mediatek.mms.callback.ISlideshowModelCallback;
import com.mediatek.mms.callback.IWorkingMessageCallback;

/**
 * Op18ComposeExt.
 * Provides attachment button on editor and Dual send button on editor in case settin gis ON
 */
public class Op18ComposeExt extends DefaultOpComposeExt {
    private static final String TAG = "Mms/Op18MmsComposeExt";

    // TODO: Base id should be defined by Host

    private static final int MENU_ADD_ATTACHMENT = 2;
    private static final int MENU_ADD_QUICK_TEXT = 8;
    private static final int MENU_ADD_TEXT_VCARD = 9;
    private static final int MENU_MMS_SETTING = 1010;
    private MenuItem mAddAttachmentMenu;
    private IComposeActivityCallback mComposeCallback = null;
    private Activity mActivity;
    private Context mPluginContext;
    private LinearLayout mPanel;
    private ImageButton mShareButton;
    private SharePanel mSharePanel;
    private EditText mTextEditor;
    public static final int ACTION_SHARE = 1;
    private boolean mShowKeyBoardFromShare = false;
    private boolean mIsSmsEnabled = true;
    private boolean mIsSoftKeyBoardShow = false;
    private boolean mIsLandscape = false;
    private boolean mIsSecondSendButtonEnabled = false;
    private boolean mIsDualButtonSettingOn = false;
    private Object mWaitingImeChangedObject = new Object();

    private static final int mReferencedTextEditorTwoLinesHeight = 65;
    private static final int mReferencedTextEditorThreeLinesHeight = 110;
    private static final int mReferencedTextEditorFourLinesHeight    = 140;
    private static final int mReferencedTextEditorSevenLinesHeight = 224;

    private static final int ADD_IMAGE               = 0;
    private static final int TAKE_PICTURE            = 1;
    private static final int ADD_VIDEO               = 2;
    private static final int RECORD_VIDEO            = 3;
    private static final int ADD_SOUND               = 4;
    private static final int RECORD_SOUND            = 5;
    private static final int ADD_SLIDESHOW           = 6;
    private static final int ADD_VCARD               = 7;
    private static final int ADD_VCALENDAR           = 8;

    private Handler mUiHandler;
    private int mSendSubId = -1;

    ImageButton mSendButtonSIM1;
    ImageButton mSendButtonSIM2;

    ImageButton mSendButtonMMS1;
    ImageButton mSendButtonMMS2;
    TextView mTextViewMMS;

    private static final int MSG_RESUME_SEND_BUTTON = 112;
    private static final long RESUME_BUTTON_INTERVAL = 1000;

    int mSubId[] = {0};
    boolean sendFromEditor = false;

    /**
     * Op01ComposeExt Construction.
     * @param context Context.
     */
    public Op18ComposeExt(Context context) {
        super(context);
        Log.d(TAG, "context is " + context.getPackageName());
        mPluginContext = context;
    }
     @Override
    public boolean onOptionsItemSelected(MenuItem item, IOpWorkingMessageExt workingMessageExt,
            InputMethodManager inputMethodManager) {
        switch (item.getItemId()) {
            case MENU_ADD_ATTACHMENT:
                mComposeCallback.showAddAttachmentDialog(true);
                Log.d(TAG, "Attach: showAddAttachmentDialog(!hasAttachedFiles)");
                return true;
       case MENU_MMS_SETTING: {
        Intent settingIntent = new Intent(mActivity, SettingListActivity.class);
        mActivity.startActivity(settingIntent);
        return true;
        }
            default:
                return false;
        }
    }

    private static final String CURRENT_NETWORK_SMS_TEST = "current_network_sms";
    boolean mCheckSimStatus = false;

    @Override
    public void onCreate(IComposeActivityCallback ipComposeActivityCallback, Intent intent,
            IntentFilter intentFilter, Activity oldCompose, Activity compose,
            Bundle savedInstanceState, Handler uiHandler, ImageButton shareButton,
            LinearLayout panel, EditText textEditor) {
        mActivity = compose;
        mComposeCallback = ipComposeActivityCallback;
        mUiHandler = uiHandler;
    initResources();
        {
            mSubId = new int[2];
            mCheckSimStatus = CheckValidityOfSim();
            // getSubId();
            //TODO : need to check from api given by MMS Setting
            if (mCheckSimStatus == true
                && 1 == Settings.Global.getInt(mPluginContext.getContentResolver(),
                CURRENT_NETWORK_SMS_TEST,
                0)) {
                     Log.w(TAG, "onCreate mIsDualButtonSettingOn" + mIsDualButtonSettingOn);
                     mIsDualButtonSettingOn = true;
            } else {
                     Log.w(TAG, "onCreate mIsDualButtonSettingOn" + mIsDualButtonSettingOn);
                     mIsDualButtonSettingOn = false;
            }
//TODO
          /* if(mCheckSimStatus == true){
            mIsDualButtonSettingOn = true;
           }else{
            mIsDualButtonSettingOn = false;
           }*/

            mShareButton = shareButton;
            mShareButton.setVisibility(View.VISIBLE);

            mShareButton.setImageDrawable(mAttachIcon);
            ViewTreeObserver observer = mShareButton.getViewTreeObserver();
            observer.addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                public void onGlobalLayout() {
                    if (mShareButton != null && mAddAttachmentMenu != null) {
                        if (mShareButton.isShown()) {
                            mAddAttachmentMenu.setVisible(false);
                        } else {
                            mAddAttachmentMenu.setVisible(true);
                        }
                    }
                }
            });
            mTextEditor = textEditor;
            mSharePanel = new SharePanel(mPluginContext);
            mPanel = panel;
            mPanel.addView(mSharePanel);
            initShareResource();
        }

    }


    @Override
    public void onResume(int subCount, String text, boolean isSmsEnabled,
            View recipientsEditor, View subjectTextEditor) {
        mComposeCallback.setHadToSlideShowEditor(false);
       //TODO
        if (mCheckSimStatus == true
            && 1 == Settings.Global.getInt(mPluginContext.getContentResolver(),
                CURRENT_NETWORK_SMS_TEST,
                0)) {
                Log.w(TAG, "onResume mIsDualButtonSettingOn" + mIsDualButtonSettingOn);
                mIsDualButtonSettingOn = true;
        } else {
           Log.w(TAG, "onResume mIsDualButtonSettingOn" + mIsDualButtonSettingOn);
                   mIsDualButtonSettingOn = false;
        }
        getSubId();
            /*if(mCheckSimStatus == true){
                   mIsDualButtonSettingOn = true;
                  }else{
                   mIsDualButtonSettingOn = false;
                  }*/

        /// sharepanel
        if (mSharePanel != null) {
            if ((isViewVisible(recipientsEditor) && recipientsEditor.hasFocus())
                    || (isViewVisible(subjectTextEditor) && subjectTextEditor.hasFocus())
                    || (isViewVisible(mTextEditor) && mTextEditor.hasFocus())) {
                showSharePanel(false);
            }
            mIsSmsEnabled = isSmsEnabled;
            if (mIsSmsEnabled) {
                mShareButton.setClickable(true);
                mShareButton.setImageDrawable(mAttachIcon);
            } else {
                mShareButton.setClickable(false);
                mShareButton.setImageDrawable(mAttachIcon);
            }
        }
    }

    /*
    void test()
    {

      Settings.Global.getInt(mPluginContext.getContentResolver(),
                Settings.Global.CURRENT_NETWORK_SMS,
                0);
    }   */

    @Override
    public boolean onPrepareOptionsMenu(Menu menu, boolean isSmsEnabled,
            boolean isRecipientsEditorVisible, int contactsSize, long threadId) {
        // not allow to edit mms
        // hide add attachment menu if sharebutton shown
        mAddAttachmentMenu = menu.findItem(MENU_ADD_ATTACHMENT);
        if (mAddAttachmentMenu != null && mShareButton != null) {
            if (mShareButton.isShown()) {
                mAddAttachmentMenu.setVisible(false);
            } else {
                mAddAttachmentMenu.setVisible(true);
            }
        }
        menu.add(0, MENU_MMS_SETTING, 0, mMenuComposeScreenSetting);

        MenuItem renameMenu;


        try {
               renameMenu = menu.findItem(MENU_ADD_QUICK_TEXT);
               if (renameMenu != null) {
                   renameMenu.setTitle(mImportTemplate);
                }
                    } catch (IndexOutOfBoundsException e) {
                 Log.w(TAG, "Menu Item MENU_ADD_QUICK_TEXT not shown in this condition");
              }

                 try {
                        renameMenu = menu.findItem(MENU_ADD_TEXT_VCARD);
                        if (renameMenu != null) {
                         renameMenu.setVisible(false);
                        }
             } catch (IndexOutOfBoundsException e) {
                 Log.w(TAG, "Menu Item MENU_ADD_TEXT_VCARD not shown in this condition");
            }

        return true;
    }
       /// sharepanel begin
    public void onRecipientsEditorFocusChange(boolean hasFocus) {
        if (hasFocus) {
            showSharePanel(false);
        }
    }
    public void onRecipientsEditorTouch() {
        showSharePanel(false);
    }

    public void onSubjectTextEditorTouch() {
        showSharePanel(false);
    }

    public void onSubjectTextEditorFocusChange(boolean hasFocus) {
        if (hasFocus) {
            showSharePanel(false);
        }
    }

    public void onConfigurationChanged() {
        if (mSharePanel != null) {
            mSharePanel.resetShareItem();
        }
    }

    public void resetConfiguration(boolean isLandscapeOld,
            boolean isLandscapeNew, boolean isSoftKeyBoardShow) {
        if (mSharePanel == null) {
            return;
        }
        mIsLandscape = isLandscapeNew;
        if (!isLandscapeNew && isLandscapeOld == isLandscapeNew && isSoftKeyBoardShow) {
            showSharePanel(false);
        }
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
        case KeyEvent.KEYCODE_BACK:
            /// M:
            if (isSharePanelShow()) {
                hideSharePanel();
                return true;
            }
            break;
        }
        return false;
    }

    public void onTextEditorTouch() {
        if (mSharePanel == null) {
            return;
        }
        if (mShowKeyBoardFromShare) {
            showSharePanel(false);
            updateFullScreenTextEditorHeight();
        }
    }

    public boolean onLayoutSizeChanged(boolean isSoftKeyBoardShow) {
        if (mSharePanel == null) {
            return false;
        }
        mIsSoftKeyBoardShow = isSoftKeyBoardShow;
        return isSharePanelShow();
    }

    public boolean updateFullScreenTextEditorHeight() {
        return isSharePanelShow();
    }

    private void initShareResource() {
        if (mSharePanel != null) {
            mShareButton.setOnClickListener(new android.view.View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (mShowKeyBoardFromShare) {
                        showSharePanelOrKeyboard(false, true);
                    } else {
                        showSharePanelOrKeyboard(true, false);
                        mTextEditor.requestFocus();
                    }
                }
            });
            mSharePanel.setHandler(mMsgHandler);
            showSharePanelOrKeyboard(false, false);
        }
    }

    private void showSharePanel(boolean isShow) {
        if (null != mSharePanel) {
            if (!mIsSmsEnabled) {
                mSharePanel.setVisibility(View.GONE);
                mShareButton.setClickable(false);
                mShareButton.setImageDrawable(mDisableIcon);
                return;
            }
            if (isShow) {
                mSharePanel.setVisibility(View.VISIBLE);
                mShareButton.setImageDrawable(mKeyboardIcon);
            } else {
                mSharePanel.setVisibility(View.GONE);

                mShareButton.setImageDrawable(mShareIcon);
            }
            mShareButton.setClickable(true);
            mShowKeyBoardFromShare = isShow;
        }
    }

    public boolean isSharePanelShow() {
        if (null != mSharePanel && mSharePanel.isShown()) {
            return true;
        }
        return false;
    }

    public void showSharePanelOrKeyboard(final boolean isShowShare, final boolean isShowKeyboard) {
        if (mSharePanel == null) {
            return;
        }
        if (isShowShare && isShowKeyboard) {
            Log.w(TAG, "Can not show both SharePanel and Keyboard");
            return;
        }

        Log.d(TAG, "showSharePanelOrKeyboard(): isShowShare = " + isShowShare
                + ", isShowKeyboard = " + isShowKeyboard + ", mIsSoftKeyBoardShow = "
                + mIsSoftKeyBoardShow);
        if (!isShowKeyboard && mIsSoftKeyBoardShow && !mIsLandscape) {
            if (!isShowShare && mShowKeyBoardFromShare) {
                showSharePanel(isShowShare);
            }
            mShowKeyBoardFromShare = isShowShare;
            mComposeCallback.showKeyBoardCallback(isShowKeyboard);
            new Thread(new Runnable() {
                @Override
                public void run() {
                    synchronized (mWaitingImeChangedObject) {
                        try {
                            /// M: fix bug ALPS00447850, wait HideSoftKeyBoard longer
                            int waitTime = 300;
                            Log.d(TAG, "showSharePanelOrKeyboard(): object start wait.");
                            mWaitingImeChangedObject.wait(waitTime);
                            Log.d(TAG, "c(): object end wait.");
                        } catch (InterruptedException e) {
                            Log.d(TAG, "InterruptedException");
                        }
                    }
                    mActivity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (isShowShare) {
                                showSharePanel(isShowShare);
                                if (mIsLandscape) {
                                    mComposeCallback.setTextEditorMaxHeight(
                                            mReferencedTextEditorTwoLinesHeight);
                                } else {
                                    mComposeCallback.setTextEditorMaxHeight(
                                            mReferencedTextEditorFourLinesHeight);
                                }
                            } else {
                                Log.d(TAG, "showSharePanelOrKeyboard(): new thread.");
                                updateFullScreenTextEditorHeight();
                            }
                        }
                    });
                }
            }).start();
        } else {
            if (isShowShare && !isShowKeyboard && mIsLandscape) {
                mComposeCallback.showKeyBoardCallback(isShowKeyboard);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        synchronized (mWaitingImeChangedObject) {
                            try {
                                /// M: fix bug ALPS01297085, wait HideSoftKeyBoard longer
                                int waitTime = 100;
                                Log.d(TAG, "showSharePanelOrKeyboard:start wait");
                                mWaitingImeChangedObject.wait(waitTime);
                                Log.d(TAG, "c(): mIsLandscape object end wait.");
                            } catch (InterruptedException e) {
                                Log.d(TAG, "InterruptedException");
                            }
                        }
                        mActivity.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                showSharePanel(isShowShare);
                                mComposeCallback.setTextEditorMaxHeight(
                                        mReferencedTextEditorTwoLinesHeight);
                            }
                        });
                    }
                }).start();
            } else {
                mComposeCallback.showKeyBoardCallback(isShowKeyboard);
                showSharePanel(isShowShare);
                if (isShowShare || isShowKeyboard) {
                    if (mIsLandscape) {
                        mComposeCallback.setTextEditorMaxHeight(
                                mReferencedTextEditorTwoLinesHeight);
                    } else {
                        mComposeCallback.setTextEditorMaxHeight(
                                mReferencedTextEditorFourLinesHeight);
                    }
                } else {
                    Log.d(TAG, "showSharePanelOrKeyboard()");
                    updateFullScreenTextEditorHeight();
                }
            }
        }
    }

    public void hideSharePanel() {
        Log.d(TAG, "hideSharePanel()");
        if (mSharePanel == null) {
            return;
        }
        showSharePanelOrKeyboard(false, false);
        updateFullScreenTextEditorHeight();
    }

    private void doMmsAction(Message msg) {
        if (mSharePanel == null) {
            return;
        }
        int commonAttachmentType = 0;
        Bundle bundle = msg.getData();
        int action = bundle.getInt(SharePanel.SHARE_ACTION);
        switch (action) {
        case SharePanel.TAKE_PICTURE:
            commonAttachmentType = TAKE_PICTURE;
            break;

        case SharePanel.RECORD_VIDEO:
            commonAttachmentType = RECORD_VIDEO;
            break;

        case SharePanel.RECORD_SOUND:
            commonAttachmentType = RECORD_SOUND;
            break;

        case SharePanel.ADD_VCARD:
            commonAttachmentType = ADD_VCARD;
            break;

        case SharePanel.ADD_IMAGE:
            commonAttachmentType = ADD_IMAGE;
            break;

        case SharePanel.ADD_VIDEO:
            commonAttachmentType = ADD_VIDEO;
            break;


        case SharePanel.ADD_SOUND:
            commonAttachmentType = ADD_SOUND;
            break;

        case SharePanel.ADD_VCALENDAR:
            commonAttachmentType = ADD_VCALENDAR;
            break;

        case SharePanel.ADD_SLIDESHOW:
            commonAttachmentType = ADD_SLIDESHOW;
            break;

        default:
            Log.e(TAG, "invalid share action type: " + action);
            hideSharePanel();
            return;
        }

        mComposeCallback.addIpAttachment(commonAttachmentType, true);
        Log.d(TAG, "attach: addAttachment(commonAttachmentType, true)");
        /// @}

        hideSharePanel();
    }
    /// sharepanel end

    public Handler mMsgHandler = new Handler() {
        public void handleMessage(Message msg) {
            Log.d(TAG, "mMsgHandler handleMessage, msg.what: " + msg.what);
            switch (msg.what) {
            case ACTION_SHARE:
                doMmsAction(msg);
                break;
            default:
                Log.d(TAG, "msg type: " + msg.what + "not handler");
                break;
            }
            super.handleMessage(msg);
        }
    };

    private boolean isViewVisible(View view) {
        if (mSharePanel == null) {
            return false;
        }
        return (null != view)
                    && (View.VISIBLE == view.getVisibility());
    }


   private boolean CheckValidityOfSim() {

      try {

        SubscriptionManager subManager =
            (SubscriptionManager) getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
         List<SubscriptionInfo> subInfoList = subManager.getActiveSubscriptionInfoList();
         Log.d(TAG, " start CheckValidityOfSim");

         if (subInfoList != null) {
          int numOfSim = subInfoList.size();

          if (numOfSim < 1) {
            return false;
          }
          getSubId();
          if (getSimState(1) == false) {
           return false;
          }
          if (getSimState(2) == false) {
           return false;
          }
         return true;
         } else {
            return false;
         }
        } catch (NullPointerException e) {
            Log.d(TAG, "CheckValidityOfSim No Sim is valid");
            return false;
        }

   }




    private void getSubId() {
      try {
       SubscriptionManager subManager =
          (SubscriptionManager) getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
       List<SubscriptionInfo> subInfoList = subManager.getActiveSubscriptionInfoList();
       Log.d(TAG, " start getSubId");
       int size = subInfoList.size();
       Log.d(TAG, " size getSubId : " + size);

       for (int i = 0; i < size; i++) {
            int simPosition = subInfoList.get(i).getSimSlotIndex();
            mSubId[simPosition] = subInfoList.get(i).getSubscriptionId();
            Log.d(TAG, " has simPosition  == " + simPosition);
            Log.d(TAG, " has subID  == " + mSubId[i]);

         }
        } catch (NullPointerException e) {
             Log.d(TAG, " not valid sim for getSubId");
        }

        Log.d(TAG, "exit getSubId");
    }

    private boolean getSimState(int simNumber) {

        TelephonyManager telephony = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        int slotId = SubscriptionManager.getSlotIndex(mSubId[simNumber - 1]);
         Log.d(TAG, "enter getSimState slotId : " + slotId + "Sim Numb :" + simNumber);

        int simState = telephony.getSimState(slotId);

        if (simState != TelephonyManager.SIM_STATE_READY) {

          Log.d(TAG, "exit getSimState" + simState);
          return false;
        }

        Log.d(TAG, "true getSimState" + simState);
        return true;

    }


boolean bSecondSimSelected = false;


@Override
public boolean sendMessageForDualSendButton() {

  Log.d(TAG, "sendMessageForDualSendButton mIsDualButtonSettingOn " + mIsDualButtonSettingOn);

  if (sendFromEditor)
  {
    sendFromEditor = false;
    return false;
  }

 if (mIsDualButtonSettingOn == true) {
      if (bSecondSimSelected == false) {
             mComposeCallback.setSubIdCallback(mSubId[0]);
             Log.d(TAG, "sendMessageForDualSendButton 2" + mSubId[0]);
             mComposeCallback.updateSendButtonStateCallback(false);
      } else {
             bSecondSimSelected = false;
             mComposeCallback.setSubIdCallback(mSubId[1]);
             Log.d(TAG, "sendMessageForDualSendButton 5" + mSubId[1]);
            // mComposeCallback.setSubIdCallback(1);
      }
      Log.d(TAG, "sendMessageForDualSendButton 88");
      mComposeCallback.confirmSendMessageIfNeededCallback();
      return true;
    }
  return false;

}

     Drawable mImageSendButtonSim1;
     Drawable mImageSendButtonSim2;

     Drawable mImageSendButtonMMS1;
     Drawable mImageSendButtonMMS2;

     Drawable mImageUnSendButtonSim1;
     Drawable mImageUnSendButtonSim2;

     Drawable mImageUnSendButtonMMS1;
     Drawable mImageUnSendButtonMMS2;

     String mImportTemplate;
     String mMenuComposeScreenSetting;

     Drawable mShareIcon;
     Drawable mDisableIcon;
     Drawable mKeyboardIcon;
     Drawable mAttachIcon;

    private void initResources() {

       mImageSendButtonMMS1 = getResources().getDrawable(R.drawable.ic_send_mms_sim1);
       mImageSendButtonMMS2 = getResources().getDrawable(R.drawable.ic_send_mms_sim2);
       mImageSendButtonSim1 = getResources().getDrawable(R.drawable.ic_send_msg_sim1);
       mImageSendButtonSim2 = getResources().getDrawable(R.drawable.ic_send_msg_sim2);


       mImageUnSendButtonMMS1 = getResources().getDrawable(R.drawable.ic_send_mms_unsend_sim1);
       mImageUnSendButtonMMS2 = getResources().getDrawable(R.drawable.ic_send_mms_unsend_sim2);
       mImageUnSendButtonSim1 = getResources().getDrawable(R.drawable.ic_send_sms_unsend_sim1);
       mImageUnSendButtonSim2 = getResources().getDrawable(R.drawable.ic_send_sms_unsend_sim2);

       mImportTemplate =  getResources().getString(R.string.menuItem_composeScreen_importTemplate);
       mMenuComposeScreenSetting =
        getResources().getString(R.string.menuItem_composeScreen_setting);

       mShareIcon = getResources().getDrawable(R.drawable.ic_dialog_attach);
       mDisableIcon = getResources().getDrawable(R.drawable.ic_dialog_attach);
       mKeyboardIcon = getResources().getDrawable(R.drawable.ipmsg_keyboard);

       mAttachIcon = getResources().getDrawable(R.drawable.ic_dialog_attach);

    }

    void setMMS1Enabled() {
        try {
             mSendButtonMMS1.setImageDrawable(mImageSendButtonMMS1);
             mSendButtonMMS1.setVisibility(View.VISIBLE);
             mSendButtonMMS1.setEnabled(true);
        } catch (NullPointerException e) {
              Log.d(TAG, "setMMS1Enabled 88");
        }
    }


    void setMMS2Enabled() {
        try {
             mSendButtonMMS2.setImageDrawable(mImageSendButtonMMS2);
             mSendButtonMMS2.setVisibility(View.VISIBLE);
             mSendButtonMMS2.setEnabled(true);
        } catch (NullPointerException  e) {
              Log.d(TAG, "setMMS2Enabled 88");
        }
    }

    void setMMS1Disabled() {
        try {
             mSendButtonMMS1.setImageDrawable(mImageUnSendButtonMMS1);
             mSendButtonMMS1.setVisibility(View.VISIBLE);
             mSendButtonMMS1.setEnabled(false);
        } catch (NullPointerException  e) {
              Log.d(TAG, "setMMS1Disabled 88");
        }

    }

    void setMMS2Disabled() {
      try {
           mSendButtonMMS2.setImageDrawable(mImageUnSendButtonMMS2);
           mSendButtonMMS2.setVisibility(View.VISIBLE);
           mSendButtonMMS2.setEnabled(false);
      } catch (NullPointerException  e) {
              Log.d(TAG, "setMMS2Disabled 88");
        }
    }

    void setSMS1Enabled() {
       try {
          mSendButtonSIM1.setImageDrawable(mImageSendButtonSim1);
          mSendButtonSIM1.setVisibility(View.VISIBLE);
          mSendButtonSIM1.setEnabled(true);
        } catch (NullPointerException  e) {
              Log.d(TAG, "setSMS1Enabled 88");
        }


    }


    void setSMS2Enabled() {
        try {
             mSendButtonSIM2.setImageDrawable(mImageSendButtonSim2);
             mSendButtonSIM2.setVisibility(View.VISIBLE);
             mSendButtonSIM2.setEnabled(true);
        } catch (NullPointerException  e) {
              Log.d(TAG, "setSMS2Enabled 88");
        }
    }


    void setSMS1Disabled() {
        try {
             mSendButtonSIM1.setImageDrawable(mImageUnSendButtonSim1);
             mSendButtonSIM1.setVisibility(View.VISIBLE);
             mSendButtonSIM1.setEnabled(false);
        } catch (NullPointerException  e) {
              Log.d(TAG, "setSMS1Disabled 88");
        }

    }


    void setSMS2Disabled() {
       try {
            mSendButtonSIM2.setImageDrawable(mImageUnSendButtonSim2);
            mSendButtonSIM2.setVisibility(View.VISIBLE);
            mSendButtonSIM2.setEnabled(false);
       } catch (NullPointerException  e) {
            Log.d(TAG, "setSMS2Disabled 88");
        }
    }

  void hideAll() {
    try {
        if (mSendButtonMMS2 != null) {
         mSendButtonMMS2.setVisibility(View.GONE);
        }
        if (mSendButtonSIM2 != null) {
         mSendButtonSIM2.setVisibility(View.GONE);
        }
       } catch (NullPointerException e) {
          Log.d(TAG, "hideAll catch exception");
       }
    }

@Override
   public boolean updateSendButton(boolean enabled, int subCount, boolean requiresMms,
                                                                 boolean hasSlideshow) {

    if (mIsDualButtonSettingOn == true && mTextViewMMS != null) {

      Log.d(TAG, "UpdateSendButton ,enabled = " + enabled);
      Log.d(TAG, "UpdateSendButton ,requiresMms = " + requiresMms);

      mTextViewMMS.setVisibility(View.GONE);
       if (enabled == true && subCount > 0) {
           Log.d(TAG, "UpdateSendButton ,enabled = " + enabled);
          if (requiresMms == true) {
               Log.d(TAG, "requiresMms == true enabled");

               if (getSimState(1) == true) {
                 setMMS1Enabled();
               } else {
                 setMMS1Disabled();
               }

               if (getSimState(2) == true) {
                   setMMS2Enabled();
                } else {
                   setMMS2Disabled();
                }

           } else {
               Log.d(TAG, "requiresSMS == true enabled ");

               if (getSimState(1) == true) {
                  setSMS1Enabled();
                } else {
                  setSMS1Disabled();
                }

               if (getSimState(2) == true) {
                  setSMS2Enabled();
                } else {
                  setSMS2Disabled();
                }

           }
         //  hideSrcSendButton();
           return true;
       } else {

           if (requiresMms == true) {
               Log.d(TAG, "requiresMms == true disables");

                  setMMS1Disabled();
                  setMMS2Disabled();


           } else {
               Log.d(TAG, "requiresMms == false disabled ");
               setSMS1Disabled();
               setSMS2Disabled();
           }
       }
       return true;
    }
   else {
    hideAll();
  }

   return false;
   }



@Override
public boolean updateSendButtonState(boolean requiresMms, int recipientCount,
                                int smsRecipientLimit, int subCount) {

   if (mIsDualButtonSettingOn == true && mTextViewMMS != null) {

      mTextViewMMS.setVisibility(View.GONE);
      Log.d(TAG, "updateSendButtonState ,requiresMms = " + requiresMms);
      if (subCount > 0 && recipientCount > 0) {
           Log.d(TAG, "updateSendButtonState , =  " + mIsSecondSendButtonEnabled);
           mIsSecondSendButtonEnabled = true;
          if (requiresMms == true) {
               Log.d(TAG, "updateSendButtonState requiresMms == true enabled");

               if (getSimState(1) == true) {
                 setMMS1Enabled();
               } else {
                 setMMS1Disabled();
               }

               if (getSimState(2) == true) {
                   setMMS2Enabled();
                } else {
                   setMMS2Disabled();
                }
           } else {
               Log.d(TAG, "updateSendButtonState disabled =" + mIsSecondSendButtonEnabled);
               mIsSecondSendButtonEnabled = false;

                if (getSimState(1) == true) {
                  setSMS1Enabled();
                } else {
                  setSMS1Disabled();
                }

               if (getSimState(2) == true) {
                  setSMS2Enabled();
                } else {
                  setSMS2Disabled();
                }


           }
           return true;
       }
       else {
           if (requiresMms == true) {
               Log.d(TAG, "requiresMms true disables" + mIsSecondSendButtonEnabled);
               mIsSecondSendButtonEnabled = true;

               setMMS1Disabled();
               setMMS2Disabled();

           } else {
               Log.d(TAG, "requiresMms == false disables ");

               setSMS1Disabled();
               setSMS2Disabled();


              }
           }
           return true;
        } else {
        hideAll();
        }
       return false;
   }


@Override
public void initResourceRefsOnResume(LinearLayout buttonWithCounter, TextView textCounter,
                                 IOpAttachmentEditorExt attachmentEditor) {

    if (mTextViewMMS == null) {
         initResourceRefs(buttonWithCounter, textCounter,
                                 attachmentEditor);
    }

}


@Override
public boolean handleAttachmentEditorHandlerMessage(Message msg, boolean sendButtonCanResponse,
               IWorkingMessageCallback opWorkingMessageCallback, ISlideshowModelCallback slideshow,
               boolean compressingImage) {
    //static final int MSG_SEND_SLIDESHOW   = 2;
    if (msg.what == 2)
    sendFromEditor = true;
    return false;
}


@Override
public void initResourceRefs(LinearLayout buttonWithCounter, TextView textCounter,
        IOpAttachmentEditorExt attachmentEditor) {

      if (mIsDualButtonSettingOn == true) {
        mSendButtonSIM1 = (ImageButton) buttonWithCounter.getChildAt(2);
        mSendButtonSIM2 = (ImageButton) buttonWithCounter.getChildAt(3);

        mSendButtonMMS1 = (ImageButton) buttonWithCounter.getChildAt(2);
        mSendButtonMMS2 = (ImageButton) buttonWithCounter.getChildAt(3);

        mTextViewMMS = (TextView) buttonWithCounter.getChildAt(1);
        mTextViewMMS.setVisibility(View.GONE);

        mSendButtonSIM1.setVisibility(View.VISIBLE);
        mSendButtonSIM2.setVisibility(View.VISIBLE);


        Log.d(TAG, "initResourceRefs 1");
        mSendButtonSIM1.setImageDrawable(mImageUnSendButtonSim1);
        mSendButtonSIM1.setEnabled(false);

        Log.d(TAG, "initResourceRefs 2");
        mSendButtonSIM2.setImageDrawable(mImageUnSendButtonSim2);
        mSendButtonSIM2.setEnabled(false);
        mSendButtonSIM2.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    // Perform action on click
                         Log.d(TAG, "onClick 2nd sim mSubId[1] " + mSubId[1]);
                    bSecondSimSelected  = true;
                 //   mComposeCallback.setSubIdCallback(mSubId[1]);
                   /* if (mComposeCallback.showWfcSendButtonPopUpCallback() == true) {

                        bSecondSimSelected  = false;
                        Log.d(TAG, "onClick 2nd sim return 1");
                        return;
                    }*/


                    if (mComposeCallback.isPreparedForSendingCallback()) {
                        Log.d(TAG, "onClick 2nd b1");
                        mComposeCallback.checkRecipientsCountCallback();
                        mComposeCallback.updateSendButtonStateCallback(false);
                        mUiHandler.sendEmptyMessageDelayed(
                        MSG_RESUME_SEND_BUTTON, RESUME_BUTTON_INTERVAL);
                    }
                    else {
                        bSecondSimSelected  = false;
                        Log.d(TAG, "onClick 2nd b2");
                        mComposeCallback.unpreparedForSendingAlertCallback();
                    }
                }
             });
         }
     }

}

