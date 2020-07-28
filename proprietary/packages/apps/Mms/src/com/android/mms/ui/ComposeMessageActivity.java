/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mms.ui;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.StatusBarManager;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.MergeCursor;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.drm.DrmStore;
import android.graphics.drawable.Drawable;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.UserManager;
import android.preference.PreferenceManager;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Intents;
import android.provider.MediaStore;
import android.provider.MediaStore.Audio;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Video;
import android.provider.Settings;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.Editable;
import android.text.InputFilter;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.TextKeyListener;
import android.text.style.URLSpan;
import android.text.util.Linkify;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.view.View.OnKeyListener;
import android.view.View.OnTouchListener;
import android.view.ViewStub;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.webkit.MimeTypeMap;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.QuickContactBadge;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;

import com.android.browser.provider.Browser;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.mms.ExceedMessageSizeException;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.TempFileProvider;
import com.android.mms.data.Contact;
import com.android.mms.data.ContactList;
import com.android.mms.data.Conversation;
import com.android.mms.data.Conversation.ConversationQueryHandler;
import com.android.mms.data.WorkingMessage;
import com.android.mms.data.WorkingMessage.MessageStatusListener;
import com.android.mms.draft.DraftManager;
import com.android.mms.drm.DrmUtils;
import com.android.mms.model.AudioModel;
import com.android.mms.model.MediaModel;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.model.TextModel;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.transaction.SmsReceiver;
import com.android.mms.transaction.TransactionBundle;
import com.android.mms.ui.MessageUtils.ResizeImageResultCallback;
import com.android.mms.ui.RecipientsEditor.RecipientContextMenuInfo;
import com.android.mms.ui.SubSelectDialog.SubClickAndDismissListener;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MessageResource;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.android.mms.util.ThreadCountManager;
import com.android.mms.util.ThumbnailManager;
import com.android.mms.widget.MmsWidgetProvider;
import com.android.mtkex.chips.MTKRecipientEditTextView;
import com.android.mtkex.chips.MTKRecipientEditTextView.ChipProcessListener;
import com.android.mtkex.chips.RecipientEntry;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.android.mms.pdu.MtkSendReq;
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkDefaultSmsSimSettings;
import com.mediatek.internal.telephony.MtkTelephonyProperties;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.callback.IComposeActivityCallback;
import com.mediatek.mms.callback.ITextSizeAdjustHost;
import com.mediatek.mms.callback.IWorkingMessageCallback;
import com.mediatek.mms.ext.IOpComposeExt;
import com.mediatek.mms.ext.IOpWorkingMessageExt;
import com.mediatek.mms.appserviceproxy.AppServiceProxy;
import com.mediatek.mms.ipmessage.IIpComposeActivityExt;
import com.mediatek.mms.ipmessage.IIpConversationExt;
import com.mediatek.mms.model.FileAttachmentModel;
import com.mediatek.mms.ui.VCardAttachment;
import com.mediatek.mms.util.CellConnMgr;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.mms.util.MmsSizeUtils;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.mms.util.VCardUtils;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.GeneralPreferenceActivity;
import com.mediatek.setting.MmsPreferenceActivity;
import com.mediatek.setting.SettingListActivity;
import com.mediatek.storage.StorageManagerEx;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Pattern;

import mediatek.telephony.MtkSmsManager;
import mediatek.telephony.MtkSmsMessage;

import static android.content.res.Configuration.KEYBOARDHIDDEN_NO;
import static com.android.mms.ui.MessageListAdapter.COLUMN_ID;
import static com.android.mms.ui.MessageListAdapter.COLUMN_MSG_TYPE;
import static com.android.mms.ui.MessageListAdapter.PROJECTION;


/**
 * This is the main UI for:
 * 1. Composing a new message;
 * 2. Viewing/managing message history of a conversation.
 *
 * This activity can handle following parameters from the intent
 * by which it's launched.
 * thread_id long Identify the conversation to be viewed. When creating a
 *         new message, this parameter shouldn't be present.
 * msg_uri Uri The message which should be opened for editing in the editor.
 * address String The addresses of the recipients in current conversation.
 * exit_on_sent boolean Exit this activity after the message is sent.
 */
public class ComposeMessageActivity extends Activity
        implements View.OnClickListener, TextView.OnEditorActionListener,
        MessageStatusListener, Contact.UpdateListener,
        ITextSizeAdjustHost, SubClickAndDismissListener {
    public static final int REQUEST_CODE_ATTACH_IMAGE     = 100;
    public static final int REQUEST_CODE_TAKE_PICTURE     = 101;
    public static final int REQUEST_CODE_ATTACH_VIDEO     = 102;
    public static final int REQUEST_CODE_TAKE_VIDEO       = 103;
    public static final int REQUEST_CODE_ATTACH_SOUND     = 104;
    public static final int REQUEST_CODE_RECORD_SOUND     = 105;
    public static final int REQUEST_CODE_CREATE_SLIDESHOW = 106;
    public static final int REQUEST_CODE_ECM_EXIT_DIALOG  = 107;
    public static final int REQUEST_CODE_ADD_CONTACT      = 108;
    public static final int REQUEST_CODE_PICK             = 109;

    /// M: fix bug ALPS00490684, update group mms state from GROUP_PARTICIPANTS to setting @{
    public static final int REQUEST_CODE_GROUP_PARTICIPANTS = 130;
    /// @}
    /// M: fix bug ALPS00448677, update or delete Contact Chip
    public static final int REQUEST_CODE_VIEW_CONTACT     = 111;
    private Contact mInViewContact;
    /// @}

    private static final String TAG = "Mms/compose";

    private static final boolean DEBUG = false;
    private static final boolean TRACE = false;
    private static final boolean LOCAL_LOGV = false;

    // Menu ID
    private static final int MENU_ADD_SUBJECT           = 0;
    private static final int MENU_DELETE_THREAD         = 1;
    private static final int MENU_ADD_ATTACHMENT        = 2;
    private static final int MENU_DISCARD               = 3;
    private static final int MENU_SEND                  = 4;
    private static final int MENU_CALL_RECIPIENT        = 5;
    private static final int MENU_CONVERSATION_LIST     = 6;
    private static final int MENU_DEBUG_DUMP            = 7;

    // Context menu ID
    private static final int MENU_VIEW_CONTACT          = 12;
    private static final int MENU_ADD_TO_CONTACTS       = 13;

    private static final int MENU_EDIT_MESSAGE          = 14;
    private static final int MENU_VIEW_SLIDESHOW        = 16;
    private static final int MENU_VIEW_MESSAGE_DETAILS  = 17;
    private static final int MENU_DELETE_MESSAGE        = 18;
    private static final int MENU_SEARCH                = 19;
    private static final int MENU_DELIVERY_REPORT       = 20;
    private static final int MENU_FORWARD_MESSAGE       = 21;
    private static final int MENU_CALL_BACK             = 22;
    private static final int MENU_SEND_EMAIL            = 23;
    private static final int MENU_COPY_MESSAGE_TEXT     = 24;
    private static final int MENU_COPY_TO_SDCARD        = 25;
    private static final int MENU_ADD_ADDRESS_TO_CONTACTS = 27;
    private static final int MENU_LOCK_MESSAGE          = 28;
    private static final int MENU_UNLOCK_MESSAGE        = 29;
    private static final int MENU_SAVE_RINGTONE         = 30;
    private static final int MENU_PREFERENCES           = 31;
    private static final int MENU_GROUP_PARTICIPANTS    = 32;

    private static final int MENU_SELECT_MESSAGE = 101;

    private static final int MENU_COPY = 207;

    private static final int MESSAGE_LIST_QUERY_TOKEN = 9527;
    private static final int MESSAGE_LIST_QUERY_AFTER_DELETE_TOKEN = 9528;

    private static final int DELETE_MESSAGE_TOKEN  = 9700;

    private static final int CHARS_REMAINING_BEFORE_COUNTER_SHOWN = 10;

    private static final long NO_DATE_FOR_DIALOG = -1L;

    private static final String KEY_EXIT_ON_SENT = "exit_on_sent";
    private static final String KEY_FORWARDED_MESSAGE = "forwarded_message";
    private static final String KEY_APPEND_MESSAGE = "append_attachment";

    private static final String EXIT_ECM_RESULT = "exit_ecm_result";

    // When the conversation has a lot of messages and a new message is sent, the list is scrolled
    // so the user sees the just sent message. If we have to scroll the list more than 20 items,
    // then a scroll shortcut is invoked to move the list near the end before scrolling.
    private static final int MAX_ITEMS_TO_INVOKE_SCROLL_SHORTCUT = 20;

    // Any change in height in the message list view greater than this threshold will not
    // cause a smooth scroll. Instead, we jump the list directly to the desired position.
    private static final int SMOOTH_SCROLL_THRESHOLD = 200;

    // M: Define the SIM lock policy/valid/capability.
    public static final int SIM_LOCK_POLICY_SLOT1_ONLY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT1;
    public static final int SIM_LOCK_POLICY_SLOT2_ONLY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ONLY_SLOT2;
    public static final int SIM_LOCK_POLICY_SLOT_ALL =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL;
    public static final int SIM_SLOT_LOCK_POLICY_LK_SLOT1 =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT1;
    public static final int SIM_SLOT_LOCK_POLICY_LK_SLOT2 =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOT2;
    public static final int SIM_SLOT_LOCK_POLICY_LK_SLOTA =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA;
    public static final int SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_CS =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS;
    public static final int SIM_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE;
    public static final int SIM_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE=
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE;
    public static final int SIM_LOCK_SIM_CAPABILITY_FULL =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_FULL;
    public static final int SIM_LOCK_SIM_CAPABILITY_PS_ONLY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_PS_ONLY;


    private int TOAST_TYPE_FOR_SAVE_DRAFT = 0;
    private int TOAST_TYPE_FOR_SEND_MSG = 1;
    private int TOAST_TYPE_FOR_ATTACH = 2;

    String USING_COLON = "USE_COLON";

    String SELECTION_CONTACT_RESULT = "contactId";
    String NUMBERS_SEPARATOR_COLON = ":";
    String NUMBERS_SEPARATOR_SIMCOLON = ";";
    String NUMBERS_SEPARATOR_COMMA = ",";

    private ContentResolver mContentResolver;

    private BackgroundQueryHandler mBackgroundQueryHandler;

    private Conversation mConversation;     // Conversation we are working in

    private boolean mExitOnSent;            // Should we finish() after sending a message?
                                            // TODO: mExitOnSent is obsolete -- remove
    private boolean mForwardMessageMode;

    private View mTopPanel;                 // View containing the recipient and subject editors
    private View mBottomPanel;              // View containing the text editor, send button, ec.
    private EditText mTextEditor;           // Text editor to type your message into
    private TextView mTextCounter;          // Shows the number of characters used in text editor
    private TextView mSendButtonMms;        // Press to send mms
    private ImageButton mSendButtonSms;     // Press to send sms
    private EditText mSubjectTextEditor;    // Text editor for MMS subject

    private AttachmentEditor mAttachmentEditor;
    private View mAttachmentEditorScrollView;

    private MessageListView mMsgListView;        // ListView for messages in this conversation
    public MessageListAdapter mMsgListAdapter = null;  // and its corresponding ListAdapter
    private RecipientsEditor mRecipientsEditor;  // UI control for editing recipients
    private ImageButton mRecipientsPicker;       // UI control for recipients picker

    private boolean mIsKeyboardOpen;             // Whether the hardware keyboard is visible
    /// M: fix bug ALPS00419856, set TextEditor Height = four when unlock screen @{
    private boolean mIsSoftKeyBoardShow;
    private static final int SOFT_KEY_BOARD_MIN_HEIGHT = 150;
    /// @}
    private boolean mIsLandscape;                // Whether we're in landscape mode

    private boolean mToastForDraftSave;   // Whether to notify the user that a draft is being saved

    private boolean mSentMessage;       // true if the user has sent a message while in this
                                        // activity. On a new compose message case, when the first
                                        // message is sent is a MMS w/ attachment, the list blanks
                                        // for a second before showing the sent message. But we'd
                                        // think the message list is empty, thus show the recipients
                                        // editor thinking it's a draft message. This flag should
                                        // help clarify the situation.

    private WorkingMessage mWorkingMessage;         // The message currently being composed.

    private AlertDialog mDiscardDraftDialog;

    private boolean mWaitingForSubActivity;
    private int mLastRecipientCount;    // Used for warning the user on too many recipients.
    private AttachmentTypeSelectorAdapter mAttachmentTypeSelectorAdapter;

    private boolean mSendingMessage;    // Indicates the current message is sending, and shouldn't send again.
    private boolean bListUpdated = false; // update the list flag
    private Intent mAddContactIntent;   // Intent used to add a new contact

    private Uri mTempMmsUri;            // Only used as a temporary to hold a slideshow uri
    private long mTempThreadId;         // Only used as a temporary to hold a threadId
    /// M: fix bug for ConversationList select all performance ,update selected threads array.@{
    private long mSelectedThreadId = 0;
    /// @}

    private AsyncDialog mAsyncDialog;   // Used for background tasks.

    private String mDebugRecipients;
    private int mLastSmoothScrollPosition;
    private boolean mScrollOnSend;      // Flag that we need to scroll the list to the end.

    private int mSavedScrollPosition = -1;  // we save the ListView's scroll position in onPause(),
                                            // so we can remember it after re-entering the activity.
                                            // If the value >= 0, then we jump to that line. If the
                                            // value is maxint, then we jump to the end.
    private long mLastMessageId;

    ///M: ALPS00726802, this int save the position in cursor of the clicked item
    private int mClickedItemPosition = -1;

    private boolean  mErrorDialogShown = true;
    /**
     * Whether this activity is currently running (i.e. not paused)
     */
    private boolean mIsRunning;
    // key for extras and icicles
    public static final String THREAD_ID = "thread_id";

    /// M: fix bug ALPS00397146, removeThumbnailManager uri
    // (Content://media/external/images/media/) when it rotated
    private static final HashMap<String, Integer> sDegreeMap = new HashMap<String, Integer>();
    /// @}

    private String mBeforeTextChangeString = "";

    //public static final String SELECTED_ID = "SELECTID";

    private InputMethodManager mInputMethodManager = null;

    ///M: actionbar customer view
    private View mActionBarCustomView;
    private TextView mTopTitle;
    private TextView mTopSubtitle;

    private boolean mHadToSlideShowEditor = false;

    /// M: record cell progress dialog is showing or not
    private boolean mIsCellDialogShowing = false;

    private boolean mForwardingMessage = false;

    /// M: fix bug ALPS01258201, mAsyncTaskNum for count asyncTask number
    //  ShowRunnable can avoid dismiss progressDialog.
    private int mAsyncTaskNum;

    /// M: fix bug ALPS01505548, delete VCard temp file
    private ArrayList<String> mVCardFiles = new ArrayList<String>();

    // add for ipmessage
    public IIpComposeActivityExt mIpCompose;
    public ComposeActivityCallback mIpComposeCallback = new ComposeActivityCallback();

    // add for Op plugin
    public IOpComposeExt mOpComposeExt;

    private ContextMenu mChipViewMenu = null;

    private boolean mIsActivityStoped = true;

    // add for Runtime permission
    private boolean mNeedRequestPermissins = false;
    private Bundle mSavedInstanceState = null;

    // has a intent ACTION_SENDTO from other apps
    public static boolean externalIntent = false;

    /// M: ALPS03562153 @{
    private boolean mCreateFromSendTo = false;
    /// @}

    private static final int RECIPIENTS_MAX_LENGTH = 2000;

    // when query message, return cursor max count is 1000, if bigger than 1000,
    // query multi times, and merge the multi cursors
    private static final int MAX_MESSAGE_PER_QUERY = 1000;
    private int queryOffset = 0;
    private ArrayList<Cursor> queryCursors = new ArrayList<Cursor>();

    // Set query state to QUERY_PENDING if already has a running query
    // then after query return, if query state is QUERY_PENDING, start a new query again
    private static int QUERY_IDLE = 0;
    private static int QUERY_RUNNING = 1;
    private static int QUERY_PENDING = 2;
    private int mQueryingState = QUERY_IDLE;

    private void dismissProgressDialog() {
        Log.d(TAG, "Composer reduce mAsyncTaskNum = " + (--mAsyncTaskNum));
        if (mAsyncTaskNum == 0) {
            runOnUiThread(new Runnable() {
                public void run() {
                    getAsyncDialog().dismissProgressDialog();
                    getAsyncDialog().resetProgressDialog();
                }
            });
        }
    }

    class ShowRunnable implements Runnable {
        public void run() {
        }
    }

    public Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (mIpCompose.handleIpMessage(msg)) {
                super.handleMessage(msg);
                return;
            }
            super.handleMessage(msg);
        }
    };

    @SuppressWarnings("unused")
    public static void log(String logMsg) {
        Thread current = Thread.currentThread();
        long tid = current.getId();
        StackTraceElement[] stack = current.getStackTrace();
        String methodName = stack[3].getMethodName();
        // Prepend current thread ID and name of calling method to the message.
        logMsg = "[" + tid + "] [" + methodName + "] " + logMsg;
        Log.d(TAG, logMsg);
    }

    //==========================================================
    // Inner classes
    //==========================================================

    private void editSlideshow() {
        // The user wants to edit the slideshow. That requires us to persist the slideshow to
        // disk as a PDU in saveAsMms. This code below does that persisting in a background
        // task. If the task takes longer than a half second, a progress dialog is displayed.
        // Once the PDU persisting is done, another runnable on the UI thread get executed to start
        // the SlideshowEditActivity.

        /// M: fix bug ALPS00520531, Do not load draft when compose is going to edit slideshow
//        mContentResolver.unregisterContentObserver(mDraftChangeObserver);

        getAsyncDialog().runAsync(new Runnable() {
            @Override
            public void run() {
                // This runnable gets run in a background thread.
                mTempMmsUri = mWorkingMessage.saveAsMms(false);
            }
        }, new Runnable() {
            @Override
            public void run() {
                // Once the above background thread is complete, this runnable is run
                // on the UI thread.
                if (mTempMmsUri == null) {
                    mWaitingForSubActivity = false;
                    return;
                }
                long threadId = mConversation.getThreadId();
                if (threadId <= 0) {
                    if (!mConversation.getRecipients().isEmpty()) {
                        threadId = mConversation.ensureThreadId();
                    }
                }
                /// M: add for fix ALPS01317511 @{
                mOpComposeExt.editSlideshow(mHadToSlideShowEditor);
                /// @}
                Intent intent = new Intent(ComposeMessageActivity.this,
                        SlideshowEditActivity.class);
                intent.setData(mTempMmsUri);
                intent.putExtra("thread_id", threadId);
                startActivityForResult(intent, REQUEST_CODE_CREATE_SLIDESHOW);
            }
        // M: fix bug ALPS00351027
        }, R.string.sync_mms_to_db);
    }

    private final Handler mAttachmentEditorHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            MmsLog.d(TAG, "mAttachmentEditorHandler, handleMessage = " + msg.what);
            /// M: Code analyze 026, If the two clicks are too close. @{

            long oldTime = mAttachClickTime;
            mAttachClickTime = SystemClock.elapsedRealtime();
            if ((mAttachClickTime - oldTime < 500) && (mAttachClickTime - oldTime > 0)) {
                MmsLog.d(TAG, "mAttachmentEditorHandler, ignore a click if too close");
                return;
            }

            /// M: OP01
            if (mOpComposeExt.handleAttachmentEditorHandlerMessage(msg,
                    mSendButtonCanResponse, mWorkingMessage,
                    mWorkingMessage.getSlideshow(), mCompressingImage)) {
                updateEditorFocus();
                return;
            }
            switch (msg.what) {
                case AttachmentEditor.MSG_EDIT_SLIDESHOW: {
                    /// M: Code analyze 024, If the click operator can be responsed. @{
                    if (mClickCanResponse) {
                        mClickCanResponse = false;
                        /// M: Code analyze 038, If the user is editing slideshow now.
                        /// Do not allow the activity finish but return
                        /// directly when back key is entered. @{
                        mIsEditingSlideshow = true;
                        /// @}
                        editSlideshow();
                    }
                    /// @}
                    break;
                }
                case AttachmentEditor.MSG_SEND_SLIDESHOW: {
                    if (isPreparedForSending()) {
                        /// M: Code analyze 028, Before sending message,check the recipients count
                        /// and add sub card selection dialog if multi sub cards exist.@{
                        // ComposeMessageActivity.this.confirmSendMessageIfNeeded();
                        checkRecipientsCount();
                        /// @}
                    }
                    break;
                }
                case AttachmentEditor.MSG_VIEW_IMAGE:
                case AttachmentEditor.MSG_PLAY_VIDEO:
                case AttachmentEditor.MSG_PLAY_AUDIO:
                case AttachmentEditor.MSG_PLAY_SLIDESHOW:
                    /// M: Code analyze 024, If the click operator can be responsed. @{
                    if (mClickCanResponse) {
                        mClickCanResponse = false;
                        viewMmsMessageAttachment(msg.what);
                    }
                    /// @}
                    /// M: Code analyze 051, Hide input keyboard.@{
                    hideInputMethod();
                    /// @}
                    break;

                case AttachmentEditor.MSG_REPLACE_IMAGE:
                    /// M: @{
                    getSharedPreferences("SetDefaultLayout", 0)
                        .edit().putBoolean("SetDefaultLayout", false).commit();
                    /// @}
                case AttachmentEditor.MSG_REPLACE_VIDEO:
                case AttachmentEditor.MSG_REPLACE_AUDIO:
                    /// M: Code analyze 051, Hide input keyboard.@{
                    hideInputMethod();
                    /// @}
                    showAddAttachmentDialog(false);
                    break;

                case AttachmentEditor.MSG_REMOVE_ATTACHMENT:
                    /// M: fix bug ALPS01538338
                    if (!mSendButtonCanResponse) {
                        MmsLog.d(TAG, "handle MSG_REMOVE_ATTACHMENT return");
                        return;
                    }
                    mWorkingMessage.removeAttachment(true);
                    updateEditorFocus();
                    break;
                default:
                    break;
            }
        }
    };

    private void updateEditorFocus() {
        MmsLog.d(TAG, "updateEditorFocus");
        if (isRecipientsEditorVisible()
                && mRecipientsEditor.isFocused()
                && mRecipientsEditor.hasFocus()) {
            MmsLog.d(TAG, "updateEditorFocus, mRecipientsEditor");
        } else if (mSubjectTextEditor != null
                && mSubjectTextEditor.isFocused()
                && mSubjectTextEditor.hasFocus()) {
            MmsLog.d(TAG, "updateEditorFocus, mSubjectTextEditor");
        } else if ((mBottomPanel != null) && (mBottomPanel.getVisibility() == View.VISIBLE)) {
            MmsLog.d(TAG, "updateEditorFocus, mTextEditor");
            mTextEditor.requestFocus();
        }
    }

    private void viewMmsMessageAttachment(final int requestCode) {
        final SlideshowModel slideshow = mWorkingMessage.getSlideshow();
        if (slideshow == null) {
            /// M: ALPS01846425. If mms has been sent, slideshow is null. @{
//            throw new IllegalStateException("mWorkingMessage.getSlideshow() == null");
            MmsLog.e(TAG, "viewMmsMessageAttachment slideshow is null",
                    new IllegalStateException("mWorkingMessage.getSlideshow() == null"));
            return;
            /// @}
        }
        /// M: Code analyze 035, The audio becomes "simple" slideshow.
        /// Launch the slideshow activity or MmsPlayerActivity to play/view media attachment. @{
        SlideModel slideOne = slideshow.get(0);
        if (slideshow.isSimple() && slideOne != null && !slideOne.hasAudio()) {
            MessageUtils.viewSimpleSlideshow(this, slideshow);
        } else {

            // M: change feature ALPS01751464
            if (slideshow.isSimple() && slideOne != null && slideOne.hasAudio()) {
                MediaModel model = slideOne.getAudio();
                if (model != null && model.hasDrmContent()) {
                    DrmUtilsEx.showDrmAlertDialog(ComposeMessageActivity.this);
                    return;
                }
            }

            // The user wants to view the slideshow. That requires us to persist the slideshow to
            // disk as a PDU in saveAsMms. This code below does that persisting in a background
            // task. If the task takes longer than a half second, a progress dialog is displayed.
            // Once the PDU persisting is done, another runnable on the UI thread get executed to
            // start the SlideshowActivity.
            getAsyncDialog().runAsync(new Runnable() {
                @Override
                public void run() {
                    // This runnable gets run in a background thread.
                    mTempMmsUri = mWorkingMessage.saveAsMms(false);
                }
            }, new Runnable() {
                @Override
                public void run() {
                    // Once the above background thread is complete, this runnable is run
                    // on the UI thread.
                    // Launch the slideshow activity to play/view.
                    Intent intent;
                    SlideModel slide = slideshow.get(0);
                    /// M: play the only audio directly
                    if ((slideshow.isSimple() && slide != null && slide.hasAudio())
                        || (requestCode == AttachmentEditor.MSG_PLAY_AUDIO)) {
                        intent = new Intent(ComposeMessageActivity.this, SlideshowActivity.class);
                    } else {
                        intent = new Intent(ComposeMessageActivity.this, MmsPlayerActivity.class);
                    }
                    intent.setData(mTempMmsUri);
                    if (mTempMmsUri == null) {
                        MmsLog.d(TAG, "viewMmsMessageAttachment mTempMmsUri == null");
                        return;
                    }
                    if (requestCode > 0) {
                        startActivityForResult(intent, requestCode);
                    } else {
                        startActivity(intent);
                    }
                    /// @}
                }
            // M: fix bug ALPS00351027
            }, R.string.sync_mms_to_db);
        }
    }

    // Whether or not we are currently enabled for SMS. This field is updated in onStart to make
    // sure we notice if the user has changed the default SMS app.
    private boolean mIsSmsEnabled;

    private final Handler mMessageListItemHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            MessageItem msgItem = (MessageItem) msg.obj;
                /// M: move if into switch, we have item not use it.
                /// M: Code analyze, fix bug ALPS00358964
            if (msgItem != null && mIpCompose.onIpMessageListItemHandler(msg.what, msgItem.mMsgId,
                    mConversation.getThreadId(), msgItem.mSubId)) {
                return;
            }

                switch (msg.what) {
                    case MessageListItem.MSG_LIST_DETAILS:
                        if (msgItem != null) {
                            showMessageDetails(msgItem);
                        }
                        break;

                    case MessageListItem.MSG_LIST_EDIT:
                        if (msgItem != null && mIsSmsEnabled) {
                            editMessageItem(msgItem);
                            updateSendButtonState();
                            drawBottomPanel();
                        }
                        break;

                    case MessageListItem.MSG_LIST_PLAY:
                        if (msgItem != null) {
                            switch (msgItem.mAttachmentType) {
                                case WorkingMessage.IMAGE:
                                case WorkingMessage.VIDEO:
                                case WorkingMessage.AUDIO:
                                case WorkingMessage.SLIDESHOW:
                                    MessageUtils.viewMmsMessageAttachment(
                                            ComposeMessageActivity.this,
                                            msgItem.mMessageUri, msgItem.mSlideshow,
                                            getAsyncDialog());
                                    break;
                            }
                        }
                        break;

                    /// M: Code analyze 039, When the cache add new item,
                    /// notifiy ComposeMessageAcitivity the data has been changed .@{
                    case MessageListAdapter.MSG_LIST_NEED_REFRASH: {
                        boolean isClearCache =
                            msg.arg1 == MessageListAdapter.MESSAGE_LIST_REFRASH_WITH_CLEAR_CACHE;
                        MmsLog.d(MessageListAdapter.CACHE_TAG,
                                "mMessageListItemHandler.handleMessage(): " +
                                    "run adapter notify in isClearCache = " + isClearCache);
                        mMsgListAdapter.setClearCacheFlag(isClearCache);
                        mMsgListAdapter.notifyDataSetChanged();
                        return;
                    }
                    /// @
                    /// M:,Support messages multi-delete opeartor. @{
                    case MessageListItem.ITEM_CLICK: { // can be deleted!!!
                        mMsgListAdapter.changeSelectedState(msg.arg1);
                        return;
                    }
                    /// @}
                    default:
                        Log.w(TAG, "Unknown message: " + msg.what);
                        return;
                }
        }
    };

    private boolean showMessageDetails(MessageItem msgItem) {
        /// M: Code analyze 040, The function getMessageDetails
        /// use MessageItem but not cursor now.@{
        /*
        Cursor cursor = mMsgListAdapter.getCursorForItem(msgItem);
        if (cursor == null) {
            return false;
        }
        */

        if (this.isFinishing() || this.isDestroyed()) {
            Log.d(TAG, "Activity finishing or destoryed");
            return false;
        }

        if (mIpCompose.showIpMessageDetails(msgItem.mIpMessageItem)) {
            return true;
        }

        String messageDetails = MessageUtils.getMessageDetails(
               //ComposeMessageActivity.this, cursor, msgItem.mMessageSize);
                ComposeMessageActivity.this, msgItem);
        /// @}
        new AlertDialog.Builder(ComposeMessageActivity.this)
                .setTitle(R.string.message_details_title)
                .setMessage(messageDetails)
                .setCancelable(true)
                .show();
        return true;
    }

    private final OnKeyListener mSubjectKeyListener = new OnKeyListener() {
        @Override
        public boolean onKey(View v, int keyCode, KeyEvent event) {
            if (event.getAction() != KeyEvent.ACTION_DOWN) {
                return false;
            }

            // When the subject editor is empty, press "DEL" to hide the input field.
            if ((keyCode == KeyEvent.KEYCODE_DEL) && (mSubjectTextEditor.length() == 0)) {
                showSubjectEditor(false);
                mWorkingMessage.setSubject(null, true);
                /// M: add for character counter
                updateCounter(mWorkingMessage.getText(), 0, 0, 0);
                return true;
            }
            return false;
        }
    };

    /**
     * Return the messageItem associated with the type ("mms" or "sms") and message id.
     * @param type Type of the message: "mms" or "sms"
     * @param msgId Message id of the message. This is the _id of the sms or pdu row and is
     * stored in the MessageItem
     * @param createFromCursorIfNotInCache true if item is not found in the MessageListAdapter's
     * cache and the code can create a new MessageItem based on the position of the current cursor.
     * If false, the function returns null if the MessageItem isn't in the cache.
     * @return MessageItem or null if not found and createFromCursorIfNotInCache is false
     */
    private MessageItem getMessageItem(String type, long msgId,
            boolean createFromCursorIfNotInCache) {
        return mMsgListAdapter.getCachedMessageItem(type, msgId,
                createFromCursorIfNotInCache ? mMsgListAdapter.getCursor() : null);
    }

    private boolean isCursorValid() {
        // Check whether the cursor is valid or not.
        Cursor cursor = mMsgListAdapter.getCursor();
        if (cursor.isClosed() || cursor.isBeforeFirst() || cursor.isAfterLast()) {
            Log.e(TAG, "Bad cursor.", new RuntimeException());
            return false;
        }
        return true;
    }

    /*** M: remove Google default code
    private void resetCounter() {
        /// M: Code analyze 032, According to the message state to update text counter.@{
        mTextEditor.setText(mWorkingMessage.getText());
        /// M: once updateCounter.
        updateCounter(mWorkingMessage.getText(), 0, 0, 0);
        if (mWorkingMessage.requiresMms()) {
            mTextCounter.setVisibility(View.GONE);
        } else {
            mTextCounter.setVisibility(View.VISIBLE);
        }
        /// @}
    }
    */

    private void updateCounter(CharSequence text, int start, int before, int count) {
        if (!mIsSmsEnabled) {
            MmsLog.w(TAG, "updateCounter(): sms is disabled!");
            mTextCounter.setVisibility(View.GONE);
            return;
        }
        if (text == null) {
            MmsLog.w(TAG, "updateCounter(): text is null!");
            return;
        }

        if (text.length() == 0) {
            mTextCounter.setVisibility(View.GONE);
            mWorkingMessage.setLengthRequiresMms(false, true);
            return;
        }

        // add for ipmessage
        if (mIpCompose.onIpUpdateCounter(text, start, before, count)) {
            return;
        }

        /// M: Code analyze 031, Add encode type for calculating message lenght and always show
        /// text counter if it is in sms mode.@{
        /*
        WorkingMessage workingMessage = mWorkingMessage;
        if (workingMessage.requiresMms()) {
            // If we're not removing text (i.e. no chance of converting back to SMS
            // because of this change) and we're in MMS mode, just bail out since we
            // then won't have to calculate the length unnecessarily.
            final boolean textRemoved = (before > count);
            if (!textRemoved) {
                showSmsOrMmsSendButton(workingMessage.requiresMms());
                return;
            }
        }
        */
        int[] params = null;

        int encodingType = SmsMessage.ENCODING_UNKNOWN;

        encodingType = mOpComposeExt.getSmsEncodingType(encodingType, ComposeMessageActivity.this);

        params = MtkSmsMessage.calculateLength(text, false, encodingType);
            /* SmsMessage.calculateLength returns an int[4] with:
             *   int[0] being the number of SMS's required,
             *   int[1] the number of code units used,
             *   int[2] is the number of code units remaining until the next message.
             *   int[3] is the encoding type that should be used for the message.
             */
        final int msgCount = params[0];
        final int remainingInCurrentMessage = params[2];
        /*
        if (!MmsConfig.getMultipartSmsEnabled()) {
            mWorkingMessage.setLengthRequiresMms(
                    msgCount >= MmsConfig.getSmsToMmsTextThreshold(), true);
        }

        // Show the counter only if:
        // - We are not in MMS mode
        // - We are going to send more than one message OR we are getting close
        boolean showCounter = false;
        if (!workingMessage.requiresMms() &&
                (msgCount > 1 ||
                 remainingInCurrentMessage <= CHARS_REMAINING_BEFORE_COUNTER_SHOWN)) {
            showCounter = true;
        }

        showSmsOrMmsSendButton(workingMessage.requiresMms());

        if (showCounter) {
            // Update the remaining characters and number of messages required.
            String counterText = msgCount > 1 ? remainingInCurrentMessage + " / " + msgCount
                    : String.valueOf(remainingInCurrentMessage);
            mTextCounter.setText(counterText);
            mTextCounter.setVisibility(View.VISIBLE);
        } else {
            mTextCounter.setVisibility(View.GONE);
        }
         */
        mWorkingMessage.setLengthRequiresMms(
            msgCount >= MmsConfig.getSmsToMmsTextThreshold(), true);
        /* MmsLog.d(TAG, "updateCounter(): message msgCount = " + msgCount
                + " TextThreshold() = " + MmsConfig.getSmsToMmsTextThreshold()
                + " remainingInCurrentMessage = " + remainingInCurrentMessage);*/
        /// M: Show the counter
        /// M: Update the remaining characters and number of messages required.
        if (msgCount >= MmsConfig.getSmsToMmsTextThreshold()) {
            mTextCounter.setVisibility(View.GONE);
            return;
        }
        mUiHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                /* MmsLog.d(TAG, "updateCounter requiresMms = " + mWorkingMessage.requiresMms()
                        + " line count = " + mTextEditor.getLineCount());*/
                if (mWorkingMessage.requiresMms() || mTextEditor.getLineCount() <= 1) {
                    mTextCounter.setVisibility(View.GONE);
                    return;
                }
                mTextCounter.setVisibility(View.VISIBLE);
                String counterText = remainingInCurrentMessage + "/" + msgCount;
                mTextCounter.setText(counterText);
            }
        }, 100);
        /// @}
    }

    @Override
    public void startActivityForResult(Intent intent, int requestCode) {
        if (mNeedRequestPermissins) {
            super.startActivityForResult(intent, requestCode);
            return;
        }
        MmsLog.d(TAG, "startActivityForResult,intent: " + intent + ", requestCode" + requestCode);

        // requestCode >= 0 means the activity in question is a sub-activity.
        if (requestCode >= 0) {
            mWaitingForSubActivity = true;
        }

        /// M : FIX CR : ALPS01795853 @{
        if (mWorkingMessage.getText().toString().trim().length() != 0) {
            if (requestCode == REQUEST_CODE_ATTACH_IMAGE
                    || requestCode == REQUEST_CODE_ATTACH_VIDEO
                    || requestCode == REQUEST_CODE_ADD_CONTACT
                    || requestCode == REQUEST_CODE_ATTACH_SOUND
                    || requestCode == REQUEST_CODE_ATTACH_RINGTONE
                    || requestCode == REQUEST_CODE_ATTACH_VCALENDAR
                    || requestCode == REQUEST_CODE_ATTACH_VCARD
                    || requestCode == REQUEST_CODE_RECORD_SOUND
                    || requestCode == REQUEST_CODE_TAKE_PICTURE
                    || requestCode == REQUEST_CODE_TAKE_VIDEO
                    || requestCode == REQUEST_CODE_TEXT_VCARD) {
                mWorkingMessage.setTruntoChooseAttach(true);
            }
        }
        /// @}


        if (mIsKeyboardOpen) {
            hideKeyboard();  // camera and other activities take a long time to hide the keyboard
        }

        /// M: Code analyze 041, Add exception handling for starting activity.@{
        if (null != intent && null != intent.getData()
                && intent.getData().getScheme().equals("mailto")) {
            try {
                super.startActivityForResult(intent, requestCode);
            } catch (ActivityNotFoundException e) {
                MmsLog.e(TAG,
                        "[ActivityNotFoundException] Failed to startActivityForResult: " + intent);
                Intent mchooserIntent = Intent.createChooser(intent, null);
                super.startActivityForResult(mchooserIntent, requestCode);
            } catch (Exception e) {
                MmsLog.e(TAG, "[Exception] Failed to startActivityForResult: " + intent);
                Toast.makeText(this, getString(R.string.message_open_email_fail),
                      Toast.LENGTH_SHORT).show();
          }
        } else {
            try {
                super.startActivityForResult(intent, requestCode);
            } catch (ActivityNotFoundException e) {
                if (requestCode == REQUEST_CODE_PICK) {
                    misPickContatct = false;
                    mShowingContactPicker = false;
                }
                Intent mchooserIntent = Intent.createChooser(intent, null);
                super.startActivityForResult(mchooserIntent, requestCode);
            }
        }
        /// @}
    }

    private void toastConvertInfo(boolean toMms) {
        if (mIpCompose.toastIpConvertInfo(toMms)) {
            return;
        }
        final int resId = toMms ? R.string.converting_to_picture_message
                : R.string.converting_to_text_message;
        Toast.makeText(this, resId, Toast.LENGTH_SHORT).show();
    }

    private class DeleteMessageListener implements OnClickListener {
        private final MessageItem mMessageItem;

        public DeleteMessageListener(MessageItem messageItem) {
            mMessageItem = messageItem;
        }

        @Override
        public void onClick(DialogInterface dialog, int whichButton) {
            dialog.dismiss();

            new Thread(new Runnable() {
                @Override
                public void run() {
                    if (mMessageItem.isMms()) {
                        WorkingMessage.removeThumbnailsFromCache(mMessageItem.getSlideshow());

                        MmsApp.getApplication().getPduLoaderManager()
                            .removePdu(mMessageItem.mMessageUri);
                        // Delete the message *after* we've removed the thumbnails because we
                        // need the pdu and slideshow for removeThumbnailsFromCache to work.
                    }
                    // add for ipmessage
                    mIpCompose.onIpDeleteMessageListenerClick(mMessageItem.mIpMessageItem);

                    /// M: google jb.mr1 patch, Conversation should scroll to the bottom
                    /// when incoming received @{
                    Boolean deletingLastItem = false;
                    Cursor cursor = mMsgListAdapter != null ? mMsgListAdapter.getCursor() : null;
                    if (cursor != null) {
                        cursor.moveToLast();
                        long msgId = cursor.getLong(COLUMN_ID);
                        deletingLastItem = msgId == mMessageItem.mMsgId;
                    }
                    Uri deleteUri = mMessageItem.mMessageUri;
                    MmsLog.dpi(TAG, "deleteUri " + deleteUri
                            + " deleteUri.host " + deleteUri.getHost());

                    mBackgroundQueryHandler.startDelete(DELETE_MESSAGE_TOKEN,
                            deletingLastItem, mMessageItem.mMessageUri,
                            mMessageItem.mLocked ? null : "locked=0", null);
                    /// @}
                    return;
                }
            }).start();
        }
    }

    private class DiscardDraftListener implements OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int whichButton) {
            mWorkingMessage.discard();
            dialog.dismiss();
            /// M: Code analyze 042, If you discard the draft message manually.@{
            mHasDiscardWorkingMessage = true;
            /// @}
            // add for ipmessage
            mIpCompose.onIpDiscardDraftListenerClick();
            finish();
            mDiscardDraftDialog = null;
        }
    }

    private class SendIgnoreInvalidRecipientListener implements OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int whichButton) {
            /// M: Code analyze 030, Check condition before sending message.@{
            checkConditionsAndSendMessage(true);
            /// @}
            dialog.dismiss();
        }
    }

    private class CancelSendingListener implements OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int whichButton) {
            if (isRecipientsEditorVisible()) {
                mRecipientsEditor.requestFocus();
            }
            dialog.dismiss();
            /// M: @{
            updateSendButtonState();
            /// @}
        }
    }

    /// M: fix bug ALPS00484778
    private ContactList mCutRecipients;

    private class CancelSendingListenerForInvalidRecipient implements OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int whichButton) {
            if (isRecipientsEditorVisible()) {
                if (mCutRecipients != null) {
                        mRecipientsEditor.removeChipChangedListener(mChipWatcher);
                        mRecipientsEditor.populate(new ContactList());
                        mRecipientsEditor.addChipChangedListener(mChipWatcher);
                        mRecipientsEditor.populate(mCutRecipients);
                        mCutRecipients = null;
                } else {
                    mRecipientsEditor.requestFocus();
                }
            }
            dialog.dismiss();
            /// M: @{
            updateSendButtonState();
            /// @}
        }
    }
    /// @}

    private void setSubId(int subId) {
        if (subId != 0) {
            mSelectedSubId = subId;
        }
    }

    private void confirmSendMessageIfNeeded() {
        if (!isRecipientsEditorVisible()) {
            /// M: Code analyze 030, Check condition before sending message.@{
            checkConditionsAndSendMessage(true);
            /// @}
            return;
        }

        boolean isMms = mWorkingMessage.requiresMms();

        if (mRecipientsEditor.hasInvalidRecipient(isMms)) {
            /// M: Code analyze 054, Even if there are some invalid recipients , we also try to
            /// send messag.Now, We do not disgingush there are some or all invalid recipients. @{
            updateSendButtonState();
                String title = getResourcesString(R.string.has_invalid_recipient,
                        mRecipientsEditor.formatInvalidNumbers(isMms));
                new AlertDialog.Builder(this)
                    .setCancelable(false)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle(title)
                    .setMessage(R.string.invalid_recipient_message)
                    .setPositiveButton(R.string.try_to_send,
                            new SendIgnoreInvalidRecipientListener())
                    .setNegativeButton(R.string.no, new CancelSendingListenerForInvalidRecipient())
                    .setOnKeyListener(new DialogInterface.OnKeyListener() {
                        @Override
                        public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                            if (keyCode == KeyEvent.KEYCODE_BACK) {
                                dialog.dismiss();
                            }
                            return false;
                        }
                    })
                    .show();
        } else {
             /// M: Code analyze 030, Check condition before
            /// sending message.(All recipients are valid.)@{
            checkConditionsAndSendMessage(true);
             /// @}
        }
    }

    /// M: Recipient Control refactory, don't use TextWatcher: replace with ChipWatcher,
    /// to reduce the frequency of notify. and to sync recipients. @{
    private final MTKRecipientEditTextView.ChipWatcher mChipWatcher =
        new MTKRecipientEditTextView.ChipWatcher() {
        public void onChipChanged(ArrayList<RecipientEntry> allChips,
                ArrayList<String> changedChipAddresses, String lastString) {
            /// M: ALPS01843842, when chip changed ,close the contact view context menu.
            if (mChipViewMenu != null) {
                mChipViewMenu.close();
                mChipViewMenu = null;
            }
            if (!isRecipientsEditorVisible()) {
                Log.w(TAG, "ChipWatcher: onChipChanged called with invisible mRecipientsEditor");
                return;
            }
            /*
            if (changedChipAddresses == null || changedChipAddresses.size() == 0) {
                return;
            }
            */
            Log.i(TAG, "ChipWatcher onChipChanged begin.");
            ContactList LastContacts = mRecipientsEditor.getContactsFromChipWatcher();
            int updateLimit = getLimitedContact();
            /// @}
            mRecipientsEditor.parseRecipientsFromChipWatcher(allChips,
                    changedChipAddresses, lastString, updateLimit);
            List<String> numbers = mRecipientsEditor.getNumbersFromChipWatcher();
            // google steps in textchange
            mWorkingMessage.setWorkingRecipients(numbers);
            /// M: google JB.MR1 patch, group mms
            boolean multiRecipients = numbers != null && numbers.size() > 1;
            boolean isGroupMms =
                MmsPreferenceActivity.getIsGroupMmsEnabled(ComposeMessageActivity.this)
                                                        && multiRecipients;
            mMsgListAdapter.setIsGroupConversation(isGroupMms);
            mWorkingMessage.setHasMultipleRecipients(multiRecipients, true);
            mWorkingMessage.setHasEmail(mRecipientsEditor.containsEmailFromChipWatcher(), true);
            int recipientCount = (numbers != null ? numbers.size() : 0);
            checkForTooManyRecipients(recipientCount);
            // google steps end
            ContactList contacts = mRecipientsEditor.getContactsFromChipWatcher();
            if (!contacts.equals(LastContacts)
                    || (changedChipAddresses != null && changedChipAddresses.size() > 0)) {
                updateTitle(contacts);
            }
            updateSendButtonState();
            mIpCompose.onIpRecipientsChipChanged(contacts.size());
        }
    };
    /// @}

    private void checkForTooManyRecipients(int recipientCount) {
        /// M: Code analyze 056,Now,the sms recipient limit is different from mms.
        /// We can set limit for sms or mms individually. @{
        final int recipientLimit = MmsConfig.getRecipientLimit();
        /// @}
        if (recipientLimit != Integer.MAX_VALUE && recipientLimit > 0) {

            //final int recipientCount = recipientCount();
            boolean tooMany = recipientCount > recipientLimit;

            if (recipientCount != mLastRecipientCount) {
                // Don't warn the user on every character they type when they're over the limit,
                // only when the actual # of recipients changes.
                mLastRecipientCount = recipientCount;
                if (tooMany) {
                    String tooManyMsg = getString(R.string.too_many_recipients, recipientCount,
                            recipientLimit);
                    Toast.makeText(ComposeMessageActivity.this,
                            tooManyMsg, Toast.LENGTH_LONG).show();
                }
            }
        }
    }

    private final OnCreateContextMenuListener mRecipientsMenuCreateListener =
        new OnCreateContextMenuListener() {
        @Override
        public void onCreateContextMenu(ContextMenu menu, View v,
                ContextMenuInfo menuInfo) {
            mChipViewMenu = menu;
            if (menuInfo != null) {
                Contact c = ((RecipientContextMenuInfo) menuInfo).recipient;
                RecipientsMenuClickListener l = new RecipientsMenuClickListener(c);

                menu.setHeaderTitle(c.getName());

                if (c.existsInDatabase()) {
                    menu.add(0, MENU_VIEW_CONTACT, 0, R.string.menu_view_contact)
                            .setOnMenuItemClickListener(l);
                /// M: Code analyze 043, Whether the address can be added to contacts app. @{
                } else if (MessageUtils.canAddToContacts(c)) {
                /// @}
                    menu.add(0, MENU_ADD_TO_CONTACTS, 0, R.string.menu_add_to_contacts)
                            .setOnMenuItemClickListener(l);
                }
            }
        }
    };

    private final class RecipientsMenuClickListener implements MenuItem.OnMenuItemClickListener {
        private final Contact mRecipient;

        RecipientsMenuClickListener(Contact recipient) {
            mRecipient = recipient;
        }

        @Override
        public boolean onMenuItemClick(MenuItem item) {
            switch (item.getItemId()) {
                // Context menu handlers for the recipients editor.
                case MENU_VIEW_CONTACT: {
                    Uri contactUri = mRecipient.getUri();
                    /// M: fix bug ALPS00448677, update or delete Contact Chip
                    mInViewContact = mRecipient;
                    /// @}
                    Intent intent = new Intent(Intent.ACTION_VIEW, contactUri);
                    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                    startActivityForResult(intent, REQUEST_CODE_VIEW_CONTACT);
                    return true;
                }
                case MENU_ADD_TO_CONTACTS: {
                    /// M: fix bug ALPS00448677, update or delete Contact Chip
                    mInViewContact = null;
                    /// @}
                    mAddContactIntent = ConversationList.createAddContactIntent(
                            mRecipient.getNumber());
                    ComposeMessageActivity.this.startActivityForResult(mAddContactIntent,
                            REQUEST_CODE_ADD_CONTACT);
                    return true;
                }
            }
            return false;
        }
    }

    private boolean canAddToContacts(Contact contact) {
        // There are some kind of automated messages, like STK messages, that we don't want
        // to add to contacts. These names begin with special characters, like, "*Info".
        final String name = contact.getName();
        if (!TextUtils.isEmpty(contact.getNumber())) {
            char c = contact.getNumber().charAt(0);
            if (isSpecialChar(c)) {
                return false;
            }
        }
        if (!TextUtils.isEmpty(name)) {
            char c = name.charAt(0);
            if (isSpecialChar(c)) {
                return false;
            }
        }
        if (!(Mms.isEmailAddress(name) ||
                Telephony.Mms.isPhoneNumber(name) ||
                contact.isMe())) {
            return false;
        }
        return true;
    }

    private boolean isSpecialChar(char c) {
        return c == '*' || c == '%' || c == '$';
    }

    private void addPositionBasedMenuItems(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        AdapterView.AdapterContextMenuInfo info;

        try {
            info = (AdapterView.AdapterContextMenuInfo) menuInfo;
        } catch (ClassCastException e) {
            Log.e(TAG, "bad menuInfo");
            return;
        }
        final int position = info.position;

        addUriSpecificMenuItems(menu, v, position);
    }

    private Uri getSelectedUriFromMessageList(ListView listView, int position) {
        // If the context menu was opened over a uri, get that uri.
        MessageListItem msglistItem = (MessageListItem) listView.getChildAt(position);
        if (msglistItem == null) {
            // FIXME: Should get the correct view. No such interface in ListView currently
            // to get the view by position. The ListView.getChildAt(position) cannot
            // get correct view since the list doesn't create one child for each item.
            // And if setSelection(position) then getSelectedView(),
            // cannot get corrent view when in touch mode.
            return null;
        }

        TextView textView;
        CharSequence text = null;
        int selStart = -1;
        int selEnd = -1;

        //check if message sender is selected
        textView = (TextView) msglistItem.findView(MessageListItem.text_view);
        if (textView != null) {
            text = textView.getText();
            selStart = textView.getSelectionStart();
            selEnd = textView.getSelectionEnd();
        }

        /// M: Code analyze 044,If sender is not being selected, it may be in the message body.@{
        if (selStart == -1) {
            textView = (TextView) msglistItem.findView(MessageListItem.text_view);
            if (textView != null) {
                text = textView.getText();
                selStart = textView.getSelectionStart();
                selEnd = textView.getSelectionEnd();
            }
        }
        /// @}

        // Check that some text is actually selected, rather than the cursor
        // just being placed within the TextView.
        if (selStart != selEnd) {
            int min = Math.min(selStart, selEnd);
            int max = Math.max(selStart, selEnd);

            URLSpan[] urls = ((Spanned) text).getSpans(min, max,
                                                        URLSpan.class);

            if (urls.length == 1) {
                return Uri.parse(urls[0].getURL());
            }
        }

        //no uri was selected
        return null;
    }

    private void addUriSpecificMenuItems(ContextMenu menu, View v, int position) {
        Uri uri = getSelectedUriFromMessageList((ListView) v, position);

        if (uri != null) {
            Intent intent = new Intent(null, uri);
            intent.addCategory(Intent.CATEGORY_SELECTED_ALTERNATIVE);
            menu.addIntentOptions(0, 0, 0,
                    new android.content.ComponentName(this, ComposeMessageActivity.class),
                    null, intent, 0, null);
        }
    }

    private final void addCallAndContactMenuItems(
            ContextMenu menu, MsgListMenuClickListener l, MessageItem msgItem) {
        if (TextUtils.isEmpty(msgItem.mBody)) {
            return;
        }
        SpannableString msg = new SpannableString(msgItem.mBody);
        Linkify.addLinks(msg, Linkify.EMAIL_ADDRESSES|Linkify.PHONE_NUMBERS|Linkify.WEB_URLS);
        ArrayList<String> uris =
            MessageUtils.extractUris(msg.getSpans(0, msg.length(), URLSpan.class));
        /// M: Code analyze 022, Add bookmark. Clear the List.@{
        mURLs.clear();
        /// @}
        // Remove any dupes so they don't get added to the menu multiple times
        HashSet<String> collapsedUris = new HashSet<String>();
        for (String uri : uris) {
            collapsedUris.add(uri.toLowerCase());
        }
        for (String uriString : collapsedUris) {
            String prefix = null;
            int sep = uriString.indexOf(":");
            if (sep >= 0) {
                prefix = uriString.substring(0, sep);
                /// M: Code analyze 022, Add bookmark. @{
                if ("mailto".equalsIgnoreCase(prefix) || "tel".equalsIgnoreCase(prefix)) {
                    uriString = uriString.substring(sep + 1);
                }
                /// @}
            }
            Uri contactUri = null;
            boolean knownPrefix = true;
            if ("mailto".equalsIgnoreCase(prefix))  {
                contactUri = getContactUriForEmail(uriString);
            } else if ("tel".equalsIgnoreCase(prefix)) {
                contactUri = getContactUriForPhoneNumber(uriString);
            } else {
                knownPrefix = false;

                /// M: Code analyze 022, Add bookmark. Maybe exist multi URL address @{
                if (msgItem.isSms() && mURLs.size() <= 0) {
                    menu.add(0, MENU_ADD_TO_BOOKMARK, 0, R.string.menu_add_to_bookmark)
                    .setOnMenuItemClickListener(l);
                }
                /// @}

                /// M: Code analyze 022, Add bookmark. fix bug ALPS00783237 @{
                for (String uri : uris) {
                    if (uri != null && uri.equalsIgnoreCase(uriString)) {
                        mURLs.add(uri);
                        break;
                    }
                }
                /// @}
            }
            if (knownPrefix && contactUri == null) {
                Intent intent = ConversationList.createAddContactIntent(uriString);

                String addContactString = getString(R.string.menu_add_address_to_contacts,
                        uriString);
                menu.add(0, MENU_ADD_ADDRESS_TO_CONTACTS, 0, addContactString)
                    .setOnMenuItemClickListener(l)
                    .setIntent(intent);
            }
        }
        /// M: New plugin API @{
        mOpComposeExt.addCallAndContactMenuItems(
                this, menu, MENU_ADD_TO_BOOKMARK, mURLs, msgItem);
        /// @}
    }

    private Uri getContactUriForEmail(String emailAddress) {
        Cursor cursor = SqliteWrapper.query(this, getContentResolver(),
                Uri.withAppendedPath(Email.CONTENT_LOOKUP_URI, Uri.encode(emailAddress)),
                new String[] { Email.CONTACT_ID, Contacts.DISPLAY_NAME }, null, null, null);

        if (cursor != null) {
            try {
                while (cursor.moveToNext()) {
                    String name = cursor.getString(1);
                    if (!TextUtils.isEmpty(name)) {
                        return ContentUris.withAppendedId(Contacts.CONTENT_URI, cursor.getLong(0));
                    }
                }
            } finally {
                cursor.close();
            }
        }
        return null;
    }

    private Uri getContactUriForPhoneNumber(String phoneNumber) {
        Contact contact = Contact.get(phoneNumber, true);
        if (contact.existsInDatabase()) {
            return contact.getUri();
        }
        return null;
    }

    private final OnCreateContextMenuListener mMsgListMenuCreateListener =
        new OnCreateContextMenuListener() {
        @Override
        public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
            if (!isCursorValid()) {
                return;
            }
            Cursor cursor = mMsgListAdapter.getCursor();
            String type = cursor.getString(COLUMN_MSG_TYPE);
            long msgId = cursor.getLong(COLUMN_ID);
            ///M: ALPS00726802, save the position in cursor of the clicked item
            mClickedItemPosition = cursor.getPosition();

            MmsLog.i(TAG, "onCreateContextMenu(): msgId=" + msgId);
            addPositionBasedMenuItems(menu, v, menuInfo);

            MessageItem msgItem = mMsgListAdapter.getCachedMessageItem(type, msgId, cursor);
            if (msgItem == null) {
                Log.w(TAG, "Cannot load message item for type = " + type
                        + ", msgId = " + msgId);
                return;
            }

            menu.setHeaderTitle(R.string.message_options);

            MsgListMenuClickListener l = new MsgListMenuClickListener(msgItem);

            // add for ipmessage
            boolean isForwardEnabled =
                msgItem.isSms() || (msgItem.isDownloaded() && isForwardable(msgId));

            if (msgItem.isSms()) {
                menu.add(0, MENU_COPY, 0, R.string.ipmsg_copy)
                .setOnMenuItemClickListener(l);
            }

            // Forward is not available for undownloaded messages.
            if (mIsSmsEnabled) {
                if (isForwardEnabled) {
                    menu.add(0, MENU_FORWARD_MESSAGE, 0,
                            R.string.menu_forward).setOnMenuItemClickListener(l);
                }
            }
            if (mIsSmsEnabled) {
                menu.add(0, MENU_DELETE_MESSAGE, 0, R.string.delete_message)
                        .setOnMenuItemClickListener(l);
            }

            if (mIsSmsEnabled) {
                if (msgItem.mLocked) {
                    menu.add(0, MENU_UNLOCK_MESSAGE, 0, R.string.menu_unlock)
                            .setOnMenuItemClickListener(l);
                } else if (mIsSmsEnabled) {
                    menu.add(0, MENU_LOCK_MESSAGE, 0, R.string.menu_lock)
                            .setOnMenuItemClickListener(l);
                }
            }

            if (msgItem.isMms()) {
                switch (msgItem.mAttachmentType) {
                    case WorkingMessage.SLIDESHOW:
                        menu.add(0, MENU_VIEW_SLIDESHOW, 0, R.string.view_slideshow)
                                .setOnMenuItemClickListener(l);
                        if (haveSomethingToCopyToSDCard(msgItem.mMsgId)) {
                            menu.add(0, MENU_COPY_TO_SDCARD, 0, R.string.copy_to_sdcard)
                                    .setOnMenuItemClickListener(l);
                        }
                        break;
                    default:
                        if (haveSomethingToCopyToSDCard(msgItem.mMsgId)) {
                            menu.add(0, MENU_COPY_TO_SDCARD, 0, R.string.copy_to_sdcard)
                                    .setOnMenuItemClickListener(l);
                        }
                        break;
                }
            }

            addCallAndContactMenuItems(menu, l, msgItem);

            menu.add(0, MENU_VIEW_MESSAGE_DETAILS, 0, R.string.view_message_details)
                .setOnMenuItemClickListener(l);

            if (msgItem.mDeliveryStatus != MessageItem.DeliveryStatus.NONE
                    || msgItem.mReadReport) {
                menu.add(0, MENU_DELIVERY_REPORT, 0, R.string.view_delivery_report)
                        .setOnMenuItemClickListener(l);
            }

            /// M: Code analyze 016, Add for select text copy. @{
            if (!TextUtils.isEmpty(msgItem.mBody)
                    && msgItem.mMessageType != MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) {
                menu.add(0, MENU_SELECT_TEXT, 0, R.string.select_text)
                        .setOnMenuItemClickListener(l);
                bListUpdated = false;
            }
            /// @}

            if (mIsSmsEnabled) {
                if (msgItem.isSms()) {
                    if (mSubCount > 0 && !msgItem.isSending()) {
                        menu.add(0, MENU_SAVE_MESSAGE_TO_SUB, 0, R.string.save_message_to_sim)
                                .setOnMenuItemClickListener(l);
                    }
                }
            }

            mIpCompose.onIpCreateContextMenu(menu, mIsSmsEnabled, isForwardEnabled,
                    msgItem.mIpMessageItem);
        }
    };

    private void editMessageItem(MessageItem msgItem) {
        if ("sms".equals(msgItem.mType)) {
            if (mWorkingMessage.requiresMms()) {
                /// M: it's a real slideshow, ignore edit request, show a toast.
                String message = getString(R.string.failed_to_add_media,
                        getString(R.string.viewer_title_sms));
                Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
                return;
            } else {
                editSmsMessageItem(msgItem);
            }
        } else {
            editMmsMessageItem(msgItem);
            mWorkingMessage.setHasMmsDraft(true);
        }
        MessagingNotification.nonBlockingUpdateSendFailedNotification(ComposeMessageActivity.this);
        /// M: @{
        if ((msgItem.isFailedMessage() || msgItem.isSending())
                && mMsgListAdapter.getCount() <= 1) {
            // For messages with bad addresses, let the user re-edit the recipients.
            initRecipientsEditor(null);
            /// M: Code analyze 046, Whether the recipientedit control has been initialized. @{
            isInitRecipientsEditor = true;
            /// @}
            mMsgListAdapter.changeCursor(null);
            invalidateOptionsMenu();
        }
    }

    private void editSmsMessageItem(MessageItem msgItem) {
        // When the message being edited is the only message in the conversation, the delete
        // below does something subtle. The trigger "delete_obsolete_threads_pdu" sees that a
        // thread contain no messages and silently deletes the thread. Meanwhile, the mConversation
        // object still holds onto the old thread_id and code thinks there's a backing thread in
        // the DB when it really has been deleted. Here we try and notice that situation and
        // clear out the thread_id. Later on, when Conversation.ensureThreadId() is called, we'll
        // create a new thread if necessary.
        synchronized (mConversation) {
            /// M: @{
            //if (mConversation.getMessageCount() <= 1) {
            Cursor adapterCursor = mMsgListAdapter.getCursor();
            if ((adapterCursor == null) || (adapterCursor.getCount() <= 1)) {
            /// @}
                mConversation.clearThreadId();
                MessagingNotification.setCurrentlyDisplayedThreadId(
                    MessagingNotification.THREAD_NONE);
            }
        }
        // Delete the old undelivered SMS and load its content.
        Uri uri = ContentUris.withAppendedId(Sms.CONTENT_URI, msgItem.mMsgId);
        SqliteWrapper.delete(ComposeMessageActivity.this,
                mContentResolver, uri, null, null);

        mWorkingMessage.setConversation(mConversation);
        mWorkingMessage.setText(msgItem.mBody);
    }

    private void editMmsMessageItem(MessageItem msgItem) {
        /// M: fix bug ALPS00834025. ignore a click if too close.
        long oldTime = mEditClickTime;
        mEditClickTime = SystemClock.elapsedRealtime();
        if ((mEditClickTime - oldTime < 1500) && (mEditClickTime - oldTime > 0)) {
            MmsLog.w(TAG, "editMmsMessageItem, ignore a click if too close");
            return;
        }

        /// M: make a failed message editable, the count should reduce too.
        if (mConversation.getMessageCount() == 1) {
            mConversation.setMessageCount(0);
        }
        /// M: Discard the current message in progress.
        mWorkingMessage.discard();

        // Load the selected message in as the working message.
        WorkingMessage newWorkingMessage = WorkingMessage.load(this, msgItem.mMessageUri);
        if (newWorkingMessage == null) {
            MmsLog.w(TAG, "editMmsMessageItem, load returns null message");
            return;
        }
        mWorkingMessage = newWorkingMessage;
        mWorkingMessage.setConversation(mConversation);
        invalidateOptionsMenu();
        /// M: @{
        mAttachmentEditor.update(mWorkingMessage);
        updateTextEditorHeightInFullScreen();
        /// @}
        drawTopPanel(false);

        // WorkingMessage.load() above only loads the slideshow. Set the
        // subject here because we already know what it is and avoid doing
        // another DB lookup in load() just to get it.
        mWorkingMessage.setSubject(msgItem.mSubject, false);

        if (mWorkingMessage.hasSubject()) {
            showSubjectEditor(true);
        }

        /// M: fix bug ALPS00433858, update read==1(readed) when reload failed-mms
        final MessageItem item = msgItem;
        new Thread(new Runnable() {
            public void run() {
                // TODO Auto-generated method stub
                Uri uri = ContentUris.withAppendedId(Mms.CONTENT_URI, item.mMsgId);
                ContentValues values = new ContentValues(1);
                values.put(Mms.READ, 1);
                SqliteWrapper.update(ComposeMessageActivity.this,
                        mContentResolver, uri, values, null, null);
            }
        }).start();
    }

    private void copyToClipboard(String str) {
        ClipboardManager clipboard =
            (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
        clipboard.setPrimaryClip(ClipData.newPlainText(null, str));
    }

    private void forwardMessage(final MessageItem msgItem) {
        /// M: Code analyze 051, Hide input keyboard
        /// (add for input method covered Compose UI issue).@{
        hideInputMethod();
        /// @}

        mTempThreadId = 0;
        // The user wants to forward the message. If the message is an mms message, we need to
        // persist the pdu to disk. This is done in a background task.
        // If the task takes longer than a half second, a progress dialog is displayed.
        // Once the PDU persisting is done, another runnable on the UI thread get executed to start
        // the ForwardMessageActivity.
        getAsyncDialog().runAsync(new Runnable() {
            @Override
            public void run() {
                // This runnable gets run in a background thread.
                if (msgItem.mType.equals("mms")) {
                    MtkSendReq sendReq = new MtkSendReq();
                    String subject = getString(R.string.forward_prefix);
                    if (msgItem.mSubject != null) {
                        subject += msgItem.mSubject;
                    }
                    sendReq.setSubject(new EncodedStringValue(subject));
                    sendReq.setBody(msgItem.mSlideshow.makeCopy());

                    mTempMmsUri = null;
                    try {
                        MtkPduPersister persister =
                                MtkPduPersister.getPduPersister(ComposeMessageActivity.this);
                        // Copy the parts of the message here.
                        /// M: google jb.mr1 patch, group mms
                        mTempMmsUri = persister.persist(sendReq, Mms.Draft.CONTENT_URI, true,
                                MmsPreferenceActivity
                                    .getIsGroupMmsEnabled(ComposeMessageActivity.this), null);
                        mTempThreadId = MessagingNotification.getThreadId(
                                ComposeMessageActivity.this, mTempMmsUri);
                    } catch (MmsException e) {
                        Log.e(TAG, "Failed to copy message: " + msgItem.mMessageUri);
                        runOnUiThread(new Runnable() {
                            public void run() {
                                Toast.makeText(ComposeMessageActivity.this,
                                        R.string.cannot_save_message, Toast.LENGTH_SHORT).show();
                            }
                        });
                        return;
                    }
                }
            }
        }, new Runnable() {
            @Override
            public void run() {
                // Once the above background thread is complete, this runnable is run
                // on the UI thread.
                Intent intent = createIntent(ComposeMessageActivity.this, 0);
                intent.putExtra(KEY_FORWARDED_MESSAGE, true);
                if (mTempThreadId > 0) {
                    intent.putExtra("thread_id", mTempThreadId);
                }

                if (msgItem.mType.equals("sms")) {
                    intent.putExtra("sms_body", msgItem.mBody);
                } else {
                    intent.putExtra("msg_uri", mTempMmsUri);
                    String subject = getString(R.string.forward_prefix);
                    if (msgItem.mSubject != null) {
                        subject += msgItem.mSubject;
                    }
                    intent.putExtra("subject", subject);
                }

                Contact contact = Contact.get(msgItem.mAddress, false);
                String nameAndNumber = Contact.formatNameAndNumber(
                        contact.getName(), contact.getNumber(), "");
                if (!mOpComposeExt.forwardMessage(
                        ComposeMessageActivity.this, msgItem, nameAndNumber, intent)) {
                    intent.putExtra(KEY_EXIT_ON_SENT, true);
                }

                // ForwardMessageActivity is simply an alias in the manifest for
                // ComposeMessageActivity. We have to make an alias because ComposeMessageActivity
                // launch flags specify singleTop. When we forward a message, we want to start a
                // separate ComposeMessageActivity. The only way to do that is to override the
                // singleTop flag, which is impossible to do in code. By creating an alias to the
                // activity, without the singleTop flag, we can launch a separate
                // ComposeMessageActivity to edit the forward message.
                intent.setClassName(ComposeMessageActivity.this,
                        "com.android.mms.ui.ForwardMessageActivity");
                startActivity(intent);
            }
        }, R.string.sync_mms_to_db);
    }

    /**
     * Context menu handlers for the message list view.
     */
    private final class MsgListMenuClickListener implements MenuItem.OnMenuItemClickListener {
        private MessageItem mMsgItem;

        public MsgListMenuClickListener(MessageItem msgItem) {
            mMsgItem = msgItem;
        }

        @Override
        public boolean onMenuItemClick(MenuItem item) {
            if (mMsgItem == null) {
                return false;
            }
            if (mIpCompose.onIpMenuItemClick(item, mMsgItem.mIpMessageItem)) {
                return true;
            }

            Intent deliveryIntent = new Intent(
                    ComposeMessageActivity.this, DeliveryReportActivity.class);
            Intent intentMultiSave = new Intent(
                    ComposeMessageActivity.this, MultiSaveActivity.class);
            if (mOpComposeExt.onMenuItemClick(
                    item, intentMultiSave, deliveryIntent)) {
                return true;
            }

            switch (item.getItemId()) {
                case MENU_EDIT_MESSAGE:
                    editMessageItem(mMsgItem);
                    drawBottomPanel();
                    return true;
                case MENU_COPY:
                case MENU_COPY_MESSAGE_TEXT:
                    /// M: keep original string when copy, if delete '\r', search will failed. @{
                    if (mMsgItem.mBody != null) {
                        copyToClipboard(mMsgItem.mBody);
                    } else {
                        MmsLog.w(TAG, "onMenuItemClick, mMsgItem.mBody == null");
                        return false;
                    }
                    /// @}
                    return true;

                case MENU_FORWARD_MESSAGE:
                    /// M: @{
                    final MessageItem mRestrictedItem = mMsgItem;
                    if (WorkingMessage.sCreationMode == 0 ||
                      !MessageUtils.isRestrictedType(ComposeMessageActivity.this,
                              mMsgItem.mMsgId)) {
                        runOnUiThread(new Runnable() {
                            public void run() {
                                forwardMessage(mRestrictedItem);
                            }
                        });
                    } else if (WorkingMessage.sCreationMode == WorkingMessage.WARNING_TYPE) {
                        new AlertDialog.Builder(ComposeMessageActivity.this)
                        .setTitle(R.string.restricted_forward_title)
                        .setIconAttribute(android.R.attr.alertDialogIcon)
                        .setMessage(R.string.restricted_forward_message)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    int createMode = WorkingMessage.sCreationMode;
                                    WorkingMessage.sCreationMode = 0;
                                    runOnUiThread(new Runnable() {
                                        public void run() {
                                            forwardMessage(mRestrictedItem);
                                        }
                                    });
                                    WorkingMessage.sCreationMode = createMode;
                                }
                            })
                        .setNegativeButton(android.R.string.cancel, null)
                        .show();
                    }
                    /// @}
                    return true;

                case MENU_VIEW_SLIDESHOW:
                    /// M: Code analyze 024, If the click operator can be responsed. @{
                    if (mClickCanResponse) {
                        mClickCanResponse = false;
                        MessageUtils.viewMmsMessageAttachment(ComposeMessageActivity.this,
                                ContentUris.withAppendedId(Mms.CONTENT_URI, mMsgItem.mMsgId), null,
                                getAsyncDialog());
                    return true;
                    }
                    /// @}
                case MENU_VIEW_MESSAGE_DETAILS:
                    return showMessageDetails(mMsgItem);

                case MENU_DELETE_MESSAGE: {
                    DeleteMessageListener l = new DeleteMessageListener(mMsgItem);

                    /// M: Code analyze 027,Add for deleting one message.@{
                    // (Why only query sms table?)
                    String where = Telephony.Mms._ID + "=" + mMsgItem.mMsgId;
                    String[] projection = new String[] { Sms.Inbox.THREAD_ID };
                    Cursor queryCursor = Sms.query(getContentResolver(), // query uri:content://sms
                            projection, where, null);

                    if (queryCursor != null) {
                        if (queryCursor.moveToFirst()) {
                            mThreadId = queryCursor.getLong(0);
                        }
                        queryCursor.close();
                    }
                    /// @}
                    confirmDeleteDialog(l, mMsgItem.mLocked);
                    return true;
                }
                case MENU_DELIVERY_REPORT:
                    showDeliveryReport(mMsgItem.mMsgId, mMsgItem.mType);
                    return true;

                case MENU_COPY_TO_SDCARD: {
                    /// M: Code analyze 021, Copy valid parts of the attachment(pdu) to SD card.
                    /// This opeartor will be removed to a separate activity.  @{
                    /// M: new feature, change default disk when storage is full
                    long availSize = MessageUtils.getAvailableBytesInFileSystemAtGivenRoot
                                                    (StorageManagerEx.getDefaultPath());
                    boolean haveExSD = MessageUtils.existingSD(ComposeMessageActivity.this, true);
                    boolean haveInSD = MessageUtils.existingSD(ComposeMessageActivity.this, false);

                    /// M: fix bug ALPS00574679, modify toast string when haven't SD Card @{
                    if (!haveExSD && !haveInSD) {
                        Toast.makeText(ComposeMessageActivity.this,
                                getString(R.string.no_sdcard_suggestion),
                                Toast.LENGTH_LONG).show();
                        return false;
                    }
                    /// @}

                    if (mMsgItem.mMessageSize > availSize) {
                        if ((haveInSD && !haveExSD) || (!haveInSD && haveExSD)) {
                            Toast.makeText(ComposeMessageActivity.this,
                                    getString(R.string.export_disk_problem),
                                    Toast.LENGTH_LONG).show();
                            return false;
                        } else {
                            new AlertDialog.Builder(ComposeMessageActivity.this)
                            .setTitle(R.string.copy_to_sdcard_fail)
                            .setIconAttribute(android.R.attr.alertDialogIcon)
                            .setMessage(R.string.change_default_disk)
                            .setPositiveButton(R.string.change,
                                    new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    Intent intent = new Intent(
                                            "android.settings.MEMORY_CARD_SETTINGS");
                                    startActivity(intent);
                                }
                            })
                            .setNegativeButton(android.R.string.cancel, null)
                            .show();
                        }
                    } else {

                        intentMultiSave.putExtra("msgid", mMsgItem.mMsgId);
                        startActivityForResult(intentMultiSave, REQUEST_CODE_MULTI_SAVE);
                     }
                    return true;
                }

                case MENU_SAVE_RINGTONE: {
                    int resId = getDrmMimeSavedStringRsrc(mMsgItem.mMsgId,
                            saveRingtone(mMsgItem.mMsgId));
                    Toast.makeText(ComposeMessageActivity.this, resId, Toast.LENGTH_SHORT).show();
                    return true;
                }
                case MENU_LOCK_MESSAGE: {
                    lockMessage(mMsgItem, true);
                    return true;
                }
                case MENU_UNLOCK_MESSAGE: {
                    lockMessage(mMsgItem, false);
                    return true;
                }

                /// M: Code analyze 022, Add bookmark. Maybe exist multi URL addresses. @{
                case MENU_ADD_TO_BOOKMARK: {
                    if (mURLs.size() == 1) {
                        Browser.saveBookmark(ComposeMessageActivity.this, null, mURLs.get(0));
                    } else if (mURLs.size() > 1) {
                        CharSequence[] items = new CharSequence[mURLs.size()];
                        for (int i = 0; i < mURLs.size(); i++) {
                            items[i] = mURLs.get(i);
                        }
                        new AlertDialog.Builder(ComposeMessageActivity.this)
                            .setTitle(R.string.menu_add_to_bookmark)
                            .setIcon(MessageResource.drawable.ic_dialog_menu_generic)
                            .setItems(items, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    Browser.saveBookmark(ComposeMessageActivity.this,
                                            null, mURLs.get(which));
                                    }
                                })
                            .show();
                    }
                    return true;
                }

                /// M: Code analyze 007, Get information from Sub or save message to Sub. @{
                case MENU_SAVE_MESSAGE_TO_SUB: {
                    mSaveMsgThread = new SaveMsgThread(mMsgItem.mType, mMsgItem.mMsgId);
                    mSaveMsgThread.start();
                    return true;
                }
                /// @}

                /// M: Code analyze 016, Add for select text copy. @{
                case MENU_SELECT_TEXT: {
                    if(bListUpdated == true) {
                      return true;
                    }
                    AdapterView.AdapterContextMenuInfo info =
                        (AdapterView.AdapterContextMenuInfo) item.getMenuInfo();
                    mMsgListAdapter.getItemId(info.position);
                    MessageListItem msglistItem = (MessageListItem) info.targetView;
                    if (msglistItem != null) {
                        Log.i(TAG, "msglistItem != null");
                        TextView textView =
                            (TextView) msglistItem.findView(MessageListItem.text_view);

                        /*Get pure text from textView
                          We will set char[] charToShow to copy text dialog, because this will
                          not pass spans to dialog, to avoid some span related exception, and
                          we don't need format info for only copy text.*/
                        String strToShow = textView.getText().toString();
                        char[] charToShow = strToShow.toCharArray();

                        AlertDialog.Builder builder =
                            new AlertDialog.Builder(ComposeMessageActivity.this);
                        LayoutInflater factory = LayoutInflater.from(builder.getContext());
                        final View textEntryView =
                            factory.inflate(R.layout.alert_dialog_text_entry, null);
                        EditText contentSelector =
                            (EditText) textEntryView.findViewById(R.id.content_selector);

                        contentSelector.setText(charToShow, 0, charToShow.length); //(textView.getText());

                        builder.setTitle(R.string.select_text)
                               .setView(textEntryView)
                               .setPositiveButton(R.string.yes, null)
                               .show();
                    }
                    return true;
                }
                case MENU_ADD_ADDRESS_TO_CONTACTS: {
                    mAddContactIntent = item.getIntent();
                    startActivityForResult(mAddContactIntent, REQUEST_CODE_ADD_CONTACT);
                    return true;
                }
                /// @}

                default:
                    return false;
            }
        }
    }

    private void lockMessage(final MessageItem msgItem, final boolean locked) {
        final Uri uri;
        if ("sms".equals(msgItem.mType)) {
            uri = Sms.CONTENT_URI;
        } else {
            uri = Mms.CONTENT_URI;
        }
        final Uri lockUri = ContentUris.withAppendedId(uri, msgItem.mMsgId);

        final ContentValues values = new ContentValues(1);
        values.put("locked", locked ? 1 : 0);

        new Thread(new Runnable() {
            @Override
            public void run() {
                if (mIpCompose.lockMessage(getApplicationContext(), msgItem.mIpMessageItem,
                        locked)) {
                    return;
                }
                getContentResolver().update(lockUri, values, null, null);
            }
        }, "ComposeMessageActivity.lockMessage").start();
    }

    /**
     * Looks to see if there are any valid parts of the attachment that can be copied to a SD card.
     * @param msgId
     */
    private boolean haveSomethingToCopyToSDCard(long msgId) {
        PduBody body = null;
        try {
            body = SlideshowModel.getPduBody(this,
                        ContentUris.withAppendedId(Mms.CONTENT_URI, msgId));
        } catch (MmsException e) {
            Log.e(TAG, "haveSomethingToCopyToSDCard can't load pdu body: " + msgId);
        }
        if (body == null) {
            return false;
        }

        boolean result = false;
        int partNum = body.getPartsNum();
        for (int i = 0; i < partNum; i++) {
            PduPart part = body.getPart(i);
            // M: fix bug ALPS00355917
            byte[] fileName = part.getFilename();
            String mSrc = null;
            if (fileName == null) {
                fileName = part.getContentLocation();
            }
            if (fileName != null) {
                mSrc = new String(fileName);
            }
            String type =  MessageUtils.getContentType(new String(part.getContentType()), mSrc);
            part.setContentType(type.getBytes());
            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                log("[CMA] haveSomethingToCopyToSDCard: part[" + i + "] contentType=" + type);
            }
            /// M: Code analyze 057,Now, if the pdu type is "application/ogg" or
            /// vcard and vcalender attachment can be saved to sdcard.
            if (MmsContentType.isImageType(type) || MmsContentType.isVideoType(type) ||
                    MmsContentType.isAudioType(type) || DrmUtils.isDrmType(type) ||
                    "application/ogg".equalsIgnoreCase(type)
                    || FileAttachmentModel.isSupportedFile(part)
                    /// M: fix bug ALPS00446644, support dcf (0ct-stream) file to save
                    || (mSrc != null && mSrc.toLowerCase().endsWith(".dcf"))) {
            /// @}
                return true;
            }
        }

        /// M: add for attachment enhance
        // Justify weather there are attachments in parts but not in slides
        // SlideshowModel mSlideShowModel = mWorkingMessage.getSlideshow();
        return mOpComposeExt.haveSomethingToCopyToSDCard(this, body);
    }

    /**
     * Copies media from an Mms to the DrmProvider
     * @param msgId
     */
    private boolean saveRingtone(long msgId) {
        boolean result = true;
        PduBody body = null;
        try {
            body = SlideshowModel.getPduBody(this,
                        ContentUris.withAppendedId(Mms.CONTENT_URI, msgId));
        } catch (MmsException e) {
            Log.e(TAG, "copyToDrmProvider can't load pdu body: " + msgId);
        }
        if (body == null) {
            return false;
        }

        int partNum = body.getPartsNum();
        for (int i = 0; i < partNum; i++) {
            PduPart part = body.getPart(i);
            String type = new String(part.getContentType());

            if (DrmUtils.isDrmType(type)) {
                // All parts (but there's probably only a single one) have to be successful
                // for a valid result.
                result &= copyPart(part, Long.toHexString(msgId));
            }
        }
        return result;
    }

    /**
     * Returns true if any part is drm'd audio with ringtone rights.
     * @param msgId
     * @return true if one of the parts is drm'd audio with rights to save as a ringtone.
     */
    private boolean isDrmRingtoneWithRights(long msgId) {
        PduBody body = null;
        try {
            body = SlideshowModel.getPduBody(this,
                        ContentUris.withAppendedId(Mms.CONTENT_URI, msgId));
        } catch (MmsException e) {
            Log.e(TAG, "isDrmRingtoneWithRights can't load pdu body: " + msgId);
        }
        if (body == null) {
            return false;
        }

        int partNum = body.getPartsNum();
        for (int i = 0; i < partNum; i++) {
            PduPart part = body.getPart(i);
            String type = new String(part.getContentType());

            if (DrmUtils.isDrmType(type)) {
                String mimeType = MmsApp.getApplication().getDrmManagerClient()
                        .getOriginalMimeType(part.getDataUri());
                if (MmsContentType.isAudioType(mimeType)
                        && DrmUtils.haveRightsForAction(part.getDataUri(),
                        DrmStore.Action.RINGTONE)) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Returns true if all drm'd parts are forwardable.
     * @param msgId
     * @return true if all drm'd parts are forwardable.
     */
    private boolean isForwardable(long msgId) {
        PduBody body = null;
        try {
            body = SlideshowModel.getPduBody(this,
                        ContentUris.withAppendedId(Mms.CONTENT_URI, msgId));
        } catch (MmsException e) {
            Log.e(TAG, "getDrmMimeType can't load pdu body: " + msgId);
        }
        if (body == null) {
            return false;
        }

        int partNum = body.getPartsNum();
        for (int i = 0; i < partNum; i++) {
            PduPart part = body.getPart(i);
            String type = new String(part.getContentType());
            if (DrmUtils.isDrmType(type) && !DrmUtils.haveRightsForAction(part.getDataUri(),
                    DrmStore.Action.TRANSFER)) {
                    return false;
            }
        }
        return true;
    }

    private int getDrmMimeMenuStringRsrc(long msgId) {
        if (isDrmRingtoneWithRights(msgId)) {
            return R.string.save_ringtone;
        }
        return 0;
    }

    private int getDrmMimeSavedStringRsrc(long msgId, boolean success) {
        if (isDrmRingtoneWithRights(msgId)) {
            return success ? R.string.saved_ringtone : R.string.saved_ringtone_fail;
        }
        return 0;
    }

    /**
     * Copies media from an Mms to the "download" directory on the SD card. If any of the parts
     * are audio types, drm'd or not, they're copied to the "Ringtones" directory.
     * @param msgId
     */
    private boolean copyMedia(long msgId) {
        boolean result = true;
        PduBody body = null;
        try {
            body = SlideshowModel.getPduBody(this,
                        ContentUris.withAppendedId(Mms.CONTENT_URI, msgId));
        } catch (MmsException e) {
            Log.e(TAG, "copyMedia can't load pdu body: " + msgId);
        }
        if (body == null) {
            return false;
        }

        int partNum = body.getPartsNum();
        for (int i = 0; i < partNum; i++) {
            PduPart part = body.getPart(i);

            // all parts have to be successful for a valid result.
            result &= copyPart(part, Long.toHexString(msgId));
        }
        return result;
    }

    private boolean copyPart(PduPart part, String fallback) {
        Uri uri = part.getDataUri();
        String type = new String(part.getContentType());
        boolean isDrm = DrmUtils.isDrmType(type);
        if (isDrm) {
            type = MmsApp.getApplication().getDrmManagerClient()
                    .getOriginalMimeType(part.getDataUri());
        }
        if (!MmsContentType.isImageType(type) && !MmsContentType.isVideoType(type) &&
                !MmsContentType.isAudioType(type)) {
            return true;    // we only save pictures, videos, and sounds. Skip the text parts,
                            // the app (smil) parts, and other type that we can't handle.
                            // Return true to pretend that we successfully saved the part so
                            // the whole save process will be counted a success.
        }
        InputStream input = null;
        FileOutputStream fout = null;
        try {
            input = mContentResolver.openInputStream(uri);
            if (input instanceof FileInputStream) {
                FileInputStream fin = (FileInputStream) input;

                byte[] location = part.getName();
                if (location == null) {
                    location = part.getFilename();
                }
                if (location == null) {
                    location = part.getContentLocation();
                }

                String fileName;
                if (location == null) {
                    // Use fallback name.
                    fileName = fallback;
                } else {
                    // For locally captured videos, fileName can end up being something like this:
                    //      /mnt/sdcard/Android/data/com.android.mms/cache/.temp1.3gp
                    fileName = new String(location);
                }
                File originalFile = new File(fileName);
                fileName = originalFile.getName();  // Strip the full path of where the "part" is
                                                    // stored down to just the leaf filename.

                // Depending on the location, there may be an
                // extension already on the name or not. If we've got audio, put the attachment
                // in the Ringtones directory.
                String dir = Environment.getExternalStorageDirectory() + "/"
                                + (MmsContentType.isAudioType(type) ?
                                        Environment.DIRECTORY_RINGTONES :
                                    Environment.DIRECTORY_DOWNLOADS)  + "/";
                String extension;
                int index = fileName.lastIndexOf('.');
                if (index == -1) {
                    extension = MimeTypeMap.getSingleton().getExtensionFromMimeType(type);
                } else {
                    extension = fileName.substring(index + 1, fileName.length());
                    fileName = fileName.substring(0, index);
                }
                MmsLog.i(TAG, "Save part extension name is: " + extension);
                if (isDrm) {
                    extension += DrmUtils.getConvertExtension(type);
                }
                // Remove leading periods. The gallery ignores files starting with a period.
                fileName = fileName.replaceAll("^.", "");

                File file = getUniqueDestination(dir + fileName, extension);

                // make sure the path is valid and directories created for this file.
                File parentFile = file.getParentFile();
                if (!parentFile.exists() && !parentFile.mkdirs()) {
                    MmsLog.wpi(TAG, " copyPart: mkdirs for " + parentFile.getPath() + " failed!");
                    return false;
                }

                fout = new FileOutputStream(file);

                byte[] buffer = new byte[8000];
                int size = 0;
                while ((size = fin.read(buffer)) != -1) {
                    fout.write(buffer, 0, size);
                }

                // Notify other applications listening to scanner events
                // that a media file has been added to the sd card
                sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE,
                        Uri.fromFile(file)));
            }
        } catch (IOException e) {
            // Ignore
            Log.e(TAG, "IOException caught while opening or reading stream", e);
            return false;
        } finally {
            if (null != input) {
                try {
                    input.close();
                } catch (IOException e) {
                    // Ignore
                    Log.e(TAG, "IOException caught while closing stream", e);
                    return false;
                }
            }
            if (null != fout) {
                try {
                    fout.close();
                } catch (IOException e) {
                    // Ignore
                    Log.e(TAG, "IOException caught while closing stream", e);
                    return false;
                }
            }
        }
        return true;
    }

    private File getUniqueDestination(String base, String extension) {
        File file = new File(base + "." + extension);

        for (int i = 2; file.exists(); i++) {
            file = new File(base + "_" + i + "." + extension);
        }
        return file;
    }

    private void showDeliveryReport(long messageId, String type) {
        Intent intent = new Intent(this, DeliveryReportActivity.class);
        intent.putExtra("message_id", messageId);
        intent.putExtra("message_type", type);

        startActivity(intent);
    }

    private static ContactList sEmptyContactList;

    private ContactList getRecipients() {
        // If the recipients editor is visible, the conversation has
        // not really officially 'started' yet.  Recipients will be set
        // on the conversation once it has been saved or sent.  In the
        // meantime, let anyone who needs the recipient list think it
        // is empty rather than giving them a stale one.
        if (isRecipientsEditorVisible()) {
            if (sEmptyContactList == null) {
                sEmptyContactList = new ContactList();
            }
            return sEmptyContactList;
        }
        return mConversation.getRecipients();
    }

    private void updateTitle(ContactList list) {
        String title = null;
        String subTitle = null;
        Drawable avatarIcon = null;
        int cnt = list.size();
        ArrayList<String> titles = new ArrayList<String>();
        switch (cnt) {
            case 0: {
                /// M: only show "new message" string if list size is 0, if a number
                /// end without "," or ";" should not display on title.
                title = getString(R.string.new_message);
                break;
            }
            case 1: {
                title = list.get(0).getName();      // get name returns the number if there's no
                                                    // name available.
                Drawable sDefaultContactImage = this.getResources().getDrawable(
                    R.drawable.ic_contact_picture);
                avatarIcon = list.get(0).getAvatar(this, sDefaultContactImage,
                                                    mConversation != null
                                                    ? mConversation.getThreadId() : -1);
                String number = list.get(0).getNumber();
                String numberAfterFormat;
//                ImageView ipCustomView = (ImageView) mActionBarCustomView
//                        .findViewById(R.id.iv_full_integrated);

                // add for ipmessage
//                if (mIpCompose.onIpUpdateTitle(number, title, ipCustomView, titles)) {
//                    title = titles.get(0);
//                    subTitle = titles.get(1);
//                }

                if (!title.equals(number)) {
                    numberAfterFormat = MessageUtils.formatNumber(number,
                            this.getApplicationContext());
                    if (!title.equals(numberAfterFormat)) {
                        subTitle = numberAfterFormat;
                    }
                }
                /// M: fix bug ALPS00488976, group mms @{
                if (mMsgListAdapter.isGroupConversation()) {
                    mMsgListAdapter.setIsGroupConversation(false);
                }
                /// @}
                break;
            }
            default: {
                // Handle multiple recipients
                avatarIcon = this.getResources().getDrawable(R.drawable.ic_contact_picture);
                title = list.formatNames(", ");
                subTitle = getResources().getQuantityString(R.plurals.recipient_count, cnt, cnt);
                break;
            }
        }
        mDebugRecipients = list.serialize();

        ActionBar actionBar = getActionBar();
        if (actionBar == null) {
            return;
        }
        if (avatarIcon != null) {
            actionBar.setIcon(avatarIcon);
        }

        if (actionBar.getCustomView() == null) {
            actionBar.setCustomView(R.layout.actionbar_message_title);
        }
        mActionBarCustomView = actionBar.getCustomView();
        mTopTitle = (TextView) mActionBarCustomView.findViewById(R.id.tv_top_title);
        mTopSubtitle = (TextView) mActionBarCustomView.findViewById(R.id.tv_top_subtitle);
        String number = null;
        if (!list.isEmpty())
        {
            number = list.get(0).getNumber();
        }
        // add for ipmessage
        ImageView ipCustomView = (ImageView) mActionBarCustomView
                .findViewById(R.id.iv_full_integrated);
        if (mIpCompose.onIpUpdateTitle(number, null, ipCustomView, titles)) {
            title = titles.get(0);
            subTitle = titles.get(1);
        }

        ///M: ALPS00772324 The recipient didn't shown completely after input a recipient's numbers
        ///M: and put the cursor to the text field.
        ///M: setMaxWidth to a very large value for textView can width can be wrap_content
        mTopTitle.setMaxWidth(3000);
        mTopTitle.setText(title);
        if (TextUtils.isEmpty(subTitle)) {
            if (cnt == 0) {
                actionBar.setIcon(R.drawable.ic_launcher_smsmms);
            }
            mTopSubtitle.setVisibility(View.GONE);
        } else {
            mTopSubtitle.setText(subTitle);
            mTopSubtitle.setVisibility(View.VISIBLE);
        }
        actionBar.setDisplayShowCustomEnabled(true);
        actionBar.setDisplayShowTitleEnabled(false);
    }

    // Get the recipients editor ready to be displayed onscreen.
    private void initRecipientsEditor(Bundle bundle) {
        /// M: Code analyze 046, Whether the recipientedit control has been initialized. @{
        if (isRecipientsEditorVisible() && isInitRecipientsEditor) {
        /// @}
            return;
        }
        // Must grab the recipients before the view is made visible because getRecipients()
        // returns empty recipients when the editor is visible.
        ContactList recipients = getRecipients();
        /// M: Code analyze 058, Remove exceed recipients.
        while (!recipients.isEmpty() && recipients.size() > RECIPIENTS_LIMIT_FOR_SMS) {
            recipients.remove(RECIPIENTS_LIMIT_FOR_SMS);
        }
        /// @}

        ViewStub stub = (ViewStub) findViewById(R.id.recipients_editor_stub);
        if (stub != null) {
            View stubView = stub.inflate();
            mRecipientsEditor = (RecipientsEditor) stubView.findViewById(R.id.recipients_editor);
            mRecipientsPicker = (ImageButton) stubView.findViewById(R.id.recipients_picker);
        } else {
            mRecipientsEditor = (RecipientsEditor) findViewById(R.id.recipients_editor);
            mRecipientsEditor.setVisibility(View.VISIBLE);
            mRecipientsPicker = (ImageButton) findViewById(R.id.recipients_picker);
            /// M: Code analyze 059, Set the pick button visible or
            /// invisible the same as recipient editor.
            mRecipientsPicker.setVisibility(View.VISIBLE);
            /// @}
        }
        mRecipientsPicker.setOnClickListener(this);
        mRecipientsEditor.removeChipChangedListener(mChipWatcher);
        mRecipientsEditor.addChipChangedListener(mChipWatcher);

        mRecipientsPicker.setEnabled(mIsSmsEnabled);
        mRecipientsEditor.setEnabled(mIsSmsEnabled);
        mRecipientsEditor.setFocusableInTouchMode(mIsSmsEnabled);
        mRecipientsEditor.setIsTouchable(mIsSmsEnabled);

        // M: indicate contain email address or not in RecipientsEditor candidates. @{
        SharedPreferences prefs =
            PreferenceManager.getDefaultSharedPreferences(ComposeMessageActivity.this);
        boolean showEmailAddress =
            prefs.getBoolean(GeneralPreferenceActivity.SHOW_EMAIL_ADDRESS, true);
        /// M: fix bug ALPS00380930
        if (mRecipientsEditor.getAdapter() == null) {
            ChipsRecipientAdapter chipsAdapter = new ChipsRecipientAdapter(this);
            chipsAdapter.setShowEmailAddress(showEmailAddress);
            mRecipientsEditor.setAdapter(chipsAdapter);
            mRecipientsEditor.setGalSearchDelayer();
        } else {
            ((ChipsRecipientAdapter) mRecipientsEditor
                    .getAdapter()).setShowEmailAddress(showEmailAddress);
        }
        // @}
        if (bundle == null) {
            mRecipientsEditor.populate(new ContactList());
            mRecipientsEditor.populate(recipients);
        }
        mRecipientsEditor.setOnCreateContextMenuListener(mRecipientsMenuCreateListener);
        // TODO : Remove the max length limitation due to the multi phone picker is added and the
        // user is able to select a large number of recipients from the Contacts. The coming
        // potential issue is that it is hard for user to edit a recipient from hundred of
        // recipients in the editor box. We may redesign the editor box UI for this use case.
        mRecipientsEditor.setFilters(new InputFilter[] {
                 new RecipientsTextLengthFilter(RECIPIENTS_MAX_LENGTH) });

        mRecipientsEditor.setOnSelectChipRunnable(new Runnable() {
            public void run() {
                // After the user selects an item in the pop-up contacts list, move the
                // focus to the text editor if there is only one recipient.  This helps
                // the common case of selecting one recipient and then typing a message,
                // but avoids annoying a user who is trying to add five recipients and
                // keeps having focus stolen away.
                if (mRecipientsEditor.getRecipientCount() == 1) {
                    // if we're in extract mode then don't request focus
                    final InputMethodManager inputManager = mInputMethodManager;
                    if (inputManager == null || !inputManager.isFullscreenMode()) {
                        if (mBottomPanel != null && mBottomPanel.getVisibility() == View.VISIBLE) {
                            mTextEditor.requestFocus();
                        }
                    }
                }
            }
        });

        mRecipientsEditor.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (hasFocus) {
                    /// M: add for ip message
                    MmsLog.d(TAG, "onFocusChange(): mRecipientsEditor get focus.");
                    if (mIsLandscape) {
                        mTextEditor.setMaxHeight(
                                mReferencedTextEditorTwoLinesHeight
                                * mCurrentMaxHeight / mReferencedMaxHeight);
                    } else {
                        updateTextEditorHeightInFullScreen();
                    }
                }
                // add for ipmessage
                mIpCompose.onIpRecipientsEditorFocusChange(
                        hasFocus, mRecipientsEditor.getNumbers());
                mOpComposeExt.onRecipientsEditorFocusChange(hasFocus);
            }
        });

        mRecipientsEditor.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                mIpCompose.onIpRecipientsEditorTouch();
                mOpComposeExt.onRecipientsEditorTouch();
                return false;
            }
        });

        mRecipientsEditor.setChipProcessListener(new ChipProcessListener() {
            public void onChipProcessDone() {
                mIsPopulatingRecipients = false;
                if (mIsConvertToastDelayed) {
                    mIsConvertToastDelayed = false;
                    toastConvertInfo(mIsConvertMms);
                }
            }
        });

        // M: fix bug ALPS00355897
        // PhoneNumberFormatter.setPhoneNumberFormattingTextWatcher(this, mRecipientsEditor);

        mTopPanel.setVisibility(View.VISIBLE);
        /// M: Code analyze 013, Get contacts from Contact app . @{
        if (mIsRecipientHasIntentNotHandle && (mIntent != null)) {
            processPickResult(mIntent);
            mIsRecipientHasIntentNotHandle = false;
            mIntent = null;
        }
        /// @}
        mIpCompose.initRecipientsEditor(mRecipientsEditor);
    }

    //==========================================================
    // Activity methods
    //==========================================================

    public static boolean cancelFailedToDeliverNotification(Intent intent, Context context) {
        if (MessagingNotification.isFailedToDeliver(intent)) {
            // Cancel any failed message notifications
            MessagingNotification.cancelNotification(context,
                        MessagingNotification.MESSAGE_FAILED_NOTIFICATION_ID);
            return true;
        }
        return false;
    }

    public static boolean cancelFailedDownloadNotification(Intent intent, Context context) {
        if (MessagingNotification.isFailedToDownload(intent)) {
            // Cancel any failed download notifications
            MessagingNotification.cancelNotification(context,
                        MessagingNotification.DOWNLOAD_FAILED_NOTIFICATION_ID);
            return true;
        }
        return false;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        mIsSmsEnabled = MmsConfig.isSmsEnabled(this);
        super.onCreate(savedInstanceState);

        mSavedInstanceState = savedInstanceState;
        MmsLog.dpi(TAG, "onCreate(): savedInstanceState = " + savedInstanceState);

        if (requestAllPermissions()) {
            MmsLog.d(TAG, "onCreate() requestAllPermissions return !!");
            mNeedRequestPermissins = true;
            return;
        }

        Intent intent = getIntent();
        String action = intent.getAction();
        if ((action != null) && Intent.ACTION_SENDTO.equals(action)) {
            Log.d(TAG, "[onCreate] action: " + action);
            mCreateFromSendTo = true;
            Contact.invalidateCache();
        }

        /// M: If cell dialog is showed before, do not show it. @{
        if (savedInstanceState != null
                && savedInstanceState.getBoolean("cell_dialog_showing", false)) {
            MmsLog.d(TAG, "cell progress dialog is showed before, do not show it now");
            savedInstanceState.putBundle("android:savedDialogs", null);
        }
        /// @}

        // add for ipmessage
        mIpCompose = IpMessageUtils.getIpMessagePlugin(this).getIpComposeActivity();
        // add for op plugin
        mOpComposeExt = OpMessageUtils.getOpMessagePlugin().getOpComposeExt();

        mMessageSubId = (int) Settings.System.getLong(getContentResolver(),
                MmsConfig.SMS_SIM_SETTING, MmsConfig.DEFAULT_SIM_NOT_SET);

        mMessageSubId = (int) MmsConfig.DEFAULT_SIM_NOT_SET;

        mInputMethodManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);

        /// M: Code analyze 002,  If a new ComposeMessageActivity is created, kill old one
        Activity compose = sCompose == null ? null : sCompose.get();
        /// M Fix CR : ALPS01275735, which has mms draft for conversation A, enter it.
        /// change recipients froam A to B, then come a B notification , click the notification,
        /// enter the thread A, it will show wrong composer UI.
        /// because conversation from cache is the same. @{
        if (compose != null && compose instanceof ComposeMessageActivity) {
            Intent in = this.getIntent();
            /// M : Fix issue ALPS01767850 @{
            if (in.getBooleanExtra("from_notification", false)) {
                boolean b = ((ComposeMessageActivity) compose).hasValidRecipient();
                if (b) {
                    mLastThreadIdFromNotification
                            = ((ComposeMessageActivity) compose).mConversation.ensureThreadId();
                    if (!((ComposeMessageActivity) compose).mWorkingMessage.hasAttachment()) {
                        sTextEditorText
                                = ((ComposeMessageActivity) compose)
                                        .mWorkingMessage.getText().toString();
                    } else {
                        sTextEditorText = null;
                    }
                }
            }
            /// @}

            long thid = in.getLongExtra("thread_id_from_notification", -1);
            if (((ComposeMessageActivity) compose).mConversation != null) {
                if (thid == ((ComposeMessageActivity) compose).mConversation.getThreadId()) {
                    ((ComposeMessageActivity) compose).mConversation.clearThreadId();
                }
            }
        }
        /// @}

        if (compose != null && !compose.isFinishing() && savedInstanceState == null) {
            MmsLog.d(TAG, "onCreate finish old compose:" + compose);
            compose.finish();
        }
        sCompose = new WeakReference(this);
        /// @}
        /// M: Code analyze 003,  Set or get max mms size.
        initMessageSettings();
        /// @}
        resetConfiguration(getResources().getConfiguration());

        setContentView(R.layout.compose_message_activity);
        setProgressBarVisibility(false);

        // Initialize members for UI elements.
        initResourceRefs();

        /// M: Code analyze 004, Set max height for text editor. @{
        WindowManager windowManager = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
        if (mIsLandscape) {
            mCurrentMaxHeight = windowManager.getDefaultDisplay().getWidth();
        } else {
            mCurrentMaxHeight = windowManager.getDefaultDisplay().getHeight();
        }
        /// @}

        mIpCompose.onIpComposeActivityCreate(this, mIpComposeCallback, mHandler, mUiHandler,
                (ImageButton) findViewById(R.id.send_button_sms),
                (TextView) findViewById(R.id.ip_message_typing_status),
                (TextView) findViewById(R.id.ipmsg_joyn_stranger_remind),
                findViewById(R.id.bottom_panel), savedInstanceState,
                (ImageButton) findViewById(R.id.share_button),
                (LinearLayout) findViewById(R.id.panel_container),
                (EditText) findViewById(R.id.embedded_text_editor));

        /// M: add for update sub state dynamically. @{
        IntentFilter intentFilter =
            new IntentFilter(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mOpComposeExt.onCreate(mIpComposeCallback, getIntent(), intentFilter, compose, this,
                savedInstanceState, mUiHandler, (ImageButton) findViewById(R.id.share_button),
                (LinearLayout) findViewById(R.id.panel_container),
                (EditText) findViewById(R.id.embedded_text_editor));
        /// @}

        /// M: Code analyze 005, Set input mode. @{
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
        /// @}

        /// M: Code analyze 006, Control Sub indicator on status bar. @{
        mStatusBarManager = (StatusBarManager) getSystemService(Context.STATUS_BAR_SERVICE);
        mComponentName = getComponentName();
        /// @}

        /// M: Code analyze 007, Get information from Sub or save message to Sub. @{
        mSubCount = 0;
        /// @}

        mContentResolver = getContentResolver();
        mBackgroundQueryHandler = new BackgroundQueryHandler(mContentResolver);

        initialize(savedInstanceState, 0, false);

        if (TRACE) {
            android.os.Debug.startMethodTracing("compose");
        }

        /// @}
        /// M: Code analyze 009,Show attachment dialog . @{
        mSoloAlertDialog = new SoloAlertDialog(this);
        /// @}
        /// M: Code analyze 007, Get information from Sub or
        /// save message to Sub.(Get all Sub info) @{
        mGetSubInfoRunnable.run();
        /// @} for ALPS00475359 end

        this.registerReceiver(mSubReceiver, intentFilter);
        /// @}
        IntentFilter shutDownFilter = new IntentFilter();
        shutDownFilter.addAction(Intent.ACTION_SHUTDOWN);
        this.registerReceiver(mShutDownReceiver, shutDownFilter);
    }

    private void showSubjectEditor(boolean show) {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            log("showSubjectEditor: " + show);
        }

        if (mSubjectTextEditor == null) {
            // Don't bother to initialize the subject editor if
            // we're just going to hide it.
            if (show == false) {
                return;
            }
            mSubjectTextEditor = (EditText) findViewById(R.id.subject);
            /// M: Code analyze 068, Unknown. Why delete these code? @{
            /// mSubjectTextEditor.setFilters(new InputFilter[] {
            ///     new LengthFilter(MmsConfig.getMaxSubjectLength())});
            /// @}
            /// M: Code analyze 001, Plugin opeartor. @{
            mOpComposeExt.showSubjectEditor(mSubjectTextEditor);
            /// @}
       }

        mSubjectTextEditor.setOnKeyListener(show ? mSubjectKeyListener : null);

        mSubjectTextEditor.removeTextChangedListener(mSubjectEditorWatcher);
        if (show) {
            mSubjectTextEditor.addTextChangedListener(mSubjectEditorWatcher);
        }

        if ((mBottomPanel != null) && (mBottomPanel.getVisibility() == View.VISIBLE)) {
            if (!show) {
                mTextEditor.requestFocus();
            }
            //mSubjectTextEditor.setNextFocusDownId(R.id.embedded_text_editor);
            mSubjectTextEditor.setImeOptions(EditorInfo.IME_ACTION_NEXT);
            mSubjectTextEditor.setImeActionLabel(getText(
                    com.android.internal.R.string.ime_action_next), EditorInfo.IME_ACTION_NEXT);
        } else {
            //mSubjectTextEditor.setNextFocusDownId(-1);
            mSubjectTextEditor.setImeOptions(EditorInfo.IME_ACTION_DONE);
            mSubjectTextEditor.setImeActionLabel(
                    getText(com.android.internal.R.string.ime_action_done),
                    EditorInfo.IME_ACTION_DONE);
        }
        CharSequence subjectText = mWorkingMessage.getSubject();
        if (subjectText != null && show) {
            mSubjectTextEditor.setTextKeepState(subjectText);
            try {
                mSubjectTextEditor.setSelection(mSubjectTextEditor.getText().toString().length());
            } catch (IndexOutOfBoundsException e) {
                mSubjectTextEditor.setSelection(
                        mSubjectTextEditor.getText().toString().length() - 1);
            }
        } else {
            mSubjectTextEditor.setText("");
        }

        mSubjectTextEditor.setVisibility(show ? View.VISIBLE : View.GONE);

        mSubjectTextEditor.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                mIpCompose.onIpSubjectTextEditorTouch();
                mOpComposeExt.onSubjectTextEditorTouch();
                return false;
            }
        });

        mSubjectTextEditor.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                mIpCompose.onIpSubjectTextEditorFocusChange(hasFocus);
                mOpComposeExt.onSubjectTextEditorFocusChange(hasFocus);
            }
        });

        mSubjectTextEditor.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_NEXT) {
                    if ((mBottomPanel != null) && (mBottomPanel.getVisibility() == View.VISIBLE)) {
                        mTextEditor.requestFocus();
                        return true;
                    }
                }
                return false;
            }
        });
        hideOrShowTopPanel();
    }

    private void hideOrShowTopPanel() {
        boolean anySubViewsVisible = (isSubjectEditorVisible() || isRecipientsEditorVisible());
        mTopPanel.setVisibility(anySubViewsVisible ? View.VISIBLE : View.GONE);
    }

    /// M : Fix issue ALPS01767850 @{
    /// If there a mms draft with thread id 1, and create a new sms message with threadid 1
    /// Then send a threadid 1 message, and click the notification to enter the 1 thread
    /// The sms message will disapre which just edit in the editor box
    private boolean needLoadDraftWhileFromNotification(Intent intent) {
        if (intent == null) {
            return true;
        }
        // check from notification or not
        boolean isFromNotification = intent.getBooleanExtra("from_notification", false);

        if (!isFromNotification) {
            Log.d(TAG,
                    "[needLoadDraftWhileFromNotification] NOT from notification, need load draft");
            return true;
        }
        intent.putExtra("from_notification", false);
        // check exist text or not
        if (sTextEditorText == null || sTextEditorText.trim().length() == 0) {
            Log.d(TAG, "[needLoadDraftWhileFromNotification] NO" +
                    " EXIST text in the editor, need load draft");
            return true;
        }
        // check the thread id from notification and the thread id from last compose is the same
        boolean isSameThread = false;

        long threadIdFromNotification = intent.getLongExtra("thread_id_from_notification", -1);

        if (mLastThreadIdFromNotification == -1 || threadIdFromNotification == -1) {
            isSameThread = false;
        } else if (mLastThreadIdFromNotification == threadIdFromNotification) {
            isSameThread = true;
        }

        Log.d(TAG, "[needLoadDraftWhileFromNotification] mLastThreadIdFromNotification : "
                + mLastThreadIdFromNotification
                + ", threadIdFromNotification : " + threadIdFromNotification);

        if (!isSameThread) {
            Log.d(TAG, "[needLoadDraftWhileFromNotification] NOT the same thread, load draft");
            return true;
        }

        mLastThreadIdFromNotification = -1;
        Log.d(TAG, "[needLoadDraftWhileFromNotification] return false");

        return false;
    }
    ///@}

    public void initialize(Bundle savedInstanceState, long originalThreadId,
                           boolean fromPremissionresult) {
        Log.d(TAG, "initialize begin");
        /// M: Code analyze 010, Support dirtory mode. @{
        Intent intent = getIntent();
        // for intent from other apps, etc. call, not check dir mode
        String act = intent.getAction();
        if (act != null && act.equals(Intent.ACTION_SENDTO)) {
            externalIntent = true;
        } else {
            externalIntent = false;
        }
        boolean showInput = false;
        boolean hiderecipient = false;
        boolean isMustRecipientEditable = false;

        // Create a new empty working message.
        mWorkingMessage = WorkingMessage.createEmpty(this);

        // add for ipmessage
//        mIpCompose.onIpInitialize(intent, mWorkingMessage);

        // Read parameters or previously saved state of this activity. This will load a new
        // mConversation
        initActivityState(savedInstanceState);

        if (LogTag.SEVERE_WARNING && originalThreadId != 0 &&
                originalThreadId == mConversation.getThreadId()) {
            LogTag.warnPossibleRecipientMismatch("ComposeMessageActivity.initialize: " +
                    " threadId didn't change from: " + originalThreadId, this);
        }

        MmsLog.dpi(TAG, "savedInstanceState = " + savedInstanceState +
            ", intent = " + intent +
            ", originalThreadId = " + originalThreadId);
        MmsLog.dpi(TAG, "mConversation = " + mConversation);

        /// M: Code analyze 010, Support dirtory mode. @{
        /* P0 migration*/
//        if (!FolderModeUtils.getMmsDirMode()) {
            if (cancelFailedToDeliverNotification(getIntent(), this)
                    && savedInstanceState == null) {
                // Show a pop-up dialog to inform user the message was
                // failed to deliver.
                undeliveredMessageDialog(getMessageDate(null));
            }
            cancelFailedDownloadNotification(getIntent(), this);
//        }
        ///  @}
        // Set up the message history ListAdapter
        initMessageList();
        /// M: fix bug for ConversationList select all performance ,update selected thread array.@{
        mSelectedThreadId = mConversation.getThreadId();
        /// @}
        // Load the draft for this thread, if we aren't already handling
        // existing data, such as a shared picture or forwarded message.
        boolean isForwardedMessage = false;
        // We don't attempt to handle the Intent.ACTION_SEND when saveInstanceState is non-null.
        // saveInstanceState is non-null when this activity is killed. In that case, we already
        // handled the attachment or the send, so we don't try and parse the intent again.
        boolean intentHandled = savedInstanceState == null &&
        /// M: unknown @{
            (handleSendIntent() || (handleForwardedMessage() && !mConversation.hasDraft()));
        /// @}
/*
        /// M: ALPS03562153 @{
        String action = intent.getAction();
        if ((action != null) && Intent.ACTION_SENDTO.equals(action)) {
            Log.d(TAG, "[initialize] action: " + action);
            mCreateFromSendTo = true;
        }
        /// @}
*/
        /// M : Fix issue ALPS01767850
        boolean need = needLoadDraftWhileFromNotification(intent);
        if (!intentHandled && need) {
            MmsLog.d(TAG, "Composer init load Draft.");
            loadDraft();
        } else if (!need) {
            mTextEditor.setText(sTextEditorText);
        }

        sTextEditorText = null;

        // Let the working message know what conversation it belongs to
        mWorkingMessage.setConversation(mConversation);

        // Show the recipients editor if we don't have a valid thread. Hide it otherwise.
        /// M: @{
        //  if (mConversation.getThreadId() <= 0) {
        if (mConversation.getThreadId() <= 0L
            || (mConversation.getMessageCount() <= 0
                    && (intent.getAction() != null || mConversation.hasDraft()))
            || (mConversation.getThreadId() > 0L && mConversation.getMessageCount() <= 0)
            || isMustRecipientEditable) {
         /// @}
            // Hide the recipients editor so the call to initRecipientsEditor won't get
            // short-circuited.
            hideRecipientEditor();
            if (fromPremissionresult == true) {
                initRecipientsEditor(null);
            } else {
                initRecipientsEditor(savedInstanceState);
            }

            // Bring up the softkeyboard so the user can immediately enter recipients. This
            // call won't do anything on devices with a hard keyboard.
            if (mIsSmsEnabled) {
                getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE |
                    WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
            }
        } else {
            hideRecipientEditor();
        }
        /// M: @{

        invalidateOptionsMenu();    // do after show/hide of recipients editor because the options
                                    // menu depends on the recipients, which depending upon the
                                    // visibility of the recipients editor, returns a different
                                    // value (see getRecipients()).

    //    updateSendButtonState();

        drawTopPanel(false);
        if (intentHandled) {
            // We're not loading a draft, so we can draw the bottom panel immediately.
            drawBottomPanel();
        }

        onKeyboardStateChanged();

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "update title, mConversation=" + mConversation.toString());
        }

        // no longer update title in onResume, so set "New Message" here.
        updateTitle(new ContactList());

        if (isForwardedMessage && isRecipientsEditorVisible()) {
            // The user is forwarding the message to someone. Put the focus on the
            // recipient editor rather than in the message editor.
            mRecipientsEditor.requestFocus();
        }

        /// M: google JB.MR1 patch, group mms
        boolean isGroupMms = mConversation.getRecipients().size() > 1;
        mMsgListAdapter.setIsGroupConversation(isGroupMms);
        mIpCompose.onIpInitialize(intent, mWorkingMessage);
        Log.d(TAG, "initialize end");
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);

        if (requestAllPermissions()) {
            mNeedRequestPermissins = true;
            return;
        }

        MmsLog.dpi(TAG, "onNewIntent: intent = " + intent.toString());
      /// M: add for fix ALPS01317511 @{
        /* P0 migration*/
        if (!mOpComposeExt.onNewIntent(intent, mHadToSlideShowEditor, mTempMmsUri, mWorkingMessage,
                this, mRecipientsEditor, 0/*FolderViewList.OPTION_INBOX*/)) {
            processNewIntent(intent);
        }
        /// @}
    }

   private void processNewIntent(Intent intent) {
        MmsLog.dpi(TAG, "processNewIntent: intent = " + intent.toString());
        setIntent(intent);
        /// M: reload working message if it is not correct.
        if (!mOpComposeExt.processNewIntent(intent)) {
            if (!intent.getBooleanExtra(FORWARD_MESSAGE, false)) {
                reloadWorkingMessage();
                resetConfiguration(getResources().getConfiguration());
            }
        }

        mWaitingAttachment = false;

        if (mQuickTextDialog != null && mQuickTextDialog.isShowing()) {
            mQuickTextDialog.dismiss();
            mQuickTextDialog = null;
        }

        if (mSubSelectDialog != null && mSubSelectDialog.isShowing()) {
            mSubSelectDialog.dismiss();
            mSubSelectDialog = null;
        }

        Conversation conversation = null;
        mSentMessage = false;

        // If we have been passed a thread_id, use that to find our
        // conversation.

        // Note that originalThreadId might be zero but if this is a draft and we save the
        // draft, ensureThreadId gets called async from WorkingMessage.asyncUpdateDraftSmsMessage
        // the thread will get a threadId behind the UI thread's back.
        long originalThreadId = mConversation.getThreadId();
        long threadId = intent.getLongExtra("thread_id", 0);
        Uri intentUri = intent.getData();

        boolean sameThread = false;
        if (threadId > 0) {
            conversation = Conversation.get(getApplicationContext(), threadId, false);
        } else {
            if (mConversation.getThreadId() == 0) {
                // We've got a draft. Make sure the working recipients are synched
                // to the conversation so when we compare conversations later in this function,
                // the compare will work.
                mWorkingMessage.syncWorkingRecipients();
            }
            // Get the "real" conversation based on the intentUri. The intentUri might specify
            // the conversation by a phone number or by a thread id. We'll typically get a threadId
            // based uri when the user pull down a notification while in ComposeMessageActivity and
            // we end up here in onNewIntent. mConversation can have a threadId of zero when we're
            // working on a draft. When a new message comes in for that same recipient, a
            // conversation will get created behind CMA's back when the message is inserted into
            // the database and the corresponding entry made in the threads table. The code should
            // use the real conversation as soon as it can rather than finding out the threadId
            // when sending with "ensureThreadId".
            conversation = Conversation.get(getApplicationContext(),
                    intentUri, false, Conversation.TYPE_MMSSMS);
        }

        /// M: Fix bug: ALPS00444760, The keyboard display under the MMS after
        // you tap shortcut enter this thread.
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
        /// @}

        ///M @{
        if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "onNewIntent: data=" + intentUri + ", thread_id extra is " + threadId +
                    ", new conversation=" + conversation + ", mConversation=" + mConversation);
        }

        // this is probably paranoid to compare both thread_ids and recipient lists,
        // but we want to make double sure because this is a last minute fix for Froyo
        // and the previous code checked thread ids only.
        // (we cannot just compare thread ids because there is a case where mConversation
        // has a stale/obsolete thread id (=1) that could collide against the new thread_id(=1),
        // even though the recipient lists are different)
        sameThread = ((conversation.getThreadId() == mConversation.getThreadId() ||
                mConversation.getThreadId() == 0) &&
                conversation.equals(mConversation));

        if (sameThread) {
            log("onNewIntent: same conversation");
            /// M Fix CR:ALPS01081972 which cannot show received message in the composer @{
            if (intent.getBooleanExtra("finish", false) == true) {
                Activity compose = sCompose == null ? null : sCompose.get();
                if (compose != null && !compose.isFinishing()) {
                    Log.d("[Mms][ComposeMessageActivity]",
                            "[onNewIntent] same conversation call finish composer");
                    compose.finish();
                    this.startActivity(intent);
                    return;
                }
            }
            /// @}
            if (mConversation.getThreadId() == 0) {
                mConversation = conversation;
                mWorkingMessage.setConversation(mConversation);
                updateThreadIdIfRunning();
                invalidateOptionsMenu();
            }
        } else {
            if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                log("onNewIntent: different conversation");
            }
            /// M: @{
            /// M: Don't let any markAsRead DB updates occur before we've loaded the messages for
            /// M: the thread.
            conversation.blockMarkAsRead(true);
            /// @}
            if ((!isRecipientsEditorVisible())
                    || (mRecipientsEditor.hasValidRecipient(mWorkingMessage.requiresMms()))) {
                    //For ALPS00457252
                    if (!mWorkingMessage.isWorthSaving()) {
                        mWorkingMessage.discard();
                    } else {
                        saveDraft(false); // if we've got a draft, save it first
                    }
            }
            /// if is not the same thread, to cancel progress dialog @{
            mUiHandler.removeCallbacks(this.mContactPickRunnable);
            if (mContactPickDialog != null && mContactPickDialog.isShowing()) {
                mContactPickDialog.dismiss();
            }
            mContactPickDialog = null;
            /// @}
            if (mAsyncDialog != null) {
                mAsyncDialog.dismissProgressDialog();
            }
            /// if from shortcut, finish origin compoer, and start again @{
            if (intent.getBooleanExtra("finish", false) == true) {
                Activity compose = sCompose == null ? null : sCompose.get();
                if (compose != null && !compose.isFinishing()) {
                    Log.d("[Mms][ComposeMessageActivity]", "[onNewIntent] call finish composer");
                    compose.finish();
                    this.startActivity(intent);
                    return;
                }
            }
            /// @}
            /// M: @{
            mMsgListAdapter.changeCursor(null);
            mConversation = conversation;
            /// @}

            initialize(null, originalThreadId, false);
            /// M: add for attach dialog do not dismiss when enter other thread. @{
            if (!mSoloAlertDialog.needShow()) {
                mSoloAlertDialog.dismiss();
            }
            /// @}
            /// M: fix bug ALPS00941735
            mIsSameConv = false;
            MmsLog.d(TAG, "onNewIntent not same thread");
        }
        loadMessageContent();

        if (mMsgListAdapter != null && mMsgListAdapter.getCount() == 0) {
            showKeyBoard(true);
            if (mRecipientsEditor != null) {
                mRecipientsEditor.requestFocus();
            }
        }
   }

    private void sanityCheckConversation() {
        if (mWorkingMessage.getConversation() != mConversation) {
            LogTag.warnPossibleRecipientMismatch(
                    "ComposeMessageActivity: mWorkingMessage.mConversation=" +
                    mWorkingMessage.getConversation() + ", mConversation=" +
                    mConversation + ", MISMATCH!", this);
        }
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        MmsLog.dpi(TAG, "onRestart() begin");

        if (mNeedRequestPermissins) {
            MmsLog.d(TAG, "onRestart() mNeedRequestPermissins return !!" + mNeedRequestPermissins);
            return;
        }
        if (requestAllPermissions()) {
            MmsLog.d(TAG, "onRestart() requestAllPermissions return !!");
            mNeedRequestPermissins = true;
            return;
        }

         /// M: For fix bug ALPS00542156 The "_" still display under the SubA after
        /// you change it to the "Always ask" or "SubB.@{
        sCompose = null;
        sCompose = new WeakReference(this);
        /// @}

        if (mWorkingMessage != null && mWorkingMessage.isDiscarded()) {
            // If the message isn't worth saving, don't resurrect it. Doing so can lead to
            // a situation where a new incoming message gets the old thread id of the discarded
            // draft. This activity can end up displaying the recipients of the old message with
            // the contents of the new message. Recognize that dangerous situation and bail out
            // to the ConversationList where the user can enter this in a clean manner.
            mWorkingMessage.unDiscard();    // it was discarded in onStop().
            if (mWorkingMessage.isWorthSaving()) {
                if (LogTag.VERBOSE) {
                    log("onRestart: mWorkingMessage.unDiscard()");
                }
                //mWorkingMessage.unDiscard();    // it was discarded in onStop().

                sanityCheckConversation();
            } else if (isRecipientsEditorVisible()) {
                if (LogTag.VERBOSE) {
                    log("onRestart: goToConversationList");
                }
                goToConversationList();
            } else {
                if (LogTag.VERBOSE) {
                    log("onRestart: loadDraft");
                }
                /// M: @{
                //loadDraft();
                /// @}
                mWorkingMessage.setConversation(mConversation);
                mAttachmentEditor.update(mWorkingMessage);
                updateTextEditorHeightInFullScreen();
                invalidateOptionsMenu();
            }
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        MmsLog.d(TAG, "onStart() enter!!");

        if (mNeedRequestPermissins) {
            return;
        }

        mIsActivityStoped = false;
        boolean isSmsEnabled = MmsConfig.isSmsEnabled(this);
        if (isSmsEnabled != mIsSmsEnabled) {
            mIsSmsEnabled = isSmsEnabled;
            invalidateOptionsMenu();
        }
        /// M: For fix bug ALPS00688631 The "_" still display under the SubA after
        /// you change it to the "Always ask" or "SubB.@{
        sCompose = null;
        sCompose = new WeakReference(this);
        /// @}
        /// M Fix CR : ALPS01257113, when sub select dialog showed in default sms. after change to
        /// non-default sms, dismiss the sub select dialog @{
        if (!mIsSmsEnabled) {
            if (mSubSelectDialog != null && mSubSelectDialog.isShowing()) {
                mSubSelectDialog.dismiss();
            }
        }
        /// @}

        /// M: Code analyze 036, Change text size if adjust font size.@{
        float textSize = MessageUtils.getPreferenceValueFloat(this,
                SettingListActivity.TEXT_SIZE, 18);
        setTextSize(textSize);
        /// @}

        /// M: Code analyze 013, Get contacts from Contact app . @{
        misPickContatct = false;
        /// @}
        // M:for ALPS01065027,just for compose messagelist in scrolling
        mMsgListView.setOnScrollListener(mScrollListener);
        mScrollListener.setIsNeedRefresh(true);
        mScrollListener.setThreadId(mConversation.getThreadId(), ComposeMessageActivity.this);
        initFocus();

        mOpComposeExt.onStart(this, this);
        loadMessageContent();

        // Update the fasttrack info in case any of the recipients' contact info changed
        // while we were paused. This can happen, for example, if a user changes or adds
        // an avatar associated with a contact.
        /// M: @{
        if (mConversation.getThreadId() == 0) {
            mWorkingMessage.syncWorkingRecipients();
        }
        /// @}

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "onStart: update title, mConversation=" + mConversation.toString());
        }

        ActionBar actionBar = getActionBar();
        /// M: Add log. @{
        if (actionBar == null) {
            MmsLog.e(TAG, "ACTION BAR is null, window feature FEATURE_ACTION_BAR: "
                    + getWindow().hasFeature(Window.FEATURE_ACTION_BAR)
                    + ", feature FEATURE_NO_TITLE: "
                    + getWindow().hasFeature(Window.FEATURE_NO_TITLE));
        } else {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
        /// @}


        if (isRecipientsEditorVisible() && mIsSmsEnabled) {
            boolean isLandscape = getResources().getConfiguration().orientation
                    == Configuration.ORIENTATION_LANDSCAPE;
            boolean isMultiWindow = isInMultiWindowMode();
            if (!isLandscape || isMultiWindow) {
                getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE |
                        WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
            }
        }

        if (mMsgListAdapter != null) {
            mMsgListAdapter.setOnDataSetChangedListener(mDataSetChangedListener);
        }
        addRecipientsListeners();
    }

    public void loadMessageContent() {
        // Don't let any markAsRead DB updates occur before we've loaded the messages for
        // the thread. Unblocking occurs when we're done querying for the conversation
        // items.
        MmsLog.d(TAG, "loadMessageContent()");
        ///M: for fix ALPS01026162, for folder mode, no content loaded.must not mark as read. @{
        /* P0 migration*/
//        if (!FolderModeUtils.getMmsDirMode()) {
            mConversation.blockMarkAsRead(true);
            mConversation.markAsRead();         // dismiss any notifications for this convo
            /// M: mark conversation as seen, update new messages notification.
            /// M: fix bug ALPS01065220. onStart() will update message seen = 1.
            /// But if before update start,
            /// do not update it.{@
            mConversation.setComposeIsPaused(false);
            /// @}
            mConversation.markAsSeen();
//        }
        /// @}
        startMsgListQuery(MESSAGE_LIST_QUERY_TOKEN, 0);
        updateSendFailedNotification();
        updateDownloadFailedNotification();
        drawBottomPanel();
    }

    private void updateSendFailedNotification() {
        final long threadId = mConversation.getThreadId();
        if (threadId <= 0)
            return;

        // updateSendFailedNotificationForThread makes a database call, so do the work off
        // of the ui thread.
        new Thread(new Runnable() {
            @Override
            public void run() {
                MessagingNotification.updateSendFailedNotificationForThread(
                        ComposeMessageActivity.this, threadId);
            }
        }, "ComposeMessageActivity.updateSendFailedNotification").start();
    }

    private void updateDownloadFailedNotification() {
        final long threadId = mConversation.getThreadId();
        if (threadId <= 0)
            return;

        // updateSendFailedNotificationForThread makes a database call, so do the work off
        // of the ui thread.
        new Thread(new Runnable() {
            @Override
            public void run() {
                MessagingNotification.updateDownloadFailedNotificationForThread(
                        ComposeMessageActivity.this, threadId);
            }
        }, "ComposeMessageActivity.updateSendFailedNotificationForThread").start();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        outState.setClassLoader(ComposeMessageActivity.class.getClassLoader());
        /// M: Code analyze 033, Save some useful information in order to restore the draft when
        /// activity restarting.@{
        // super is commented for a bug work around see MessageUtils.showErrorDialog()
        super.onSaveInstanceState(outState); //why delete this line?
        if (mNeedRequestPermissins) {
            return;
        }
        String number = "";

        // save recipients of this coversation
        if (mRecipientsEditor != null && isRecipientsEditorVisible()) {
            // TODO need re-coding for below code
            //outState.putString("recipients", mRecipientsEditor.allNumberToString());
            /// M: We are compressing the image, so save the thread id in order to restore the
            /// M: draft when activity restarting.
            if (mCompressingImage) {
                outState.putLong("thread", mConversation.ensureThreadId());
            } else if (mRecipientsEditor.getRecipientCount() < 1) {
                outState.putLong("thread", mConversation.ensureThreadId());
                if ((mBackgroundQueryHandler != null) && (mConversation.getThreadId() > 0)) {
                    Conversation.asyncDeleteObsoleteThreadID(
                            mBackgroundQueryHandler, mConversation.getThreadId());
                }
            } else if (mRecipientsEditor.getRecipientCount() > 0) {
                ArrayList<String> numbers = (ArrayList<String>) (mRecipientsEditor.getNumbers());
                outState.putString("recipients", TextUtils.join(";", numbers.toArray()));
            }
        } else {
            /// M: save the current thread id
            outState.putLong("thread", mConversation.getThreadId());
        }
        outState.putBoolean("compressing_image", mCompressingImage);
        // add for ipmessage
        mIpCompose.onIpSaveInstanceState(outState, mConversation.getThreadId());
        /// @}
        mWorkingMessage.writeStateToBundle(outState);

        if (mExitOnSent) {
            outState.putBoolean(KEY_EXIT_ON_SENT, mExitOnSent);
        }
        if (mForwardMessageMode) {
            outState.putBoolean(KEY_FORWARDED_MESSAGE, mForwardMessageMode);
        }
        if (!mAppendAttachmentSign) {
            MmsLog.d(TAG, "onSaveInstanceState mAppendAttachmentSign : " + mAppendAttachmentSign);
            outState.putBoolean(KEY_APPEND_MESSAGE, mAppendAttachmentSign);
        }
        /// M: save cell progress dialog is showing or not
        outState.putBoolean("cell_dialog_showing", mIsCellDialogShowing);
    }

    @Override
    protected void onResume() {
        /// M: fix bug ALPS00444752, set false to enable to Show ContactPicker
        mShowingContactPicker = false;
        super.onResume();
        if (mNeedRequestPermissins) {
            return;
        }

        mNeedSaveDraftAfterStop = false;
        initFocus();
        getAsyncDialog().resetShowProgressDialog();
         /// M: Code analyze 005, Set input mode. @{
        Configuration config = getResources().getConfiguration();
        boolean isLandscape = getResources().getConfiguration().orientation
                == Configuration.ORIENTATION_LANDSCAPE;
        boolean isMultiWindow = isInMultiWindowMode();
        MmsLog.d(TAG, "onResume, isLandscape=" + isLandscape + ", isMultiWindow=" + isMultiWindow);
        if (isLandscape && !isMultiWindow) {
            getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE |
                    WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        }
        /// @}

        /// M: Code analyze 024, If the click operator can be responsed. @{
        //  button can response to start other activity
        mClickCanResponse = true;
        /// @}
        /// M: Code analyze 038, If the user is editing slideshow now.
        /// Do not allow the activity finish but return directly when back key is entered. @{
        mIsEditingSlideshow = false;
        /// @}
        String text = null;
        if (mWorkingMessage != null && mWorkingMessage.getText() != null) {
            text = mWorkingMessage.getText().toString();
        }

        mOpComposeExt.initResourceRefsOnResume(
                        (LinearLayout) findViewById(R.id.button_with_counter),
                        null,
                        mAttachmentEditor.mOpAttachmentEditorExt);

        mOpComposeExt.onResume(mSubCount, text, mIsSmsEnabled,
                mRecipientsEditor, mSubjectTextEditor);

        if (mDrawBottomPanel) {
            drawBottomPanel();
        }
        // OLD: get notified of presence updates to update the titlebar.
        // NEW: we are using ContactHeaderWidget which displays presence, but updating presence
        //      there is out of our control.
        //Contact.startPresenceObserver();

        //addRecipientsListeners();

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "onResume: update title, mConversation=" + mConversation.toString());
        }

        // There seems to be a bug in the framework such that setting the title
        // here gets overwritten to the original title.  Do this delayed as a
        // workaround.
        if (!isRecipientsEditorVisible()) {
            new Thread(new Runnable() {
                public void run() {
                    /// M: Fix CR ALPS00558119
                    /// When the Contact phone is long enough ,after add audio,
                    /// The title will show only phony number @{
                    ///@M:fix bug ALPS00871320 tablet has wide space to show more contact
                    int limite = getLimitedContact();
                    ///@
                    final ContactList recipients = getRecipients();
                    int min = Math.min(limite, recipients.size());
                    for (int i = 0; i < min; i++) {
                        recipients.get(i).reload();
                    }
                    mMessageListItemHandler.postDelayed(new Runnable() {
                        public void run() {
                            updateTitle(recipients);
                        }
                    }, 10);
                }
            }).start();
        }

        mIsRunning = true;
        updateThreadIdIfRunning();

        // add for ipmessage
        mIpCompose.onIpComposeActivityResume(mIsSmsEnabled,
                mTextEditor, mTextEditorWatcher, mTextCounter,
                mRecipientsEditor, mSubjectTextEditor);
        /// M: just update state but didn't notify will miss some mms/sms switch toast.
        //mWorkingMessage.updateStateForGroupMmsChanged();

        updateSendButtonState();

        if (mOldThreadID == -1 && mConversation != null) {
            mOldThreadID = mConversation.getThreadId();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        MmsLog.d(TAG, "onPause() enter, activity=" + ComposeMessageActivity.this);
        if (mNeedRequestPermissins) {
            return;
        }
        /// M: fix bug ALPS00421362. Allow any blocked calls to update the thread's read status.
        /// M: fix bug ALPS01026162.for folder mode, no content loaded.must not mark as read.
        if (this.isFinishing() && !isRecipientsEditorVisible()
            /* P0 migration*/
                /*&& !FolderModeUtils.getMmsDirMode()*/) {
            mConversation.blockMarkAsRead(false);
            mConversation.markAsRead();
        }

        mDrawBottomPanel = true;

        // OLD: stop getting notified of presence updates to update the titlebar.
        // NEW: we are using ContactHeaderWidget which displays presence, but updating presence
        //      there is out of our control.
        //Contact.stopPresenceObserver();
        //removeRecipientsListeners();

        // remove any callback to display a progress spinner
        if (mAsyncDialog != null) {
            mAsyncDialog.clearPendingProgressDialog();
        }

        // Remember whether the list is scrolled to the end when we're paused so we can rescroll
        // to the end when resumed.
        if (mMsgListAdapter != null &&
                mMsgListView.getLastVisiblePosition() >= mMsgListAdapter.getCount() - 1) {
            mSavedScrollPosition = Integer.MAX_VALUE;
        } else {
            mSavedScrollPosition = mMsgListView.getFirstVisiblePosition();
        }
        if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            Log.v(TAG, "onPause: mSavedScrollPosition=" + mSavedScrollPosition);
        }

        mIsRunning = false;
        mErrorDialogShown = true;

        // add for ipmessage
        mIpCompose.onIpComposeActivityPause();

        /// M: Stop in conversation notification sound
        MessagingNotification.stopInConversationNotificationSound();
        /// @}

        checkObsoleteThreadId();
    }

    @Override
    protected void onStop() {
        super.onStop();

        if (mNeedRequestPermissins) {
            MmsLog.d(TAG, "onStop() no permissions");
            return;
        }

        mIsActivityStoped = true;
//        mContentResolver.unregisterContentObserver(mDraftChangeObserver);
        /// M: fix bug ALPS01065220. onStart() will update message seen = 1.
        /// But if before update start,
        /// do not update it.{@
        mConversation.setComposeIsPaused(true);
        /// @}
        /// M: Code analyze 013, Get contacts from Contact app . @{
        if (misPickContatct) {
            return;
        }
        /// @}
        // Allow any blocked calls to update the thread's read status.
      /// M: fix bug ALPS01026162.for folder mode, no content loaded.must not mark as read.
      /* P0 migration*/
        if (!isRecipientsEditorVisible()) {
            mConversation.blockMarkAsRead(false);
            mConversation.markAsRead();
        }

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            log("onStop: save draft");
        }
        MessagingNotification.setCurrentlyDisplayedThreadId(MessagingNotification.THREAD_NONE);
        /// M: If image is being compressed, wait for it
        if (isFinishing()) {
            waitForCompressing();
        }
        /// M: @{
        if ((!isRecipientsEditorVisible()) ||
                (mRecipientsEditor.hasValidRecipient(mWorkingMessage.requiresMms()))) {
            if (isWorthSaving() && MessageUtils.allowSafeDraft(this,
                    MmsConfig.getDeviceStorageFullStatus(), false, TOAST_TYPE_FOR_SAVE_DRAFT)) {
                if (!mCompressingImage) {
                    saveDraft(true);
                } else {
                    mNeedSaveDraftAfterStop = true;
                    log("onStop: skip save draft");
                }
                if (mConversation.getMessageCount() == 0) {
                    ConversationListAdapter.removeSelectedState(mSelectedThreadId);
                }
            }
        }

        // remove the useless draft in db
        if (!mWorkingMessage.isWorthSaving() && MessageUtils.isHome(this)) {
            Log.i(TAG, "back to launcher, delete unsaved empty draft");
            mWorkingMessage.discard();
            mHasDiscardWorkingMessage = true;
        }

        /// @}
        /// M: @{
        MmsWidgetProvider.notifyDatasetChanged(getApplicationContext());
        MmsLog.i(TAG,
                "onStop(): mWorkingMessage.isDiscarded() == " + mWorkingMessage.isDiscarded());
        /// @}
        /// M: fix bug ALPS00380930, fix RecipientsAdapter cursor leak @{
        //if (mRecipientsEditor != null && isFinishing()) {
        //    CursorAdapter recipientsAdapter = (CursorAdapter)mRecipientsEditor.getAdapter();
        //    if (recipientsAdapter != null) {
        //        recipientsAdapter.changeCursor(null);
        //    }
        //}
        /// @}

        // / M: fix bug ALPS00451836, remove FLAG_DISMISS_KEYGUARD flags
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD |
                                   WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);

        if (mMsgListAdapter != null) {
            MmsLog.d(TAG, "set OnDataSetChangedListener null");
            mMsgListAdapter.setOnDataSetChangedListener(null);
        }
        removeRecipientsListeners();
    }


    private void dismissmDiscardDraftDialog() {
        if (mDiscardDraftDialog != null) {
            if (mDiscardDraftDialog.isShowing()) {
                mDiscardDraftDialog.dismiss();
            }
            mDiscardDraftDialog = null;
        }
    }

    @Override
    protected void onDestroy() {
        if (mNeedRequestPermissins) {
            super.onDestroy();
            MmsLog.d(TAG, "onDestroy() no permissions");
            return;
        }
        dismissmDiscardDraftDialog();
        /// it will not show draft icon in the conversationlist @{
        if (!mWorkingMessage.isWorthSaving()) {
            Log.i(TAG, "delete unsavedempty draft: ");
            mWorkingMessage.discard();
            mHasDiscardWorkingMessage = true;
        }
        /// M: fix bug ALPS00473488, delete ObsoleteThread through threadID when discard()
        if (mConversation.getMessageCount() == 0
                && (!mWorkingMessage.isWorthSaving()
                        || mWorkingMessage.isDiscarded())
                && isRecipientsEditorVisible()) {
            Conversation.asyncDeleteObsoleteThreads(mBackgroundQueryHandler,
                    ConversationList.DELETE_OBSOLETE_THREADS_TOKEN);
            mConversation.setDiscardThreadId(0);
        }
        mIsStartMultiDeleteActivity = false;
        /// @}
        MmsLog.d(TAG, "onDestroy() activity=" + ComposeMessageActivity.this);
        if (TRACE) {
            android.os.Debug.stopMethodTracing();
        }

        mUiHandler.removeCallbacks(mContactPickRunnable);
        if (mContactPickDialog != null && mContactPickDialog.isShowing()) {
            mContactPickDialog.dismiss();
        }
        mContactPickDialog = null;

        // ALPS04235876, need dismiss dialog to prevent window leak
        mSoloAlertDialog.dismiss();
        mSoloAlertDialog = null;

        if (mQuickTextDialog != null && mQuickTextDialog.isShowing()) {
            mQuickTextDialog.dismiss();
        }
        mQuickTextDialog = null;

        if (mSubSelectDialog != null && mSubSelectDialog.isShowing()) {
            mSubSelectDialog.dismiss();
        }
        mSubSelectDialog = null;

        unregisterReceiver(mSubReceiver);
        unregisterReceiver(mShutDownReceiver);

        /// @}
        mScrollListener.destroyThread();
        /// M: Stop not started queries @{
        if (mBackgroundQueryHandler != null) {
            mBackgroundQueryHandler.cancelOperation(MESSAGE_LIST_QUERY_TOKEN);
        }
        /// @}
        if (mMsgListAdapter != null) {
            mMsgListAdapter.destroyTaskStack();
            /// M: we need unregister cursor, so no more callback
            mMsgListAdapter.changeCursor(null);
        }
        mIpCompose.onIpComposeActivityDestroy();
        /* Mem opt*/
        Log.d(TAG, "onDestroy");
        if (sCompose != null) {
            Log.d(TAG, "onDestroy1 this = " + this);
            Log.d(TAG, "onDestroy1 sCompose.get() = " + sCompose.get());
            if (this.equals(sCompose.get())) {
                sCompose = null;
            }
            MtkPduPersister.releaseOwnPduPersister(this);
        }
        if (mAttachmentEditor != null) {
            mAttachmentEditor.reset();
        }
        super.onDestroy();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (mNeedRequestPermissins) {
            return;
        }
        mIpCompose.onIpConfig(newConfig);
        mOpComposeExt.onConfigurationChanged();
        if (LOCAL_LOGV) {
            Log.v(TAG, "onConfigurationChanged: " + newConfig);
        }

        if (resetConfiguration(newConfig)) {
            // Have to re-layout the attachment editor because we have different layouts
            // depending on whether we're portrait or landscape.
            drawTopPanel(isSubjectEditorVisible());
        }
        onKeyboardStateChanged();
    }

    private boolean mUpdateForScrnOrientationChanged = false;
    // returns true if landscape/portrait configuration has changed
    private boolean resetConfiguration(Configuration config) {
        mIsKeyboardOpen = config.keyboardHidden == KEYBOARDHIDDEN_NO;
        boolean isLandscape = config.orientation == Configuration.ORIENTATION_LANDSCAPE;
        mIpCompose.resetIpConfiguration(mIsLandscape, isLandscape, mIsSoftKeyBoardShow);
        mOpComposeExt.resetConfiguration(mIsLandscape, isLandscape, mIsSoftKeyBoardShow);

        /// M: Code analyze 004, Set max height for text editor. @{
        if ((mTextEditor != null)
                && (mTextEditor.getVisibility() == View.VISIBLE) && isLandscape) {
            mUiHandler.postDelayed(new Runnable() {
                public void run() {
                    mTextEditor.setMaxHeight(mReferencedTextEditorTwoLinesHeight *
                            mCurrentMaxHeight / mReferencedMaxHeight);
                }
            }, 100);
        }
        /// @}

        MmsLog.d(TAG, "resetConfiguration(): isLandscape = " + isLandscape + ", mIsKeyboardOpen = "
                + mIsKeyboardOpen);

        if (mIsLandscape != isLandscape) {
            MmsLog.d(TAG, "resetConfiguration-mUpdateForScrnOrientationChanged mIsLandscape = "
                    + mIsLandscape);
            mUpdateForScrnOrientationChanged = true;
            mIsLandscape = isLandscape;
            return true;
        }
        return false;
    }

    private void onKeyboardStateChanged() {
        // If the keyboard is hidden, don't show focus highlights for
        // things that cannot receive input.
        mTextEditor.setEnabled(mIsSmsEnabled);
        if (!mIsSmsEnabled) {
            if (mRecipientsEditor != null) {
                mRecipientsEditor.setFocusableInTouchMode(false);
                mRecipientsEditor.setIsTouchable(false);
            }
            if (mSubjectTextEditor != null) {
                mSubjectTextEditor.setFocusableInTouchMode(false);
            }
            mTextEditor.setFocusableInTouchMode(false);
            mTextEditor.setHint(R.string.sending_disabled_not_default_app);
        } else if (mIsKeyboardOpen) {
            if (mRecipientsEditor != null) {
                mRecipientsEditor.setFocusableInTouchMode(true);
                mRecipientsEditor.setIsTouchable(true);
            }
            if (mSubjectTextEditor != null) {
                mSubjectTextEditor.setFocusableInTouchMode(true);
            }
            mTextEditor.setFocusableInTouchMode(true);
            /// M: add for ip message
            updateTextEditorHint();
        } else {
            if (mRecipientsEditor != null) {
                mRecipientsEditor.setFocusable(false);
                mRecipientsEditor.setIsTouchable(false);
            }
            if (mSubjectTextEditor != null) {
                mSubjectTextEditor.setFocusable(false);
            }
            mTextEditor.setFocusable(false);
            mTextEditor.setHint(R.string.open_keyboard_to_compose_message);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (mNeedRequestPermissins) {
            return false;
        }
        if (mIpCompose != null && mIpCompose.onIpKeyDown(keyCode, event)) {
            return true;
        }
        if (mOpComposeExt != null && mOpComposeExt.onKeyDown(keyCode, event)) {
            return true;
        }

        switch (keyCode) {
            case KeyEvent.KEYCODE_DEL:
                if ((mMsgListAdapter != null) && (mMsgListView != null) && mMsgListView.isFocused()) {
                    Cursor cursor;
                    try {
                        cursor = (Cursor) mMsgListView.getSelectedItem();
                    } catch (ClassCastException e) {
                        Log.e(TAG, "Unexpected ClassCastException.", e);
                        return super.onKeyDown(keyCode, event);
                    }

                    if (cursor != null) {
                        String type = cursor.getString(COLUMN_MSG_TYPE);
                        long msgId = cursor.getLong(COLUMN_ID);
                        MessageItem msgItem = mMsgListAdapter.getCachedMessageItem(type, msgId,
                                cursor);
                        if (msgItem != null) {
                            DeleteMessageListener l = new DeleteMessageListener(msgItem);
                            confirmDeleteDialog(l, msgItem.mLocked);
                        }
                        return true;
                    }
                }
                break;
            case KeyEvent.KEYCODE_DPAD_CENTER:
                /// M: @{
                break;
                /// @}
            case KeyEvent.KEYCODE_ENTER:
                /// M: Code analyze 028, Before sending message,check the recipients count
                /// and add sub card selection dialog if multi sim cards exist.@{
                /** Fix bug ALPS001070636
                if (isPreparedForSending()) {
                    checkRecipientsCount();
                    return true;
                } else {
                    unpreparedForSendingAlert();
                /// @}
                }
                **/
                break;
            case KeyEvent.KEYCODE_BACK:
                /// M: Code analyze 038, If the user is editing slideshow now.
                /// Do not allow the activity finish but return directly
                /// when back key is entered. @{
                if (mIsEditingSlideshow) {
                    return true;
                }
                /// @}

                // M: when out of composemessageactivity,try to send read report
                checkAndSendReadReport();
                /// @}
                mIsCheckObsolete = true;
                exitComposeMessageActivity(new Runnable() {
                    @Override
                    public void run() {
                        finish();
                    }
                });
                return true;
            case KeyEvent.KEYCODE_MENU:
                invalidateOptionsMenu();
                return false;
        }

        return super.onKeyDown(keyCode, event);
    }

    private void unpreparedForSendingAlert() {
        if (!isHasRecipientCount()) {
            new AlertDialog.Builder(this).setIconAttribute(
                    android.R.attr.alertDialogIcon).setTitle(
                    R.string.cannot_send_message).setMessage(
                    R.string.cannot_send_message_reason).setPositiveButton(
                    R.string.yes, new CancelSendingListener()).show();
        } else {
            new AlertDialog.Builder(this).setIconAttribute(
                    android.R.attr.alertDialogIcon).setTitle(
                    R.string.cannot_send_message).setMessage(
                    R.string.cannot_send_message_reason_no_content)
                    .setPositiveButton(R.string.yes,
                            new CancelSendingListener()).show();
        }
    }

    private void exitComposeMessageActivity(final Runnable exit) {
          VideoThumbnailCache.clear();
        // If the message is empty, just quit -- finishing the
        // activity will cause an empty draft to be deleted.
        if (!mWorkingMessage.isWorthSaving()) {
            /// M: Code analyze 042, If you discard the draft message manually.@{
            if (mMsgListAdapter.getCount() == 0) {
                ConversationListAdapter.removeSelectedState(mSelectedThreadId);
            }
            mWorkingMessage.discard();
            /// M: fix bug ALPS03584600.TextEditorWatcher onTextChanged may be called after
            // Compose activity has exit, these callback may cause onPause delay and ANR happen
            if (mTextEditor != null) {
                mTextEditor.removeTextChangedListener(mTextEditorWatcher);
            }
            mHasDiscardWorkingMessage = true;
            /// @}
            exit.run();
            return;
        }

        if (isRecipientsEditorVisible() &&
                !mRecipientsEditor.hasValidRecipient(mWorkingMessage.requiresMms())) {
            mDiscardDraftDialog = MessageUtils.showDiscardDraftConfirmDialog(
                                this, new DiscardDraftListener());
            return;
        }

        if (needSaveDraft()) {
            if (MessageUtils.allowSafeDraft(this, MmsConfig.getDeviceStorageFullStatus(), true,
                    TOAST_TYPE_FOR_SAVE_DRAFT)) {
                /// M: for requery searchactivity.
                SearchActivity.setNeedRequery();
                if (mIsSmsEnabled) {
                    SearchActivity.setWaitSaveDraft();
                }
            }
        }
        /// M: fix bug ALPS03584600.TextEditorWatcher onTextChanged may be called after
        // Compose activity has exit, these callback may cause onPause delay and ANR happen
        if (mTextEditor != null) {
            mTextEditor.removeTextChangedListener(mTextEditorWatcher);
        }
        mToastForDraftSave = true;
        exit.run();
    }

    private void goToConversationList() {
        finish();
        /// M: add extra flags
        Intent it = new Intent(this, ConversationList.class);
        it.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_SINGLE_TOP
                | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(it);
    }

    private void hideRecipientEditor() {
        if (mRecipientsEditor != null) {
            mRecipientsEditor.removeChipChangedListener(mChipWatcher);
            mRecipientsEditor.setVisibility(View.GONE);
            /// M: Code analyze 059, Set the pick button visible or
            /// invisible the same as recipient editor.
            mRecipientsPicker.setVisibility(View.GONE);
            /// @}
            hideOrShowTopPanel();
            mRecipientsEditor.recycleBitmap();
        }
    }

    private boolean isRecipientsEditorVisible() {
        return (null != mRecipientsEditor)
                    && (View.VISIBLE == mRecipientsEditor.getVisibility());
    }

    private boolean isSubjectEditorVisible() {
        return (null != mSubjectTextEditor)
                    && (View.VISIBLE == mSubjectTextEditor.getVisibility());
    }

    @Override
    public void onAttachmentChanged() {
        // Have to make sure we're on the UI thread. This function can be called off of the UI
        // thread when we're adding multi-attachments
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                drawBottomPanel();
                updateSendButtonState();
                drawTopPanel(isSubjectEditorVisible());
                if (null != mRecipientsEditor) {
                    if (mWorkingMessage.hasSlideshow()) {
                        mRecipientsEditor.setImeActionLabel(
                                getString(MessageResource.string.ime_action_done),
                                EditorInfo.IME_ACTION_DONE);
                        mRecipientsEditor.setImeOptions(EditorInfo.IME_ACTION_DONE);
                    } else {
                        mRecipientsEditor.setImeActionLabel(
                                getString(MessageResource.string.ime_action_next),
                                EditorInfo.IME_ACTION_NEXT);
                        mRecipientsEditor.setImeOptions(EditorInfo.IME_ACTION_NEXT);
                    }
                }

                mInputMethodManager.restartInput(mRecipientsEditor);
            }
        });
    }
    private boolean mIsConvertToastDelayed = false;
    /// M: Code analyze 060, For bug ALPS00050082, When the protocol has been changed,
    /// whether show a toast . @{
    private boolean mIsConvertMms = false;
    @Override
    public void onProtocolChanged(final boolean mms, final boolean needToast) {
        // Have to make sure we're on the UI thread. This function can be called off of the UI
        // thread when we're adding multi-attachments
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                //toastConvertInfo(mms);
                /// M: fix ALPS01033728, don't call showSmsOrMmsSendButton() here, method
                /// updateSendButtonState() will call it.
                //showSmsOrMmsSendButton(mms);
                mIpCompose.onIpProtocolChanged(mms, needToast);
                updateSendButtonState();

                if (needToast) {
                    if (mIsPopulatingRecipients) {
                        mIsConvertToastDelayed = true;
                        mIsConvertMms = mms;
                    } else {
                        toastConvertInfo(mms);
                    }
                }
            }
        });
    }
    /// @}

    // Show or hide the Sms or Mms button as appropriate. Return the view so that the caller
    // can adjust the enableness and focusability.
    private View showSmsOrMmsSendButton(boolean isMms) {
        View showButton = null;
        View hideButton = null;
        /* SIM ON OFF*/
        int subCount = getEnabledSubCount();
        // add for ipmessage

        if (isMms) {
            /* SIM ON OFF*/
            if (subCount == 0 || (isRecipientsEditorVisible()
                    && TextUtils.isEmpty(mRecipientsEditor.getText()))
                    /// M: fix bug ALPS00563318, show gray mms_send_button
                /// when haven't subject, text and attachment
                || ((mSubjectTextEditor == null || (mSubjectTextEditor != null
                        && TextUtils.isEmpty(mSubjectTextEditor.getText().toString().trim())))
                        && mTextEditor != null
                        && TextUtils.isEmpty(mTextEditor.getText().toString().trim())
                        && !mWorkingMessage.hasAttachment())
                        || !mIsSmsEnabled) {
                mSendButtonMms.setCompoundDrawablesWithIntrinsicBounds(null, null, null,
                        getResources().getDrawable(R.drawable.ic_send_sms_unsend));
            } else {
                mSendButtonMms.setCompoundDrawablesWithIntrinsicBounds(null, null, null,
                        getResources().getDrawable(R.drawable.ic_send_ipmsg));
            }
            showButton = mSendButtonMms;
            hideButton = mSendButtonSms;
        } else {
            if (!mIpCompose.onIpShowSmsOrMmsSendButton(isMms)) {
                /* SIM ON OFF*/
                if ((mTextEditor.getText().toString().isEmpty())
                        || subCount == 0
                        || (isRecipientsEditorVisible()
                                && TextUtils.isEmpty(mRecipientsEditor.getText()))
                        || recipientCount() > MmsConfig.getRecipientLimit()
                        || !mIsSmsEnabled) {
                    ///@}
                    mSendButtonSms.setImageResource(R.drawable.ic_send_sms_unsend);
                } else {
                    mSendButtonSms.setImageResource(R.drawable.ic_send_ipmsg);
                }
            }
            showButton = mSendButtonSms;
            hideButton = mSendButtonMms;
        }

        if (showButton != null) {
            showButton.setVisibility(View.VISIBLE);
        }
        if (hideButton != null) {
            hideButton.setVisibility(View.GONE);
        }
        updateTextEditorHint();
        return showButton;
    }

    Runnable mResetMessageRunnable = new Runnable() {
        @Override
        public void run() {
            resetMessage();
        }
    };

    @Override
    public void onPreMessageSent() {
        runOnUiThread(mResetMessageRunnable);
    }

    @Override
    public void onMessageSent() {
        // This callback can come in on any thread; put it on the main thread to avoid
        // concurrency problems
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                /// M: @{
                mWaitingForSendMessage = false;
                /// @}
                // If we already have messages in the list adapter, it
                // will be auto-requerying; don't thrash another query in.
                // TODO: relying on auto-requerying seems unreliable when priming an MMS into the
                // outbox. Need to investigate.
//                if (mMsgListAdapter.getCount() == 0) {
                    if (LogTag.VERBOSE) {
                        log("onMessageSent");
                    }
                    startMsgListQuery(MESSAGE_LIST_QUERY_TOKEN, 0);
//                }

                // The thread ID could have changed if this is a new message that we just inserted
                // into the database (and looked up or created a thread for it)
                updateThreadIdIfRunning();
            }
        });
    }

    @Override
    public void onMaxPendingMessagesReached() {
        saveDraft(false);

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(ComposeMessageActivity.this, R.string.too_many_unsent_mms,
                        Toast.LENGTH_LONG).show();
                mSendingMessage = false;
                updateSendButtonState();
            }
        });
    }

    @Override
    public void onAttachmentError(final int error) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                handleAddAttachmentError(error, R.string.type_picture);
                onMessageSent();        // now requery the list of messages
            }
        });
    }

    // We don't want to show the "call" option unless there is only one
    // recipient and it's a phone number.
    private boolean isRecipientCallable() {
        ContactList recipients = getRecipients();
        if (mIpCompose.isIpRecipientCallable(recipients.getNumbers())) {
            return true;
        }
        return (recipients.size() == 1 && !recipients.containsEmail());
    }
    /// M: Code analyze 061, Add video call menu.
    private void dialRecipient(Boolean isVideoCall) {
        if (isRecipientCallable()) {
            String number = getRecipients().get(0).getNumber();
            number = mIpCompose.dialIpRecipient(number);
            Intent dialIntent ;
            if (isVideoCall) {
                dialIntent = new Intent(Intent.ACTION_CALL, Uri.parse("tel:" + number));
                dialIntent.putExtra("com.android.phone.extra.video", true);
            } else {
                dialIntent = new Intent(Intent.ACTION_DIAL, Uri.parse("tel:" + number));
            }
            hideInputMethod();
            startActivity(dialIntent);
        }
    }
    /// @}

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu) ;

        if (mNeedRequestPermissins) {
            return false;
        }

        menu.clear();
        /// M: google JB.MR1 patch, group mms
        if (mIsSmsEnabled) {
            if (getRecipients().size() > 1) {
                menu.add(0, MENU_GROUP_PARTICIPANTS, 0, R.string.menu_group_participants);
            }
        }

        if (!isRecipientsEditorVisible()) {
            if (getRecipients().size() == 1) {
                menu.add(0, MENU_SHOW_CONTACT, 0, R.string.menu_view_contact)
                    .setIcon(R.drawable.ic_menu_recipients)
                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            }

            if (mIsSmsEnabled) {
                menu.add(0, MENU_SELECT_MESSAGE, 0, R.string.select_message);
            }

        }

        /// M: Code analyze 061, Add video call menu.
        TelephonyManager telephony =
            (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        if (telephony != null && telephony.isVoiceCapable() && isRecipientCallable()) {
            MenuItem item = menu.add(0, MENU_CALL_RECIPIENT, 0, R.string.menu_call)
                .setIcon(R.drawable.ic_menu_call)
                .setTitle(R.string.menu_call);
            if (!isRecipientsEditorVisible()) {
                // If we're not composing a new message, show the call icon in the actionbar
                item.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            }
        /// @}
        }

        /// M: Code analyze 014, Add quick text. @{
        if (mIsSmsEnabled) {
            if (!mWorkingMessage.hasSlideshow()
                    || (mSubjectTextEditor != null && mSubjectTextEditor.isFocused())) {
                menu.add(0, MENU_ADD_QUICK_TEXT, 0, R.string.menu_insert_quick_text).setIcon(
                    R.drawable.ic_menu_quick_text);
            }
        }
        /// @}

        /// M: Code analyze 015, Add text vcard. @{
        if (mIsSmsEnabled) {
            if (!mWorkingMessage.hasSlideshow()) {
                menu.add(0, MENU_ADD_TEXT_VCARD, 0, R.string.menu_insert_text_vcard);
            }
        }
        /// @}
        if (mIsSmsEnabled) {
            if (!isSubjectEditorVisible()) {
                menu.add(0, MENU_ADD_SUBJECT, 0,
                        R.string.add_subject).setIcon(R.drawable.ic_menu_edit);
            }

            menu.add(0, MENU_ADD_ATTACHMENT, 0, R.string.add_attachment)
                    .setIcon(R.drawable.ic_menu_attachment)
                .setTitle(R.string.add_attachment)
                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);    // add to actionbar
        }
        buildAddAddressToContactMenuItem(menu);

        if (LogTag.DEBUG_DUMP) {
            menu.add(0, MENU_DEBUG_DUMP, 0, R.string.menu_debug_dump);
        }

        int contactSize = 0;
        if (mConversation.getRecipients() != null) {
            contactSize = mConversation.getRecipients().size();
        }

        if (isRecipientsEditorVisible() && mIsSmsEnabled) {
            menu.add(0, MENU_DISCARD, 0, R.string.discard).setIcon(
                android.R.drawable.ic_menu_delete);
        }

        mOpComposeExt.onPrepareOptionsMenu(menu, mIsSmsEnabled,
                isRecipientsEditorVisible(), contactSize, mConversation.getThreadId());

        // add for ipmessage
        mIpCompose.onIpPrepareOptionsMenu(mConversation.getIpConv(this), menu);

        return true;
    }

    private void buildAddAddressToContactMenuItem(Menu menu) {
        // Look for the first recipient we don't have a contact for and create a menu item to
        // add the number to contacts.
        for (Contact c : getRecipients()) {
            /// M: Code analyze 043, Whether the address can be added to contacts app. @{
            if (!c.existsInDatabase() && MessageUtils.canAddToContacts(c)) {
            /// @}
                Intent intent = ConversationList.createAddContactIntent(c.getNumber());
                menu.add(0, MENU_ADD_ADDRESS_TO_CONTACTS, 0, R.string.menu_add_to_contacts)
                    .setIcon(android.R.drawable.ic_menu_add)
                    .setIntent(intent);
                break;
            }
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (mNeedRequestPermissins) {
            return false;
        }
        // add for ipmessage
        if (mIpCompose.onIpMsgOptionsItemSelected(
                mConversation.getIpConv(this), item, mConversation.getThreadId())) {
            return true;
        }
        if (mOpComposeExt.onOptionsItemSelected(item, mWorkingMessage.mOpWorkingMessageExt,
                mInputMethodManager)) {
            return true;
        }

        switch (item.getItemId()) {
            case MENU_ADD_SUBJECT:
                showSubjectEditor(true);
                mWorkingMessage.setSubject("", true);
                /// M: Code analyze 052, Show input keyboard.@{
                mInputMethodManager.showSoftInput(
                        getCurrentFocus(), InputMethodManager.SHOW_IMPLICIT);
                /// @}
                updateSendButtonState();
                mSubjectTextEditor.requestFocus();
                break;
            case MENU_ADD_ATTACHMENT:
                // Launch the add-attachment list dialog
                /// M: Code analyze 051, Hide input keyboard.@{
                hideInputMethod();
                showAddAttachmentDialog(!mWorkingMessage.hasAttachedFiles());
                MmsLog.d(TAG, "Attach: showAddAttachmentDialog(!hasAttachedFiles)");
                break;
            /// M: Code analyze 014, Add quick text. @{
            case MENU_ADD_QUICK_TEXT:
                showQuickTextDialog();
                break;
            /// @}
            /// M: Code analyze 015, Add text vcard. @{
            case MENU_ADD_TEXT_VCARD: {
                Intent intent =
                    new Intent("mediatek.intent.action.contacts.list.PICKMULTICONTACTS");
                intent.setType(Contacts.CONTENT_TYPE);
                startActivityForResult(intent, REQUEST_CODE_TEXT_VCARD);
                break;
            }
            /// @}
            case MENU_DISCARD:
                /// M: fix bug for ConversationList select all performance,
                /// update selected threads array.@{
                ConversationListAdapter.removeSelectedState(mSelectedThreadId);
                /// @}
                /// M: Fix bug of inputmethod not disappear
                hideInputMethod();
                mWorkingMessage.discard();
                finish();
                break;
            case MENU_SEND:
                if (isPreparedForSending()) {
                    /// M: Code analyze 028, Before sending message,check the recipients count
                    /// and add sub card selection dialog if multi sub cards exist.@{
                    updateSendButtonState(false);
                    checkRecipientsCount();
                    mSendButtonCanResponse = true;
                    /// @}
                }
                break;
            case MENU_SEARCH:
                onSearchRequested();
                break;
            case MENU_DELETE_THREAD:
                /// M: Code analyze 012, add for multi-delete @{
                Intent it = new Intent(this, MultiDeleteActivity.class);
                it.putExtra("thread_id", mConversation.getThreadId());
                startActivityForResult(it, REQUEST_CODE_FOR_MULTIDELETE);
                /// @}
                break;

            case android.R.id.home:
                mIsCheckObsolete = true;
                mConversation.setDiscardThreadId(mConversation.getThreadId());
            case MENU_CONVERSATION_LIST:
                exitComposeMessageActivity(new Runnable() {
                    @Override
                    public void run() {
                        goToConversationList();
                    }
                });
                break;
            case MENU_CALL_RECIPIENT:
                dialRecipient(false);
                break;
            /// M: Code analyze 061, Add video call menu.
            case MENU_CALL_RECIPIENT_BY_VT:
                dialRecipient(true);
                break;
            /// @}
            /// M: google jb.mr1 patch, group mms
            case MENU_GROUP_PARTICIPANTS: {
                Intent intent = new Intent(this, RecipientListActivity.class);
                intent.putExtra(THREAD_ID, mConversation.getThreadId());
                startActivityForResult(intent, REQUEST_CODE_GROUP_PARTICIPANTS);
                break;
            }
            case MENU_VIEW_CONTACT: {
                // View the contact for the first (and only) recipient.
                ContactList list = getRecipients();
                if (list.size() == 1 && list.get(0).existsInDatabase()) {
                    Uri contactUri = list.get(0).getUri();
                    Intent intent = new Intent(Intent.ACTION_VIEW, contactUri);
                    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                    startActivity(intent);
                }
                break;
            }
            case MENU_ADD_ADDRESS_TO_CONTACTS:
                mAddContactIntent = item.getIntent();
                startActivityForResult(mAddContactIntent, REQUEST_CODE_ADD_CONTACT);
                break;
            case MENU_PREFERENCES: {
                Intent settingIntent = null;
                settingIntent = new Intent(this, SettingListActivity.class);
                startActivityIfNeeded(settingIntent, -1);
                break;
            }
            case MENU_DEBUG_DUMP:
                mWorkingMessage.dump();
                Conversation.dump();
                LogTag.dumpInternalTables(this);
                break;
            case MENU_ADD_TO_CONTACTS: {
                mAddContactIntent =
                    ConversationList.createAddContactIntent(getRecipients().get(0).getNumber());
                startActivityForResult(mAddContactIntent, REQUEST_CODE_ADD_CONTACT);
                break;
            }
            /// M: show contact detail or create new contact. @{
            case MENU_SHOW_CONTACT:
                Contact contact = getRecipients().get(0);
                contact.reload(true);
                if (contact.existsInDatabase()) {
                    mQuickContact.assignContactUri(contact.getUri());
                } else if ( MessageUtils.canAddToContacts(contact)) {
                    // change to menu MENU_CREATE_CONTACT
                    item.setTitle(R.string.menu_add_to_contacts);
                    String number = contact.getNumber();
                    if (Mms.isEmailAddress(number)) {
                        mQuickContact.assignContactFromEmail(number, true);
                    } else {
                        mQuickContact.assignContactFromPhone(number, true);
                    }
                }

                hideInputMethod();
                mQuickContact.onClick(mActionBarCustomView);
                break;
            /// @}
            case MENU_SELECT_MESSAGE:
                Intent intentSelectMessage = new Intent(this, MultiDeleteActivity.class);
                intentSelectMessage.putExtra("thread_id", mConversation.getThreadId());
                startActivityForResult(intentSelectMessage, REQUEST_CODE_FOR_MULTIDELETE);
                mIsStartMultiDeleteActivity = true;
                break;
            default:
                MmsLog.d(TAG, "unkown option.");
                break;
        }

        return true;
    }

    private void addAttachment(int type, boolean append) {
        if (!MessageUtils.allowSafeDraft(this, MmsConfig.getDeviceStorageFullStatus(), true,
                TOAST_TYPE_FOR_ATTACH)) {
            return;
        }
        mWaitingAttachment = true;
        // Calculate the size of the current slide if we're doing a replace so the
        // slide size can optionally be used in computing how much room is left for an attachment.
        int currentSlideSize = 0;
        SlideshowModel slideShow = mWorkingMessage.getSlideshow();

        /// M: Code analyze 025, Add video or audio attachment and check the attachment size.@{
        if (append) {
            mAppendAttachmentSign = true;
        } else {
            mAppendAttachmentSign = false;
        }
        /// @}
        MmsLog.d(TAG, "addAttachment type=" + type + ", append=" + append);
        if (slideShow != null) {
            if (!(append && type == AttachmentTypeSelectorAdapter.ADD_SOUND)) {
                WorkingMessage.removeThumbnailsFromCache(slideShow);
            }
            SlideModel slide = slideShow.get(0);
            currentSlideSize = slide == null ? 0 : slide.getSlideSize();
        }
        /// M: Code analyze 025, Add video or audio attachment and check the attachment size.@{
        if ((type != AttachmentTypeSelectorAdapter.ADD_SLIDESHOW)
                && (type != AttachmentTypeSelectorAdapter.ADD_VCARD)
            && (!checkSlideCount(mAppendAttachmentSign))) {
            return;
        }
        /// @}
        switch (type) {
            case AttachmentTypeSelectorAdapter.ADD_IMAGE:
                MessageUtils.selectImage(this, REQUEST_CODE_ATTACH_IMAGE);
                break;

            case AttachmentTypeSelectorAdapter.TAKE_PICTURE: {
                MessageUtils.capturePicture(this, REQUEST_CODE_TAKE_PICTURE);
                break;
            }

            case AttachmentTypeSelectorAdapter.ADD_VIDEO:
                MessageUtils.selectVideo(this, REQUEST_CODE_ATTACH_VIDEO);
                break;

            case AttachmentTypeSelectorAdapter.RECORD_VIDEO: {
                /// M: Code analyze 025, Add video or audio attachment
                /// and check the attachment size.@{
                long sizeLimit = 0;
                if (mAppendAttachmentSign) {
                    sizeLimit = computeAttachmentSizeLimit(slideShow, 0);
                } else {
                    sizeLimit = computeAttachmentSizeLimit(slideShow, currentSlideSize);
                }
                /// M: fix bug ALPS01221817 & ALPS01231411, Should subtract mText Size
                if ((slideShow == null || !mAppendAttachmentSign)
                        && !TextUtils.isEmpty(mWorkingMessage.getText())) {
                    int textSize = mWorkingMessage.getText().toString().getBytes().length;
                    sizeLimit -= textSize;
                }
                if (sizeLimit > MIN_SIZE_FOR_CAPTURE_VIDEO) {
                    MessageUtils.recordVideo(this, REQUEST_CODE_TAKE_VIDEO, sizeLimit);
                } else {
                    Toast.makeText(this,
                            getString(R.string.space_not_enough),
                            Toast.LENGTH_SHORT).show();
                }
                /// @}
            }
            break;

            case AttachmentTypeSelectorAdapter.ADD_SOUND:
                /// M: Code analyze 018, Add ringtone for sound attachment.  @{
                //MessageUtils.selectAudio(this, REQUEST_CODE_ATTACH_SOUND);
                AlertDialog.Builder alertBuilder = new AlertDialog.Builder(this);
                alertBuilder.setTitle(getString(R.string.add_music));
                String[] items = new String[2];
                items[0] = getString(R.string.attach_ringtone);
                items[1] = getString(R.string.attach_sound);
                alertBuilder.setItems(items, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        switch (which) {
                            case 0:
                                MessageUtils.selectRingtone(ComposeMessageActivity.this,
                                        REQUEST_CODE_ATTACH_RINGTONE);
                                break;
                            case 1:
                                if (!Environment.getExternalStorageState()
                                        .equals(Environment.MEDIA_MOUNTED)) {
                                    Toast.makeText(ComposeMessageActivity.this,
                                                    getString(R.string.Insert_sdcard),
                                                    Toast.LENGTH_LONG).show();
                                    return;
                                }
                                MessageUtils.selectAudio(ComposeMessageActivity.this,
                                        REQUEST_CODE_ATTACH_SOUND);
                                break;
                            default:
                                break;
                        }
                    }
                });
                alertBuilder.create().show();
               /// @}
                break;

            case AttachmentTypeSelectorAdapter.RECORD_SOUND:
                /// M: Code analyze 025, Add video or audio attachment
                /// and check the attachment size.@{
                long sizeLimit = 0;
                if (mAppendAttachmentSign) {
                    sizeLimit = computeAttachmentSizeLimit(slideShow, 0);
                } else {
                    sizeLimit = computeAttachmentSizeLimit(slideShow, currentSlideSize);
                }
                /// M: fix bug ALPS01221817 & ALPS01231411, Should subtract mText Size
                if ((slideShow == null || !mAppendAttachmentSign)
                        && !TextUtils.isEmpty(mWorkingMessage.getText())) {
                    int textSize = mWorkingMessage.getText().toString().getBytes().length;
                    sizeLimit -= textSize;
                }
                if (sizeLimit > ComposeMessageActivity.MIN_SIZE_FOR_RECORD_AUDIO) {
                    MessageUtils.recordSound(this, REQUEST_CODE_RECORD_SOUND, sizeLimit);
                } else {
                    Toast.makeText(this, getString(R.string.space_not_enough_for_audio),
                            Toast.LENGTH_SHORT).show();
                }
                /// @}
                break;

            case AttachmentTypeSelectorAdapter.ADD_SLIDESHOW:
                editSlideshow();
                break;
            /// M: Code analyze 019, Add vcard attachment.@{
            case AttachmentTypeSelectorAdapter.ADD_VCARD:
                Intent intent =
                    new Intent("mediatek.intent.action.contacts.list.PICKMULTICONTACTS");
                intent.setType(Contacts.CONTENT_TYPE);
                startActivityForResult(intent, REQUEST_CODE_ATTACH_VCARD);
                break;
            /// @}
            /// M: Code analyze 020, Add vcalendar attachment.  @{
            case AttachmentTypeSelectorAdapter.ADD_VCALENDAR:
                Intent i = new Intent("mediatek.intent.action.CALENDARCHOICE");
                i.setType("text/x-vcalendar");
                i.putExtra("request_type", 0);
                startActivityForResult(i, REQUEST_CODE_ATTACH_VCALENDAR);
                break;
            /// @}
            default:
                break;
        }
    }

    public static long computeAttachmentSizeLimit(SlideshowModel slideShow, int currentSlideSize) {
        // Computer attachment size limit. Subtract 1K for some text.
        /// M: Code analyze 003,  Set or get max mms size.
        long sizeLimit = MmsConfig.getUserSetMmsSizeLimit(true) - SlideshowModel.SLIDESHOW_SLOP;
        /// @}
        if (slideShow != null) {
            sizeLimit -= slideShow.getCurrentSlideshowSize();

            // We're about to ask the camera to capture some video (or the sound recorder
            // to record some audio) which will eventually replace the content on the current
            // slide. Since the current slide already has some content (which was subtracted
            // out just above) and that content is going to get replaced, can add the size of the
            // current slide into the available space used to capture a video (or audio).
            sizeLimit += currentSlideSize;
        } else {
            sizeLimit -= MmsSizeUtils.getSlideshowInitSize();
        }
        return sizeLimit;
    }

    private void showAddAttachmentDialog(final boolean append) {
        /// M: Code analyze 009,Show attachment dialog . Create a new class to @{
        mSoloAlertDialog.show(append);
        /// @}
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
            log("requestCode=" + requestCode + ", resultCode=" + resultCode + ", data=" + data);
        if (!isAllPermissionGranted() || mWorkingMessage == null) {
            return;
        }
        mWaitingForSubActivity = false;          // We're back!

        /// M : FIX CR : ALPS01795853 @{
        mWorkingMessage.setTruntoChooseAttach(false);
        ///@}

        // M: fix bug ALPS00354728
        boolean mNeedAppendAttachment = true;

        MmsLog.d(TAG, "onActivityResult mAppendAttachmentSign : " + mAppendAttachmentSign);
        if (mAppendAttachmentSign) {
            mNeedAppendAttachment = true;
        } else if (!mAppendAttachmentSign) {
            mNeedAppendAttachment = false;
        }

        /// @}
        if (mWorkingMessage.isFakeMmsForDraft()) {
            // We no longer have to fake the fact we're an Mms. At this point we are or we aren't,
            // based on attachments and other Mms attrs.
            mWorkingMessage.removeFakeMmsForDraft();
        }

        // add for ipmessage
        mIpCompose.onIpMsgActivityResult(this, requestCode, resultCode, data);

         /// M: Code analyze 012, add for multi-delete @{
         if (requestCode == REQUEST_CODE_FOR_MULTIDELETE && resultCode == RESULT_OK) {
             ContactList recipients = mConversation.getRecipients();
             mConversation = Conversation.upDateThread(getApplicationContext(),
                     mConversation.getThreadId(), false);
             mIsStartMultiDeleteActivity = false;
             if (mConversation.getMessageCount() <= 0 || mConversation.getThreadId() <= 0) {
                 mMsgListAdapter.changeCursor(null);
                 if (needSaveDraft() && (recipients != null)) {
                     makeDraftEditable(recipients);
                 } else {
                     mWorkingMessage.discard();

                     /// M: Rebuild the contacts cache now that a thread and its associated unique
                     /// M: recipients have been deleted.
                     Contact.initCache(getApplicationContext());

                     /// M: Make sure the conversation cache reflects the threads in the DB.
                     Conversation.init(getApplicationContext());
                     /// M: fix bug for ConversationList select all performance,
                     /// update selected threads array.@{
                     ConversationListAdapter.removeSelectedState(mSelectedThreadId);
                     /// @}
                     finish();
                 }
             }
            return;
        }
        /// @}
        if (requestCode == REQUEST_CODE_PICK && mIsSmsEnabled) {
            mWorkingMessage.asyncDeleteDraftSmsMessage(mConversation);
        }

        /// M: fix bug ALPS00490684, update group mms state from GROUP_PARTICIPANTS to setting @{
        if (requestCode == REQUEST_CODE_GROUP_PARTICIPANTS) {
            boolean multiRecipients = mConversation.getRecipients().size() > 1;
            boolean isGroupMms = MmsPreferenceActivity.getIsGroupMmsEnabled(
                    ComposeMessageActivity.this) && multiRecipients;
            mMsgListAdapter.setIsGroupConversation(isGroupMms);
            mWorkingMessage.setHasMultipleRecipients(multiRecipients, true);
            mWorkingMessage.deleteGruoupMmsDraft();
        }
        /// @}

        if (requestCode == REQUEST_CODE_ADD_CONTACT) {
            // The user might have added a new contact. When we tell contacts to add a contact
            // and tap "Done", we're not returned to Messaging. If we back out to return to
            // messaging after adding a contact, the resultCode is RESULT_CANCELED. Therefore,
            // assume a contact was added and get the contact and force our cached contact to
            // get reloaded with the new info (such as contact name). After the
            // contact is reloaded, the function onUpdate() in this file will get called
            // and it will update the title bar, etc.
            if (!isRecipientsEditorVisible()) {
                if (mAddContactIntent != null) {
                    String address =
                        mAddContactIntent.getStringExtra(ContactsContract.Intents.Insert.EMAIL);
                    if (address == null) {
                        address = mAddContactIntent.getStringExtra(
                                ContactsContract.Intents.Insert.PHONE);
                    }
                    if (address != null) {
                        Contact contact = Contact.get(address, false);
                        if (contact != null) {
                            contact.reload();
                        }
                    }
                }
            }
        }

        if (resultCode != RESULT_OK && requestCode == REQUEST_CODE_CREATE_SLIDESHOW
                && mWorkingMessage != null) {
            mWorkingMessage.setForceUpdateThreadId(true);
        }

        if (resultCode != RESULT_OK) {
            if (LogTag.VERBOSE) log("bail due to resultCode=" + resultCode);
            mWaitingAttachment = false;
            return;
        }

        /// M: disable when non-default sms
        if (!mIsSmsEnabled && requestCode != REQUEST_CODE_ECM_EXIT_DIALOG
                && requestCode != REQUEST_CODE_MULTI_SAVE
                && requestCode != REQUEST_CODE_CREATE_SLIDESHOW) {
            Toast.makeText(ComposeMessageActivity.this,
                    R.string.compose_disabled_toast, Toast.LENGTH_SHORT).show();
            return;
        }

        WorkingMessage newMessage = null;
        IOpWorkingMessageExt newOpWorkingMessageExt = null;
        if (requestCode == REQUEST_CODE_CREATE_SLIDESHOW && data != null) {
            newMessage = WorkingMessage.load(this, data.getData());
            if (newMessage != null) {
                newOpWorkingMessageExt = newMessage.mOpWorkingMessageExt;
            }
        }

        switch (requestCode) {
            case REQUEST_CODE_CREATE_SLIDESHOW:
                if (data != null) {
                    mWaitingAttachment = false;
                    int slideSize = 0;
                    if (newMessage != null && newMessage.getSlideshow() != null) {
                        slideSize = newMessage.getSlideshow().size();
                        MmsLog.i(TAG, "REQUEST_CODE_CREATE_SLIDESHOW" +
                                    " newWorkingMessage Slideshow num = " + slideSize);
                    } else {
                        MmsLog.i(TAG, "REQUEST_CODE_CREATE_SLIDESHOW" +
                                " newWorkingMessage Slideshow = null or newMessage = null");
                    }

                    if (newMessage != null) {
                        /// M: Code analyze 053, If exist vcard attachment, move it before
                        /// creating a new slideshow.Because the Workingmessage object  has been
                        /// changed, reset subject and tell the convertion to user.@{
                        /// M: add for vcard, vcard is exclusive with other attach, so remove them
                        if (newMessage.hasMediaAttachments()) {
                            newMessage.removeAllFileAttaches();
                        }
                        boolean isMmsBefore = mWorkingMessage.requiresMms();
                        newMessage.setSubject(mWorkingMessage.getSubject(), false);

                        /// M: fix bug ALPS01265824, need remove FileAttachment
                        /// when text + attachmentSize > limit
                        if (newMessage.isRemoveFileAttachment()) {
                            int[] params = null;
                            int encodingType = SmsMessage.ENCODING_UNKNOWN;
                            encodingType = mOpComposeExt.getSmsEncodingType(encodingType,
                                    ComposeMessageActivity.this);
                            params = MtkSmsMessage.calculateLength(newMessage.getText(),
                                    false, encodingType);
                            final int msgCount = params[0];
                            newMessage.setLengthRequiresMms(
                                    msgCount >= MmsConfig.getSmsToMmsTextThreshold(), false);

                            newMessage.removeAllFileAttaches();
                            newMessage.correctAttachmentState();
                        }

                        mWorkingMessage = newMessage;
                        updateThreadIdIfRunning();
                        drawTopPanel(isSubjectEditorVisible());
                        updateSendButtonState();
                        invalidateOptionsMenu();

                        boolean isMmsAfter = mWorkingMessage.requiresMms();
                        if (!isMmsAfter) {
                            mWorkingMessage.asyncDeleteDraftMmsMessage(
                                    newMessage.getConversation().getThreadId());
                        }
                        if (isMmsAfter && !isMmsBefore) {
                            toastConvertInfo(true);
                        } else if (!isMmsAfter && isMmsBefore) {
                            toastConvertInfo(false);
                        } else if (!isMmsAfter && !isMmsBefore) {
                            mWorkingMessage.setForceUpdateThreadId(true);
                        }
                        /// @}

                        if (null != mRecipientsEditor) {
                            if (mWorkingMessage.hasSlideshow()) {
                                mRecipientsEditor.setImeActionLabel(
                                        getString(MessageResource.string.ime_action_done),
                                        EditorInfo.IME_ACTION_DONE);
                                mRecipientsEditor.setImeOptions(EditorInfo.IME_ACTION_DONE);
                            } else {
                                mRecipientsEditor.setImeActionLabel(
                                        getString(MessageResource.string.ime_action_next),
                                        EditorInfo.IME_ACTION_NEXT);
                                mRecipientsEditor.setImeOptions(EditorInfo.IME_ACTION_NEXT);
                            }
                        }
                    }
                }
                // M: fix bug ALPS00354728
                MmsLog.i(TAG, "mNeedAppendAttachment = " + mNeedAppendAttachment);

                break;

            case REQUEST_CODE_TAKE_PICTURE: {
                // create a file based uri and pass to addImage(). We want to read the JPEG
                // data directly from file (using UriImage) instead of decoding it into a Bitmap,
                // which takes up too much memory and could easily lead to OOM.

                /// M: fix bug ALPS00408589
                String scrappath = TempFileProvider.getScrapPath(this);
                if (scrappath != null) {
                    File file = new File(scrappath);
                    Uri uri = Uri.fromFile(file);

                    // Remove the old captured picture's thumbnail from the cache
                    MmsApp.getApplication().getThumbnailManager().removeThumbnail(uri);

                    addImageAsync(uri, mNeedAppendAttachment);
                }
                break;
            }

            case REQUEST_CODE_ATTACH_IMAGE: {
                if (data != null) {
                    addImageAsync(data.getData(), mNeedAppendAttachment);
                }
                break;
            }
            case REQUEST_CODE_TAKE_VIDEO:
                Uri videoUri = TempFileProvider.renameScrapVideoFile(System.currentTimeMillis()
                        + ".3gp", null, this);
                // Remove the old captured video's thumbnail from the cache
                MmsApp.getApplication().getThumbnailManager().removeThumbnail(videoUri);

                addVideoAsync(videoUri, mNeedAppendAttachment);      // can handle null videoUri
                break;

            case REQUEST_CODE_ATTACH_VIDEO:
                if (data != null) {
                    addVideoAsync(data.getData(), mNeedAppendAttachment);
                }
                break;

            case REQUEST_CODE_ATTACH_SOUND: {
                Uri uri = (Uri) data.getParcelableExtra(RingtoneManager.EXTRA_RINGTONE_PICKED_URI);
                if (Settings.System.getUriFor(Settings.System.RINGTONE).equals(uri)) {
                    break;
                }
                addAudioAsync(data.getData(), mNeedAppendAttachment);
                break;
            }

            case REQUEST_CODE_RECORD_SOUND:
                if (data != null) {
                    addAudioAsync(data.getData(), mNeedAppendAttachment);
                }
                break;

            /// M: @{
            case REQUEST_CODE_ATTACH_RINGTONE:
                if (data != null) {
                    Uri uri = (Uri) data.getParcelableExtra(
                            RingtoneManager.EXTRA_RINGTONE_PICKED_URI);
                    if (Settings.System.getUriFor(Settings.System.RINGTONE).equals(uri)) {
                        break;
                    }
                    addAudioAsync(uri, mNeedAppendAttachment);
                }
                break;
            /// @}

            case REQUEST_CODE_ECM_EXIT_DIALOG:
                if (data != null) {
                    boolean outOfEmergencyMode = data.getBooleanExtra(EXIT_ECM_RESULT, false);
                    if (outOfEmergencyMode) {
                        sendMessage(false);
                    }
                }
                break;

            case REQUEST_CODE_PICK:
                /// M: Code analyze 013, Get contacts from Contact app . @{
                if (data != null) {
                    if (mRecipientsEditor != null) {
                        processPickResult(data);
                    } else {
                        mIsRecipientHasIntentNotHandle = true;
                        mIntent = data;
                    }
                }
                misPickContatct = false;
                return;
                /// @}
            /// M: Code analyze 015, Add text vcard. @{
            case REQUEST_CODE_TEXT_VCARD:
                if (data != null) {
                    long[] contactIds =
                        data.getLongArrayExtra("com.mediatek.contacts.list.pickcontactsresult");
                    addTextVCardAsync(contactIds);
                } else {
                    MmsLog.wpi(TAG, "data should not be null," + "requestCode=" + requestCode
                            + ", resultCode=" + resultCode + ", data=" + data);
                }
                misPickContatct = false;
                return;
            /// @}
            /// M: Code analyze 019, Add vcard attachment.@{
            case REQUEST_CODE_ATTACH_VCARD:
                asyncAttachVCardByContactsId(data);
                misPickContatct = false;
                isInitRecipientsEditor = false;
                return;
            /// @}
            /// M: Code analyze 020, Add vcalendar attachment.  @{
            case REQUEST_CODE_ATTACH_VCALENDAR:
                asyncAttachVCalendar(data.getData());
                misPickContatct = false;
                isInitRecipientsEditor = false;
                return;
            /// @}
            /// M: Code analyze 021, Copy all valid parts of the attachment(pdu) to SD card.
            /// This opeartor will be removed to a separate activity.  @{
            case REQUEST_CODE_MULTI_SAVE:
                boolean succeeded = false;
                if (data != null && data.hasExtra("multi_save_result")) {
                    succeeded = data.getBooleanExtra("multi_save_result", false);
                    int resId =
                        succeeded ? R.string.copy_to_sdcard_success : R.string.copy_to_sdcard_fail;
                    Toast.makeText(ComposeMessageActivity.this, resId, Toast.LENGTH_SHORT).show();
                }
                return;
            /// @}
            default:
                if (LogTag.VERBOSE) log("bail due to unknown requestCode=" + requestCode);
                break;
        }
        /// M: @{
        isInitRecipientsEditor = false; /// why add this variable here???
    }

    /**
     * Handle duplicate selected contacts and put new selected contact to list.
     * @param mSelectContactsNumbers the result of selected contacts number.
     * @param editor the current recipientEditor.
     * @param contactsId the result of selected contacts id.
     * @param list the new select contacts.
     * @param allList all contacts.
     * @return whether has duplicate recipients.
     */
    private boolean processDuplicatePickResult(String mSelectContactsNumbers,
            RecipientsEditor editor, long[] contactsId,
            ContactList list, ContactList allList) {
        boolean isDuplicate = false;
        try {
            /// M: @{
            //list = ContactList.blockingGetByUris(uris);
            /// M: add for ip message
            /// M: To append recipients into RecipientsEditor, no need to load avatar,
            /// because Editor will query and notify avatar info to MMS later. If append
            /// 100 recipients, will saving almost 3s.
            Contact.sNeedLoadAvatar = false;
            ContactList selected = TextUtils.isEmpty(mSelectContactsNumbers) ?
                 ContactList.blockingGetByIds(contactsId) :
                     ContactList.getByNumbers(mSelectContactsNumbers, false, false);
            Contact.sNeedLoadAvatar = true;
            final List<String> numbers = editor.getNumbers();

            /** M: better merge strategy.
             * Avoid the use of mRecipientsEditor.contrcutionContactsFromInput()
             * all Contacts in selected list should be added.
             * */
            /// M: remove duplicated numbers and format
            List<String> selectedNumbers = Arrays.asList(selected.getProtosomaitcNumbers());
            if (selectedNumbers.size() < selected.size()) {
                isDuplicate = true;
            }
            String selectedNumberAfterFormat = "";
            if (numbers.size() > 0) {
                for (String number : numbers) {
                    if (!number.trim().equals("")) {
                        Contact c = Contact.get(number, false);
                        allList.add(c);
                    }
                }
                /// M: format existing numbers(remove "-" and " ")
                List<String> formatedNumbers = Arrays.asList(allList.getNumbers(true));
                for (String selectedNumber : selectedNumbers) {
                    selectedNumberAfterFormat = MessageUtils.parseMmsAddress(selectedNumber);
                    if (selectedNumberAfterFormat != null
                            && !selectedNumberAfterFormat.trim().equals("")) {
                        if (!formatedNumbers.contains(selectedNumberAfterFormat)) {
                            Contact c = Contact.get(selectedNumber, false);
                            list.add(c);
                        } else {
                            //M: ALPS01831885, fix contact cache error.
                            Contact c = Contact.get(selectedNumber, false);
                            c.removeFromCache();
                            isDuplicate = true;
                        }
                    }
                }
                allList.addAll(list);
            } else {
                for (String selectedNumber : selectedNumbers) {
                    selectedNumberAfterFormat = MessageUtils.parseMmsAddress(selectedNumber);
                    if (selectedNumberAfterFormat != null && !selectedNumber.trim().equals("")) {
                        Contact c = Contact.get(selectedNumber, false);
                        list.add(c);
                    }
                }
                allList.addAll(list);
            }
            /// @}
        } finally {
            Message msg = mUiHandler.obtainMessage();
            msg.what = MSG_DISMISS_CONTACT_PICK_DIALOG;
            mUiHandler.sendMessage(msg);
            return isDuplicate;
        }
    }

    private ProgressDialog mContactPickDialog;

    private Runnable mContactPickRunnable = new Runnable() {
        public void run() {
            if (mContactPickDialog != null) {
                mContactPickDialog.show();
            }
        }
    };

    public String getNumbersFromIntent(Intent intent) {
        if (intent == null) {
            return null;
        }
        boolean usingColon = intent.getBooleanExtra(USING_COLON, false);
        String selectContactsNumbers = intent.getStringExtra(SELECTION_CONTACT_RESULT);
        if (usingColon) {
            if (selectContactsNumbers == null || selectContactsNumbers.length() < 1) {
                return null;
            }
            String[] numberArray = selectContactsNumbers.split(NUMBERS_SEPARATOR_COLON);
            String numberTempl = "";
            int simcolonIndex = -1;
            int colonIndex = -1;
            int separatorIndex = -1;
            for (int index = 0; index < numberArray.length; index++) {
                numberTempl = numberArray[index];
                simcolonIndex = numberTempl.indexOf(NUMBERS_SEPARATOR_SIMCOLON);
                colonIndex = numberTempl.indexOf(NUMBERS_SEPARATOR_COMMA);
                if (simcolonIndex > 0) {
                    if (colonIndex < 0) {
                        separatorIndex = simcolonIndex;
                    } else if (simcolonIndex < colonIndex) {
                        separatorIndex = simcolonIndex;
                    } else if (colonIndex > 0) {
                        separatorIndex = colonIndex;
                    }
                } else {
                    if (colonIndex > 0) {
                        separatorIndex = colonIndex;
                    }
                }
                if (separatorIndex > 0) {
                    numberArray[index] = numberTempl.substring(0, separatorIndex);
                }
                simcolonIndex = -1;
                colonIndex = -1;
                separatorIndex = -1;
            }
            return TextUtils.join(NUMBERS_SEPARATOR_SIMCOLON, numberArray);
        }
        return selectContactsNumbers;
    }

    private void processPickResult(final Intent data) {
        // The EXTRA_PHONE_URIS stores the phone's urls that were selected by user in the
        // multiple phone picker.
        /// M: Code analyze 013, Get contacts from Contact app . @{
        /*final Parcelable[] uris =
            data.getParcelableArrayExtra(Intents.EXTRA_PHONE_URIS);

        final int recipientCount = uris != null ? uris.length : 0;*/

        final long[] contactsId =
            data.getLongArrayExtra("com.mediatek.contacts.list.pickdataresult");
        final String mSelectContactsNumbers = getNumbersFromIntent(data);
        MmsLog.dpi(TAG,
                "processPickResult, data = " + data.toString() + ", contactsId = "
                        + Arrays.toString(contactsId) + ", mSelectContactsNumbers = "
                        + mSelectContactsNumbers);
        if ((contactsId == null || contactsId.length <= 0)
                && TextUtils.isEmpty(mSelectContactsNumbers)) {
            return;
        }
        int recipientCount = mRecipientsEditor.getRecipientCount();
        if (!TextUtils.isEmpty(mSelectContactsNumbers)) {
            recipientCount += mSelectContactsNumbers.split(";").length;
        } else {
            recipientCount += contactsId.length;
        }
        /// @}
        /// M: Code analyze 056,Now,the sms recipient limit is different from mms.
        /// We can set limit for sms or mms individually. @{
        final int recipientLimit = MmsConfig.getRecipientLimit();
        /// @}
        if (recipientLimit != Integer.MAX_VALUE && recipientCount > recipientLimit) {
            new AlertDialog.Builder(this)
                    .setTitle(R.string.pick_too_many_recipients)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setMessage(getString(R.string.too_many_recipients,
                            recipientCount, recipientLimit))
                    .setPositiveButton(android.R.string.ok, null)
                    .create().show();
            return;
        }

        /// M: @{
//        final Handler handler = new Handler();
        if (mContactPickDialog == null) {
            mContactPickDialog = new ProgressDialog(this);
            mContactPickDialog.setMessage(getText(R.string.adding_recipients));
            mContactPickDialog.setIndeterminate(true);
            mContactPickDialog.setCancelable(false);
        }

        // Only show the progress dialog if we can not finish off parsing the return data in 1s,
        // otherwise the dialog could flicker.
        mUiHandler.postDelayed(mContactPickRunnable, 500);
        new Thread(new Runnable() {
            public void run() {
                final ContactList list = new ContactList();
                final ContactList allList = new ContactList();
                final boolean isDuplicate = processDuplicatePickResult(mSelectContactsNumbers,
                        mRecipientsEditor, contactsId, list, allList);
                // TODO: there is already code to update the contact header widget and recipients
                // editor if the contacts change. we can re-use that code.
                final Runnable populateWorker = new Runnable() {
                    public void run() {
                        mConversation.setRecipients(allList);
                        if (list.size() > 0) {
                            // Fix ALPS01594370, if has attach, recipient eidtor always on focus.
                            // And adding a recipient bebind the editable recipient is not allowed.
                            if ((mBottomPanel == null)
                                    || (mBottomPanel.getVisibility() != View.VISIBLE)) {
                                mRecipientsEditor.clearFocus();
                            }
                            mIsPopulatingRecipients = true;
                            mRecipientsEditor.populate(list);
                        }
                        if (mRecipientsEditor != null && isRecipientsEditorVisible()) {
                            mRecipientsEditor.requestFocus();
                        }
                        if (isDuplicate) {
                            Toast.makeText(ComposeMessageActivity.this,
                                    R.string.add_duplicate_recipients, Toast.LENGTH_SHORT).show();
                        }
                    }
                };
                mUiHandler.post(populateWorker);
            }
        }, "ComoseMessageActivity.processPickResult").start();
    }

    private boolean mIsPopulatingRecipients = false;
    /// M: Code analyze 062, Resize image. @{
    private final ResizeImageResultCallback mResizeImageCallback =
        new ResizeImageResultCallback() {
        // TODO: make this produce a Uri, that's what we want anyway
        @Override
        public void onResizeResult(PduPart part, boolean append) {
            mNeedSaveAsMms = false;
            if (part == null) {
                MmsLog.d(TAG, "onResizeResult part == null");
                notifyCompressingDone();
                handleAddAttachmentError(WorkingMessage.MESSAGE_SIZE_EXCEEDED, R.string.type_picture);
                return;
            }

            mWorkingMessage.setResizeImage(true);
            Context context = ComposeMessageActivity.this;
            MtkPduPersister persister = MtkPduPersister.getPduPersister(context);
            int result;
            if (mWorkingMessage.isDiscarded()) {
                notifyCompressingDone();
                return;
            }
            Uri messageUri = mWorkingMessage.getMessageUri();
            if (null == messageUri) {
                try {
                    messageUri = mWorkingMessage.saveAsMms(true);
                } catch (IllegalStateException e) {
                    notifyCompressingDone();
                    MmsLog.e(TAG, e.getMessage() + ", go to ConversationList!");
                    goToConversationList();
                }
            }
            if (messageUri == null) {
                MmsLog.d(TAG, "onResizeResult messageUri == null");
                result = WorkingMessage.UNKNOWN_ERROR;
            } else {
                try {
                    /// M: it is modifying the mms draft, maybe
                    /// interlaced with WorkingMessage.saveDraft!
                    Uri dataUri;
                    int mode;
                    synchronized (WorkingMessage.sDraftMmsLock) {
                        dataUri = persister.persistPart(part,
                                ContentUris.parseId(messageUri), null);
                        mode = mWorkingMessage.sCreationMode;
                        mWorkingMessage.sCreationMode = 0;
                        result = mWorkingMessage.setAttachment(WorkingMessage.IMAGE,
                                dataUri, append);
                        /// M: fix bug ALPS00914391, remove redundancy part
                        if (result != mWorkingMessage.OK
                                && mWorkingMessage.getSlideshow() != null) {
                            try {
                                PduBody pb = mWorkingMessage.getSlideshow().toPduBody();
                                MessageUtils.updatePartsIfNeeded(mWorkingMessage.getSlideshow(),
                                        MtkPduPersister.getPduPersister(context),
                                        messageUri, pb, null);
                                if (pb != null) {
                                    mWorkingMessage.getSlideshow().sync(pb);
                                }
                            }  catch (MmsException e) {
                                Log.e(TAG, "Cannot update the message: " + messageUri, e);
                            }
                        }

                    }
                    mWorkingMessage.sCreationMode = mode;
                    if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                        log("ResizeImageResultCallback: dataUri=" + dataUri);
                    }
                } catch (MmsException e) {
                    result = WorkingMessage.UNKNOWN_ERROR;
                }
            }

            /// M:added for bug ALPS00317889 begin,for not pop up alertDialog if
            // attachment size is reaching limited size
            if (!mShowDialogForMultiImage) {
                handleAddAttachmentError(result, R.string.type_picture);
            }
            if (result == WorkingMessage.MESSAGE_SIZE_EXCEEDED) {
                mShowDialogForMultiImage = true;
            }
            /// M:added for bug ALPS00317889 end
            if (result == WorkingMessage.OK) {
                try {
                    if (mWorkingMessage.saveAsMms(false) != null) {
                        mHasDiscardWorkingMessage = true;
                    }
                } catch (IllegalStateException e) {
                    MmsLog.e(TAG, e.getMessage() + ", go to ConversationList!");
                    notifyCompressingDone();
                    goToConversationList();
                }
                mNeedSaveDraftAfterStop = false;
            }
            notifyCompressingDone();
            mWaitingAttachment = false;
        }
    };
    /// @}

    private void handleAddAttachmentError(final int error, final int mediaTypeStringId) {
        if (error == WorkingMessage.OK) {
            return;
        }
        Log.d(TAG, "handleAddAttachmentError: " + error);

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int titleId;
                int messageId;
                mWorkingMessage.removeFakeMmsForDraft();
                updateSendButtonState();

                switch(error) {
                /// M: Code analyze 063, For new feature ALPS00233419, Creation mode . @{
                case WorkingMessage.WARNING_TYPE:
                /// @}
                case WorkingMessage.UNKNOWN_ERROR:
                    Resources res = getResources();
                    String mediaType = res.getString(mediaTypeStringId);
                    String message = res.getString(R.string.error_add_attachment, mediaType);
                    Toast.makeText(ComposeMessageActivity.this,
                            message, Toast.LENGTH_SHORT).show();
                    return;
                case WorkingMessage.UNSUPPORTED_TYPE:
                /// M: Code analyze 063, For new feature ALPS00233419, Creation mode . @{
                case WorkingMessage.RESTRICTED_TYPE:
                /// @}
                    titleId = R.string.unsupport_media_type;
                    messageId = R.string.select_different_media_type;
                    break;
                case WorkingMessage.MESSAGE_SIZE_EXCEEDED:
                    titleId = R.string.exceed_message_size_limitation;
                    messageId = R.string.failed_to_add_image;
                    break;
                case WorkingMessage.IMAGE_TOO_LARGE:
                    titleId = R.string.failed_to_resize_image;
                    messageId = R.string.resize_image_error_information;
                    break;
                 /// M: Code analyze 063, For new feature ALPS00233419, Creation mode . @{
                case WorkingMessage.RESTRICTED_RESOLUTION:
                    titleId = R.string.select_different_media_type;
                    messageId = R.string.image_resolution_too_large;
                    break;
                /// @}
                default:
                    throw new IllegalArgumentException("unknown error " + error);
                }
                if (mErrorDialogShown) {
                    MessageUtils.showErrorDialog(ComposeMessageActivity.this, titleId, messageId,
                            0, 0);
                    mErrorDialogShown = false;
                }
            }
        });
    }

    /// M: Code analyze 064, Add image attachment. @{
    private void addImageAsync(final Uri uri, final boolean append) {
        mCompressingImage = true;
        getAsyncDialog().runAsync(new Runnable() {
            @Override
            public void run() {
                mShowDialogForMultiImage = false; /// M:added for bug ALPS00317889
                addImage(uri, append);
                if (mNeedSaveAsMms) {
                    saveAsMms(false);
                } else if (mNeedSaveDraftAfterStop) {
                    if ((!isRecipientsEditorVisible()) ||
                            (mRecipientsEditor.hasValidRecipient(mWorkingMessage.requiresMms()))) {
                        if (MessageUtils.allowSafeDraft(ComposeMessageActivity.this,
                                MmsConfig.getDeviceStorageFullStatus(), false,
                                TOAST_TYPE_FOR_SAVE_DRAFT)) {
                            saveDraft(true);
                            Log.v(TAG, "save draft if needed after compressing" +
                                    " image after onStop and update MmsWidget");
                            MmsWidgetProvider.notifyDatasetChanged(getApplicationContext());
                        }
                    }
                }
             }
        }, null, R.string.adding_attachments_title);
    }

    private void addImage(final Uri uri, final boolean append) {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "addImage: append=" + append + ", uri=" + uri);
        }
        mNeedSaveAsMms = true;
        int result = WorkingMessage.OK;
        try {
            if (append) {
                mWorkingMessage.checkSizeBeforeAppend();
            }
        } catch (ExceedMessageSizeException e) {
            result = WorkingMessage.MESSAGE_SIZE_EXCEEDED;
            notifyCompressingDone();
            handleAddAttachmentError(result, R.string.type_picture);
            mNeedSaveAsMms = false;
            return;
        }

        result = mWorkingMessage.setAttachment(WorkingMessage.IMAGE, uri, append);

        if (result != WorkingMessage.IMAGE_TOO_LARGE &&
                result != WorkingMessage.MESSAGE_SIZE_EXCEEDED) {
            mWaitingAttachment = false;
        }
        if (result == WorkingMessage.IMAGE_TOO_LARGE ||
            result == WorkingMessage.MESSAGE_SIZE_EXCEEDED) {
            MmsLog.dpi(TAG, "addImage: resize image " + uri);

            /// M: Adjust whether its a DRM IMAGE
            if (FeatureOption.MTK_DRM_APP) {
                if (!DrmUtilsEx.checkUriContainsDrm(this, uri)) {
                    mToastCountForResizeImage++;
                    if (mToastCountForResizeImage == 1) {
                        MessageUtils.resizeImage(this, uri,
                                mAttachmentEditorHandler, mResizeImageCallback, append,
                            true);
                    } else {
                        MessageUtils.resizeImage(this, uri,
                                mAttachmentEditorHandler, mResizeImageCallback, append,
                            false);
                    }
                } else {
                    notifyCompressingDone();
                    handleAddAttachmentError(result, R.string.type_picture);
                    mNeedSaveAsMms = false;
                }
            } else {
                mToastCountForResizeImage++;
                if (mToastCountForResizeImage == 1) {
                    MessageUtils.resizeImage(this, uri,
                            mAttachmentEditorHandler, mResizeImageCallback, append, true);
                } else {
                    MessageUtils.resizeImage(this, uri,
                            mAttachmentEditorHandler, mResizeImageCallback, append, false);
                }
            }
            return;
        } else if (result == WorkingMessage.WARNING_TYPE) {
            mNeedSaveAsMms = false;
            notifyCompressingDone();
            runOnUiThread(new Runnable() {
                public void run() {
                    showConfirmDialog(uri, append,
                            WorkingMessage.IMAGE, R.string.confirm_restricted_image);
                }
            });
            return;
        }
        notifyCompressingDone();
        if (result != WorkingMessage.OK) {
            mNeedSaveAsMms = false;
        }
        handleAddAttachmentError(result, R.string.type_picture);
    }
    /// @}

    /// M: Code analyze 065, Add video attachment. @{
    private void addVideoAsync(final Uri uri, final boolean append) {
        getAsyncDialog().runAsync(new Runnable() {
            @Override
            public void run() {
                addVideo(uri, append);
                saveAsMms(false);
           }
        }, null, R.string.adding_attachments_title);
    }

    private void addVideo(final Uri uri, final boolean append) {
        mNeedSaveAsMms = false;
        if (uri != null) {
            mNeedSaveAsMms = true;
            int result = WorkingMessage.OK;
            try {
                if (append) {
                    mWorkingMessage.checkSizeBeforeAppend();
                }
            } catch (ExceedMessageSizeException e) {
                result = WorkingMessage.MESSAGE_SIZE_EXCEEDED;
                handleAddAttachmentError(result, R.string.type_video);
                mNeedSaveAsMms = false;
                return;
            }
            result = mWorkingMessage.setAttachment(WorkingMessage.VIDEO, uri, append);
            if (result == WorkingMessage.WARNING_TYPE) {
                mNeedSaveAsMms = false;
                runOnUiThread(new Runnable() {
                    public void run() {
                        showConfirmDialog(uri, append,
                                WorkingMessage.VIDEO, R.string.confirm_restricted_video);
                    }
                });
            } else {
                handleAddAttachmentError(result, R.string.type_video);
                if (result != WorkingMessage.OK) {
                    mNeedSaveAsMms = false;
                }
            }
        }
    }
    /// @}

    private void addAudioAsync(final Uri uri, final boolean append) {
        getAsyncDialog().runAsync(new Runnable() {
            @Override
            public void run() {
                addAudio(uri, append);
                saveAsMms(false);
            }
        }, null, R.string.adding_attachments_title);
    }

    AsyncDialog getAsyncDialog() {
        if (mAsyncDialog == null) {
            mAsyncDialog = new AsyncDialog(this);
        }
        return mAsyncDialog;
    }

    public boolean getForwordingState() {
        return mForwardingMessage;
    }

    /// M: Code analyze 017, Handle forwarded message.(see:forwardMessage())@{
    private boolean handleForwardedMessage() {
        Intent intent = getIntent();

        // If this is a forwarded message, it will have an Intent extra
        // indicating so.  If not, bail out.
        if (!mForwardMessageMode) {
            mForwardingMessage = false;
            return false;
        }
        mForwardingMessage = true;

        Uri uri = intent.getParcelableExtra("msg_uri");

        if (Log.isLoggable(LogTag.APP, Log.DEBUG)) {
            MmsLog.dpi(TAG, "handle forwarded message " + uri);
        }

        if (uri != null) {
            mWorkingMessage = WorkingMessage.load(this, uri);
            if (mWorkingMessage != null) {
                mWorkingMessage.setSubject(intent.getStringExtra("subject"), false);
                SlideshowModel mSlideshowModel = mWorkingMessage.getSlideshow();
                if (mSlideshowModel != null) {
                    int mSsmSize = mSlideshowModel.size();
                    for (int index = 0; index < mSsmSize; index++) {
                        SlideModel mSlideModel = mSlideshowModel.get(index);
                        if (mSlideModel != null) {
                            if (mSlideModel.hasText()) {
                                TextModel mTextModel = mSlideModel.getText();
                                String textChar = mTextModel.getText();
                                long textLength = textChar.length();
                                if (textLength > MmsConfig.getMaxTextLimit()) {
                                    mTextModel.setText(
                                            textChar.substring(0, MmsConfig.getMaxTextLimit()));
                                }
                            }
                        }
                    }
                }
            }
        } else {
            String smsAddress = null;
            if (intent.hasExtra(SMS_ADDRESS)) {
                smsAddress = intent.getStringExtra(SMS_ADDRESS);
                if (smsAddress != null) {
                   //TODO need re-coding
                   //mRecipientsEditor.addRecipient(smsAddress, true);
                }
            }
            mWorkingMessage.setText(intent.getStringExtra(SMS_BODY));
        }
        /// M:
        // add for ipmessage
        mIpCompose.onIpHandleForwardedMessage(intent);
        // let's clear the message thread for forwarded messages
        mMsgListAdapter.changeCursor(null);

        return true;
    }
    /// @}

    // Handle send actions, where we're told to send a picture(s) or text.
    private boolean handleSendIntent() {
        Intent intent = getIntent();
        Bundle extras = intent.getExtras();
        if (extras == null) {
            if (UserManager.get(getApplicationContext()).hasUserRestriction(
                  UserManager.DISALLOW_SMS)){
                Toast.makeText(ComposeMessageActivity.this,
                    R.string.user_has_no_premission, Toast.LENGTH_SHORT).show();
            }
            return false;
        }
        /// M: Code analyze 066,Handle intent. @{
        /// M: add for saveAsMms
        mWorkingMessage.setConversation(mConversation);
        final String mimeType = intent.getType();
        String action = intent.getAction();
        MmsLog.i(TAG, "Get mimeType: " + mimeType + " Get action: " + action);
        /// M: disable when non-default sms
        if (!mIsSmsEnabled) {
            if (UserManager.get(getApplicationContext()).hasUserRestriction(
                  UserManager.DISALLOW_SMS)){
                Toast.makeText(ComposeMessageActivity.this,
                    R.string.user_has_no_premission, Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(ComposeMessageActivity.this,
                    R.string.compose_disabled_toast, Toast.LENGTH_SHORT).show();
            }
            return false;
        }
        if (Intent.ACTION_SEND.equals(action)) {
            if (extras.containsKey(Intent.EXTRA_STREAM)) {
                final Uri uri = (Uri) extras.getParcelable(Intent.EXTRA_STREAM);
                if (uri == null) {
                    MmsLog.w(TAG, "handleSendIntent uri == null");
                    return false;
                }
                if (mimeType.equals("text/plain")) {
                    String fileName = "";
                    if (uri != null) {
                        if (uri.getScheme() != null && uri.getScheme().equals("file")) {
                            String mUriStr = Uri.decode(uri.toString());
                            fileName = mUriStr.substring(mUriStr.lastIndexOf("/") + 1,
                                    mUriStr.length());
                        } else {
                            Cursor c = mContentResolver.query(uri, null, null, null, null);
                            if (c != null) {
                                try {
                                    if (c.getCount() == 1 && c.moveToFirst()) {
                                        fileName = c.getString(
                                                c.getColumnIndex(Images.Media.DISPLAY_NAME));
                                    }
                                } catch (Exception e) {
                                    e.printStackTrace();
                                } finally {
                                    c.close();
                                }
                            }
                        }
                    }
                    if (fileName == null) {
                        fileName = this.getString(R.string.type_common_file);
                    }
                    String mMessage = this.getString(R.string.failed_to_add_media, fileName);
                    Toast.makeText(this, mMessage, Toast.LENGTH_SHORT).show();
                    return false;
                }
                getAsyncDialog().runAsync(new Runnable() {
                    @Override
                    public void run() {
                        /// M: fix bug ALPS00397146, removeThumbnailManager uri
                        // (Content://media/external/images/media/) when it rotated
                        String fileName = "";
                        int degree = 0;
                        String uriStr = uri.toString();
                        if (uriStr.startsWith("content://media/external/images/media")) {
                            Cursor c = mContentResolver.query(uri, null, null, null, null);
                            if (c != null) {
                                try {
                                    if (c.getCount() == 1 && c.moveToFirst()) {
                                        fileName = c.getString(
                                                c.getColumnIndex(Images.Media.DISPLAY_NAME));
                                        degree = c.getInt(
                                                c.getColumnIndex(Images.Media.ORIENTATION));
                                    }
                                } catch (Exception e) {
                                    e.printStackTrace();
                                } finally {
                                    c.close();
                                }
                            }

                            if (sDegreeMap != null && sDegreeMap.containsKey(uriStr)) {
                                if (sDegreeMap.get(uriStr).intValue() != degree) {
                                    Uri thumbnailUri =
                                        Uri.parse(uriStr + ThumbnailManager.FLAG_FNAME + fileName);
                                    MmsApp.getApplication().getThumbnailManager()
                                                                    .removeThumbnail(thumbnailUri);
                                    sDegreeMap.remove(uriStr);
                                    sDegreeMap.put(uriStr, degree);
                                }
                            } else if (sDegreeMap != null) {
                                sDegreeMap.put(uriStr, degree);
                            }
                        }
                        /// @}
                        Uri tempUri = uri;
                        if (uri.toString().contains(TempFileProvider.SCRAP_VIDEO_URI.toString())) {
                            File file = new File(TempFileProvider
                                    .getScrapVideoPath(ComposeMessageActivity.this));
                            if (file != null && file.exists()) {
                                tempUri = Uri.fromFile(file);
                            }
                        }

                        String scheme = tempUri.getScheme();
                        if (scheme != null && scheme.equals("file")) {
                            /// M: fix bug ALPS01400468, workaround for GoogleDrive Audio Uri
                            if (uri.toString().contains(AudioModel.sAuthorityForGoogleDrive)) {
                                AudioModel.sTypeForGoogleDrive = mimeType;
                            }
                            addFileAttachment(mimeType, tempUri, false);
                        } else {
                            String type = MessageUtils.queryContentType(
                                    ComposeMessageActivity.this, (Uri) uri);
                            if (TextUtils.isEmpty(type)) {
                                type = mimeType;
                            }
                            addAttachment(type, tempUri, false);
                        }
                        SlideshowModel slides = mWorkingMessage.getSlideshow();
                        if (slides != null && (slides.size() > 0
                                || slides.sizeOfFilesAttach() > 0)) {
                            mWorkingMessage.saveAsMms(false);
                        }
                        VCardUtils.deleteVCardTempFiles(getApplicationContext(), mVCardFiles);
                    }
                }, null, R.string.adding_attachments_title);
                intent.setAction(SIGN_CREATE_AFTER_KILL_BY_SYSTEM);
                return true;
            } else if (extras.containsKey(Intent.EXTRA_TEXT)) {
                mWorkingMessage.setText(extras.getString(Intent.EXTRA_TEXT));
                intent.setAction(SIGN_CREATE_AFTER_KILL_BY_SYSTEM);
                return true;
            }
        } else if (Intent.ACTION_SEND_MULTIPLE.equals(action) &&
                extras.containsKey(Intent.EXTRA_STREAM)) {
            SlideshowModel slideShow = mWorkingMessage.getSlideshow();
            final ArrayList<Parcelable> uris = extras.getParcelableArrayList(Intent.EXTRA_STREAM);
            int currentSlideCount = slideShow != null ? slideShow.size() : 0;
            int importCount = uris.size();
            if (importCount + currentSlideCount > SlideshowEditor.MAX_SLIDE_NUM) {
                importCount = Math.min(SlideshowEditor.MAX_SLIDE_NUM - currentSlideCount,
                        importCount);
            }

            // Attach all the pictures/videos asynchronously off of the UI thread.
            // Show a progress dialog if adding all the slides hasn't finished
            // within half a second.
            final int numberToImport = importCount;
            MmsLog.i(TAG, "numberToImport: " + numberToImport);
            final WorkingMessage msg = mWorkingMessage;
            getAsyncDialog().runAsync(new Runnable() {
                @Override
                public void run() {
                    mToastCountForResizeImage = 0;
                    for (int i = 0; i < numberToImport; i++) {
                        Parcelable uri = uris.get(i);

                        String scheme = ((Uri) uri).getScheme();
                        String authority = ((Uri) uri).getAuthority();
                        if (scheme != null && scheme.equals("file")) {
                            // change "file://..." Uri to "Content://....,
                            // and attemp to add this attachment"
                            /// M: fix bug ALPS604911, change MmsContentType
                            /// when share multi-file from FileManager @{
                            String type = MessageUtils.getContentType((Uri) uri);
                            if (TextUtils.isEmpty(type)) {
                                type = mimeType;
                            }
                            /// @}
                            addFileAttachment(type, (Uri) uri, true);
                        } else if (authority != null && (authority.contains("com.android.email")
                                || (authority.equals(MediaStore.AUTHORITY)
                                        && mimeType != null && mimeType.contains("*")))) {
                            addAttachment(mimeType, (Uri) uri, true);
                        } else {
                            String type = MessageUtils.queryContentType(
                                    ComposeMessageActivity.this, (Uri) uri);
                            if (TextUtils.isEmpty(type)) {
                                type = mimeType;
                            }
                            addAttachment(type, (Uri) uri, true);
                        }
                    }
                    mToastCountForResizeImage = 0;
                    SlideshowModel slides = mWorkingMessage.getSlideshow();
                    if (slides != null && (slides.size() > 0 || slides.sizeOfFilesAttach() > 0)) {
                        mWorkingMessage.saveAsMms(false);
                    }
                    VCardUtils.deleteVCardTempFiles(getApplicationContext(), mVCardFiles);
                }
            }, null, R.string.adding_attachments_title);
            intent.setAction(SIGN_CREATE_AFTER_KILL_BY_SYSTEM);
            return true;
        } else if (SIGN_CREATE_AFTER_KILL_BY_SYSTEM.equals(action)) {
        /// @}
            return true;
        }
        return false;
    }

    // mVideoUri will look like this: content://media/external/video/media
    private static final String mVideoUri = Video.Media.getContentUri("external").toString();
    // mImageUri will look like this: content://media/external/images/media
    private static final String mImageUri = Images.Media.getContentUri("external").toString();

    private void addAttachment(String type, Uri uri, boolean append) {
        if (uri != null) {
            // When we're handling Intent.ACTION_SEND_MULTIPLE, the passed in items can be
            // videos, and/or images, and/or some other unknown types we don't handle. When
            // a single attachment is "shared" the type will specify an image or video. When
            // there are multiple types, the type passed in is "*/*". In that case, we've got
            // to look at the uri to figure out if it is an image or video.
            boolean wildcard = "*/*".equals(type);
            MmsLog.i(TAG, "Got send intent mimeType :" + type);
            if (type == null) {
                handleAddAttachmentError(WorkingMessage.UNSUPPORTED_TYPE, R.string.type_picture);
            }
            else if (type.startsWith("image/") || (wildcard &&
                        uri.toString().startsWith(mImageUri))) {
                addImage(uri, append);
            } else if (type.startsWith("video/") ||
                    (wildcard && uri.toString().startsWith(mVideoUri))) {
                addVideo(uri, append);
            }
             /// M: Code analyze 067, Add audio attachment. @{
            else if (type.startsWith("audio/") || type.equals("application/ogg")
                || (wildcard && uri.toString().startsWith(mAudioUri))) {
                addAudio(uri, append);
            /// @}
            /// M: Code analyze 019, Add vcard attachment.  @{
            } else if (type.equalsIgnoreCase("text/x-vcard")) {
                VCardAttachment va = new VCardAttachment(ComposeMessageActivity.this);
                String fileName = va.getVCardFileNameByUri(uri);
                setFileAttachment(fileName, WorkingMessage.VCARD, false);
                mVCardFiles.add(fileName);
             /// M: Code analyze 020, Add vcalendar attachment.  @{
            } else if (type.equalsIgnoreCase("text/x-vcalendar")) {
                attachVCalendar(uri);
            } else {
                handleAddAttachmentError(WorkingMessage.UNSUPPORTED_TYPE, R.string.type_audio);
            }
            /// @}
        }
    }

    private String getResourcesString(int id, String mediaName) {
        Resources r = getResources();
        return r.getString(id, mediaName);
    }

    private void drawBottomPanel() {
        // Reset the counter for text editor.
        /// M: @{
        mDrawBottomPanel = false;
        /// M: remove Google default code
        // Reset the counter for text editor.
        //resetCounter();

        if (mWorkingMessage.hasSlideshow()) {
            if (mWorkingMessage.getIsUpdateAttachEditor()) {
                mBottomPanel.setVisibility(View.GONE);
                if (mSubjectTextEditor != null) {
                    //mSubjectTextEditor.setNextFocusDownId(-1);
                    mSubjectTextEditor.setImeOptions(EditorInfo.IME_ACTION_DONE);
                    mSubjectTextEditor.setImeActionLabel(
                            getText(com.android.internal.R.string.ime_action_done),
                            EditorInfo.IME_ACTION_DONE);
                }

                mAttachmentEditor.update(mWorkingMessage);
                mAttachmentEditor.requestFocus();
            } else {
                MmsLog.d(TAG, "drawBottomPanel, isUpdateAttachEditor == false");
            }
            if (mRecipientsEditor != null) {
                mRecipientsEditor.setEnabled(mIsSmsEnabled);
                mRecipientsEditor.setFocusableInTouchMode(mIsSmsEnabled);
                mRecipientsEditor.setIsTouchable(mIsSmsEnabled);
            }
            if (mSubjectTextEditor != null) {
                mSubjectTextEditor.setEnabled(mIsSmsEnabled);
                mSubjectTextEditor.setFocusableInTouchMode(mIsSmsEnabled);
            }
            if (mRecipientsPicker != null) {
                mRecipientsPicker.setEnabled(mIsSmsEnabled);
            }
            if (mTextEditor != null) {
                int len = mTextEditor.getText().length();
                if (len > 0) {
                    try {
                        mTextEditor.setSelection(len);
                    } catch (IndexOutOfBoundsException e) {
                        mTextEditor.setSelection(len - 1);
                    }
                }
                hideKeyboard();
            }
            return;
        }

        mAttachmentEditor.update(mWorkingMessage);
        updateTextEditorHeightInFullScreen();
        /// @}
        mBottomPanel.setVisibility(View.VISIBLE);
        if (mSubjectTextEditor != null) {
            //mSubjectTextEditor.setNextFocusDownId(R.id.embedded_text_editor);
            mSubjectTextEditor.setImeOptions(EditorInfo.IME_ACTION_NEXT);
            mSubjectTextEditor.setImeActionLabel(
                    getText(com.android.internal.R.string.ime_action_next),
                    EditorInfo.IME_ACTION_NEXT);
        }

        CharSequence text = mWorkingMessage.getText();

        // TextView.setTextKeepState() doesn't like null input.
        mTextEditor.removeTextChangedListener(mTextEditorWatcher);
        if (mIsSmsEnabled) {
            mTextEditor.addTextChangedListener(mTextEditorWatcher);
            if (text != null) {
                mTextEditor.setTextKeepState(text);
                /// M: @{
                try {
                    mTextEditor.setSelection(mTextEditor.getText().toString().length());
                } catch (IndexOutOfBoundsException e) {
                    mTextEditor.setSelection(mTextEditor.getText().toString().length() - 1);
                }
                /// @}
            } else {
                mTextEditor.setText("");
            }
        } else {
            mTextEditor.setText("");
            mTextEditor.setHint(R.string.sending_disabled_not_default_app);
            mTextEditor.setFocusable(false);
        }
        mTextEditor.setEnabled(mIsSmsEnabled);
        mTextEditor.setFocusableInTouchMode(mIsSmsEnabled);

        if (mRecipientsEditor != null) {
            mRecipientsEditor.setEnabled(mIsSmsEnabled);
            mRecipientsEditor.setFocusableInTouchMode(mIsSmsEnabled);
            mRecipientsEditor.setIsTouchable(mIsSmsEnabled);
        }
        if (mSubjectTextEditor != null) {
            mSubjectTextEditor.setEnabled(mIsSmsEnabled);
            mSubjectTextEditor.setFocusableInTouchMode(mIsSmsEnabled);
        }
        if (mRecipientsPicker != null) {
            mRecipientsPicker.setEnabled(mIsSmsEnabled);
        }

        /// M: add for character counter
        // Reset the counter for text editor.
        updateCounter(mWorkingMessage.getText(), 0, 0, 0);
    }

    private void drawTopPanel(boolean showSubjectEditor) {
        /// M: why ? @{
        //boolean showingAttachment = mAttachmentEditor.update(mWorkingMessage);
        //mAttachmentEditorScrollView.setVisibility(showingAttachment ? View.VISIBLE : View.GONE);
        //mAttachmentEditorScrollView.setVisibility(showingAttachment ? View.VISIBLE : View.GONE);

        boolean isHasSubject = false;
        if (mWorkingMessage == null) {
            isHasSubject = false;
        } else {
            isHasSubject = mWorkingMessage.hasSubject();
        }
        boolean isDeleteMode = false;
        if (mMsgListAdapter == null) {
            isDeleteMode = false;
        } else {
            isDeleteMode = mMsgListAdapter.mIsDeleteMode;
        }
        showSubjectEditor((showSubjectEditor || isHasSubject) && !isDeleteMode);
        mAttachmentEditor.update(mWorkingMessage);
        updateTextEditorHeightInFullScreen();
        /// @}
    }

    //==========================================================
    // Interface methods
    //==========================================================

    @Override
    public void onClick(View v) {
        /// M: Code analyze 028, Before sending message,check the recipients count
        /// and add sub card selection dialog if multi sub cards exist.@{
        /*if ((v == mSendButtonSms || v == mSendButtonMms) && isPreparedForSending()){
            confirmSendMessageIfNeeded();
        }
        */

        if (mOpComposeExt.onClick(v, mRecipientsPicker)) {
            return;
        }
        if (v == mSendButtonSms || v == mSendButtonMms) {
            MmsLog.d(TAG, "onClick send button click!");
            if (mSendButtonCanResponse) {
                ///M: WFC: Show pop-up, if condition satisfy @ {
/*                if (showWfcSendButtonPopUp()) {
                    return;
                }
                */
                /// @}
                mSendButtonCanResponse = false;
                if (isPreparedForSending()) {
                    /// M: Since sending message here, why not disable button 'Send'??
                    /* Performance optimize, use send special send buttong update*/
                    //updateSendButtonState(false);
                    updateSendButtonForSend();
                    checkRecipientsCount();
                    mUiHandler.sendEmptyMessageDelayed(MSG_RESUME_SEND_BUTTON,
                            RESUME_BUTTON_INTERVAL);
                } else {
                    mSendButtonCanResponse = true;
                    unpreparedForSendingAlert();
                }
            }
        /// @}
        } else if ((v == mRecipientsPicker)) {
             /// M: Code analyze 013, Get contacts from Contact app . @{
             //launchMultiplePhonePicker();
            if (recipientCount() >= RECIPIENTS_LIMIT_FOR_SMS) {
                Toast.makeText(ComposeMessageActivity.this,
                        R.string.cannot_add_recipient, Toast.LENGTH_SHORT).show();
            } else {
                /// M: fix bug ALPS00444752, dis-clickble when showing ContactPicker
                if (!mShowingContactPicker) {
                    addContacts(mRecipientsEditor != null
                            ? (RECIPIENTS_LIMIT_FOR_SMS - mRecipientsEditor.getNumbers().size())
                                    : RECIPIENTS_LIMIT_FOR_SMS, REQUEST_CODE_PICK);
                }
            }
             /// @}
        }
    }

    /// M: fix bug ALPS00444752, set false to enable to Show ContactPicker
    private boolean mShowingContactPicker = false;

    private void launchMultiplePhonePicker() {
        Intent intent = new Intent(Intents.ACTION_GET_MULTIPLE_PHONES);
        intent.addCategory("android.intent.category.DEFAULT");
        intent.setType(Phone.CONTENT_TYPE);
        // We have to wait for the constructing complete.
        ContactList contacts = mRecipientsEditor.constructContactsFromInput(true);
        int urisCount = 0;
        Uri[] uris = new Uri[contacts.size()];
        urisCount = 0;
        for (Contact contact : contacts) {
            if (Contact.CONTACT_METHOD_TYPE_PHONE == contact.getContactMethodType()) {
                    uris[urisCount++] = contact.getPhoneUri();
            }
        }
        if (urisCount > 0) {
            intent.putExtra(Intents.EXTRA_PHONE_URIS, uris);
        }
        startActivityForResult(intent, REQUEST_CODE_PICK);
    }

    @Override
    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
        if (event != null) {
            // if shift key is down, then we want to insert the '\n' char in the TextView;
            // otherwise, the default action is to send the message.
            if (!event.isShiftPressed()) {
                if (event.getAction() == KeyEvent.ACTION_DOWN) {
                    return false;
                }
                /// M: Code analyze 028, Before sending message,check the recipients count
                /// and add sub card selection dialog if multi sub cards exist.@{
                if (isPreparedForSending()) {
                    checkRecipientsCount();
                } else {
                    unpreparedForSendingAlert();
                    }
                /// @}
                return true;
            }
            return false;
        }
        /// M: Code analyze 028, Before sending message,check the recipients count
        /// and add sub card selection dialog if multi sub cards exist.@{
        if (isPreparedForSending()) {
            //confirmSendMessageIfNeeded();
            checkRecipientsCount();
        } else {
            unpreparedForSendingAlert();
        }
        /// @}
        return true;
    }

    private final TextWatcher mTextEditorWatcher = new TextWatcher() {
        private int mStart;

        /// M: fix bug ALPS00612093, postDelay for ANR
        private Runnable mUpdateRunnable = new Runnable() {
            public void run() {
                updateSendButtonState();
            }
        };

        private Runnable mRunnable = new Runnable() {
            public void run() {
                Toast.makeText(ComposeMessageActivity.this, R.string.dialog_sms_limit,
                        Toast.LENGTH_SHORT).show();
            }
        };

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            mBeforeTextChangeString = s.toString();
            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                MmsLog.v(TAG, "beforeTextChanged count = " + count + " after =" + after);
            }
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            // This is a workaround for bug 1609057. Since onUserInteraction() is
            // not called when the user touches the soft keyboard, we pretend it was
            // called when textfields changes.  This should be removed when the bug
            // is fixed.
            onUserInteraction();

            mWorkingMessage.setText(s);
            /// M: fix bug ALPS00712509, show toast when paste many word and > 300k
            if (mWorkingMessage.isExceedSize()) {
                mWorkingMessage.setIsExceedSize(false);
                mUiHandler.removeCallbacks(mRunnable);
                mUiHandler.postDelayed(mRunnable, 200);
            }
            /// M: @{
            mAttachmentEditor.onTextChangeForOneSlide();
            /// @}

            /// M: fix bug ALPS00612093, postDelay for ANR
            mUiHandler.removeCallbacks(mUpdateRunnable);
            mUiHandler.postDelayed(mUpdateRunnable, 100);

            updateCounter(s, start, before, count);
            mStart = start;
            /// M: @{
            //ensureCorrectButtonHeight();
            /// @}

            // add for ipmessage
            mIpCompose.onIpTextChanged(s, start, before, count);
        }

        @Override
        public void afterTextChanged(Editable s) {
            // add for ipmessage
            mIpCompose.onIpAfterTextChanged(s, mBeforeTextChangeString);
        }
    };

    /**
     * Ensures that if the text edit box extends past two lines then the
     * button will be shifted up to allow enough space for the character
     * counter string to be placed beneath it.
     */
    /*** M: remove Google default code
    private void ensureCorrectButtonHeight() {
        int currentTextLines = mTextEditor.getLineCount();
        if (currentTextLines <= 2) {
            mTextCounter.setVisibility(View.GONE);
        }
        else if (currentTextLines > 2 && mTextCounter.getVisibility() == View.GONE) {
            // Making the counter invisible ensures that it is used to correctly
            // calculate the position of the send button even if we choose not to
            // display the text.
            mTextCounter.setVisibility(View.INVISIBLE);
        }
    }
    */

    private final TextWatcher mSubjectEditorWatcher = new TextWatcher() {
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) { }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            mWorkingMessage.setSubject(s, true);
            if (s != null && TextUtils.getTrimmedLength(s) > 0) {
                /// M: Code analyze 032, According to the message state to update text counter.@{
                mTextCounter.setVisibility(View.GONE);
                // / @}
                // updateSendButtonState();
            }
            // / M: for fixed ALPS00562999,when change subject to null,update send button.@{
            updateSendButtonState();
            // / @}
        }

        @Override
        public void afterTextChanged(Editable s) {
            String subjectString = s.toString();
            if (subjectString.length() == 0) {
                Log.d(TAG, "[mSubjectEditorWatcher.afterTextChanged] subject lenght is 0,"
                    + "current mms or sms : " + mWorkingMessage.requiresMms());
                if (!mWorkingMessage.requiresMms()) {
                    if (mConversation != null && mWorkingMessage != null) {
                        mWorkingMessage.asyncDeleteDraftMmsMessage(mConversation);
                        mWorkingMessage.clearConversation(mConversation, true);
                    }
                }
            }
        }
    };

    //==========================================================
    // Private methods
    //==========================================================

    /**
     * Initialize all UI elements from resources.
     */
    private void initResourceRefs() {
        /// M: Code analyze 004, Set max height for text editor. @{
        mHeightChangedLinearLayout =
            (HeightChangedLinearLayout) findViewById(R.id.changed_linear_layout);
        mHeightChangedLinearLayout.setLayoutSizeChangedListener(mLayoutSizeChangedListener);
        /// @}
        mMsgListView = (MessageListView) findViewById(R.id.history);
        mMsgListView.setDivider(null);      // no divider so we look like IM conversation.
        mMsgListView.setDividerHeight(getResources()
                .getDimensionPixelOffset(R.dimen.ipmsg_message_list_divier_height));

        // called to enable us to show some padding between the message list and the
        // input field but when the message list is scrolled that padding area is filled
        // in with message content
        mMsgListView.setClipToPadding(false);

        /** M: 4.1  used this code.
        mMsgListView.setOnSizeChangedListener(new OnSizeChangedListener() {
            public void onSizeChanged(int width, int height, int oldWidth, int oldHeight) {
                // The message list view changed size, most likely because the keyboard
                // appeared or disappeared or the user typed/deleted chars in the message
                // box causing it to change its height when expanding/collapsing to hold more
                // lines of text.
                smoothScrollToEnd(false, height - oldHeight);
            }
        });
        */

        /// M: turn off children clipping because we draw the border outside of our own
        /// M: bounds at the bottom.  The background is also drawn in code to avoid drawing
        /// M: the top edge.
        mMsgListView.setClipChildren(false);

        mBottomPanel = findViewById(R.id.bottom_panel);
        mTextEditor = (EditText) findViewById(R.id.embedded_text_editor);
        /// M: @{
        //  mTextEditor.setOnEditorActionListener(this);
        /// @}
        mTextEditor.removeTextChangedListener(mTextEditorWatcher);
        mTextEditor.addTextChangedListener(mTextEditorWatcher);
        mTextEditor.setFilters(new InputFilter[] {
                new TextLengthFilter(MmsConfig.getMaxTextLimit())});
        mTextEditor.setOnKeyListener(new OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                // add for ipmessage
                mIpCompose.onIpTextEditorKey(v, keyCode, event);
                return false;
            }
        });

        mTextEditor.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                mIpCompose.onIpTextEditorTouch();
                mOpComposeExt.onTextEditorTouch();
                return false;
            }
        });

        mTextEditor.addOnLayoutChangeListener(new View.OnLayoutChangeListener() {
            @Override
            public void onLayoutChange(View v, int left, int top, int right,
                    int bottom, int oldLeft, int oldTop, int oldRight, int oldBottom) {
                if (Log.isLoggable(TAG, Log.VERBOSE)) {
                    MmsLog.v(TAG, "mTextEditor onLayoutChange mUpdateForScrnOrientationChanged = "
                            + mUpdateForScrnOrientationChanged);
                }
                if (mUpdateForScrnOrientationChanged) {
                    mUiHandler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            updateCounter(mWorkingMessage.getText(), 0, 0, 0);
                        }
                    }, 1000);
                    mUpdateForScrnOrientationChanged = false;
                }
            }
        });

        mTextCounter = (TextView) findViewById(R.id.text_counter);
        mSendButtonMms = (TextView) findViewById(R.id.send_button_mms);
        mSendButtonSms = (ImageButton) findViewById(R.id.send_button_sms);
        mSendButtonMms.setOnClickListener(this);
        mSendButtonSms.setOnClickListener(this);
        mTopPanel = findViewById(R.id.recipients_subject_linear);
        mTopPanel.setFocusable(false);
        mAttachmentEditor = (AttachmentEditor) findViewById(R.id.attachment_editor);
        mAttachmentEditor.setHandler(mAttachmentEditorHandler);
        //mAttachmentEditorScrollView = findViewById(R.id.attachment_editor_scroll_view);
        mQuickContact = (QuickContactBadge) findViewById(R.id.avatar);
        mOpComposeExt.initResourceRefs((LinearLayout) findViewById(R.id.button_with_counter),
                null,
                mAttachmentEditor.mOpAttachmentEditorExt);
    }

    private void confirmDeleteDialog(OnClickListener listener, boolean locked) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        /// M: Code analyze 027,Add for deleting one message.@{
        // Set different title and icon for locked message.
        builder.setTitle(locked ? R.string.confirm_dialog_locked_title :
            R.string.confirm_dialog_title);
        builder.setIconAttribute(android.R.attr.alertDialogIcon);
        /// @}
        builder.setCancelable(true);
        builder.setMessage(locked ? R.string.confirm_delete_locked_message :
                    R.string.confirm_delete_message);
        builder.setPositiveButton(R.string.delete, listener);
        builder.setNegativeButton(R.string.no, null);
        builder.show();
    }

    void undeliveredMessageDialog(long date) {
        String body;

        if (date >= 0) {
            body = getString(R.string.undelivered_msg_dialog_body,
                    MessageUtils.formatTimeStampString(this, date));
        } else {
            // FIXME: we can not get sms retry time.
            body = getString(R.string.undelivered_sms_dialog_body);
        }

        Toast.makeText(this, body, Toast.LENGTH_LONG).show();
    }

    /* Performance optimize, reduce msg list query delay time*/
    private int msgListQueryDelay = 10; //100;
    private void startMsgListQuery() {
        startMsgListQuery(MESSAGE_LIST_QUERY_TOKEN, msgListQueryDelay);
    }

    private void startMsgListQuery(final int token, int delay) {
        if (isRecipientsEditorVisible()) {
            return;
        }

        final Uri conversationUri = mConversation.getUri();

        MmsLog.dpi(TAG, "startMsgListQuery, uri= " + conversationUri +
                " token = " + token + " mQueryingState = " + mQueryingState);
        if (conversationUri == null) {
            mQueryingState = QUERY_IDLE;
            log("##### startMsgListQuery: conversationUri is null, bail!");
            return;
        }

        final long threadId = mConversation.getThreadId();
        try {
            // check query state, don't start the query if already has a querying
            if (mQueryingState == QUERY_IDLE) {
                mQueryingState = QUERY_RUNNING;
                mBackgroundQueryHandler.postDelayed(new Runnable() {
                    public void run() {
                        Uri conversationUriLimit = Uri.parse(conversationUri +
                                "/limit/" + MAX_MESSAGE_PER_QUERY +
                                "/offset/" + queryOffset);

                        mBackgroundQueryHandler.startQuery(
                                token,
                                threadId,
                                conversationUriLimit,
                                PROJECTION,
                                null, null, null);
                    }
                }, delay);
            } else {
                // pending the query, restart it when last query returned
                MmsLog.dpi(TAG, "startMsgListQuery, query running, return");
                mQueryingState = QUERY_PENDING;
            }
        } catch (SQLiteException e) {
            SqliteWrapper.checkSQLiteException(this, e);
        }
    }

    private void initMessageList() {
        if (mMsgListAdapter != null) {
            return;
        }

        String highlightString = getIntent().getStringExtra("highlight");
        Pattern highlight = highlightString == null
            ? null
        /// M: ALPS00619099, highlight all matched search string, not only on word border @ {
            : Pattern.compile(Pattern.quote(highlightString), Pattern.CASE_INSENSITIVE);
        /// @}

        // Initialize the list adapter with a null cursor.
        mMsgListAdapter = new MessageListAdapter(this, null, mMsgListView, true, highlight);

        mMsgListAdapter.setMsgListItemHandler(mMessageListItemHandler);
        mMsgListView.setAdapter(mMsgListAdapter);
        mMsgListView.setSelector(android.R.color.transparent);
        mMsgListView.setItemsCanFocus(false);
        mMsgListView.setVisibility(View.VISIBLE);
        mMsgListView.setOnCreateContextMenuListener(mMsgListMenuCreateListener);
        mMsgListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                MmsLog.d(MessageListItem.TAG_DIVIDER, "OnItemClickListener(): view = " + view);
                if (ComposeMessageActivity.this.isFinishing() || ComposeMessageActivity.this.isDestroyed()) {
                    Log.d(TAG, "Activity finishing or destoryed");
                    return;
                }
                if (view != null) {
                    ((MessageListItem) view).onMessageListItemClick();
                }
            }
        });
        /// M: Code analyze 050, Add scroll listener and touch listener for MessageListView.@{
        /**
         * M: Adjust the velocity scale and friction of messages list view,
         * and The below two values are just experience
         * values. For density > 2 which usually on FHD devices with larger(1.5x)
         * friction, set Velocity scale to 0.6 For
         * density <= 2, still use Velocity scale 0.4
         */
        final float density = getResources().getDisplayMetrics().density;
        final float DEFAULT_DENSITY = 2.0f;
        final float FHD_VELOCITY_SCALE = 0.6f;
        final float DEFAULT_VELOCITY_SCALE = 0.4f;
        final float DEFAULT_FRICTION = 0.005f;

        mMsgListView.setVelocityScale(density > DEFAULT_DENSITY
                ? FHD_VELOCITY_SCALE : DEFAULT_VELOCITY_SCALE);
        mMsgListView.setFriction(DEFAULT_FRICTION);
        mMsgListView.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                hideInputMethod();
                return false;
            }
        });
        // add for ipmessage
        mIpCompose.onIpInitMessageList(mMsgListView, mMsgListAdapter.mIpMessageListAdapter);
    }

    private void loadDraft() {
        if (mWorkingMessage.isWorthSaving()) {
            Log.w(TAG, "loadDraft() called with non-empty working message");
            mWorkingMessage.checkDraftSmsMessage(mConversation);
            return;
        }

        // add for ipmessage
        if (mIpCompose.loadIpMessagDraft(mConversation.getThreadId())) {
            return;
        }

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            log("loadDraft() call WorkingMessage.loadDraft");
        }

        /// M:[Just Comment] 4.1 Code.  There may be a bug.
        mWorkingMessage = WorkingMessage.loadDraft(this, mConversation,
                new Runnable() {
                    @Override
                    public void run() {
                        drawTopPanel(false);
                        drawBottomPanel();
                        updateSendButtonState();
                    }
                });
//        if (mConversation != null && mConversation.getRecipients() != null
//                && mConversation.getRecipients().size() > 20) {
//            MmsLog.d(TAG, "register mDraftChangeObserver");
//            mContentResolver.registerContentObserver(
//                    Mms.CONTENT_URI, true, mDraftChangeObserver);
//        }
    }

    private void saveDraft(boolean isStopping) {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            LogTag.debug("saveDraft");
        }

        // add for ipmessage
        mIpCompose.onIpSaveDraft(mConversation.getThreadId());
        /// @}
        // TODO: Do something better here.  Maybe make discard() legal
        // to call twice and make isEmpty() return true if discarded
        // so it is caught in the clause above this one?
        if (mWorkingMessage.isDiscarded()) {
            return;
        }

        if (!mWaitingForSubActivity && !mWorkingMessage.isWorthSaving() &&
                (!isRecipientsEditorVisible() || recipientCount() == 0
                /* M: ALPS01821584. discard when onStop */ || isStopping)) {
            if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                log("not worth saving, discard WorkingMessage and bail");
            }
            mWorkingMessage.discard(false);
            return;
        }
        /// M : Fix CR: ALPS00996195
        /// group message, more than one recipients, no need to saveDraft,
        /// is going to MultiDeleteActivity, just return. if contains draft, discard it @{
        if (mIsStartMultiDeleteActivity) {
            if (MmsPreferenceActivity.getIsGroupMmsEnabled(this) && !needSaveDraft()
                && mConversation.getRecipients().size() > 1) {
                Log.d("[Mms][Draft][ComposeMessageActivity]", "[saveDraft] group message & "
                        + " no need save drat & mIsStartMultiDeleteActivity"
                        + " true, then just return!");
                if (mConversation.hasDraft()) {
                    Log.d("[Mms][Draft][ComposeMessageActivity]",
                            "[saveDraft] mConversation has draft, then delete it");
                    mWorkingMessage.discard(false);
                }
                return;
            }
            /// M Fix CR ALPS01201355, which will save draft the contact contains email address.
            /// when start multideleteactivity. if contains mms draft ,discard it @{
            if (getRecipients().containsEmail() && !needSaveDraft()) {
                Log.d("[Mms][Draft][ComposeMessageActivity]", "[saveDraft] contains email address"
                    + ",no need to save draft, just return!!");
                if (mConversation.hasDraft()) {
                    Log.d("[Mms][Draft][ComposeMessageActivity]",
                            "[saveDraft] mConversation has draft, then delete it");
                    mWorkingMessage.discard(false);
                }
                return;
            }
            /// @}
        }
        /// @}

        Log.d("[Mms][Draft][ComposeMessageActivity]", "[saveDraft] call workingmessage.saveDraft");
        mWorkingMessage.saveDraft(isStopping);

        if (mToastForDraftSave && mIsSmsEnabled) {
            Toast.makeText(this, R.string.message_saved_as_draft,
                    Toast.LENGTH_SHORT).show();
        }
    }

    private boolean isPreparedForSending() {
        /* SIM ON OFF*/
        int subCount = getEnabledSubCount();

        if (isRecipientsEditorVisible()) {
            String recipientText = mRecipientsEditor.getText() == null ?
                    "" : mRecipientsEditor.getText().toString();
            /* SIM ON OFF*/
            return subCount > 0 && !TextUtils.isEmpty(recipientText) && mIsSmsEnabled
                    && (mWorkingMessage.hasAttachment() || mWorkingMessage.hasText()
                            || mWorkingMessage.hasSubject());
        } else {
            /* SIM ON OFF*/
            return subCount > 0 && mIsSmsEnabled && (mWorkingMessage.hasAttachment()
                    || mWorkingMessage.hasText()
                            || mWorkingMessage.hasSubject());
        }
        /// @}
    }

    private int recipientCount() {
        int recipientCount;

        // To avoid creating a bunch of invalid Contacts when the recipients
        // editor is in flux, we keep the recipients list empty.  So if the
        // recipients editor is showing, see if there is anything in it rather
        // than consulting the empty recipient list.
        if (isRecipientsEditorVisible()) {
            recipientCount = mRecipientsEditor.getRecipientCount();
        } else {
            recipientCount = getRecipients().size();
        }
        return recipientCount;
    }

    public void sendMessage(boolean bCheckEcmMode) {
        if (mWorkingMessage.requiresMms() && (mWorkingMessage.hasSlideshow()
                || mWorkingMessage.hasAttachment())) {
            int messageSize = 0;
            messageSize = mWorkingMessage.getCurrentMessageSize();
            MmsLog.d(TAG, "Compose.sendMessage(): messageSize=" + messageSize);
            if (messageSize > MmsConfig.getUserSetMmsSizeLimit(true)) {
                MessageUtils.showErrorDialog(ComposeMessageActivity.this,
                        R.string.exceed_message_size_limitation,
                        R.string.exceed_message_size_limitation, 0, 0);
                updateSendButtonState();
                return;
            }
        }

        if (bCheckEcmMode) {
            // TODO: expose this in telephony layer for SDK build
            int phoneId = SubscriptionManager.getPhoneId(mSelectedSubId);
            /* P0 migration*/
            String inEcm = TelephonyManager.getTelephonyProperty(phoneId,
                    MtkTelephonyProperties.PROPERTY_INECM_MODE_BY_SLOT, null);
            //String inEcm = SystemProperties.get(TelephonyProperties.PROPERTY_INECM_MODE);

            boolean ecc_ims = false;
                        IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                                    ServiceManager.getService("phoneEx"));
            if (iTelEx != null) {
                try {
                    ecc_ims = iTelEx.getIsLastEccIms();
                } catch (RemoteException e) {
                    Log.e(TAG, "RemoteException getIsLastEccIms()");
                }
            }
 
            if (ecc_ims) {
                MmsLog.d(TAG, "sendMessage ecc_ims->1");
                //send message
            } else {
                //follow normal routine
                if (Boolean.parseBoolean(inEcm)) {
                    try {
                        startActivityForResult(
                                new Intent(TelephonyIntents.ACTION_SHOW_NOTICE_ECM_BLOCK_OTHERS,
                                            null), REQUEST_CODE_ECM_EXIT_DIALOG);
                        return;
                    } catch (ActivityNotFoundException e) {
                        // continue to send message
                        Log.e(TAG, "Cannot find EmergencyCallbackModeExitDialog", e);
                    }
                }
            }
        }

        /// M: Fix bug ALPS00407718
        if (mExitOnSent) {
            hideKeyboard();
        }
        /// M: Code analyze 011, use another method for performance.(update mDebugRecipients)@{
        ContactList contactList = isRecipientsEditorVisible() ?
                mRecipientsEditor.constructContactsFromInput(false) : getRecipients();
        mDebugRecipients = contactList.serialize();
        /// @}

        if (!mSendingMessage) {
            if (LogTag.SEVERE_WARNING) {
                String sendingRecipients = mConversation.getRecipients().serialize();
                if (!sendingRecipients.equals(mDebugRecipients)) {
                    String workingRecipients = mWorkingMessage.getWorkingRecipients();
                    if (!mDebugRecipients.equals(workingRecipients)) {
                    }
                }
                sanityCheckConversation();
            }

            // send can change the recipients. Make sure we remove the listeners first and then add
            // them back once the recipient list has settled.
            removeRecipientsListeners();

            /// M: If msg can be sent, AttachmentEditor can not be reponsed.
            mClickCanResponse = false;
            /// M:the method is extend to support gemini @{
            mWorkingMessage.send(mDebugRecipients, mSelectedSubId);
            MmsLog.d(TAG, "Compose.sendMessage(): after sendMessage. mConversation.ThreadId="
                    + mConversation.getThreadId()
                    + ", MessageCount=" + mConversation.getMessageCount());
            /// @}
            /** M:
             *   If message count is 0, it should be a new message.
             *   After tap send button, the sent message will have draft flag for a short time.
             *   That means, the message count will be 0 for a short time.
             *   If user tap home key in this short time, it will change the
             *   conversation id to 0 in the method savedraft().
             *   When the screen is back to Message Composer, it will
             *   query database with thread(conversation) id 0.
             *   So, it will query for nothing. The screen is always blank.
             *   Fix this issue by force to set message count with 1.
             */
            if (mConversation.getMessageCount() == 0) {
                mConversation.setMessageCount(1);
            }
            /// M: @{
            mWaitingForSendMessage = true;
            /// M: when tap fail icon, don't add recipients
            isInitRecipientsEditor = false;
            mMsgListView.setVisibility(View.VISIBLE);
            /// @}

            mSentMessage = true;
            mSendingMessage = true;
            addRecipientsListeners();

            mScrollOnSend = true;   // in the next onQueryComplete, scroll the list to the end.
            // M: reset mCutRecipients
            mCutRecipients = null;
        }
        // But bail out if we are supposed to exit after the message is sent.
        if (mExitOnSent) {
            /// M: fix bug ALPS00722349, update thread after forward message, avoid conversation
            /// draft state become abnormal.
            boolean isChecked = mConversation.isChecked();
            mConversation = Conversation.upDateThread(getApplicationContext(),
                    mConversation.getThreadId(), false);
            mConversation.setIsChecked(isChecked);
            ///M: add for guarantee the message sent. @{
            mUiHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    finish();
                }
            }, 150);
            /// @}
        }
    }

    private void resetMessage() {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            log("resetMessage");
        }

        /// M: should hide RecipientsEditor before TextEditor requestFocus. To avoid do
        /// afterTextChange after message sent.
        hideRecipientEditor();
        updateTitle(mConversation.getRecipients());

        // Make the attachment editor hide its view.
        mAttachmentEditor.hideView();
//            mAttachmentEditorScrollView.setVisibility(View.GONE);
        ///M: change the order between Editor.requestFocus & showSubjectEditor
        /// for fix issue ALPS00569570 @{
        // Focus to the text editor.
        mTextEditor.requestFocus();

        // Hide the subject editor.
        showSubjectEditor(false);
        /// @}

        // We have to remove the text change listener while the text editor gets cleared and
        // we subsequently turn the message back into SMS. When the listener is listening while
        // doing the clearing, it's fighting to update its counts and itself try and turn
        // the message one way or the other.
        mTextEditor.removeTextChangedListener(mTextEditorWatcher);

        // Clear the text box.
        TextKeyListener.clear(mTextEditor.getText());

        mWorkingMessage.clearConversation(mConversation, false);
        mWorkingMessage = WorkingMessage.createEmpty(this);
        mWorkingMessage.setConversation(mConversation);



        if ((mMessageSubId == MmsConfig.DEFAULT_SIM_SETTING_ALWAYS_ASK
                || mMessageSubId == MmsConfig.DEFAULT_SIM_NOT_SET)) {
            mSelectedSubId = 0;
        }

        drawBottomPanel();

        // "Or not", in this case.
        updateSendButtonState();
        // add for ipmessage
        mIpCompose.onIpResetMessage();
        // Our changes are done. Let the listener respond to text changes once again.
        mTextEditor.removeTextChangedListener(mTextEditorWatcher);
        mTextEditor.addTextChangedListener(mTextEditorWatcher);

        // Close the soft on-screen keyboard if we're in landscape mode so the user can see the
        // conversation.
        ///M: but when the device was tablet, we can't hide the IME
        boolean isTablet = getResources().getBoolean(R.bool.isTablet);
        if (mIsLandscape && !isTablet) {
            hideKeyboard();
        }

        mLastRecipientCount = 0;
        mSendingMessage = false;
        invalidateOptionsMenu();
        /// M: update list, this must put after hideRecipientEditor(); to avoid a bug.
        startMsgListQuery(MESSAGE_LIST_QUERY_TOKEN, 0);
        /// M: reset flags
        mClickCanResponse = true;
   }

    private void hideKeyboard() {
        InputMethodManager inputMethodManager =
            (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        inputMethodManager.hideSoftInputFromWindow(mTextEditor.getWindowToken(), 0);
    }

    public void updateSendButtonState() {
        boolean enable = false;
        /* SIM ON OFF*/
        int subCount = getEnabledSubCount();
        if (isPreparedForSending()) {
            /* SIM ON OFF*/
            if (subCount > 0) {
                // When the type of attachment is slideshow, we should
                // also hide the 'Send' button since the slideshow view
                // already has a 'Send' button embedded.
                if (!mWorkingMessage.hasSlideshow()) {
                    enable = true;
                } else {
                    mAttachmentEditor.setCanSend(true);
                }
            }
        } else {
            // / @}
            mAttachmentEditor.setCanSend(false);
        }

        boolean requiresMms = mWorkingMessage.requiresMms();
        /* SIM ON OFF*/
        if (mOpComposeExt.updateSendButton(enable, subCount, requiresMms,
                                           mWorkingMessage.hasSlideshow())) {
            return;
        }

        View sendButton = showSmsOrMmsSendButton(requiresMms);
        if (!requiresMms && (recipientCount() > MmsConfig.getRecipientLimit())) {
            enable = false;
        }
        if (!mIsSmsEnabled) {
            enable = false;
        }
        if (MessageUtils.isUseSubSimulator()) {
            enable = true;
        }
        if (sendButton != null) {
            sendButton.setEnabled(enable);
            sendButton.setFocusable(enable);
        }
    }

    private long getMessageDate(Uri uri) {
        if (uri != null) {
            Cursor cursor = SqliteWrapper.query(this, mContentResolver,
                    uri, new String[] { Mms.DATE }, null, null, null);
            if (cursor != null) {
                try {
                    if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                        return cursor.getLong(0) * 1000L;
                    }
                } finally {
                    cursor.close();
                }
            }
        }
        return NO_DATE_FOR_DIALOG;
    }

/// M: @{
//    private void initActivityState(Intent intent) {
//        // If we have been passed a thread_id, use that to find our conversation.
//        long threadId = intent.getLongExtra("thread_id", 0);
//        if (threadId > 0) {
//            if (LogTag.VERBOSE) log("get mConversation by threadId " + threadId);
//            mConversation = Conversation.get(this, threadId, false);
//        } else {
//            Uri intentData = intent.getData();
//            if (intentData != null) {
//                // try to get a conversation based on the data URI passed to our intent.
//                if (LogTag.VERBOSE) log("get mConversation by intentData " + intentData);
//                mConversation = Conversation.get(this, intentData, false);
//                mWorkingMessage.setText(getBody(intentData));
//            } else {
//                // special intent extra parameter to specify the address
//                String address = intent.getStringExtra("address");
//                if (!TextUtils.isEmpty(address)) {
//                    if (LogTag.VERBOSE) log("get mConversation by address " + address);
//                    mConversation = Conversation.get(this, ContactList.getByNumbers(address,
//                            false /* don't block */, true /* replace number */), false);
//                } else {
//                    if (LogTag.VERBOSE) log("create new conversation");
//                    mConversation = Conversation.createNew(this);
//                }
//            }
//        }
//        addRecipientsListeners();
//
//        mExitOnSent = intent.getBooleanExtra("exit_on_sent", false);
//        if (intent.hasExtra("sms_body")) {
//            mWorkingMessage.setText(intent.getStringExtra("sms_body"));
//        }
//        mWorkingMessage.setSubject(intent.getStringExtra("subject"), false);
//    }
    private void initActivityState(Bundle bundle) {
        Intent intent = getIntent();
        /// M: Code analyze 033, Save some useful information in order to restore the draft when
        /// activity restarting.@{
        mIsTooManyRecipients = false;
        if (bundle != null) {
            mCompressingImage = bundle.getBoolean("compressing_image", false);
            String recipientsStr = bundle.getString("recipients");
            int recipientCount = 0;
            if (recipientsStr != null) {
                recipientCount = recipientsStr.split(";").length;
                mConversation = Conversation.get(getApplicationContext(),
                    ContactList.getByNumbers(recipientsStr,
                            false /* don't block */, true /* replace number */),
                            false, Conversation.TYPE_MMSSMS);
            } else {
                Long threadId = bundle.getLong("thread", 0);
                mConversation = Conversation.get(getApplicationContext(), threadId, false);
            }
            // M: fix bug ALPS00352078
            mWorkingMessage.setConversation(mConversation);

            mExitOnSent = bundle.getBoolean(KEY_EXIT_ON_SENT, false);
            mForwardMessageMode = bundle.getBoolean(KEY_FORWARDED_MESSAGE, false);

            mWorkingMessage.readStateFromBundle(bundle);

            if (!mCompressingImage && mConversation.hasDraft()
                    && mConversation.getMessageCount() == 0) {
                if (!mWorkingMessage.requiresMms()) {
                    Log.w(TAG, "delete sms draft");
                    mWorkingMessage.asyncDeleteDraftSmsMessage(mConversation);
                } else {
                    if (mWorkingMessage.getSlideshow() != null
                            && mWorkingMessage.getSlideshow().size() == 1
                            && !mWorkingMessage.getSlideshow().get(0).hasAudio()
                            && !mWorkingMessage.getSlideshow().get(0).hasImage()
                            && !mWorkingMessage.getSlideshow().get(0).hasVideo()
                            && mWorkingMessage.getSlideshow().sizeOfFilesAttach() == 0) {
                        mWorkingMessage.asyncDeleteDraftMmsMessage(mConversation);
                        mWorkingMessage.removeAllFileAttaches();
                        mWorkingMessage.removeAttachment(false);
                    }
                }
                mWorkingMessage.clearConversation(mConversation, true);
            }
            if (recipientCount > RECIPIENTS_LIMIT_FOR_SMS) {
                mIsTooManyRecipients = true;
            }
            mCompressingImage = false;

            mAppendAttachmentSign = bundle.getBoolean(KEY_APPEND_MESSAGE, true);
            MmsLog.d(TAG, "initActivityState mAppendAttachmentSign : " + mAppendAttachmentSign);
            /// @}
            return;
        }
        /// M: Code analyze 019, Add vcard attachment.  @{
        long[] contactsIds = VCardUtils.getContactsIds(
                intent.getStringExtra("multi_export_contacts"));
        /// @}
        // If we have been passed a thread_id, use that to find our conversation.
        long threadId = intent.getLongExtra("thread_id", 0);
        if (threadId > 0) {
            if (LogTag.VERBOSE) log("get mConversation by threadId " + threadId);
            mConversation = Conversation.get(getApplicationContext(), threadId, false);
        /// M: Code analyze 019, Add vcard attachment.  @{
        } else if (contactsIds != null && contactsIds.length > 0) {
            //addTextVCard(contactsIds);
            addTextVCardAsync(contactsIds);
            mConversation = Conversation.createNew(getApplicationContext());
            return;
        /// @}
        } else {
            Uri intentData = intent.getData();
            /// M: Code analyze 034, If intent is SEND,just create a new empty thread,
            /// otherwise Conversation.get() will throw exception.
            String action = intent.getAction();
            if (intentData != null && (TextUtils.isEmpty(action) ||
                            !action.equals(Intent.ACTION_SEND))) {
                /// M: group-contact send message
                // try to get a conversation based on the data URI passed to our intent.
                if (intentData.getPathSegments().size() < 2) {
                    mConversation = mConversation.get(getApplicationContext(),
                            ContactList.getByNumbers(
                           getStringForMultipleRecipients(Conversation.getRecipients(intentData)),
                                 false /* don't block */, true /* replace number */),
                                 false, Conversation.TYPE_MMSSMS);
                } else {
                    mConversation = Conversation.get(getApplicationContext(), intentData,
                            false, Conversation.TYPE_MMSSMS);
                }
                /// @}
                mWorkingMessage.setText(getBody(intentData));
            } else {
                // special intent extra parameter to specify the address
                String address = intent.getStringExtra("address");
                if (!TextUtils.isEmpty(address)) {
                    if (LogTag.VERBOSE) log("get mConversation by address ");
                    mConversation = Conversation.get(getApplicationContext(),
                            ContactList.getByNumbers(address,
                            false /* don't block */, true /* replace number */),
                            false, Conversation.TYPE_MMSSMS);
                } else {
                    if (LogTag.VERBOSE) log("create new conversation");
                    mConversation = Conversation.createNew(getApplicationContext());
                }
            }
        }
        //addRecipientsListeners();
        updateThreadIdIfRunning();

        mExitOnSent = intent.getBooleanExtra(KEY_EXIT_ON_SENT, false);
        mForwardMessageMode = intent.getBooleanExtra(KEY_FORWARDED_MESSAGE, false);
        if (intent.hasExtra("sms_body")) {
            /// M: Code analyze 017, Handle forwarded message.(see:forwardMessage()).
            /// Forward sms message and set sms body.@{
            String sms_body = intent.getStringExtra("sms_body");
            /// M: Modify for ALPS00759207
            mWorkingMessage.setText(sms_body);
            /// @}
        }
        mWorkingMessage.setSubject(intent.getStringExtra("subject"), false);
    }

    private void initFocus() {
        if (!mIsKeyboardOpen) {
            return;
        }

        // If the recipients editor is visible, there is nothing in it,
        // and the text editor is not already focused, focus the
        // recipients editor.
        if (isRecipientsEditorVisible()
                && TextUtils.isEmpty(mRecipientsEditor.getText())
                && !mTextEditor.isFocused()) {
            mRecipientsEditor.requestFocus();
            return;
        }

        // If we decided not to focus the recipients editor, focus the text editor.
        if (mSubjectTextEditor == null
                || (mSubjectTextEditor != null && !mSubjectTextEditor.isFocused())) {
            if ((mBottomPanel != null) && (mBottomPanel.getVisibility() == View.VISIBLE)) {
                mTextEditor.requestFocus();
            }
        }
    }

    private final MessageListAdapter.OnDataSetChangedListener
                    mDataSetChangedListener = new MessageListAdapter.OnDataSetChangedListener() {
        @Override
        public void onDataSetChanged(MessageListAdapter adapter) {
        }

        @Override
        public void onContentChanged(MessageListAdapter adapter) {
        /// M: @{
            if (mMsgListAdapter != null &&
                mMsgListAdapter.getOnDataSetChangedListener() != null) {
                //MmsLog.d(TAG, "OnDataSetChangedListener is not cleared");
                startMsgListQuery();
            } else {
                MmsLog.d(TAG, "OnDataSetChangedListener is cleared");
            }
        /// @}
        }
    };

    /**
     * smoothScrollToEnd will scroll the message list to the bottom if the list is already near
     * the bottom. Typically this is called to smooth scroll a newly received message into view.
     * It's also called when sending to scroll the list to the bottom, regardless of where it is,
     * so the user can see the just sent message. This function is also called when the message
     * list view changes size because the keyboard state changed or the compose message field grew.
     *
     * @param force always scroll to the bottom regardless of current list position
     * @param listSizeChange the amount the message list view size has vertically changed
     */
    private void smoothScrollToEnd(boolean force, int listSizeChange) {
        int lastItemVisible = mMsgListView.getLastVisiblePosition();
        int lastItemInList = mMsgListAdapter.getCount() - 1;
        if (lastItemVisible < 0 || lastItemInList < 0) {
            if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                Log.v(TAG, "smoothScrollToEnd: lastItemVisible=" + lastItemVisible +
                        ", lastItemInList=" + lastItemInList +
                        ", mMsgListView not ready");
            }
            return;
        }

        View lastChildVisible =
                mMsgListView.getChildAt(lastItemVisible - mMsgListView.getFirstVisiblePosition());
        int lastVisibleItemBottom = 0;
        int lastVisibleItemHeight = 0;
        if (lastChildVisible != null) {
            lastVisibleItemBottom = lastChildVisible.getBottom();
            lastVisibleItemHeight = lastChildVisible.getHeight();
        }

        if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            Log.v(TAG, "smoothScrollToEnd newPosition: " + lastItemInList +
                    " mLastSmoothScrollPosition: " + mLastSmoothScrollPosition +
                    " first: " + mMsgListView.getFirstVisiblePosition() +
                    " lastItemVisible: " + lastItemVisible +
                    " lastVisibleItemBottom: " + lastVisibleItemBottom +
                    " lastVisibleItemBottom + listSizeChange: " +
                    (lastVisibleItemBottom + listSizeChange) +
                    " mMsgListView.getHeight() - mMsgListView.getPaddingBottom(): " +
                    (mMsgListView.getHeight() - mMsgListView.getPaddingBottom()) +
                    " listSizeChange: " + listSizeChange);
        }
        // Only scroll if the list if we're responding to a newly sent message (force == true) or
        // the list is already scrolled to the end. This code also has to handle the case where
        // the listview has changed size (from the keyboard coming up or down or the message entry
        // field growing/shrinking) and it uses that grow/shrink factor in listSizeChange to
        // compute whether the list was at the end before the resize took place.
        // For example, when the keyboard comes up, listSizeChange will be negative, something
        // like -524. The lastChild listitem's bottom value will be the old value before the
        // keyboard became visible but the size of the list will have changed. The test below
        // add listSizeChange to bottom to figure out if the old position was already scrolled
        // to the bottom. We also scroll the list if the last item is taller than the size of the
        // list. This happens when the keyboard is up and the last item is an mms with an
        // attachment thumbnail, such as picture. In this situation, we want to scroll the list so
        // the bottom of the thumbnail is visible and the top of the item is scroll off the screen.
        int listHeight = mMsgListView.getHeight();
        boolean lastItemTooTall = lastVisibleItemHeight > listHeight;
        boolean willScroll = force ||
                ((listSizeChange != 0 || lastItemInList != mLastSmoothScrollPosition) &&
                lastVisibleItemBottom + listSizeChange <=
                    listHeight - mMsgListView.getPaddingBottom());
        if (willScroll || (lastItemTooTall && lastItemInList == lastItemVisible)) {
            if (Math.abs(listSizeChange) > SMOOTH_SCROLL_THRESHOLD) {
                // When the keyboard comes up, the window manager initiates a cross fade
                // animation that conflicts with smooth scroll. Handle that case by jumping the
                // list directly to the end.
                if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                    Log.v(TAG, "keyboard state changed. setSelection=" + lastItemInList);
                }
                if (lastItemTooTall) {
                    // If the height of the last item is taller than the whole height of the list,
                    // we need to scroll that item so that its top is negative or above the top of
                    // the list. That way, the bottom of the last item will be exposed above the
                    // keyboard.
                    mMsgListView.setSelectionFromTop(lastItemInList,
                            listHeight - lastVisibleItemHeight);
                } else {
                    mMsgListView.setSelection(lastItemInList);
                }
            } else if (lastItemInList - lastItemVisible > MAX_ITEMS_TO_INVOKE_SCROLL_SHORTCUT) {
                if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                    Log.v(TAG, "too many to scroll, setSelection=" + lastItemInList);
                }
                mMsgListView.setSelection(lastItemInList);
            } else {
                if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                    Log.v(TAG, "smooth scroll to " + lastItemInList);
                }
                if (lastItemTooTall) {
                    // If the height of the last item is taller than the whole height of the list,
                    // we need to scroll that item so that its top is negative or above the top of
                    // the list. That way, the bottom of the last item will be exposed above the
                    // keyboard. We should use smoothScrollToPositionFromTop here, but it doesn't
                    // seem to work -- the list ends up scrolling to a random position.
                    mMsgListView.setSelectionFromTop(lastItemInList,
                            listHeight - lastVisibleItemHeight);
                } else {
                    mMsgListView.smoothScrollToPosition(lastItemInList);
                }
                mLastSmoothScrollPosition = lastItemInList;
            }
        }
    }

    private final class BackgroundQueryHandler extends ConversationQueryHandler {
        public BackgroundQueryHandler(ContentResolver contentResolver) {
            super(contentResolver);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            /// M: @{
            MmsLog.d(TAG, "onQueryComplete, token=" + token + "activity="
                    + ComposeMessageActivity.this);
            /// @}
            switch(token) {
                case MESSAGE_LIST_QUERY_TOKEN:
                    if (mQueryingState == QUERY_RUNNING) {
                        mQueryingState = QUERY_IDLE;
                    } else if (mQueryingState == QUERY_PENDING) {
                        if (cursor != null) {
                            cursor.close();
                        }
                        MmsLog.w(TAG, "onQueryComplete, has pending query, start a new query");
                        mQueryingState = QUERY_IDLE;
                        startMsgListQuery();
                        return;
                    }
                    /// @}
                    if (cursor == null) {
                        MmsLog.w(TAG, "onQueryComplete, cursor is null.");
                        return;
                    }
                    /// M: If adapter or listener has been cleared, just close this cursor@{
                    if (mMsgListAdapter == null) {
                        MmsLog.w(TAG, "onQueryComplete, mMsgListAdapter is null.");
                        cursor.close();
                        return;
                    }
                    if (mMsgListAdapter.getOnDataSetChangedListener() == null) {
                        MmsLog.d(TAG, "OnDataSetChangedListener is cleared");
                        cursor.close();
                        return;
                    }
                    /// @}
                    if (isRecipientsEditorVisible()) {
                        MmsLog.d(TAG, "RecipientEditor visible, it means no messagelistItem!");
                        cursor.close();
                        return;
                    }
                    // check consistency between the query result and 'mConversation'
                    long tid = (Long) cookie;

                    if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                        log("##### onQueryComplete: msg history result for threadId " + tid);
                    }
                    if (tid != mConversation.getThreadId()) {
                        log("onQueryComplete: msg history query result is for threadId " +
                                tid + ", but mConversation has threadId " +
                                mConversation.getThreadId() + " starting a new query");
                        if (cursor != null) {
                            cursor.close();
                        }
                        startMsgListQuery();
                        return;
                    }

                    // check consistency b/t mConversation & mWorkingMessage.mConversation
                    ComposeMessageActivity.this.sanityCheckConversation();

                    int newSelectionPos = -1;
                    long targetMsgId = getIntent().getLongExtra("select_id", -1);
                    String targetMsgType = getIntent().getStringExtra("msg_type");
                    if (targetMsgId != -1) {
                      if (cursor != null) {
                        cursor.moveToPosition(-1);
                        while (cursor.moveToNext()) {
                            long msgId = cursor.getLong(COLUMN_ID);
                            String messageType = cursor.getString(COLUMN_MSG_TYPE);
                            if (msgId == targetMsgId && messageType.equals(targetMsgType)) {
                                newSelectionPos = cursor.getPosition();
                                getIntent().putExtra("select_id", -1);
                                break;
                            }
                          }
                        }
                    } else if (mSavedScrollPosition != -1) {
                        // mSavedScrollPosition is set when this activity pauses. If equals maxint,
                        // it means the message list was scrolled to the end. Meanwhile, messages
                        // could have been received. When the activity resumes and we were
                        // previously scrolled to the end, jump the list so any new messages are
                        // visible.
                        if (mSavedScrollPosition == Integer.MAX_VALUE) {
                            int cnt = mMsgListAdapter.getCount();
                            if (cnt > 0) {
                                // Have to wait until the adapter is loaded before jumping to
                                // the end.
                                newSelectionPos = cnt - 1;
                                mSavedScrollPosition = -1;
                            }
                        } else {
                            // remember the saved scroll position before the activity is paused.
                            // reset it after the message list query is done
                            newSelectionPos = mSavedScrollPosition;
                            mSavedScrollPosition = -1;
                        }
                    }

                    int cursorCount = cursor.getCount();
                    if (queryOffset == 0) {
                        queryCursors.clear();
                    }
                    queryCursors.add(cursor);
                    queryOffset += cursor.getCount();
                    if (cursorCount >= MAX_MESSAGE_PER_QUERY) {
                        // query once more
                        startMsgListQuery();
                        return;
                    } else {
                        int length = queryCursors.size();
                        Cursor[] cursors = new Cursor[length];
                        for (int i = 0; i < length; i++){
                            cursors[i] = queryCursors.get(i);
                        }
                        Cursor merCur = new MergeCursor(cursors);
                        mMsgListAdapter.changeCursor(merCur);
                        queryCursors.clear();
                        queryOffset = 0;
                    }

                    if(bListUpdated == false) {
                       bListUpdated = true;
                    }
                    if (newSelectionPos != -1) {
                        /// M: remove bug ALPS00404266 patch, keep item top @{
                        mMsgListView.setSelection(newSelectionPos);     // jump the list to the pos
                        //View child = mMsgListView.getChildAt(newSelectionPos);
                        //int top = 0;
                        //if (child != null) {
                        //    top = child.getTop();
                        //}
                       // mMsgListView.setSelectionFromTop(newSelectionPos, top);
                        /// @}
                    } else {
                        /// M: google jb.mr1 patch, Conversation should scroll to the bottom
                        /// when incoming received @{
                        int count = mMsgListAdapter.getCount();
                        Cursor cur = mMsgListAdapter.getCursor();
                        long lastMsgId = 0;
                        if (cur != null && count > 0) {
                            /// ALPS03874406, cursor window is full and the last row data
                            /// was not loaded into cursor, exception happens when cursor.getLong()
                            try {
                                cur.moveToLast();
                                lastMsgId = cur.getLong(COLUMN_ID);
                            } catch (IllegalStateException e) {
                                MmsLog.e(TAG, "get cursor last row data error, maybe" +
                                        " CursorWindow is full, move cursor to first");
                                cur.moveToFirst();
                                lastMsgId = cur.getLong(COLUMN_ID);
                            }
                        }
                        // mScrollOnSend is set when we send a message. We always want to scroll
                        // the message list to the end when we send a message, but have to wait
                        // until the DB has changed. We also want to scroll the list when a
                        // new message has arrived.
                        smoothScrollToEnd(mScrollOnSend || lastMsgId != mLastMessageId, 0);
                        mLastMessageId = lastMsgId;
                        /// @}
                        mScrollOnSend = false;
                    }
                    // Adjust the conversation's message count to match reality. The
                    // conversation's message count is eventually used in
                    // WorkingMessage.clearConversation to determine whether to delete
                    // the conversation or not.
                    if (mMsgListAdapter.getCount() == 0 && mWaitingForSendMessage) {
                        mConversation.setMessageCount(1);
                    } else {
                        mConversation.setMessageCount(mMsgListAdapter.getCount());
                    }
                    updateThreadIdIfRunning();

                    MmsLog.d(TAG, "onQueryComplete(): Conversation.ThreadId="
                            + mConversation.getThreadId()
                            + ", MessageCount=" + mConversation.getMessageCount());

                    // Once we have completed the query for the message history, if
                    // there is nothing in the cursor and we are not composing a new
                    // message, we must be editing a draft in a new conversation (unless
                    // mSentMessage is true).
                    // Show the recipients editor to give the user a chance to add
                    // more people before the conversation begins.
                    if (mMsgListAdapter.getCursor() != null
                            && mMsgListAdapter.getCursor().getCount() == 0
                            && !isRecipientsEditorVisible() && !mSentMessage) {
                        /// M: fix bug ALPS01098902, avoding checkObsoleteThreadId in this case
                        if (mSubSelectDialog != null && mSubSelectDialog.isShowing()
                                && mOldThreadID > 0 && mCutRecipients != null) {
                            mIsSameConv = false;
                        }
                        initRecipientsEditor(null);
                    }

                    // FIXME: freshing layout changes the focused view to an unexpected
                    // one, set it back to TextEditor forcely.
                    if (mSubjectTextEditor == null || (mSubjectTextEditor != null
                            && !mSubjectTextEditor.isFocused())) {
                        mTextEditor.requestFocus();
                    }

                    invalidateOptionsMenu();    // some menu items depend on the adapter's count
                    if (!mIsActivityStoped) {
                        mConversation.blockMarkAsRead(false);
                        mConversation.markAsRead();
                    }
                    return;

                case MESSAGE_LIST_QUERY_AFTER_DELETE_TOKEN:
                    // check consistency between the query result and 'mConversation'
                    tid = (Long) cookie;

                    if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                        log("## onQueryComplete (after delete): msg history result for threadId "
                                + tid);
                    }
                    if (cursor == null) {
                        return;
                    }
                    if (tid > 0 && cursor.getCount() == 0) {
                        // We just deleted the last message and the thread will get deleted
                        // by a trigger in the database. Clear the threadId so next time we
                        // need the threadId a new thread will get created.
                        log("##### MESSAGE_LIST_QUERY_AFTER_DELETE_TOKEN clearing thread id: "
                                + tid);
                        Conversation conv = Conversation.get(getApplicationContext(), tid,
                                false);
                        if (conv != null) {
                            conv.clearThreadId();
                            conv.setDraftState(false);
                        }
                    }
                    cursor.close();
                    break;
                default:
                    MmsLog.w(TAG, "unknown token.");
                    break;
            }
        }

        @Override
        protected void onDeleteComplete(int token, Object cookie, int result) {
            super.onDeleteComplete(token, cookie, result);
            /// M: fix bug ALPS00351620; for requery searchactivity.
            MmsLog.d(TAG, "onDeleteComplete() token="+token);
            SearchActivity.setNeedRequery();
            switch(token) {
                case ConversationList.DELETE_CONVERSATION_TOKEN:
                    /// M: @{
                    /*
                    mConversation.setMessageCount(0);
                    // fall through
                    */
                    try {
                        IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                                    ServiceManager.getService("phoneEx"));
                        if (iTelEx.isTestIccCard(0)) {
                            MmsLog.d(TAG, "All threads has been deleted, send notification..");
                            MtkSmsManager
                                .getSmsManagerForSubscriptionId(
                                    SmsReceiver.sLastIncomingSmsSubId)
                                       .setSmsMemoryStatus(true);
                        }
                    } catch (Exception ex) {
                        MmsLog.e(TAG, " " + ex.getMessage());
                    }
                    // Update the notification for new messages since they
                    // may be deleted.
                    MessagingNotification.nonBlockingUpdateNewMessageIndicator(
                            ComposeMessageActivity.this, MessagingNotification.THREAD_NONE, false);
                    // Update the notification for failed messages since they
                    // may be deleted.
                    updateSendFailedNotification();
                    MessagingNotification.updateDownloadFailedNotification(
                            ComposeMessageActivity.this);
                    break;
                    /// @}
                case DELETE_MESSAGE_TOKEN:
                    /// M: google jb.mr1 patch, Conversation should scroll to the bottom
                    /// when incoming received @{
                    if (cookie instanceof Boolean && ((Boolean) cookie).booleanValue()) {
                        // If we just deleted the last message, reset the saved id.
                        mLastMessageId = 0;
                    }
                    /// @}
                    /// M: Code analyze 027,Add for deleting one message.@{
                    ContactList recipients = getRecipients();
                    mConversation = Conversation.upDateThread(getApplicationContext(),
                            mConversation.getThreadId(), false);
                    mThreadCountManager.isFull(mThreadId, ComposeMessageActivity.this,
                            ThreadCountManager.OP_FLAG_DECREASE);
                    /// @}
                    // Update the notification for new messages since they
                    // may be deleted.
                    MessagingNotification.nonBlockingUpdateNewMessageIndicator(
                            ComposeMessageActivity.this, MessagingNotification.THREAD_NONE, false);
                    // Update the notification for failed messages since they
                    // may be deleted.
                    updateSendFailedNotification();
                    /// M: Code analyze 027,Add for deleting one message.@{
                    MessagingNotification
                        .updateDownloadFailedNotification(ComposeMessageActivity.this);
                    MmsLog.d(TAG, "onDeleteComplete(): MessageCount = "
                            + mConversation.getMessageCount() +
                            ", ThreadId = " + mConversation.getThreadId());
                    if (mIpCompose.onDeleteComplete(token)) {
                        break;
                    }
                    if (mConversation.getMessageCount() <= 0
                            || mConversation.getThreadId() <= 0L) {
                        mMsgListAdapter.changeCursor(null);
                        if (needSaveDraft() && (recipients != null)) {
                            if (!isRecipientsEditorVisible()) {
                                makeDraftEditable(recipients);
                            }
                        } else {
                            /// M: fix bug for ConversationList select all performance,
                            /// update selected threads array.@{
                            ConversationListAdapter.removeSelectedState(mSelectedThreadId);
                            /// @
                            finish();
                        }
                    }
                    /// @}
                    break;
            }
            // If we're deleting the whole conversation, throw away
            // our current working message and bail.
            if (token == ConversationList.DELETE_CONVERSATION_TOKEN) {
                ContactList recipients = mConversation.getRecipients();
                mWorkingMessage.discard();

                // Remove any recipients referenced by this single thread from the
                // contacts cache. It's possible for two or more threads to reference
                // the same contact. That's ok if we remove it. We'll recreate that contact
                // when we init all Conversations below.
                if (recipients != null) {
                    for (Contact contact : recipients) {
                        contact.removeFromCache();
                    }
                }

                // Make sure the conversation cache reflects the threads in the DB.
                Conversation.init(getApplicationContext());
                finish();
            } else if (token == DELETE_MESSAGE_TOKEN) {
                /// M: Code analyze 027,Add for deleting one message.@{
                // Check to see if we just deleted the last message
                startMsgListQuery(MESSAGE_LIST_QUERY_AFTER_DELETE_TOKEN, 0);
                /// @}
            }
        }
    }

    @Override
    public void onUpdate(final Contact updated) {
        /** M:
         * In a bad case ANR will happen. When many contact is update, onUpdate will be
         * invoked very frequently,and the code here will run many times. In mRecipientsEditor,
         * if there are 100[can be more?] recipients,
         * mRecipientsEditor.constructContactsFromInput is time cost.
         * ANR may happen if process many this message consequently.
         * so reduce the frequence, and touch event message have more changces to process.
         */
        if (isRecipientsEditorVisible()) {
            return;
        }
        //log("[CMA] onUpdate contact updated: " + updated);
        if (mPrevRunnable != null) {
            mMessageListItemHandler.removeCallbacks(mPrevRunnable);
        }
        mPrevRunnable = new Runnable() {
            public void run() {
                ContactList recipients = getRecipients();
                if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                    //MmsLog.vpi(TAG, "[CMA] onUpdate contact updated: " + updated);
                    //MmsLog.vpi(TAG, "[CMA] onUpdate recipients: " + recipients);
                }
                updateTitle(recipients);
                /// M: Contact is modified, update the menu icon of contact.
                invalidateOptionsMenu();

                // The contact information for one (or more) of the recipients has changed.
                // Rebuild the message list so each MessageItem will get the last contact info.
                ComposeMessageActivity.this.mMsgListAdapter.notifyDataSetChanged();
            }
        };
        /// M: Using an existing handler for the post, rather than conjuring up a new one.
        mMessageListItemHandler.postDelayed(mPrevRunnable, UPDATE_DELAY);
    }

    private void addRecipientsListeners() {
        Contact.addListener(this);
    }

    private void removeRecipientsListeners() {
        Contact.removeListener(this);
    }

    public static Intent createIntent(Context context, long threadId) {
        Intent intent = new Intent(context, ComposeMessageActivity.class);

        if (threadId > 0) {
            intent.setData(Conversation.getUri(threadId));
        }

        return intent;
    }

    private String getBody(Uri uri) {
        if (uri == null) {
            return null;
        }
        String urlStr = uri.getSchemeSpecificPart();
        if (!urlStr.contains("?")) {
            return null;
        }
        urlStr = urlStr.substring(urlStr.indexOf('?') + 1);
        String[] params = urlStr.split("&");
        for (String p : params) {
            if (p.startsWith("body=")) {
                try {
                    return URLDecoder.decode(p.substring(5), "UTF-8");
                } catch (UnsupportedEncodingException e) { }
            }
        }
        return null;
    }

    private void updateThreadIdIfRunning() {
        if (mIsRunning && mConversation != null) {
            if (mConversation.getMessageCount() > 0) {
                MessagingNotification.setCurrentlyDisplayedThreadId(mConversation.getThreadId());
            } else {
                MessagingNotification
                    .setCurrentlyDisplayedThreadId(MessagingNotification.THREAD_NONE);
            }
        }
        // If we're not running, but resume later, the current thread ID will be set in onResume()
    }

     //////////////////////////////////////////////////////////////////////////////////////
     // MTK add

     /// M: Code analyze 012, add for multi-delete @{
     public static final int REQUEST_CODE_FOR_MULTIDELETE  = 110;
     /// @}

     /// M: Code analyze 025, Add video or audio attachment and check the attachment size.@{
     public static final int MIN_SIZE_FOR_CAPTURE_VIDEO    = 1024 * 10;  // 10K
     public static final int MIN_SIZE_FOR_RECORD_AUDIO = 1024 * 5; // 5K
     // M: fix bug ALPS00354728
     private boolean mAppendAttachmentSign = true;
     /// @}

     /// M: Code analyze 014, Add quick text. @{
     private static final int MENU_ADD_QUICK_TEXT         = 8;
     private AlertDialog mQuickTextDialog;
     /// @}

     /// M: Code analyze 015, Add text vcard. @{
     private static final int MENU_ADD_TEXT_VCARD         = 9;
     public static final int REQUEST_CODE_TEXT_VCARD       = 22;
     /// @}

     private static final int MENU_CALL_RECIPIENT_BY_VT  = 10;
     /// M: Code analyze 016, Add for select text copy. @{
     private static final int MENU_SELECT_TEXT             = 36;
     /// @}

     private static final String SIGN_CREATE_AFTER_KILL_BY_SYSTEM = "ForCreateAfterKilledBySystem";

     /// M: Code analyze 017, Handle forwarded message.(see:forwardMessage())@{
     public static final String SMS_ADDRESS = "sms_address";
     public static final String SMS_BODY = "sms_body";
     public static final String FORWARD_MESSAGE = "forwarded_message";
     /// @}

     // State variable indicating an image is being compressed, which may take a while.
     private boolean mCompressingImage = false;
     private int mToastCountForResizeImage = 0; // For indicate whether show toast message for
                        //resize image or not. If mToastCountForResizeImage equals 0, show toast.
     /// M: Code analyze 010, Support dirtory mode. @{
     /* P0 migration*/
     //private int mHomeBox = 0;
     /// @}

     /// M: Code analyze 009,Show attachment dialog . @{
     private SoloAlertDialog mSoloAlertDialog;
     /// @}

     private boolean  mDrawBottomPanel = false;

     /// M: the member is only used by onUpdate
     private static final long UPDATE_DELAY = 100L;

     /// M: Code analyze 011, use another method for performance
     ///(use this to limit the contact query count) @{
     private static final int UPDATE_LIMIT_LANDSCAPE = 20;
     private static final int UPDATE_LIMIT_PORTRAIT = 20;
     /// @}

     ///@ M:fix bug ALPS00871320 tablet has wide space to show more contact
     private static final int UPDATE_LIMIT_LANDSCAPE_TABLET = 30;
     private static final int UPDATE_LIMIT_PORTRAIT_TABLET = 30;

     private int getLimitedContact() {
        boolean isPortrait =
            getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT;
        int updateLimit = 0;

        boolean isTablet = getResources().getBoolean(R.bool.isTablet);
        if (isPortrait) {
            updateLimit = isTablet ? UPDATE_LIMIT_PORTRAIT_TABLET
                    : UPDATE_LIMIT_PORTRAIT;
        } else {
            updateLimit = isTablet ? UPDATE_LIMIT_LANDSCAPE_TABLET
                    : UPDATE_LIMIT_LANDSCAPE;
        }
        return updateLimit;
     }

     ///@
     private Runnable mPrevRunnable;
     private boolean mNeedSaveAsMms = false;

     /// M: Code analyze 038, If the user is editing slideshow now.
     /// Do not allow the activity finish but return directly when back key is entered. @{
     private boolean mIsEditingSlideshow = false;
     /// @}

     /// M: Code analyze 026, If the two clicks are too close. @{
     private long mAttachClickTime = -1;
     private long mEditClickTime = -1;
     /// @}

     // mAudioUri will look like this: content://media/external/images/media
     private static final String mAudioUri = Audio.Media.getContentUri("external").toString();

     /// M: Code analyze 050, Add scroll listener and touch listener for MessageListView.@{
     private static int CHANGE_SCROLL_LISTENER_MIN_CURSOR_COUNT = 100;
     private MyScrollListener mScrollListener = new MyScrollListener(
             CHANGE_SCROLL_LISTENER_MIN_CURSOR_COUNT, "MessageList_Scroll_Tread");
     /// @}

     /// M: Code analyze 018, Add ringtone for sound attachment.  @{
     public static final int REQUEST_CODE_ATTACH_RINGTONE  = 20;
     /// @}

     /// M: Code analyze 019, Add vcard attachment.  @{
     public static final int REQUEST_CODE_ATTACH_VCARD       = 21;
     /// @}

     /// M: Code analyze 020, Add vcalendar attachment.  @{
     public static final int REQUEST_CODE_ATTACH_VCALENDAR = 25;
     /// @}

     /// M: Code analyze 021, Copy all valid parts of the attachment(pdu) to SD card.
     /// This opeartor will be removed to a separate activity.    @{
     public static final int REQUEST_CODE_MULTI_SAVE       = 23;
     /// @}

     /// M: Code analyze 022, Add bookmark. @{
     private static final int MENU_ADD_TO_BOOKMARK         = 35;
     private ArrayList<String> mURLs = new ArrayList<String>();
     /// @}

     /// M: Code analyze 007, Get information from Sub or save message to Sub. @{
     private static final int MENU_SAVE_MESSAGE_TO_SUB     = 32;
     private static final int SUB_SELECT_FOR_SEND_MSG                     = 1;
     private static final int SUB_SELECT_FOR_SAVE_MSG_TO_SUB             = 2;
     private static final int MSG_QUIT_SAVE_MESSAGE_THREAD                 = 100;
     private static final int MSG_SAVE_MESSAGE_TO_SUB                     = 102;
     private static final int MSG_SAVE_MESSAGE_TO_SUB_AFTER_SELECT_SUB     = 104;
     private static final int MSG_SAVE_MESSAGE_TO_SUB_SUCCEED             = 106;
     private static final int MSG_SAVE_MESSAGE_TO_SUB_FAILED_GENERIC     = 108;
     private static final int MSG_SAVE_MESSAGE_TO_SUB_FAILED_SUB_FULL     = 110;

     private static final String SELECT_TYPE    = "Select_type";
     private int mSubCount; //The count of current sub cards.  0/1/2
    private List<SubscriptionInfo> mSubInfoList;
     private Handler mSaveMsgHandler = null;
     private Thread mSaveMsgThread = null;
     private AlertDialog mSubSelectDialog;
     private int mMessageSubId;
    private int mAssociatedSubId;
    private int mSelectedSubId;
     /// @}

     /// M: Code analyze 006, Control Sub indicator on status bar. @{
     private StatusBarManager mStatusBarManager;
     private ComponentName mComponentName;
     /// @}

     /// M: Code analyze 056,Now,the sms recipient limit is different from mms.
     /// We can set limit for sms or mms individually. @{
     private static final int RECIPIENTS_LIMIT_FOR_SMS     = MmsConfig.getRecipientLimit();
     /// @}
     private boolean mIsTooManyRecipients;     // Whether the recipients are too many

     /// M: Code analyze 046, Whether the recipientedit control has been initialized. @{
     private boolean isInitRecipientsEditor = true; // true, init Recip Editor and add recipient;
                                                   // false, init Recip Editor, but recipient
     /// @}

     private boolean mWaitingForSendMessage;

     /// M: Code analyze 023, Delete the char value of '\r' . @{
     private static final String STR_RN = "\\r\\n"; // for "\r\n"
     private static final String STR_CN = "\n"; // the char value of '\n'
     /// @}

     /// M: Code analyze 027,Add for deleting one message.@{
     private ThreadCountManager mThreadCountManager = ThreadCountManager.getInstance();
     private Long mThreadId = -1L;
     /// @}

     /// M: Code analyze 002,  If a new ComposeMessageActivity is created, kill old one
     private static WeakReference<ComposeMessageActivity> sCompose = null;
     /// @}

     private static String sTextEditorText;
     private long mLastThreadIdFromNotification;

     private boolean mSendButtonCanResponse = true;    // can click send button
     private static final long RESUME_BUTTON_INTERVAL = 1000;
     private static final int MSG_RESUME_SEND_BUTTON  = 112;

     /// M: Code analyze 024, If the click operator can be responsed. @{
     boolean mClickCanResponse = true;           // can click button or some view items
     /// @}

     /// M: Code analyze 013, Get contacts from Contact app . @{
     // handle NullPointerException in onActivityResult() for pick up recipients
     private boolean mIsRecipientHasIntentNotHandle = false;
     private Intent mIntent = null;
     private boolean misPickContatct = false;
     /// @}

     /// M: Code analyze 004, Set max height for text editor. @{
     private HeightChangedLinearLayout mHeightChangedLinearLayout;
     private static final int mReferencedTextEditorTwoLinesHeight = 65;
     private static final int mReferencedTextEditorThreeLinesHeight = 110;
     private static final int mReferencedTextEditorFourLinesHeight    = 140;
     private static final int mReferencedTextEditorSevenLinesHeight = 224;
     private static final int mReferencedAttachmentEditorHeight     = 266;
     private static final int mReferencedMaxHeight                    = 800;
     private int mCurrentMaxHeight                                    = 800;
     /// @}

     /// M: Code analyze 042, If you discard the draft message manually.@{
     private boolean mHasDiscardWorkingMessage = false;
     /// @}

    /// M: fix bug ALPS00572383, delay show "suggested" on the SelectSubDialog @{
    private final HashMap<Integer, Integer> mHashSub = new HashMap<Integer, Integer>();
    private static final int MSG_SELECT_SUB_DIALOG_SHOW = 500;
    private int mAssociatedSubQueryDone;
    private int mSelectSubType;
    /// @}

    private static final int MSG_DISMISS_CONTACT_PICK_DIALOG = 9009;
    /// M:
    private Handler mUiHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_SAVE_MESSAGE_TO_SUB_SUCCEED:
                Toast.makeText(ComposeMessageActivity.this,
                        R.string.save_message_to_sim_successful, Toast.LENGTH_SHORT).show();
                // M: fix for bug ALPS01468873
                //sendBroadcast(new Intent(ManageSimMessages.ACTION_NOTIFY_SIMMESSAGE_UPDATE));
                break;

            case MSG_SAVE_MESSAGE_TO_SUB_FAILED_GENERIC:
                Toast.makeText(ComposeMessageActivity.this,
                        R.string.save_message_to_sim_unsuccessful, Toast.LENGTH_SHORT).show();
                break;

            case MSG_SAVE_MESSAGE_TO_SUB_FAILED_SUB_FULL:
                int slotId = -1;
                if (mSubCount == 1) {
                    slotId = mSubInfoList.get(0).getSimSlotIndex();
                } else {
                    if (mSelectedSubId > 0) {
                        SubscriptionInfo subInfo = SubscriptionManager.from(
                                MmsApp.getApplication()).getActiveSubscriptionInfo(mSelectedSubId);
                        if (subInfo != null) {
                            slotId = subInfo.getSimSlotIndex();
                        }
                    }
                }
                Toast.makeText(ComposeMessageActivity.this,
                        getString(R.string.save_message_to_sim_unsuccessful) + ", "
                        + (getString(R.string.sim_full_title)), Toast.LENGTH_SHORT).show();
                /// @}
                break;
            /// M: Code analyze 007, Get information from Sub or save message to Sub. @{
            case MSG_SAVE_MESSAGE_TO_SUB: //multi sub cards
                String type = (String) msg.obj;
                long msgId = msg.arg1;
                saveMessageToSub(type, msgId);
                break;
            /// @}
            case MSG_RESUME_SEND_BUTTON:
                mSendButtonCanResponse = true;
                break;
            case MSG_SELECT_SUB_DIALOG_SHOW:
                /// M: fix bug ALPS00572383, delay show "suggested" on the SelectSimDialog @{
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        int position = -1;
                        if (mHashSub != null && mSelectSubType == SUB_SELECT_FOR_SEND_MSG) {
                            for (int i = 0; i < mHashSub.size(); i++) {
                                if (mHashSub != null
                                        && mHashSub.get(i) == mAssociatedSubQueryDone) {
                                    position = i;
                                    break;
                                }
                            }

                            if (position != -1 && mSubSelectDialog != null
                                                        && mSubSelectDialog.isShowing()) {
                                ListView listView = mSubSelectDialog.getListView();
                                if (listView != null) {
                                    View view = listView.getChildAt(position);
                                    if (view != null) {
                                       TextView textView =
                                           (TextView) view.findViewById(R.id.sim_suggested);
                                       if (textView != null) {
                                           textView.setText(getString(R.string.suggested));
                                       }
                                    }
                                }
                                mHashSub.clear();
                            }
                        }
                    }
                });
                break;
                /// @}
            case MSG_DISMISS_CONTACT_PICK_DIALOG:
                mUiHandler.removeCallbacks(mContactPickRunnable);
                if (mContactPickDialog != null && mContactPickDialog.isShowing()) {
                    mContactPickDialog.dismiss();
                }
                mContactPickDialog = null;
                break;
            default:
                MmsLog.w(TAG, "inUIHandler msg unhandled.");
                break;
            }
        }
    };

    /// M: Code analyze 007, Get information from Sub or save message to Sub. @{
    private final class SaveMsgThread extends Thread {
        private String msgType = null;
        private long msgId = 0;
        public SaveMsgThread(String type, long id) {
            msgType = type;
            msgId = id;
        }
        public void run() {
            Looper.prepare();
            if (null != Looper.myLooper()) {
                mSaveMsgHandler = new SaveMsgHandler(Looper.myLooper());
            }
            Message msg = mSaveMsgHandler.obtainMessage(MSG_SAVE_MESSAGE_TO_SUB);
            msg.arg1 = (int) msgId;
            msg.obj = msgType;
            if (mSubCount > 1) { // multi sub cards
                mUiHandler.sendMessage(msg);
            } else {
                mSaveMsgHandler.sendMessage(msg); //single sub card
            }
            Looper.loop();
        }
    }

    private final class SaveMsgHandler extends Handler {
        public SaveMsgHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_QUIT_SAVE_MESSAGE_THREAD: {
                    MmsLog.i(MmsApp.TXN_TAG, "exit save message thread");
                    getLooper().quit();
                    break;
                }

                case MSG_SAVE_MESSAGE_TO_SUB: { //single sub card
                    String type = (String) msg.obj;
                    long msgId = msg.arg1;
                    //saveMessageTo(type, msgId);
                    getMessageAndSaveToSub(type, msgId);
                    break;
                }

                case MSG_SAVE_MESSAGE_TO_SUB_AFTER_SELECT_SUB: {
                    Intent it = (Intent) msg.obj;
                    getMessageAndSaveToSub(it);
                    break;
                }

                default:
                    break;
            }
        }
    }

    private void saveMessageToSub(String msgType, long msgId) { //multi sub cards exist
        MmsLog.d(MmsApp.TXN_TAG, "save message to sub, message type:" + msgType
                + "; message id:" + msgId + "; sub count:" + mSubCount);

        Intent intent = new Intent();
        intent.putExtra("message_type", msgType);
        intent.putExtra("message_id", msgId);
        intent.putExtra(SELECT_TYPE, SUB_SELECT_FOR_SAVE_MSG_TO_SUB);
        mSelectSubType = SUB_SELECT_FOR_SAVE_MSG_TO_SUB;
        showSubSelectedDialog(intent);
    }

    private void getMessageAndSaveToSub(Intent intent) {
        MmsLog.i(MmsApp.TXN_TAG,
                "get message and save to sub, selected sub id = " + mSelectedSubId);
        String msgType = intent.getStringExtra("message_type");
        long msgId = intent.getLongExtra("message_id", 0);
        if (msgType == null) {
            //mSaveMsgHandler.sendEmptyMessage(MSG_SAVE_MESSAGE_TO_SIM_FAILED_GENERIC);
            mUiHandler.sendEmptyMessage(MSG_SAVE_MESSAGE_TO_SUB_FAILED_GENERIC);
            return;
        }
        getMessageAndSaveToSub(msgType, msgId);
    }

    private void getMessageAndSaveToSub(String msgType, long msgId) {
        int result = 0;
        ///M: ALPS00726802, get the orignal position when long click this item
        Cursor cursor = mMsgListAdapter.getCursor();
        if (mClickedItemPosition >= 0 && mClickedItemPosition < cursor.getCount()) {
            cursor.moveToPosition(mClickedItemPosition);
        }
        MessageItem msgItem = getMessageItem(msgType, msgId, true);
        if (msgItem == null || msgItem.mBody == null) {
            MmsLog.e(MmsApp.TXN_TAG, "getMessageAndSaveToSub, can not get Message Item.");
            return;
        }

        String scAddress = null;

        ArrayList<String> messages = null;
        messages = SmsManager.getDefault().divideMessage(msgItem.mBody);

        int smsStatus = 0;
        long timeStamp = 0;
        if (msgItem.isReceivedMessage()) {
            smsStatus = SmsManager.STATUS_ON_ICC_READ;
            timeStamp = msgItem.mSmsDate;
            scAddress = msgItem.getServiceCenter();
        } else if (msgItem.isSentMessage()) {
            smsStatus = SmsManager.STATUS_ON_ICC_SENT;
        } else if (msgItem.isFailedMessage()) {
            smsStatus = SmsManager.STATUS_ON_ICC_UNSENT;
        } else {
            MmsLog.w(MmsApp.TXN_TAG, "Unknown sms status");
        }
        int subId = -1;
        if (mSubCount == 1) {
            mSelectedSubId = (int) mSubInfoList.get(0).getSubscriptionId();
        }
        subId = mSelectedSubId;
        if (scAddress == null) {
            scAddress = MtkSmsManager.getSmsManagerForSubscriptionId(subId).getScAddress();
        }

        MmsLog.dpi(MmsApp.TXN_TAG, "\t subId\t= " + subId
                + "\t scAddress\t= " + scAddress
                + "\t Address\t= " + msgItem.mAddress
                + "\t msgBody\t= " + msgItem.mBody
                + "\t smsStatus\t= " + smsStatus
                + "\t timeStamp\t= " + timeStamp
                + "\t messages size\t= " + messages.size());

        /// M: add for ALPS01844319, check SIM whether locked first, return fail if locked. @{
        TelephonyManager telephony =
            (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);

        boolean simOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        boolean simOn = true;

        MmsLog.d(MmsApp.TXN_TAG, "simOnOffEnabled = " + simOnOffEnabled);
        if (simOnOffEnabled == true) {
            int slotId = SubscriptionManager.getSlotIndex(subId);
            MmsLog.d(MmsApp.TXN_TAG, "slotId = " + slotId);
                int slotState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
                MmsLog.d(MmsApp.TXN_TAG, "slotState = " + slotState);
                if (slotState == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                    simOn = true;
                } else {
                    simOn = false;
                }
        }

        if (simOn == true) {
            int simState = telephony.getSimState(SubscriptionManager.getSlotIndex(subId));
            MmsLog.d(MmsApp.TXN_TAG, "\t simState \t= " + simState);
            // set result as fail if SIM locked.
            if (simState == TelephonyManager.SIM_STATE_PIN_REQUIRED) {
                result = SmsManager.RESULT_ERROR_GENERIC_FAILURE;
            } else {
                result = MtkSmsManager.getSmsManagerForSubscriptionId(subId).copyTextMessageToIccCard(
                        scAddress, msgItem.mAddress, messages, smsStatus, timeStamp);
            }
        } else {
            result = SmsManager.RESULT_ERROR_GENERIC_FAILURE;
        }

        /// @}
        MmsLog.d(MmsApp.TXN_TAG, "\t result\t= " + result);
        if (result == MtkSmsManager.RESULT_ERROR_SUCCESS) {
            MmsLog.d(MmsApp.TXN_TAG, "save message to sub succeed.");
            mUiHandler.sendEmptyMessage(MSG_SAVE_MESSAGE_TO_SUB_SUCCEED);
        } else if (result == MtkSmsManager.RESULT_ERROR_SIM_MEM_FULL) {
            MmsLog.w(MmsApp.TXN_TAG, "save message to sub failed: sub memory full.");
            mUiHandler.sendEmptyMessage(MSG_SAVE_MESSAGE_TO_SUB_FAILED_SUB_FULL);
        } else {
            MmsLog.w(MmsApp.TXN_TAG, "save message to sub failed: generic error.");
            mUiHandler.sendEmptyMessage(MSG_SAVE_MESSAGE_TO_SUB_FAILED_GENERIC);
        }
        mSaveMsgHandler.sendEmptyMessageDelayed(MSG_QUIT_SAVE_MESSAGE_THREAD, 5000);
    }

    Runnable mGetSubInfoRunnable = new Runnable() {
        public void run() {
            getSubInfoList();
        }
    };

     private void getSubInfoList() {
        mSubInfoList = SubscriptionManager.from(this).getActiveSubscriptionInfoList();
        mSubCount = (mSubInfoList != null && !mSubInfoList.isEmpty()) ? mSubInfoList.size() : 0;
    }

     private void subSelection() {
        if (mIpCompose.subSelection()) {
            return;
        }
        int selectedSubId = mOpComposeExt.subSelection();
        if (selectedSubId != 0) {
            mSelectedSubId = selectedSubId;
            return;
        }
        /* SIM ON OFF*/
        int count = getEnabledSubCount();
        if (count == 1) {
            selectedSubId = getFirstEnabledSubId();
            if (selectedSubId < 0) {
                return;
            }
            mSelectedSubId = selectedSubId; // (int) mSubInfoList.get(0).getSubscriptionId();
            confirmSendMessageIfNeeded();
        } else if (count > 1) { // multi sub cards exist.
            Intent intent = new Intent();
            mSelectSubType = SUB_SELECT_FOR_SEND_MSG;
            intent.putExtra(SELECT_TYPE, SUB_SELECT_FOR_SEND_MSG);
            // getContactSIM
            mAssociatedSubId = -1;
/*
  * Op18 : Dual Send button is shown in compose screen .
  * This setting is not saved along with subscriptionManager.getDefaultSmsSubId()
  * So ,  check its value in plugin , if false handle default else plugin will handle send message

*/
         MmsLog.d(TAG, "mAssociatedSubId = " + mAssociatedSubId);

           if (mAssociatedSubId == -1 && mOpComposeExt.sendMessageForDualSendButton() == false) {

                // getDefaultSIM()
                mMessageSubId = SubscriptionManager.getDefaultSmsSubscriptionId();
                MmsLog.d(TAG, "mMessageSubId = " + mMessageSubId);
                if (mMessageSubId == MtkDefaultSmsSimSettings.ASK_USER_SUB_ID
                        || mOpComposeExt.subSelectionDialog(mMessageSubId)) {
                    // always ask, show SIM selection dialog
                    showSubSelectedDialog(intent);
                    updateSendButtonState();
                } else if (mMessageSubId == MmsConfig.DEFAULT_SIM_NOT_SET) {
                    /*
                     * not set default Sub:
                     * if recipients are morn than 2,or there is no associated Sub,
                     * show SIM selection dialog
                     * else send message via associated SIM
                     */
                    if (mAssociatedSubId == -1) {
                        showSubSelectedDialog(intent);
                        updateSendButtonState();
                    } else {
                        mSelectedSubId = mAssociatedSubId;
                        confirmSendMessageIfNeeded();
                    }
                } else {
                    /*
                     * default SIM:
                     * if recipients are morn than 2,or there is no associated SIM,
                     * send message via default SIM
                     * else show SIM selection dialog
                     */
                    boolean isAssociatedSubExsit = false;
                    List<SubscriptionInfo> subinfoList
                        = SubscriptionManager.from(this).getActiveSubscriptionInfoList();
                    int subCount = 0;
                    if (subinfoList != null) {
                        subCount = subinfoList.size();
                    }
                    for (int slotId = 0; slotId < subCount; slotId++) {
                        SubscriptionInfo subInfo = subinfoList.get(slotId);
                        if (subInfo != null) {
                            int subId = subInfo.getSubscriptionId();
                            isAssociatedSubExsit = (subId == mAssociatedSubId ? true : false);
                            if (isAssociatedSubExsit) {
                                break;
                            }
                        }
                    }
                    if ((mAssociatedSubId == -1 || (mMessageSubId == mAssociatedSubId)
                            || !isAssociatedSubExsit)
                            && MessageUtils.isSmsSubIdActive(this, mMessageSubId)) {
                        mSelectedSubId = (int) mMessageSubId;
                        confirmSendMessageIfNeeded();
                    } else {
                        showSubSelectedDialog(intent);
                        updateSendButtonState();
                    }
                }
            }
        }
    }

    @Override
    public void onDialogClick(int subId, Intent intent) {
        mSelectedSubId = (int) subId;
        if (intent.getIntExtra(SELECT_TYPE, -1) == SUB_SELECT_FOR_SEND_MSG) {
            confirmSendMessageIfNeeded();
            mIsOneSubSelected = true;
        } else if (intent.getIntExtra(SELECT_TYPE, -1) == SUB_SELECT_FOR_SAVE_MSG_TO_SUB) {
            // getMessageAndSaveToSim(it);
            Message msg = mSaveMsgHandler.obtainMessage(MSG_SAVE_MESSAGE_TO_SUB_AFTER_SELECT_SUB);
            msg.obj = intent;
            // mSaveMsgHandler.sendMessageDelayed(msg, 60);
            mSaveMsgHandler.sendMessage(msg);
        }
    }

    @Override
    public void onCancelClick() {

    }

    @Override
    public void onDialogDismiss() {
        if (isRecipientsEditorVisible() && mCutRecipients != null) {
            mRecipientsEditor.removeChipChangedListener(mChipWatcher);
            mRecipientsEditor.populate(new ContactList());
            mRecipientsEditor.addChipChangedListener(mChipWatcher);
            mRecipientsEditor.populate(mCutRecipients);
            mCutRecipients = null;
            // / M : Fix CR ALPS01009525
            // / which will a empty thread in the widget that send group message
            // from contact @{
            if (mOldThreadID > 0) {
                Conversation.asyncDeleteObsoleteThreadID(mBackgroundQueryHandler, mOldThreadID);
            }
            // / @}
        } else if (mCutRecipients != null) {
            if (mConversation != null) {
                Log.d(TAG,
                        "onDismiss, mark old thread draft status to false: "
                                + mConversation.getThreadId());
                mConversation.setDraftState(false);
            }
            mWorkingMessage.syncWorkingRecipients();
            Conversation conv = mWorkingMessage.getConversation();
            long threadId = 0L;
            if (conv != null) {
                conv.ensureThreadId();
                threadId = conv.getThreadId();
            }
            // / M: fix bug ALPS00595715 @{
            mWorkingMessage.saveDraft(false);
            if (mIsOneSubSelected) {
                startMsgListQuery(MESSAGE_LIST_QUERY_TOKEN, 0);
                mIsOneSubSelected = false;
            } else {
                startActivity(createIntent(ComposeMessageActivity.this, threadId));
            }
            // /@}
            // / M: fix bug ALPS00595715 @{
            if (mOldThreadID > 0 && mOldThreadID != threadId) {
                mIsSameConv = false;
                MmsLog.d(TAG, "onDismiss not same thread");
            }
            mCutRecipients = null;
        }
    }

    private boolean mIsOneSubSelected = false;

    private void showSubSelectedDialog(Intent intent) {
        mIsOneSubSelected = false;
        SubSelectDialog subSelectDialog = new SubSelectDialog(this, this);

        String str = null;
        mSubSelectDialog = subSelectDialog.showSubSelectedDialog(true, str, intent);

        /// M: fix bug ALPS00572383, delay show "suggested" on the SelectSubDialog @{
        if (mAssociatedSubQueryDone != -1) {
            mUiHandler.sendEmptyMessage(MSG_SELECT_SUB_DIALOG_SHOW);
        }
        /// @}
    }
    /// @}
    /// M: Code analyze 003,  Set or get max mms size.
    private void initMessageSettings() {
        MessageUtils.setMmsLimitSize(this);
    }
    /// @}

    private void showConfirmDialog(Uri uri, boolean append, int type, int messageId) {
        if (isFinishing()) {
            return;
        }

        final Uri mRestrictedMidea = uri;
        final boolean mRestrictedAppend = append;
        final int mRestrictedType = type;

        new AlertDialog.Builder(ComposeMessageActivity.this)
        .setTitle(R.string.unsupport_media_type)
        .setIconAttribute(android.R.attr.alertDialogIcon)
        .setMessage(messageId)
        .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public final void onClick(DialogInterface dialog, int which) {
                    /// M: disable when non-default sms
                    if (!mIsSmsEnabled) {
                        Toast.makeText(ComposeMessageActivity.this,
                                R.string.compose_disabled_toast, Toast.LENGTH_SHORT).show();
                        return;
                    }
                    if (mRestrictedMidea == null || mRestrictedType == WorkingMessage.TEXT
                        || mWorkingMessage.isDiscarded()) {
                        return;
                    }
                    getAsyncDialog().runAsync(new Runnable() {
                        public void run() {
                            /// M: fix bug ALPS01258201, show progessDialog
                            runOnUiThread(new Runnable() {
                                public void run() {
                                    Log.d(TAG, "reCreate and show ProgressDialog");
                                    getAsyncDialog().reCreateProgressDialog(
                                            R.string.adding_attachments_title);
                                    getAsyncDialog().resetShowProgressDialog();
                                }
                            });
                            int createMode = WorkingMessage.sCreationMode;
                            WorkingMessage.sCreationMode = 0;
                            int result = mWorkingMessage.setAttachment(mRestrictedType,
                                    mRestrictedMidea, mRestrictedAppend);
                            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                                MmsLog.vpi(TAG, "Restricted Midea: dataUri=" + mRestrictedMidea);
                            }
                            if (mRestrictedType == WorkingMessage.IMAGE
                                && (result == WorkingMessage.IMAGE_TOO_LARGE
                                        || result == WorkingMessage.MESSAGE_SIZE_EXCEEDED)) {
                                MmsLog.dpi(TAG, "showConfirmDialog: resize image "
                                        + mRestrictedMidea);
                                MessageUtils.resizeImage(ComposeMessageActivity.this,
                                        mRestrictedMidea, mAttachmentEditorHandler,
                                        mResizeImageCallback, mRestrictedAppend,
                                    true);
                                WorkingMessage.sCreationMode = createMode;
                                dismissProgressDialog();
                                return;
                            }
                            WorkingMessage.sCreationMode = createMode;
                            int typeId = R.string.type_picture;
                            if (mRestrictedType == WorkingMessage.AUDIO) {
                                typeId = R.string.type_audio;
                            } else if (mRestrictedType == WorkingMessage.VIDEO) {
                                typeId = R.string.type_video;
                            }
                            handleAddAttachmentError(result, typeId);
                            /// M: fix bug ALPS00726611, must save draft when click on WARNING_TYPE
                            if (result == WorkingMessage.OK) {
                                if (mWorkingMessage.saveAsMms(false) != null) {
                                    mHasDiscardWorkingMessage = true;
                                }
                            }
                            dismissProgressDialog();
                        }
                    }, new ShowRunnable(), R.string.adding_attachments_title);
                    Log.d(TAG, "Composer add mAsyncTaskNum = " + (++mAsyncTaskNum));
            }
        })
        .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
            public final void onClick(DialogInterface dialog, int which) {
                mWorkingMessage.removeFakeMmsForDraft();
                updateSendButtonState();
            }
        })
        .setOnCancelListener(new DialogInterface.OnCancelListener() {
            public void onCancel(DialogInterface arg0) {
                mWorkingMessage.removeFakeMmsForDraft();
                updateSendButtonState();
            }
        })
        .show();
    }

     /// M: Code analyze 067, Add audio attachment. @{
    private void addAudio(final Uri uri, final boolean append) {
        mNeedSaveAsMms = false;
        MmsLog.i(TAG, "addAudio(): uri = " + uri + ", append = " + append);
        if (uri != null) {
            mNeedSaveAsMms = true;
            int result = WorkingMessage.OK;
            try {
                if (append) {
                    mWorkingMessage.checkSizeBeforeAppend();
                }
            } catch (ExceedMessageSizeException e) {
                result = WorkingMessage.MESSAGE_SIZE_EXCEEDED;
                handleAddAttachmentError(result, R.string.type_audio);
                mNeedSaveAsMms = false;
                return;
            }
            result = mWorkingMessage.setAttachment(WorkingMessage.AUDIO, uri, append);
            if (result == WorkingMessage.WARNING_TYPE) {
                mNeedSaveAsMms = false;
                runOnUiThread(new Runnable() {
                    public void run() {
                        showConfirmDialog(uri, append,
                                WorkingMessage.AUDIO, R.string.confirm_restricted_audio);
                    }
                });
            } else {
                handleAddAttachmentError(result, R.string.type_audio);
                if (result != WorkingMessage.OK) {
                    mNeedSaveAsMms = false;
                }
            }
        }
    }
    /// @}

    /// M: Code analyze 015, Add text vcard. @{
    private void addTextVCardAsync(final long[] contactsIds) {
        MmsLog.i(TAG, "compose.addTextVCardAsync(): contactsIds.length() = " + contactsIds.length);
        getAsyncDialog().runAsync(new Runnable() {
            public void run() {
                //addTextVCard(contactsIds);
               String textVCard = TextUtils.isEmpty(mTextEditor.getText()) ? "" : "\n";
               VCardAttachment tvc = new VCardAttachment(ComposeMessageActivity.this);
               final String textString = tvc.getTextVCardString(contactsIds, textVCard);
               runOnUiThread(new Runnable() {
               public void run() {
                   insertText(mTextEditor, textString);
               }

             });
           }
        }, null, R.string.menu_insert_text_vcard); // the str is ok for reuse[or use a new string].
    }
    /// @}

    private void addFileAttachment(String type, Uri uri, boolean append) {
        if (!addFileAttachment(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, uri, append)) {
            if (!addFileAttachment(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, uri, append)) {
                if (!addFileAttachment(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                        uri, append)) {
                    MmsLog.i(TAG, "This file is not in media store(audio, video or image)," +
                            "attemp to add it like file uri");
                    addAttachment(type, (Uri) uri, append);
                }
            }
        }
    }

    private boolean addFileAttachment(Uri mediaStoreUri, Uri uri, boolean append) {
        String path = uri.getPath();
        if (path != null) {
            Cursor c = getContentResolver().query(mediaStoreUri,
                    new String[] {MediaStore.MediaColumns._ID, Audio.Media.MIME_TYPE},
                    MediaStore.MediaColumns.DATA + "=?",
                    new String[] {path}, null);
            if (c != null) {
                try {
                    if (c.moveToFirst()) {
                        Uri contentUri = Uri.withAppendedPath(mediaStoreUri, c.getString(0));
                        MmsLog.ipi(TAG, "Get id in MediaStore:" + c.getString(0)
                                + " Get content type in MediaStore:" + c.getString(1)
                                + " Get uri in MediaStore:" + contentUri);
                        String contentType = c.getString(1);
                        addAttachment(contentType, contentUri, append);
                        return true;
                    } else {
                        MmsLog.ipi(TAG, "MediaStore:" + mediaStoreUri.toString()
                                + " has not this file");
                    }
                } finally {
                    c.close();
                }
            }
        }
        return false;
    }

    private boolean isHasRecipientCount() {
        int recipientCount = recipientCount();
        return (recipientCount > 0 && recipientCount < RECIPIENTS_LIMIT_FOR_SMS);
    }

    private String getResourcesString(int id) {
        Resources r = getResources();
        return r.getString(id);
    }

    /// M: Code analyze 030, Check condition before sending message.@{
    public void checkConditionsAndSendMessage(final boolean bCheckEcmMode) {
        // check pin
        if (mSelectedSubId <= 0) {
            mSelectedSubId = SubscriptionManager.getDefaultSubscriptionId();
        }
        // add CellConnMgr feature
        if (showWfcSendButtonPopUp()) {
            updateSendButtonState(true);
            return;
        }
        final CellConnMgr cellConnMgr = new CellConnMgr(getApplicationContext());
        final int state = cellConnMgr.getCurrentState(mSelectedSubId, CellConnMgr.STATE_FLIGHT_MODE
                | CellConnMgr.STATE_SIM_LOCKED | CellConnMgr.STATE_RADIO_OFF);
        ///M: WFC: Do not show pop-up, if wfc is OFF @ {
        if (!MtkTelephonyManagerEx.getDefault().isWifiCallingEnabled(mSelectedSubId) &&
            (((state & CellConnMgr.STATE_FLIGHT_MODE) == CellConnMgr.STATE_FLIGHT_MODE ) ||
            ((state & CellConnMgr.STATE_RADIO_OFF) == CellConnMgr.STATE_RADIO_OFF ) ||
            ((state & (CellConnMgr.STATE_FLIGHT_MODE | CellConnMgr.STATE_RADIO_OFF))
                == (CellConnMgr.STATE_FLIGHT_MODE | CellConnMgr.STATE_RADIO_OFF))))  {
            final ArrayList<String> stringArray =
                cellConnMgr.getStringUsingState(mSelectedSubId, state);
            MmsLog.d(TAG, "CellConnMgr 1, stringArray length is " + stringArray.size());
            if (stringArray.size() == 4) {
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setTitle(stringArray.get(0));
                builder.setMessage(stringArray.get(1));
                builder.setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                        mTextEditor.requestFocus();
                        updateSendButtonState(true);
                    }
                });
                AlertDialog planeModeDialog = builder.show();
                planeModeDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
                    @Override
                    public void onDismiss(DialogInterface dialog) {
                        dialog.dismiss();
                        updateSendButtonState(true);
                    }
                });
            }
        } else if ((state & CellConnMgr.STATE_SIM_LOCKED) == CellConnMgr.STATE_SIM_LOCKED) {
            final ArrayList<String> stringArray =
                cellConnMgr.getStringUsingState(mSelectedSubId, state);
            MmsLog.d(TAG, "CellConnMgr 2, stringArray length is " + stringArray.size());
            if (stringArray.size() == 4) {
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setTitle(stringArray.get(0));
                builder.setCancelable(true);
                builder.setMessage(stringArray.get(1));
                builder.setPositiveButton(stringArray.get(2),
                        new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                            cellConnMgr.handleRequest(mSelectedSubId, state);
                            mTextEditor.requestFocus();
                            updateSendButtonState(true);
                    }
                });
                builder.setNegativeButton(stringArray.get(3),
                        new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                            mTextEditor.requestFocus();
                            updateSendButtonState(true);
                    }
                });
                builder.setOnKeyListener(new DialogInterface.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK) {
                            dialog.dismiss();
                            mTextEditor.requestFocus();
                            updateSendButtonState(true);
                        }
                        return false;
                    }
                });
                AlertDialog dialog = builder.create();
                dialog.setCanceledOnTouchOutside(false);
                dialog.show();
            }
        } else {
            if(mWorkingMessage.requiresMms()) {
                int slot = SubscriptionManager.getSlotIndex(mSelectedSubId);
                int lockPolicy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
                int slotCap = MtkTelephonyManagerEx.getDefault().getShouldServiceCapability(slot);
                Log.d(TAG, "slot: " + slot + " mSelectedSubId: "+mSelectedSubId + " GetSimLockPolicy: " +
                        lockPolicy + " getShouldServiceCap: "+slotCap);
                switch (lockPolicy) {
                ///simmelock request 1:
                case SIM_LOCK_POLICY_SLOT1_ONLY:
                ///simmelock request 2:
                case SIM_LOCK_POLICY_SLOT2_ONLY:
                ///simmelock request 3:
                case SIM_LOCK_POLICY_SLOT_ALL:
                ///simmelock request 4:
                case SIM_SLOT_LOCK_POLICY_LK_SLOT1:
                ///simmelock request 5:
                case SIM_SLOT_LOCK_POLICY_LK_SLOT2:
                ///simmelock request 6:
                case SIM_SLOT_LOCK_POLICY_LK_SLOTA:
                ///simmelock request 7:
                case SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_CS:
                ///simmelock request 8:
                case SIM_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE:
                ///simmelock request 9:
                case SIM_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE:
                    if(!(slotCap == SIM_LOCK_SIM_CAPABILITY_FULL || slotCap == SIM_LOCK_SIM_CAPABILITY_PS_ONLY)) {
                        Toast.makeText(ComposeMessageActivity.this,
                            R.string.smart_req_mms_indicator, Toast.LENGTH_LONG).show();
                        updateSendButtonState();
                        return;
                    }
                    break;
                default:
                    break;
                }
            }

            if (!mOpComposeExt.checkConditionsAndSendMessage(mWorkingMessage.requiresMms(),
                    bCheckEcmMode)) {
                checkBeforeSendMessage(bCheckEcmMode);
            }
        }
   }
   /// @}

    /// M: Code analyze 049, Update send button or attachment editor state.@{
    private void updateSendButtonState(final boolean enabled) {
        /* SIM ON OFF*/
        int subCount = getEnabledSubCount();
        if (!mWorkingMessage.hasSlideshow()) {
            View sendButton = showSmsOrMmsSendButton(mWorkingMessage.requiresMms());
            /* SIM ON OFF*/
            if (mOpComposeExt.updateSendButtonState(mWorkingMessage.requiresMms(),
                    recipientCount(), MmsConfig.getRecipientLimit(), subCount)) {
                return;
            }
            sendButton.setEnabled(enabled && (subCount > 0));
            sendButton.setFocusable(enabled && (subCount > 0));
        } else {
            /* SIM ON OFF*/
            mAttachmentEditor.setCanSend(enabled && (subCount > 0));
        }
    }
    /// @}

    private void updateSendButtonForSend() {
        /* SIM ON OFF*/
        int subCount = getEnabledSubCount();
        mSendButtonMms.setVisibility(View.GONE);
        mSendButtonSms.setVisibility(View.VISIBLE);
        mSendButtonSms.setEnabled(false);
        mSendButtonSms.setFocusable(false);
        if (mWorkingMessage.hasSlideshow()) {
            /* SIM ON OFF*/
            mAttachmentEditor.setCanSend(false && (subCount > 0));
        }
    }

    private void insertText(EditText edit, String insertText) {
        int where = edit.getSelectionStart();

        if (where == -1) {
            edit.append(insertText);
        } else {
            edit.getText().insert(where, insertText);
        }
    }

    /**
     * This filter will constrain edits not to make the length of the text
     * greater than the specified length.
     */
    class TextLengthFilter extends InputFilter.LengthFilter {
        public TextLengthFilter(int max) {
            super(max);
            mMaxLength = max;
        }

        public CharSequence filter(CharSequence source, int start, int end,
                                   Spanned dest, int dstart, int dend) {
            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                MmsLog.vpi(TAG, "TextLengthFilter source = " + source +
                        " start = " + start + " end = " + end + " dstart = "
                        + dstart + " dend = " + dend + " dest length = " + dest.length());
            }
            int keep = mMaxLength - (dest.length() - (dend - dstart));
            if (keep < (end - start)) {
                Toast.makeText(ComposeMessageActivity.this,
                        R.string.exceed_editor_size_limitation,
                        Toast.LENGTH_SHORT).show();
                mInputMethodManager.restartInput(
                        ComposeMessageActivity.this.getWindow().getCurrentFocus());
            }
            return super.filter(source, start, end, dest, dstart, dend);
            /*
            if (keep <= 0) {
                return "";
            } else if (keep >= end - start) {
                return null; // keep original
            } else {
                return source.subSequence(start, start + keep);
            }
            */
        }

        private int mMaxLength;
    }

    class RecipientsTextLengthFilter extends InputFilter.LengthFilter {
        public RecipientsTextLengthFilter(int max) {
            super(max);
            mMaxLength = max;
        }

        public CharSequence filter(CharSequence source, int start, int end,
                                   Spanned dest, int dstart, int dend) {
            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                MmsLog.d(TAG, "RecipientsTextLengthFilter source = " + source +
                        " start = " + start + " end = " + end + " dstart = "
                        + dstart + " dend = " + dend + " dest length = " + dest.length());
                MmsLog.d(TAG, "mMaxLength = " + mMaxLength);
                MmsLog.d(TAG, "mRecipientsEditor.isFocused() = " + mRecipientsEditor.isFocused());
            }
            int keep = mMaxLength - (dest.length() - (dend - dstart));
            if ((keep < (end - start)) && mRecipientsEditor.isFocused()) {
                Toast.makeText(ComposeMessageActivity.this,
                        R.string.exceed_editor_size_limitation,
                        Toast.LENGTH_SHORT).show();
                mInputMethodManager.restartInput(
                        ComposeMessageActivity.this.getWindow().getCurrentFocus());
            }
            return super.filter(source, start, end, dest, dstart, dend);
        }

        private int mMaxLength;
    }

    /// M: Code analyze 051, Hide input keyboard.@{
    private void hideInputMethod() {
        if (this.getWindow() != null && this.getWindow().getCurrentFocus() != null) {
            mInputMethodManager
                .hideSoftInputFromWindow(this.getWindow().getCurrentFocus().getWindowToken(), 0);
        }
    }
    /// @}

    // toast there are too many recipients.
    private void toastTooManyRecipients(int recipientCount) {
        final String tooManyRecipients = getString(R.string.too_many_recipients,
                recipientCount, RECIPIENTS_LIMIT_FOR_SMS);
        mUiHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(ComposeMessageActivity.this,
                        tooManyRecipients, Toast.LENGTH_LONG).show();
            }
        }, 1000);
    }

    /// M: Code analyze 013, Get contacts from Contact app . @{
    private void addContacts(int pickCount, int requestCode) {
        /// M: @{
        /*Intent intent = new Intent("android.intent.action.CONTACTSMULTICHOICE");
          intent.setType(Phone.CONTENT_ITEM_TYPE);
          intent.putExtra("request_email", true);
          intent.putExtra("pick_count", pickCount);
          misPickContatct = true;
          startActivityForResult(intent, REQUEST_CODE_PICK_CONTACT);*/

        /// M: fix bug ALPS00444752, set true to disable to Show ContactPicker
        mShowingContactPicker = true;
        misPickContatct = true;
        Intent intent = new Intent(MessageUtils.ACTION_CONTACT_SELECTION);
        intent.setType(Phone.CONTENT_TYPE);

        try {
            startActivityForResult(intent, requestCode);
        } catch (ActivityNotFoundException e) {
            mShowingContactPicker = false;
            misPickContatct = false;
            Toast.makeText(this, this.getString(R.string.no_application_response),
                    Toast.LENGTH_SHORT).show();
            MmsLog.e(TAG, e.getMessage());
        }
        /// @}
    }
/*
    private int getContactSub(final String num) {
        class Int {
            private int value = -1;
            public void  set(int n) {
                value = n;
            }
            public int get() {
                return value;
            }
        }
        final Int subID = new Int();
        final Object dbQueryLock = new Object();
        final Context mContextTemp = this.getApplicationContext();
        // query the db in another thread.
        new Thread(new Runnable() {
            public void run() {
                int subId = -1;
                String number = num;
                String formatNumber = MessageUtils.formatNumber(number, mContextTemp);
                String TrimFormatNumber = formatNumber;
                if (formatNumber != null) {
                    TrimFormatNumber = formatNumber.replace(" ", "");
                }
                Cursor associateSubCursor = ComposeMessageActivity.this.getContentResolver().query(
                    Data.CONTENT_URI,
                    new String[]{ContactsContract.Data.SIM_ASSOCIATION_ID},
                    Data.MIMETYPE + "='" + CommonDataKinds.Phone.CONTENT_ITEM_TYPE
                    + "' AND (" + Data.DATA1 + "=?" +
                    " OR " + Data.DATA1 + "=?" +
                    " OR " + Data.DATA4 + "=?" +
                    ") AND (" + ContactsContract.Data.SIM_ASSOCIATION_ID + "!= -1)",
                    new String[]{number, formatNumber, TrimFormatNumber},
                    null
                );

                if ((null != associateSubCursor) && (associateSubCursor.getCount() > 0)) {
                    associateSubCursor.moveToFirst();
                    // Get only one record is OK
                    subId = (Integer) associateSubCursor.getInt(0);
                } else {
                    subId = -1;
                }
                if (associateSubCursor != null) {
                    associateSubCursor.close();
                }
                synchronized (dbQueryLock) {
                    MmsLog.d(MmsApp.TXN_TAG, "before notify");
                    subID.set(subId);
                    dbQueryLock.notify();
                    /// M: fix bug ALPS00572383, delay show "suggested" on the SelectSimDialog @{
                    mAssociatedSubQueryDone = subId;
                }
                /// @}
            }
        }).start();
        // change UI thread wait from 500ms to 1000ms at most, for CR ALPS00721717
        synchronized (dbQueryLock) {
            try {
                MmsLog.d(MmsApp.TXN_TAG, "before wait");
                dbQueryLock.wait(1000);
            } catch (InterruptedException e) {
                //time out
            }
            MmsLog.d(MmsApp.TXN_TAG, "subID = " + subID.get());
            return subID.get();
        }
    }
*/
    private void checkRecipientsCount() {
        //if (isRecipientsEditorVisible()) {
        //mRecipientsEditor.structLastRecipient();
        //}
//        hideInputMethod();
        if (mIpCompose.onIpCheckRecipientsCount()) {
            return;
        }
        if (!MessageUtils.allowSafeDraft(this, MmsConfig.getDeviceStorageFullStatus(), true,
                TOAST_TYPE_FOR_SEND_MSG)) {
            updateSendButtonState();
            return;
        }
        final int mmsLimitCount = MmsConfig.getMmsRecipientLimit();
        String message = "";
        if (recipientCount() > mmsLimitCount) {
            message = getString(R.string.max_recipients_message, mmsLimitCount);
        }
        boolean isRecipientsEditorEmpty = isRecipientsEditorVisible()
                && "".equals(mRecipientsEditor.getText().toString().replaceAll(";", "")
                        .replaceAll(",", "").trim());
        boolean isConversationRecipientEmpty = !isRecipientsEditorVisible()
                && "".equals(mConversation.getRecipients().serialize().replaceAll(";", "")
                        .replaceAll(",", ""));

        if (mWorkingMessage.requiresMms() && (recipientCount() > mmsLimitCount)) {
            message = "";
            if (recipientCount() > mmsLimitCount) {
                message = getString(R.string.max_recipients_message, mmsLimitCount);
            }

            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.max_recipients_title);
            builder.setIconAttribute(android.R.attr.alertDialogIcon);
            builder.setCancelable(true);
            builder.setMessage(message);
            builder.setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            /*
                             * If entering an existing thread, #mRecipientsEditor
                             * never gets initialized.
                             * So, when mRecipientsEditor is not visible, it might be null.
                             */
                            List<String> recipientsList;
                            if (isRecipientsEditorVisible()) {
                                recipientsList = mRecipientsEditor.getNumbers();
                            } else {
                                recipientsList = new ArrayList<String>(
                                        Arrays.asList(getRecipients().getNumbers()));
                            }
                            List<String> newRecipientsList = new ArrayList<String>();

                            if (recipientCount() > mmsLimitCount * 2) {
                                for (int i = 0; i < mmsLimitCount; i++) {
                                    newRecipientsList.add(recipientsList.get(i));
                                }
                                mWorkingMessage.setWorkingRecipients(newRecipientsList);
                            } else {
                                for (int i = recipientCount() - 1; i >= mmsLimitCount; i--) {
                                    recipientsList.remove(i);
                                }
                                mWorkingMessage.setWorkingRecipients(recipientsList);
                                /// M: fix bug ALPS00432629
                                newRecipientsList = recipientsList;
                            }
                            subSelection();

                            /// M: fix bug ALPS00432629, update title
                            /// when recipientsList cut to 20 @{
                            ContactList list = ContactList.getByNumbers(newRecipientsList, false);
                            mCutRecipients = list;
                        }
                    });
                }
            });
            builder.setNegativeButton(R.string.no, null);
            builder.show();
            updateSendButtonState();
        } else {
            /** M:
             * fix CR ALPS00069541
             * if the message copy from sub card with unknown recipient
             * the recipient will be ""
             */
            if ((isRecipientsEditorVisible() && isRecipientsEditorEmpty)
                    || (!isRecipientsEditorVisible() && isConversationRecipientEmpty)) {
                new AlertDialog.Builder(this)
                        .setIconAttribute(android.R.attr.alertDialogIcon)
                        .setTitle(R.string.cannot_send_message)
                        .setMessage(R.string.cannot_send_message_reason)
                        .setPositiveButton(R.string.yes, new CancelSendingListener())
                        .show();
            } else {
                /// M: reply message with the card directly if
                /// only one card related in conversation
                subSelection();
            }
        }
    }

    /// M: Code analyze 002,  If a new ComposeMessageActivity is created, kill old one
    public static Activity getComposeContext() {
        return sCompose == null ? null : sCompose.get();
    }
    /// @}

    public void onShutDown() {
       /// M: fix bug ALPS01539513
       if (mWorkingMessage != null && mWorkingMessage.requiresMms()
               && mWorkingMessage.getSlideshow() != null
               && !mWorkingMessage.getSlideshow().needUpdate()) {
           Log.d(TAG, "onShutDown return");
           return;
       }
       /// M: don't save draft again if Compose is finished.
       if (!this.isFinishing()) {
           saveDraft(false);
       }
       /// @}
    }

    /*
    this function is add for read report
    */
    private final int READ_REPORT_DISABLED                      = 0;
    private final int READ_REPORT_SINGLE_MODE_ENABLED           = 1;
    private final int READ_REPORT_GEMINI_MODE_ENABLED           = 2;

    private void checkAndSendReadReport() {
        final Context ct = ComposeMessageActivity.this;
        final long threadId = mConversation.getThreadId();
        MmsLog.d(MmsApp.TXN_TAG, "checkAndSendReadReport,threadId:" + threadId);
        new Thread(new Runnable() {
            public void run() {
                SharedPreferences prefs =
                    PreferenceManager.getDefaultSharedPreferences(ComposeMessageActivity.this);
                int rrAllowed = READ_REPORT_DISABLED;
                /// @}
                // / M: [ALPS00465911] [3G Gemini+]JE when Message -> settings
                // -> cell center -> back to idle @{
                int[] totalSubIds = SubscriptionManager.from(
                        ComposeMessageActivity.this).getActiveSubscriptionIdList();
                // / @}
                rrAllowed = READ_REPORT_GEMINI_MODE_ENABLED;

                MmsLog.d(MmsApp.TXN_TAG, "rrAllowed=" + rrAllowed);
                // if read report is off, mark the mms read report status readed.
                if (rrAllowed == READ_REPORT_DISABLED) {
                    ContentValues values = new ContentValues(1);
                    String where =
                        Mms.THREAD_ID + " = " + threadId + " and " + Mms.READ_REPORT + " = 128";
                    // update uri inbox is not used, must indicate here.
                    where += " and " + Mms.MESSAGE_BOX + " = " + Mms.MESSAGE_BOX_INBOX;
                    values.put(Mms.READ_REPORT,
                            MtkPduHeaders.READ_STATUS__DELETED_WITHOUT_BEING_READ);
                    SqliteWrapper.update(ct, ct.getContentResolver(), Mms.Inbox.CONTENT_URI,
                                        values,
                                        where,
                                        null);
                    return;
                }
                if (rrAllowed > READ_REPORT_DISABLED) {
                    StringBuilder suffix = new StringBuilder();
                    switch (rrAllowed) {
                        case READ_REPORT_SINGLE_MODE_ENABLED:
                            // nothing to do in single card mode
                            break;
                        case READ_REPORT_GEMINI_MODE_ENABLED:
                            boolean isAppendAnd = true;
                            for (int subId : totalSubIds) {
                                if (prefs.getBoolean(Long.toString(subId) + "_"
                                        + MmsPreferenceActivity.READ_REPORT_AUTO_REPLY, false)) {
                                    MmsLog.e(MmsApp.TXN_TAG, "readReport supported on subId: "
                                            + subId);
                                    // slot i has card and read report on
                                    if (isAppendAnd) {
                                        suffix.append(
                                                Telephony.Mms.SUBSCRIPTION_ID + " = " + subId);
                                        isAppendAnd = false;
                                    } else {
                                        suffix.append(" or " + Telephony.Mms.SUBSCRIPTION_ID
                                                + " = " + subId);
                                    }
                                } else {
                                    MmsLog.e(MmsApp.TXN_TAG,
                                            "mark subId " + subId + " card readed");
                                    markReadReportProcessed(ct, threadId, subId);
                                }
                            }
                            if (!TextUtils.isEmpty(suffix.toString())) {
                                suffix.insert(0, " and (");
                                suffix.append(") ");
                            }
                            break;
                        default:
                            MmsLog.e(MmsApp.TXN_TAG, "impossible value for rrAllowed.");
                            break;
                        }
                    boolean networkOk = true;
                    int airplaneMode = Settings.System.getInt(ct.getContentResolver(),
                            Settings.System.AIRPLANE_MODE_ON, 0);
                    //network not ok.next time will try.
                    if ((networkOk != true) || (airplaneMode == 1)) {
                        MmsLog.d(MmsApp.TXN_TAG, "networkok:"
                                + networkOk + ",airplaneMode:" + airplaneMode);
                        return;
                    }
                    Cursor cs = null;
                    try {
                        String where = Mms.THREAD_ID + " = " + threadId
                        + " and " + Mms.READ_REPORT + " = 128" + suffix.toString();
                        cs = SqliteWrapper.query(ct, ct.getContentResolver(),
                                Mms.Inbox.CONTENT_URI,
                                new String[]{Mms._ID, Telephony.Mms.SUBSCRIPTION_ID},
                                where, null, null);
                        if (cs != null) {
                            final int count = cs.getCount();
                            if (count > 0) {
                                //mark the ones need send read report status to pending as 130.
                                ContentValues values = new ContentValues(1);
                                values.put(Mms.READ_REPORT,
                                        MtkPduHeaders.READ_STATUS__DELETED_WITHOUT_BEING_READ);
                                // update uri inbox is not used, must indicate here.
                                where += " and " + Mms.MESSAGE_BOX + " = " + Mms.MESSAGE_BOX_INBOX;
                                SqliteWrapper.update(ct, ct.getContentResolver(),
                                        Mms.Inbox.CONTENT_URI, values, where, null);
                                //show a toast.
                                runOnUiThread(new Runnable() {
                                    public void run() {
                                        Toast.makeText(ComposeMessageActivity.this,
                                                    ct.getResources().getQuantityString(
                                                            R.plurals.read_report_toast_msg,
                                                            count, count),
                                                    Toast.LENGTH_SHORT).show();
                                    }
                                });
                            }
                            startSendReadReport(cs);
                        }
                    } catch (Exception e) {
                        MmsLog.e(MmsApp.TXN_TAG,
                                "exception happend when scan read report!:" + e.getMessage());
                    } finally {
                        if (cs != null) {
                            cs.close();
                        }
                    }
                }
            }

            private void markReadReportProcessed(Context ct, long threadId, int subId) {
                ContentValues values = new ContentValues(1);
                values.put(Mms.READ_REPORT, MtkPduHeaders.READ_STATUS__DELETED_WITHOUT_BEING_READ);
                String where = Mms.THREAD_ID + " = " + threadId
                        + " and " + Mms.READ_REPORT + " = 128"
                        + " and " + Telephony.Mms.SUBSCRIPTION_ID + " = " + subId;
                // update uri inbox is not used, must indicate here.
                where += " and " + Mms.MESSAGE_BOX + " = " + Mms.MESSAGE_BOX_INBOX;
                SqliteWrapper.update(ct, ct.getContentResolver(), Mms.Inbox.CONTENT_URI,
                                    values,
                                    where,
                                    null);
            }

            private void startSendReadReport(final Cursor cursor) {
                cursor.moveToPosition(-1);
                while (cursor.moveToNext()) {
                    MmsLog.d(MmsApp.TXN_TAG, "send an intent for read report.");
                    long msgId = cursor.getLong(0);
                    Intent rrIntent = new Intent();
                    rrIntent.setClassName(MmsConfig.MMS_APP_SERVICE_PACKAGE,
                            MmsConfig.TRANSACTION_SERVICE);
                    rrIntent.putExtra(TransactionBundle.URI, Mms.Inbox.CONTENT_URI + "/" + msgId);
                    rrIntent.putExtra(TransactionBundle.TRANSACTION_TYPE,
                           TransactionBundle.READREC_TRANSACTION);

                    int subId = cursor.getInt(1);
                    MmsLog.d(MmsApp.TXN_TAG, "subId:" + subId);
                    rrIntent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId);

                    ct.startService(rrIntent);
                    //AppServiceProxy.getInstance(ct).startServiceByParam(ct, rrIntent);
                }
            }
        }).start();
    }

    /**
     * Remove the number which is the same as any one before;
     * When the count of recipients over the limit, make a
     * toast and remove the recipients over the limit.
     * @param recipientsString the numbers slipt by ','.
     * @return recipientsString the numbers slipt by ',' after modified.
     */
    private String getStringForMultipleRecipients(String recipientsString) {
        recipientsString = recipientsString.replaceAll(",", ";");
        String[] recipients_all = recipientsString.split(";");
        List<String> recipientsList = new ArrayList<String>();
        for (String recipient : recipients_all) {
            recipientsList.add(recipient);
        }

        Set<String> recipientsSet = new HashSet<String>();
        recipientsSet.addAll(recipientsList);

        if (recipientsSet.size() > RECIPIENTS_LIMIT_FOR_SMS) {
            toastTooManyRecipients(recipients_all.length);
        }

        recipientsList.clear();
        recipientsList.addAll(recipientsSet);

        recipientsString = "";
        int count = recipientsList.size() > RECIPIENTS_LIMIT_FOR_SMS
                ? RECIPIENTS_LIMIT_FOR_SMS : recipientsList.size();
        StringBuffer buf = new StringBuffer();
        buf.append(recipientsString);
        for (int i = 0; i < count; i++) {
            if (i == (count - 1)) {
                buf.append(recipientsList.get(i));
            } else {
                buf.append(recipientsList.get(i) + ";");
            }
        }
        recipientsString = buf.toString();
        return recipientsString;
    }

    /**
     * Get Conversation.
     * * Notice: This function is also called by operator plugin(OP01). When modify this function,
     * please notify operator team.
     *
     * @return Conversation
     */
    public Conversation getConversation() {
        return mConversation;
    }

    /// M: Code analyze 014, Add quick text. @{
    private void showQuickTextDialog() {
        mQuickTextDialog = null;
        //if (mQuickTextDialog == null) {
            List<String> quickTextsList = new ArrayList<String>();

            /// M: new feature, add default quick text when frist "insert quick text" @{
            if (MmsConfig.getInitQuickText()) {
                String[] defaultQuickTexts =
                    getResources().getStringArray(R.array.default_quick_texts);
                for (int i = 0; i < defaultQuickTexts.length; i++) {
                    quickTextsList.add(defaultQuickTexts[i]);
                }
            } else {
                // add user's quick text
                //if (MmsConfig.getQuicktexts().size() == 0) {
                    MmsConfig.updateAllQuicktexts();
                //}
                quickTextsList = MmsConfig.getQuicktexts();
            }
            /// @}

            List<Map<String, ?>> entries = new ArrayList<Map<String, ?>>();
            for (String text : quickTextsList) {
                HashMap<String, Object> entry = new HashMap<String, Object>();
                entry.put("text", text);
                entries.add(entry);
            }

            final SimpleAdapter qtAdapter = new SimpleAdapter(this, entries,
                    R.layout.quick_text_list_item,
                    new String[] {"text"}, new int[] {R.id.quick_text});

            AlertDialog.Builder qtBuilder = new AlertDialog.Builder(this);

            qtBuilder.setTitle(getString(R.string.select_quick_text));
            qtBuilder.setCancelable(true);
            qtBuilder.setAdapter(qtAdapter, new DialogInterface.OnClickListener() {
                @SuppressWarnings("unchecked")
                public final void onClick(DialogInterface dialog, int which) {
                    HashMap<String, Object> item =
                        (HashMap<String, Object>) qtAdapter.getItem(which);
                    if (mSubjectTextEditor != null && mSubjectTextEditor.isFocused()) {
                        insertText(mSubjectTextEditor, (String) item.get("text"));
                    } else {
                        insertText(mTextEditor, (String) item.get("text"));
                    }
                    dialog.dismiss();
                }
            });
            mQuickTextDialog = qtBuilder.create();
        //}
        mQuickTextDialog.show();
    }
    /// @}

    /// M: Code analyze 004, Set max height for text editor. @{
    private final HeightChangedLinearLayout.LayoutSizeChangedListener mLayoutSizeChangedListener =
            new HeightChangedLinearLayout.LayoutSizeChangedListener() {
        private int mMaxHeight = 0;
        @Override
        public void onLayoutSizeChanged(int w, int h, int oldw, int oldh) {
            /// M: fix bug ALPS00419856, set TextEditor Height = four when unlock screen @{
            if (h - oldh > SOFT_KEY_BOARD_MIN_HEIGHT) {
                mIsSoftKeyBoardShow = false;
            } else {
                mIsSoftKeyBoardShow = true;
            }
            /// @}
            mMaxHeight = (h > mMaxHeight) ? h : mMaxHeight;
            if (h == oldh || mTextEditor == null || mTextEditor.getVisibility() == View.GONE) {
                return;
            }

            boolean resultIp = mIpCompose.onIpLayoutSizeChanged(mIsSoftKeyBoardShow);
            boolean resultOp = mOpComposeExt.onLayoutSizeChanged(mIsSoftKeyBoardShow);
            if (!mIsLandscape) {
                if (h > oldh && !resultIp && !resultOp) {
                    updateTextEditorHeightInFullScreen();
                } else {
                    mUiHandler.postDelayed(new Runnable() {
                        public void run() {
                            mTextEditor.setMaxHeight(mReferencedTextEditorFourLinesHeight
                                    * mCurrentMaxHeight / mReferencedMaxHeight);
                        }
                    }, 100);
                }
            }
            boolean isLandscape = getResources().getConfiguration().orientation
                    == Configuration.ORIENTATION_LANDSCAPE;
            boolean isMultiWindow = isInMultiWindowMode();
            MmsLog.d(TAG, "onLayoutSizeChanged(): isLandscape=" + isLandscape + "isMultiWindow=" + isMultiWindow);
            if (isLandscape && isMultiWindow) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        MmsLog.d(TAG, "onLayoutSizeChanged(): initFocus1");
                        initFocus();
                    }
                });
            }
        }
    };

    private void updateTextEditorHeightInFullScreen() {
        if (mIsLandscape || mTextEditor == null || mTextEditor.getVisibility() == View.GONE) {
            return;
        }
        mUiHandler.postDelayed(new Runnable() {
            public void run() {
                updateFullScreenTextEditorHeight();
            }
        }, 100);
    }

    private void updateFullScreenTextEditorHeight() {
        boolean resultIp = mIpCompose.updateIpFullScreenTextEditorHeight();
        boolean resultOp = mOpComposeExt.updateFullScreenTextEditorHeight();
        if (mAttachmentEditor.getVisibility() == View.VISIBLE
                && mAttachmentEditor.getHeight() > 0
                && !mWorkingMessage.hasSlideshow()) {
            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                MmsLog.d(TAG, "updateTextEditorHeight(): mTextEditor.setMaxHeight: "
                        + (mReferencedTextEditorSevenLinesHeight
                            * mCurrentMaxHeight / mReferencedMaxHeight));
            }
            if (mIsLandscape) {
                mTextEditor.setMaxHeight(
                        mReferencedTextEditorTwoLinesHeight
                        * mCurrentMaxHeight / mReferencedMaxHeight);
            } else {
                if (mIsSoftKeyBoardShow) {
                    mTextEditor.setMaxHeight(mReferencedTextEditorFourLinesHeight
                            * mCurrentMaxHeight / mReferencedMaxHeight);
                } else {
                    mTextEditor.setMaxHeight(mReferencedTextEditorSevenLinesHeight
                            * mCurrentMaxHeight / mReferencedMaxHeight);
                }
            }
        } else {
            /// M: fix bug ALPS00419856, set TextEditor Height = four when unlock screen @{
            if (mIsSoftKeyBoardShow && !mIsLandscape) {
                if (Log.isLoggable(TAG, Log.VERBOSE)) {
                    MmsLog.d(TAG, "updateFullScreenTextEditorHeight() updateTextEditorHeight()" +
                            ": mTextEditor.setMaxHeight: " + (mReferencedTextEditorFourLinesHeight
                                * mCurrentMaxHeight / mReferencedMaxHeight));
                }
                mTextEditor.setMaxHeight(mReferencedTextEditorFourLinesHeight
                        * mCurrentMaxHeight / mReferencedMaxHeight);
            /// @}
            } else if (!mIsSoftKeyBoardShow && !mIsLandscape) {
                if (Log.isLoggable(TAG, Log.VERBOSE)) {
                    MmsLog.v(TAG, "updateTextEditorHeight(): mTextEditor.setMaxHeight: "
                    + ((mReferencedTextEditorSevenLinesHeight + mReferencedAttachmentEditorHeight)
                                * mCurrentMaxHeight / mReferencedMaxHeight));
                }
                if (!resultIp && !resultOp) {
                    mTextEditor.setMaxHeight(
                            (mReferencedTextEditorSevenLinesHeight
                                    + mReferencedAttachmentEditorHeight)
                                    * mCurrentMaxHeight / mReferencedMaxHeight);
                }
            } else {
                if (Log.isLoggable(TAG, Log.VERBOSE)) {
                    MmsLog.v(TAG, "updateTextEditorHeight(): mTextEditor.setMaxHeight: "
                        + (mReferencedTextEditorTwoLinesHeight * mCurrentMaxHeight
                                / mReferencedMaxHeight));
                }
                mTextEditor.setMaxHeight(mReferencedTextEditorTwoLinesHeight * mCurrentMaxHeight
                            / mReferencedMaxHeight);
            }
        }
    }
    /// @}

   @Override
    public void startActivity(Intent intent) {
        try {
            super.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Intent mChooserIntent = Intent.createChooser(intent, null);
            super.startActivity(mChooserIntent);
        }
    }

    /**
     * Simple cache to prevent having to load the same PduBody again and again for the same uri.
     */

    private boolean needSaveDraft() {
        return ((!isRecipientsEditorVisible())
                    || (mRecipientsEditor.hasValidRecipient(mWorkingMessage.requiresMms())))
                && !mWorkingMessage.isDiscarded() && (mWorkingMessage.isWorthSaving());
    }

    public void onPreMmsSent() {
        startMsgListQuery(MESSAGE_LIST_QUERY_TOKEN, 0);
    }

    private boolean checkSlideCount(boolean append) {
        String mMsg = this.getString(R.string.cannot_add_slide_anymore);
        Toast mToast = Toast.makeText(this, mMsg, Toast.LENGTH_SHORT);
        int mSlideCount = 0;
        SlideshowModel slideShow = mWorkingMessage.getSlideshow();
        if (slideShow != null) {
            mSlideCount = slideShow.size();
        }
        if (mSlideCount >= SlideshowEditor.MAX_SLIDE_NUM && append) {
            mToast.show();
            return false;
        }
        return true;
    }

    /// M: Code analyze 009,Show attachment dialog . @{
    private class SoloAlertDialog extends AlertDialog {
        private AlertDialog mAlertDialog;

        private SoloAlertDialog(Context context) {
            super(context);
        }

        public boolean needShow() {
            return mAlertDialog == null || !mAlertDialog.isShowing();
        }

        public void show(final boolean append) {
            if (!needShow()) {
                return;
            }

            if (ComposeMessageActivity.this.isFinishing()
                    || ComposeMessageActivity.this.isDestroyed()) {
                MmsLog.d(TAG, "SoloAlertDialog.show, Activity finishing or destoryed");
                return;
            }
            AlertDialog.Builder builder = new AlertDialog.Builder(ComposeMessageActivity.this);
            builder.setIcon(R.drawable.ic_dialog_attach);
            builder.setTitle(R.string.add_attachment);

            // if (mAttachmentTypeSelectorAdapter == null) {
            // add for vcard, if there is a real slideshow, hide vCard
            int mode = AttachmentTypeSelectorAdapter.MODE_WITH_SLIDESHOW;
            if (mWorkingMessage.hasSlideshow()) {
                mode |= AttachmentTypeSelectorAdapter.MODE_WITHOUT_FILE_ATTACHMENT;
            } else {
                mode |= AttachmentTypeSelectorAdapter.MODE_WITH_FILE_ATTACHMENT;
            }
            if (MessageUtils.isVCalendarAvailable(ComposeMessageActivity.this)) {
                mode |= AttachmentTypeSelectorAdapter.MODE_WITH_VCALENDAR;
            }
            if (MessageUtils.isSoundRecorderAvailable(ComposeMessageActivity.this)) {
                mode |= AttachmentTypeSelectorAdapter.MODE_WITH_SOUNDRECORDER;
            }
            mAttachmentTypeSelectorAdapter = new AttachmentTypeSelectorAdapter(getContext(), mode);
            // }
            builder.setAdapter(mAttachmentTypeSelectorAdapter,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            addAttachment(
                                    mAttachmentTypeSelectorAdapter.buttonToCommand(which), append);
                            dialog.dismiss();
                        }
                    });
            mAlertDialog = builder.show();
        }

        public void dismiss() {
            if (mAlertDialog != null && mAlertDialog.isShowing()) {
                mAlertDialog.dismiss();
            }
            mAlertDialog = null;
            super.dismiss();
        }
    }
    /// @}

    private void waitForCompressing() {
        synchronized (ComposeMessageActivity.this) {
            while (mCompressingImage) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    MmsLog.e(TAG, "intterrupted exception e ", e);
                }
            }
        }
    }

    private void notifyCompressingDone() {
        synchronized (ComposeMessageActivity.this) {
            mCompressingImage = false;
            notify();
        }
    }

    private boolean mCompressingVCardFinished = false;

    /// M: change return value to int. if attachFile.length is 0,will
    /// return WorkingMessage.UNKNOWN_ERROR
    /// The return value will be used in asyncAttachVCardByContactsId
    /// to judge wether need to saveAsMms or not
    private int setFileAttachment(final String fileName, final int type, final boolean append) {
        final File attachFile = getFileStreamPath(fileName);
        MmsLog.d(TAG, "setFileAttachment(): attachFile.exists()?=" + attachFile.exists() +
                        ", attachFile.length()=" + attachFile.length());
        final Resources res = getResources();
        if (attachFile.exists() && attachFile.length() > 0) {
            Uri attachUri = Uri.fromFile(attachFile);
            int result = WorkingMessage.OK;
            try {
                if (append) {
                    mWorkingMessage.checkSizeBeforeAppend();
                }
            } catch (ExceedMessageSizeException e) {
                result = WorkingMessage.MESSAGE_SIZE_EXCEEDED;
                handleAddAttachmentError(result, R.string.type_common_file);
                return result;
            }
            result = mWorkingMessage.setAttachment(type, attachUri, append);
            handleAddAttachmentError(result, R.string.type_common_file);
            mCompressingVCardFinished = true;
            return result;
        } else {
            mUiHandler.post(new Runnable() {
                public void run() {
                    Toast.makeText(ComposeMessageActivity.this,
                            res.getString(R.string.failed_to_add_media, fileName),
                            Toast.LENGTH_SHORT).show();
                }
            });
            return WorkingMessage.UNKNOWN_ERROR;
        }
    }

    public void setCompressingVCardFinished(boolean compressingVCardFinished) {
        mCompressingVCardFinished = compressingVCardFinished;
    }

    private void asyncAttachVCalendar(final Uri eventUri) {
        if (eventUri == null) {
            return;
        }
        getAsyncDialog().runAsync(new Runnable() {
            public void run() {
                attachVCalendar(eventUri);
                /// call saveAsMms to save the draft @{
                mWorkingMessage.saveAsMms(false);
                /// @}
            }
        }, null, R.string.adding_attachments_title);
    }

    private void attachVCalendar(Uri eventUri) {
        if (eventUri == null) {
            Log.w(TAG, "attachVCalendar, oops uri is null");
            return;
        }
        int result = WorkingMessage.OK;

        /// M: OP01
        if (!mOpComposeExt.attachVCalendar(mAppendAttachmentSign,
                WorkingMessage.VCALENDAR, eventUri, R.string.type_common_file)) {
            result = mWorkingMessage.setAttachment(WorkingMessage.VCALENDAR, eventUri, false);
            handleAddAttachmentError(result, R.string.type_common_file);
        }
        /// @}
    }

    /// M: Code analyze 019, Add vcard attachment.  @{
    private void asyncAttachVCardByContactsId(final Intent data) {
        if (data == null) {
            return;
        }
        getAsyncDialog().runAsyncInThreadPool(new Runnable() {
            public void run() {
                long[] contactsId =
                    data.getLongArrayExtra("com.mediatek.contacts.list.pickcontactsresult");
                VCardAttachment va = new VCardAttachment(ComposeMessageActivity.this);
                int result = WorkingMessage.OK;
                String fileName = va.getVCardFileNameByContactsId(contactsId);
                /// M: add for attachmentEnhance Modify ALPS00474719 @{
                if (mOpComposeExt.asyncAttachVCardByContactsId(mAppendAttachmentSign)) {
                    // add vcard and OP01
                    result = setFileAttachment(fileName, WorkingMessage.VCARD, true);
                } else {
                    // replace or Not OP01
                    result = setFileAttachment(fileName, WorkingMessage.VCARD, false);
                }

                /// Fix CR : ALPS00970618
                /// Fix CR : ALPS01035223 Add VCard with 1000 contacts cause exceed message
                /// limitation the result should be OK, that saveAsMms or there will be errors @{
                if (result == WorkingMessage.OK) {
                    Log.d("[Mms][Composer]", "asyncAttachVCardByContactsId result is ok");
                    /// Fix CR ALPS01011718 @{
                    if (mCompressingVCardFinished && mWorkingMessage.isDiscarded()) {
                        mWorkingMessage.unDiscard();
                    }
                    /// @}
                    mWorkingMessage.saveAsMms(false);
                }
                /// @}
                VCardUtils.deleteVCardTempFile(getApplicationContext(), fileName);
            }
        }, null, R.string.adding_attachments_title);
    }
    /// @}

    /// M: Code analyze 036, Change text size if adjust font size.@{
    public void setTextSize(float size) {
        if (mTextEditor != null) {
            mTextEditor.setTextSize(size);
        }
        if (mMsgListAdapter != null) {
            mMsgListAdapter.setTextSize(size);
        }

        if (mMsgListView != null && mMsgListView.getVisibility() == View.VISIBLE) {
            int count = mMsgListView.getChildCount();
            for (int i = 0; i < count; i++) {
                MessageListItem item =  (MessageListItem) mMsgListView.getChildAt(i);
                if (item != null) {
                    item.setBodyTextSize(size);
                }
            }
        }
    }
    /// @}

    public boolean  dispatchTouchEvent(MotionEvent ev) {
        boolean ret = false;
        /// M: Code analyze 001, Plugin opeartor. @{
        if (mOpComposeExt != null) {
            ret = mOpComposeExt.dispatchTouchEvent(ev);
        }
        /// @}

        if (!ret) {
            ret = super.dispatchTouchEvent(ev);
        }
        return ret;
    }

    private void showKeyBoard(boolean isShow) {
        if (isShow && mIsSmsEnabled) {
            mTextEditor.requestFocus();
            mInputMethodManager.showSoftInput(mTextEditor, 0);
            mIsKeyboardOpen = true;
        } else {
            hideInputMethod();
        }
    }

    private long genThreadIdFromContacts(Intent data) {
        final long[] contactsId = data
                .getLongArrayExtra("com.mediatek.contacts.list.pickdataresult");
        if (contactsId == null || contactsId.length <= 0) {
            Log.d(TAG, "[genThreadIdFromContacts] get recipients from contacts is null");
            return 0;
        }
        final ContactList aList = new ContactList();
        final ContactList list = new ContactList();
        ContactList selected = ContactList.blockingGetByIds(contactsId);
        List<String> selectedNumbers = Arrays.asList(selected.getProtosomaitcNumbers());
        final List<String> numbers = Arrays.asList(mConversation.getRecipients()
                .getProtosomaitcNumbers());
        String selectedNumberAfterFormat = "";
        if (numbers.size() > 0) {
            for (String number : numbers) {
                if (!number.trim().equals("")) {
                    Contact c = Contact.get(number, false);
                    aList.add(c);
                }
            }
            // / M: format existing numbers(remove "-" and " ")
            List<String> formatedNumbers = Arrays.asList(aList.getNumbers(true));
            for (String selectedNumber : selectedNumbers) {
                selectedNumberAfterFormat = MessageUtils.parseMmsAddress(selectedNumber);
                if (selectedNumberAfterFormat != null
                        && !selectedNumberAfterFormat.trim().equals("")
                        && !formatedNumbers.contains(selectedNumberAfterFormat)) {
                    Contact c = Contact.get(selectedNumber, false);
                    list.add(c);
                }
            }
            aList.addAll(list);
        }
        if (aList == null || aList.size() <= 0) {
            return 0;
        }
        long id = Conversation.getOrCreateThreadId(getApplicationContext(), aList);
        return id;
    }

    private boolean mIsStartMultiDeleteActivity = false;

    private void updateTextEditorHint() {
        // add for ipmessage
        if (mIpCompose.onIpUpdateTextEditorHint()) {
            return;
        }
        if (mIsSmsEnabled) {
            mTextEditor.setHint(R.string.ipmsg_sms_hint);
        } else {
            mTextEditor.setHint(R.string.sending_disabled_not_default_app);
        }
        mTextEditor.setFilters(new InputFilter[] {
            new TextLengthFilter(MmsConfig.getMaxTextLimit())
        });
        updateCounter(mWorkingMessage.getText(), 0, 0, 0);
    }

    private String formatLastOnlineTime(int time) {
        return MessageUtils.formatTimeStampString(this, (long) (time * 1000L), true);
    }

    private void checkBeforeSendMessage(boolean bCheckEcmMode) {
        boolean needSendIpMessage =
            mIpCompose.checkIpMessageBeforeSendMessage(mSelectedSubId, bCheckEcmMode);
        if (!needSendIpMessage) {
            sendMessage(bCheckEcmMode);
        }
    }

    /// M:added for bug ALPS00317889
    private boolean mShowDialogForMultiImage = false;

    /// M: show contact detail or create new contact. @{
    private static final int MENU_SHOW_CONTACT          = 121;
    private static final int MENU_CREATE_CONTACT        = 122;
    private QuickContactBadge mQuickContact;
    /// @}

    /// M: fix bug ALPS00414023, update sub state dynamically. @{
    private BroadcastReceiver mSubReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (action != null &&
                    (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED) ||
                    action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED))) {
                    /// M: fix bug ALPS00429274, dismiss Dialog when SUB_CHANGED @{
                    if (mSubSelectDialog != null && mSubSelectDialog.isShowing()) {
                        mSubSelectDialog.dismiss();
                    }
                    /// @}
                    /// M: Fix Bug: ALPS00503002 ;
                    getSubInfoList();
                    updateSendButtonState();
                    mMsgListAdapter.notifyDataSetChanged();
            } else if (action != null
                    && (action.equals(TelephonyIntents.ACTION_DEFAULT_SMS_SUBSCRIPTION_CHANGED)
                        || action.equals(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE))) {
                mGetSubInfoRunnable.run();
            }
        }
    };

    private void saveAsMms(boolean notify) {
        if (!MessageUtils.allowSafeDraft(this, MmsConfig.getDeviceStorageFullStatus(), false,
                TOAST_TYPE_FOR_SAVE_DRAFT)) {
            return;
        }
        if (mNeedSaveAsMms) {
            mNeedSaveAsMms = false;
            if (mWorkingMessage.saveAsMms(notify) != null) {
                mHasDiscardWorkingMessage = true;
            }
        }
    }

    /// M: after delete the last message of one thread,
    /// don't finish this activity if have draft. @{
    private void makeDraftEditable(ContactList recipients) {
        if (!mConversation.getRecipients().equals(recipients)) {
            mConversation.setRecipients(recipients);
            MmsLog.d(TAG, "makeDraftEditable, do not equal");
        } else {
            MmsLog.d(TAG, "makeDraftEditable, equal");
            mWorkingMessage.asyncDeleteDraftSmsMessage(mConversation);
            mConversation.clearThreadId();
        }
        mWorkingMessage.setConversation(mConversation);
        updateThreadIdIfRunning();
        invalidateOptionsMenu();
        hideRecipientEditor();
        initRecipientsEditor(null);
        isInitRecipientsEditor = true;
    }
    /// @}

    private static final int CELL_PROGRESS_DIALOG = 1;

    @Override
    protected Dialog onCreateDialog(int id) {
        //add for ipmessage
        Dialog dlg = mIpCompose.onIpCreateDialog(id);
        if (dlg != null) {
            return dlg;
        }

        ProgressDialog dialog = null;
        if (id == CELL_PROGRESS_DIALOG) {
            dialog = new ProgressDialog(ComposeMessageActivity.this);
            // mProgressDialog.setTitle(getText(titleResId));
            dialog.setMessage(getString(R.string.sum_search_networks));
            dialog.setIndeterminate(true);
            dialog.setCancelable(false);
            dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
            // / M: fix bug ALPS00451836, remove FLAG_DISMISS_KEYGUARD flags
            if (getWindow() != null) {
                getWindow().clearFlags(
                        WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD
                                | WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);
            }
        }
        return dialog;
    }

    /// M: fix bug ALPS00941735, set Obsolete ThreadId and except it when ConversationList query
    long mOldThreadID = -1;

    boolean mIsCheckObsolete = false;

    boolean mIsSameConv = true;

    private void checkObsoleteThreadId() {
        if (isRecipientsEditorVisible()) {
            List<String> numbers =  mRecipientsEditor.getNumbers();
            if (numbers != null && numbers.size() > 0) {
                if (mOldThreadID > 0 && mIsCheckObsolete
                    && mIsSameConv && !mWorkingMessage.isDiscarded()) {
                    mIsCheckObsolete = false;
                    mWorkingMessage.syncWorkingRecipients();
                    long newThreadId = mWorkingMessage.getConversation().ensureThreadId();
                    if (mOldThreadID != newThreadId) {
                        DraftManager.sEditingThread.add(mOldThreadID);
                        MmsLog.d(TAG, "new recipent list != old, old Obsolete thread id = "
                            + mOldThreadID);
                    }
                }
            }
        }
    }

    /*
     * M: reload working message when working message is not correct.
     */
    private void reloadWorkingMessage() {
        if (mTempMmsUri == null) {
            return;
        }
        WorkingMessage newMessage = WorkingMessage.load(this, mTempMmsUri);

        if (newMessage != null) {
            if (newMessage.hasMediaAttachments()) {
                newMessage.removeAllFileAttaches();
            }
            boolean isMmsBefore = mWorkingMessage.requiresMms();
            newMessage.setSubject(mWorkingMessage.getSubject(), false);

            mWorkingMessage = newMessage;
            updateThreadIdIfRunning();

            boolean isMmsAfter = mWorkingMessage.requiresMms();
            if (!isMmsAfter && !isMmsBefore) {
                mWorkingMessage.setForceUpdateThreadId(true);
            }
        }
    }

    /**
     * M: For Just not send length required mms with Slot one.<br>
     * This function:check the mms is whether legnth required mms.
     *
     * @return
     */
    public boolean isLengthRequiredMms() {
        MmsLog.d(TAG, "isLengthRequiredMms Checked");
        if (mWorkingMessage == null) {
            return false;
        }
        if (!mWorkingMessage.requiresMms()) {
            return false;
        }
        if (mWorkingMessage.hasSubject()) {
            return false;
        }
        if (mWorkingMessage.hasAttachedFiles()) {
            return false;
        }
        if (mWorkingMessage.hasAttachment()) {
            return false;
        }
        if ((mWorkingMessage.getState() & 1) > 0) {
            return false;
        }
        SlideshowModel ssm = mWorkingMessage.getSlideshow();
        if (ssm != null && ssm.size() != 1) {
            return false;
        } else if (ssm != null && ssm.get(0) != null && !ssm.get(0).hasText()) {
            return false;
        }
        int slotId = SubscriptionManager.from(MmsApp.getApplication())
                .getActiveSubscriptionInfo(mSelectedSubId).getSimSlotIndex();
        if (slotId != 0) {
            return false;
        }
        if (!TelephonyManager.getDefault().isNetworkRoaming(mSelectedSubId)) {
            return false;
        }
        if (ssm == null && mWorkingMessage.hasText()) {
            return true;
        }
        return true;
    }

    /**
     * M: For Just not send length required mms with Slot one.<br>
     */
    public void confirmForChangeMmsToSms() {
        if (!isLengthRequiredMms()) {
            MmsLog.d(TAG, "isLengthRequiredMms Checked false");
            confirmSendMessageIfNeeded();
            return;
        }
        MmsLog.d(TAG, "isLengthRequiredMms Checked true");
        new AlertDialog.Builder(this)
            .setTitle("")
            .setIconAttribute(android.R.attr.alertDialogIcon)
            .setPositiveButton(R.string.yes, new OnClickListener() {
                @Override
                public void onClick(DialogInterface arg0, int arg1) {
                    mWorkingMessage.setLengthRequiresMms(false, false);
                    confirmSendMessageIfNeeded();
                }
            }).setNegativeButton(R.string.no, new OnClickListener() {
                @Override
                public void onClick(DialogInterface arg0, int arg1) {
                    mWorkingMessage.setLengthRequiresMms(true, false);
                    updateSendButtonState();
                }
            }).setCancelable(true).show();
    }

    /// M: Whether attachment is being added or not. @{
    private boolean mWaitingAttachment = false;
    public boolean isWaitingAttachment() {
        return mWaitingAttachment;
    }

    public void setWaitingAttachment(boolean waitingAttachment) {
        mWaitingAttachment = waitingAttachment;
    }
    /// @}

    /// M: add for CMCC feature fix ALPS01317511 @{
    public boolean isWorthSaving() {
        boolean ret = false;
        if (mWorkingMessage != null) {
            ret = mWorkingMessage.isWorthSaving();
        }
        MmsLog.d(TAG, "isWorthSaving: ret = " + ret);
        return ret;
    }

    public boolean hasValidRecipient() {
        boolean ret = true;
        if (isRecipientsEditorVisible()
                && !mRecipientsEditor.hasValidRecipient(mWorkingMessage.requiresMms())) {
            ret = false;
        }
        MmsLog.d(TAG, "hasValidRecipient: ret = " + ret);
        return ret;
    }
    /// @}


    /**
     * Get Ip conversation for ip plugin.
     * Notice: This function is also called by operator plugin(OP01). When modify this function,
     * please notify operator team.
     *
     * @return IIpConversationExt
     */
    public IIpConversationExt getIpConversation() {
        return mConversation.mIpConv;
    }

    /*
     * M: ALPS01956117. If image is compressing after onStop,
     * async save draft operation should be delayed.
     */
    private boolean mNeedSaveDraftAfterStop = false;

    public class ComposeActivityCallback implements IComposeActivityCallback {

        public void convertIpMessageToMmsOrSms(int type, boolean isAppend, String path,
                String text, int subId) {
            switch (type) {
                case IpMessageUtils.IpMessageType.TEXT:
                    mWorkingMessage.setText(text);
                    break;
                case IpMessageUtils.IpMessageType.PICTURE:
                    if (!TextUtils.isEmpty(text)) {
                        mWorkingMessage.setText(text);
                        if (mTextEditor != null && mTextEditor.getVisibility() == View.VISIBLE) {
                            mTextEditor.setText(text);
                        }
                    }
                    File imageFile = new File(path);
                    Uri imageUri = Uri.fromFile(imageFile);
                    addImage(imageUri, isAppend);
                    saveAsMms(true);
                    break;
                case IpMessageUtils.IpMessageType.VOICE:
                    if (!TextUtils.isEmpty(text)) {
                        mWorkingMessage.setText(text);
                        if (mTextEditor != null && mTextEditor.getVisibility() == View.VISIBLE) {
                            mTextEditor.setText(text);
                        }
                    }
                    File voiceFile = new File(path);
                    Uri voiceUri = Uri.fromFile(voiceFile);
                    addAudio(voiceUri, isAppend);
                    saveAsMms(true);
                    break;
                case IpMessageUtils.IpMessageType.VCARD:
                    File vCardFile = new File(path);
                    Uri vCardUri = Uri.fromFile(vCardFile);
                    VCardAttachment va = new VCardAttachment(ComposeMessageActivity.this);
                    String fileName = va.getVCardFileNameByUri(vCardUri);
                    setFileAttachment(fileName, WorkingMessage.VCARD, false);
                    saveAsMms(true);
                    VCardUtils.deleteVCardTempFile(getApplicationContext(), fileName);
                    break;
                case IpMessageUtils.IpMessageType.VIDEO:
                    if (!TextUtils.isEmpty(text)) {
                        mWorkingMessage.setText(text);
                        if (mTextEditor != null && mTextEditor.getVisibility() == View.VISIBLE) {
                            mTextEditor.setText(text);
                        }
                    }
                    File videoFile = new File(path);
                    Uri videoUri = Uri.fromFile(videoFile);
                    addVideo(videoUri, isAppend);
                    saveAsMms(true);
                    break;
                case IpMessageUtils.IpMessageType.CALENDAR:
                    File vCalendarFile = new File(path);
                    Uri vCalendarUri = Uri.fromFile(vCalendarFile);
                    attachVCalendar(vCalendarUri);
                    saveAsMms(true);
                    break;
                default:
                    break;
            }
            if (subId > 0) {
                mSelectedSubId = subId;
            }
        }

        public long getCurrentThreadId() {
            return mConversation.getThreadId();
        }

        public void deleteDraftSmsMessage() {
            mWorkingMessage.asyncDeleteDraftSmsMessage(mConversation);
        }

        @Override
        public String getRecipientsEditorInfo() {
            if (mRecipientsEditor != null && isRecipientsEditorVisible()) {
                if (mRecipientsEditor.getNumbers().size() == 1) {
                    return mRecipientsEditor.getNumbers().get(0);
                }
            }
            return null;
        }

        @Override
        public List<String> getRecipientsEditorInfoList() {
            if (mRecipientsEditor != null && isRecipientsEditorVisible()) {
                return mRecipientsEditor.getNumbers();
            }
            return null;
        }

        @Override
        public String[] getConversationInfo() {
            if (mConversation != null && mConversation.getRecipients() != null) {
                return mConversation.getRecipients().getNumbers();
            }
            return null;
        }

        public IIpConversationExt getIpConversation() {
            return mConversation.mIpConv;
        }

        public void callbackCheckConditionsAndSendMessage(boolean bCheckEcmMode) {
            checkConditionsAndSendMessage(true);
        }

        public void callbackUpdateSendButtonState() {
            updateSendButtonState();
        }

        public void callbackUpdateButtonState(boolean canResponse) {
            mSendButtonCanResponse = canResponse;
        }

        public void callbackSendMessage(boolean bCheckEcmMode) {
            sendMessage(bCheckEcmMode);
        }

        public void asyncDeleteDraftSmsMessage() {
            mWorkingMessage.asyncDeleteDraftSmsMessage(mConversation);
        }

        public void addIpContacts(int pickCount, int requestCode) {
            addContacts(pickCount, requestCode);
        }

        public long genIpThreadIdFromContacts(Intent data) {
            return genThreadIdFromContacts(data);
        }

        public void setIpOnlineDividerString(String onLine) {
            mMsgListAdapter.setOnlineDividerString(onLine);
        }

        public void updateIpOnlineDividerTime() {
            mMsgListAdapter.updateOnlineDividerTime();
        }

        public void notifyIpDataSetChanged() {
            mMsgListAdapter.notifyDataSetChanged();
        }

        public void invalidateIpOptionsMenu() {
            invalidateOptionsMenu();
        }

        public View showIpOrMmsSendButton(boolean isMms) {
            return showSmsOrMmsSendButton(isMms);
        }

        public void setClearIpCacheFlag(boolean clearCacheFlag) {
            mMsgListAdapter.setClearCacheFlag(clearCacheFlag);
        }

        public void setIpDraftState(boolean hasDraft) {
            mConversation.setDraftState(hasDraft);
        }

        public void runIpAsyncInThreadPool(final Runnable backgroundTask,
                final Runnable postExecuteTask, final int dialogStringId) {
            mAsyncDialog.runAsyncInThreadPool(backgroundTask, postExecuteTask, dialogStringId);
        }

        public void syncIpWorkingRecipients() {
            mWorkingMessage.syncWorkingRecipients();
        }

        public void guaranteeIpThreadId() {
            mConversation.guaranteeThreadId();
        }

        public void onPreIpMessageSent() {
            onPreMessageSent();
        }

        public void resetIpMessage() {
            resetMessage();
        }

        public void onIpMessageSent() {
            onMessageSent();
        }

        public void onIpClick(View view) {
            onClick(view);
        }

        public boolean isIpSubjectEditorVisible() {
            return isSubjectEditorVisible();
        }

        public void showIpSubjectEditor(boolean isShow) {
            showSubjectEditor(isShow);
        }

        public void drawIpBottomPanel() {
            drawBottomPanel();
        }

        public void hideIpSharePanel() {
        }

        public void addIpAttachment(int commonAttachmentType, boolean append) {
            if (append) {
                addAttachment(commonAttachmentType, append);
            } else {
                addAttachment(commonAttachmentType, !mWorkingMessage.hasAttachedFiles());
            }
        }

        public void enableShareButton(boolean enable) {
        }

        public void resetSharePanel() {
        }

        public IWorkingMessageCallback getWorkingMessage() {
            return mWorkingMessage;
        }

        public void hideIpRecipientEditor() {
            hideRecipientEditor();
        }

        public void onIpSubSelection() {
            subSelection();
        }

        public boolean isIpRecipientEditorVisible() {
            return isRecipientsEditorVisible();
        }

        public void updateIpTitle() {
            updateTitle(mConversation.getRecipients());
        }

        public void hideIpInputMethod() {
            hideInputMethod();
        }

        public void showAddAttachmentDialog(final boolean append) {
            ComposeMessageActivity.this.showAddAttachmentDialog(append);
        }

        public boolean checkSizeBeforeAppend() {
            try {
                mWorkingMessage.checkSizeBeforeAppend();
            } catch (ExceedMessageSizeException e) {
                int result = WorkingMessage.MESSAGE_SIZE_EXCEEDED;
                handleAddAttachmentError(result, R.string.type_common_file);
                return false;
            }
            return true;
        }

        public int setAttachment(int type, Uri uri, boolean append) {
            return mWorkingMessage.setAttachment(type, uri, append);
        }

        public void handleAddAttachmentError(int result, int mediaTypeStringId) {
            ComposeMessageActivity.this.handleAddAttachmentError(result, mediaTypeStringId);
        }

        public void checkRecipientsCountCallback() {
            checkRecipientsCount();
        }

        public void confirmForChangeMmsToSmsCallback() {
            confirmForChangeMmsToSms();
        }

        public void goToConversationListCallback() {
            goToConversationList();
        }

        public void setHomeBox(int box) {
            /* P0 migration*/
            //mHomeBox = box;
        }

        public void setHadToSlideShowEditor(boolean hadToSlideShowEditor) {
            mHadToSlideShowEditor = hadToSlideShowEditor;
        }

        public void setWorkingMessage(IWorkingMessageCallback workingMessage) {
            mWorkingMessage = (WorkingMessage) workingMessage;
        }

        public void updateThreadIdIfRunningCallback() {
            updateThreadIdIfRunning();
        }

        public void drawTopPanelCallback(boolean showSubjectEditor) {
            drawTopPanel(showSubjectEditor);
        }

        public void processNewIntentCallback(Intent intent) {
            processNewIntent(intent);
        }

        public void updateCounterCallback(CharSequence text, int start, int before, int count) {
            updateCounter(text, start, before, count);
        }

        public boolean isPreparedForSendingCallback() {
            return isPreparedForSending();
        }

        public void setIsPickContatct(boolean isPickContatct) {
            misPickContatct = isPickContatct;
        }

        public void updateSendButtonStateCallback(boolean enabled) {
            updateSendButtonState(enabled);
        }

        public void updateTextEditorHintCallback() {
            updateTextEditorHint();
        }

        public void setSendButtonCanResponse(boolean sendButtonCanResponse) {
            mSendButtonCanResponse = sendButtonCanResponse;
        }

        public boolean getSendButtonCanResponse() {
            return mSendButtonCanResponse;
        }

        public void callbackCheckBeforeSendMessage(boolean bCheckEcmMode) {
            checkBeforeSendMessage(bCheckEcmMode);
        }

        public boolean isHasRecipientCountCallback() {
            return isHasRecipientCount();
        }

        public void requestRecipientsEditorFocus() {
            if (isRecipientsEditorVisible()) {
                mRecipientsEditor.requestFocus();
            }
        }

        public CharSequence getTextEditorText() {
            return mTextEditor.getText();
        }

        public void hideOrShowTopPanelCallback() {
            hideOrShowTopPanel();
        }

        public boolean getIsSmsEnabled() {
            return mIsSmsEnabled;
        }

        public OnCreateContextMenuListener getOnCreateContextMenuListener() {
            return mRecipientsMenuCreateListener;
        }

        public String getNumbersFromIntentCallback(Intent intent) {
            return getNumbersFromIntent(intent);
        }

        public void confirmSendMessageIfNeededCallback() {
            confirmSendMessageIfNeeded();
        }

        public void setSubIdCallback(int subId) {
            setSubId(subId);
        }

        public void updateFullScreenTextEditorHeightCallback() {
            updateFullScreenTextEditorHeight();
        }

        public void showKeyBoardCallback(boolean isShow) {
            showKeyBoard(isShow);
        }

        public void setTextEditorMaxHeight(int linesHeight) {
            mTextEditor.setMaxHeight(linesHeight * mCurrentMaxHeight / mReferencedMaxHeight);
        }

        public void unpreparedForSendingAlertCallback() {
            unpreparedForSendingAlert();
        }

        public boolean showWfcSendButtonPopUpCallback() {
            return showWfcSendButtonPopUp();
        }
    }

    public void setCanResponse(boolean canResponse) {
        mClickCanResponse = canResponse;
    }

    ///M: WFC: Show pop-up @ {
    private boolean showWfcSendButtonPopUp() {
        if (MessagingNotification.doShowWfcPopup(mSelectedSubId, getApplicationContext())) {
            new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.wfc_no_cellular_network)
                .setMessage(R.string.wfc_connect_to_available_wifi)
                .setPositiveButton(R.string.OK, new OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        dialog.dismiss();
                    }
            }).show();
            return true;
        }
        return false;
    }
   /// @}

    @Override
    public void onRequestPermissionsResult(
            final int requestCode, final String permissions[], final int[] grantResults) {
        Log.d(TAG, " onRequestPermissionsResult Activity Count: "
                + PermissionCheckUtil.sPermissionsActivityStarted);
        PermissionCheckUtil.setPermissionActivityCount(false);
        if (isAllPermissionGranted()) {
            mNeedRequestPermissins = false;

            //onCreate begin
            mForwardingMessage = false;

            /// M: If cell dialog is showed before, do not show it. @{
            if (mSavedInstanceState != null
                    && mSavedInstanceState.getBoolean("cell_dialog_showing", false)) {
                MmsLog.d(TAG, "cell progress dialog is showed before, do not show it now");
                mSavedInstanceState.putBundle("android:savedDialogs", null);
            }
            /// @}

            // add for ipmessage
            mIpCompose = IpMessageUtils.getIpMessagePlugin(this).getIpComposeActivity();
            // add for op plugin
            mOpComposeExt = OpMessageUtils.getOpMessagePlugin().getOpComposeExt();

            mMessageSubId = (int) MmsConfig.DEFAULT_SIM_NOT_SET;

            mInputMethodManager =
                (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);

            /// M: Code analyze 002,  If a new ComposeMessageActivity is created, kill old one
            Activity compose = sCompose == null ? null : sCompose.get();
            /// M Fix CR : ALPS01275735, which has mms draft for conversation A, enter it.
            /// change recipients froam A to B, then come a B notification , click the notification,
            /// enter the thread A, it will show wrong composer UI.
            /// because conversation from cache is the same. @{
            if (compose != null && compose instanceof ComposeMessageActivity) {
                Intent in = this.getIntent();
                /// M : Fix issue ALPS01767850 @{
                if (in.getBooleanExtra("from_notification", false)) {
                    boolean b = ((ComposeMessageActivity) compose).hasValidRecipient();
                    if (b) {
                        mLastThreadIdFromNotification
                                = ((ComposeMessageActivity) compose)
                                    .mConversation.ensureThreadId();
                        if (!((ComposeMessageActivity) compose).mWorkingMessage.hasAttachment()) {
                            sTextEditorText
                                    = ((ComposeMessageActivity) compose)
                                            .mWorkingMessage.getText().toString();
                        } else {
                            sTextEditorText = null;
                        }
                    }
                }
                /// @}
                long thid = in.getLongExtra("thread_id_from_notification", -1);
                if (((ComposeMessageActivity) compose).mConversation != null) {
                    if (thid == ((ComposeMessageActivity) compose).mConversation.getThreadId()) {
                        ((ComposeMessageActivity) compose).mConversation.clearThreadId();
                    }
                }
            }
            /// @}

            if (compose != null && !compose.isFinishing() && mSavedInstanceState == null) {
                MmsLog.d(TAG, "onCreate finish old compose");
                compose.finish();
            }
            sCompose = null;
            sCompose = new WeakReference(this);
            /// @}
            /// M: Code analyze 003,  Set or get max mms size.
            initMessageSettings();
            /// @}
            resetConfiguration(getResources().getConfiguration());
            /// M: Code analyze 004, Set max height for text editor. @{
            WindowManager windowManager = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
            if (mIsLandscape) {
                mCurrentMaxHeight = windowManager.getDefaultDisplay().getWidth();
            } else {
                mCurrentMaxHeight = windowManager.getDefaultDisplay().getHeight();
            }
            /// @}
            setContentView(R.layout.compose_message_activity);
            setProgressBarVisibility(false);

            mIpCompose.onIpComposeActivityCreate(this, mIpComposeCallback, mHandler, mUiHandler,
                    (ImageButton) findViewById(R.id.send_button_sms),
                    (TextView) findViewById(R.id.ip_message_typing_status),
                    (TextView) findViewById(R.id.ipmsg_joyn_stranger_remind),
                    findViewById(R.id.bottom_panel), mSavedInstanceState,
                    (ImageButton) findViewById(R.id.share_button),
                    (LinearLayout) findViewById(R.id.panel_container),
                    (EditText) findViewById(R.id.embedded_text_editor));

            /// M: add for update sub state dynamically. @{
            IntentFilter intentFilter =
                new IntentFilter(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
            intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
            mOpComposeExt.onCreate(mIpComposeCallback, getIntent(), intentFilter, compose, this,
                    mSavedInstanceState, mUiHandler, (ImageButton) findViewById(R.id.share_button),
                    (LinearLayout) findViewById(R.id.panel_container),
                    (EditText) findViewById(R.id.embedded_text_editor));
            /// @}

            /// M: Code analyze 005, Set input mode. @{
            getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
            /// @}

            // Initialize members for UI elements.
            initResourceRefs();
            /// M: Code analyze 006, Control Sub indicator on status bar. @{
            mStatusBarManager = (StatusBarManager) getSystemService(Context.STATUS_BAR_SERVICE);
            mComponentName = getComponentName();
            /// @}

            /// M: Code analyze 007, Get information from Sub or save message to Sub. @{
            mSubCount = 0;
            /// @}

            mContentResolver = getContentResolver();
            mBackgroundQueryHandler = new BackgroundQueryHandler(mContentResolver);

            initialize(mSavedInstanceState, 0, true);

            if (TRACE) {
                android.os.Debug.startMethodTracing("compose");
            }

            /// M: Code analyze 009,Show attachment dialog . @{
            mSoloAlertDialog = new SoloAlertDialog(this);
            /// @}
            /// M: Code analyze 007, Get information from Sub or
            /// save message to Sub.(Get all Sub info) @{
            mGetSubInfoRunnable.run();
            /// @} for ALPS00475359 end

            this.registerReceiver(mSubReceiver, intentFilter);

            IntentFilter shutDownFilter = new IntentFilter();
            shutDownFilter.addAction(Intent.ACTION_SHUTDOWN);
            this.registerReceiver(mShutDownReceiver, shutDownFilter);
            /// @}

            //onStart begin
            mIsActivityStoped = false;
            boolean isSmsEnabled = MmsConfig.isSmsEnabled(this);
            if (isSmsEnabled != mIsSmsEnabled) {
                mIsSmsEnabled = isSmsEnabled;
                invalidateOptionsMenu();
            }
            /// M Fix CR : ALPS01257113, when sub select dialog
            /// showed in default sms. after change to
            /// non-default sms, dismiss the sub select dialog @{
            if (!mIsSmsEnabled) {
                if (mSubSelectDialog != null && mSubSelectDialog.isShowing()) {
                    mSubSelectDialog.dismiss();
                }
            }
            /// @}

            /// M: Code analyze 036, Change text size if adjust font size.@{
            float textSize =
                MessageUtils.getPreferenceValueFloat(this, SettingListActivity.TEXT_SIZE, 18);
            setTextSize(textSize);
            /// @}

            /// M: Code analyze 013, Get contacts from Contact app . @{
            misPickContatct = false;
            /// @}
            // M:for ALPS01065027,just for compose messagelist in scrolling
            mMsgListView.setOnScrollListener(mScrollListener);
            mScrollListener.setIsNeedRefresh(true);
            mScrollListener.setThreadId(mConversation.getThreadId(), ComposeMessageActivity.this);
            initFocus();

            mOpComposeExt.onStart(this, this);
            loadMessageContent();

            // Update the fasttrack info in case any of the recipients' contact info changed
            // while we were paused. This can happen, for example, if a user changes or adds
            // an avatar associated with a contact.
            /// M: @{
            if (mConversation.getThreadId() == 0) {
                mWorkingMessage.syncWorkingRecipients();
            }
            /// @}

            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                MmsLog.vpi(TAG, "onStart: update title, mConversation=" + mConversation.toString());
            }

            ActionBar actionBar = getActionBar();
            /// M: Add log. @{
            if (actionBar == null) {
                MmsLog.e(TAG, "ACTION BAR is null, window feature FEATURE_ACTION_BAR: "
                        + getWindow().hasFeature(Window.FEATURE_ACTION_BAR)
                        + ", feature FEATURE_NO_TITLE: "
                        + getWindow().hasFeature(Window.FEATURE_NO_TITLE));
            } else {
                actionBar.setDisplayHomeAsUpEnabled(true);
            }
            /// @}

            if (isRecipientsEditorVisible() && mIsSmsEnabled &&
                    getResources().getConfiguration().orientation
                        != Configuration.ORIENTATION_LANDSCAPE) {
                getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE |
                        WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
            }

            if (mMsgListAdapter != null) {
                mMsgListAdapter.setOnDataSetChangedListener(mDataSetChangedListener);
            }
            addRecipientsListeners();
        } else {
            finish();
        }
    }

    private boolean requestAllPermissions() {
        ArrayList<String> missingList;
        CtaManager manager = CtaManagerFactory.getInstance().makeCtaManager();
        if (manager.isCtaSupported()) {
            missingList = PermissionCheckUtil.getMissingPermissions(this,
                        PermissionCheckUtil.ALL_CTA_PERMISSIONS);
        } else {
            missingList = PermissionCheckUtil.getMissingPermissions(this,
                    PermissionCheckUtil.ALL_PERMISSIONS);
        }

        if (missingList.size() == 0) {
            Log.d(TAG, " requestAllPermissions all permissions granted");
            return false;
        }

        final String[] missingArray = new String[missingList.size()];
        missingList.toArray(missingArray);
        Log.d(TAG, " requestAllPermissions all permissions start!");
        PermissionCheckUtil.setPermissionActivityCount(true);
        requestPermissions(missingArray,
                PermissionCheckUtil.REQUIRED_PERMISSIONS_REQUEST_CODE);
        return true;
    }

    private boolean isAllPermissionGranted() {
        String[] permissions;
        CtaManager manager = CtaManagerFactory.getInstance().makeCtaManager();
        if (manager.isCtaSupported()) {
            permissions = PermissionCheckUtil.ALL_CTA_PERMISSIONS;
        } else {
            permissions = PermissionCheckUtil.ALL_PERMISSIONS;
        }

        for (int i = 0; i < permissions.length; i++) {
            if (!PermissionCheckUtil.hasPermission(this, permissions[i])) {
                // Show toast
                if (PermissionCheckUtil.isRequiredPermission(permissions[i])
                        || PermissionCheckUtil.isNeverGrantedPermission(this, permissions[i])) {
                    if (!PermissionCheckUtil.isPermissionChecking()) {
                        PermissionCheckUtil.showNoPermissionsToast(this);
                    }
                }
                Log.d(TAG, "isAllPermissionGranted return false");
                return false;
            }
        }

        Log.d(TAG, "isAllPermissionGranted return true");
        return true;
    }

    private final BroadcastReceiver mShutDownReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (Intent.ACTION_SHUTDOWN.equals(intent.getAction())) {
                onShutDown();
            }
        }
    };

    /* SIM ON OFF*/
    private int getEnabledSubCount() {
        boolean simOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        if (simOnOffEnabled == false) {
            return mSubCount;
        }

        if (mSubInfoList == null || mSubCount == 0) {
            return mSubCount;
        }

        int enabledSubCount = 0;
        for (int i = 0; i < mSubCount; i++) {
            //int subId = (int) mSubInfoList.get(i).getSubscriptionId();
            SubscriptionInfo subInfo = mSubInfoList.get(i);
            if (subInfo != null) {
                int slotId = subInfo.getSimSlotIndex();
                int slotState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
                if (slotState == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                    enabledSubCount++;
                }
            }
        }
        return enabledSubCount;
    }

    private int getFirstEnabledSubId() {
        int selectedSubId;

        boolean simOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        if (simOnOffEnabled == false) {
            selectedSubId = (int) mSubInfoList.get(0).getSubscriptionId();
            return selectedSubId;
        }

        for (int i = 0; i < mSubCount; i++) {
            //int subId = (int) mSubInfoList.get(i).getSubscriptionId();
            SubscriptionInfo subInfo = mSubInfoList.get(i);
            if (subInfo != null) {
                int slotId = subInfo.getSimSlotIndex();
                int slotState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
                if (slotState == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                    selectedSubId = subInfo.getSubscriptionId();
                    return selectedSubId;
                }
            }
        }
        MmsLog.d(TAG, "return -1!!!");
        return -1;
    }
}
