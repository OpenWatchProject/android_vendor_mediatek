/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
 /*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.android.mms.data;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteDiskIOException;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.provider.Telephony.MmsSms.WordsTable;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Threads;
import android.telephony.SmsMessage;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.android.common.contacts.DataUsageStatUpdater;
import com.android.common.userhappiness.UserHappinessSignals;
import com.android.mms.ContentRestrictionException;
import com.android.mms.ExceedMessageSizeException;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.ResolutionException;
import com.android.mms.RestrictedResolutionException;
import com.android.mms.UnsupportContentTypeException;
import com.android.mms.draft.DraftManager;
import com.android.mms.draft.IDraftInterface;
import com.android.mms.draft.MmsDraftData;
import com.android.mms.model.AudioModel;
import com.android.mms.model.ImageModel;
import com.android.mms.model.MediaModel;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.model.SlideshowModel.MediaType;
import com.android.mms.model.TextModel;
import com.android.mms.model.VideoModel;
import com.android.mms.transaction.MessageSender;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.transaction.MmsMessageSender;
import com.android.mms.transaction.SmsMessageSender;
import com.android.mms.ui.ComposeMessageActivity;
import com.android.mms.ui.MessageUtils;
import com.android.mms.ui.SlideshowEditor;
import com.android.mms.util.MmsLog;
import com.android.mms.util.Recycler;
import com.android.mms.util.ThumbnailManager;
import com.android.mms.widget.MmsWidgetProvider;
import com.google.android.mms.ContentType;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.EncodedStringValue;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.android.mms.pdu.MtkSendReq;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.mms.callback.IConversationCallback;
import com.mediatek.mms.callback.ISplitToMmsAndSmsConversationCallback;
import com.mediatek.mms.callback.IWorkingMessageCallback;
import com.mediatek.mms.ext.IOpWorkingMessageExt;
import com.mediatek.mms.util.FileAttachmentUtils;
import com.mediatek.mms.util.MmsSizeUtils;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.MmsPreferenceActivity;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Contains all state related to a message being edited by the user.
 */
public class WorkingMessage implements IWorkingMessageCallback {
    private static final String TAG = "WorkingMessage";
    private static final String TAG_DRAFT = "[Mms][Draft][WorkingMessage]";
    private static final boolean DEBUG = false;

    // Public intents
    public static final String ACTION_SENDING_SMS = "android.intent.action.SENDING_SMS";

    // Intent extras
    public static final String EXTRA_SMS_MESSAGE = "android.mms.extra.MESSAGE";
    public static final String EXTRA_SMS_RECIPIENTS = "android.mms.extra.RECIPIENTS";
    public static final String EXTRA_SMS_THREAD_ID = "android.mms.extra.THREAD_ID";

    //for save message uri when MMS is stoping, and when recreate MMS, it can be read
    public static final String SAVE_MSG_URI_KEY = "pref_msg_uri_key";
    public static final String SAVE_MSG_THREADID_KEY = "pref_msg_threadid_key";

    // Database access stuff
    private final Activity mActivity;
    private final ContentResolver mContentResolver;

    // States that can require us to save or send a message as MMS.
    private static final int RECIPIENTS_REQUIRE_MMS = (1 << 0);     // 1
    private static final int HAS_SUBJECT = (1 << 1);                // 2
    private static final int HAS_ATTACHMENT = (1 << 2);             // 4
    private static final int LENGTH_REQUIRES_MMS = (1 << 3);        // 8
    private static final int FORCE_MMS = (1 << 4);                  // 16
    /// M: google JB.MR1 patch, group mms
    private static final int MULTIPLE_RECIPIENTS = (1 << 5);        // 32

    // A bitmap of the above indicating different properties of the message;
    // any bit set will require the message to be sent via MMS.
    private int mMmsState;

    // Errors from setAttachment()
    public static final int OK = 0;
    public static final int UNKNOWN_ERROR = -1;
    public static final int MESSAGE_SIZE_EXCEEDED = -2;
    public static final int UNSUPPORTED_TYPE = -3;
    public static final int IMAGE_TOO_LARGE = -4;

    // Attachment types
    public static final int TEXT = 0;
    public static final int IMAGE = 1;
    public static final int VIDEO = 2;
    public static final int AUDIO = 3;
    public static final int SLIDESHOW = 4;

    // Current attachment type of the message; one of the above values.
    private int mAttachmentType;

    // Conversation this message is targeting.
    private Conversation mConversation;

    // Text of the message.
    private CharSequence mText;
    // Slideshow for this message, if applicable.  If it's a simple attachment,
    // i.e. not SLIDESHOW, it will contain only one slide.
    private SlideshowModel mSlideshow;
    // Data URI of an MMS message if we have had to save it.
    private Uri mMessageUri;
    // MMS subject line for this message
    private CharSequence mSubject;

    // Set to true if this message has been discarded.
    private boolean mDiscarded = false;

    // Track whether we have drafts
    private volatile boolean mHasMmsDraft;
    private volatile boolean mHasSmsDraft;

    // Cached value of mms enabled flag
    private static boolean sMmsEnabled = MmsConfig.getMmsEnabled();

    // Our callback interface
    private final MessageStatusListener mStatusListener;
    private List<String> mWorkingRecipients;

    /// M: Modify @{
    private static final String FDN_URI = "content://icc/fdn/subId/";
    private static final String[] FDN_PROJECTION = new String[] {
        "index",
        "name",
        "number"
    };
    private static final int FDN_COLUMN_INDEX = 0;
    private static final int FDN_COLUMN_NAME = 1;
    private static final int FDN_COLUMN_NUMBER = 2;
    /// @}

    /// M:
    private static final String M_TAG = "Mms/WorkingMessage";

    /// M: Code analyze 034, For new feature ALPS00231349, add vCard support . @{
    public static final int ATTACHMENT = 5;
    public static final int VCARD = 6;
    /// @}
    /// M: Code analyze 035, For new feature ALPS00249336,  add vCalendar support . @{
    public static final int VCALENDAR = 7;
    /// @}

    /// M: Code analyze 036, For bug ALPS00270539, mms draft edit lock. at any
    /// time, only one thread can modify a mms draft. here currently use a static
    /// lock is ok, because WorkingMessage is only one at any time. if the condition
    /// is changed this must be changed too . @{
    public static Object sDraftMmsLock = new Object();
    /// @}

    private long mOldThreadId;

    /// M: Fix CR : ALPS01012417 @{
    private long mOldSmsSaveThreadId;
    /// @}

    /// M: Fix CR : ALPS01078057 @{
    private long mOldMmsSaveThreadId = 0;
    /// @}

    /// M : FIX CR : ALPS01795853 @{
    private boolean mIsTurnToChooseAttach;
    /// @}

    /// M: Fix CR: ALPS01234459 @{
    private boolean mIsLoadingDraft;
    /// @}

    // Draft message stuff
    private static final String[] MMS_DRAFT_PROJECTION = {
        Mms._ID,                // 0
        Mms.SUBJECT,            // 1
        Mms.SUBJECT_CHARSET     // 2
    };

    private static final int MMS_ID_INDEX         = 0;
    private static final int MMS_SUBJECT_INDEX    = 1;
    private static final int MMS_SUBJECT_CS_INDEX = 2;

    /// M: Code analyze 039, For new feature ALPS00233419, Creation mode . @{
    private static final String CREATION_MODE_RESTRICTED = "RESTRICTED";
    private static final String CREATION_MODE_WARNING    = "WARNING";
    private static final String CREATION_MODE_FREE       = "FREE";

    public static final int WARNING_TYPE    = -10;
    public static final int RESTRICTED_TYPE = -11;
    public static final int RESTRICTED_RESOLUTION = -12;

    public static int sCreationMode  = 0;
    /// @}

    //Set resizedto true if the image is
    private boolean mResizeImage = false;

    /// M: Code analyze 043, For bug ALPS00117913, Mms Basic Coding Convention Correction . @{
    private static final String FILE_NOT_FOUND = "File not found.";

    private static final String READ_WRITE_FAILURE = "Read or write file failure.";

    private boolean mIsDeleteDraftWhenLoad = false;
    /// @}

    /// M: fix bug ALPS00513231, force update threadId @{
    private boolean mForceUpdateThreadId = false;

    public void setForceUpdateThreadId(boolean update) {
        mForceUpdateThreadId = update;
    }
    /// @}

    private Bundle mBundle;

    public IOpWorkingMessageExt mOpWorkingMessageExt = null;

    /**
     * Callback interface for communicating important state changes back to
     * ComposeMessageActivity.
     */
    public interface MessageStatusListener {
        /**
         * Called when the protocol for sending the message changes from SMS
         * to MMS, and vice versa.
         *
         * @param mms If true, it changed to MMS.  If false, to SMS.
         */
        /// M: Code analyze 044, For bug ALPS00050082, add toast . @{
        void onProtocolChanged(boolean mms, boolean needToast);
        /// @}

        /**
         * Called when an attachment on the message has changed.
         */
        void onAttachmentChanged();

        /**
         * Called just before the process of sending a message.
         */
        void onPreMessageSent();

        /**
         * Called once the process of sending a message, triggered by
         * {@link send} has completed. This doesn't mean the send succeeded,
         * just that it has been dispatched to the network.
         */
        void onMessageSent();

        /**
         * Called if there are too many unsent messages in the queue and we're not allowing
         * any more Mms's to be sent.
         */
        void onMaxPendingMessagesReached();

        /**
         * Called if there's an attachment error while resizing the images just before sending.
         */
        void onAttachmentError(int error);

        /** M:Code analyze 045, For bug ALPS00241360, to solve White screen
         * appears about 3 seconds when sending MMS. Called just before the
         * process of sending a mms.
         */
        void onPreMmsSent();
    }

    private WorkingMessage(ComposeMessageActivity activity) {
        MmsLog.d(TAG, "new WorkingMessage(Composer)");
        mActivity = activity;
        mContentResolver = mActivity.getContentResolver();
        mStatusListener = activity;
        mAttachmentType = TEXT;
        mText = "";
        /// M: Code analyze 040, For bug ALPS00116011, the creation mode can't
        /// take effect immediately after modify in settings Should update static
        /// variable after peference is changed . @{
        updateCreationMode(activity);
        /// @}
        this.mInterface = new DraftInterface();
        mOpWorkingMessageExt = OpMessageUtils.getOpMessagePlugin().getOpWorkingMessageExt();
        mOpWorkingMessageExt.initOpWorkingMessage(this);
    }

    /**
     * Creates a new working message.
     */
    public static WorkingMessage createEmpty(ComposeMessageActivity activity) {
        // Make a new empty working message.
        WorkingMessage msg = new WorkingMessage(activity);
        return msg;
    }

    /**
     * Create a new WorkingMessage from the specified data URI, which typically
     * contains an MMS message.
     */
    public static WorkingMessage load(ComposeMessageActivity activity, Uri uri) {
        // If the message is not already in the draft box, move it there.
        if (!uri.toString().startsWith(Mms.Draft.CONTENT_URI.toString())) {
            MtkPduPersister persister = MtkPduPersister.getPduPersister(activity);
            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                MmsLog.dpi(TAG, "load: moving %s to drafts" + uri);
            }
            try {
                uri = persister.move(uri, Mms.Draft.CONTENT_URI);
            } catch (MmsException e) {
                MmsLog.epi(TAG, "Can't move %s to drafts" + uri);
                return null;
            }
        }

        WorkingMessage msg = new WorkingMessage(activity);
        /// M: Code analyze 046, For bug ALPS00114670, to solve JE happen
        /// when tap discard slidehsow . @{
        msg.setConversation(activity.getConversation());
        /// @}
        long threadId = 0;
        boolean noRecipientes = activity.getConversation().getRecipients().isEmpty();
        if (!noRecipientes) {
            threadId = activity.getConversation().ensureThreadId();
        }

        MmsDraftData mdd = DraftManager.getInstance().loadDraft(DraftManager.SYNC_LOAD_ACTION,
                threadId, uri, activity, null);
        if (mdd == null) {
            Log.d(TAG_DRAFT, "[load] load from DraftManager result is null!!");
            return null;
        }
        Log.d(TAG_DRAFT, "[load] boolean result is : " + mdd.getBooleanResult());
        if (mdd.getBooleanResult()) {
            msg.mHasMmsDraft = true;
            msg.mSlideshow = mdd.getSlideshow();
            msg.mMessageUri = mdd.getMessageUri();
            msg.syncTextFromSlideshow();
            msg.correctAttachmentState();
            MmsLog.dpi(TAG_DRAFT, "[load] message uri : " + msg.mMessageUri);
            return msg;
        }

        return null;
    }

    public void correctAttachmentState() {
        if (mSlideshow == null) {
            return;
        }
        int slideCount = mSlideshow.size();
        /// M: Code analyze 034, For new feature ALPS00231349,  add vCard support . @{
        final int fileAttachCount = mSlideshow.sizeOfFilesAttach();
        /// @}
        // If we get an empty slideshow, tear down all MMS
        // state and discard the unnecessary message Uri.
        /// M: Code analyze 034, For new feature ALPS00231349,  add vCard support . @{
        if (0 == fileAttachCount) {
        /// @}
            if (slideCount == 0 || isEmptySlide()) {
                //add for attachment enhance
                MmsLog.d(TAG, "WorkingMessage CorrectAttachmentState RemoveAttachment");
                removeAttachment(true);
            } else if (slideCount > 1) {
                mAttachmentType = SLIDESHOW;
            } else {
                SlideModel slide = mSlideshow.get(0);
                if (slide.hasImage()) {
                    mAttachmentType = IMAGE;
                } else if (slide.hasVideo()) {
                    mAttachmentType = VIDEO;
                } else if (slide.hasAudio()) {
                    mAttachmentType = AUDIO;
                }
            }
        } else { /// M: Code analyze 034, For new feature ALPS00231349,  add vCard support . @{
            mAttachmentType = ATTACHMENT;
        }
        /// @}

        updateState(HAS_ATTACHMENT, hasAttachment(), false);
    }

//add for attachment enhance
     private boolean isEmptySlide() {
        int slideCount = mSlideshow.size();

        if (slideCount == 1) {
            if (mSlideshow != null) {
                MmsLog.d(TAG, "mSlideshow != null");
               if (mSlideshow.get(0) != null) {
                    //Xlog.e(TAG, "contentType = " + mSlideshow.get(0).get(0).getContentType());
                    MmsLog.d(TAG, "mAttachmentType = " + mAttachmentType);
                    //MmsLog.e(TAG, "mSlideshow.get(0).get(0) "+ mSlideshow.get(0).get(0));

                    if (mSlideshow.get(0).get(0) != null && mSlideshow.get(0).size() == 1) {
                        MmsLog.d(TAG, "mSlideshow.get(0).get(0).size " + mSlideshow.get(0).size());
                        if (mSlideshow.get(0).get(0).getContentType()
                                .compareTo(ContentType.TEXT_PLAIN) == 0
                        && (mAttachmentType == VCARD || mAttachmentType == VCALENDAR ||
                        mAttachmentType == ATTACHMENT)) {
                            MmsLog.d(TAG, "isEmptySlide return true");
                            return true;
                        }
                    } else if (mSlideshow.get(0).size() == 0 || mSlideshow.get(0).get(0) == null) {
                        ///M: Modify for ALPS01253847
                        return true;
                    }
               }
           }
        }

        MmsLog.d(TAG, "isEmptySlide return false");
        return false;
    }

    /**
     * Load the draft message for the specified conversation, or a new empty message if
     * none exists.
     */
    public static WorkingMessage loadDraft(final ComposeMessageActivity activity,
                                           final Conversation conv,
                                           final Runnable onDraftLoaded) {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "loadDraft " + conv);
        }

        final WorkingMessage msg = createEmpty(activity);
        /// M: Code analyze 037, For bug ALPS00291328,  save conversation to avoid JE . @{
        msg.setConversation(conv);
        /// @}
        if (conv.getThreadId() <= 0) {
            if (onDraftLoaded != null) {
                onDraftLoaded.run();
            }
            return msg;
        }

        final long threadId = conv.getThreadId();
        msg.mOldThreadId = threadId;

        /// M: modify for fix alps01194967  && alps01208583 &&
        /// alps01224306, load sms draft in backGround. @{
        new Thread(new Runnable() {
            public void run() {
                final String draftText = msg.readDraftSmsMessage(conv);
                MmsLog.dpi(TAG_DRAFT, "[loadDraft] draftText : " + draftText);
                if (TextUtils.isEmpty(draftText)) {
                    msg.mInterface.loadRunnable = onDraftLoaded;
                    /// M: Fix CR : ALPS01234459, while loading draft, can't set text @{
                    msg.mIsLoadingDraft = true;
                    /// @}
                    DraftManager.getInstance().loadDraft(DraftManager.ASYNC_LOAD_ACTION,
                            threadId, null, activity, msg.mInterface);
                } else {
                    msg.mHasSmsDraft = true;
                    if (onDraftLoaded != null && activity != null) {
                        activity.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                msg.setText(draftText);
                                onDraftLoaded.run();
                            }
                        });
                    }
                }
            }
        }, "WorkingMessage.loadDraft").start();
        /// @}
        return msg;
    }

    private DraftInterface mInterface;

    private class DraftInterface implements IDraftInterface {
        private Handler mHanlder;
        DraftInterface() {
            mHanlder = new Handler();
        }

        public Runnable loadRunnable;

        public void loadFinished(MmsDraftData mdd) {
            if (mdd != null) {
                MmsLog.dpi(TAG_DRAFT, "[loadFinished] enter, and uri : "
                        + mdd.getMessageUri() + ", subject : " + mdd.getSubject());

                String subject = mdd.getSubject();
                if (subject != null && subject.length() != 0) {
                    mHasMmsDraft = true;
                    setSubject(subject, false);
                }

                Log.d(TAG_DRAFT, "[loadFinished] boolean result : " + mdd.getBooleanResult());
                if (mdd.getBooleanResult()) {
                    mHasMmsDraft = true;
                    SlideshowModel slideshow = mdd.getSlideshow();
                    mMessageUri = mdd.getMessageUri();

                    mSlideshow = slideshow;
                    syncTextFromSlideshow();
                    correctAttachmentState();
                    if (mActivity != null && MmsConfig.isSmsEnabled(mActivity)) {
                        Log.d(TAG_DRAFT, "[loadFinished] defualt sms, do delete mms draft");
                        if (getSlideshow() != null && getSlideshow().size() == 1
                                && !getSlideshow().get(0).hasAudio()
                                && !getSlideshow().get(0).hasImage()
                                && !getSlideshow().get(0).hasVideo()
                                && getSlideshow().sizeOfFilesAttach() == 0
                                && TextUtils.isEmpty(subject)
                                && TextUtils.isEmpty(getText())) {
                            Log.d(TAG_DRAFT, "[loadFinished] delete");
                            asyncDeleteDraftMmsMessage(mConversation);
                            removeAllFileAttaches();
                            removeAttachment(false);
                            setSubject(subject, false);
                            if (mConversation.getMessageCount() <= 0) {
                                mConversation.clearThreadId();
                            }
                        }
                    }
                }
            }
            /// M Fix CR : ALPS01234459 @{
            mIsLoadingDraft = false;
            /// M @}
            if (loadRunnable != null) {
                mHanlder.post(loadRunnable);
            }
            /// M Fix CR : ALPS01071659 @{
            deleteGruoupMmsDraft();
            /// @}
        }

        public void updateAfterSaveDraftFinished(final Uri msgUri,
                final int create, final boolean result) {
            MmsLog.dpi(TAG_DRAFT, "[updateAfterSaveDraftFinished] msgUri is : " + msgUri
                   + ", create : " + create);
            // use a thread to enhance the quit compose performance which can quit to
            // conversation list quickly

            if (msgUri == null && create == 1 && !result) {
                Log.d(TAG_DRAFT, "[updateAfterSaveDraftFinished]"
                        + " MmsException happened, and save failed!");
                removeAllFileAttaches();
                removeAttachment(true);
                mActivity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        // TODO Auto-generated method stub
                        MessageUtils.showErrorDialog(mActivity,
                                android.R.string.dialog_alert_title,
                                R.string.error_add_attachment, 0, R.string.type_common_file);
                    }
                });
                return;
            }
            MmsLog.dpi(TAG_DRAFT, "[updateAfterSaveDraftFinished] before msg uri : "
                                               + mMessageUri);
            if (msgUri != null) {
                mMessageUri = msgUri;
            }
            MmsLog.dpi(TAG_DRAFT, "[updateAfterSaveDraftFinished] after msg uri : "
                                               + mMessageUri);

            // / which if the save req is create, quit compose and mms maybe
            // be killed
            // / so store the uri into bundle, and start MMs again, which
            // can load from preference.xml @{
            if (mMessageUri != null && mBundle != null && create == 1) {
                // /M: fix bug: ALPS00568220,
                // /M: save message Uri in preference, some times MMS
                // will be killed by system
                SharedPreferences sp = PreferenceManager
                        .getDefaultSharedPreferences(mActivity);
                SharedPreferences.Editor editor = sp.edit();
                editor.putString(SAVE_MSG_URI_KEY, mMessageUri.toString());
                MmsLog.dpi(TAG_DRAFT, "[updateAfterSaveDraftFinished]"
                        + "save message uri to preference : " + mMessageUri.toString());
                editor.apply();
            }
            // / @}
            if (mMessageUri != null) {
                if (!mConversation.getRecipients().isEmpty()) {
                    mConversation.ensureThreadId();
                    if (mBundle != null) {
                        SharedPreferences sp1 =
                            PreferenceManager.getDefaultSharedPreferences(mActivity);
                        SharedPreferences.Editor editor1 = sp1.edit();
                        Log.d(TAG_DRAFT, "[updateAfterSaveDraftFinished]"
                                + "save thread id to preference : "
                                + mConversation.getThreadId());
                        editor1.putLong(SAVE_MSG_THREADID_KEY, mConversation.getThreadId());
                        editor1.apply();
                    }
                }
                mConversation.setDraftState(true);
            } else {
                mConversation.setDraftState(false);
            }
            // / @}
            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                MmsLog.dpi(TAG, "updateAfterSaveDraftFinished conv: "
                        + mConversation + " uri: " + mMessageUri);
            }

            // / @}

            // Be paranoid and delete any SMS drafts that might be lying
            // around. Must do
            // this after ensureThreadId so conv has the correct thread
            // id.
            asyncDeleteDraftSmsMessage(mConversation);

            /// M : Fix CR : ALPS01031682
            /// press home key, will saveDraft(Sms), and after change the recipients saveMms
            /// the draft will update to the old thread @{
            if (mOldSmsSaveThreadId > 0) {
                if (mOldSmsSaveThreadId != mConversation.getThreadId()) {
                    deleteDraftSmsMessage(mOldSmsSaveThreadId);
                }
            }
            /// M Fix CR: ALPS01105564 two draft will show in the conversation list
            /// because the sound recorder will finish itself, when lock the screnn.
            /// under this situtation, will send 2 save req to draftmanager,
            /// meanwhile the mMessageUri is null
            /// which will create 2 pdu id in the DB and bind to the origin thread .
            /// after change the recipients, the thread will not be deleted,
            /// so 2 threads showed in the conversationlist @{
            deleteOldMmsDraft(mMessageUri, mConversation.getThreadId());
            /// @}

            /// M: Fix CR : ALPS01078057. when the thread changed,
            /// if the old thread saved Mms draft before,
            /// should reset the old thread's draft state @{
            if (mOldMmsSaveThreadId != 0
                    && mOldMmsSaveThreadId != mConversation.getThreadId()) {
                asyncDeleteOldMmsDraft(mOldMmsSaveThreadId);
            }
            mOldMmsSaveThreadId = mConversation.getThreadId();
            /// @}
        }
    }

    /**
     * Sets the text of the message to the specified CharSequence.
     */
    public void setText(CharSequence s) {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "setText: s = " + s);
        }
        /// M Fix CR : ALPS01234459, while loading draft, no need to set text @{
        if (mIsLoadingDraft) {
            Log.d(TAG, "[setText] run loading draft, do not need to set text");
            return;
        }
        /// M @}
        mText = s;
        /// M: @{
        if (mText != null && TextUtils.getTrimmedLength(mText) >= 0) {
            syncTextToSlideshow();
        }
        if (mText == null || mText.length() == 0) {
            /// M: fix bug ALPS00945813, remove MmsDraft when Length_Mms_Text are deleted
            if (mMmsState == LENGTH_REQUIRES_MMS && !hasSubject()
                    && !hasMediaAttachments() && !hasAttachedFiles()) {
                asyncDeleteDraftMmsMessage(mConversation);
                clearConversation(mConversation, true);
            }
            /// @}
            /// M Fix CR ALPS01141440, which still has mms draft after forward message @{
            if ((mActivity instanceof ComposeMessageActivity)
                    && ((ComposeMessageActivity) mActivity).getForwordingState()) {
                if (mMmsState == LENGTH_REQUIRES_MMS && !hasSubject()
                    && !hasMediaAttachments() && !hasAttachedFiles()) {
                    asyncDelete(mMessageUri, null, null);
                    clearConversation(mConversation, true);
                    mMessageUri = null;
                }
            }
            /// @}
        }

    }

    /**
     * Returns the current message text.
     */
    public CharSequence getText() {
        return mText;
    }

    /**
     * @return True if the message has any text. A message with just whitespace is not considered
     * to have text.
     */
    public boolean hasText() {
        if (requiresMms()) {
            return mText != null && TextUtils.getTrimmedLength(mText) > 0;
        } else {
            return mText != null && !TextUtils.isEmpty(mText);
        }
    }

    public void removeAttachment(boolean notify) {
        if (mActivity != null && !MmsConfig.isSmsEnabled(mActivity)) {
            Log.d(TAG_DRAFT,
                    "[removeAttachment] not default sms, can't remove attachment,just return!!");
            return;
        }
        MmsLog.d(TAG, "WorkingMessage RemoveAttachment");
        removeThumbnailsFromCache(mSlideshow);
        /// M: fix bug ALPS00956551, need clear mText when add attachment twice
        if (hasSlideshow()) {
            mText = "";
        }
        mAttachmentType = TEXT;
        /// M: fix bug ALPS02141556 @{
        synchronized (this) {
            mSlideshow = null;
        }
        /// @}
        if (mMessageUri != null) {
            asyncDelete(mMessageUri, null, null);
            mMessageUri = null;
        }
        // mark this message as no longer having an attachment
        updateState(HAS_ATTACHMENT, false, notify);
        if (notify) {
            // Tell ComposeMessageActivity (or other listener) that the attachment has changed.
            // In the case of ComposeMessageActivity, it will remove its attachment panel because
            // this working message no longer has an attachment.
            mStatusListener.onAttachmentChanged();
        }

        clearConversation(mConversation, true);
    }

    public static void removeThumbnailsFromCache(SlideshowModel slideshow) {
        if (slideshow != null) {
            ThumbnailManager thumbnailManager = MmsApp.getApplication().getThumbnailManager();
            boolean removedSomething = false;
            Iterator<SlideModel> iterator = slideshow.iterator();
            while (iterator.hasNext()) {
                SlideModel slideModel = iterator.next();
                if (slideModel.hasImage()) {
                    /// M: change thumbnail's uri @{
                    ImageModel im = slideModel.getImage();
                    Uri uri = ThumbnailManager.getThumbnailUri(im);
                    thumbnailManager.removeThumbnail(uri);
                    /// @}
                    removedSomething = true;
                } else if (slideModel.hasVideo()) {
                    /// M: change thumbnail's uri @{
                    VideoModel vm = slideModel.getVideo();
                    Uri uri = ThumbnailManager.getThumbnailUri(vm);
                    thumbnailManager.removeThumbnail(uri);
                    /// @}
                    removedSomething = true;
                }
            }
            if (removedSomething) {
                // HACK: the keys to the thumbnail cache are the part uris, such as mms/part/3
                // Because the part table doesn't have auto-increment ids, the part ids are reused
                // when a message or thread is deleted. For now, we're clearing the whole thumbnail
                // cache so we don't retrieve stale images when part ids are reused. This will be
                // fixed in the next release in the mms provider.
                MmsApp.getApplication().getThumbnailManager().clearBackingStore();
            }
        }
    }

    /**
     * Adds an attachment to the message, replacing an old one if it existed.
     * @param type Type of this attachment, such as {@link IMAGE}
     * @param dataUri Uri containing the attachment data (or null for {@link TEXT})
     * @param append true if we should add the attachment to a new slide
     * @return An error code such as {@link UNKNOWN_ERROR} or {@link OK} if successful
     */
    public int setAttachment(int type, Uri dataUri, boolean append) {
        MmsLog.dpi(TAG, "setAttachment type = " + type + " uri = " + dataUri);
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "setAttachment type= " + type + "uri " + dataUri);
        }
        int result = OK;
        SlideshowEditor slideShowEditor = new SlideshowEditor(mActivity, mSlideshow);

        // Special case for deleting a slideshow. When ComposeMessageActivity gets told to
        // remove an attachment (search for AttachmentEditor.MSG_REMOVE_ATTACHMENT), it calls
        // this function setAttachment with a type of TEXT and a null uri. Basically, it's turning
        // the working message from an MMS back to a simple SMS. The various attachment types
        // use slide[0] as a special case. The call to ensureSlideshow below makes sure there's
        // a slide zero. In the case of an already attached slideshow, ensureSlideshow will do
        // nothing and the slideshow will remain so that if a user adds a slideshow again, they'll
        // see their old slideshow they previously deleted. Here we really delete the slideshow.
        if (type == TEXT && mAttachmentType == SLIDESHOW && mSlideshow != null && dataUri == null
                && !append) {
            slideShowEditor.removeAllSlides();
        }

        /// M: This is added for failed to share only one picture with message. @{
        if (mSlideshow == null) {
            append = true;
        }
        /// @}

        // Make sure mSlideshow is set up and has a slide.
        ensureSlideshow();      // mSlideshow can be null before this call, won't be afterwards
        slideShowEditor.setSlideshow(mSlideshow);

        // Change the attachment and translate the various underlying
        // exceptions into useful error codes.
        try {
            /// M: Code analyze 034, For new feature ALPS00231349,  add vCard support . @{
            if (type >= ATTACHMENT) {
                if (mSlideshow == null) {
                    mSlideshow = SlideshowModel.createNew(mActivity);
                }
                new FileAttachmentUtils().setOrAppendFileAttachment(
                        mActivity, mSlideshow, mText, type, dataUri, append);
            } else { /// @}
                if (append) {
                    appendMedia(type, dataUri);
                } else {
                    changeMedia(type, dataUri);
                }
            }
            /// @}
        } catch (MmsException e) {
            /// M:
            MmsLog.e(TAG, e.getMessage() != null ? e.getMessage() : "setAttachment MmsException");
            result = UNKNOWN_ERROR;
        } catch (UnsupportContentTypeException e) {
            /// M: fix bug ALPS604911, modify toast msg
            result = UNSUPPORTED_TYPE;
        } catch (ExceedMessageSizeException e) {
            result = MESSAGE_SIZE_EXCEEDED;
        } catch (ResolutionException e) {
            result = IMAGE_TOO_LARGE;
        /// M: @{
        } catch (ContentRestrictionException e) {
            result = sCreationMode;
        } catch (RestrictedResolutionException e) {
            result = RESTRICTED_RESOLUTION;
        } catch (IllegalStateException e) {
            MmsLog.e(TAG, e.getMessage());
            result = UNKNOWN_ERROR;
        } catch (IllegalArgumentException e) {
            MmsLog.e(TAG, e.getMessage());
            result = UNKNOWN_ERROR;
        } catch (SecurityException e) {
            MmsLog.e(TAG, e.getMessage());
            result = UNKNOWN_ERROR;
        } catch (RuntimeException e) {
            MmsLog.e(TAG, e.getMessage());
            result = UNKNOWN_ERROR;
        }
        /// @}

        MmsLog.d(TAG, "setAttachment result = " + result);

        // If we were successful, update mAttachmentType and notify
        // the listener than there was a change.
        if (result == OK) {
            mAttachmentType = type;
            /// M: @{
            if (mSlideshow == null) {
                return UNKNOWN_ERROR;
            }
            if (mSlideshow.size() > 1) {
                mAttachmentType = SLIDESHOW;
            }
            /// @}
        } else if (append) {
            // We added a new slide and what we attempted to insert on the slide failed.
            // Delete that slide, otherwise we could end up with a bunch of blank slides.
//            SlideshowEditor slideShowEditor = new SlideshowEditor(mActivity, mSlideshow);
            /// M: @{
            if (slideShowEditor == null || mSlideshow == null) {
                return UNKNOWN_ERROR;
            }
             /// @}

            /// M: Modify ALPS00566470 @{
            if (!mOpWorkingMessageExt.setAttachment(type, ATTACHMENT,
                    slideShowEditor)) {
                slideShowEditor.removeSlide(mSlideshow.size() - 1);
            }
        }

        // correctAttachmentState();
        mIsUpdateAttachEditor = true;

        if (mSlideshow != null && type == IMAGE) {
            // Prime the image's cache; helps A LOT when the image is coming from the network
            // (e.g. Picasa album). See b/5445690.
            int numSlides = mSlideshow.size();
            if (numSlides > 0) {
                ImageModel imgModel = mSlideshow.get(numSlides - 1).getImage();
                if (imgModel != null) {
                    cancelThumbnailLoading();
                    imgModel.loadThumbnailBitmap(null);
                }
            }
        }
        mStatusListener.onAttachmentChanged();  // have to call whether succeeded or failed,
                                                // because a replace that fails, removes the slide

        if (!MmsConfig.getMultipartSmsEnabled()) {
            if (!append && mAttachmentType == TEXT && type == TEXT) {
                int[] params = SmsMessage.calculateLength(getText(), false);
                /* SmsMessage.calculateLength returns an int[4] with:
                *   int[0] being the number of SMS's required,
                *   int[1] the number of code units used,
                *   int[2] is the number of code units remaining until the next message.
                *   int[3] is the encoding type that should be used for the message.
                */
                int msgCount = params[0];
                /** M; change 4.1 google default
                // if (msgCount > 1) {
                //    // The provider doesn't support multi-part sms's so as soon as the user types
                //    // an sms longer than one segment, we have to turn the message into an mms.
                 */
                if (msgCount >= MmsConfig.getSmsToMmsTextThreshold()) {
                    setLengthRequiresMms(true, false);
                } else {
                    updateState(HAS_ATTACHMENT, hasAttachment(), true);
                }
            } else {
                updateState(HAS_ATTACHMENT, hasAttachment(), true);
            }
        } else {
            // Set HAS_ATTACHMENT if we need it.
            updateState(HAS_ATTACHMENT, hasAttachment(), true);
        }
        correctAttachmentState();
        if (type != IMAGE && mActivity instanceof ComposeMessageActivity) {
            ((ComposeMessageActivity) mActivity).setWaitingAttachment(false);
        }
        return result;
    }

    /**
     * Returns true if this message contains anything worth saving.
     */
    public boolean isWorthSaving() {
        /// M:
        MmsLog.d(M_TAG, "isWorthSaving(): hasText()="
                + hasText() + ", hasSubject()=" + hasSubject()
                + ", hasAttachment()=" + hasAttachment() + ", hasSlideshow()=" + hasSlideshow());
        // If it actually contains anything, it's of course not empty.
        if (hasText() || hasSubject() || hasAttachment() || hasSlideshow()) {
            return true;
        }
        // When saveAsMms() has been called, we set FORCE_MMS to represent
        // sort of an "invisible attachment" so that the message isn't thrown
        // away when we are shipping it off to other activities.
        if (isFakeMmsForDraft()) {
            return true;
        }
        return false;
    }

    private void cancelThumbnailLoading() {
        int numSlides = mSlideshow != null ? mSlideshow.size() : 0;
        if (numSlides > 0) {
            ImageModel imgModel = mSlideshow.get(numSlides - 1).getImage();
            if (imgModel != null) {
                imgModel.cancelThumbnailLoading();
            }
        }
    }

    /**
     * Returns true if FORCE_MMS is set.
     * When saveAsMms() has been called, we set FORCE_MMS to represent
     * sort of an "invisible attachment" so that the message isn't thrown
     * away when we are shipping it off to other activities.
     */
    public boolean isFakeMmsForDraft() {
        return (mMmsState & FORCE_MMS) > 0;
    }

    /**
     * Makes sure mSlideshow is set up.
     */
    private void ensureSlideshow() {
        if (mSlideshow != null) {
            /// M: Code analyze 034, For new feature ALPS00231349,  add vCard support . @{
            if (mSlideshow.size() > 0) {
                return;
            } else {
                ///M: Modify for ALPS01250908
                try {
                    mSlideshow.add(new SlideModel(mSlideshow));
                } catch (ContentRestrictionException e) {
                    Log.e(TAG, "throw a ContentRestrictionException in ensureSlideshow");
                }
                return;
            }
            /// @}
        }

        SlideshowModel slideshow = SlideshowModel.createNew(mActivity);
        SlideModel slide = new SlideModel(slideshow);
        slideshow.add(slide);

        mSlideshow = slideshow;
    }

    /**
     * Change the message's attachment to the data in the specified Uri.
     * Used only for single-slide ("attachment mode") messages.
     */
    private void changeMedia(int type, Uri uri) throws MmsException {
        SlideModel slide = mSlideshow.get(0);

        MediaModel media;
        Uri uriTemp = null;
        if (slide == null) {
            Log.w(LogTag.TAG, "[WorkingMessage] changeMedia: no slides!");
            return;
        }

        /// M: add for attachment enhance @{
        /// M: If we're changing to text, just bail out. @{
        if (type == TEXT) {
            if (mSlideshow != null && mSlideshow.size() > 0) {
                /// M: Code analyze 034, For new feature ALPS00231349,  add vCard support. @{
                if (mOpWorkingMessageExt.removeAllAttachFiles()) {
                    mSlideshow.removeAllAttachFiles();
                }
                mSlideshow.clear();
                mSlideshow = null;
                ensureSlideshow();
                if (isResizeImage()) {
                    // Delete our MMS message, if there is one.
                    if (mMessageUri != null) {
                        asyncDelete(mMessageUri, null, null);
                        setResizeImage(false);
                    }
                }
            }
            return;
        }
        /// M: get thumbnail uri @{
        if (slide.hasImage()) {
            ImageModel imageModel = slide.getImage();
            uriTemp = Uri.parse(imageModel.getUri().toString() + ThumbnailManager.FLAG_FNAME
                + imageModel.getSrc());
        } else if (slide.hasVideo()) {
            VideoModel videoModel = slide.getVideo();
            uriTemp = Uri.parse(videoModel.getUri().toString() + ThumbnailManager.FLAG_FNAME
                + videoModel.getSrc());
        }
        /// @}

        // Make a correct MediaModel for the type of attachment.
        if (type == IMAGE) {
            media = new ImageModel(mActivity, uri, mSlideshow.getLayout().getImageRegion());
        } else if (type == VIDEO) {
            media = new VideoModel(mActivity, uri, mSlideshow.getLayout().getImageRegion());
        } else if (type == AUDIO) {
            media = new AudioModel(mActivity, uri);
        } else {
            throw new IllegalArgumentException("changeMedia type=" + type + ", uri=" + uri);
        }
        /// M: Code analyze 041, For new feature , add drm support . @{
        if (media.getMediaPackagedSize() < 0) {
            throw new ExceedMessageSizeException("Exceed message size limitation");
        }

        int increaseSize = media.getMediaPackagedSize()
                + MmsSizeUtils.getSlideshowReserveSize()
                - mSlideshow.getCurrentSlideshowSize();
        // OP01
        increaseSize = mOpWorkingMessageExt.changeMedia(increaseSize,
                mSlideshow.getAttachFilesPackagedSize());
        if (increaseSize > 0) {
            mSlideshow.checkMessageSize(increaseSize);
        }

        // Remove any previous attachments.
        removeSlideAttachments(slide);

        slide.add(media);

        if (uriTemp != null) {
            MmsApp.getApplication().getThumbnailManager().removeThumbnail(uriTemp);
        }
        /// @}
        // For video and audio, set the duration of the slide to that of the attachment.
        if (type == VIDEO || type == AUDIO) {
            slide.updateDuration(media.getDuration());
        }
    }

    /**
     * Add the message's attachment to the data in the specified Uri to a new slide.
     */
    private void appendMedia(int type, Uri uri) throws MmsException {

        // If we're changing to text, just bail out.
        if (type == TEXT) {
            return;
        }

        // The first time this method is called, mSlideshow.size() is going to be
        // one (a newly initialized slideshow has one empty slide). The first time we
        // attach the picture/video to that first empty slide. From then on when this
        // function is called, we've got to create a new slide and add the picture/video
        // to that new slide.
        boolean addNewSlide = true;
        if (mSlideshow.size() == 1 && !mSlideshow.isSimple()) {
            addNewSlide = false;
        }
        if (mSlideshow.size() == 1 && mSlideshow.get(0) != null
                && mSlideshow.get(0).hasAudio() && mSlideshow.get(0).hasImage()) {
            addNewSlide = true;
        }
        if (addNewSlide) {
            SlideshowEditor slideShowEditor = new SlideshowEditor(mActivity, mSlideshow);
            if (!slideShowEditor.addNewSlide()) {
                return;
            }
        }
        mIsUpdateAttachEditor = false;
        // Make a correct MediaModel for the type of attachment.
        MediaModel media;
        SlideModel slide = mSlideshow.get(mSlideshow.size() - 1);
        if (type == IMAGE) {
            media = new ImageModel(mActivity, uri, mSlideshow.getLayout().getImageRegion());

            String[] fileNames = mSlideshow.getAllMediaNames(MediaType.IMAGE);
            media.setSrc(MessageUtils.getUniqueName(fileNames, media.getSrc()));
        } else if (type == VIDEO) {
            media = new VideoModel(mActivity, uri, mSlideshow.getLayout().getImageRegion());

            String[] fileNames = mSlideshow.getAllMediaNames(MediaType.VIDEO);
            media.setSrc(MessageUtils.getUniqueName(fileNames, media.getSrc()));
        } else if (type == AUDIO) {
            media = new AudioModel(mActivity, uri);

            String[] fileNames = mSlideshow.getAllMediaNames(MediaType.AUDIO);
            media.setSrc(MessageUtils.getUniqueName(fileNames, media.getSrc()));
        } else {
            throw new IllegalArgumentException("changeMedia type=" + type + ", uri=" + uri);
        }

        if (media.getMediaPackagedSize() < 0) {
            throw new ExceedMessageSizeException("Exceed message size limitation");
        }

        slide.add(media);

        /// M: for vcard, since we append a media, remove vCard
        removeAllFileAttaches();

        // For video and audio, set the duration of the slide to
        // that of the attachment.
        if (type == VIDEO || type == AUDIO) {
            slide.updateDuration(media.getDuration());
        }
    }

    /**
     * Returns true if the message has an attachment (including slideshows).
     */
    public boolean hasAttachment() {
        return (mAttachmentType > TEXT);
    }

    /**
     * Returns the slideshow associated with this message.
     */
    public SlideshowModel getSlideshow() {
        return mSlideshow;
    }

    /**
     * Returns true if the message has a real slideshow, as opposed to just
     * one image attachment, for example.
     */
    public boolean hasSlideshow() {
        /// M: Code analyze 034, For new feature ALPS00231349,  add vCard support . @{
        return (mSlideshow != null && mSlideshow.size() > 1);
        /// @}
    }

    /**
     * Sets the MMS subject of the message.  Passing null indicates that there
     * is no subject.  Passing "" will result in an empty subject being added
     * to the message, possibly triggering a conversion to MMS.  This extra
     * bit of state is needed to support ComposeMessageActivity converting to
     * MMS when the user adds a subject.  An empty subject will be removed
     * before saving to disk or sending, however.
     */
    public void setSubject(CharSequence s, boolean notify) {
        /// M: @{
        boolean flag = ((s != null) && TextUtils.getTrimmedLength(s) > 0);
        mSubject = s;
        if (flag) {
            updateState(HAS_SUBJECT, flag, notify);
        } else {
            updateState(HAS_SUBJECT, flag, notify);
        }
        /// @}
        /// M Fix CR ALPS01141440 @{
        if (mSubject == null || mSubject.length() == 0) {
            if ((mActivity instanceof ComposeMessageActivity)
                    && ((ComposeMessageActivity) mActivity).getForwordingState()) {
                if (mMmsState == 0) {
                    asyncDelete(mMessageUri, null, null);
                    if (mConversation != null) {
                        clearConversation(mConversation, true);
                    }
                    mMessageUri = null;
                }
            }
        }
       /// @}
    }

    /**
     * Returns the MMS subject of the message.
     */
    public CharSequence getSubject() {
        return mSubject;
    }

    /**
     * Returns true if this message has an MMS subject. A subject has to be more than just
     * whitespace.
     * @return
     */
    public boolean hasSubject() {
        return mSubject != null && TextUtils.getTrimmedLength(mSubject) > 0;
    }

    /**
     * Moves the message text into the slideshow.  Should be called any time
     * the message is about to be sent or written to disk.
     */
    private synchronized void syncTextToSlideshow() {
        /// M: Because progress run async, so get the value first.
        boolean waitingAttachment = false;
        if (mActivity instanceof ComposeMessageActivity) {
            waitingAttachment = ((ComposeMessageActivity) mActivity).isWaitingAttachment();
        }

        /// M: fix bug ALPS01365426, must calculate mIsExceedSize
        if (mSlideshow != null && !TextUtils.isEmpty(mText)
                && TextUtils.getTrimmedLength(mText) >= 0) {
            int currentSize = getCurrentMessageSize();
            if (currentSize > MmsConfig.getUserSetMmsSizeLimit(true) && !waitingAttachment) {
                mIsExceedSize = true;
            }
        }
        /// @}

        if (mSlideshow == null || mSlideshow.size() != 1)
            return;
        try {
            SlideModel slide = mSlideshow.get(0);
            TextModel text;
            /// M: change google default. @{
            TextModel oldText = null;
            if (slide != null) {
                oldText = slide.getText();
            }
            /// M: fix bug ALPS01013522, workaround avoiding mSlideshow NPE
            MmsLog.d(TAG, "syncTextToSlideshow() mSlideshow = "
                    + String.valueOf(mSlideshow == null));
            // Add a TextModel to slide 0 if one doesn't already exist
            text = new TextModel(mActivity, ContentType.TEXT_PLAIN, "text_0.txt",
                    mSlideshow != null ? mSlideshow.getLayout().getTextRegion()
                                   : SlideshowModel.createNew(mActivity)
                                           .getLayout().getTextRegion(),
                    (!TextUtils.isEmpty(mText) &&
                            TextUtils.getTrimmedLength(mText) >= 0)
                                    ? (mText.toString()).getBytes() : null);

            //klocwork issue pid:18444
            if (slide != null) {
                int oldLength = (oldText != null && oldText.getText() != null)
                                ? oldText.getText().getBytes().length : 0;
                int newLength = mText != null ? mText.toString().getBytes().length : 0;
                if (newLength - oldLength > 0) {
                    if (getCurrentMessageSize() + (newLength - oldLength)
                                > MmsConfig.getUserSetMmsSizeLimit(true)) {
                        throw new ExceedMessageSizeException();
                    }
                }
                slide.add(text);
            }
        } catch (ExceedMessageSizeException e) {
            if (!TextUtils.isEmpty(mText) && TextUtils.getTrimmedLength(mText) >= 0
                    && !waitingAttachment) {
                mIsExceedSize = true;
            }
            return;
        } catch (ArrayIndexOutOfBoundsException e) {
            /// M: fix bug ALPS01659457
            Log.e(TAG, "syncTextToSlideshow ArrayIndexOutOfBoundsException");
            return;
        }
        /// @}
    }

    /**
     * Sets the message text out of the slideshow.  Should be called any time
     * a slideshow is loaded from disk.
     */
    private void syncTextFromSlideshow() {
        // Don't sync text for real slideshows.
        if (mSlideshow.size() != 1) {
            return;
        }

        SlideModel slide = mSlideshow.get(0);
        if (slide == null || !slide.hasText()) {
            return;
        }

        mText = slide.getText().getText();
    }

    /**
     * Removes the subject if it is empty, possibly converting back to SMS.
     */
    private void removeSubjectIfEmpty(boolean notify) {
        if (!hasSubject()) {
            setSubject(null, notify);
        }
    }

    /**
     * Gets internal message state ready for storage.  Should be called any
     * time the message is about to be sent or written to disk.
     */
    private void prepareForSave(boolean notify) {
        // Make sure our working set of recipients is resolved
        // to first-class Contact objects before we save.
        syncWorkingRecipients();

        if (requiresMms()) {
            ensureSlideshow();
            syncTextToSlideshow();
            /// M:
            removeSubjectIfEmpty(notify);
        }
    }

    /**
     * Resolve the temporary working set of recipients to a ContactList.
     */
    public void syncWorkingRecipients() {
        if (mWorkingRecipients != null) {
            ContactList recipients = ContactList.getByNumbers(mWorkingRecipients, false);
            mConversation.setRecipients(recipients);    // resets the threadId to zero
            /// M: google JB.MR1 patch, group mms
            setHasMultipleRecipients(recipients.size() > 1, true);
            mWorkingRecipients = null;
        }
    }

    public void updateStateForGroupMmsChanged() {
        ContactList recipients = mConversation.getRecipients();
        setHasMultipleRecipients(recipients.size() > 1, false);
    }

    public String getWorkingRecipients() {
        // this function is used for DEBUG only
        if (mWorkingRecipients == null) {
            return null;
        }
        ContactList recipients = ContactList.getByNumbers(mWorkingRecipients, false);
        return recipients.serialize();
    }

    // Call when we've returned from adding an attachment. We're no longer forcing the message
    // into a Mms message. At this point we either have the goods to make the message a Mms
    // or we don't. No longer fake it.
    public void removeFakeMmsForDraft() {
        updateState(FORCE_MMS, false, false);
    }

    /**
     * Force the message to be saved as MMS and return the Uri of the message.
     * Typically used when handing a message off to another activity.
     */
    /// M: add synchronized.
    public synchronized Uri saveAsMms(boolean notify) {
        if (DEBUG) LogTag.debug("saveAsMms mConversation=%s", mConversation);
        if (mActivity != null && !MmsConfig.isSmsEnabled(mActivity)) {
            Log.d(TAG_DRAFT, "[saveAsMms] Non-default sms, cann't save,just return!!");
            return mMessageUri;
        }
        // If we have discarded the message, just bail out.
        if (mDiscarded) {
            MmsLog.dpi(TAG, "saveAsMms mDiscarded: true mConversation: " + mConversation +
                    " returning NULL uri and bailing");
            return null;
        }

        // FORCE_MMS behaves as sort of an "invisible attachment", making
        // the message seem non-empty (and thus not discarded).  This bit
        // is sticky until the last other MMS bit is removed, at which
        // point the message will fall back to SMS.
        updateState(FORCE_MMS, true, notify);

        // Collect our state to be written to disk.
        prepareForSave(true /* notify */);

        if (!mConversation.getRecipients().isEmpty()) {
            mConversation.ensureThreadId();
        }
        mConversation.setDraftState(true);

        //MtkPduPersister persister = MtkPduPersister.getPduPersister(mActivity);
        MtkSendReq sendReq = makeMtkSendReq(mConversation, mSubject);

        sendReq = (MtkSendReq) mOpWorkingMessageExt.opSaveAsMms(sendReq);

        long threadId = mConversation.getThreadId();

        Log.d(TAG_DRAFT, "[saveAsMms] threadId : " + threadId);

        MmsDraftData mdd =
            DraftManager.getInstance().saveDraft(DraftManager.SYNC_SAVE_ACTION, threadId,
                mMessageUri, mSlideshow, sendReq, mActivity, null);
        if (mdd != null) {
//                if (mMessageUri == null) {
                mMessageUri = mdd.getMessageUri();
                if (mdd.getCreateOrUpdate() == 1) {
                    if (mMessageUri == null && !mdd.getBooleanResult()) {
                        Log.d(TAG_DRAFT, "[saveAsMms] MmsException happened,and save failed!");
                        removeAllFileAttaches();
                        removeAttachment(true);
                        mActivity.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                // TODO Auto-generated method stub
                                MessageUtils.showErrorDialog(mActivity,
                                                android.R.string.dialog_alert_title,
                                                R.string.error_add_attachment, 0,
                                                R.string.type_common_file);
                            }
                        });
                    }
                }
                MmsLog.dpi(TAG_DRAFT,
                        "[saveAsMms] call draft manager return , and mMessageUri : "
                        + mMessageUri);
                asyncDeleteDraftSmsMessage(mConversation);
//                }
        }

        /// M: google jb.mr1 patch
        // If we don't already have a Uri lying around, make a new one.  If we do
        // have one already, make sure it is synced to disk.

        mHasMmsDraft = true;
        return mMessageUri;
    }

    /// M : FIX CR : ALPS01795853
    /// While exist A MMS draft, then create a new composer, input A phone number
    /// and input some text, then add image from gallery, this will cause delete the
    /// MMS draft whcih saved before
    ///
    /// Root cause : while enter the document UI, composer will enter the onStop,
    ///              which will save the sms draft that the content is inputted text
    ///              after save finished, will delete the MMS draft @{
    public void setTruntoChooseAttach(boolean chooseOrNot) {
        Log.d(TAG_DRAFT, "[setTruntoChooseAttach] set to be : " + chooseOrNot);
        mIsTurnToChooseAttach = chooseOrNot;
    }
    /// @}

    /**
     * Save this message as a draft in the conversation previously specified
     * to {@link setConversation}.
     */
    public void saveDraft(final boolean isStopping) {
        if (mActivity != null && !MmsConfig.isSmsEnabled(mActivity)) {
            Log.d(TAG_DRAFT, "[saveDraft] not default sms, no need to save draft, just return!!");
            return;
        }
        // If we have discarded the message, just bail out.
        if (mDiscarded) {
            MmsLog.dpi(TAG_DRAFT, "saveDraft mDiscarded: true mConversation: " + mConversation +
                " skipping saving draft and bailing");
            return;
        }

        // Make sure setConversation was called.
        if (mConversation == null) {
            throw new IllegalStateException("saveDraft() called with no conversation");
        }

        if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "saveDraft for mConversation " + mConversation);
        }

        // Get ready to write to disk. But don't notify message status when saving draft
        prepareForSave(false /* notify */);

        if (requiresMms()) {
            asyncUpdateDraftMmsMessage(mConversation, isStopping, mInterface);
            mHasMmsDraft = true;

            /// M: Update state of the draft cache.
            mConversation.setDraftState(true);
        } else {
            String content = mText.toString();

            // bug 2169583: don't bother creating a thread id only to delete the thread
            // because the content is empty. When we delete the thread in updateDraftSmsMessage,
            // we didn't nullify conv.mThreadId, causing a temperary situation where conv
            // is holding onto a thread id that isn't in the database. If a new message arrives
            // and takes that thread id (because it's the next thread id to be assigned), the
            // new message will be merged with the draft message thread, causing confusion!
            if (!TextUtils.isEmpty(content)) {
                asyncUpdateDraftSmsMessage(mConversation, content);
                mHasSmsDraft = true;
            } else {
                // When there's no associated text message, we have to handle the case where there
                // might have been a previous mms draft for this message. This can happen when a
                // user turns an mms back into a sms, such as creating an mms draft with a picture,
                // then removing the picture.
                /// Which used to fix load draft from widget, the compose will flicker
                /// while flicking, this will delete mms draft @{
                //asyncDeleteDraftMmsMessage(mConversation);
                /// @}
                mMessageUri = null;
            }
        }
        // Update state of the draft cache.
        /// M: coomment google default.
        // mConversation.setDraftState(true);

    }

    /// M: Code analyze 035, For bug ALPS00095817,  delete draft . @{
    synchronized public void discard() {
        discard(true);
    }
    /// @}

    public void unDiscard() {
        if (DEBUG) LogTag.debug("unDiscard");

        mDiscarded = false;
    }

    /**
     * Returns true if discard() has been called on this message.
     */
    public boolean isDiscarded() {
        return mDiscarded;
    }

    /**
     * To be called from our Activity's onSaveInstanceState() to give us a chance
     * to stow our state away for later retrieval.
     *
     * @param bundle The Bundle passed in to onSaveInstanceState
     */
    public void writeStateToBundle(Bundle bundle) {
        mBundle = bundle;
        if (hasSubject()) {
            bundle.putString("subject", mSubject.toString());
        }

        if (mMessageUri != null) {
            bundle.putParcelable("msg_uri", mMessageUri);
        } else if (hasText()) {
            bundle.putString("sms_body", mText.toString());
        }
        /// M: fix bug ALPS00779871
        if (mMessageUri == null) {
            SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mActivity);
            SharedPreferences.Editor editor = sp.edit();
            editor.putString(SAVE_MSG_URI_KEY, "");
            editor.putLong(SAVE_MSG_THREADID_KEY, -1);
            editor.apply();
        }
    }

    /**
     * To be called from our Activity's onCreate() if the activity manager
     * has given it a Bundle to reinflate
     * @param bundle The Bundle passed in to onCreate
     */
    public void readStateFromBundle(Bundle bundle) {
        if (bundle == null) {
            return;
        }

        String subject = bundle.getString("subject");
        setSubject(subject, false);

        Uri uri = (Uri) bundle.getParcelable("msg_uri");
        ///M: fix bug: ALPS00568220,
        ///M: load message Uri from preference, because some times MMS will be killed by system
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mActivity);
        long threadId = sp.getLong(SAVE_MSG_THREADID_KEY, -1);
        if (uri == null && mConversation != null && mConversation.getThreadId() == threadId
                && threadId != -1) {
            String uriString = sp.getString(SAVE_MSG_URI_KEY, null);
            if (uriString != null && !uriString.equals("")) {
                uri = Uri.parse(uriString);
            }
        }
        SharedPreferences.Editor editor = sp.edit();
        editor.putString(SAVE_MSG_URI_KEY, "");
        editor.putLong(SAVE_MSG_THREADID_KEY, -1);
        editor.apply();

        if (uri != null) {
            threadId = 0L;
            if (mConversation != null) {
                threadId = mConversation.getThreadId();
                if (threadId <= 0 && !mConversation.getRecipients().isEmpty()) {
                    threadId = mConversation.ensureThreadId();
                }
            }

            MmsDraftData mdd = DraftManager.getInstance().loadDraft(DraftManager.SYNC_LOAD_ACTION,
                    threadId, uri, mActivity, null);
            if (mdd != null && mdd.getBooleanResult()) {
                mSlideshow = mdd.getSlideshow();
                mMessageUri = mdd.getMessageUri();
                syncTextFromSlideshow();
                correctAttachmentState();
            }
//            loadFromUri(uri);
            /// M: fix bug ALPS00779135
            if (mSlideshow != null) {
                mHasMmsDraft = true;
            }
            return;
        } else {
            String body = bundle.getString("sms_body");
            /// M: Code analyze 049, For bug ALPS00106234, to solve the
            /// "JE" pops up after you press Back key . @{
            if (body == null) {
                mText = "";
            } else {
                mText = body;
            }
            /// @}
        }
    }

    /**
     * Update the temporary list of recipients, used when setting up a
     * new conversation.  Will be converted to a ContactList on any
     * save event (send, save draft, etc.)
     */
    public void setWorkingRecipients(List<String> numbers) {
        mWorkingRecipients = numbers;
        String s = null;
        if (numbers != null) {
            int size = numbers.size();
            switch (size) {
            case 1:
                s = numbers.get(0);
                break;
            case 0:
                s = "empty";
                break;
            default:
                s = "{...} len=" + size;
            }
        }
        /// M:
        //MmsLog.dpi(TAG, "setWorkingRecipients: numbers=" + s);
    }

    private void dumpWorkingRecipients() {
        Log.i(TAG, "-- mWorkingRecipients:");

        if (mWorkingRecipients != null) {
            int count = mWorkingRecipients.size();
            for (int i = 0; i < count; i++) {
                Log.d(TAG, "   [" + i + "] " + mWorkingRecipients.get(i));
            }
            Log.i(TAG, "");
        }
    }

    public void dump() {
        Log.i(TAG, "WorkingMessage:");
        dumpWorkingRecipients();
        if (mConversation != null) {
            MmsLog.dpi(TAG, "mConversation: " + mConversation.toString());
        }
    }

    /**
     * Set the conversation associated with this message.
     */
    public void setConversation(Conversation conv) {
        if (DEBUG) {
            MmsLog.dpi(TAG, "setConversation " + mConversation + " -> " + conv);
        }

        mConversation = conv;

        // Convert to MMS if there are any email addresses in the recipient list.
        if (conv != null) {
            ContactList contactList = conv.getRecipients();
            setHasEmail(contactList.containsEmail(), false);
            // / M: google JB.MR1 patch, group mms
            setHasMultipleRecipients(contactList.size() > 1, false);
        }
    }

    public Conversation getConversation() {
        return mConversation;
    }

    /**
     * Hint whether or not this message will be delivered to an
     * an email address.
     */
    public void setHasEmail(boolean hasEmail, boolean notify) {
        if (MmsConfig.getEmailGateway() != null) {
            updateState(RECIPIENTS_REQUIRE_MMS, false, notify);
        } else {
            updateState(RECIPIENTS_REQUIRE_MMS, hasEmail, notify);
        }
    }
    /** google JB.MR1 patch, group mms
     * Set whether this message will be sent to multiple recipients. This is a hint whether the
     * message needs to be sent as an mms or not. If MmsConfig.getGroupMmsEnabled is false, then
     * the fact that the message is sent to multiple recipients is not a factor in determining
     * whether the message is sent as an mms, but the other factors (such as, "has a picture
     * attachment") still hold true.
     */
    public void setHasMultipleRecipients(boolean hasMultipleRecipients, boolean notify) {
        updateState(MULTIPLE_RECIPIENTS,
                hasMultipleRecipients &&
                MmsPreferenceActivity.getIsGroupMmsEnabled(mActivity),
                notify);
    }

    /**
     * Returns true if this message would require MMS to send.
     */
    public boolean requiresMms() {
        return (mMmsState > 0);
    }

    /**
     * Set whether or not we want to send this message via MMS in order to
     * avoid sending an excessive number of concatenated SMS messages.
     * @param: mmsRequired is the value for the LENGTH_REQUIRES_MMS bit.
     * @param: notify Whether or not to notify the user.
     */
    public void setLengthRequiresMms(boolean mmsRequired, boolean notify) {
        updateState(LENGTH_REQUIRES_MMS, mmsRequired, notify);
    }

    private static String stateString(int state) {
        if (state == 0)
            return "<none>";

        StringBuilder sb = new StringBuilder();
        if ((state & RECIPIENTS_REQUIRE_MMS) > 0)
            sb.append("RECIPIENTS_REQUIRE_MMS | ");
        if ((state & HAS_SUBJECT) > 0)
            sb.append("HAS_SUBJECT | ");
        if ((state & HAS_ATTACHMENT) > 0)
            sb.append("HAS_ATTACHMENT | ");
        if ((state & LENGTH_REQUIRES_MMS) > 0)
            sb.append("LENGTH_REQUIRES_MMS | ");
        if ((state & FORCE_MMS) > 0)
            sb.append("FORCE_MMS | ");
        /// M: google JB.MR1 patch, group mms
        if ((state & MULTIPLE_RECIPIENTS) > 0)
            sb.append("MULTIPLE_RECIPIENTS | ");

        if (sb.length() > 3) {
            sb.delete(sb.length() - 3, sb.length());
        }
        return sb.toString();
    }

    /**
     * Sets the current state of our various "MMS required" bits.
     *
     * @param state The bit to change, such as {@link HAS_ATTACHMENT}
     * @param on If true, set it; if false, clear it
     * @param notify Whether or not to notify the user
     */
    private void updateState(int state, boolean on, boolean notify) {
        /// M:
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.i(TAG, "WorkingMessage.updateState(" + state + ", " + on + ", " + notify + ")");
        }
        if (!sMmsEnabled) {
            // If Mms isn't enabled, the rest of the Messaging UI should not be using any
            // feature that would cause us to to turn on any Mms flag and show the
            // "Converting to multimedia..." message.
            return;
        }
        int oldState = mMmsState;
        if (on) {
            mMmsState |= state;
            /// M: Code analyze 048, For bug ALPS00338410, The message only with recipients
            /// should not be saved as draft display in the all
            /// messages list after you press "Back" . @{
            if ((mMmsState & ~FORCE_MMS) > 0) {
                mMmsState = (mMmsState & ~FORCE_MMS);
            }
            /// @}
        } else {
            mMmsState &= ~state;
        }

        // If we are clearing the last bit that is not FORCE_MMS,
        // expire the FORCE_MMS bit.
        if (mMmsState == FORCE_MMS && ((oldState & ~FORCE_MMS) > 0)) {
            mMmsState = 0;
        }
        // Notify the listener if we are moving from SMS to MMS
        // or vice versa.
        if (notify) {
            if (oldState == 0 && mMmsState != 0) {
                /// M: Code analyze 044, For bug ALPS00050082, add toast . @{
                mStatusListener.onProtocolChanged(true, true);
                /// @}
            } else if (oldState != 0 && mMmsState == 0) {
                /// M: Code analyze 044, For bug ALPS00050082, add toast . @{
                mStatusListener.onProtocolChanged(false, true);
                /// @}
            }
        }

        if (oldState != mMmsState) {
            MmsLog.d(TAG, stateString(mMmsState) + mMmsState);
            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) LogTag.debug("updateState: %s%s = %s",
                    on ? "+" : "-",
                    stateString(state), stateString(mMmsState));
        }
    }

    /**
     * Send this message over the network.  Will call back with onMessageSent() once
     * it has been dispatched to the telephony stack.  This WorkingMessage object is
     * no longer useful after this method has been called.
     *
     * @throws ContentRestrictionException if sending an MMS and uaProfUrl is not defined
     * in mms_config.xml.
     * M: Code analyze 047, For new feature ALPS00316567,
     * extend this method for msim, add a parameter for subId
     */
    public void send(final String recipientsInUI, final int subId) {
        MmsLog.d(MmsApp.TXN_TAG, "Enter send(). subId = " + subId);
        final long origThreadId = mConversation.getThreadId();

        if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            LogTag.debug("send origThreadId: " + origThreadId);
        }

        removeSubjectIfEmpty(true /* notify */);

        // Get ready to write to disk.
        prepareForSave(true /* notify */);

        // We need the recipient list for both SMS and MMS.
        final Conversation conv = mConversation;
        final String msgTxt = mText.toString();
        final SplitToMmsAndSmsConversation spliter =
            new SplitToMmsAndSmsConversation(conv, msgTxt);

         if (spliter.getMMSConversation() != null) {
            // uaProfUrl setting in mms_config.xml must be present to send an MMS.
            // However, SMS service will still work in the absence of a uaProfUrl address.
            if (MmsConfig.getUaProfUrl() == null) {
                String err = "WorkingMessage.send MMS sending failure. mms_config.xml is " +
                        "missing uaProfUrl setting.  uaProfUrl is required for MMS service, " +
                        "but can be absent for SMS.";
                RuntimeException ex = new NullPointerException(err);
                Log.e(TAG, err, ex);
                // now, let's just crash.
                throw ex;
            }

            // Make local copies of the bits we need for sending a message,
            // because we will be doing it off of the main thread, which will
            // immediately continue on to resetting some of this state.
            final Uri mmsUri = mMessageUri;
            final MtkPduPersister persister = MtkPduPersister.getPduPersister(mActivity);

            final SlideshowModel slideshow = mSlideshow;
            final CharSequence subject = mSubject;

            if (Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
                MmsLog.dpi(TAG, "Send mmsUri: " + mmsUri);
            }

            // Do the dirty work of sending the message off of the main UI thread.
            new Thread(new Runnable() {
                public void run() {
                    MtkSendReq sendReq = makeMtkSendReq(spliter.getMMSConversation(), subject);

                    sendReq = (MtkSendReq) mOpWorkingMessageExt.opSendThreadRun(sendReq);

                    // Make sure the text in slide 0 is no longer holding onto a reference to
                    // the text in the message text box.
                    slideshow.prepareForSend();
                    /// M: Code analyze 047, For new feature ALPS00316567,
                    /// add a parameter for gemini . @{
                    if (spliter.getSMSConversation() != null) {
                        ///M: Message got split into SMS & MMS. Need to
                        /// set the mmsuri as NULL to again update it.
                        sendMmsWorker(spliter.getMMSConversation(),
                                null, persister, slideshow, sendReq, subId);
                    }
                    else {
                        sendMmsWorker(spliter.getMMSConversation(),
                                mmsUri, persister, slideshow, sendReq, subId);
                    }
                    /// @}

                    updateSendStats(spliter.getMMSConversation());
                }
            }, "WorkingMessage.send MMS").start();
            // update the Recipient cache with the new to address, if it's different
            RecipientIdCache.updateNumbers(spliter.getMMSConversation().getThreadId(),
                    spliter.getMMSConversation().getRecipients());
        }
        if (spliter.getSMSConversation() != null) {
            new Thread(new Runnable() {
                public void run() {
                    /// M: Code analyze 047, For new feature ALPS00316567,
                    /// add a parameter for msim . @{
                    boolean ecc_ims = false;
                    IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                            .getService("phoneEx"));

                    if (iTelEx != null) {
                        try {
                            ecc_ims = iTelEx.getIsLastEccIms();
                            if (ecc_ims) {
                                iTelEx.exitEmergencyCallbackMode(subId);
                                MmsLog.d(TAG, "sendMessage ecc_ims->1");
                            }
                        } catch (RemoteException e) {
                            MmsLog.e(TAG, "RemoteException getIsLastEccIms()");
                        }
                    }

                    preSendSmsWorker(spliter.getSMSConversation(), msgTxt, recipientsInUI, subId,
                            (spliter.getMMSConversation() != null) ? true : false);
                    /// @}
                    updateSendStats(spliter.getSMSConversation());
                }
            }, "WorkingMessage.send SMS").start();
            // update the Recipient cache with the new to address, if it's different
            RecipientIdCache.updateNumbers(spliter.getSMSConversation().getThreadId(),
                    spliter.getSMSConversation().getRecipients());
        }

        // Mark the message as discarded because it is "off the market" after being sent.\
        /// M: comment google default.
        // mDiscarded = true;
    }

    // Check for the Partial Compliant conditions
    private boolean isPartiallySMSCompliant() {
         boolean result = false;
         if (((mMmsState & FORCE_MMS) == 0)
             // NO slideshow that will convert the whole message for the whole
             // recipients into MMS
         && ((mMmsState & LENGTH_REQUIRES_MMS) == 0)
             // DO NOT exceed a particular threshold of SMS truncation that
             // will convert the whole message for the whole recipients into
             // MMS
         && ((mMmsState & HAS_ATTACHMENT) == 0)
             // NO attachment that will convert the whole message for the
             // whole recipients into MMS
         && ((mMmsState & HAS_SUBJECT) == 0)) {
             // NO subject that will convert the whole message for the whole
             // recipients into MMS
             // DO NOT check RECIPIENTS_REQUIRE_MMS to allow email & alias as
             // recipients
            result = true;
             // Conversation where MMS could not be mandatory for some
             // recipients : aka phone numbers.
        }
        return result;
    }

    // set conversation and split it into SMS & MMS conversation
    private class SplitToMmsAndSmsConversation implements ISplitToMmsAndSmsConversationCallback {
        private Conversation mSmsConv;

        private Conversation mMmsConv;

        private ContactList listSms;

        private ContactList listMms;

        // Split if necessary the initial conversation in one MMS conversation
        // or/and one SMS conversation
        // Allow to send SMS with phone numbers recipients when the conversation
        // is still SMS compliant.

        public SplitToMmsAndSmsConversation(final Conversation conv, final String msgTxt) {
            mMmsConv = null;
            mSmsConv = null;
            Log.d(TAG, "setConversationToSplitAsSmsMms");

            if (!mOpWorkingMessageExt.onCreateSplitToMmsAndSmsConv(this, conv)) {
                if (requiresMms() || addressContainsEmailToMms(conv, msgTxt)) {
                    mMmsConv = conv;
                } else {
                    mSmsConv = conv;
                }
                return;
            }
        }

        private void splitConv(final Conversation conv) {
            ContactList list = null;
            Contact contact = null;
            String number = null;
            listSms = null;
            listMms = null;

            // Conversation with SMS/MMS Split is supported
            if (conv != null) {
                list = conv.getRecipients();
                if (list != null) {
                    if (requiresMms()) {
                        if (isPartiallySMSCompliant()) {
                            for (int i = 0; i < list.size(); i++) {
                                contact = list.get(i);
                                if (contact != null) {
                                    number = contact.getNumber();
                                    if (!TextUtils.isEmpty(number)) {
                                        if (Mms.isPhoneNumber(number)) {
                                            addContactAsSms(contact);
                                        } else {
                                            addContactAsMms(contact);
                                        }
                                    } else {
                                        addContactAsMms(contact);
                                    }
                                } // contact
                            }
                        } else {
                            listMms = list;
                        } // isPartiallySMSCompliant
                    } else {
                        listSms = list;
                    } // requiresMms

                    if (listMms != null) {
                        mMmsConv = conv;
                        mMmsConv.setRecipients(listMms);
                        if (listSms != null) {
                            mSmsConv = Conversation.createNew(mActivity.getApplicationContext());
                            mSmsConv.setRecipients(listSms);
                        }
                    } else if (listSms != null) {
                        mSmsConv = conv;
                        mSmsConv.setRecipients(listSms);
                    }
                }
            }
        }

        // Add contact in MMS recipient list
        private void addContactAsMms(Contact contact) {
            if (listMms == null) {
                listMms = new ContactList();
            }
            if (!listMms.contains(contact)) {
                listMms.add(contact);
            }
        }

        // Add contact in MMS recipient list
        private void addContactAsSms(Contact contact) {
            if (listSms == null) {
                listSms = new ContactList();
            }
            if (!listSms.contains(contact)) {
                listSms.add(contact);
            }
        }

        protected Conversation getSMSConversation() {
            return mSmsConv;
        }

        protected Conversation getMMSConversation() {
            return mMmsConv;
        }

        /// M: @{
        public void splitConvCallback(final IConversationCallback conv) {
            splitConv((Conversation) conv);
        }
        /// @}
    }

    // Be sure to only call this on a background thread.
    private void updateSendStats(final Conversation conv) {
        String[] dests = conv.getRecipients().getNumbers();
        final ArrayList<String> phoneNumbers = new ArrayList<String>(Arrays.asList(dests));

        DataUsageStatUpdater updater = new DataUsageStatUpdater(mActivity);
        updater.updateWithPhoneNumber(phoneNumbers);
    }

    private boolean addressContainsEmailToMms(Conversation conv, String text) {
        if (MmsConfig.getEmailGateway() != null) {
            String[] dests = conv.getRecipients().getNumbers();
            int length = dests.length;
            for (int i = 0; i < length; i++) {
                if (Mms.isEmailAddress(dests[i]) || MessageUtils.isAlias(dests[i])) {
                    String mtext = dests[i] + " " + text;
                    int[] params = SmsMessage.calculateLength(mtext, false);
                    if (params[0] > 1) {
                        updateState(RECIPIENTS_REQUIRE_MMS, true, true);
                        ensureSlideshow();
                        syncTextToSlideshow();
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // Message sending stuff
    /// M: Code analyze 047, For new feature ALPS00316567, add a parameter for msim . @{
    private void preSendSmsWorker(Conversation conv, String msgText, String recipientsInUI,
            int subId, boolean hasBeenSplit) {
    /// @}
        // If user tries to send the message, it's a signal the inputted text is what they wanted.
        UserHappinessSignals.userAcceptedImeText(mActivity);

        mStatusListener.onPreMessageSent();

        /// M: Mark the message as discarded because it is "off the market" after being sent.
        mDiscarded = true;

        long origThreadId = conv.getThreadId();

        if (mForceUpdateThreadId) {
            conv.setNeedForceUpdateThreadId(true);
            mForceUpdateThreadId = false;
        }

        if (mForceUpdateThreadId) {
            conv.setNeedForceUpdateThreadId(true);
            mForceUpdateThreadId = false;
        }

        // Make sure we are still using the correct thread ID for our recipient set.
        long threadId = conv.ensureThreadId();

        String semiSepRecipients = conv.getRecipients().serialize();

        // recipientsInUI can be empty when the user types in a number and hits send
        if (LogTag.SEVERE_WARNING && ((origThreadId != 0 && origThreadId != threadId) ||
               ((!hasBeenSplit) && (!semiSepRecipients.equals(recipientsInUI))
                       && !TextUtils.isEmpty(recipientsInUI)))) {
            String msg = origThreadId != 0 && origThreadId != threadId ?
                    "WorkingMessage.preSendSmsWorker threadId changed or " +
                    "recipients changed. origThreadId: " +
                    origThreadId + " new threadId: " + threadId +
                    " also mConversation.getThreadId(): " +
                    mConversation.getThreadId()
                :
                    "Recipients in window: \"" +
                    recipientsInUI + "\" differ from recipients from conv: \"" +
                    semiSepRecipients + "\"";

            //LogTag.warnPossibleRecipientMismatch(msg, mActivity);
        }

        // just do a regular send. We're already on a non-ui thread so no need to fire
        // off another thread to do this work.
        /// M: Code analyze 047, For new feature ALPS00316567, add a parameter for msim . @{
        sendSmsWorker(msgText, semiSepRecipients, threadId, subId);
        /// @}

        // Be paranoid and clean any draft SMS up.
        deleteDraftSmsMessage(threadId);
        /// M Fix CR:ALPS01268191 which is google issue. while save a mms draft, enter the
        /// draft, change the recipients to be phone number, and send the sms. @{
        Log.d(TAG_DRAFT, "[preSendSmsWorker] mOldThreadId : " + mOldThreadId);
        if (mOldThreadId != 0) {
//            if (mOldThreadId != threadId) {
                asyncDeleteDraftMmsMessage(mOldThreadId);
//            }
        }
        /// @}
    }

    /// M: Code analyze 047, For new feature ALPS00316567, add a parameter for msim . @{
    private void sendSmsWorker(String msgText,
            String semiSepRecipients, long threadId, int subId) {
    /// @}
        String[] dests = TextUtils.split(semiSepRecipients, ";");
        if (LogTag.VERBOSE || Log.isLoggable(LogTag.TRANSACTION, Log.VERBOSE)) {
            MmsLog.dpi(LogTag.TRANSACTION, "sendSmsWorker sending message: recipients=" +
                    semiSepRecipients + ", threadId=" + threadId);
        }
        MessageSender sender = new SmsMessageSender(mActivity, dests, msgText, threadId, subId);

        try {
            sender.sendMessage(threadId);

            // Make sure this thread isn't over the limits in message count
            Recycler.getSmsRecycler().deleteOldMessagesByThreadId(mActivity, threadId);
        } catch (Exception e) {
            Log.e(TAG, "Failed to send SMS message, threadId=" + threadId, e);
        }

        mStatusListener.onMessageSent();
        MmsWidgetProvider.notifyDatasetChanged(mActivity);
    }

    /// M: Code analyze 047, For new feature ALPS00316567, add a parameter for msim . @{
    private void sendMmsWorker(Conversation conv, Uri mmsUri, MtkPduPersister persister,
            SlideshowModel slideshow, MtkSendReq sendReq, int subId) {
    /// @}
        long threadId = 0;
        Cursor cursor = null;
        boolean newMessage = false;
        try {
            // Put a placeholder message in the database first
            mStatusListener.onPreMessageSent();

            // Make sure we are still using the correct thread ID for our
            // recipient set.
            if (mForceUpdateThreadId) {
                conv.setNeedForceUpdateThreadId(true);
                mForceUpdateThreadId = false;
            }
            threadId = conv.ensureThreadId();
            conv.setNeedForceUpdateThreadId(false);

            MmsLog.dpi(TAG, "sendMmsWorker: update draft MMS message " + mmsUri +
                        " threadId: " + threadId);

            // One last check to verify the address of the recipient.
            String[] dests = conv.getRecipients().getNumbers(true /* scrub for MMS address */);
            if (dests.length == 1) {
                // verify the single address matches what's in the database. If we get a different
                // address back, jam the new value back into the MtkSendReq.
                String newAddress =
                    Conversation.verifySingleRecipient(mActivity, conv.getThreadId(), dests[0]);

                if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                    MmsLog.dpi(TAG, "sendMmsWorker: newAddress " + newAddress +
                            " dests[0]: " + dests[0]);
                }

                if (!newAddress.equals(dests[0])) {
                    dests[0] = newAddress;
                    EncodedStringValue[] encodedNumbers = EncodedStringValue.encodeStrings(dests);
                    if (encodedNumbers != null) {
                        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                            LogTag.debug("sendMmsWorker: REPLACING number!!!");
                        }
                        sendReq.setTo(encodedNumbers);
                    }
                }
            }
            newMessage = mmsUri == null;
            if (newMessage) {
                // check if a draft is already present for current thread
                final String selection = Mms.THREAD_ID + " = " + threadId;
                cursor = SqliteWrapper.query(mActivity, mContentResolver, Mms.Draft.CONTENT_URI,
                        MMS_DRAFT_PROJECTION, selection, null, null);
                if (cursor == null) {
                    Log.d(TAG, "[readDraftMmsMessage] cursor is null");
                }
                try {
                    if (cursor != null) {
                        if (cursor.moveToFirst()) {
                            Uri uri = ContentUris.withAppendedId(Mms.Draft.CONTENT_URI,
                                    cursor.getLong(MMS_ID_INDEX));
                            Log.d(TAG, "retrieved uri = " + uri);
                            mMessageUri = mmsUri = uri;
                            newMessage = mmsUri == null;
                        }
                    }
                } finally {
                    if (cursor != null) {
                        cursor.close();
                    }
                }

            }
            if (newMessage) {
                // Write something in the database so the new message will appear as sending
                ContentValues values = new ContentValues();
                values.put(Mms.MESSAGE_BOX, Mms.MESSAGE_BOX_OUTBOX);
                values.put(Mms.THREAD_ID, threadId);
                values.put(Mms.MESSAGE_TYPE, MtkPduHeaders.MESSAGE_TYPE_SEND_REQ);
                /// M: Code analyze 047, For new feature ALPS00316567, add a parameter for msim. @{
                values.put(Telephony.BaseMmsColumns.SUBSCRIPTION_ID, subId);
                /// @}
                mmsUri = SqliteWrapper.insert(mActivity, mContentResolver, Mms.Outbox.CONTENT_URI,
                        values);
            }
//            mStatusListener.onMessageSent();

            // If user tries to send the message, it's a signal the inputted text is
            // what they wanted.
            UserHappinessSignals.userAcceptedImeText(mActivity);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        MmsDraftData res = DraftManager.getInstance()
                .saveDraft(DraftManager.SYNC_SAVE_ACTION, threadId,
                mmsUri, slideshow, sendReq, mActivity, null);
        if (res != null) {
            if (res.getMessageUri() != null) {
                MmsLog.dpi(TAG_DRAFT, "[sendMmsWorker] mmsUir : " + mmsUri);
                mmsUri = res.getMessageUri();
            }
        }
        /*if (newMessage) {
            // Create a new MMS message if one hasn't been made yet.
            mmsUri = createDraftMmsMessage(persister, sendReq, slideshow, mmsUri,
                    mActivity);
        } else {
            // Otherwise, sync the MMS message in progress to disk.
            updateDraftMmsMessage(mmsUri, persister, slideshow, sendReq);
        }*/

        // Be paranoid and clean any draft SMS up.
        deleteDraftSmsMessage(threadId);

        if (isAbortSendingMmsByFdnList(conv, subId)) {
            MmsLog.d(TAG, "Abort Sending By FDN");
            if (!mmsUri.toString().startsWith(Mms.Draft.CONTENT_URI.toString())) {
                ContentValues values = new ContentValues();
                values.put(PendingMessages.PROTO_TYPE, MmsSms.MMS_PROTO);
                values.put(PendingMessages.MSG_ID, ContentUris.parseId(mmsUri));
                values.put(PendingMessages.MSG_TYPE, MtkPduHeaders.MESSAGE_TYPE_SEND_REQ);
                values.put(PendingMessages.ERROR_TYPE, 0);
                values.put(PendingMessages.ERROR_CODE, 0);
                values.put(PendingMessages.RETRY_INDEX, 0);
                values.put(PendingMessages.DUE_TIME, 0);

                SqliteWrapper.insert(mActivity, mActivity.getContentResolver(),
                        PendingMessages.CONTENT_URI, values);
            }
            markMmsMessageWithError(mmsUri, subId);
            MessagingNotification.notifySendFailed(mActivity, true);

            /// M : Fix CR : ALPS01798784, while after send the MMS, lock the screen
            /// the conversation message count will set to be 0, and the threadid will be
            /// set to be 0, so while unlock the screen,
            /// the message will not show in the message list @{
            mStatusListener.onMessageSent();
            /// @}

            // popup toast in a new thread will cause fd leak, change to send broadcast
            Intent intent = new Intent();
            intent.setAction(MmsApp.ACTION_MMS_TOAST);
            intent.putExtra("toast_type", MmsApp.FDN_CHECK_FAILURE);
            mActivity.getApplicationContext().sendBroadcast(intent);
            return;
        }

        /// M: Mark the message as discarded because it is "off the market" after being sent.
        mDiscarded = true;

        try {
            MessageSender sender = new MmsMessageSender(mActivity, mmsUri,
                    slideshow.getCurrentSlideshowSize(), subId);
            if (!sender.sendMessage(threadId)) {
                // The message was sent through SMS protocol, we should
                // delete the copy which was previously saved in MMS drafts.
                SqliteWrapper.delete(mActivity, mContentResolver, mmsUri, null, null);
            }
            /// M: add for fix issue ALPS00804679 @{
            else {
                Log.d(TAG, "sendMmsWorker: sendMessage success, mark draft false. threadId = "
                        + threadId);
                conv.setDraftState(false);
            }
            ///@}

            /// M : Fix CR : ALPS01798784, while after send the MMS, lock the screen
            /// the conversation message count will set to be 0, and the threadid will be
            /// set to be 0, so while unlock the screen,
            /// the message will not show in the message list @{
            mStatusListener.onMessageSent();
            /// @}

            // Make sure this thread isn't over the limits in message count
            Recycler.getMmsRecycler().deleteOldMessagesByThreadId(mActivity, threadId);
        } catch (Exception e) {
            Log.e(TAG, "Failed to send message: " + mmsUri + ", threadId=" + threadId, e);
        }
        MmsWidgetProvider.notifyDatasetChanged(mActivity);
    }

    private boolean isAbortSendingMmsByFdnList(Conversation conv, int subId) {
        boolean isFdnEnabled = false;
        try {
            IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                    .getService("phoneEx"));
            if (telephonyEx != null && telephonyEx.isFdnEnabled(subId)) {
                isFdnEnabled = true;
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[isAbortSendingMmsByFdnList]catch exception:");
            e.printStackTrace();
            isFdnEnabled = false;
        }
        Log.d(TAG, "isAbortSendingMmsByFdnList, isFdnEnabled = " + isFdnEnabled);
        if (!isFdnEnabled) {
            return false;
        } else {
            Uri uri = null;
            ContactList contactlist = conv.getRecipients();
            String[] numbers = contactlist.getNumbers();
            uri = Uri.parse(FDN_URI + subId);

            Cursor cursor = SqliteWrapper.query(mActivity, mActivity.getContentResolver(),
                    uri, FDN_PROJECTION, null, null, null);
            ArrayList<String> fdnList = new ArrayList<String>();
            if (cursor != null) {
                try {
                    while (cursor.moveToNext()) {
                        String number = cursor.getString(FDN_COLUMN_NUMBER);
                        if (number != null) {
                            fdnList.add(number);
                        }
                    }
                } finally {
                    cursor.close();
                }
            }
            for (String contactNumber : numbers) {
                MmsLog.dpi(TAG, "isAbortSending, contactNumber = " + contactNumber);
                if (!isInStringArray(contactNumber, fdnList)) {
                    return true;
                }
            }
            return false;
        }
    }

    private boolean isInStringArray(String string, ArrayList<String> arrayList) {
        for (String oneString : arrayList) {
            if (string.equals(oneString)) {
                return true;
            }
        }
        return false;
    }

    private void markMmsMessageWithError(Uri mmsUri, int subId) {
        try {
            MtkPduPersister p = MtkPduPersister.getPduPersister(mActivity);
            // Move the message into MMS Outbox. A trigger will create an entry in
            // the "pending_msgs" table.
            p.move(mmsUri, Mms.Outbox.CONTENT_URI);

            // Now update the pending_msgs table with an error for that new item.
            /// M Fix CR ALPS00584603
            /// update pending_sim_id column which in pending_msgs table @{
            ContentValues valuePendingTable = new ContentValues(2);
            valuePendingTable.put(PendingMessages.ERROR_TYPE, MmsSms.ERR_TYPE_GENERIC_PERMANENT);
            valuePendingTable.put(Telephony.MmsSms.PendingMessages.SUBSCRIPTION_ID, subId);
            long msgId = ContentUris.parseId(mmsUri);
            SqliteWrapper.update(mActivity, mContentResolver,
                    PendingMessages.CONTENT_URI,
                    valuePendingTable, PendingMessages.MSG_ID + "=" + msgId, null);
            /// @}
            /// M update sim_id column in pdu table @{
            ContentValues valuePduTable = new ContentValues(1);
            valuePduTable.put(Telephony.BaseMmsColumns.SUBSCRIPTION_ID, subId);
            valuePduTable.put(Mms.READ, 0);
            SqliteWrapper.update(mActivity, mContentResolver, mmsUri, valuePduTable, null, null);
            /// @}
        } catch (MmsException e) {
            // Not much we can do here. If the p.move throws an exception, we'll just
            // leave the message in the draft box.
            Log.e(TAG, "Failed to move message to outbox and mark as error: " + mmsUri, e);
        }
    }

    /**
     * makeMtkSendReq should always return a non-null MtkSendReq, whether the dest addresses are
     * valid or not.
     */
    private static MtkSendReq makeMtkSendReq(Conversation conv, CharSequence subject) {
        /// M: change google default, @{
        // String[] dests = conv.getRecipients().getNumbers(true /* scrub for MMS address */);
        String[] dests = conv.getRecipients().getNumbers(false /*don't scrub for MMS address */);
        // @}

        MtkSendReq req = new MtkSendReq();
        EncodedStringValue[] encodedNumbers = EncodedStringValue.encodeStrings(dests);
        if (encodedNumbers != null) {
            req.setTo(encodedNumbers);
        }

        if (!TextUtils.isEmpty(subject)) {
            req.setSubject(new EncodedStringValue(subject.toString()));
        }

        req.setDate(System.currentTimeMillis() / 1000L);

        return req;
    }

    private void asyncUpdateDraftMmsMessage(final Conversation conv,
            final boolean isStopping, final IDraftInterface callback) {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "asyncUpdateDraftMmsMessage conv= " + conv + " mMessageUri= "
                                + mMessageUri);
        }

        MtkSendReq sendReq = makeMtkSendReq(conv, mSubject);

        sendReq = (MtkSendReq) mOpWorkingMessageExt.opAsyncUpdateDraftMmsMessage(sendReq);

        long threadId = conv.getThreadId();

        Log.d(TAG_DRAFT, "[asyncUpdateDraftMmsMessage] before thread id : " + threadId);
        if (threadId <= 0 && !conv.getRecipients().isEmpty()) {
            threadId = conv.ensureThreadId();
            Log.d(TAG_DRAFT, "[asyncUpdateDraftMmsMessage] after thread id : " + threadId);
        }

        DraftManager.getInstance().saveDraft(DraftManager.ASYNC_SAVE_ACTION, threadId,
                mMessageUri, mSlideshow, sendReq, mActivity, callback);
    }

    private static final String SMS_DRAFT_WHERE = Sms.TYPE + "=" + Sms.MESSAGE_TYPE_DRAFT;
    private static final String[] SMS_BODY_PROJECTION = { Sms.BODY };
    private static final int SMS_BODY_INDEX = 0;

    /**
     * Reads a draft message for the given thread ID from the database,
     * if there is one, deletes it from the database, and returns it.
     * @return The draft message or an empty string.
     */
    private String readDraftSmsMessage(Conversation conv) {
        long thread_id = conv.getThreadId();
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "readDraftSmsMessage conv: " + conv);
        }
        // If it's an invalid thread or we know there's no draft, don't bother.
        if (thread_id <= 0/* || !conv.hasDraft()*/) {
            return "";
        }

        Uri thread_uri = ContentUris.withAppendedId(Sms.Conversations.CONTENT_URI, thread_id);
        String body = "";

        Cursor c = SqliteWrapper.query(mActivity, mContentResolver,
                        thread_uri, SMS_BODY_PROJECTION, SMS_DRAFT_WHERE, null, null);
        boolean haveDraft = false;
        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    body = c.getString(SMS_BODY_INDEX);
                    haveDraft = true;
                }
            } finally {
                c.close();
            }
        }

        // We found a draft, and if there are no messages in the conversation,
        // that means we deleted the thread, too. Must reset the thread id
        // so we'll eventually create a new thread.
        /* P0 migration*/
        if (haveDraft && (conv.getMessageCount() == 0)) {
            asyncDeleteDraftSmsMessage(conv);

            // Clean out drafts for this thread -- if the recipient set changes,
            // we will lose track of the original draft and be unable to delete
            // it later.  The message will be re-saved if necessary upon exit of
            // the activity.
            clearConversation(conv, true);
        }
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            LogTag.debug("readDraftSmsMessage haveDraft: ", !TextUtils.isEmpty(body));
        }

        return body;
    }

    public void clearConversation(final Conversation conv, boolean resetThreadId) {
        /// M: Code analyze 052, For bug ALPS00047256, to solve shows a wrong thread . @{
        int messageCount = 0;
        final Uri sAllThreadsUri =
            Threads.CONTENT_URI.buildUpon().appendQueryParameter("simple", "true").build();

        if (conv.getMessageCount() == 0) {
            Cursor cursor = SqliteWrapper.query(
                mActivity,
                mContentResolver,
                sAllThreadsUri,
                new String[] {Threads.MESSAGE_COUNT, Threads._ID} ,
                Threads._ID + "=" + conv.getThreadId(), null, null);
            if (cursor != null) {
                try {
                    if (cursor.moveToFirst()) {
                        messageCount =
                            cursor.getInt(cursor.getColumnIndexOrThrow(Threads.MESSAGE_COUNT));
                    }
                } finally {
                    cursor.close();
                }
            }
        }
        if (resetThreadId && conv.getMessageCount() == 0 && messageCount == 0) {
            LogTag.debug("clearConversation calling clearThreadId");
            conv.clearThreadId();
        }

        conv.setDraftState(false);
    }

    private void asyncUpdateDraftSmsMessage(final Conversation conv, final String contents) {
        new Thread(new Runnable() {
            public void run() {
                if (conv.getRecipients().isEmpty()) {
                    if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                        LogTag.debug("asyncUpdateDraftSmsMessage no recipients, not saving");
                    }
                    return;
                }
                /// M: Code analyze 054, For bug ALPS00120202, Message]can't save draft
                /// if enter message from messageDirect widget Sometimes thread id is
                /// deleted as obsolete thread, so need to guarantee it exists . @{
                conv.guaranteeThreadId();
                /// @}
                conv.setDraftState(true);
                updateDraftSmsMessage(conv, contents);
            }
        }, "WorkingMessage.asyncUpdateDraftSmsMessage").start();
    }

    private void updateDraftSmsMessage(final Conversation conv, String contents) {
        final long threadId = conv.getThreadId();
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "updateDraftSmsMessage tid=" + threadId + ", contents= " + contents);
        }
        // If we don't have a valid thread, there's nothing to do.
        if (threadId <= 0) {
            return;
        }

        ContentValues values = new ContentValues(3);
        values.put(Sms.THREAD_ID, threadId);
        values.put(Sms.BODY, contents);
        values.put(Sms.TYPE, Sms.MESSAGE_TYPE_DRAFT);
        SqliteWrapper.insert(mActivity, mContentResolver, Sms.CONTENT_URI, values);
        Log.d(TAG_DRAFT,
                "[updateDraftSmsMessage] mIsTurnToChooseAttach : " + mIsTurnToChooseAttach);
        if (!mIsTurnToChooseAttach) {
            asyncDeleteDraftMmsMessage(conv);
        }

        if (mOldThreadId != 0) {
            if (mOldThreadId != threadId) {
                asyncDeleteDraftMmsMessage(mOldThreadId);
            }
        }
        /// M : Fix CR : ALPS01012417  Two threads showed in ConversationList
        /// new message, input phone number, content, press home key, press message icon
        /// will load the last sms message, change phone number,back to conversationlist. @{
        if (mOldSmsSaveThreadId > 0) {
            if (mOldSmsSaveThreadId != threadId) {
                deleteDraftSmsMessage(mOldSmsSaveThreadId);
            }
        }
        mOldSmsSaveThreadId = threadId;
        /// @}

        /// M: Fix CR : ALPS01078057. when the thread changed,
        /// if the old thread saved Mms draft before,
        /// should reset the old thread's draft state @{
        if (mOldMmsSaveThreadId != 0 && mOldMmsSaveThreadId != threadId) {
            asyncDeleteDraftMmsMessage(mOldMmsSaveThreadId);
            mOldMmsSaveThreadId = 0;
        }
        /// @}
        mMessageUri = null;
    }

    private void asyncDelete(final Uri uri, final String selection, final String[] selectionArgs) {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "asyncDelete " + uri + " where " + selection);
        }
        new Thread(new Runnable() {
            public void run() {
                delete(uri, selection, selectionArgs);
            }
        }, "WorkingMessage.asyncDelete").start();
    }

    public void checkDraftSmsMessage(final Conversation conv) {
        final WorkingMessage msg = this;
          new Thread(new Runnable() {
            public void run() {
                final String draftText = msg.readDraftSmsMessage(conv);
                Log.d(TAG_DRAFT, "checkDraftSmsMessage draftText : " + draftText);
                if (!TextUtils.isEmpty(draftText)) {
                    msg.mHasSmsDraft = true;
                }
            }
        }, "WorkingMessage.loadDraft").start();
    }

    public void asyncDeleteDraftSmsMessage(Conversation conv) {
        mHasSmsDraft = false;

        final long threadId = conv.getThreadId();
        if (threadId > 0) {
            asyncDelete(ContentUris.withAppendedId(Sms.Conversations.CONTENT_URI, threadId),
                SMS_DRAFT_WHERE, null);
        }
    }

    //M: Add for modify issue ALPS01812929
    public void asyncDeleteDraftSmsMessage(Conversation conv, final Runnable r) {
        mHasSmsDraft = false;

        final long threadId = conv.getThreadId();
        if (threadId > 0) {
            asyncDelete(ContentUris.withAppendedId(Sms.Conversations.CONTENT_URI, threadId),
                SMS_DRAFT_WHERE, null, r);
        }
    }

    private void deleteDraftSmsMessage(long threadId) {
        SqliteWrapper.delete(mActivity, mContentResolver,
                ContentUris.withAppendedId(Sms.Conversations.CONTENT_URI, threadId),
                SMS_DRAFT_WHERE, null);
    }

    public void asyncDeleteDraftMmsMessage(Conversation conv) {
        mHasMmsDraft = false;

        final long threadId = conv.getThreadId();
        if (threadId > 0) {
            final String where = Mms.THREAD_ID + " = " + threadId;
            asyncDelete(Mms.Draft.CONTENT_URI, where, null);

        /// M: Reset MMS's message URI because the MMS is deleted */
        mMessageUri = null;

        }
    }

    public void asyncDeleteDraftMmsMessage(final long threadId) {
        mHasMmsDraft = false;
        if (threadId > 0) {
            final String where = Mms.THREAD_ID + " = " + threadId;
            asyncDelete(Mms.Draft.CONTENT_URI, where, null);

            /// M: Reset MMS's message URI because the MMS is deleted */
            mMessageUri = null;
        }
    }

    /// M:
    public int getCurrentMessageSize() {
        if (mSlideshow != null) {
            int currentMessageSize = mSlideshow.getCurrentSlideshowSize();
            return currentMessageSize;
        }
        return 0;
    }

    /// M: Code analyze 040, For bug ALPS00116011, the creation mode can't take
    /// effect immediately after modify in settings Should update static variable
    /// after peference is changed . @{
    public static void updateCreationMode(Context context) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        String creationMode =
            sp.getString(MmsPreferenceActivity.CREATION_MODE, CREATION_MODE_FREE);
        if (creationMode.equals(CREATION_MODE_WARNING)) {
            sCreationMode = WARNING_TYPE;
        } else if (creationMode.equals(CREATION_MODE_RESTRICTED)) {
            sCreationMode = RESTRICTED_TYPE;
        } else {
            sCreationMode = OK;
        }
    }
    /// @}

    public boolean isResizeImage() {
        return mResizeImage;
    }

    public void setResizeImage(boolean resizeImage) {
        this.mResizeImage = resizeImage;
    }

    public Uri getMessageUri() {
        return this.mMessageUri;
    }

    /**
    * Fix CR : ALPS01105564
    *  which used to delete the thread which has 2 mms drafts. only remain the last draft(pdu)
    */
    private void deleteOldMmsDraft(Uri msgUri, long tid) {
        if (msgUri != null && tid > 0) {
            String pduId = msgUri.getLastPathSegment();
            final String where = Mms.THREAD_ID + "=" + tid + " and " + WordsTable.ID
                 + " < " + pduId;
            Log.d(TAG, "deleteOldMmsDraft where : " + where);
            delete(Mms.Draft.CONTENT_URI, where, null);
        }
    }


    /// M: Code analyze 055, For bug ALPS00234739, Remove old Mms draft in
    /// conversation list instead of compose view . @}
    private void asyncDeleteOldMmsDraft(final long threadId) {
        if (mMessageUri != null && threadId > 0) {
            String pduId = mMessageUri.getLastPathSegment();
            final String where = Mms.THREAD_ID + "=" + threadId + " and " + WordsTable.ID
                    + " != " + pduId;
            asyncDelete(Mms.Draft.CONTENT_URI, where, null);
        }
    }
    /// @}

    /**
     * Delete all drafts of current thread by threadId.
     *
     * @param threadId
     */
    public void asyncDeleteAllMmsDraft(final long threadId) {
        if (threadId > 0) {
            MmsLog.d(TAG, "asyncDeleteAllMmsDraft");
            final String where = Mms.THREAD_ID + "=" + threadId;
            asyncDelete(Mms.Draft.CONTENT_URI, where, null);
        }
    }

    private WorkingMessage(Activity activity, MessageStatusListener l) {
        mActivity = activity;
        mContentResolver = mActivity.getContentResolver();
        mStatusListener = l;
        mAttachmentType = TEXT;
        mText = "";
        /// M: Code analyze 040, For bug ALPS00116011, the creation mode can't
        /// take effect immediately after modify in settings Should update static
        /// variable after peference is changed . @{
        updateCreationMode(activity);
        /// @}
        mOpWorkingMessageExt = OpMessageUtils.getOpMessagePlugin().getOpWorkingMessageExt();
        mOpWorkingMessageExt.initOpWorkingMessage(this);
    }

    public static WorkingMessage createEmpty(Activity activity, MessageStatusListener l) {
        // Make a new empty working message.
        WorkingMessage msg = new WorkingMessage(activity, l);
        return msg;
    }

    public void setHasMmsDraft(boolean hasMmsDraft) {
        mHasMmsDraft = hasMmsDraft;
    }

    public boolean checkSizeBeforeAppend() {
        if (mSlideshow == null) {
            return true;
        }
        mSlideshow.checkMessageSize(0);
        return true;
    }

    private void removeSlideAttachments(SlideModel slide) {
        slide.removeImage();
        slide.removeVideo();
        slide.removeAudio();

        /// M: add for attachment enhance, For new feature ALPS00231349, add vCard support. @{
        if (mSlideshow != null && mOpWorkingMessageExt.removeAllAttachFiles()) {
            mSlideshow.removeAllAttachFiles();
        }
    }

    /// M: Code analyze 034, For new feature ALPS00231349, add vCard support . @{
    public void removeAllFileAttaches() {
        if (mSlideshow != null && mOpWorkingMessageExt.removeAllAttachFiles()) {
            mSlideshow.removeAllAttachFiles();
        }
    }

    public boolean hasMediaAttachments() {
        if (mSlideshow == null) {
            return false;
        }
        if (hasSlideshow()) {
            return true;
        }
        final SlideModel slide = mSlideshow.get(0);
        return (slide != null) && (slide.hasAudio() || slide.hasVideo() || slide.hasImage());
    }



    public boolean hasAttachedFiles() {
        return mSlideshow != null && mSlideshow.sizeOfFilesAttach() > 0;
    }

    /// M: Code analyze 035, For bug ALPS00095817,  delete draft . @{
    public synchronized void discard(boolean isDiscard) {
        if (mActivity != null && !MmsConfig.isSmsEnabled(mActivity)) {
            Log.d(TAG_DRAFT, "[discard] not default sms, cann't discard, just return!!");
            return;
        }

        MmsLog.d(M_TAG, "discard(): Start. mConversation.ThreadId=" + mConversation.getThreadId()
                + ", MessageCount=" + mConversation.getMessageCount());
        if (LogTag.VERBOSE || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            LogTag.debug("[WorkingMessage] discard");
        }

        if (mDiscarded) {
            return;
        }

        // Mark this message as discarded in order to make saveDraft() no-op.
        mDiscarded = isDiscard;
        cancelThumbnailLoading();

        // Delete any associated drafts if there are any.
        if (mHasMmsDraft) {
            asyncDeleteCurrentDraftMmsMessage(mConversation);
        }
        if (mHasSmsDraft) {
            /// M: fix bug ALPS01471051, ensureThreadId before discard sms draft
            if (mConversation.getThreadId() == 0) {
                mConversation.ensureThreadId();
                Log.d(TAG_DRAFT, "[discard] ensureThreadId before discard sms draft");
            }
            //M: Add for modify issue ALPS01812929
            asyncDeleteDraftSmsMessage(mConversation, new Runnable() {
                            public void run() {
                                if (mConversation != null) {
                                    mConversation.setDraftState(false);
                                }
                            }
                        });
        }
        if (mConversation.getThreadId() > 0) {
            clearConversation(mConversation, true);
        }
    }
    /// @}

    private void delete(final Uri uri, final String selection, final String[] selectionArgs) {
        /// M: Code analyze 056, For bug ALPS00094352, to solve JE
        /// of MMS occured when I received serveral SM  . @}
        if (uri != null) {
            int result = -1; // if delete is unsuccessful, the delete()
            // method will return -1
            int retryCount = 0;
            int maxRetryCount = 10;
            while (result == -1 && retryCount < maxRetryCount) {
                try {
                    result = SqliteWrapper.delete(mActivity,
                            mContentResolver, uri, selection, selectionArgs);

                } catch (SQLiteDiskIOException e) {
                    MmsLog.e(TAG, "asyncDelete(): SQLiteDiskIOException:"
                            + " delete thread unsuccessful! Try time=" + retryCount);
                } finally {
                    retryCount++;
                }
            }
        }
        /// @}
    }

    /// M: Code analyze 057, For bug ALPS00234653, to solve Can't continue
    /// to play the vedio after share a video in mms . @}
    private void asyncDeleteCurrentDraftMmsMessage(Conversation conv) {
        mHasMmsDraft = false;

        final long threadId = conv.getThreadId();
        if (threadId > 0) {
            final String where = Mms.THREAD_ID + " = " + threadId;

            asyncDelete(mMessageUri, where, null, new Runnable() {
                public void run() {
                    if (mConversation != null) {
                        mConversation.setDraftState(false);
                    }
                }
            });

            // / @}
            mMessageUri = null;
        } else if (mMessageUri != null) {
            asyncDelete(mMessageUri, null, null);
            mMessageUri = null;
        }
    }
    /// @}

    ///M: add for fix ALPS00452425 @{
    private void asyncDelete(final Uri uri, final String selection,
            final String[] selectionArgs, final Runnable r) {

        new Thread(new Runnable() {
            public void run() {
                delete(uri, selection, selectionArgs);
                MmsLog.dpi(TAG, "delete finish, uri = " + uri);
                if (r != null) {
                    r.run();
                }
            }
        }, "WorkingMessage.asyncDelete2").start();
    }
    /// @}

    /// M Fix CR : ALPS01071659
    /// First group message, save a mms draft,which only contain text,
    /// back to conversationlist ,change to not normal message,
    /// the enter the mms draft send the message,
    /// back to conversationlist, the group mms draft still there
    /// when loafFinished, call this to delete the mms draft
    /// which is not group message, and only contains text @{
    public void deleteGruoupMmsDraft() {
        if (mActivity != null && MmsConfig.isSmsEnabled(mActivity)) {
            if (!MmsPreferenceActivity.getIsGroupMmsEnabled(mActivity)
                    && mConversation.getRecipients().size() > 1
                    && mMmsState != RECIPIENTS_REQUIRE_MMS) {
                if (!hasAttachedFiles() && !hasSlideshow() && !hasAttachment()
                         && !hasMediaAttachments() && !hasSubject()) {
                    asyncDeleteDraftMmsMessage(mConversation);
                    clearConversation(mConversation, true);
                }
            }
        }
    }
    /// @}

    /// M: fix bug ALPS00712509, show toast when paste many word and > 300k
    private boolean mIsExceedSize;

    public boolean isExceedSize() {
        return mIsExceedSize;
    }

    public void setIsExceedSize(boolean isExceedSize) {
        mIsExceedSize = isExceedSize;
    }
    /// @}

    private boolean mIsUpdateAttachEditor = true;

    public boolean getIsUpdateAttachEditor() {
        if (!mIsUpdateAttachEditor) {
            MmsLog.d(TAG, "mIsUpdateAttachEditor == false");
        }
        return mIsUpdateAttachEditor;
    }

    public void setIsUpdateAttachEditor(boolean update) {
        mIsUpdateAttachEditor = update;
    }

    /// M: fix bug ALPS01265824, need remove FileAttachment when text + attachmentSize > limit
    public boolean isRemoveFileAttachment() {
        if (mSlideshow != null
                && mSlideshow.getCurrentSlideshowSize() > MmsConfig.getUserSetMmsSizeLimit(true)) {
            return true;
        }
        return false;
    }

    // Add for IpMessage callback
    public boolean requiresIpMms() {
        return requiresMms();
    }

    public void setIpText(CharSequence s) {
        setText(s);
    }

    public CharSequence getIpText() {
        return getText();
    }

    public void syncWorkingIpRecipients() {
        syncWorkingRecipients();
    }

    public void setIpSubject(CharSequence s, boolean notify) {
        setSubject(s, notify);
    }

    public boolean hasDrmMedia() {
        return mSlideshow == null ? false : mSlideshow.hasDrmMedia();
    }

    public boolean hasDrmMediaRight() {
        return mSlideshow == null ? false : mSlideshow.hasDrmMediaRight();
    }

    public IWorkingMessageCallback loadCallback(Object activity, Uri uri) {
        return load((ComposeMessageActivity) activity, uri);
    }

    public boolean hasMediaAttachmentsCallback() {
        return hasMediaAttachments();
    }

    public void removeAllFileAttachesCallback() {
        removeAllFileAttaches();
    }

    public int getState() {
        return mMmsState;
    }

    public void correctAttachmentStateCallback() {
        correctAttachmentState();
    }

    public boolean isIpWorthSaving() {
        return isWorthSaving();
    }

    public CharSequence getIpSubject() {
        return getSubject();
    }

    public void onAttachmentChangedCallback() {
        mStatusListener.onAttachmentChanged();
    }

    public void discardCallback() {
        discard();
    }

    public void updateStateExt(int state, boolean on, boolean notify) {
        updateState(state, on, notify);
    }

    public void setLengthRequiresMmsCallback(boolean mmsRequired, boolean notify) {
        setLengthRequiresMms(mmsRequired, notify);
    }
}
