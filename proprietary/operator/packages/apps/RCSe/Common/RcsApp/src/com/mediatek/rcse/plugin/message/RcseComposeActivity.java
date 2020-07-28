package com.mediatek.rcse.plugin.message;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnClickListener;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.media.MediaMetadataRetriever;
import android.media.RingtoneManager;
import android.media.ThumbnailUtils;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.NetworkInfo.State;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.StatFs;
import android.os.SystemClock;
import android.provider.MediaStore;
import android.provider.Settings;
import android.provider.Telephony;
import android.provider.ContactsContract.Contacts;
import android.provider.MediaStore.Audio;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Video;
import android.provider.MediaStore.Video.Thumbnails;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Threads;
import android.telephony.SmsManager;
import android.text.Editable;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.TextKeyListener;
import android.util.Log;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.inputmethod.InputMethodManager;
import android.webkit.MimeTypeMap;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.os.SystemProperties;

import com.mediatek.mms.ipmessage.IIpMessageItemExt;
import com.mediatek.mms.ipmessage.IIpConversationExt;
import com.mediatek.mms.ipmessage.DefaultIpComposeActivityExt;
import com.mediatek.mms.callback.IComposeActivityCallback;
import com.mediatek.mms.callback.IWorkingMessageCallback;
import com.mediatek.mms.ipmessage.IIpMessageItemExt;
import com.mediatek.mms.ipmessage.IIpMessageListAdapterExt;
import com.mediatek.rcse.activities.widgets.AttachmentTypeSelectorAdapter;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IpMessageConsts.*;
//import com.mediatek.rcse.provider.TempFileProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.AppSettings;
import com.mediatek.rcse.settings.RcsSettings;
import com.mediatek.storage.StorageManagerEx;
import com.mediatek.rcs.R;

public class RcseComposeActivity extends DefaultIpComposeActivityExt
implements INotificationsListener {

    private static String TAG = "RcseComposeActivity";

    private static boolean DEBUG = false;

    // / M: add for ip message, context menu
    private static final int MENU_RETRY = 200;
    private static final int MENU_DELETE_IP_MESSAGE = 201;
    private static final int MENU_SHARE = 202;
    private static final int MENU_SEND_SPAM = 235;
    private static final int MENU_SAVE_ATTACHMENT = 205;
    private static final int MENU_VIEW_IP_MESSAGE = 206;
    private static final int MENU_SEND_VIA_TEXT_MSG = 208;
    private static final int MENU_EXPORT_SD_CARD = 209;
    private static final int MENU_FORWARD_IPMESSAGE = 21;
    private static final int MENU_LOCK_MESSAGE          = 28;
    // / M: add for ip message, option menu
    private static final int MENU_INVITE_FRIENDS_TO_CHAT = 100;
    private static final int MENU_ADD_ATTACHMENT        = 2;
    private static final int MENU_MARK_AS_SPAM = 102;
    private static final int MENU_REMOVE_SPAM = 251;
    private static final int MENU_VIEW_ALL_MEDIA = 105;
    private static final int MENU_INVITE_FRIENDS_TO_IPMSG = 107;
    private static final int MENU_SEND_BY_SMS_IN_JOYN_MODE = 108;
    private static final int MENU_SEND_BY_JOYN_IN_SMS_MODE = 109;
    public static final Uri TEMP_VIDEO_URI =
            Uri.parse("content://mms_temp_file/scrapSpaceVideo");
    public static final Uri TEMP_CONTENT_URI =
            Uri.parse("content://mms_temp_file/scrapSpace");

    // / M: Ip message add jump to another chat menu. @{
    private static final int MENU_JUMP_TO_JOYN = 221;
    private static final int MENU_JUMP_TO_XMS = 222;
    // / @}
    private long mRequestTimeMillis;
    private static final long AUTOMATED_RESULT_THRESHOLD_MILLLIS = 250;

    // / M: Ip message add export chat menu. @{
    private static final int MENU_EXPORT_CHAT = 223;
    // / @}

    public static final int MSG_LIST_RESEND_IPMSG = 20;
    public static final int OK = 0;
    public static final int UPDATE_SENDBUTTON = 2;
    private static final int MENU_COPY = 207;

    private boolean mIsIpServiceEnabled = false;

    private ImageButton mSendButtonIpMessage; // press to send ipmessage
    private TextView mTypingStatus;
    private TextView mRemoteStrangerText;
    private boolean mIsDestroyTypingThread = false;
    private long mLastSendTypingStatusTimestamp = 0L;

    // /M: whether or not show invite friends to use ipmessage interface.
    private boolean mShowInviteMsg = false;

    // /M: working IP message
    private int mIpMessageDraftId = 0;
    private IpMessage mIpMessageDraft;
    private IpMessage mIpMessageForSend;
    private boolean mIsClearTextDraft = false;
    private AlertDialog mReplaceDialog = null;
    public static int mCurrentChatMode = IpMessageConsts.ChatMode.XMS;

    // chat mode number
    private String mChatModeNumber = "";
    private boolean mIsSmsEnabled;

    // ipmessage status
    private boolean mIsIpMessageRecipients = false;

    private String mChatSenderName = "";

    private static final int REQUEST_CODE_IPMSG_TAKE_PHOTO = 200;
    private static final int REQUEST_CODE_IPMSG_RECORD_VIDEO = 201;
    private static final int REQUEST_CODE_IPMSG_SHARE_CONTACT = 203;
    private static final int REQUEST_CODE_IPMSG_CHOOSE_PHOTO = 204;
    private static final int REQUEST_CODE_IPMSG_CHOOSE_VIDEO = 205;
    private static final int REQUEST_CODE_IPMSG_RECORD_AUDIO = 206;
    private static final int REQUEST_CODE_IPMSG_CHOOSE_AUDIO = 208;
    private static final int REQUEST_CODE_IPMSG_SHARE_VCALENDAR = 209;

    // / M: IP message
    public static final int IPMSG_TAKE_PHOTO = 100;
    public static final int IPMSG_RECORD_VIDEO = 101;
    public static final int IPMSG_RECORD_AUDIO = 102;
    public static final int IPMSG_CHOOSE_PHOTO = 104;
    public static final int IPMSG_CHOOSE_VIDEO = 105;
    public static final int IPMSG_CHOOSE_AUDIO = 106;
    public static final int IPMSG_SHARE_CONTACT = 108;
    public static final int IPMSG_SHARE_CALENDAR = 109;
    public static final int IPMSG_SHARE_SLIDESHOW = 110;
    public static final int IPMSG_SHARE_FILE = 111;

    private static final int PERMISSION_REQUEST_CODE_IPMSG_TAKE_PHOTO = 600;
    private static final int PERMISSION_REQUEST_CODE_IPMSG_RECORD_VIDEO = 601;
    private static final int PERMISSION_REQUEST_CODE_IPMSG_SHARE_CONTACT = 603;
    private static final int PERMISSION_REQUEST_CODE_IPMSG_CHOOSE_PHOTO = 604;
    private static final int PERMISSION_REQUEST_CODE_IPMSG_CHOOSE_VIDEO = 605;
    private static final int PERMISSION_REQUEST_CODE_IPMSG_RECORD_AUDIO = 606;
    private static final int PERMISSION_REQUEST_CODE_IPMSG_CHOOSE_AUDIO = 608;
    private static final int PERMISSION_REQUEST_CODE_IPMSG_SHARE_VCALENDAR = 609;
    private static final int PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE = 610;
    private static final int PERMISSION_REQUEST_CODE_IPMSG_RECEIVE_FILE = 611;

    public static final int REQUEST_CODE_IPMSG_PICK_CONTACT = 210;
    public static final int REQUEST_CODE_INVITE_FRIENDS_TO_CHAT = 211;

    public static final int REQUEST_CODE_IPMSG_SHARE_FILE = 214;
    public static final int REQUEST_CODE_IP_MSG_PICK_CONTACTS = 215;

    private static final int SMS_CONVERT = 0;
    private static final int MMS_CONVERT = 1;
    private static final int SERVICE_IS_NOT_ENABLED = 0;
    private static final int RECIPIENTS_ARE_NOT_IP_MESSAGE_USER = 1;
    // M: add logic for the current ipmessage user can not send ip message
    private static final int RECIPIENTS_IP_MESSAGE_NOT_SENDABLE = 2;

    // /M: for forward ipMsg
    public static final String FORWARD_IPMESSAGE = "forwarded_ip_message";
    public static final String IP_MESSAGE_ID = "ip_msg_id";
    public static final String CHOICE_FILEMANAGER_ACTION = "com.mediatek.filemanager.ADD_FILE";
    public static final String FILE_SCHEMA = "file://";
    public static final String SMS_BODY = "sms_body";

    private static final int RECIPIENTS_LIMIT_FOR_SMS = 100;;

    // /M: for check whether or not can convert IpMessage to Common message.
    // /M: can not support translating media through common message , -1;
    private static final int MESSAGETYPE_UNSUPPORT = -1;
    private static final int MESSAGETYPE_COMMON = 0;
    private static final int MESSAGETYPE_TEXT = 1;
    private static final int MESSAGETYPE_MMS = 2;
    private static final int MENU_SELECT_TEXT = 36;

    public static final int ACTION_SHARE = 0;
    public static final String SHARE_ACTION = "shareAction";
    public static final String FORWARD_MESSAGE = "forwarded_message";
    public static final int RESULT_OK = -1;

    private String mIpMessageVcardName = "";
    private ImageView mFullIntegratedView;
    private String mPhotoFilePath = "";
    private String mDstPath = "";
    private int mDuration = 0;
    private String mCalendarSummary = "";
    private EditText mTextEditor;
    private TextView mTextCounter;
    private Handler mUiHandler;

    private Activity mContext;
    private IComposeActivityCallback mCallback;
    private IWorkingMessageCallback mWorkingMessage;
    private View mBottomPanel;
    Resources mResource = null;

    private static final int ACTION_RCS_SHARE = 1;
    private boolean mShowKeyBoardFromShare = false;
    private boolean mIsSoftKeyBoardShow = false;
    private boolean mIsLandscape = false;
    private Object mWaitingImeChangedObject = new Object();

    private static final int mReferencedTextEditorTwoLinesHeight = 65;
    private static final int mReferencedTextEditorThreeLinesHeight = 110;
    private static final int mReferencedTextEditorFourLinesHeight = 140;
    private static final int mReferencedTextEditorSevenLinesHeight = 224;

    private static final int ADD_IMAGE = 0;
    private static final int TAKE_PICTURE = 1;
    private static final int ADD_VIDEO = 2;
    private static final int RECORD_VIDEO = 3;
    private static final int ADD_SOUND = 4;
    private static final int RECORD_SOUND = 5;
    private static final int ADD_SLIDESHOW = 6;
    private static final int ADD_VCARD = 7;
    private static final int ADD_VCALENDAR = 8;

    protected static final int MY_PERMISSIONS_REQUEST_STORAGE = 101;
    private ImageButton mShareButton;
    private SharePanel mSharePanel;

    public static Context mPluginContext;
    private RcseConversation mRcseConversation = null;

    public RcseComposeActivity(Context pluginContext) {
        mPluginContext = pluginContext;
    }

    @Override
    public boolean onIpComposeActivityCreate(Activity context, IComposeActivityCallback callback,
            Handler handler,Handler uiHandler,
            ImageButton sendButton, TextView typingTextView, TextView strangerTextView,
            View bottomPanel, Bundle bundle,
            ImageButton shareButton, LinearLayout panel,  EditText textEditor) {
        Logger.v(TAG, "onIpComposeActivityCreate enter. sendButton = " + sendButton);
        try {
            mResource = MediatekFactory.getApplicationContext()
                    .getPackageManager()
                    .getResourcesForApplication(CoreApplication.APP_NAME);
        } catch (NameNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

     // Init MMS Service before sendign MMs
        SmsManager.getDefault().getAutoPersisting();
        synchronized (this) {
            try {
                Logger.v(TAG, "onIpComposeActivityCreate  wait for autopersisting");
                this.wait(2000);
            } catch (InterruptedException e) {
                Logger.v(TAG, "InterruptedException");
            }
        }
        mContext = context;
        mCallback = callback;
        mUiHandler = handler;
        mSendButtonIpMessage = sendButton;
        mTypingStatus = typingTextView;
        mRemoteStrangerText = strangerTextView;
        mBottomPanel = bottomPanel;
        mShareButton = shareButton;
        mSharePanel = new SharePanel(mPluginContext, this);
        panel.addView(mSharePanel);
        /// sharepanel
        initShareResource();
        // / M: add for ipmessage
        if (IpMmsConfig.isServiceEnabled(mContext)) {
            boolean isServiceReady = IpMessageServiceMananger.getInstance(mContext)
                    .serviceIsReady();
            Logger.v(TAG, "onCreate(): is ip service ready ?= " + isServiceReady);
            if (!isServiceReady) {
                Logger.v(TAG, "Turn on ipmessage service by Composer.");
                IpMessageServiceMananger.getInstance(mContext).startIpService();
            }
        }
        mIsIpServiceEnabled = IpMmsConfig.isServiceEnabled(mContext);
        Logger.v(TAG, "mIsIpServiceEnabled =" + mIsIpServiceEnabled);
        if(hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)) {
            Logger.v(TAG, "onIpComposeActivityCreate  hasPermission File True");
        } else {
            Logger.d(TAG, "onIpComposeActivityCreate  hasPermission File False");
            /*mContext.requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE },
                    PERMISSION_REQUEST_CODE_IPMSG_RECEIVE_FILE);*/
        }

        return false;
    }

    public boolean showIpMessageDetails(IIpMessageItemExt msgItem) {
        Logger.v(TAG, "showIpMessageDetails enter.  msgItem= " + msgItem);
        if (msgItem == null) {
            return false;
        }
        RcseMessageItem rcsMsgItem = (RcseMessageItem) msgItem;
        if (rcsMsgItem.mIpMessageId != 0) {
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setTitle("Message details")
            .setMessage(RcseMessageListItem.getIpTextMessageDetails(mContext, rcsMsgItem))
            .setCancelable(true)
            .show();
            return true;
        }
        return false;
    }

    public boolean onIpComposeActivityResume(boolean isSmsEnabled,
            EditText textEditor,  TextWatcher watcher, TextView textCounter,
            View recipientsEditor, View subjectTextEditor) {
        Logger.v(TAG, "onIpComposeActivityResume enter. isSmsEnabled = " + isSmsEnabled);
        String[] numbers = mCallback.getConversationInfo();
        if (numbers.length != 0) {
            mChatModeNumber = numbers[0];
        }
        mIsSmsEnabled = isSmsEnabled;
        mTextEditor = textEditor;
        mTextCounter = textCounter;
        if (mIpMessageDraft != null) {
            Logger.v(TAG, "show IpMsg saveIpMessageForAWhile");
            saveIpMessageForAWhile(mIpMessageDraft);
            mCallback.callbackUpdateButtonState(true);
        }

        if ((isViewVisible(recipientsEditor) && recipientsEditor.hasFocus())
                || (isViewVisible(subjectTextEditor) && subjectTextEditor.hasFocus())
                || (isViewVisible(mTextEditor) && mTextEditor.hasFocus())) {
            showSharePanel(false);
        }
        // / M: add for ip message, notification listener
        IpMessageUtils.addIpMsgNotificationListeners(mContext, this);
        mIsIpServiceEnabled = IpMmsConfig.isServiceEnabled(mContext);

        // / M: add for ip message, Joyn chat under converaged inbox will set to
        // enable
        if (mCurrentChatMode == IpMessageConsts.ChatMode.JOYN) {
            mShareButton.setVisibility(View.VISIBLE);
            enableShareButton(true);
            mIsIpServiceEnabled = true;
        }
        IpMessageChatManger.getInstance(mContext).enterChatMode(mChatModeNumber);

        if (!TextUtils.isEmpty(mTextEditor.getText().toString())) {
            //mIpMsgHandler.post(mSendTypingRunnable);
        }

        TextView remoteStrangerText = null;
        boolean isStranger = IpMessageContactManager.getInstance(mContext)
                .isStrangerContact(mChatModeNumber);
        if (isStranger) {
            if (remoteStrangerText != null) {
                String reminderString = IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.ipmsg_joyn_stranger_remind);
                remoteStrangerText.setText(reminderString);
                remoteStrangerText.setVisibility(View.VISIBLE);
            } else {
                Logger.v(TAG, "remoteStrangerText is null!");
            }
        }
        return true;
    }

    @Override
    public boolean onIpComposeActivityPause() {
        Logger.v(TAG, "onIpComposeActivityPause enter. mIsIpServiceEnabled = "
                + mIsIpServiceEnabled);
        // / M: add for ip message, notification listener
        IpMessageUtils.removeIpMsgNotificationListeners(mContext, this);
        if (!TextUtils.isEmpty(mChatModeNumber)) {
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "onPause(): exit Chat Mode. number = "
                    + mChatModeNumber);
            IpMessageChatManger.getInstance(mContext).exitFromChatMode(mChatModeNumber);
        }

        if (mIsIpServiceEnabled) {
            mIpMsgHandler.post(mSendStopTypingRunnable);
        }

        // / M: add for ipmessage
        mIsDestroyTypingThread = true;
        synchronized (mShowTypingLockObject) {
            mShowTypingLockObject.notifyAll();
        }
        return true;
    }

    @Override
    public boolean onIpAfterTextChanged(Editable s, String beforeTextChangeString) {
        Logger.v(TAG, "onIpAfterTextChanged beforeTextChangeString = " + beforeTextChangeString);
        if (mIsIpServiceEnabled && !TextUtils.isEmpty(mChatModeNumber)) {
            if (s.length() > 0) {
                Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "send chat mode, typing");
                IpMessageChatManger.getInstance(mContext).sendChatMode(
                        mChatModeNumber, ContactStatus.TYPING);
            } else {
                Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "send chat mode, stop typing");
                IpMessageChatManger.getInstance(mContext).sendChatMode(
                        mChatModeNumber, ContactStatus.STOP_TYPING);
            }
        }
        // / M: remove "typing" feature for a while
        if (mIsIpServiceEnabled && !TextUtils.isEmpty(mChatModeNumber)) {/*
            if (TextUtils.isEmpty(beforeTextChangeString) && s.length() > 0) {
                mIpMsgHandler.post(mSendTypingRunnable);
                mLastSendTypingStatusTimestamp = System.currentTimeMillis();
            } else {
                long currentTimeStamp = System.currentTimeMillis();
                if ((currentTimeStamp - mLastSendTypingStatusTimestamp) > 3000) {
                    mIpMsgHandler.post(mSendTypingRunnable);
                    mLastSendTypingStatusTimestamp = currentTimeStamp;
                }
            }

            mIpMsgHandler.removeCallbacks(mSendStopTypingRunnable);
            if (s.length() == 0) {
                mIpMsgHandler.postDelayed(mSendStopTypingRunnable, 3000);
            } else {
                mIpMsgHandler.postDelayed(mSendStopTypingRunnable, 20000);
            }
        */}
        return true;
    }

    @Override
    public boolean onIpTextEditorKey(View v, int keyCode, KeyEvent event) {
        Logger.v(TAG, "onIpTextEditorKey keyCode =" + keyCode + " event =" + event);
        if (keyCode == KeyEvent.KEYCODE_DEL && event.getAction() == KeyEvent.ACTION_DOWN) {
            String s = ((EditText) v).getText().toString();
            int index = ((EditText) v).getSelectionStart();
            Logger.v(TAG, "mTextEditor.onKey(): is text empty ?=" + TextUtils.isEmpty(s)
                + ", index = " + index);
            if (TextUtils.isEmpty(s) || index == 0) {
                clearIpMessageDraft();
            }
        }
        return true;
    }

    @Override
    public boolean onIpMessageListItemHandler(int msg, long currentMsgId, long threadId, long subId) {
        Logger.v(TAG, "onIpMessageListItemHandler msg =" + msg + " currentMsgId =" + currentMsgId);
        if (msg == MSG_LIST_RESEND_IPMSG) {
            long[][] allFailedIpMsgIds = getAllFailedIpMsgByThreadId(threadId);
            Logger.v(TAG, "mMessageListItemHandler.handleMessage(): Msg_list_reand_ipmsg, "
                    + "allFailedIpMsg len:" + allFailedIpMsgIds.length);
            showResendConfirmDialg(currentMsgId, subId, allFailedIpMsgIds);
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpUpdateCounter(CharSequence text, int start, int before, int count) {
        Logger.v(TAG, "onIpUpdateCounter text =" + text + " count =" + count);
        if (updateIpMessageCounter(text, start, before, count)) {
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpDeleteMessageListenerClick(IIpMessageItemExt ipMsgItem) {
        Logger.v(TAG, "onIpDeleteMessageListenerClick ipMessageId =" + ipMsgItem);
        if (ipMsgItem == null) {
            return false;
        }
        RcseMessageItem rcseMsgItem = (RcseMessageItem) ipMsgItem;
        long ipMessageId = rcseMsgItem.mIpMessageId;
        if (ipMessageId > 0) { // / M: add for ipmessage
            // / M: delete ipmessage recorded in external db.
            long[] ids = new long[1];
            ids[0] = rcseMsgItem.mMsgId;
            Logger.v(TAG, "delete ipmessage, id:" + ids[0]);
            IpMessageManager.getInstance(mContext).deleteIpMsg(ids, false);
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpDiscardDraftListenerClick() {
        Logger.v(TAG, "onIpDiscardDraftListenerClick mIpMessageDraft =" + mIpMessageDraft);
        /// M: clear IP message draft.@{
        if (mIpMessageDraft != null) {
            if (mIpMessageDraft.getId() > 0) {
                IpMessageManager.getInstance(mContext).deleteIpMsg(
                        new long[] {
                            mIpMessageDraft.getId()
                        }, true);
            }
            mIpMessageDraft = null;
        }
        // / @}
        return true;
    }

    @Override
    public boolean onIpCreateContextMenu(ContextMenu menu, boolean isSmsEnabled,
            boolean isForwardEnabled, IIpMessageItemExt ipMsgItem) {
        Logger.v(TAG, "onIpCreateContextMenu ipMsgItem =" + ipMsgItem);
        if (ipMsgItem == null) {
            return false;
        }
        RcseMessageItem rcseMsgItem = (RcseMessageItem) ipMsgItem;
        long ipMessageId = rcseMsgItem.mIpMessageId;
        long msgId = rcseMsgItem.mMsgId;

        IpMsgListMenuClickListener l = new IpMsgListMenuClickListener(rcseMsgItem);
        if (isSmsEnabled) {
            if ((IpMmsConfig.isActivated(mContext) || IpMmsConfig.isServiceEnabled(mContext))
                    && ipMessageId > 0) {
                int ipStatus = IpMessageManager.getInstance(mContext).getStatus(msgId);
                Logger.v(TAG, "onIpCreateContextMenu ipStatus =" + ipStatus);
                if (ipStatus == IpMessageStatus.FAILED || ipStatus == IpMessageStatus.NOT_DELIVERED || ipStatus == IpMessageStatus.MO_TIMEOUT) {
                    IpMessage ipMessage = IpMessageManager.getInstance(mContext)
                            .getIpMsgInfo(msgId);
                    int commonType = canConvertIpMessageToMessage(ipMessage);
                    if(IpMessageServiceMananger.getInstance(mContext).getIntegrationMode() == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED) {
                    if (commonType == MESSAGETYPE_TEXT) {
                        menu.add(0, MENU_SEND_VIA_TEXT_MSG, 0,
                                IpMessageResourceMananger.getInstance(mContext)
                                                .getSingleString(IpMessageConsts.string.ipmsg_send_via_text_msg))
                                .setOnMenuItemClickListener(l);
                    } else if (commonType == MESSAGETYPE_MMS) {
                        menu.add(0, MENU_SEND_VIA_TEXT_MSG, 0,
                                IpMessageResourceMananger.getInstance(mContext)
                                                .getSingleString(IpMessageConsts.string.ipmsg_send_via_mms))
                                .setOnMenuItemClickListener(l);
                    }
                    }
                    menu.add(0, MENU_RETRY, 0,
                            IpMessageResourceMananger.getInstance(mContext)
                                                .getSingleString(IpMessageConsts.string.ipmsg_retry))
                                .setOnMenuItemClickListener(l);
                }
            }
        }
        if (isSmsEnabled) {
            if (ipMessageId > 0) {
                IpMessage ipMessage = IpMessageManager.getInstance(mContext).getIpMsgInfo(msgId);
                if (ipMessage != null && ipMessage.getType() >= IpMessageType.PICTURE
                        && ipMessage.getType() < IpMessageType.UNKNOWN_FILE){
                    menu.removeItem(MENU_COPY);
                }
                if (ipMessage != null && (ipMessage.getType() == IpMessageType.TEXT)) {
                   // menu.add(0, MENU_COPY, 0, IpMessageResourceMananger.getInstance(mContext)
                           // .getSingleString(IpMessageConsts.string.ipmsg_copy))
                           // .setOnMenuItemClickListener(l);
                }
            }
        }
        // Forward is not available for undownloaded messages.
        if (isSmsEnabled) {
            if (isForwardEnabled) {
                if (ipMessageId > 0) {
                    if (rcseMsgItem.mIpMessage instanceof IpAttachMessage) {
                        IpAttachMessage ipAttachMessage = (IpAttachMessage) rcseMsgItem.mIpMessage;
                        if (!ipAttachMessage.isInboxMsgDownloalable()) {
                            //menu.add(0, MENU_FORWARD_IPMESSAGE, 0, R.string.menu_forward).setOnMenuItemClickListener(l);
                        }
                    } else {
                        //menu.add(0, MENU_FORWARD_IPMESSAGE, 0, R.string.menu_forward).setOnMenuItemClickListener(l);
                    }
                }
            }
        }

        boolean spamReport= false;
        if(RcsSettings.getInstance().isSupportOP08()) {
            spamReport = true;
        }
        Logger.v(TAG, "spam enabled is" + spamReport);
        /// M: Fix ipmessage bug @{
        if (isSmsEnabled && ipMessageId > 0 && spamReport) {
        /// @}
            if (rcseMsgItem.mIpMessage instanceof IpAttachMessage) {
                IpAttachMessage ipAttachMessage = (IpAttachMessage) rcseMsgItem.mIpMessage;
                if (ipAttachMessage.isInboxMsgDownloalable()) {
                   menu.add(0, MENU_SEND_SPAM, 0,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.mark_as_spam))
                    .setOnMenuItemClickListener(l);
                }
            } else {
                if(rcseMsgItem.mIpMessage.getFrom()!=null &&  !rcseMsgItem.mIpMessage.getFrom().equals("") && mRcseConversation!=null) {
                Logger.v(TAG, "menu send spam from field" + rcseMsgItem.mIpMessage.getFrom());
                if (((RcseConversation)mRcseConversation).isSpam()) {
                    Logger.v(TAG, "menu send spam already, so remove");
                    menu.add(0, MENU_REMOVE_SPAM, 0,
                            IpMessageResourceMananger.getInstance(mContext)
                                        .getSingleString(IpMessageConsts.string.remove_from_spam))
                        .setOnMenuItemClickListener(l);
                } else {
                menu.add(0, MENU_SEND_SPAM, 0,
                        IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.mark_as_spam))
                    .setOnMenuItemClickListener(l);
            }
            }
        }
        }

        /// M: add for ipmessage, only text ipmessage can show select text
        if (ipMessageId > 0 && rcseMsgItem.mIpMessage != null) {
            if (rcseMsgItem.mIpMessage.getType() != IpMessageType.TEXT) {
                menu.removeItem(MENU_SELECT_TEXT);
            }
        }

        if (ipMessageId > 0 && rcseMsgItem.mIpMessage != null) {
                menu.removeItem(MENU_FORWARD_IPMESSAGE);
                menu.removeItem(MENU_LOCK_MESSAGE);

            }
        /// @}
        return true;
    }

    private final class IpMsgListMenuClickListener implements MenuItem.OnMenuItemClickListener {
        private long mIpMessageId;
        private long mMsgId;
        private RcseMessageItem mIpMsgItem;
        public IpMsgListMenuClickListener(RcseMessageItem ipMsgItem) {
            mIpMsgItem = ipMsgItem;
        }

        @Override
        public boolean onMenuItemClick(MenuItem item) {
            return onIpMenuItemClick(item, mIpMsgItem);
        }
    }

    @Override
    public boolean onIpMenuItemClick(MenuItem item, IIpMessageItemExt ipMsgItem) {
        Logger.v(TAG, "onIpMenuItemClick ipMsgItem =" + ipMsgItem);

        if (ipMsgItem != null && ((RcseMessageItem) ipMsgItem).mIpMessageId > 0
                && onIpMessageMenuItemClick(item, (RcseMessageItem) ipMsgItem)) {
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpUpdateTitle(String number, String titleOriginal,
            ImageView ipCustomView, ArrayList<String> titles) {

        Logger.d(TAG, "onIpUpdateTitle number = "
                + number + " mChatModeNumber:" + mChatModeNumber);

        if(mChatModeNumber.equals("") && number != null)
            mChatModeNumber = number;
        else if (number == null) {
            number = mChatModeNumber;
        }

        if (TextUtils.isEmpty(number)) {
            return false;
        }

        if (!(IpMessageContactManager.getInstance(mContext).isIpMessageNumber(number))) {
            return false;
        }

        int integrationMode = 0;
        String title = IpMessageContactManager.getInstance(mContext).getNameByNumber(number);
        String subTitle = mChatModeNumber;
        if (TextUtils.isEmpty(title)) {
            title = titleOriginal;
        }
        String numberAfterFormat = number;
        if (!title.equals(number)) {
            if (number.startsWith(IpMessageConsts.JOYN_START)) {
                numberAfterFormat = IpMessageUtils.formatNumber(number.substring(4),
                        mContext.getApplicationContext());
            }
            if (!title.equals(numberAfterFormat)) {
                subTitle = numberAfterFormat;
            }
        }
        integrationMode = IpMessageContactManager.getInstance(mContext)
                .getIntegratedModeForContact(number);

        mFullIntegratedView = ipCustomView;
        Drawable integratedIcon = IpMessageResourceMananger.getInstance(mContext)
                .getSingleDrawable(
                IpMessageConsts.drawable.ipmsg_full_integrated);
        mFullIntegratedView.setImageDrawable(integratedIcon);
        if (integrationMode == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED) {
            mFullIntegratedView.setVisibility(View.VISIBLE);
        } else {
            mFullIntegratedView.setVisibility(View.GONE);
        }
        Logger.d(TAG, "onIpUpdateTitle exit title: " + title + " subTitle: "
                + subTitle + " integrationMode: " + integrationMode);
        titles.add(title);
        titles.add(subTitle);
        return true;
    }

    @Override
    public boolean onIpRecipientsEditorFocusChange(boolean hasFocus, List<String> numbers) {
        Logger.d(TAG, "onIpRecipientsEditorFocusChange hasFocus =" + hasFocus);
        if (numbers.size() == 1) {
            updateCurrentChatMode(null, numbers.get(0));
        } else {
            updateCurrentChatMode(null, "");
        }
        if (hasFocus) {
            showSharePanel(false);
        }
        return true;
    }

    @Override
    public boolean onIpInitialize(Intent intent, IWorkingMessageCallback workingMessageCallback) {
        Logger.d(TAG, "onIpInitialize intent = " + intent);
        mWorkingMessage = workingMessageCallback;
        String[] numbers = mCallback.getConversationInfo();
        Logger.v(TAG, "onIpInitialize numbers = " + numbers);
        if(numbers == null) {
            Logger.d(TAG, "onIpInitialize numbers null, so getNumber() " + getNumber());
            updateCurrentChatMode(intent, getNumber());
        }

        if (numbers != null && numbers.length != 0) {
            Logger.v(TAG, "onIpInitialize number " + numbers[0]);
            updateCurrentChatMode(intent, numbers[0]);
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpSaveInstanceState(Bundle outState, long threadId) {
        Logger.d(TAG, "onIpSaveInstanceState threadId =" + threadId);
        outState.putBoolean("saved_ipmessage_draft", mIpMessageDraft != null);
        if (threadId != 0) {
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpShowSmsOrMmsSendButton(boolean isMms) {
        log("onIpShowSmsOrMmsSendButton isMms");
        if (mIsIpServiceEnabled
                && ((mIpMessageDraft != null) || isCurrentRecipientIpMessageUser())
                && mCurrentChatMode == IpMessageConsts.ChatMode.JOYN
                && mSendButtonIpMessage != null) {
            mSendButtonIpMessage.setImageDrawable(
                    IpMessageResourceMananger.getInstance(mContext).getSingleDrawable(
                    IpMessageConsts.drawable.enter_joyn_chat));
            mSendButtonIpMessage.setEnabled(true);
            if (mSendButtonIpMessage != null) {
                mSendButtonIpMessage.setVisibility(View.VISIBLE);
            }
            if(mShareButton != null) {
                mShareButton.setVisibility(View.VISIBLE);
            }
            mCurrentChatMode = IpMessageConsts.ChatMode.JOYN;
            return true;
        } else {
            if(mShareButton != null) {
                mShareButton.setVisibility(View.GONE);
            }
            mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
            return false;
        }
    }

    @Override
    public boolean onIpPrepareOptionsMenu(IIpConversationExt ipConv, Menu menu) {
        mRcseConversation = ((RcseConversation) ipConv);
        // / M: whether has IPMsg APK or not. ture: has ; false: no;
        boolean hasIpMsgApk = IpMmsConfig.isServiceEnabled(mContext);
        // / M: the identity of whether the current users all are ipmessage user
        // or not.
        boolean hasIpMsgUser = isCurrentRecipientIpMessageUser();
        // / M: true: the host has been activated.
        boolean hasActivatedHost = IpMmsConfig.isActivated(mContext);

        Log.d(TAG, "onIpPrepareOptionsMenu:" + mIsSmsEnabled
                + " :" + hasIpMsgUser
                + " :" + hasActivatedHost
                + " :" + mCurrentChatMode);

        if (mIsSmsEnabled) {
            if (hasIpMsgUser && hasActivatedHost) {
                menu.add(0, MENU_INVITE_FRIENDS_TO_CHAT, 0,
                        IpMessageResourceMananger.getInstance(mContext).
                        getSingleString(IpMessageConsts.string.ipmsg_invite_friends_to_chat));
                if (IpMessageServiceMananger.getInstance(mContext)
                        .isFeatureSupported(IpMessageConsts.FeatureId.EXPORT_CHAT)) {
                    menu.add(0, MENU_EXPORT_CHAT, 0,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_export_chat));
                }
                if (getRecipientSize() == 1) {
                    if (mCurrentChatMode == IpMessageConsts.ChatMode.JOYN
                            && IpMessageContactManager.getInstance(mContext).getRegisterStatus()) {
                        if (!IpMessageServiceMananger.getInstance(mContext)
                                .isAlwaysSendMessageByJoyn()) {
                            menu.add(1, MENU_SEND_BY_SMS_IN_JOYN_MODE, 0,
                                    IpMessageResourceMananger.getInstance(mContext).getSingleString(
                                    IpMessageConsts.string.ipmsg_send_by_xms));
                        }
                    }
                    if (mCurrentChatMode == IpMessageConsts.ChatMode.XMS
                            && IpMessageContactManager.getInstance(mContext).getRegisterStatus()) {
                        menu.add(1, MENU_SEND_BY_JOYN_IN_SMS_MODE, 0,
                                IpMessageResourceMananger.getInstance(mContext).getSingleString(
                                IpMessageConsts.string.ipmsg_send_by_joyn));
                    }
                }
               // return true;
            } else if (!hasIpMsgUser && mIsIpServiceEnabled && mShowInviteMsg) {
                menu.add(
                        0,
                        MENU_INVITE_FRIENDS_TO_IPMSG,
                        0,
                        IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(
                                        IpMessageConsts.string.ipmsg_invite_friends_to_ipmsg));
                //return true;
            }
        }
        if (getRecipientSize() == 1) {
                String number = getNumber();
                // add for joyn converged inbox mode
                if (number.startsWith(IpMessageConsts.JOYN_START)) {
                    number = number.substring(4);
                }

                if (mIsSmsEnabled) {
                    if (mCurrentChatMode == IpMessageConsts.ChatMode.JOYN) {
                        log("onIpPrepareOptionsMenu, EnterXMS chat :");
                        String enterXmsChat = IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(
                                IpMessageConsts.string.ipmsg_enter_xms_chat);
                        //Drawable enterXmsIcon = mContext.getResources().getDrawable(
                        //        R.drawable.ic_launcher_smsmms);
                        Drawable enterXmsIcon = IpMessageResourceMananger.getInstance(
                                mContext).getSingleDrawable(
                                IpMessageConsts.drawable.ipmsg_jump_to_xms);
                        menu.add(0, MENU_JUMP_TO_XMS, 0, enterXmsChat).setIcon(enterXmsIcon)
                                .setTitle(enterXmsChat)
                                .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
                        menu.removeItem(MENU_ADD_ATTACHMENT);
                    } else {
                        if (IpMessageServiceMananger.getInstance(mContext)
                                .getIntegrationMode() == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED &&
                                IpMessageContactManager.getInstance(mContext).isIpMessageNumber(
                                getNumber())) {
                            log("onIpPrepareOptionsMenu, Enter joyn chat :FULLY_INTEGRATED");
                            String enterJoynChat = IpMessageResourceMananger.getInstance(
                                    mContext).getSingleString(
                                    IpMessageConsts.string.ipmsg_enter_joyn_chat);
                            Drawable enterJoynIcon = IpMessageResourceMananger.getInstance(
                                    mContext).getSingleDrawable(
                                    IpMessageConsts.drawable.ipmsg_jump_to_joyn);
                            menu.add(0, MENU_JUMP_TO_JOYN, 0, enterJoynChat)
                                    .setIcon(enterJoynIcon).setTitle(enterJoynChat)
                                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
                        } else if(IpMessageContactManager.getInstance(mContext).isIpMessageNumber(
                                getNumber())) {
                            log("onIpPrepareOptionsMenu, Enter joyn chat :CONVERGED_INBOX");
                            String enterJoynChat = IpMessageResourceMananger.getInstance(
                                    mContext).getSingleString(
                                    IpMessageConsts.string.ipmsg_enter_joyn_chat);
                            Drawable enterJoynIcon = IpMessageResourceMananger.getInstance(
                                    mContext).getSingleDrawable(
                                    IpMessageConsts.drawable.ipmsg_jump_to_joyn);
                            menu.add(0, MENU_JUMP_TO_JOYN, 0, enterJoynChat)
                                    .setIcon(enterJoynIcon).setTitle(enterJoynChat)
                                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
                            }
                    }
                }
            return true;
        }

        if (mIsSmsEnabled && hasActivatedHost) {
            log("onPrepareOptionsMenu(): spam = " + ((RcseConversation) ipConv).isSpam());
            if (((RcseConversation)ipConv).isSpam()) {
                menu.add(0, MENU_REMOVE_SPAM, 0, IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.remove_from_spam));
            } else {
                menu.add(0, MENU_MARK_AS_SPAM, 0, IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.mark_as_spam));
            }
            return true;
        }
        return false;
    }

    @Override
    public void onIpMsgActivityResult(Context context, int requestCode, int resultCode, Intent data) {
        Logger.d(TAG, "onIpMsgActivityResult, requestCode :" + requestCode);
        if (resultCode != RESULT_OK) {
            Logger.v(TAG, "bail due to resultCode=" + resultCode);
            return;
        }

        // / M: add for ip message
        switch (requestCode) {
            case REQUEST_CODE_INVITE_FRIENDS_TO_CHAT:
                List<String> allList = new ArrayList<String>();
                String[] numbers = mCallback.getConversationInfo();
                if (numbers != null && numbers.length > 0) {
                    for (String number : numbers) {
                        allList.add(IpMessageContactManager.getInstance(mContext)
                                .getContactIdByNumber(number) + "");
                    }
                }

                String[] mSelectContactsIds = data
                        .getStringArrayExtra(IpMessageUtils.SELECTION_CONTACT_RESULT);
                if (mSelectContactsIds != null && mSelectContactsIds.length > 0) {
                    String idStr = "";
                    for (int index = 0; index < mSelectContactsIds.length; index++) {
                        idStr = mSelectContactsIds[index];
                        if (!allList.contains(idStr)) {
                            allList.add(idStr);
                        }
                    }
                }

                if (allList != null && allList.size() > 0) {
                    Intent intent = new Intent(RemoteActivities.NEW_GROUP_CHAT);
                    intent.putExtra(RemoteActivities.KEY_ARRAY,
                            allList.toArray(new String[allList.size()]));
                    IpMessageUtils.startRemoteActivity(mContext, intent);
                    allList = null;
                } else {
                    Logger.v(TAG,
                            "onActivityResult(): SELECT_CONTACT get contact id is NULL!");
                }
                return;
            case REQUEST_CODE_IP_MSG_PICK_CONTACTS:
                long threadId = mCallback.genIpThreadIdFromContacts(data);
                if (threadId <= 0) {
                    Log.d(TAG, "[onIpMsgActivityResult] return thread id <= 0");
                    break;
                }
                Intent it = createIntent(mContext.getApplicationContext(), threadId);
                mContext.startActivity(it);
                break;
            case REQUEST_CODE_IPMSG_TAKE_PHOTO:
                Logger.v(TAG,
                        "onActivityResult(): REQUEST_CODE_IPMSG_TAKE_PHOTO");
                if (!getVideoOrPhoto(data, requestCode)) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_err_file),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                if (!IpMessageUtils.isValidAttach(mDstPath, false)) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_err_file),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                if (!IpMessageUtils.isPic(mDstPath)) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_invalid_file_type),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                if (IpMessageSettingsManger.getInstance(mContext).isPicNeedResize()) {
                    new Thread(mResizePic, "ipmessage_resize_pic").start();
                } else {
                    sendImage(requestCode);
                }
                return;

            case REQUEST_CODE_IPMSG_RECORD_VIDEO:
                if (!getVideoOrPhoto(data, requestCode)) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_err_file),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                if (!IpMessageUtils.isVideo(mDstPath)) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_invalid_file_type),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                if (!IpMessageUtils.isFileStatusOk(mContext, mDstPath)) {
                    Logger.e(TAG, "onIpMsgActivityResult(): record video failed, invalid file");
                    return;
                }
                mIpMsgHandler.postDelayed(mSendVideo, 100);
                return;

            case REQUEST_CODE_IPMSG_SHARE_CONTACT:
                asyncIpAttachVCardByContactsId(data);
                return;

            case REQUEST_CODE_IPMSG_CHOOSE_PHOTO:
                if (!getVideoOrPhoto(data, requestCode)) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_err_file),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                if (!IpMessageUtils.isPic(mDstPath)) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_invalid_file_type),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                if (IpMessageSettingsManger.getInstance(mContext).isPicNeedResize()) {
                    new Thread(mResizePic).start();
                } else {
                    sendImage(requestCode);
                }
                return;

            case REQUEST_CODE_IPMSG_CHOOSE_VIDEO:
                getVideoOrPhoto(data,requestCode);
                if (!IpMessageUtils.isVideo(mDstPath)) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_invalid_file_type),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                if (!IpMessageUtils.isFileStatusOk(mContext, mDstPath)) {
                    Logger.e(TAG, "onIpMsgActivityResult(): choose video failed, invalid file");
                    return;
                }
                int mmsSizeLimit = 300 * 1024;
                /*if (IpMessageUtils.getFileSize(mDstPath) > mmsSizeLimit) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_file_limit),
                            Toast.LENGTH_SHORT).show();
                    return;
                }*/
                mIpMsgHandler.postDelayed(mSendVideo, 100);
                return;

            case REQUEST_CODE_IPMSG_RECORD_AUDIO:
                if (!getVideoOrPhoto(data, requestCode)) {
                    return;
                }
                if (!IpMessageUtils.isAudio(mDstPath)) {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_invalid_file_type),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                if (!IpMessageUtils.isFileStatusOk(mContext, mDstPath)) {
                    Logger.e(TAG, "onIpMsgActivityResult(): record audio failed, invalid file");
                    return;
                }
                mIpMsgHandler.postDelayed(mSendAudio, 100);
                return;
            case REQUEST_CODE_IPMSG_CHOOSE_AUDIO:
                if (data != null) {
                    Uri uri = (Uri) data
                            .getParcelableExtra(RingtoneManager.EXTRA_RINGTONE_PICKED_URI);
                    if (Settings.System.getUriFor(Settings.System.RINGTONE).equals(uri)) {
                        return;
                    }
                    if (getAudio(data)) {
                        mIpMsgHandler.postDelayed(mSendAudio, 100);
                    }
                }
                return;
            case REQUEST_CODE_IPMSG_SHARE_VCALENDAR:
                String calendar = data.getDataString();
                if (TextUtils.isEmpty(calendar)) {
                    return;
                }
                getCalendar(mContext, calendar);
                mIpMsgHandler.postDelayed(mSendCalendar, 100);
                return;
            case REQUEST_CODE_IPMSG_SHARE_FILE:
                if (data != null) {
                    sendFileViaJoyn(data);
                } else {
                    Logger.v(TAG,
                            "doInBackground() mData is null,that is onActivityResult() get a null intent");
                }
                return;
            default:
                break;
        }
    }

    public boolean getVideoOrPhoto(Intent data, int requestCode, Uri uri) {
        boolean isOk = true;
        Logger.d(TAG, "getVideoOrPhoto():uri:" + uri);
        try {
        Cursor cursor = null;
        if (requestCode == REQUEST_CODE_IPMSG_TAKE_PHOTO
                || requestCode == REQUEST_CODE_IPMSG_CHOOSE_PHOTO) {
            final String[] selectColumn = {
                "_data"
            };
            cursor = mContext.getContentResolver().query(uri, selectColumn, null, null, null);
        } else {
            final String[] selectColumn = {
                    "_data"
            };
            cursor = mContext.getContentResolver().query(uri, selectColumn, null, null, null);
        }
        if (null == cursor) {
            if (requestCode == REQUEST_CODE_IPMSG_RECORD_AUDIO) {
                mDstPath = uri.getEncodedPath();
                mDuration = data.getIntExtra("audio_duration", 0);
                mDuration = mDuration / 1000 == 0 ? 1 : mDuration / 1000;
            } else {
                mPhotoFilePath = uri.getEncodedPath();
                mDstPath = mPhotoFilePath;
                Logger.v(TAG, "getVideoOrPhoto():mDstPath" + mDstPath);
            }
            return true;
        }
        if (0 == cursor.getCount()) {
            cursor.close();
            Logger.e(TAG, "getVideoOrPhoto(): take video cursor getcount is 0");
            return false;
        }
        cursor.moveToFirst();
        if (requestCode == REQUEST_CODE_IPMSG_TAKE_PHOTO
                || requestCode == REQUEST_CODE_IPMSG_CHOOSE_PHOTO) {
            mPhotoFilePath = cursor.getString(cursor.getColumnIndex("_data"));
            mDstPath = mPhotoFilePath;
        } else {
            mDstPath = cursor.getString(cursor.getColumnIndex("_data"));
            isOk = initMediaDuration(uri);
        }
        if (null != cursor && !cursor.isClosed()) {
            cursor.close();
        }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return false;
        }
        Logger.d(TAG, "getVideoOrPhoto():mDstPath" + mDstPath + ":" + mDuration);
        return isOk;
    }

    @Override
    public boolean onIpHandleForwardedMessage(Intent intent) {
        Logger.d(TAG, "onIpHandleForwardedMessage, intent :" + intent);
        if (intent.getBooleanExtra(FORWARD_IPMESSAGE, false)) {/*
            long ipMsgId = intent.getLongExtra(IP_MESSAGE_ID, 0);
            mIpMessageDraft = IpMessageManager.getInstance(mContext).getIpMsgInfo(ipMsgId);
            mIpMessageDraft.setId(0);
            if (mIpMessageDraft.getType() == IpMessageType.PICTURE
                    && !TextUtils.isEmpty(((IpImageMessage) mIpMessageDraft).getCaption())) {
                mWorkingMessage.setIpText(((IpImageMessage) mIpMessageDraft).getCaption());
            } else if (mIpMessageDraft.getType() == IpMessageType.VOICE
                    && !TextUtils.isEmpty(((IpVoiceMessage) mIpMessageDraft).getCaption())) {
                mWorkingMessage.setIpText(((IpVoiceMessage) mIpMessageDraft).getCaption());
            } else if (mIpMessageDraft.getType() == IpMessageType.VIDEO
                    && !TextUtils.isEmpty(((IpVideoMessage) mIpMessageDraft).getCaption())) {
                mWorkingMessage.setIpText(((IpVideoMessage) mIpMessageDraft).getCaption());
            }
            saveIpMessageForAWhile(mIpMessageDraft);
            return true;
        */}
        return false;
    }

    @Override
    public boolean onIpInitMessageList(ListView list, IIpMessageListAdapterExt adapter) {
        Logger.d(TAG, "onIpInitMessageList, mIsIpMessageRecipients :" + mIsIpMessageRecipients);
        mIsIpMessageRecipients = isCurrentRecipientIpMessageUser();
        // / M: add for ip message, online divider
        if (mIsIpMessageRecipients && getRecipientSize() == 1) {
            if (AppSettings.getInstance().isStoreForwardWarningActivated()) {
                mCallback
                        .setIpOnlineDividerString(getOnlineDividerString(IpMessageContactManager
                                .getInstance(mContext).getRemoteStatusByNumber(
                                        getNumber())));
            } else {
                mCallback
                        .setIpOnlineDividerString(getOnlineDividerString(IpMessageConsts.ContactStatus.ONLINE));

            }
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpSaveDraft( long threadId) {
        Logger.d(TAG, "onIpSaveDraft, number :" /*+ number*/ + " threadId =" + threadId);
        /// M: add for save IP message draft.@{
        if (mContext.isFinishing()) {
            if (mIpMessageDraft != null) {
                //saveIpMessageDraft(number);
                return true;
            }
            if (mIpMessageDraftId > 0) {
                IpMessageUtils.deleteIpMessageDraft(mContext, threadId);
            }
        }
        return false;
    }

    @Override
    public boolean onIpResetMessage() {
        Logger.v(TAG, "onIpResetMessage, mCurrentChatMode :" + mCurrentChatMode);
        if (mCurrentChatMode != IpMessageConsts.ChatMode.XMS) {
            mIsIpServiceEnabled = IpMmsConfig.isServiceEnabled(mContext);
        }

        // / M: add for ip message, update online divider
        mIsIpMessageRecipients = isIpMessageRecipients();
        if (mIsIpServiceEnabled && isNetworkConnected(mContext.getApplicationContext())) {
            if (!TextUtils.isEmpty(mChatModeNumber)) {
                Logger.v(TAG, "resetMessage(): update mChatModeNumber from " + mChatModeNumber
                        + " to " + getNumber());
                IpMessageChatManger.getInstance(mContext).exitFromChatMode(mChatModeNumber);
                mChatModeNumber = getNumber();
                IpMessageChatManger.getInstance(mContext).enterChatMode(mChatModeNumber);
            } else if (TextUtils.isEmpty(mChatModeNumber)) {
                Logger.v(TAG,
                        "resetMessage(): update mChatModeNumber after send message, mChatModeNumber = "
                                + mChatModeNumber);
                mChatModeNumber = getNumber();
                IpMessageChatManger.getInstance(mContext).enterChatMode(mChatModeNumber);
            }
        }
        if (mIpMessageForSend == null || mIsClearTextDraft) {
            // / M: add for ip message, clear IP message draft
            mIpMessageDraftId = 0;
            clearIpMessageDraft();
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpUpdateTextEditorHint() {
        log("onIpUpdateTextEditorHint, mIsIpServiceEnabled:" + mIsIpServiceEnabled
                + " mIsSmsEnabled:" + mIsSmsEnabled + " mCurrentChatMode:" + mCurrentChatMode);
        if (mIsIpServiceEnabled && isNetworkConnected(mContext.getApplicationContext()) && mTextEditor != null) {
            if (null != mIpMessageDraft
                    || (mWorkingMessage != null && !mWorkingMessage.requiresIpMms() && isCurrentRecipientIpMessageUser()
                    && mCurrentChatMode == IpMessageConsts.ChatMode.JOYN)) {
                if (mIsSmsEnabled) {
                    mTextEditor.setHint(IpMessageResourceMananger.getInstance(mContext)
                            .getSingleString(
                            IpMessageConsts.string.ipmsg_hint));
                } else {
                    mTextEditor.setHint(R.string.sending_disabled_not_default_app);
                }
                mTextEditor.setFilters(new InputFilter[] {
                        new TextLengthFilter(IpMessageManager.getInstance(mContext).getMaxTextLimit())
                    });
                updateIpMessageCounter(mWorkingMessage.getIpText(), 0, 0, 0);
                return true;
            }
        }
        return false;
    }

    public Handler mIpMsgHandler = new Handler() {
        public void handleMessage(Message msg) {
            Logger.v(TAG, "mIpMsgHandler handleMessage, msg.what: " + msg.what);
            switch (msg.what) {
                case ACTION_RCS_SHARE:
                    if (IpMmsConfig.isServiceEnabled(mContext)
                            && isNetworkConnected(mContext)
                            && IpMessageUtils.getSDCardStatus()) {
                        doMoreAction(msg);
                    }
                    break;
                case ACTION_SHARE:
                    doMmsAction(msg);
                    break;
                default:
                    Logger.v(TAG, "msg type: " + msg.what + "not handler");
                    break;
            }
            super.handleMessage(msg);
        }
    };

    /**
     * This filter will constrain edits not to make the length of the text
     * greater than the specified length.
     */
    class TextLengthFilter extends InputFilter.LengthFilter {
        Toast mExceedMessageSizeToast = null;
        InputMethodManager mInputMethodManager = null;
        public TextLengthFilter(int max) {
            super(max);
            Logger.v(TAG, "TextLengthFilter entry ");
            mMaxLength = max;
            mExceedMessageSizeToast = Toast.makeText(mContext,
                    "Text limit reached.",
                    Toast.LENGTH_SHORT);
            mInputMethodManager = (InputMethodManager) mContext.getSystemService(Context.INPUT_METHOD_SERVICE);
        }

        public CharSequence filter(CharSequence source, int start, int end,
                                   Spanned dest, int dstart, int dend) {
            int keep = mMaxLength - (dest.length() - (dend - dstart));
            if (keep < (end - start)) {
                mExceedMessageSizeToast.show();
                mInputMethodManager.restartInput(mContext.getWindow().getCurrentFocus());
            }
            return super.filter(source, start, end, dest, dstart, dend);
        }

        private int mMaxLength;
    }

    @Override
    public boolean handleIpMessage(Message msg) {
        Logger.v(TAG, "handleIpMessage  msg.what: " + msg.what + "mCurrentChatMode:" + mCurrentChatMode);
        if(mCurrentChatMode == IpMessageConsts.ChatMode.XMS)
            return false;
        switch (msg.what) {
            case ACTION_SHARE:
                 Logger.v(TAG, "handleIpMessage  ACTION_SHARE: ");
                if (IpMmsConfig.isServiceEnabled(mContext)
                        /*&& isNetworkConnected(mContext)
                        && IpMessageUtils.getSDCardStatus()*/) {
                    doMoreAction(msg);
                }
                return true;
            default:
                Logger.v(TAG, "msg type: " + msg.what + "not handler");
                break;
        }
        return false;
    }

    private void clearIpMessageDraft() {
        Logger.v(TAG, "clearIpMessageDraft() mIpMessageDraft = " + mIpMessageDraft);
        mIpMessageDraft = null;
        mCallback.callbackUpdateSendButtonState();
    }

    // / M: add for IP message draft. @{
    private void saveIpMessageDraft(String number) {
        Logger.v(TAG, "saveIpMessageDraft() number = " + number);
        mChatModeNumber = number;
        if (mIpMessageDraft == null) {
            Logger.e(TAG, "saveIpMessageDraft(): mIpMessageDraft is null!");
            return;
        }
        mIpMessageDraft.setStatus(IpMessageStatus.DRAFT);

        mIpMessageDraft.setTo(number);
        mCallback.setIpDraftState(true);
        new Thread(new Runnable() {
            @Override
            public void run() {
                Logger.v(TAG, "saveIpMessageDraft(): calling API: saveIpMsg().");
                int ret = -1;
                ret = IpMessageManager.getInstance(mContext).saveIpMsg(
                        mIpMessageDraft, IpMessageSendMode.AUTO);
                if (ret < 0) {
                    Logger.w(TAG, "saveIpMessageDraft(): save IP message draft failed.");
                    mCallback.setIpDraftState(false);
                } else {
                    Logger.v(TAG, "saveIpMessageDraft(): save IP message draft successfully.");
                }
            }
        }).start();
    }

    @Override
    public boolean loadIpMessagDraft(long threadId) {
        Logger.v(TAG, "loadIpMessagDraft() threadId = " + threadId);
        if (mIpMessageDraft != null) {
            Logger.w(TAG, "loadIpMessagDraft(): mIpMessageDraft is not null!");
            return false;
        }
        IpMessage ipMessage = IpMessageUtils.readIpMessageDraft(mContext, threadId);
        if (ipMessage != null) {
            mIpMessageDraftId = ipMessage.getId();
            String caption = IpMessageUtils.getIpMessageCaption(ipMessage);
            Logger.v(TAG, "loadIpMessagDraft(): ipMessage is not null, mIpMessageDraftId = "
                    + mIpMessageDraftId + "caption = " + caption);
            if (!TextUtils.isEmpty(caption)) {
                mWorkingMessage.setIpText(caption);
                mTextEditor.setText(caption);
            }
            saveIpMessageForAWhile(ipMessage);
            return true;
        }
        Logger.w(TAG, "loadIpMessagDraft(): ipMessage is null!");
        return false;
    }

    private void saveIpMessageForAWhile(IpMessage ipMessage) {
        Logger.v(TAG, "saveIpMessageForAWhile() ipMessage = " + ipMessage);
        if (mIpMessageDraft != null && mIpMessageForSend != null) {
            showReplaceAttachDialog();
            return;
        }
        mIpMessageDraft = ipMessage;
        mIpMessageForSend = null;
        if (mWorkingMessage.requiresIpMms()) {
            convertIpMessageToMmsOrSms(true);
            mCallback.callbackUpdateSendButtonState();
            return;
        }
        switch (mIpMessageDraft.getType()) {
            case IpMessageType.TEXT:
                IpTextMessage textMessage = (IpTextMessage) mIpMessageDraft;
                mTextEditor.setText(textMessage.getBody());
                break;
            default:
                Logger.e(TAG, "saveIpMessageForAWhile(): Error IP message type. type = "
                        + mIpMessageDraft.getType());
                break;
        }
        onIpUpdateTextEditorHint();
    }

    private String getOnlineDividerString(int currentRecipientStatus) {
        Logger.v(TAG, "getOnlineDividerString() currentRecipientStatus = " + currentRecipientStatus);
        if (IpMmsConfig.isActivated(mContext) && mIsIpMessageRecipients) {
            switch (currentRecipientStatus) {
                case ContactStatus.OFFLINE:
                    Logger.v(TAG, "compose.getOnlineDividerString(): OFFLINE");

                    String onlineTimeString = IpMessageResourceMananger.getInstance(mContext)
                            .getSingleString(
                            IpMessageConsts.string.ipmsg_divider_never_online);
                    Logger.v(TAG, "compose.getOnlineDividerString(): OFFLINE");
                    return onlineTimeString;
                case ContactStatus.ONLINE:
                case ContactStatus.TYPING:
                case ContactStatus.RECORDING:
                    String name = IpMessageContactManager.getInstance(mContext)
                            .getNameByNumber(getNumber());
                    return String.format(IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_divider_online),
                            name);
                case ContactStatus.STATUSCOUNT:
                    Logger.v(TAG, "compose.getOnlineDividerString(): STATUSCOUNT");
                    break;
                default:
                    Logger.w(TAG, "compose.getOnlineDividerString(): unknown user status!");
                    break;
            }
        }
        return "";
    }

    @Override
    public boolean checkIpMessageBeforeSendMessage(long subId, boolean bCheckEcmMode) {
        Logger.d(TAG, "checkIpMessageBeforeSendMessage() mCurrentChatMode = " + mCurrentChatMode);
        if (mCurrentChatMode != IpMessageConsts.ChatMode.XMS) {
            mIsIpServiceEnabled = IpMmsConfig.isServiceEnabled(mContext);
        }
        mIsIpMessageRecipients = isCurrentRecipientIpMessageUser();

        if (mIpMessageForSend != null) {
            sendMessageForIpMsg(mIpMessageForSend, false, true);
            return true;
        }

        if (null != mIpMessageDraft) {
            // Has IP message draft
            if (!mIsIpServiceEnabled || !isNetworkConnected(mContext.getApplicationContext())) {
                // / M: disabled service
                if (mIpMessageDraft.getType() == IpMessageType.TEXT) {
                    showIpMessageConvertToMmsOrSmsDialog(SMS_CONVERT, SERVICE_IS_NOT_ENABLED);
                } else {
                    showIpMessageConvertToMmsOrSmsDialog(MMS_CONVERT, SERVICE_IS_NOT_ENABLED);
                }
                return true;
            }

            // / M: Fix ipmessage bug for ALPS01674002 @{
            if (!mIsIpMessageRecipients) {
                // / M: non-IP message User
                if (mIpMessageDraft.getType() == IpMessageType.TEXT) {
                    showIpMessageConvertToMmsOrSmsDialog(SMS_CONVERT,
                            !mIsIpMessageRecipients ? RECIPIENTS_ARE_NOT_IP_MESSAGE_USER
                                    : RECIPIENTS_IP_MESSAGE_NOT_SENDABLE);
                } else {
                    showIpMessageConvertToMmsOrSmsDialog(MMS_CONVERT,
                            !mIsIpMessageRecipients ? RECIPIENTS_ARE_NOT_IP_MESSAGE_USER
                                    : RECIPIENTS_IP_MESSAGE_NOT_SENDABLE);
                }
                // / @}
                return true;
            }

            if (sendMessageForIpMsg(mIpMessageDraft, true, true)) {
                mIpMessageDraftId = 0;
                clearIpMessageDraft();
            }
            return true;
        } else {
            Logger.d(TAG, "mIsIpMessageRecipients:" + mIsIpMessageRecipients);
            // No IP message draft
            if (mIsIpServiceEnabled
                    && mIsIpMessageRecipients
                    && (mCurrentChatMode == IpMessageConsts.ChatMode.JOYN || mSendJoynMsgInSmsMode)) {
                boolean isSmsCanConvertToIpmessage = !mWorkingMessage.requiresIpMms();
                if (isSmsCanConvertToIpmessage) {
                    // / M: send IP text message
                    if(!sendIpTextMessage()) {
                        Logger.e(TAG, "Empty text box, propably MMS Image ");
                        mSendSmsInJoynMode = false;
                        mSendJoynMsgInSmsMode = false;
                        return false;
                    }
                }
            }
            mSendSmsInJoynMode = false;
            mSendJoynMsgInSmsMode = false;
            return false;
        }
    }

    /**
     * M: whether the current recipient(s) are ipmessage user or not
     *
     * @return
     */
    private boolean isCurrentRecipientIpMessageUser() {
        if (IpMessageContactManager.getInstance(mContext).isIpMessageNumber(mChatModeNumber)) {
            return true;
        } else {
            return false;
        }
    }

    // / M: Fix ipmessage bug ,fix bug ALPS 01556382@{
    private boolean isCurrentIpmessageSendable() {
        if (IpMessageContactManager.getInstance(mContext).getStatusByNumber(mChatModeNumber) == ContactStatus.OFFLINE) {
            Logger.d(TAG, "isCurrentIpmessageSendable(): false ");
            return false;
        } else {
            return true;
        }
    }
    // / @}

    private boolean isIpMessageRecipients() {
        mIsIpMessageRecipients = IpMessageContactManager.getInstance(mContext).isIpMessageNumber(
                mChatModeNumber);
        Logger.v(TAG, "isIpMessageRecipients(): " + mIsIpMessageRecipients);
        return mIsIpMessageRecipients;
    }

    private Runnable mHideReminderRunnable = new Runnable() {
        @Override
        public void run() {
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                    "notificationsReceived(): hide reminder.");
            if (null != mTypingStatus) {
                mTypingStatus.setVisibility(View.GONE);
            }
        }
    };

    // show mms has cost reminder view
    private Runnable mShowMmsReminderRunnable = new Runnable() {
        @Override
        public void run() {
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                    "notificationsReceived(): show mms reminder view.");
            if (null != mTypingStatus) {
                mTypingStatus.setText(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(
                                IpMessageConsts.string.ipmsg_mms_cost_remind));
                mTypingStatus.setVisibility(View.VISIBLE);
            }
        }
    };

    // show joyn has cost reminder view
    private Runnable mShowJoynReminderRunnable = new Runnable() {
        @Override
        public void run() {
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                    "notificationsReceived(): show joyn reminder view.");
            if (null != mTypingStatus) {
                mTypingStatus.setText(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(
                                IpMessageConsts.string.ipmsg_joyn_cost_remind));
                mTypingStatus.setVisibility(View.VISIBLE);
            }
        }
    };

    private final int MMS_COST_REMINDER = 0;
    private final int JOYN_COST_REMINDER = 1;

    private void showReminderView(int type) {
        if (type == MMS_COST_REMINDER) {
            mIpMsgHandler.removeCallbacks(mShowMmsReminderRunnable);
            mIpMsgHandler.post(mShowMmsReminderRunnable);
        } else if (type == JOYN_COST_REMINDER) {
            mIpMsgHandler.removeCallbacks(mShowJoynReminderRunnable);
            mIpMsgHandler.post(mShowJoynReminderRunnable);
        }
        mIpMsgHandler.removeCallbacks(mHideReminderRunnable);
        mIpMsgHandler.postDelayed(mHideReminderRunnable, 2000);
    }

    @Override
    public void notificationsReceived(Intent intent) {
        Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                "compose.notificationsReceived(): start, intent = " + intent);
        String action = intent.getAction();
        if (TextUtils.isEmpty(action)) {
            return;
        }
        switch (IpMessageUtils.getActionTypeByAction(action)) {
        // / M: toast and update UI after activate ipmessage service @{
            case IpMessageUtils.IPMSG_REG_STATUS_ACTION:
                int regStatus = intent.getIntExtra(IpMessageConsts.RegStatus.REGSTATUS, 0);
                switch (regStatus) {
                    case IpMessageConsts.RegStatus.REG_OVER:
                        mContext.runOnUiThread(new Runnable() {
                            public void run() {
                                if (mUiHandler != null) {
                                    mUiHandler.sendEmptyMessage(UPDATE_SENDBUTTON);
                                }
                                Toast.makeText(
                                        mContext.getApplicationContext(),
                                        IpMessageResourceMananger
                                                .getInstance(mContext)
                                                .getSingleString(
                                                IpMessageConsts.string.ipmsg_nms_enable_success),
                                        Toast.LENGTH_SHORT).show();
                            }
                        });
                        break;
                    default:
                        break;
                }
                Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                        "notificationsReceived(): regStatus = " + regStatus);
                break;
            // / @}
            case IpMessageUtils.IPMSG_IM_STATUS_ACTION:
                Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                        "notificationsReceived(): IM status, mChatModeNumber = " + mChatModeNumber);
                mIsIpMessageRecipients = isCurrentRecipientIpMessageUser();
                if (!mIsIpServiceEnabled || !isNetworkConnected(mContext.getApplicationContext())
                        || getRecipientSize() != 1
                        || !mIsIpMessageRecipients || TextUtils.isEmpty(mChatModeNumber)) {
                    return;
                }
                final String number = intent.getStringExtra(IpMessageConsts.NUMBER);
                Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                        "notificationsReceived(): number = " + number + ", mChatModeNumber = "
                                + mChatModeNumber);
                if (!TextUtils.isEmpty(number)) {
                    int status = IpMessageContactManager.getInstance(mContext).getRemoteStatusByNumber(
                            number);
                    Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                            "notificationsReceived(): IM status. number = " + number
                                    + ", status = " + status);

                    mChatSenderName = IpMessageContactManager.getInstance(mContext)
                            .getNameByNumber(number);
                    switch (status) {
                        case ContactStatus.TYPING:
                            mIpMsgHandler.post(mShowTypingStatusRunnable);
                            break;
                        case ContactStatus.STOP_TYPING:
                            mIpMsgHandler.post(mHideTypingStatusRunnable);
                            break;
                        case ContactStatus.RECORDING:
                        case ContactStatus.STOP_RECORDING:
                            return;
                        case ContactStatus.ONLINE:
                        case ContactStatus.OFFLINE:
                            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                                    "notificationsReceived(): user online status changed, "
                                            + "number = " + number + ", mChatModeNumber = "
                                            + mChatModeNumber);
                            if (IpMessageServiceMananger.getInstance(mContext)
                                    .getIntegrationMode() == IpMessageConsts.IntegrationMode.CONVERGED_INBOX) {
                                if (AppSettings.getInstance()
                                        .isStoreForwardWarningActivated()) {
                                    mCallback
                                            .setIpOnlineDividerString(getOnlineDividerString(status));
                                    mCallback.updateIpOnlineDividerTime();
                                } else {
                                    mCallback
                                            .setIpOnlineDividerString(getOnlineDividerString(IpMessageConsts.ContactStatus.ONLINE));
                            mCallback.updateIpOnlineDividerTime();
                                }
                                return;
                            }
                            mContext.runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    mCallback.notifyIpDataSetChanged();
                                    // / M: Fix ipmessage bug @{
                                    int preChatMode = mCurrentChatMode;
                                    updateCurrentChatMode(null, mChatModeNumber);
                                    if (preChatMode != mCurrentChatMode) {
                                        mCallback.showIpOrMmsSendButton(false);
                                        mCallback.invalidateIpOptionsMenu();
                                    }
                                    // / @}
                                }
                            });
                            return;
                        case ContactStatus.STATUSCOUNT:
                        default:
                            return;
                    }
                }
                break;
            case IpMessageUtils.IPMSG_FALLBACK_MMS:
                String filePath = intent.getStringExtra("fileUrl");
                String captionText = intent.getStringExtra("caption");
                int type = intent.getIntExtra("type", 1);
                long messageIDinMms = intent.getLongExtra(IpMessageConsts.IpMessageStatus.IP_MESSAGE_ID, -1);
                Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                        "compose.notificationsReceived(): fallback entry, filePath = " + filePath);
                /*// Init MMS Service before sendign MMs
                SmsManager.getDefault().getAutoPersisting();
                synchronized (this) {
                    try {
                        this.wait(2000);
                    } catch (InterruptedException e) {
                        Logger.v(TAG, "InterruptedException");
                    }
                }*/
                mCallback.convertIpMessageToMmsOrSms(type, true, filePath, captionText, -1);
                mCallback.callbackCheckConditionsAndSendMessage(true);
                long ids[] = {messageIDinMms};
                IpMessageManager.getInstance(mContext).deleteIpMsg(ids,false);
                if (IpMessageServiceMananger.getInstance(mContext)
                        .getIntegrationMode() == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED) {
                    mIsIpServiceEnabled = false;
                    mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
                    mCallback.showIpOrMmsSendButton(false);
                    mCallback.invalidateIpOptionsMenu();
                    mCallback.resetSharePanel();
                }
                break;
            case IpMessageUtils.IPMSG_ERROR_ACTION:
            case IpMessageUtils.IPMSG_NEW_MESSAGE_ACTION:
            case IpMessageUtils.IPMSG_REFRESH_CONTACT_LIST_ACTION:
            case IpMessageUtils.IPMSG_REFRESH_GROUP_LIST_ACTION:
            case IpMessageUtils.IPMSG_SERCIVE_STATUS_ACTION:
            case IpMessageUtils.IPMSG_SAVE_HISTORY_ACTION:
            case IpMessageUtils.IPMSG_ACTIVATION_STATUS_ACTION:
                break;
            case IpMessageUtils.IPMSG_IP_MESSAGE_STATUS_ACTION:
                mContext.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mCallback.setClearIpCacheFlag(true);
                        mCallback.notifyIpDataSetChanged();
                    }
                });
                break;
            case IpMessageUtils.IPMSG_DOWNLOAD_ATTACH_STATUS_ACTION:
            case IpMessageUtils.IPMSG_SET_PROFILE_RESULT_ACTION:
            case IpMessageUtils.IPMSG_BACKUP_MSG_STATUS_ACTION:
            case IpMessageUtils.IPMSG_RESTORE_MSG_STATUS_ACTION:
            default:
                return;
        }
    }

    private boolean onIpMessageMenuItemClick(MenuItem menuItem, RcseMessageItem rcseMsgItem) {
        long ipMessageId = rcseMsgItem.mIpMessageId;
        long msgId = rcseMsgItem.mMsgId;

        Logger.d(TAG, "onIpMessageMenuItemClick(): ipMessageId:" + ipMessageId + "menuID:" + menuItem.getItemId());
        switch (menuItem.getItemId()) {
            case MENU_SEND_VIA_TEXT_MSG:
                if (IpMessageServiceMananger.getInstance(mContext).getIntegrationMode() == IpMessageConsts.IntegrationMode.CONVERGED_INBOX) {
                    jumpToJoynChat(false, ipMessageId);
                } else {
                     Logger.v(TAG, "onIpMessageMenuItemClick(): MENU_SEND_VIA_TEXT_MSG");
                    if (mWorkingMessage.requiresIpMms()
                            || !TextUtils.isEmpty(mTextEditor.getText().toString())
                            || mIpMessageDraft != null) {
                        showDiscardCurrentMessageDialog(msgId);
                    } else {
                        sendViaMmsOrSms(msgId);
                    }
                }
                return true;

            case MENU_RETRY:
                IpMessageManager.getInstance(mContext).resendMessage(msgId,
                        rcseMsgItem.mSubId);
                return true;

            case MENU_FORWARD_IPMESSAGE:
                Logger.v(TAG, "MENU_FORWARD_IPMESSAGE");
                hideInputMethod();
                if (IpMessageServiceMananger.getInstance(mContext).getIntegrationMode() == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED) {
                    forwardIpMsg(mContext, msgId);
                    return true;
                }
                return false;

            case MENU_SEND_SPAM:
                IpMessage ipmsg = IpMessageManager.getInstance(mContext).getIpMsgInfo(msgId);
                Logger.v(TAG, " Click MENU_SEND_SPAM");
                ((RcseConversation)mRcseConversation).setSpam(true);
                IpMessageManager.getInstance(mContext).initSpamReport(
                        ipmsg.getFrom(), ipmsg.getmMessageId(),ipmsg.getType());
                //((RcseConversation)ipConv).setSpam(true);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        String numbers = getNumber();
                        int contactId = IpMessageContactManager.getInstance(mContext)
                                .getContactIdByNumber(numbers);
                        int[] contactIds = {
                            contactId
                        };
                        if (!IpMessageContactManager.getInstance(mContext)
                                .addContactToSpamList(contactIds)) {
                            Logger.w(TAG,
                                    "onIpMsgOptionsItemSelected(): Mark as spam failed!");
                        }
                    }
                }).start();
                Toast.makeText(mContext, "Moved To Spam", Toast.LENGTH_LONG).show();
                return true;

            case MENU_REMOVE_SPAM:
                Logger.v(TAG, " Click MENU_REMOVE_SPAM");
                final boolean isSpamFromRemove = ((RcseConversation)mRcseConversation).isSpam();
                ((RcseConversation)mRcseConversation).setSpam(false);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        String number = getNumber();
                        int contactId = IpMessageContactManager.getInstance(mContext)
                                .getContactIdByNumber(number);
                        int[] contactIds = {
                            contactId
                        };
                        if (!IpMessageContactManager.getInstance(mContext)
                                .deleteContactFromSpamList(contactIds)) {
                            Logger.w(TAG, "onIpMsgOptionsItemSelected(): Remove spam failed!");
                            ((RcseConversation)mRcseConversation).setSpam(isSpamFromRemove);
                        }
                    }
                }).start();
                Toast.makeText(mContext, "Removed From Spam", Toast.LENGTH_LONG).show();
                return true;

            case MENU_SHARE:
                IpMessage ipMsp = IpMessageManager.getInstance(mContext).getIpMsgInfo(msgId);
                shareIpMsg(ipMsp);
                return true;

            case MENU_EXPORT_SD_CARD:
                IpMessage ipMessageForSave = IpMessageManager.getInstance(mContext)
                        .getIpMsgInfo(
                        msgId);
                Logger.v(TAG, "onIpMessageMenuItemClick(): Save IP message. msgId = " + msgId
                        + ", type = " + ipMessageForSave.getType());
                if (ipMessageForSave.getType() >= IpMessageType.PICTURE
                        && ipMessageForSave.getType() < IpMessageType.UNKNOWN_FILE) {
                    saveMsgInSDCard((IpAttachMessage) ipMessageForSave);
                }
                return true;

            case MENU_VIEW_IP_MESSAGE:
                IpMessage ipMessageForView = IpMessageManager.getInstance(mContext).getIpMsgInfo(msgId);
                Logger.v(TAG, "onIpMessageMenuItemClick(): View IP message. msgId = " + msgId
                        + ", type = " + ipMessageForView.getType());
                if (ipMessageForView.getType() >= IpMessageType.PICTURE
                        && ipMessageForView.getType() < IpMessageType.UNKNOWN_FILE) {
                    openMedia(msgId, (IpAttachMessage) ipMessageForView);
                }
                return true;

            default:
                break;
        }
        return false;
    }

    private Runnable mSendTypingRunnable = new Runnable() {
        @Override
        public void run() {
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "send chat mode, typing");
            if (!TextUtils.isEmpty(mChatModeNumber)) {
                IpMessageChatManger.getInstance(mContext).sendChatMode(
                        mChatModeNumber, ContactStatus.TYPING);
            }
        }
    };

    private Runnable mSendStopTypingRunnable = new Runnable() {
        @Override
        public void run() {
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "send chat mode, stop typing");
            if (!TextUtils.isEmpty(mChatModeNumber)) {
                IpMessageChatManger.getInstance(mContext).sendChatMode(
                        mChatModeNumber, ContactStatus.STOP_TYPING);
            }
        }
    };

    private boolean forwardIpMsg(Context context, long msgId) {
        if (msgId <= 0) {
            return false;
        }
        Intent intent = createIntent(context, 0);
        intent.putExtra(FORWARD_MESSAGE, true); // "forwarded_message", boolean
        intent.putExtra(FORWARD_IPMESSAGE, true); // "ip_msg_media_path",
                                                  // boolean
        intent.putExtra(IP_MESSAGE_ID, msgId); // "ip_msg_id", long
        intent.setClassName(context, "com.android.mms.ui.ForwardMessageActivity");
        mContext.startActivity(intent);
        return true;
    }

    private boolean isNetworkConnected(Context context) {
        boolean isNetworkConnected = false;
        ConnectivityManager connManager = (ConnectivityManager) mContext.getSystemService(mContext.CONNECTIVITY_SERVICE);
        State state = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI).getState();
        if (State.CONNECTED == state) {
            isNetworkConnected = true;
        }
        if (!isNetworkConnected) {
            state = connManager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE).getState();
            if (State.CONNECTED == state) {
                isNetworkConnected = true;
            }
        }
        return isNetworkConnected;
    }

    private NetworkInfo getActiveNetworkInfo(Context context) {
        ConnectivityManager connectivity = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivity == null) {
            return null;
        }
        return connectivity.getActiveNetworkInfo();
    }

    private void showReplaceAttachDialog() {
        if (mReplaceDialog != null) {
            return; // / M: shown already.
        }
        mReplaceDialog = new AlertDialog.Builder(mContext)
                .setTitle(
                        IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(IpMessageConsts.string.ipmsg_replace_attach))
                .setMessage(
                        IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(IpMessageConsts.string.ipmsg_replace_attach_msg))
                .setPositiveButton(R.string.OK, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mIpMessageDraft = null;
                        mReplaceDialog = null;
                        saveIpMessageForAWhile(mIpMessageForSend);
                    }
                })
                .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mIpMessageForSend = null;
                        mReplaceDialog = null;
                    }
                }).create();
        mReplaceDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                mReplaceDialog = null;
            }
        });
        mReplaceDialog.show();
    }

    private boolean updateIpMessageCounter(CharSequence text, int start, int before, int count) {
        Logger.v(TAG, "updateIpMessageCounter()");
        if (mIsIpServiceEnabled && isNetworkConnected(mContext.getApplicationContext())
                && IpMessageUtils.getSDCardStatus() && isCurrentRecipientIpMessageUser()
                && mWorkingMessage != null
                && !mWorkingMessage.requiresIpMms()
                && text != null && text.length() > 0 && isCurrentIpmessageSendable()) {
            final int length = text.length();
            mUiHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    Logger.v(TAG,
                            "updateIpMessageCounter(): IP text message, mTextEditor.getLineCount() = "
                                    + mTextEditor.getLineCount());
                    if (mTextEditor.getLineCount() > 1) {
                        mTextCounter.setVisibility(View.VISIBLE);
                        mTextCounter.setText(length + "/"
                                + IpMessageManager.getInstance(mContext).getMaxTextLimit());
                    } else {
                        mTextCounter.setVisibility(View.GONE);
                        mTextCounter.setText(length + "/"
                                + IpMessageManager.getInstance(mContext).getMaxTextLimit());
                    }
                }
            }, 100);
            return true;
        }
        return false;
    }

    private void startFileManager() {
        Logger.v(TAG, "startFileManager()");
        Intent intent = new Intent(CHOICE_FILEMANAGER_ACTION);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        mContext.startActivityForResult(intent, this.REQUEST_CODE_IPMSG_SHARE_FILE);
    }

    private boolean sendFileViaJoyn(Intent data) {
        String fileName = Uri.decode(data.getDataString());
        if (fileName != null && fileName.startsWith(FILE_SCHEMA)) {
            String fileFullPath = fileName.substring(FILE_SCHEMA.length(), fileName.length());
            if (fileFullPath != null) {
                Uri contentUri = null;
                Cursor c = mContext.getContentResolver().query(
                        MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                        new String[] {
                                MediaStore.MediaColumns._ID, Images.Media.MIME_TYPE
                        }, MediaStore.MediaColumns.DATA + "=?", new String[] {
                            fileFullPath
                        }, null);
                if (c != null && c.getCount() != 0 && c.moveToFirst()) {
                    contentUri = Uri.withAppendedPath(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                            c.getString(0));
                } else {
                    if (c != null) {
                        c.close();
                    }
                    c = mContext.getContentResolver().query(
                            MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                            new String[] {
                                    MediaStore.MediaColumns._ID, Audio.Media.MIME_TYPE
                            }, MediaStore.MediaColumns.DATA + "=?", new String[] {
                                fileFullPath
                            }, null);
                    if (c != null && c.getCount() != 0 && c.moveToFirst()) {
                        contentUri = Uri.withAppendedPath(
                                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, c.getString(0));
                    } else {
                        if (c != null) {
                            c.close();
                        }
                        c = mContext.getContentResolver().query(
                                MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                                new String[] {
                                        MediaStore.MediaColumns._ID, Video.Media.MIME_TYPE
                                }, MediaStore.MediaColumns.DATA + "=?", new String[] {
                                    fileFullPath
                                }, null);
                        if (c != null && c.getCount() != 0 && c.moveToFirst()) {
                            contentUri = Uri.withAppendedPath(
                                    MediaStore.Video.Media.EXTERNAL_CONTENT_URI, c.getString(0));
                        }
                    }
                }
                try {
                    if (c != null && c.getCount() != 0 && c.moveToFirst()) {
                        Logger.i(TAG, "Get id in MediaStore:" + c.getString(0));
                        Logger.i(TAG, "Get content type in MediaStore:" + c.getString(1));
                        Logger.i(TAG, "Get uri in MediaStore:" + contentUri);
                        String contentType = c.getString(1);
                        data.setData(contentUri);
                        if (contentType.startsWith("image/")) {
                            onIpMsgActivityResult(mContext, REQUEST_CODE_IPMSG_CHOOSE_PHOTO,
                                    RESULT_OK,
                                    data);
                        } else if (contentType.startsWith("video/")) {
                            onIpMsgActivityResult(mContext, REQUEST_CODE_IPMSG_CHOOSE_VIDEO,
                                    RESULT_OK,
                                    data);
                        } else if (contentType.startsWith("audio/")) {
                            onIpMsgActivityResult(mContext, REQUEST_CODE_IPMSG_CHOOSE_AUDIO,
                                    RESULT_OK,
                                    data);
                        }
                        return true;
                    } else {
                        sendUnknownMsg(fileFullPath);
                        Logger.e(TAG, "MediaStore:" + MediaStore.Audio.Media.EXTERNAL_CONTENT_URI
                                + " has not this file");
                    }
                } finally {
                    if (c != null) {
                        c.close();
                    }
                }
            }
        }
        return false;
    }

    // / M: Fix ipmessage bug for ALPS 01566645 @{
    public void updateCurrentChatMode(Intent intent, String number) {
        // / M: add for ip message, check recipients is ipmessage User
        mChatModeNumber = number;
        mIsIpMessageRecipients = isCurrentRecipientIpMessageUser();

        boolean status = (TextUtils.isEmpty(mChatModeNumber)) ? false
                : isCurrentIpmessageSendable();

        Logger.d(TAG, "updateCurrentChatMode mIsIpMessageRecipients = "
                + mIsIpMessageRecipients + " status = " + status
                +" mode=" + IpMessageServiceMananger.getInstance(mContext).getIntegrationMode());

        // / @}
        if (IpMessageServiceMananger.getInstance(mContext).getIntegrationMode() == IpMessageConsts.IntegrationMode.ISMS_MODE) {
            if (mIsIpMessageRecipients && status) {
                mCurrentChatMode = IpMessageConsts.ChatMode.JOYN;
            } else {
                mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
            }
            return;
        }

        if (intent != null && intent.hasExtra("chatmode")) {
            mCurrentChatMode = intent.getIntExtra("chatmode", IpMessageConsts.ChatMode.XMS);
        } else {
            if (IpMessageServiceMananger.getInstance(mContext).getIntegrationMode() == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED) {
                Logger.d(TAG, "is full integrated mode");
                if (mIsIpMessageRecipients && status) {
                    mCurrentChatMode = IpMessageConsts.ChatMode.JOYN;
                } else {
                    mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
                }
            } else {
                if (number.startsWith(IpMessageConsts.JOYN_START)) {
                    mCurrentChatMode = IpMessageConsts.ChatMode.JOYN;
                } else {
                    mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
                }
            }
        }
        Logger.d(TAG, "updateCurrentChatMode mCurrentChatMode = " + mCurrentChatMode);
        if(mCurrentChatMode == IpMessageConsts.ChatMode.JOYN){
            mShareButton.setVisibility(View.VISIBLE);
        }
        mCallback.updateTextEditorHintCallback();
    }

    // / @}

    private Object mShowTypingLockObject = new Object();
    private Thread mShowTypingThread = new Thread(new Runnable() {
        @Override
        public void run() {
            String showingStr = IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_typing_text);
            final String displayString0 = showingStr + ".    ";
            final String displayString1 = showingStr + "..   ";
            final String displayString2 = showingStr + "...  ";
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                    "notificationsReceived(): mShowTypingThread.showingStr " + showingStr);
            int i = 0;
            while (!mIsDestroyTypingThread) {
                while (null != mTypingStatus && mTypingStatus.getVisibility() != View.GONE) {
                    switch (i % 3) {
                        case 0:
                            mIpMsgHandler.post(new Runnable() {
                                @Override
                                public void run() {
                                    Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                                            "mShowTypingThread: display 0.");
                                    mTypingStatus.setText(displayString0);
                                }
                            });
                            i++;
                            break;
                        case 1:
                            mIpMsgHandler.post(new Runnable() {
                                @Override
                                public void run() {
                                    Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                                            "mShowTypingThread: display 1.");
                                    mTypingStatus.setText(displayString1);
                                }
                            });
                            i++;
                            break;
                        case 2:
                            mIpMsgHandler.post(new Runnable() {
                                @Override
                                public void run() {
                                    Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                                            "mShowTypingThread: display 2.");
                                    mTypingStatus.setText(displayString2);
                                }
                            });
                            i++;
                            break;
                        default:
                            break;
                    }
                    synchronized (this) {
                        try {
                            this.wait(1000);
                        } catch (InterruptedException e) {
                            Logger.v(TAG, "InterruptedException");
                        }
                    }
                }
                synchronized (mShowTypingLockObject) {
                    try {
                        mShowTypingLockObject.wait();
                    } catch (InterruptedException e) {
                        Logger.v(TAG, "InterruptedException");
                    }
                }
            }
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "mShowTypingThread: destroy thread.");
        }
    }, "showTypingThread");

    private Runnable mResizePic = new Runnable() {
        @Override
        public void run() {
            Logger.v(TAG, "mResizePic(): start resize pic.");
            byte[] img = IpMessageUtils.resizeImg(mPhotoFilePath, (float) 500);
            if (null == img) {
                return;
            }
            Logger.v(TAG, "mResizePic(): put stream to file.");
            try {
                IpMessageUtils.nmsStream2File(img, mDstPath);
            } catch (Exception e) {
                e.printStackTrace();
                return;
            }
            Logger.v(TAG, "mResizePic(): post send pic.");
            mIpMsgHandler.postDelayed(mSendPic, 100);
        }
    };

    private boolean sendMessageForIpMsg(final IpMessage ipMessage, boolean isSendSecondTextMessage,
            final boolean isDelDraft) {
        Logger.d(TAG, "sendMessageForIpMsg(): start.");
        if (mWorkingMessage.requiresIpMms() && mIpMessageForSend == null) {
            mIpMessageDraft = ipMessage;
            convertIpMessageToMmsOrSms(true);
            mCallback.callbackUpdateSendButtonState();
            return false;
        }
        ipMessage.setTo(mChatModeNumber);
        if (TextUtils.isEmpty(ipMessage.getTo()) && mIpMessageForSend == null) {
            saveIpMessageForAWhile(ipMessage);
            mCallback.callbackUpdateSendButtonState();
            mCallback.callbackUpdateButtonState(true);
            return false;
        }

        if (!IpMmsConfig.isServiceEnabled(mContext)) {
            if (mIpMessageForSend == null) {
                mIpMessageDraft = ipMessage;
            }
            if (ipMessage.getType() == IpMessageType.TEXT) {
                showIpMessageConvertToMmsOrSmsDialog(SMS_CONVERT, SERVICE_IS_NOT_ENABLED);
            } else {
                showIpMessageConvertToMmsOrSmsDialog(MMS_CONVERT, SERVICE_IS_NOT_ENABLED);
            }
            return false;
        }

        // / M: Fix ipmessage bug for ALPS01674002 @{
        boolean isCurrentIpUser = isCurrentRecipientIpMessageUser();
        if (!isCurrentIpUser) {
            if (mIpMessageForSend == null) {
                mIpMessageDraft = ipMessage;
            }
            if (ipMessage.getType() == IpMessageType.TEXT) {
                showIpMessageConvertToMmsOrSmsDialog(SMS_CONVERT,
                        !isCurrentIpUser ? RECIPIENTS_ARE_NOT_IP_MESSAGE_USER
                                : RECIPIENTS_IP_MESSAGE_NOT_SENDABLE);
            } else {
                showIpMessageConvertToMmsOrSmsDialog(MMS_CONVERT,
                        !isCurrentIpUser ? RECIPIENTS_ARE_NOT_IP_MESSAGE_USER
                                : RECIPIENTS_IP_MESSAGE_NOT_SENDABLE);
            }
            return false;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "sendMessageForIpMsg(): calling API: saveIpMsg().");
                int ret = -1;
                ipMessage.setStatus(IpMessageStatus.OUTBOX);

                ret = IpMessageManager.getInstance(mContext).saveIpMsg(
                        ipMessage, IpMessageSendMode.AUTO);

                if (ret < 0) {
                    Logger.w(TAG, "sendMessageForIpMsg(): ");
                } else {
                    if (ipMessage.getType() == IpMessageType.TEXT) {
                        mCallback.asyncDeleteDraftSmsMessage();
                    }
                }
            }
        }).start();

        if (isSendSecondTextMessage && ipMessage.getType() != IpMessageType.TEXT
                && TextUtils.isEmpty(IpMessageUtils.getIpMessageCaption(ipMessage))
                && mTextEditor != null && mTextEditor.getVisibility() == View.VISIBLE
                && !TextUtils.isEmpty(mTextEditor.getText().toString())) {
            IpTextMessage ipTextMessage = new IpTextMessage();
            ipTextMessage.setBody(mTextEditor.getText().toString());
            ipTextMessage.setTo(mChatModeNumber);
            ipTextMessage.setStatus(IpMessageStatus.OUTBOX);

            final IpMessage ipTextMessageForSend = ipTextMessage;
            new Thread(new Runnable() {
                @Override
                public void run() {
                    Logger.v(TAG,
                            "sendMessageForIpMsg(): send second text IP message, calling API: saveIpMsg().");
                    int ret = -1;
                    ret = IpMessageManager.getInstance(mContext).saveIpMsg(
                            ipTextMessageForSend, IpMessageSendMode.AUTO);
                    if (ret < 0) {
                        Logger.w(TAG,
                                "sendMessageForIpMsg(): send second text IP message failed!");
                    }
                }
            }).start();
        }
        mCallback.syncIpWorkingRecipients();
        mCallback.guaranteeIpThreadId();
        if (mIpMessageForSend == null) {
            mCallback.onPreIpMessageSent();
            Logger.v(TAG, "sendMessageForIpMsg(): after guaranteeThreadId()");
        } else {
            if(ipMessage.getType() == IpMessageType.TEXT) {
                mCallback.resetIpMessage();
            }
            if((ipMessage.getType() != IpMessageType.TEXT) && mTextEditor != null
                    && TextUtils.isEmpty(mTextEditor.getText().toString())) {
                mCallback.resetIpMessage();
            }
            mIpMessageForSend = null;
        }
        mCallback.onIpMessageSent();
        return true;
    }

    private boolean sendIpTextMessage() {
        String body = mTextEditor.getText().toString();
        if (TextUtils.isEmpty(body)) {
            Logger.w(TAG, "sendIpTextMessage(): No content for sending!");
            return false;
        }
        IpTextMessage msg = new IpTextMessage();
        msg.setBody(body);
        msg.setType(IpMessageType.TEXT);
        mIpMessageForSend = msg;
        mIsClearTextDraft = true;
        sendMessageForIpMsg(msg, false, false);
        return true;
    }

    private Runnable mSendAudio = new Runnable() {
        public void run() {
            if (IpMessageUtils.isExistsFile(mDstPath) && IpMessageUtils.getFileSize(mDstPath) != 0) {
                IpVoiceMessage msg = new IpVoiceMessage();
                msg.setPath(mDstPath);
                msg.setDuration(mDuration);
                msg.setType(IpMessageType.VOICE);
                mIpMessageForSend = msg;
                sendMessageForIpMsg(msg, false, false);
                mIpMsgHandler.removeCallbacks(mSendAudio);
            }
        }
    };

    private Runnable mSendPic = new Runnable() {
        public void run() {
            Logger.v(TAG, "mSendPic(): start.");
            if (IpMessageUtils.isExistsFile(mDstPath) && IpMessageUtils.getFileSize(mDstPath) != 0) {
                Logger.v(TAG, "mSendPic(): start send image.");
                sendImage(REQUEST_CODE_IPMSG_TAKE_PHOTO);
                mIpMsgHandler.removeCallbacks(mSendPic);
                // refreshAndScrollList();
            }
            Logger.v(TAG, "mSendPic(): end.");
        }
    };

    private Runnable mSendVideo = new Runnable() {
        public void run() {
            Logger.v(TAG, "mSendVideo(): start send video. Path = " + mDstPath);
            if (IpMessageUtils.isExistsFile(mDstPath) && IpMessageUtils.getFileSize(mDstPath) != 0) {
                IpVideoMessage msg = new IpVideoMessage();
                msg.setPath(mDstPath);
                msg.setDuration(mDuration);
                msg.setType(IpMessageType.VIDEO);
                mIpMessageForSend = msg;
                sendMessageForIpMsg(msg, false, false);
                mIpMsgHandler.removeCallbacks(mSendVideo);
                // refreshAndScrollList();
            }
        }
    };

    private Runnable mSendVcard = new Runnable() {
        public void run() {
            // / M: cracks, wait activity resume, ensure dialog context valid.
           /* if (!mContext.isFinishing()) {
                mIpMsgHandler.postDelayed(mSendVcard, 100);
                Logger.v(TAG, "mSendVcard, wait activity resume.");
                return;
            }*/
            Logger.i(TAG, "mSendVcard(): entry");
            if (IpMessageUtils.isExistsFile(mDstPath) /*&& IpMessageUtils.getFileSize(mDstPath) != 0*/) {
                IpVCardMessage msg = new IpVCardMessage();
                msg.setPath(mDstPath);
                msg.setName(mIpMessageVcardName);
                msg.setType(IpMessageType.VCARD);
                mIpMessageForSend = msg;
                sendMessageForIpMsg(msg, false, false);
                mIpMsgHandler.removeCallbacks(mSendVcard);
                // refreshAndScrollList();
            }
        }
    };

    private Runnable mSendCalendar = new Runnable() {
        public void run() {
            // / M: cracks, wait activity resume, ensure dialog context valid.
          /*  if (!mContext.isFinishing()) {
                mIpMsgHandler.postDelayed(mSendCalendar, 100);
                Logger.v(TAG, "mSendCalendar, wait activity resume.");
                return;
            }*/

            if (IpMessageUtils.isExistsFile(mDstPath) /*&& IpMessageUtils.getFileSize(mDstPath) != 0*/) {
                IpVCalendarMessage msg = new IpVCalendarMessage();
                msg.setPath(mDstPath);
                msg.setSummary(mCalendarSummary);
                msg.setType(IpMessageType.CALENDAR);
                mIpMessageForSend = msg;
                sendMessageForIpMsg(mIpMessageForSend, false, false);
                mIpMsgHandler.removeCallbacks(mSendCalendar);
            }
        }
    };

    public boolean getVideoOrPhoto(Intent data, int requestCode) {
        boolean isOk = true;
        Logger.v(TAG, "getVideoOrPhoto():intent:" + data);
        if (null == data) {
            Logger.e(TAG, "getVideoOrPhoto(): take video error, result intent is null.");
            return false;
        }

        try {
            Uri uri = data.getData();

            Logger.v(TAG, "uri(): " + uri.toString());

            // Need to check the implementation of TempFileProvider
            // vendor/mediatek/proprietary/packages/apps/Mms/src/com/android/mms/
            // TempFileProvider.java

            Cursor cursor = null;
            if (requestCode == REQUEST_CODE_IPMSG_TAKE_PHOTO
                    || requestCode == REQUEST_CODE_IPMSG_CHOOSE_PHOTO) {
                final String[] selectColumn = {
                    "_data"
                };
                cursor = mContext.getContentResolver().query(uri, selectColumn, null, null, null);
            } else {
                final String[] selectColumn = {
                        "_data"
                };
                cursor = mContext.getContentResolver().query(uri, selectColumn, null, null, null);
            }
            if (null == cursor) {
                if (requestCode == REQUEST_CODE_IPMSG_RECORD_AUDIO) {
                    mDstPath = uri.getEncodedPath();
                    mDuration = data.getIntExtra("audio_duration", 0);
                    mDuration = mDuration / 1000 == 0 ? 1 : mDuration / 1000;
                } else {
                    mPhotoFilePath = uri.getEncodedPath();
                    mDstPath = mPhotoFilePath;
                }
                Logger.e(TAG, "cursor is null:" + mDstPath);
                return true;
            }
            if (0 == cursor.getCount()) {
                cursor.close();
                Logger.e(TAG, "getVideoOrPhoto(): take video cursor getcount is 0");
                return false;
            }
            cursor.moveToFirst();
            if (requestCode == REQUEST_CODE_IPMSG_TAKE_PHOTO
                    || requestCode == REQUEST_CODE_IPMSG_CHOOSE_PHOTO) {
                mPhotoFilePath = cursor.getString(cursor.getColumnIndex("_data"));
                mDstPath = mPhotoFilePath;
            } else {
                mDstPath = cursor.getString(cursor.getColumnIndex("_data"));
                isOk = initMediaDuration(uri);
            }
            if (null != cursor && !cursor.isClosed()) {
                cursor.close();
            }
            Logger.v(TAG, "mDstPath:" + mDstPath + ": mDuration:" + mDuration);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return false;
        }
        return isOk;
    }

    private boolean getAudio(Intent data) {
        Uri uri = (Uri) data.getParcelableExtra(RingtoneManager.EXTRA_RINGTONE_PICKED_URI);
        if (Settings.System.getUriFor(Settings.System.RINGTONE).equals(uri)) {
            return false;
        }
        if (null == uri) {
            uri = data.getData();
        }
        if (null == uri) {
            Logger.e(TAG, "getAudio(): choose audio failed, uri is null");
            return false;
        }
        final String scheme = uri.getScheme();
        if (scheme.equals("file")) {
            mDstPath = uri.getEncodedPath();
        } else {
            ContentResolver cr = mContext.getContentResolver();
            Cursor c = cr.query(uri, null, null, null, null);
            c.moveToFirst();
            mDstPath = c.getString(c.getColumnIndexOrThrow(Audio.Media.DATA));
            c.close();
        }

        if (!IpMessageUtils.isAudio(mDstPath)) {
            Toast.makeText(
                    mContext,
                    IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_invalid_file_type), Toast.LENGTH_SHORT)
                    .show();
            return false;
        }

        if (!IpMessageServiceMananger.getInstance(mContext).isFeatureSupported(
                FeatureId.FILE_TRANSACTION)
                && !IpMessageUtils.isFileStatusOk(mContext, mDstPath)) {
            Logger.e(TAG, "getAudio(): choose audio failed, invalid file");
            return false;
        }

        return initMediaDuration(uri);
    }

    private void sendImage(int requestCode) {
        IpImageMessage msg = new IpImageMessage();
        msg.setType(IpMessageType.PICTURE);
        msg.setPath(mDstPath);
        mIpMessageForSend = msg;
        Logger.v(TAG, "sendImage(): start send message.");
        sendMessageForIpMsg(msg, false, false);
    }

    private void sendUnknownMsg(String path) {
        IpAttachMessage msg = new IpAttachMessage();
        msg.setPath(path);
        msg.setType(IpMessageType.UNKNOWN_FILE);
        mIpMessageForSend = msg;
        sendMessageForIpMsg(mIpMessageForSend, false, false);
    }

    public void getCalendar(Context context, String calendar) {
        Uri calendarUri = Uri.parse(calendar);
        InputStream is = null;
        OutputStream os = null;
        Cursor cursor = mContext.getContentResolver().query(calendarUri, null, null, null, null);
        if (null != cursor) {
            if (0 == cursor.getCount()) {
                Logger.e(TAG, "getCalendar(): take calendar cursor getcount is 0");
            } else {
                cursor.moveToFirst();
                mCalendarSummary = cursor.getString(0);
                if (mCalendarSummary != null) {
                    int sub = mCalendarSummary.lastIndexOf(".");
                    mCalendarSummary = mCalendarSummary.substring(0, sub);
                }
            }
            cursor.close();

            String fileName = System.currentTimeMillis() + ".vcs";
            mDstPath = IpMmsConfig.getVcalendarTempPath(mContext) + File.separator + fileName;

            File file = new File(mDstPath);
            if (!file.getParentFile().exists()) {
                file.getParentFile().mkdirs();
            }
            file.delete();
            try {
                if (!file.createNewFile()) {
                    return;
                }
            } catch (IOException e) {
                Logger.e(TAG, "getCalendar()");
                return;
            }
            try {
                is = mContext.getContentResolver().openInputStream(calendarUri);
                os = new BufferedOutputStream(new FileOutputStream(file));
            } catch (FileNotFoundException e) {
                Logger.e(TAG, "getCalendar()");
            }
            byte[] buffer = new byte[256];
            try {
                for (int len = 0; (len = is.read(buffer)) != -1;) {
                    os.write(buffer, 0, len);
                }
                is.close();
                os.close();
            } catch (IOException e) {
                Logger.e(TAG, "getCalendar()");
            }
        }
    }

    private String getNameViaContactId(long contactId) {
        if (contactId <= 0) {
            Logger.w(TAG, "getNameViaContactId(): contactId is invalid!");
            return null;
        }

        String displayName = "";

        Cursor cursor = mContext.getContentResolver().query(Contacts.CONTENT_URI, new String[] {
            Contacts.DISPLAY_NAME
        }, Contacts._ID + "=?", new String[] {
            String.valueOf(contactId)
        }, null);
        if (cursor != null && cursor.moveToFirst()) {
            displayName = cursor.getString(cursor.getColumnIndex(Contacts.DISPLAY_NAME));
        }
        if (cursor != null) {
            cursor.close();
        }

        return displayName == null ? "" : displayName;
    }

    private boolean mSendSmsInJoynMode = false;

    private boolean mSendJoynMsgInSmsMode = false;

    @Override
    public boolean onIpMsgOptionsItemSelected(final IIpConversationExt ipConv,
            MenuItem item, long threadId) {
        Logger.d(TAG, "onIpMsgOptionsItemSelected():" + item.getItemId());
        switch (item.getItemId()) {
            case MENU_INVITE_FRIENDS_TO_CHAT:
                if (mCurrentChatMode == IpMessageConsts.ChatMode.XMS) {
                    if (IpMessageServiceMananger.getInstance(mContext).isFeatureSupported(
                            IpMessageConsts.FeatureId.EXTEND_GROUP_CHAT)) {
                        int curCount = getRecipientSize();
                        int pickCount = RECIPIENTS_LIMIT_FOR_SMS - curCount;
                        mCallback.addIpContacts(pickCount, REQUEST_CODE_IP_MSG_PICK_CONTACTS);
                        return true;
                    }
                }
                Intent intent = new Intent(RemoteActivities.CONTACT);
                intent.putExtra(RemoteActivities.KEY_REQUEST_CODE,
                        REQUEST_CODE_INVITE_FRIENDS_TO_CHAT);
                intent.putExtra(RemoteActivities.KEY_TYPE, SelectContactType.IP_MESSAGE_USER);
                String[] originalContacts = {getNumber()};
                intent.putExtra(RemoteActivities.KEY_ARRAY, originalContacts);
                IpMessageUtils.startRemoteActivityForResult(mContext, intent);
                return true;
            case MENU_SEND_BY_SMS_IN_JOYN_MODE:
                mSendSmsInJoynMode = true;
                mSendJoynMsgInSmsMode = false;
                mCallback.onIpClick(mSendButtonIpMessage);
                return true;
            case MENU_SEND_BY_JOYN_IN_SMS_MODE:
                mSendSmsInJoynMode = false;
                mSendJoynMsgInSmsMode = true;
                mCallback.onIpClick(mSendButtonIpMessage);
                return true;
            case MENU_MARK_AS_SPAM:
                final boolean isSpamFromMark = ((RcseConversation)ipConv).isSpam();
                ((RcseConversation)ipConv).setSpam(true);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        String numbers = getNumber();
                        int contactId = IpMessageContactManager.getInstance(mContext)
                                .getContactIdByNumber(numbers);
                        int[] contactIds = {
                            contactId
                        };
                        if (!IpMessageContactManager.getInstance(mContext)
                                .addContactToSpamList(contactIds)) {
                            Logger.w(TAG,
                                    "onIpMsgOptionsItemSelected(): Mark as spam failed!");
                            ((RcseConversation)ipConv).setSpam(isSpamFromMark);
                        }
                    }
                }).start();
                return true;
            case MENU_REMOVE_SPAM:
                final boolean isSpamFromRemove = ((RcseConversation)ipConv).isSpam();
                ((RcseConversation)ipConv).setSpam(false);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        String number = getNumber();
                        int contactId = IpMessageContactManager.getInstance(mContext)
                                .getContactIdByNumber(number);
                        int[] contactIds = {
                            contactId
                        };
                        if (!IpMessageContactManager.getInstance(mContext)
                                .deleteContactFromSpamList(contactIds)) {
                            Logger.w(TAG, "onIpMsgOptionsItemSelected(): Remove spam failed!");
                            ((RcseConversation)ipConv).setSpam(isSpamFromRemove);
                        }
                    }
                }).start();
                Toast.makeText(mContext, "Removed From Spam", Toast.LENGTH_LONG).show();
                return true;
            // / M: Add Jump to another chat in converaged inbox mode. @{
            case MENU_JUMP_TO_JOYN:
                Logger.v(TAG, "onIpMsgOptionsItemSelected() MENU_JUMP_TO_JOYN");
                if (IpMessageServiceMananger.getInstance(mContext)
                        .getIntegrationMode() == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED) {
                    mIsIpServiceEnabled = true;
                    mCurrentChatMode = IpMessageConsts.ChatMode.JOYN;
                    mCallback.showIpOrMmsSendButton(false);
                    mCallback.invalidateIpOptionsMenu();
                    mCallback.resetSharePanel();
                } else if (IpMessageServiceMananger.getInstance(mContext)
                        .getIntegrationMode() == IpMessageConsts.IntegrationMode.CONVERGED_INBOX) {
                    mCurrentChatMode = IpMessageConsts.ChatMode.JOYN;
                    jumpToJoynChat(true, 0);
                }
                mCallback.updateTextEditorHintCallback();
                return true;
            case MENU_JUMP_TO_XMS:
                Logger.v(TAG, "onIpMsgOptionsItemSelected() MENU_JUMP_TO_XMS");
                if (IpMessageServiceMananger.getInstance(mContext)
                        .getIntegrationMode() == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED) {
                    mIsIpServiceEnabled = false;
                    mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
                    mCallback.showIpOrMmsSendButton(false);
                    mCallback.invalidateIpOptionsMenu();
                    mCallback.resetSharePanel();
                } else if (IpMessageServiceMananger.getInstance(mContext)
                        .getIntegrationMode() == IpMessageConsts.IntegrationMode.CONVERGED_INBOX) {
                    mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
                    jumpToJoynChat(false, 0);
                }
                mCallback.updateTextEditorHintCallback();
                return true;
            // / @}

            // / M: Add export chat fuature. @{
            case MENU_EXPORT_CHAT:
                IpMessageChatManger.getInstance(mContext).exportChat(threadId);
                return true;
            // / @}
            default:
                return false;

        }
    }

    private void showIpMessageConvertToMmsOrSmsDialog(int mode, int convertReason) {
        String message = "";
        if (convertReason == SERVICE_IS_NOT_ENABLED) {
            message = mode == SMS_CONVERT ? IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(
                    IpMessageConsts.string.ipmsg_convert_to_sms_for_service) : IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_convert_to_mms_for_service);
        } else if (convertReason == RECIPIENTS_ARE_NOT_IP_MESSAGE_USER) {
            message = mode == SMS_CONVERT ? IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(
                    IpMessageConsts.string.ipmsg_convert_to_sms_for_recipients) : IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_convert_to_mms_for_recipients);
        }
        // / M: Fix ipmessage bug for ALPS01674002 @{
        else if (convertReason == RECIPIENTS_IP_MESSAGE_NOT_SENDABLE) {
            message = mode == SMS_CONVERT ? IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(
                    IpMessageConsts.string.ipmsg_ip_msg_not_sendable_to_sms) : IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_ip_msg_not_sendable_to_mms);
        }
        // / @}
        new AlertDialog.Builder(mContext)
                .setTitle(
                        mode == SMS_CONVERT ? IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(
                                IpMessageConsts.string.ipmsg_convert_to_sms) : IpMessageResourceMananger.getInstance(mContext).getSingleString(
                                        IpMessageConsts.string.ipmsg_convert_to_mms))
                .setMessage(message)
                .setPositiveButton(
                        IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(IpMessageConsts.string.ipmsg_continue),
                        new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                if (mIpMessageDraft != null) {
                                    convertIpMessageToMmsOrSms(true);
                                }
                                if (mIpMessageForSend != null) {
                                    if (mIpMessageForSend.getType() == IpMessageType.TEXT) {
                                        Logger.e(TAG,
                                                "convertIpMessageToMmsOrSms(): convert IP message to SMS.");
                                        mIpMessageForSend = null;
                                        mCallback.callbackCheckConditionsAndSendMessage(true);
                                    } else {
                                        mIpMessageDraft = mIpMessageForSend;
                                        mIpMessageForSend = null;
                                        convertIpMessageToMmsOrSms(true);
                                    }
                                }
                                dialog.dismiss();
                            }
                        }).setNegativeButton(android.R.string.cancel, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Logger.v(TAG, "showIpMessageConvertToMmsOrSmsDialog(): cancel.");
                        if (mIpMessageForSend != null) {
                            mIpMessageForSend = null;
                        } else {
                            saveIpMessageForAWhile(mIpMessageDraft);
                        }
                        mCallback.callbackUpdateSendButtonState();
                        mCallback.callbackUpdateButtonState(true);
                        dialog.dismiss();
                    }
                }).setOnKeyListener(new DialogInterface.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK) {
                            mCallback.callbackUpdateSendButtonState();
                        }
                        return false;
                    }
                }).show();
    }

    /**
     * M: for checking which type IpMessage can be converted to.
     *
     * @param ipMessage
     * @return
     */
    private int canConvertIpMessageToMessage(IpMessage ipMessage) {
        if (ipMessage == null) {
            return MESSAGETYPE_UNSUPPORT;
        }
        switch (ipMessage.getType()) {
            case IpMessageType.TEXT:
                return MESSAGETYPE_TEXT;
            case IpMessageType.PICTURE:
            case IpMessageType.VOICE:
            case IpMessageType.VCARD:
            case IpMessageType.VIDEO:
                return MESSAGETYPE_MMS;
            case IpMessageType.UNKNOWN_FILE:
            case IpMessageType.COUNT:
            case IpMessageType.GROUP_CREATE_CFG:
            case IpMessageType.GROUP_ADD_CFG:
            case IpMessageType.GROUP_QUIT_CFG:
                return MESSAGETYPE_UNSUPPORT;
            default:
                return MESSAGETYPE_UNSUPPORT;
        }
    }

    private boolean convertIpMessageToMmsOrSms(boolean isAppend) {
        Logger.v(TAG,
                "convertIpMessageToMmsOrSms(): IP message type = " + mIpMessageDraft.getType());
        int type = mIpMessageDraft.getType();
        String path = null;
        String text = null;
        int subId = -1;
        switch (mIpMessageDraft.getType()) {
            case IpMessageType.TEXT:
                Logger.v(TAG, "convertIpMessageToMmsOrSms(): convert to SMS.");
                IpTextMessage textMessage = (IpTextMessage) mIpMessageDraft;
                text = textMessage.getBody();
                break;
            case IpMessageType.PICTURE:
                Logger.v(TAG, "convertIpMessageToMmsOrSms(): convert to MMS from image.");
                IpImageMessage imageMessage = (IpImageMessage) mIpMessageDraft;
                Logger.v(TAG,
                        "convertIpMessageToMmsOrSms(): imagePath = " + imageMessage.getPath());
                text = imageMessage.getCaption();
                path = imageMessage.getPath();
                break;
            case IpMessageType.VOICE:
                Logger.v(TAG, "convertIpMessageToMmsOrSms(): convert to MMS from voice.");
                IpVoiceMessage voiceMessage = (IpVoiceMessage) mIpMessageDraft;
                text = voiceMessage.getCaption();
                path = voiceMessage.getPath();
                break;
            case IpMessageType.VCARD:
                Logger.v(TAG, "convertIpMessageToMmsOrSms(): convert to MMS from vCard.");
                IpVCardMessage vCardMessage = (IpVCardMessage) mIpMessageDraft;
                path = vCardMessage.getPath();
                break;
            case IpMessageType.VIDEO:
                Logger.v(TAG, "convertIpMessageToMmsOrSms(): convert to MMS from video.");
                IpVideoMessage videoMessage = (IpVideoMessage) mIpMessageDraft;
                text = videoMessage.getCaption();
                path = videoMessage.getPath();
                break;
            case IpMessageType.CALENDAR:
                Logger.v(TAG, "convertIpMessageToMmsOrSms(): convert to MMS from vCalendar.");
                IpVCalendarMessage vCalendarMessage = (IpVCalendarMessage) mIpMessageDraft;
                path = vCalendarMessage.getPath();
                break;
            case IpMessageType.UNKNOWN_FILE:
            case IpMessageType.COUNT:
                Logger.w(TAG,
                        "convertIpMessageToMmsOrSms(): Unknown IP message type. type = "
                                + mIpMessageDraft.getType());
                return false;
            case IpMessageType.GROUP_CREATE_CFG:
            case IpMessageType.GROUP_ADD_CFG:
            case IpMessageType.GROUP_QUIT_CFG:
                // / M: group chat type
                Logger.w(TAG, "convertIpMessageToMmsOrSms(): Group IP message type. type = "
                        + mIpMessageDraft.getType());
                return false;
            default:
                Logger.w(TAG, "convertIpMessageToMmsOrSms(): Error IP message type. type = "
                        + mIpMessageDraft.getType());
                return false;
        }
        subId = mIpMessageDraft.getSimId();
        mCallback.convertIpMessageToMmsOrSms(type, isAppend, path, text, subId);
        mIpMessageDraftId = 0;
        clearIpMessageDraft();
        return true;
    }

    private void saveMsgInSDCard(IpAttachMessage ipAttachMessage) {
        if (!IpMessageUtils.getSDCardStatus()) {
            IpMessageUtils.createLoseSDCardNotice(
                    mContext,
                    IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_cant_save));
            return;
        }

        long availableSpace = IpMessageUtils.getSDcardAvailableSpace();
        int size = ipAttachMessage.getSize();

        if (availableSpace <= IpMessageUtils.SDCARD_SIZE_RESERVED || availableSpace <= size) {
            String tips = IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_sdcard_space_not_enough);
            Toast.makeText(mContext, tips, Toast.LENGTH_LONG).show();
            return;
        }

        String source = ipAttachMessage.getPath();
        if (TextUtils.isEmpty(source)) {
            Logger.e(TAG, "saveMsgInSDCard(): save ipattachmessage failed, source empty!");
            Toast.makeText(mContext, mContext.getString(R.string.copy_to_sdcard_fail), Toast.LENGTH_SHORT)
                    .show();
            return;
        }

        String attName = source.substring(source.lastIndexOf("/") + 1);
        String dstFile = "";
        dstFile = IpMessageUtils.getCachePath(mContext) + attName;
        int i = 1;
        while (IpMessageUtils.isExistsFile(dstFile)) {
            dstFile = IpMessageUtils.getCachePath(mContext) + "(" + i + ")" + attName;
            i++;
        }
        IpMessageUtils.copy(source, dstFile);
        String saveSuccess = String.format(IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(
                        IpMessageConsts.string.ipmsg_save_file), dstFile);
        Toast.makeText(mContext, saveSuccess, Toast.LENGTH_SHORT).show();
    }

    private void openMedia(long msgId, IpAttachMessage ipAttachMessage) {
        if (ipAttachMessage == null) {
            Logger.e(TAG, "openMedia(): ipAttachMessage is null!");
            return;
        }
        Logger.v(TAG, "openMedia(): ipAttachMessage type = " + ipAttachMessage.getType());

        if (ipAttachMessage.getType() == IpMessageType.VCARD) {
            IpVCardMessage msg = (IpVCardMessage) ipAttachMessage;
            if (TextUtils.isEmpty(msg.getPath())) {
                Logger.e(TAG, "openMedia(): open vCard failed.");
                return;
            }
            if (!IpMessageUtils.getSDCardStatus()) {
                IpMessageUtils.createLoseSDCardNotice(
                        mContext,
                        IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(IpMessageConsts.string.ipmsg_cant_share));
                return;
            }
            if (getAvailableBytesInFileSystemAtGivenRoot(StorageManagerEx.getDefaultPath()) < msg
                    .getSize()) {
                Toast.makeText(mContext, mContext.getString(R.string.export_disk_problem), Toast.LENGTH_LONG)
                        .show();
            }
            String dest = IpMessageUtils.getCachePath(mContext) + "temp"
                    + msg.getPath().substring(msg.getPath().lastIndexOf(".vcf"));
            IpMessageUtils.copy(msg.getPath(), dest);
            File vcardFile = new File(dest);
            Uri vcardUri = Uri.fromFile(vcardFile);
            Logger.d(TAG, "openMedia(): vcardUri = " + vcardUri);
            Intent i = new Intent();
            i.setAction(android.content.Intent.ACTION_VIEW);
            i.setDataAndType(vcardUri, "text/x-vcard");
            i.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            //i.setFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            mContext.startActivity(i);
        } else if (ipAttachMessage.getType() == IpMessageType.CALENDAR) {
            IpVCalendarMessage msg = (IpVCalendarMessage) ipAttachMessage;
            if (TextUtils.isEmpty(msg.getPath())) {
                Logger.e(TAG, "openMedia(): open vCalendar failed.");
                return;
            }
            if (!IpMessageUtils.getSDCardStatus()) {
                IpMessageUtils.createLoseSDCardNotice(
                        mContext,
                        IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(IpMessageConsts.string.ipmsg_cant_share));
                return;
            }
            if (getAvailableBytesInFileSystemAtGivenRoot(StorageManagerEx.getDefaultPath()) < msg
                    .getSize()) {
                Toast.makeText(mContext, mContext.getString(R.string.export_disk_problem), Toast.LENGTH_LONG)
                        .show();
            }
            String dest = IpMessageUtils.getCachePath(mContext) + "temp"
                    + msg.getPath().substring(msg.getPath().lastIndexOf(".vcs"));
            IpMessageUtils.copy(msg.getPath(), dest);
            File calendarFile = new File(dest);
            Uri calendarUri = Uri.fromFile(calendarFile);
            Intent intent = new Intent();
            intent.setAction(android.content.Intent.ACTION_VIEW);
            intent.setDataAndType(calendarUri, "text/x-vcalendar");
            intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            try {
                mContext.startActivity(intent);
            } catch (ActivityNotFoundException e) {
                Logger.e(TAG, "can't open calendar");
            }
        } else {
            Intent intent = new Intent(RemoteActivities.MEDIA_DETAIL);
            intent.putExtra(RemoteActivities.KEY_MESSAGE_ID, msgId);
            IpMessageUtils.startRemoteActivity(mContext, intent);
        }
    }

    private class InviteFriendsToIpMsgListener implements OnClickListener {
        public void onClick(DialogInterface dialog, int whichButton) {
            mTextEditor.setText(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_invite_friends_content));

        }
    }

    /**
     * M:
     *
     * @param ipMessage
     */
    private void shareIpMsg(IpMessage ipMessage) {
        if (null == ipMessage) {
            Logger.v(TAG, "shareIpMsg(): message item is null!");
            return;
        }
        if (ipMessage instanceof IpAttachMessage) {
            if (!IpMessageUtils.getSDCardStatus()) {
                IpMessageUtils.createLoseSDCardNotice(
                        mContext,
                        IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(IpMessageConsts.string.ipmsg_cant_share));
                return;
            }
        }
        Intent intent = new Intent(Intent.ACTION_SEND);
        intent = setIntent(intent, ipMessage);
        intent.putExtra(Intent.EXTRA_SUBJECT, IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(IpMessageConsts.string.ipmsg_logo));
        try {
            mContext.startActivity(Intent.createChooser(
                    intent,
                    IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_share_title)));
        } catch (Exception e) {
            Logger.v(TAG, "shareIpMsg(): Exception:" + e.toString());
        }
    }

    private Intent setIntent(Intent intent, IpMessage ipMessage) {
        if (ipMessage.getType() == IpMessageType.TEXT) {
            intent.setType("text/plain");
            intent.putExtra(Intent.EXTRA_TEXT, ((IpTextMessage) ipMessage).getBody());
        } else if (ipMessage.getType() == IpMessageType.PICTURE) {
            IpImageMessage msg = (IpImageMessage) ipMessage;
            int index = msg.getPath().lastIndexOf(".");
            String end = msg.getPath().substring(index);
            String dest = IpMessageUtils.getCachePath(mContext) + "temp" + end;
            IpMessageUtils.copy(msg.getPath(), dest);
            intent.setType("image/*");
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.putExtra(Intent.EXTRA_STREAM, u);
            if (!TextUtils.isEmpty(msg.getCaption())) {
                intent.putExtra(SMS_BODY, msg.getCaption());
                intent.putExtra(Intent.EXTRA_TEXT, msg.getCaption());
            }
        } else if (ipMessage.getType() == IpMessageType.VOICE) {
            IpVoiceMessage msg = (IpVoiceMessage) ipMessage;
            int index = msg.getPath().lastIndexOf("/");
            String name = msg.getPath().substring(index);
            String dest = IpMessageUtils.getCachePath(mContext) + name;
            IpMessageUtils.copy(msg.getPath(), dest);
            intent.setType("audio/*");
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.putExtra(Intent.EXTRA_STREAM, u);
            if (!TextUtils.isEmpty(msg.getCaption())) {
                intent.putExtra(SMS_BODY, msg.getCaption());
                intent.putExtra(Intent.EXTRA_TEXT, msg.getCaption());
            }
        } else if (ipMessage.getType() == IpMessageType.VCARD) {
            IpVCardMessage msg = (IpVCardMessage) ipMessage;
            int index = msg.getPath().lastIndexOf("/");
            String name = msg.getPath().substring(index);
            String dest = IpMessageUtils.getCachePath(mContext) + name;
            IpMessageUtils.copy(msg.getPath(), dest);
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.setDataAndType(u, "text/x-vcard");
            intent.putExtra(Intent.EXTRA_STREAM, u);
        } else if (ipMessage.getType() == IpMessageType.VIDEO) {
            IpVideoMessage msg = (IpVideoMessage) ipMessage;
            int index = msg.getPath().lastIndexOf("/");
            String name = msg.getPath().substring(index);
            String dest = IpMessageUtils.getCachePath(mContext) + name;
            IpMessageUtils.copy(msg.getPath(), dest);
            intent.setType("video/*");
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.putExtra(Intent.EXTRA_STREAM, u);
            if (!TextUtils.isEmpty(msg.getCaption())) {
                intent.putExtra(SMS_BODY, msg.getCaption());
                intent.putExtra(Intent.EXTRA_TEXT, msg.getCaption());
            }
        } else if (ipMessage.getType() == IpMessageType.CALENDAR) {
            IpVCalendarMessage msg = (IpVCalendarMessage) ipMessage;
            int index = msg.getPath().lastIndexOf("/");
            String name = msg.getPath().substring(index);
            String dest = IpMessageUtils.getCachePath(mContext) + name;
            IpMessageUtils.copy(msg.getPath(), dest);
            File f = new File(dest);
            Uri uri = Uri.fromFile(f);
            intent.setType("text/x-vcalendar");
            intent.putExtra(Intent.EXTRA_STREAM, uri);
        } else {
            intent.setType("unknown");
        }
        return intent;
    }

    /**
     * M:
     *
     * @param ids
     */
    private void showResendConfirmDialg(final long currentMsgId, final long currentSubId,
            final long[][] allFailedIpMsgIds) {
        IpMessage ipMessage = IpMessageManager.getInstance(mContext).getIpMsgInfo(currentMsgId);
        if (ipMessage == null) {
            Logger.e(TAG, "showResendConfirmDialg(): ipMessage is null.");
            return;
        }
        // / M: add for fix ALPS01600871 @{
        if (!mIsSmsEnabled) {
            return;
        }
        // / @}
        String title = "";
        if (ipMessage.getStatus() == IpMessageStatus.FAILED) {
            title = IpMessageResourceMananger.getInstance(mContext).getSingleString(
                    IpMessageConsts.string.ipmsg_failed_title);
        } else {
            title = IpMessageResourceMananger.getInstance(mContext).getSingleString(
                    IpMessageConsts.string.ipmsg_not_delivered_title);
        }
        String sendViaMsg = "";
        if (ipMessage.getType() == IpMessageType.TEXT) {
            sendViaMsg = IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_resend_via_sms);
        } else {
            sendViaMsg = IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_resend_via_mms);
        }
        List<String> buttonList = new ArrayList<String>();
        buttonList.add(IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(IpMessageConsts.string.ipmsg_try_again));
        if (allFailedIpMsgIds != null && allFailedIpMsgIds.length > 1) {
            buttonList.add(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_try_all_again));
        }
        buttonList.add(sendViaMsg);
        final int buttonCount = buttonList.size();
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext).setTitle(title);
        ArrayAdapter<String> resendAdapter = new ArrayAdapter<String>(mContext,
                R.layout.resend_dialog_item, R.id.resend_item, buttonList);
        builder.setAdapter(resendAdapter, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                final int tryAgain = 0;
                final int tryAllAgain = 1;
                final int sendViaMmsOrSms = 2;
                switch (which) {
                    case tryAgain:
                        IpMessageManager.getInstance(mContext)
                                .resendMessage(currentMsgId, (int) currentSubId);
                        break;
                    case tryAllAgain:
                        if (buttonCount == 3) {
                            for (int index = 0; index < allFailedIpMsgIds.length; index++) {
                                IpMessageManager.getInstance(mContext)
                                        .resendMessage(allFailedIpMsgIds[index][0],
                                                (int) allFailedIpMsgIds[index][1]);
                            }
                            break;
                        } else if (buttonCount != 2) {
                            break;
                        } else {
                            // / M: listSize == 2, run case sendViaMmsOrSms.
                            // fall through
                        }
                    case sendViaMmsOrSms:
                        if (mWorkingMessage.requiresIpMms()
                                || !TextUtils.isEmpty(mTextEditor.getText().toString())
                                || mIpMessageDraft != null) {
                            showDiscardCurrentMessageDialog(currentMsgId);
                        } else {
                            sendViaMmsOrSms(currentMsgId);
                        }
                        break;
                    default:
                        break;
                }
            }
        });
        builder.show();

    }

    public String dialIpRecipient(String number) {
        Logger.v(TAG, "dialIpRecipient() entry: number" + number);
        if (number.startsWith("9+++"))
            number = number.substring(4);
        Logger.v(TAG, "dialIpRecipient() exit: number" + number);
        return number;
    }

    private void sendViaMmsOrSms(long currentMsgId) {
         Logger.d(TAG, "sendViaMmsOrSms(): currentMsgId" + currentMsgId);
        final IpMessage ipMessage = IpMessageManager.getInstance(mContext)
                .getIpMsgInfo(currentMsgId);
        if (ipMessage != null) {
            mIpMessageDraft = ipMessage;
            if (convertIpMessageToMmsOrSms(false)) {
                mContext.runOnUiThread(new Runnable() {
                    public void run() {
                        if (mCallback.isIpSubjectEditorVisible()) {
                            mCallback.showIpSubjectEditor(false);
                            mWorkingMessage.setIpSubject(null, true);
                        }
                        mCallback.drawIpBottomPanel();
                        boolean isMms = mWorkingMessage.requiresIpMms();
                        mCallback.showIpOrMmsSendButton(isMms ? true : false);
                        mIsIpServiceEnabled = false;
                        mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
                        mCallback.invalidateIpOptionsMenu();
                        mCallback.resetSharePanel();
                    }
                });
                IpMessageManager.getInstance(mContext).deleteIpMsg(
                        new long[] {
                            currentMsgId
                        }, true);
            }
        }
    }

    private void showDiscardCurrentMessageDialog(final long currentMsgId) {
        new AlertDialog.Builder(mContext)
                .setTitle("Discard")
                .setMessage(
                        IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(
                                        IpMessageConsts.string.ipmsg_resend_discard_message))
                .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                    }
                })
                .setPositiveButton(
                        IpMessageResourceMananger.getInstance(mContext)
                                .getSingleString(IpMessageConsts.string.ipmsg_continue),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                sendViaMmsOrSms(currentMsgId);
                            }
                        }).show();
    }

    /**
     * M:
     *
     * @param threadId
     * @return
     */
    private long[][] getAllFailedIpMsgByThreadId(long threadId) {
        Cursor cursor = mContext.getContentResolver().query(Sms.CONTENT_URI, new String[] {
                Sms._ID, "1"/*Telephony.TextBasedSmsColumns.SUB_ID TODO*/
        }, "thread_id = " + threadId + " and ipmsg_id > 0 and type = " + Sms.MESSAGE_TYPE_FAILED,
                null, null);
        try {
            if (cursor == null) {
                return null;
            }
            long[][] count = new long[cursor.getCount()][2];
            int index = 0;
            while (cursor.moveToNext()) {
                count[index][0] = cursor.getLong(0);
                count[index][1] = cursor.getLong(1);
                index++;
            }
            return count;
        } finally {
            cursor.close();
        }
    }

    private static void toastNoSubCard(Context context) {
        Toast.makeText(
                context,
                IpMessageResourceMananger.getInstance(context).getSingleString(
                        IpMessageConsts.string.ipmsg_no_sim_card), Toast.LENGTH_LONG).show();
    }

    private void doMoreActionForMms(Message msg) {
        int commonAttachmentType = 0;
        Bundle bundle = msg.getData();
        int action = bundle.getInt(SHARE_ACTION);
        switch (action) {
            case IPMSG_TAKE_PHOTO:
                commonAttachmentType = AttachmentTypeSelectorAdapter.TAKE_PICTURE;
                break;

            case IPMSG_RECORD_VIDEO:
                commonAttachmentType = AttachmentTypeSelectorAdapter.RECORD_VIDEO;
                break;

            case IPMSG_SHARE_CONTACT:
                commonAttachmentType = AttachmentTypeSelectorAdapter.ADD_VCARD;
                break;

            case IPMSG_CHOOSE_PHOTO:
                commonAttachmentType = AttachmentTypeSelectorAdapter.ADD_IMAGE;
                break;

            case IPMSG_CHOOSE_VIDEO:
                commonAttachmentType = AttachmentTypeSelectorAdapter.ADD_VIDEO;
                break;

            case IPMSG_RECORD_AUDIO:
                commonAttachmentType = AttachmentTypeSelectorAdapter.RECORD_SOUND;
                break;

            case IPMSG_CHOOSE_AUDIO:
                commonAttachmentType = AttachmentTypeSelectorAdapter.ADD_SOUND;
                break;

            case IPMSG_SHARE_CALENDAR:
                commonAttachmentType = AttachmentTypeSelectorAdapter.ADD_VCALENDAR;
                break;

            case IPMSG_SHARE_SLIDESHOW:
                commonAttachmentType = AttachmentTypeSelectorAdapter.ADD_SLIDESHOW;
                break;

            default:
                Logger.e(TAG, "doMoreActionForMms(): invalid share action type: " + action);
                hideSharePanel();
                return;
        }
        mCallback.addIpAttachment(commonAttachmentType, true);
        hideSharePanel();
    }

    public boolean lockMessage(Context context, IIpMessageItemExt msgItem, boolean locked) {
        if (msgItem == null || !locked) {
            return false;
        }
        RcseMessageItem rcseMsgItem = (RcseMessageItem) msgItem;
        long ipMessageId = rcseMsgItem.mIpMessageId;
        if (ipMessageId > 0) { // / M: add for ipmessage
            Toast.makeText(mContext, "Joyn messages can't be locked", Toast.LENGTH_LONG).show();
            Logger.v(TAG, "lockMessage(): RCS locked " + locked);
            return true;
        }
        Logger.v(TAG, "lockMessage(): Non RCS locked " + locked);
        return false;
    }


    /// sharepanel begin
    public boolean onIpLayoutSizeChanged(boolean isSoftKeyBoardShow) {
        mIsSoftKeyBoardShow = isSoftKeyBoardShow;
        return isSharePanelShow();
    }

    public void onIpRecipientsEditorTouch() {
        showSharePanel(false);
    }

    public void onIpSubjectTextEditorFocusChange(boolean hasFocus) {
        if (hasFocus) {
            showSharePanel(false);
        }
    }

    public void onIpSubjectTextEditorTouch() {
        showSharePanel(false);
    }

    public void onIpTextEditorTouch() {
        if (mShowKeyBoardFromShare) {
            showSharePanel(false);
            updateIpFullScreenTextEditorHeight();
        }
    }

    public void resetIpConfiguration(boolean isLandscapeOld,
            boolean isLandscapeNew, boolean isSoftKeyBoardShow) {
        mIsLandscape = isLandscapeNew;
        if (!isLandscapeNew && isLandscapeOld == isLandscapeNew && isSoftKeyBoardShow) {
            showSharePanel(false);
        }
    }

    public boolean updateIpFullScreenTextEditorHeight() {
        return isSharePanelShow();
    }



    public boolean isSharePanelShow() {
        if (null != mSharePanel && mSharePanel.isShown()) {
            return true;
        }
        return false;
    }

    private void initShareResource() {
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
        mSharePanel.setHandler(mIpMsgHandler);
        showSharePanelOrKeyboard(false, false);
    }

    // Show text message or joyn message in the panel.
    private void showSharePanel(boolean isShow) {
        Log.i(TAG, "showSharePanel isShow is:" + isShow);
        if (null != mSharePanel) {
            if (!mIsSmsEnabled) {
                mSharePanel.setVisibility(View.GONE);
                mShareButton.setClickable(false);
                mShareButton.setImageDrawable(mPluginContext.getResources()
                        .getDrawable(R.drawable.ipmsg_share_disable));
                return;
            }
            if (isShow) {
                mSharePanel.setVisibility(View.VISIBLE);
                mShareButton.setImageDrawable(mPluginContext.getResources()
                        .getDrawable(R.drawable.ipmsg_keyboard));
            } else {
                mSharePanel.setVisibility(View.GONE);
                mShareButton.setImageDrawable(mPluginContext.getResources()
                        .getDrawable(R.drawable.ipmsg_share));
            }
            mShareButton.setClickable(true);
            mShowKeyBoardFromShare = isShow;
        }
    }

    public void showSharePanelOrKeyboard(final boolean isShowShare, final boolean isShowKeyboard) {
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
            mCallback.showKeyBoardCallback(isShowKeyboard);
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
                    mContext.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if (isShowShare) {
                                showSharePanel(isShowShare);
                                if (mIsLandscape) {
                                    mCallback.setTextEditorMaxHeight(
                                            mReferencedTextEditorTwoLinesHeight);
                                } else {
                                    mCallback.setTextEditorMaxHeight(
                                            mReferencedTextEditorFourLinesHeight);
                                }
                            } else {
                                Log.d(TAG, "showSharePanelOrKeyboard(): new thread.");
                                updateIpFullScreenTextEditorHeight();
                            }
                        }
                    });
                }
            }).start();
        } else {
            if (isShowShare && !isShowKeyboard && mIsLandscape) {
                mCallback.showKeyBoardCallback(isShowKeyboard);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        synchronized (mWaitingImeChangedObject) {
                            try {
                                /// M: fix bug ALPS01297085, wait HideSoftKeyBoard longer
                                int waitTime = 100;
                                Log.d(TAG, "showSharePanelOrKeyboard() mIsLandscape: object start wait.");
                                mWaitingImeChangedObject.wait(waitTime);
                                Log.d(TAG, "c(): mIsLandscape object end wait.");
                            } catch (InterruptedException e) {
                                Log.d(TAG, "InterruptedException");
                            }
                        }
                        mContext.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                showSharePanel(isShowShare);
                                mCallback.setTextEditorMaxHeight(
                                        mReferencedTextEditorTwoLinesHeight);
                            }
                        });
                    }
                }).start();
            } else {
                mCallback.showKeyBoardCallback(isShowKeyboard);
                showSharePanel(isShowShare);
                if (isShowShare || isShowKeyboard) {
                    if (mIsLandscape) {
                        mCallback.setTextEditorMaxHeight(
                                mReferencedTextEditorTwoLinesHeight);
                    } else {
                        mCallback.setTextEditorMaxHeight(
                                mReferencedTextEditorFourLinesHeight);
                    }
                } else {
                    Log.d(TAG, "showSharePanelOrKeyboard()");
                    updateIpFullScreenTextEditorHeight();
                }
            }
        }
    }

    public void hideSharePanel() {
        Log.d(TAG, "hideSharePanel()");
        showSharePanelOrKeyboard(false, false);
        updateIpFullScreenTextEditorHeight();
    }

    private void doMmsAction(Message msg) {
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

        mCallback.addIpAttachment(commonAttachmentType, true);
        Log.d(TAG, "attach: addAttachment(commonAttachmentType, true)");
        /// @}

        hideSharePanel();
    }
    /// sharepanel end

//    public Handler mMsgHandler = new Handler() {
//        public void handleMessage(Message msg) {
//            Log.d(TAG, "mMsgHandler handleMessage, msg.what: " + msg.what);
//            switch (msg.what) {
//            case ACTION_SHARE:
//              doMmsAction(msg);
//                break;
//            default:
//                Log.d(TAG, "msg type: " + msg.what + "not handler");
//                break;
//            }
//            super.handleMessage(msg);
//        }
//    };


    private void enableShareButton(boolean enable) {
        Log.d(TAG, "enableShareButton() enable = " + enable);
        if (!enable) {
            showSharePanel(false);
            mShareButton.setImageDrawable(mPluginContext.getResources()
                    .getDrawable(R.drawable.ipmsg_share_disable));
            mShareButton.setClickable(false);
        } else {
            mShareButton.setImageDrawable(mPluginContext.getResources()
                    .getDrawable(R.drawable.ipmsg_share));
            mShareButton.setClickable(true);
        }
    }

    private boolean isViewVisible(View view) {
        return (null != view)
                    && (View.VISIBLE == view.getVisibility());
    }
    /// sharepanel end

    public boolean hasPermission(final String permission) {
        final Context context = MediatekFactory.getApplicationContext();
        final int permissionState = mContext.checkSelfPermission(permission);
        Logger.v(TAG, "hasPermission() : permission = " + permission + " permissionState = " + permissionState);
        return permissionState == PackageManager.PERMISSION_GRANTED;

    }

    private void doMoreAction(Message msg) {
        Logger.v(TAG, "doMoreAction  msg.what: " + msg.what);
        Bundle bundle = msg.getData();
        int action = bundle.getInt(SHARE_ACTION);
        // /M: If recipient is not a user-IPMessage,Send IP multi-media message
        // via MMS/SMS {@
        // / M: Fix ipmessage bug for ALPS01674002
        if ((mWorkingMessage.requiresIpMms() || mCurrentChatMode == IpMessageConsts.ChatMode.XMS || !isCurrentIpmessageSendable())
                && action != IPMSG_SHARE_FILE) {
            Logger.v(TAG, "doMoreActionforMMS  msg.what: " + msg.what);
            doMoreActionForMms(msg);
            return;
        }
        mRequestTimeMillis = SystemClock.elapsedRealtime();
        // /@}
        boolean isNoRecipient = getRecipientSize() == 0;
        switch (action) {
            case IPMSG_TAKE_PHOTO:
                if (isNoRecipient) {
                    toastNoRecipients(mContext);
                    return;
                }
                takePhoto();
                break;

            case IPMSG_RECORD_VIDEO:
                if (isNoRecipient) {
                    toastNoRecipients(mContext);
                    return;
                }
                recordVideo();
                break;

            case IPMSG_SHARE_CONTACT:
                if (isNoRecipient) {
                    toastNoRecipients(mContext);
                    return;
                }
                if(hasPermission(Manifest.permission.READ_CONTACTS)) {
                    Logger.v(TAG, "doMoreAction  hasPermission Contacts True");
                shareContact();
                } else {
                    Logger.v(TAG, "doMoreAction  hasPermission Contacts False");
                    mContext.requestPermissions(new String[] { Manifest.permission.READ_CONTACTS },
                            PERMISSION_REQUEST_CODE_IPMSG_SHARE_CONTACT);
                }

                break;

            case IPMSG_CHOOSE_PHOTO:
                if (isNoRecipient) {
                    toastNoRecipients(mContext);
                    return;
                }
                if(hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)) {
                    Logger.v(TAG, "doMoreAction  hasPermission Photo True");
                choosePhoto();
                } else {
                    Logger.v(TAG, "doMoreAction  hasPermission Photo False");
                    mContext.requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE },
                            PERMISSION_REQUEST_CODE_IPMSG_CHOOSE_PHOTO);
                }
                break;

            case IPMSG_CHOOSE_VIDEO:
                if (isNoRecipient) {
                    toastNoRecipients(mContext);
                    return;
                }
                if(hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)) {
                    Logger.v(TAG, "doMoreAction  hasPermission Video True");
                chooseVideo();
                } else {
                    Logger.v(TAG, "doMoreAction  hasPermission Video False");
                    mContext.requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE },
                            PERMISSION_REQUEST_CODE_IPMSG_CHOOSE_VIDEO);
                }
                break;

            case IPMSG_RECORD_AUDIO:
                if (isNoRecipient) {
                    toastNoRecipients(mContext);
                    return;
                }
                recordAudio();
                break;

            case IPMSG_CHOOSE_AUDIO:
                if (isNoRecipient) {
                    toastNoRecipients(mContext);
                    return;
                }
                if(hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)) {
                    Logger.v(TAG, "doMoreAction  hasPermission Audio True");
                chooseAudio();
                } else {
                    Logger.v(TAG, "doMoreAction  hasPermission Audio False");
                    mContext.requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE },
                            PERMISSION_REQUEST_CODE_IPMSG_CHOOSE_AUDIO);
                }
                break;

            case IPMSG_SHARE_CALENDAR:
                if (isNoRecipient) {
                    toastNoRecipients(mContext);
                    return;
                }
                shareCalendar();
                break;
            case IPMSG_SHARE_FILE:
                Logger.v(TAG, "doMoreAction(): select from file manager: " + action);
                if (isNoRecipient) {
                    toastNoRecipients(mContext);
                    return;
                }
                if(hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)) {
                    Logger.v(TAG, "doMoreAction  hasPermission File True");
                startFileManager();
                } else {
                    Logger.v(TAG, "doMoreAction  hasPermission File False");
                    mContext.requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE },
                            PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE);
                }
                break;

            default:
                Logger.e(TAG, "doMoreAction(): invalid share action type: " + action);
                break;
        }
        hideSharePanel();
    }

     public boolean isNeverGrantedPermission(String permission) {
        return !mContext.shouldShowRequestPermissionRationale(permission);
    }

    public void onIPRequestPermissionsResult(
            final int requestCode, final String permissions[], final int[] grantResults) {
        Logger.v(TAG, "onRequestPermissionsResult requestCode:" + requestCode + "permissions:" + permissions);
        if (grantResults.length <= 0
                || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
            Logger.v(TAG, "onRequestPermissionsResult not granted");
            if(isNeverGrantedPermission(permissions[0])) {
                Toast.makeText(mContext, "Permission denied.You can change them in Settings->Apps.", Toast.LENGTH_LONG).show();
            }
            return;
        } else {
            Logger.v(TAG, "onRequestPermissionsResult granted");
        }
        // Now permissions are granted, start the feature
        if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_TAKE_PHOTO ) {
            takePhoto();
        } else if(requestCode == PERMISSION_REQUEST_CODE_IPMSG_RECORD_VIDEO ) {
            recordVideo();
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_CHOOSE_PHOTO  ) {
            choosePhoto();
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_CHOOSE_VIDEO ) {
            chooseVideo();
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_RECORD_AUDIO ) {
            recordAudio();
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_CHOOSE_AUDIO) {
            chooseAudio();
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_SHARE_VCALENDAR ) {
            shareCalendar();
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE ) {
            startFileManager();
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_SHARE_CONTACT ) {
            shareContact();
        } else if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_RECEIVE_FILE) {

        }
    }

    public void takePhoto() {
        Logger.v(TAG, "takePhoto  entry ");

        Intent imageCaptureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        imageCaptureIntent.putExtra(MediaStore.EXTRA_OUTPUT, TEMP_CONTENT_URI);
        Logger.d(TAG, "takePhoto  uri: " + TEMP_CONTENT_URI);
        try {
            mContext.startActivityForResult(imageCaptureIntent, REQUEST_CODE_IPMSG_TAKE_PHOTO);
        } catch (Exception e) {
            Toast.makeText(
                    mContext,
                    IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_no_app), Toast.LENGTH_SHORT).show();
            Logger.e(TAG, "takePhoto() exception" + e.getMessage());
        }
    }

    public void choosePhoto() {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_PICK);
        intent.setType("image/*");
        String fileName = System.currentTimeMillis() + ".jpg";
        mPhotoFilePath = IpMmsConfig.getPicTempPath(mContext) + File.separator + fileName;
        mDstPath = mPhotoFilePath;
        Logger.v(TAG, "choosePhoto  mDstPath: " + mDstPath);
        try {
            mContext.startActivityForResult(intent, REQUEST_CODE_IPMSG_CHOOSE_PHOTO);
        } catch (Exception e) {
            Toast.makeText(
                    mContext,
                    IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_no_app), Toast.LENGTH_SHORT).show();
            Logger.e(TAG, "choosePhoto()");
        }
    }

    public void recordVideo() {
        Logger.v(TAG, "recordVideo ");
        int durationLimit = IpMessageUtils.getVideoCaptureDurationLimit();
        Intent intent = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 0);
        intent.putExtra(MediaStore.EXTRA_SIZE_LIMIT, (long) 300 * 1024);
        intent.putExtra(MediaStore.EXTRA_DURATION_LIMIT, durationLimit);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, TEMP_VIDEO_URI);
        try {
            mContext.startActivityForResult(intent, REQUEST_CODE_IPMSG_RECORD_VIDEO);
        } catch (Exception e) {
            Toast.makeText(
                    mContext,
                    IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_no_app), Toast.LENGTH_SHORT).show();
            Logger.e(TAG, "recordVideo()");
        }
    }

    public void chooseVideo() {
        Logger.v(TAG, "chooseVideo ");
        Intent intent = new Intent(Intent.ACTION_PICK);
        intent.setType("video/*");
        try {
            mContext.startActivityForResult(intent, REQUEST_CODE_IPMSG_CHOOSE_VIDEO);
        } catch (Exception e) {
            Toast.makeText(
                    mContext,
                    IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_no_app), Toast.LENGTH_SHORT).show();
            Logger.e(TAG, "chooseVideo()");
        }
    }

    public void recordAudio() {
        Logger.v(TAG, "recordAudio ");
        Intent intent = new Intent(RemoteActivities.AUDIO);
        intent.putExtra(RemoteActivities.KEY_REQUEST_CODE, REQUEST_CODE_IPMSG_RECORD_AUDIO);
        intent.putExtra(IpMessageConsts.RemoteActivities.KEY_SIZE, 300L * 1024);
        IpMessageUtils.startRemoteActivityForResult(mContext, intent);
    }

    public void shareContact() {
        Logger.v(TAG, "shareContact ");
        Intent intent = new Intent("mediatek.intent.action.contacts.list.PICKMULTICONTACTS");
        intent.setType(Contacts.CONTENT_TYPE);
        mContext.startActivityForResult(intent, REQUEST_CODE_IPMSG_SHARE_CONTACT);
    }

    public void chooseAudio() {
        Logger.v(TAG, "chooseAudio ");
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(mContext);
        alertBuilder.setTitle(mResource.getString(R.string.add_music));
        String[] items = new String[2];
        items[0] = mResource.getString(R.string.attach_ringtone);
        items[1] = mResource.getString(R.string.attach_sound);
        alertBuilder.setItems(items, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case 0:
                        IpMessageUtils.selectRingtone(mContext,
                                REQUEST_CODE_IPMSG_CHOOSE_AUDIO);
                        break;
                    case 1:
                        if (!Environment.getExternalStorageState()
                                .equals(Environment.MEDIA_MOUNTED)) {
                            Toast.makeText(mContext.getApplicationContext(),
                                    mResource.getString(R.string.Insert_sdcard), Toast.LENGTH_LONG).show();
                            return;
                        }
                        IpMessageUtils.selectAudio(mContext,
                                REQUEST_CODE_IPMSG_CHOOSE_AUDIO);
                        break;
                    default:
                        break;
                }
            }
        });
        alertBuilder.create().show();
    }

    public void shareCalendar() {
        Intent intent = new Intent("mediatek.intent.action.CALENDARCHOICE");
        intent.setType("text/x-vcalendar");
        intent.putExtra("request_type", 0);
        try {
            mContext.startActivityForResult(intent, REQUEST_CODE_IPMSG_SHARE_VCALENDAR);
        } catch (ActivityNotFoundException e) {
            Toast.makeText(
                    mContext,
                    IpMessageResourceMananger.getInstance(mContext).getSingleString(
                            IpMessageConsts.string.ipmsg_no_app), Toast.LENGTH_SHORT).show();
            Logger.e(TAG, "shareCalendar()");
        }
    }

    private Runnable mHideTypingStatusRunnable = new Runnable() {
        @Override
        public void run() {
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                    "notificationsReceived(): hide typing status.");
            if (null != mTypingStatus) {
                mTypingStatus.setVisibility(View.GONE);
            }
        }
    };

    private Runnable mShowTypingStatusRunnable = new Runnable() {
        @Override
        public void run() {
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                    "notificationsReceived(): show Istyping status view.");
            if (null != mTypingStatus) {
                String text = "";
                if (mChatSenderName != null && !TextUtils.isEmpty(mChatSenderName)) {
                    text = mChatSenderName
                            + " is "
                            + IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(IpMessageConsts.string.ipmsg_typing_text);
                } else {
                    Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                            "mChatSenderName is null or empty");
                    return;
                }
                mTypingStatus.setText(text);
                mTypingStatus.setVisibility(View.VISIBLE);
            }
        }
    };

    // / M: Add Jump to another chat in converaged inbox mode. @{
    private void jumpToJoynChat(boolean isToJoynChat, long ipMessageId) {
        String number;
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_SENDTO);
        Logger.v(TAG, "jumpToJoynChat ipMessage = " + ipMessageId + "isToJoynChat:" + isToJoynChat);
        IpMessage ipMessage = IpMessageManager.getInstance(mContext).getIpMsgInfo(ipMessageId);
        if (isToJoynChat) {
            number = IpMessageConsts.JOYN_START + mChatModeNumber;
            intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);
        } else {
            number = mChatModeNumber.substring(4);
            intent.putExtra("chatmode", IpMessageConsts.ChatMode.XMS);
        }

        Uri uri = Uri.parse("smsto: " + number);
        intent.setData(uri);

        if (ipMessage != null) {
            if (ipMessage.getType() == IpMessageType.TEXT) {
                Logger.v(TAG, "jumpToJoynChat ipMessage = TEXT is" + ((IpTextMessage) ipMessage).getBody());
                intent.putExtra("sms_body", ((IpTextMessage) ipMessage).getBody());
            } else if (ipMessage.getType() >= IpMessageType.PICTURE
                    && ipMessage.getType() <= IpMessageType.UNKNOWN_FILE) {
                intent.setAction(Intent.ACTION_SEND);
                intent.setComponent(new ComponentName("com.android.mms",
                        "com.android.mms.ui.ComposeMessageActivity"));
                intent = setIntent(intent, ipMessage);
                Logger.v(TAG, "jumpToJoynChat ipMessage = ACTION_SEND is");
            }
        }
        mContext.startActivity(intent);
    }

    private void asyncIpAttachVCardByContactsId(final Intent data) {
        if (data == null) {
            return;
        }
       // mCallback.runIpAsyncInThreadPool(new Runnable() {
          //  public void run() {
                long[] contactsId = data.getLongArrayExtra("com.mediatek.contacts.list.pickcontactsresult");
                VCardAttachment va = new VCardAttachment(mContext);
                int result = OK;
                mIpMessageVcardName = va.getVCardFileNameByContactsId(contactsId, true);
                mDstPath = IpMessageUtils.getCachePath(mContext)
                        + mIpMessageVcardName;
                Logger.d(TAG, "asyncIpAttachVCardByContactsId(): mIpMessageVcardName = "
                        + mIpMessageVcardName + ", mDstPath = " + mDstPath);
                mIpMsgHandler.postDelayed(mSendVcard, 100);
           // }
       // }, null, R.string.adding_attachments_title);
    }

    private static void toastNoRecipients(Context context) {
        Toast.makeText(
                context,
                IpMessageResourceMananger.getInstance(context)
            .getSingleString(IpMessageConsts.string.ipmsg_need_input_recipients), Toast.LENGTH_SHORT).show();
    }

    public static Intent createIntent(Context context, long threadId) {
        Intent intent = new Intent();
        intent.setComponent(new ComponentName("com.android.mms",
                "com.android.mms.ui.ComposeMessageActivity"));
        if (threadId > 0) {
            intent.setData(getUri(threadId));
        }

        return intent;
    }

    public static Uri getUri(long threadId) {
        return ContentUris.withAppendedId(Threads.CONTENT_URI, threadId);
    }

    public static long getAvailableBytesInFileSystemAtGivenRoot(String rootFilePath) {
        StatFs stat = new StatFs(rootFilePath);
        // final long totalBlocks = stat.getBlockCount();
        // put a bit of margin (in case creating the file grows the system by a
        // few blocks)
        final long availableBlocks = stat.getAvailableBlocks() - 128;

        // long mTotalSize = totalBlocks * stat.getBlockSize();
        long mAvailSize = availableBlocks * stat.getBlockSize();

        Logger.i(TAG, "getAvailableBytesInFileSystemAtGivenRoot(): "
                + "available space (in bytes) in filesystem rooted at: " + rootFilePath + " is: "
                + mAvailSize);
        return mAvailSize;
    }

    private boolean isRecipientsEditorVisible() {
        if (mCallback.getRecipientsEditorInfo() == null) {
            Logger.d(TAG, "isRecipientsEditorVisible, false");
            return false;
        } else {
            Logger.d(TAG, "isRecipientsEditorVisible, true");
            return true;
        }
    }

    private String getNumber() {
        String[] numbers = mCallback.getConversationInfo();
        if (numbers != null && numbers.length > 0) {
            return numbers[0];
        }
        return "";
    }

    private int getRecipientSize() {
        String[] numbers = mCallback.getConversationInfo();
        if (numbers != null && numbers.length > 0) {
            return numbers.length;
        }
        return 0;
    }

    private void hideInputMethod() {
        Logger.v(TAG, "hideInputMethod()");
        if (mContext.getWindow() != null && mContext.getWindow().getCurrentFocus() != null) {
            InputMethodManager inputMethodManager = (InputMethodManager) mContext
                    .getSystemService(Context.INPUT_METHOD_SERVICE);
            inputMethodManager.hideSoftInputFromWindow(mContext.getWindow().getCurrentFocus()
                    .getWindowToken(), 0);
        }
    }

    private boolean initMediaDuration(Uri uri) {
        if (uri == null) {
            throw new IllegalArgumentException("Uri may not be null.");
        }

        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        int duration = 0;
        try {
            retriever.setDataSource(mContext, uri);
            String dur = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
            if (dur != null) {
                duration = Integer.parseInt(dur);
            }
            mDuration = duration;
            mDuration = mDuration / 1000 == 0 ? 1 : mDuration / 1000;
            Logger.v(TAG, "Got video duration:" + duration);
        } catch (Exception ex) {
            Logger.e(TAG, "MediaMetadataRetriever failed to get duration for " + ex);
            return false;
        } finally {
            retriever.release();
        }
        return true;
    }

    public void log(String msg) {
        if (Build.IS_ENG || DEBUG) {
            Logger.d(TAG, msg);
        } else {
            Logger.v(TAG, msg);
        }
    }

}
