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

package com.mediatek.mms.plugin;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.util.Log;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.Toast;
import com.android.browser.provider.Browser;
import com.android.internal.telephony.PhoneConstants;
import com.google.android.mms.pdu.PduBody;

import com.mediatek.mms.callback.IColumnsMapCallback;
import com.mediatek.mms.callback.IComposeActivityCallback;
import com.mediatek.mms.callback.IMessageItemCallback;
import com.mediatek.mms.callback.IMessageListAdapterCallback;
import com.mediatek.mms.callback.IRecipientsEditorCallback;
import com.mediatek.mms.callback.ISlideshowModelCallback;
import com.mediatek.mms.callback.ITextSizeAdjustHost;
import com.mediatek.mms.callback.IWorkingMessageCallback;
import com.mediatek.mms.ext.DefaultOpComposeExt;
import com.mediatek.mms.ext.IOpAttachmentEditorExt;
import com.mediatek.mms.ext.IOpWorkingMessageExt;
import com.mediatek.mms.plugin.R;
import com.mediatek.provider.MtkSettingsExt;

import java.util.ArrayList;
import java.util.List;

/**
 * Op01ComposeExt.
 *
 */
public class Op01ComposeExt extends DefaultOpComposeExt {
    private static final String TAG = "Mms/Op01MmsComposeExt";

    // TODO: Base id should be defined by Host
    public static final int MENU_SELECT_TEXT     = 101;
    private static final int MENU_ADD_ATTACHMENT        = 2;
    /* q0 migration, phase out share panel*/
    //private MenuItem mAddAttachmentMenu;
    //private Toast mExceedSubjectSizeToast                       =   null;
    /* q0 migration, phase out subject len*/
    //private static final int DEFAULT_SUBJECT_LENGTH             =   40;
    private IComposeActivityCallback mIpComposeActivityCallback = null;
    /* q0 migration, phase out text size adjust*/
//    private Op01MmsTextSizeAdjust mMmsTextSizeAdjustExt;
    private Activity mActivity;

    private Context mPluginContext;
    /* q0 migration, phase out share panel*/
    /*
    private LinearLayout mPanel;
    private ImageButton mShareButton;
    private SharePanel mSharePanel;

    private EditText mTextEditor;
    */
    /* q0 migration, phase out share panel*/
    /*
    public static final int ACTION_SHARE = 1;
    private boolean mShowKeyBoardFromShare = false;
    private boolean mIsSmsEnabled = true;
    private boolean mIsSoftKeyBoardShow = false;
    private boolean mIsLandscape = false;
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
*/
    // For API addMmsUrlToBookMark
    int mIcon;

    private int mSendSubId = -1;

    /**
     * Op01ComposeExt Construction.
     * @param context Context.
     */
    public Op01ComposeExt(Context context) {
        super(context);
        Log.d(TAG, "context is " + context.getPackageName());
        mPluginContext = context;
        //if (EncapsulationConstant.USE_MTK_PLATFORM) {
            mIcon = com.mediatek.R.drawable.ic_dialog_menu_generic;
        //} else {
        //    mIcon = android.R.color.background_light;
        //}
    }

    /**
     * This filter will constrain edits not to make the length of the text
     * greater than the specified length ( eg. 40 Bytes).
     */
    /* q0 migration, phase out subject len*/
    /*
    class MyLengthFilter implements InputFilter {
        private final int mMax;
        public MyLengthFilter(Context context, int max) {
            Log.d(TAG, "MyLengthFilter:context is " + context.getPackageName());
            mMax = max;
            }

        private CharSequence getMaxByteSequence(CharSequence str, int keep) {
            String source = str.toString();
            int byteSize = source.getBytes().length;
            if (byteSize <= keep) {
                return str;
            } else {
                int charSize = source.length();
                while (charSize > 0) {
                    source = source.substring(0, source.length() - 1);
                    charSize--;
                    if (source.getBytes().length <= keep) {
                        break;
                    }
                }
                return source;
            }
        }

        private CharSequence getMaxByteSequenceEx(CharSequence str, int keep) {
            Log.d(TAG, "getMaxByteSequenceEx");
            //Log.d(TAG, "str = " + str + ", keep = " + keep);

            SpannableStringBuilder source = new SpannableStringBuilder(str);
            int byteSize = source.toString().getBytes().length;
            //Log.d(TAG, "byteSize = " + byteSize);
            if (byteSize <= keep) {
                return str;
            } else {
                int charSize = source.length();
                //Log.d(TAG, "charSize = " + charSize);
                SpannableString[] spans = source.getSpans(0, charSize-1, SpannableString.class);
                if (spans == null || spans.length == 0) {
                    //Log.d(TAG, "no span");
                    while (charSize > 0) {
                        source.delete(charSize-1, charSize-1);
                        charSize--;
                        if (source.toString().getBytes().length <= keep) {
                            break;
                        }
                    }
                    return source.toString();
                } else {
                    //Log.d(TAG, "have span");
                    int spansLen = spans.length;
                    //Log.d(TAG, "spansLen = " + spansLen);
                    int spansStarts[] = new int[spansLen];
                    int spansEnds[] = new int[spansLen];
                    for (int i = 0; i < spansLen; i++) {
                        spansStarts[i] = source.getSpanStart(spans[i]);
                        spansEnds[i] = source.getSpanEnd(spans[i]);
                        //Log.d(TAG, "span " + i + " start=" + spansStarts[i] + " end=" + spansEnds[i]);
                    }
                    int spanIdx = spansLen - 1;
                    while (charSize > 0) {
                        //Log.d(TAG, "spanIdx = " + spanIdx);
                        //Log.d(TAG, "charSize = " + charSize);
                        int lastChar = source.length() - 1;
                        //Log.d(TAG, "lastChar = " + lastChar);
                        if (lastChar > spansEnds[spanIdx]) {
                            source.delete(charSize-1, charSize-1);
                            charSize--;
                        } else {
                            source.removeSpan(spans[spanIdx]);
                            spanIdx--;
                            charSize = source.length();
                        }
                        //Log.d(TAG, "spanIdx = " + spanIdx);
                        //Log.d(TAG, "charSize = " + charSize);
                        //Log.d(TAG, "source.getBytes().length = " + source.toString().getBytes().length);

                        if (source.toString().getBytes().length <= keep) {
                            break;
                        }
                    }
                    return source.toString();
                }
            }
        }

        //this is just the method code in LengthFilter, just add a Toast to show max length exceed.
        public CharSequence filter(CharSequence source, int start, int end,
                                   Spanned dest, int dstart, int dend) {

            int destOldLength = dest.toString().getBytes().length;
            int destReplaceLength = dest.subSequence(dstart, dend).toString().getBytes().length;
            CharSequence sourceSubString = source.subSequence(start, end);
            int sourceReplaceLength = sourceSubString.toString().getBytes().length;
            int newLength =  destOldLength - destReplaceLength + sourceReplaceLength;
            if (newLength > mMax) {
                // need cut the new input charactors
                //mExceedSubjectSizeToast.show();
                int keep = mMax - (destOldLength - destReplaceLength);
                if (keep <= 0) {
                    return "";
                } else {
                    return getMaxByteSequenceEx(sourceSubString, keep);
                }
            } else {
                return null; // can replace
            }
        }
    }

    @Override
    public void showSubjectEditor(EditText subjectEditor) {
        Context context = subjectEditor.getContext();
        subjectEditor.setFilters(new InputFilter[] {
                        new MyLengthFilter(context, DEFAULT_SUBJECT_LENGTH) });
    }
    */

    /// M: New plugin API @{
    int mMenuId = 0;
    ArrayList<String> mUrls = null;
    Context mContext;
    MenuClickListener mMenulistener = new MenuClickListener();

    @Override
    public void addCallAndContactMenuItems(Context context, ContextMenu menu, int menuId,
            ArrayList<String> urls, IMessageItemCallback msgItem) {
        Log.d(TAG, "addCallAndContactMenuItems addMmsUrlToBookMark");
        if (!(msgItem.isMmsCallback() && msgItem.isDownloadedCallback()
                && (urls != null && urls.size() > 0))) {
            return;
        }
        Log.d(TAG, "call menu.add()");
        menu.add(0, menuId, 0, getString(R.string.menu_add_to_bookmark))
            .setOnMenuItemClickListener(mMenulistener);
        mMenuId = menuId;
        mUrls = urls;
        mContext = context;
        return;
    }

    /**
     * MenuClickListener.
     *
     */
    private final class MenuClickListener implements MenuItem.OnMenuItemClickListener {

        public MenuClickListener() {
        }

        @Override
        public boolean onMenuItemClick(MenuItem item) {
            if (item.getItemId() != mMenuId) {
                return false;
            }

            if (mUrls.size() == 1) {
                Log.d(TAG, "MenuClickListener, mUrls.size() == 1, mUrls.get(0) = "
                     + mUrls.get(0));
                Browser.saveBookmark(mContext, null, mUrls.get(0));
            } else if (mUrls.size() > 1) {
                Log.d(TAG, "MenuClickListener, mUrls.size() > 1");
                CharSequence[] items = new CharSequence[mUrls.size()];
                for (int i = 0; i < mUrls.size(); i++) {
                    items[i] = mUrls.get(i);
                }
                new AlertDialog.Builder(mContext)
                    .setTitle(getString(R.string.menu_add_to_bookmark))
                    .setIcon(mIcon)
                    .setItems(items, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            Browser.saveBookmark(mContext, null, mUrls.get(which));
                            }
                        })
                    .show();
            }
            return true;
        }
    }
    /// @}

    @Override
    public boolean onOptionsItemSelected(MenuItem item, IOpWorkingMessageExt workingMessageExt,
            InputMethodManager inputMethodManager) {
        switch (item.getItemId()) {
            case MENU_ADD_ATTACHMENT:
                mIpComposeActivityCallback.showAddAttachmentDialog(true);
                Log.d(TAG, "Attach: showAddAttachmentDialog(!hasAttachedFiles)");
                return true;
            default:
                return false;
        }
    }

    @Override
    public boolean handleAttachmentEditorHandlerMessage(Message msg, boolean sendButtonCanResponse,
            IWorkingMessageCallback workingMessage, ISlideshowModelCallback slideshow,
            boolean compressingImage) {
        if (compressingImage) {
            return true;
        }

        int msgWhat = msg.what;
        switch (msgWhat) {
            case IOpAttachmentEditorExt.MSG_REMOVE_EXTERNAL_ATTACHMENT:
                if (!sendButtonCanResponse) {
                    Log.d(TAG, "handle MSG_REMOVE_EXTERNAL_ATTACHMENT return");
                    return true;
                }
                if (slideshow != null) {
                    if (slideshow.getAttachFilesCallback() != null) {
                        Log.d(TAG, "removeExternalAttachment");
                        slideshow.removeAllAttachFilesCallback();
                        workingMessage.correctAttachmentStateCallback();
                        workingMessage.onAttachmentChangedCallback();
                    }
                }
                return true;
            case IOpAttachmentEditorExt.MSG_REMOVE_SLIDES_ATTACHMENT:
                Log.d(TAG, "handle MSG_REMOVE_SLIDES_ATTACHMENT");
                if (!sendButtonCanResponse) {
                    Log.d(TAG, "handle MSG_REMOVE_SLIDES_ATTACHMENT return");
                    return true;
                }
                if (slideshow != null) {
                    super.handleAttachmentEditorHandlerMessage(null, sendButtonCanResponse, workingMessage, slideshow, compressingImage);
                    Log.d(TAG, "mSlideshow.size() = " + slideshow.sizeCallback());
                    int size = slideshow.sizeCallback();
                    for (int i = 0; i < size; i++) {
                        slideshow.removeCallback(0);
                    }
                    workingMessage.correctAttachmentStateCallback();

                    Log.d(TAG, "removeSlidesAttachment");
                    workingMessage.onAttachmentChangedCallback();
                }
                return true;
            default:
                break;
        }
        return false;
    }

    @Override
    public boolean attachVCalendar(boolean append, int type, Uri uri, int mediaTypeStringId) {
        if (!append) {
            return false;
        }
        if (!mIpComposeActivityCallback.checkSizeBeforeAppend()) {
            return true;
        }

        int result = mIpComposeActivityCallback.setAttachment(type, uri, append);
        mIpComposeActivityCallback.handleAddAttachmentError(result, mediaTypeStringId);
        return true;
    }

    @Override
    public void onCreate(IComposeActivityCallback ipComposeActivityCallback, Intent intent,
            IntentFilter intentFilter, Activity oldCompose, Activity compose,
            Bundle savedInstanceState, Handler uiHandler, ImageButton shareButton,
            LinearLayout panel, EditText textEditor) {
        mActivity = compose;
        mIpComposeActivityCallback = ipComposeActivityCallback;
        /* q0 migration, phase out text size adjust*/
        //mMmsTextSizeAdjustExt = new Op01MmsTextSizeAdjust();

        if (intent.getBooleanExtra("notification", false)) {
            int threadCount = intent.getIntExtra("thread_count", 1);
            int messageCount = intent.getIntExtra("message_count", 1);
            if (threadCount > 1 || messageCount > 1) {
 //               mIpComposeActivityCallback.setHomeBox(Op01MmsUtils.FOLDER_OPTION_INBOX);
                mIpComposeActivityCallback.goToConversationListCallback();
            }
        }
        /* q0 migration, phase out share panel*/
        /*
        Log.d(TAG, "onCreate thissop01 panel.getChildCount() = "+panel.getChildCount());
        if (panel.getChildCount() > 0) {
            mSharePanel = null;
        } else {
            mShareButton = shareButton;
            mShareButton.setVisibility(View.VISIBLE);
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
        */
    }
    /* q0 migration, phase out attach enhance*/
    /*
    @Override
    public boolean haveSomethingToCopyToSDCard(Context context, PduBody body) {
        ISlideshowModelCallback slideshowCallback = null;
        try {
            slideshowCallback =
                Op01MessagePluginExt.sMessageUtilsCallback.createFromPduBodyCallback(this, body);
        } catch (Exception e) {
            Log.e(TAG, "Create from pdubody exception!");
            return false;
        }

        if (slideshowCallback != null
                && slideshowCallback.getAttachFilesCallback() != null
                && slideshowCallback.getAttachFilesCallback().size() > 0) {
            return true;
        }
        return false;
    }
*/
    /* q0 migration, phase out attach enhance*/
    /*
    @Override
    public boolean onMenuItemClick(MenuItem item,
            Intent intentMultiSave, Intent deliveryIntent) {
        Bundle data = new Bundle();
        data.putLong(Op01AttachmentEnhance.MMS_SAVE_MODE,
                Op01AttachmentEnhance.MMS_SAVE_ALL_ATTACHMENT);
        intentMultiSave.putExtras(data);
        return false;
    }
*/
    @Override
    public boolean processNewIntent(Intent intent) {
        return true;
    }

    @Override
    public boolean subSelectionDialog(int subId) {
        if (subId == MtkSettingsExt.System.SMS_SIM_SETTING_AUTO) {
            return true;
        } else {
            return false;
        }
    }

    @Override
    public void editSlideshow(boolean hadToSlideShowEditor) {
        mIpComposeActivityCallback.setHadToSlideShowEditor(true);
    }

    @Override
    public void onResume(int subCount, String text, boolean isSmsEnabled,
            View recipientsEditor, View subjectTextEditor) {
        mIpComposeActivityCallback.setHadToSlideShowEditor(false);
        /* q0 migration, phase out share panel*/
        /*
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
                Drawable icon = getResources().getDrawable(R.drawable.ipmsg_share);
                mShareButton.setImageDrawable(icon);
            } else {
                mShareButton.setClickable(false);
                Drawable da = getResources().getDrawable(R.drawable.ipmsg_share_disable);
                mShareButton.setImageDrawable(da);
            }
        }
        */
    }

    @Override
    public boolean onNewIntent(final Intent intent, boolean hadToSlideShowEditor, Uri tempMmsUri,
            final IWorkingMessageCallback workingMessage, Object activity,
            IRecipientsEditorCallback recipientsEditor, final int box) {
        boolean ret = false;
        if (intent.getBooleanExtra("notification", false)) {
            if (hadToSlideShowEditor) {
                if (tempMmsUri != null) {
                    IWorkingMessageCallback newMessage =
                                        workingMessage.loadCallback(activity, tempMmsUri);
                    if (newMessage != null) {
                        if (newMessage.hasMediaAttachmentsCallback()) {
                            newMessage.removeAllFileAttachesCallback();
                        }
                        newMessage.setIpSubject(workingMessage.getIpSubject(), false);
                        mIpComposeActivityCallback.setWorkingMessage(newMessage);
                        mIpComposeActivityCallback.updateThreadIdIfRunningCallback();
                        mIpComposeActivityCallback.drawTopPanelCallback(
                                mIpComposeActivityCallback.isIpSubjectEditorVisible());
                        mIpComposeActivityCallback.callbackUpdateSendButtonState();
                        mIpComposeActivityCallback.invalidateIpOptionsMenu();
                    }
                }
            }
            if (workingMessage.isIpWorthSaving()
                    && mIpComposeActivityCallback.isIpRecipientEditorVisible() &&
                    !recipientsEditor.hasValidRecipientCallback(workingMessage.requiresIpMms())) {
                ret = true;
                Op01MessagePluginExt.sMessageUtilsCallback
                        .showDiscardDraftConfirmDialogCallback(mActivity, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        workingMessage.discardCallback();
                        int threadCount = intent.getIntExtra("thread_count", 1);
                        int messageCount = intent.getIntExtra("message_count", 1);
                        if (threadCount > 1 || messageCount > 1) {
          //                  mIpComposeActivityCallback.setHomeBox(box);
                            mIpComposeActivityCallback.goToConversationListCallback();
                        } else {
                            mIpComposeActivityCallback.processNewIntentCallback(intent);
                            mIpComposeActivityCallback.updateIpTitle();
                        }
                    }
                });
            }
        }

        Log.d(TAG, "handleNotificationIntent, ret = " + ret);
        return ret;
    }
    /* q0 migration, phase out forward sms with info*/
    /*
    @Override
    public boolean forwardMessage(Context context, IMessageItemCallback msgItem,
            String nameAndNumber, Intent intent) {
        if (msgItem.getType().equals("sms")) {
            String smsBody = new Op01MmsPreference(getBaseContext()).formatSmsBody(context,
                    msgItem.getBody(), nameAndNumber, msgItem.getBoxIdCallback());
            intent.putExtra("sms_body", smsBody);
        }
        return true;
    }
*/
    /* q0 migration, phase out text size adjust*/
    /*
    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        return mMmsTextSizeAdjustExt.dispatchTouchEvent(ev);
    }

    @Override
    public void onStart(ITextSizeAdjustHost host, Activity activity) {
        mMmsTextSizeAdjustExt.init(host, activity);
        mMmsTextSizeAdjustExt.refresh();
    }
*/
    /* q0 migration, phase out auto sim*/
    /*
    @Override
    public String checkRecipientsCount(String message, boolean requiresMms,
            int recipientCount, int mmsLimitCount, boolean isRecipientsEditorEmpty,
            boolean isConversationRecipientEmpty, Intent intent,
            IMessageListAdapterCallback msgListAdapter, List<SubscriptionInfo> subInfoList,
            boolean isRecipientsEditorVisible) {
        if (!(requiresMms && (recipientCount > mmsLimitCount))) {
            if (!isRecipientsEditorEmpty && !isConversationRecipientEmpty) {
                mSendSubId = getAutoSelectSubId(intent, msgListAdapter, subInfoList);
            }
        }
        return message;
    }
*/
    /* q0 migration, phase out share panel*/
    /*
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
        return true;
    }
    */
    /* q0 migration, phase out auto sim*/
    /*
    private int getAutoSelectSubId(Intent intent, IMessageListAdapterCallback msgListAdapter,
                                        List<SubscriptionInfo> subInfoList) {
        boolean isValid = false;
        int subId = -1;
        int subCount = subInfoList.size();
        int subIdinSetting = SubscriptionManager.getDefaultSmsSubscriptionId();
        if (subIdinSetting == (int) MtkSettingsExt.System.SMS_SIM_SETTING_AUTO) {
            subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                    SubscriptionManager.INVALID_SUBSCRIPTION_ID);
            if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                if (msgListAdapter != null) {
                    Cursor c = msgListAdapter.getCursorCallback();
                    if (c != null && c.moveToLast()) {
                        int count = c.getCount();
                        Log.d(TAG, "count = " + count);
                        subId = getSubIdFromCursor(c, msgListAdapter.getColumnsMap());
                    }
                }
            }
            int enabledSubCount = Op01MmsUtils.getEnabledSubCount(mPluginContext);
            Log.d(TAG, "getAutoSelectSubId: enabledSubCount = " + enabledSubCount);
            if (enabledSubCount == 1) {
                for (int i = 0; i < subCount; i++) {
                    int thisSub = subInfoList.get(i).getSubscriptionId();
                    Log.d(TAG, "getAutoSelectSubId: i = " + i + ", thisSub = " + thisSub);
                    if (Op01MmsUtils.isSubEnabled(mPluginContext, thisSub)) {
                        Log.d(TAG, "getAutoSelectSubId: i = " + i + "is enabled");
                        isValid = true;
                        subId = thisSub;
                        break;
                    }
                }
            } else {
                for (int i = 0; i < subCount; i++) {
                    if (subInfoList.get(i).getSubscriptionId() == subId) {
                        isValid = true;
                        break;
                    }
                }
            }
            if (!isValid) {
                subId = -1;
            }
            Log.d(TAG, "getAutoSelectSubId: subId = " + subId);
        }
        return subId;
    }

    @Override
    public int subSelection() {
        int subId = 0;
        if (mSendSubId >= 1) {
            subId = mSendSubId;
            mSendSubId = -1;
            Log.d(TAG, "send msg from mSendSubIdForDualBtn = " + subId);
            mIpComposeActivityCallback.setSubIdCallback(subId);
            mIpComposeActivityCallback.confirmSendMessageIfNeededCallback();
        }
        return subId;
    }
*/
    /**
     * getSubIdFromCursor.
     * @param c Cursor
     * @param columnsMap IColumnsMapCallback
     * @return subid
     */
    /* q0 migration, phase out auto sim*/
    /*
    public int getSubIdFromCursor(Cursor c, IColumnsMapCallback columnsMap) {
        int simId = -1;
        try {
            String type = c.getString(columnsMap.getColumnMsgType());
            if (type.equals("mms")) {
                simId = c.getInt(columnsMap.getColumnMmsSubId());
            } else if (type.equals("sms")) {
                simId = c.getInt(columnsMap.getColumnSmsSubId());
            }
        } catch (Exception e) {
            Log.e(TAG, "getSimId error happens, please check!");
        } finally {
            Log.d(TAG, "getSimId id = " + simId);
            return simId;
        }
    }
*/
    /* q0 migration, phase out share panel*/
    /*
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
    */
    /* q0 migration, phase out share panel*/
    /*
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
                        if (mActivity.isInMultiWindowMode()) {
                            mIpComposeActivityCallback.hideIpInputMethod();
                            mIpComposeActivityCallback.showAddAttachmentDialog(true);
                            return;
                        }
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
                Drawable disableIcon = getResources().getDrawable(R.drawable.ipmsg_share_disable);
                mShareButton.setImageDrawable(disableIcon);
                return;
            }
            if (isShow) {
                mSharePanel.setVisibility(View.VISIBLE);
                Drawable keyboardIcon = getResources().getDrawable(R.drawable.ipmsg_keyboard);
                mShareButton.setImageDrawable(keyboardIcon);
            } else {
                mSharePanel.setVisibility(View.GONE);
                Drawable shareIcon = getResources().getDrawable(R.drawable.ipmsg_share);
                mShareButton.setImageDrawable(shareIcon);
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
            mIpComposeActivityCallback.showKeyBoardCallback(isShowKeyboard);
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
                                    mIpComposeActivityCallback.setTextEditorMaxHeight(
                                            mReferencedTextEditorTwoLinesHeight);
                                } else {
                                    mIpComposeActivityCallback.setTextEditorMaxHeight(
                                            mReferencedTextEditorFourLinesHeight);
                                }
                            }
                        }
                    });
                }
            }).start();
        } else {
            if (isShowShare && !isShowKeyboard && mIsLandscape) {
                mIpComposeActivityCallback.showKeyBoardCallback(isShowKeyboard);
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
                                mIpComposeActivityCallback.setTextEditorMaxHeight(
                                        mReferencedTextEditorTwoLinesHeight);
                            }
                        });
                    }
                }).start();
            } else {
                mIpComposeActivityCallback.showKeyBoardCallback(isShowKeyboard);
                showSharePanel(isShowShare);
                if (isShowShare || isShowKeyboard) {
                    if (mIsLandscape) {
                        mIpComposeActivityCallback.setTextEditorMaxHeight(
                                mReferencedTextEditorTwoLinesHeight);
                    } else {
                        mIpComposeActivityCallback.setTextEditorMaxHeight(
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

        mIpComposeActivityCallback.addIpAttachment(commonAttachmentType, true);
        Log.d(TAG, "attach: addAttachment(commonAttachmentType, true)");
        /// @}

        hideSharePanel();
    }
    */
    /// sharepanel end
    /* q0 migration, phase out share panel*/
    /*
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
    */
    /* q0 migration, phase out share panel*/
    /*
    private boolean isViewVisible(View view) {
        if (mSharePanel == null) {
            return false;
        }
        return (null != view)
                    && (View.VISIBLE == view.getVisibility());
    }
    */
}

