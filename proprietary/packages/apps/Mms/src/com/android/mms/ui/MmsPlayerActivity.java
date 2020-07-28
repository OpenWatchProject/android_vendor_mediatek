/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
import android.content.ActivityNotFoundException;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.ClipboardManager;
import android.text.SpannableString;
import android.text.TextUtils;
import android.text.style.URLSpan;
import android.text.util.Linkify;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.browser.provider.Browser;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.data.ContactList;
import com.android.mms.model.ImageModel;
import com.android.mms.model.MediaModel;
import com.android.mms.model.RegionModel;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.model.TextModel;
import com.android.mms.model.VideoModel;
import com.android.mms.transaction.MessageSender;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.transaction.MmsMessageSender;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.android.mms.util.Recycler;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.MultimediaMessagePdu;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;
import com.google.android.mms.util.PduCache;
import com.mediatek.android.mms.pdu.MtkMultimediaMessagePdu;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.android.mms.pdu.MtkSendReq;
import com.mediatek.mms.callback.IMmsPlayerActivityCallback;
import com.mediatek.mms.callback.ITextSizeAdjustHost;
import com.mediatek.mms.ext.IOpMmsPlayerActivityExt;
import com.mediatek.mms.model.FileAttachmentModel;
import com.mediatek.mms.util.CellConnMgr;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SettingListActivity;

import java.util.ArrayList;
import java.util.regex.Pattern;

/** M:
 * Plays the given slideshow in full-screen mode with a common controller.
 */
public class MmsPlayerActivity extends Activity
        implements Contact.UpdateListener, ITextSizeAdjustHost {
    private static final String TAG = "MmsPlayerActivity";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;
    private MmsPlayerActivityAdapter mListAdapter;
    private static final int MENU_PLAYASSLIDE = 1;
    private Uri mMessageUri;
    private ListView mListView;
    private ContentResolver mContentResolver;
    private Cursor mCursor = null;
    private AsyncQueryHandler mQueryHandler = null;

    // context menu item
    private static final int MENU_COPY_MESSAGE_TEXT         = 1;
    private static final int MENU_ADD_TO_BOOKMARK           = 2;
    private static final int MENU_ADD_TO_CONTACTS           = 3;
    private static final int MENU_ADD_ADDRESS_TO_CONTACTS   = 4;
    private static final int MENU_SEND_EMAIL                = 5;
    private static final int MENU_CALL_BACK                 = 6;
    private static final int MENU_SEND_SMS                  = 7;
    private static final int MENU_SELECT_TEXT               = 8; // add for select text copy
    private static final int MENU_LOCK                      = 90;
    private static final int MENU_UNLOCK                    = 100;

    public static final int REQUEST_CODE_ADD_CONTACT      = 1;
    private Context mContext;
    private ArrayList<String> mURLs = new ArrayList<String>();
    private String mCopyText;

    private AsyncDialog mAsyncDialog;

    //add for cmcc dir ui begin
    private TextView mRecipient;
    private TextView mDate;
    private TextView mThrough;
    private TextView subName;
    private TextView mMmsSubject;
    private ImageView mHasFileAttachment;
    private TextView mAttachName;
    private ImageView mLockedInd;
    private boolean mLocked;
    private Long mDateLong;
    private int mSubId;
    private String mNumber;
    private String mName;
    private int mIndex = -1; //the first number index in ContactList can be added to contact
    private ContactList mContactList;
    private boolean mShowAddContact = false;
    private Uri mMsgUri;
    private int mMsgBox;
    private boolean mShowResend;
    private long mThreadId;
    private long mMsgId;
    private int mDeliveryReport;
    private int mReadReport;
    private String mSubject;
    private SlideshowModel mSlideshow;
    private boolean mClearCache = true;
    private boolean mParseMsgUriSuccess = false;
    private static final String[] PDU_PROJECTION = new String[] {
        Mms._ID,
        Mms.MESSAGE_BOX,
        Mms.THREAD_ID,
        Mms.RETRIEVE_TEXT,
        Mms.SUBJECT,
        Mms.SUBJECT_CHARSET,
        Mms.DELIVERY_REPORT,
        Mms.READ_REPORT,
        Mms.DATE,
        Telephony.Mms.SUBSCRIPTION_ID,
        Mms.LOCKED
    };
    private static final int PDU_COLUMN_ID                    = 0;
    private static final int PDU_COLUMN_MESSAGE_BOX           = 1;
    private static final int PDU_COLUMN_THREAD_ID             = 2;
    private static final int PDU_COLUMN_RETRIEVE_TEXT         = 3;
    private static final int PDU_COLUMN_SUBJECT               = 4;
    private static final int PDU_COLUMN_SUBJECT_CHARSET       = 5;
    private static final int COLUMN_MMS_DELIVERY_REPORT       = 6;
    private static final int COLUMN_MMS_READ_REPORT           = 7;
    private static final int COLUMN_MMS_DATE                  = 8;
    private static final int COLUMN_MMS_SUB_ID                = 9;
    private static final int COLUMN_MMS_LOCKED                = 10;

    // option menu item
    private static final int MENU_REPLY           = 2;
    private static final int MENU_RESEND          = 3;
    private static final int MENU_ADD_CONTACT     = 4;
    private static final int MENU_VIEW_REPORT     = 5;
    private static final int MENU_FORWARD         = 6;
    private static final int MENU_DELETE          = 7;
    private static final int MNEU_DETAIL          = 8;
    private static final int MENU_CALL_RECIPIENT  = 9;
    private static final int MENU_CALL_RECIPIENT_BY_VT  = 10;
    private static final int MENU_COPY_TO_SDCARD        = 11;
    //add for cmcc dir ui end

    /// M: ALPS00619099, highlight matched search string in mms player @ {
    private Pattern mHighlight = null;
    /// @}

//    private static CellConnMgr mCellMgr = null;//csw
    // Whether or not we are currently enabled for SMS. This field is updated in onStart to make
    // sure we notice if the user has changed the default SMS app.
    private boolean mIsSmsEnabled;

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }

        mContext = this;
        mAsyncDialog = new AsyncDialog(this);
        Intent intent = getIntent();
        mMessageUri = intent.getData();
        //add for cmcc dir ui begin
        mOpMmsPlayerActivityExt = OpMessageUtils.getOpMessagePlugin().getOpMmsPlayerActivityExt();

        /// M: ALPS00619099, highlight matched search string in mms player @ {
        String highlightString = getIntent().getStringExtra("highlight");
        mHighlight = highlightString == null
            ? null
            : Pattern.compile(Pattern.quote(highlightString), Pattern.CASE_INSENSITIVE);
        /// @}

        //add for cmcc dir ui end
        setContentView(R.layout.mms_player_activity);
        mListView = (ListView) findViewById(R.id.slide_item_list);
        asyncLoadMms();
    }

    private void initListAdapter(Uri msg) {
        //PduBody body = null;
        SlideshowModel slideshowModel = mSlideshow;
        if (slideshowModel == null) {
            return;
        }

        int slideNum = slideshowModel.size();
        Log.i(TAG, "initListAdapter,getPartsNum:" + slideNum);
        ArrayList<MmsPlayerActivityItemData> attachments =
            new ArrayList<MmsPlayerActivityItemData>(slideNum);
        for (int i = 0; i < slideNum; i++) {
            SlideModel slideModel = slideshowModel.get(i);
            if (slideModel == null) {
                break;
            }
            String text = null;
            Uri imageUri = null;
            String imageType = null;
            Uri videoUri = null;
            String videoType = null;
            String audioName = null;
            Uri audioUri = null;
            String audioType = null;
            int mImageOrVideoLeft = 0;
            int mImageOrVideoTop = 0;
            int mImageOrVideoWidth = 0;
            int mImageOrVideoHeight = 0;
            int mTextLeft = 0;
            int mTextTop = 0;
            int mTextWidth = 0;
            int mTextHeight = 0;
            for (MediaModel media : slideModel) {
                if (media.isImage()) {
                    imageUri = media.getUri();
                    imageType = media.getContentType();
                    RegionModel imageRegionModel = ((ImageModel) media).getRegion();
                    if (imageRegionModel != null) {
                        mImageOrVideoLeft = imageRegionModel.getLeft();
                        mImageOrVideoTop = imageRegionModel.getTop();
                        mImageOrVideoWidth = imageRegionModel.getWidth();
                        mImageOrVideoHeight = imageRegionModel.getHeight();
                    }
                } else if (media.isVideo()) {
                    videoUri = media.getUri();
                    videoType = media.getContentType();
                    RegionModel videoRegionModel = ((VideoModel) media).getRegion();
                    if (videoRegionModel != null) {
                        mImageOrVideoLeft = videoRegionModel.getLeft();
                        mImageOrVideoTop = videoRegionModel.getTop();
                        mImageOrVideoWidth = videoRegionModel.getWidth();
                        mImageOrVideoHeight = videoRegionModel.getHeight();
                    }
                } else if (media.isAudio()) {
                    audioName = media.getSrc();
                    audioUri = media.getUri();
                    audioType = media.getContentType();
                    MmsLog.ipi(TAG, " add audio name: " + audioName);
                } else if (media.isText()) {
                    TextModel sm = slideModel.getText();
                    if (sm != null) {
                        text = sm.getText();
                        RegionModel textRegionModel = sm.getRegion();
                        if (textRegionModel != null) {
                            mTextLeft = textRegionModel.getLeft();
                            mTextTop = textRegionModel.getTop();
                            mTextWidth = textRegionModel.getWidth();
                            mTextHeight = textRegionModel.getHeight();
                        }
                    }
                }
            }
            MmsLog.ipi(TAG, "Add slide: " + i + " imageUri is: "
                    + imageUri + " videoUri is: " + videoUri
                    + " audio name is: " + audioName);
            attachments.add(new MmsPlayerActivityItemData(this, imageUri, videoUri,
                    audioName, text, mImageOrVideoLeft,
                    mImageOrVideoTop, mImageOrVideoWidth,
                    mImageOrVideoHeight, mTextLeft, mTextTop, mTextWidth,
                    mTextHeight, imageType, videoType, audioUri, audioType));
        }

        /// M: OP01
        attachments.trimToSize();
        /// M: ALPS00619099, highlight matched search string in mms player @ {
        if (mHighlight != null) {
            mListAdapter = new MmsPlayerActivityAdapter(this, attachments, mHighlight);
        } else {
            mListAdapter = new MmsPlayerActivityAdapter(this, attachments);
        }
        /// @}
        mListView.setAdapter(mListAdapter);
        mListView.setOnCreateContextMenuListener(mTextMenuCreateListener);
    }


    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        if (mListAdapter != null && mClearCache) {
            mListAdapter.clearAllCache();
        }
        super.onDestroy();
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        return false;
    }

    @Override
    protected void onStop() {
        super.onStop();
        Contact.removeListener(this);
    }

    @Override
    protected void onStart() {
        super.onStart();

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        float textSize = MessageUtils.getPreferenceValueFloat(this,
                SettingListActivity.TEXT_SIZE, 18);
        setTextSize(textSize);
        mOpMmsPlayerActivityExt.onStart(this, this);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setTitle(R.string.mms);
        Contact.addListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        if (mListAdapter != null) {
            mListAdapter.notifyDataSetChanged();
            /// M: ALPS01586782, more click issue @ {
            mListAdapter.setIsClick(false);
            ///@}
        }
        mClearCache = true;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, MENU_PLAYASSLIDE, 0, R.string.play_as_slideshow);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
            MmsLog.d(TAG, "onCreateOptionsMenu enter!!");
            if (PermissionCheckUtil.requestRequiredPermissions(this)) {
                return true;
            }

         Intent intentMultiSave = new Intent(MmsPlayerActivity.this, MultiSaveActivity.class);
         mOpMmsPlayerActivityExt.onOptionsItemSelected(intentMultiSave);

        switch(item.getItemId()) {
        case MENU_PLAYASSLIDE:
            Intent intent = new Intent(this, SlideshowActivity.class);
            intent.setData(mMessageUri);
            this.startActivity(intent);
            break;
         // add for cmcc dir ui mode begin
         case MENU_FORWARD:
            // this is a little slow if mms is big.
            new Thread(new Runnable() {
                public void run() {
                    forwardMms();
                }
            }).start();
            break;
        case MENU_DELETE:
            confirmToDeleteMessage(mMsgUri);
            break;
        case MNEU_DETAIL:
            onClickDetails(null);
            break;
        case MENU_ADD_CONTACT:
            addToContact();
            break;
        case MENU_REPLY:
            int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
            int slot = SubscriptionManager.getSlotIndex(mSubId);
            MmsLog.d(TAG, "slot is:" + slot + ",subId:" + mSubId);
            if (SubscriptionManager.isValidSubscriptionId(slot)) {
                subId = mSubId;
            }
            MessageUtils.replyMessage(subId, this, mNumber);
            break;
        case MENU_RESEND:
            Uri uri = null;

            /// M: @{ Add for ALPS01092608, when resend a mms, we should check mms size firstly.
            int messageSize = 0;
            messageSize = mSlideshow.getCurrentSlideshowSize();
            MmsLog.d(TAG, "ResendMessage(): messageSize=" + messageSize);
            if (messageSize > MmsConfig.getUserSetMmsSizeLimit(true)) {
                MessageUtils.showErrorDialog(MmsPlayerActivity.this,
                        R.string.exceed_message_size_limitation,
                        R.string.exceed_message_size_limitation, 0, 0);
                break;
            }
            ///@}

            try {
                final MtkPduPersister persister = MtkPduPersister.getPduPersister(this);
                final long threadId = mThreadId;
                // add CellConnMgr feature
                final CellConnMgr cellConnMgr = new CellConnMgr(getApplicationContext());
                final int state = cellConnMgr.getCurrentState(mSubId, CellConnMgr.STATE_FLIGHT_MODE
                    | CellConnMgr.STATE_SIM_LOCKED | CellConnMgr.STATE_RADIO_OFF);
                MmsLog.d(TAG, "CellConnMgr, state is " + state);
                if (((state & CellConnMgr.STATE_FLIGHT_MODE) == CellConnMgr.STATE_FLIGHT_MODE) ||
                        ((state & CellConnMgr.STATE_SIM_LOCKED) == CellConnMgr.STATE_SIM_LOCKED) ||
                        ((state & CellConnMgr.STATE_RADIO_OFF) == CellConnMgr.STATE_RADIO_OFF) ||
                        ((state & (CellConnMgr.STATE_FLIGHT_MODE | CellConnMgr.STATE_RADIO_OFF))
                                    == (CellConnMgr.STATE_FLIGHT_MODE
                                            | CellConnMgr.STATE_RADIO_OFF))) {
                    final ArrayList<String> stringArray = cellConnMgr.getStringUsingState(mSubId,
                            state);
                    MmsLog.d(TAG, "CellConnMgr, stringArray length is " + stringArray.size());
                    if (stringArray.size() == 4) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(this);
                        builder.setTitle(stringArray.get(0));
                        builder.setMessage(stringArray.get(1));
                        builder.setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        dialog.dismiss();
                                    }
                                });
                        builder.show();
                    }
                } else {
                    try {
                        uri = persister.move(mMsgUri, Mms.Draft.CONTENT_URI);
                        MessageSender sender = new MmsMessageSender(MmsPlayerActivity.this,
                                uri, mSlideshow.getCurrentSlideshowSize(), mSubId);
                        Log.d(TAG, "MmsPlayerActivity: resendMsg  mSubId =" + mSubId);
                        sender.sendMessage(threadId);
                    } catch (MmsException e) {
                        MmsLog.epi(TAG, "Can't resend mms. + uri=" + uri);
                    }
                    // Make sure this thread isn't over the limits in message count
                    Recycler.getMmsRecycler().deleteOldMessagesByThreadId(
                            getApplicationContext(), threadId);
                    finish();
                }
            } catch (Exception e) {
                Log.e(TAG, "Failed to send message: threadId=" + mThreadId, e);
            }
            break;
        case MENU_VIEW_REPORT:
            showDeliveryReport();
            break;
        case android.R.id.home:
            finish();
            break;
        case MENU_CALL_RECIPIENT:
            dialRecipient(false);
            break;
        case MENU_CALL_RECIPIENT_BY_VT:
            dialRecipient(true);
            break;
        case MENU_COPY_TO_SDCARD:
            if (!Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
                Toast.makeText(MmsPlayerActivity.this,
                        getString(R.string.Insert_sdcard), Toast.LENGTH_LONG).show();
                return false;
            }

            final long iMsgId = mMsgId;
            intentMultiSave.putExtra("msgid", iMsgId);
            startActivityForResult(intentMultiSave, 0);
            return true;
        case MENU_LOCK:
            lockMessage(true);
            break;
        case MENU_UNLOCK:
            lockMessage(false);
            break;
        default:
            return false;
        // add for cmcc dir ui mode end
        }
        return false;
    }

    private void confirmToDeleteMessage(final Uri msgUri) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.confirm_dialog_title);
        builder.setIconAttribute(android.R.attr.alertDialogIcon);
        builder.setCancelable(true);
        builder.setMessage(mLocked ? R.string.confirm_delete_locked_message :
            R.string.confirm_delete_message);
        builder.setPositiveButton(R.string.delete,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        /// M: fix bug ALPS00351620; for requery searchactivity.
                                        PduCache.getInstance().purge(msgUri);
                                        SearchActivity.setNeedRequery();
                                        SqliteWrapper.delete(MmsPlayerActivity.this,
                                                getContentResolver(), msgUri, null, null);
                                        dialog.dismiss();
                                        Intent mIntent = new Intent();
                                        mIntent.putExtra("delete_flag", true);
                                        setResult(RESULT_OK, mIntent);
                                        finish();
                                    }
        });
        builder.setNegativeButton(R.string.no, null);
        builder.show();
    }

    private final OnCreateContextMenuListener mTextMenuCreateListener =
        new OnCreateContextMenuListener() {
        public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
            TextMenuClickListener l = new TextMenuClickListener();
            int itemId = ((AdapterView.AdapterContextMenuInfo) menuInfo).position;
            MmsPlayerActivityItemData data =
                (MmsPlayerActivityItemData) mListAdapter.getItem(itemId);
            if (data != null) {
                addCallAndContactMenuItems(menu, l, data.getText());
            } else {
                Log.i(TAG, "getItem null");
            }

        }
    };

    private final class TextMenuClickListener implements MenuItem.OnMenuItemClickListener {
        public boolean onMenuItemClick(MenuItem item) {
            switch (item.getItemId()) {
            case MENU_COPY_MESSAGE_TEXT:
                ClipboardManager clip =
                    (ClipboardManager) MmsPlayerActivity.this.getSystemService(
                            mContext.CLIPBOARD_SERVICE);
                clip.setText(mCopyText);
                return true;

            case MENU_ADD_TO_BOOKMARK:
                if (mURLs.size() == 1) {
                    Browser.saveBookmark(mContext, null, mURLs.get(0));
                } else if (mURLs.size() > 1) {
                    CharSequence[] items = new CharSequence[mURLs.size()];
                    for (int i = 0; i < mURLs.size(); i++) {
                        items[i] = mURLs.get(i);
                    }
                    new AlertDialog.Builder(mContext)
                        .setTitle(R.string.menu_add_to_bookmark)
                        .setIcon(R.drawable.ic_dialog_menu_generic)
                        .setItems(items, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                Browser.saveBookmark(mContext, null, mURLs.get(which));
                                }
                            })
                        .show();
                }
                return true;

            case MENU_ADD_ADDRESS_TO_CONTACTS:
                Intent mAddContactIntent = item.getIntent();
                startActivityForResult(mAddContactIntent, REQUEST_CODE_ADD_CONTACT);
                return true;

            // add for select text copy
            case MENU_SELECT_TEXT:
                AlertDialog.Builder dialog = new AlertDialog.Builder(mContext)
                                                    .setPositiveButton(R.string.yes, null);
                LayoutInflater factory = LayoutInflater.from(dialog.getContext());
                final View textEntryView =
                    factory.inflate(R.layout.alert_dialog_text_entry, null);
                EditText contentSelector =
                    (EditText) textEntryView.findViewById(R.id.content_selector);
                contentSelector.setText(mCopyText);
                dialog.setTitle(R.string.select_text)
                    .setView(textEntryView)
                    .show();
                return true;
            case MENU_SEND_EMAIL:
                try {
                    startActivity(item.getIntent());
                } catch (ActivityNotFoundException e) {
                    Intent mChooserIntent = Intent.createChooser(item.getIntent(), null);
                    startActivity(mChooserIntent);
                }
                return true;
            default:
                return false;
            }
        }
    }


    private void addCallAndContactMenuItems(
            ContextMenu menu, TextMenuClickListener l, CharSequence text) {
        // Add all possible links in the address & message
        StringBuilder textToSpannify = new StringBuilder();
        textToSpannify.append(text);

        SpannableString msg = new SpannableString(textToSpannify.toString());
        Linkify.addLinks(msg, Linkify.EMAIL_ADDRESSES|Linkify.PHONE_NUMBERS|Linkify.WEB_URLS);
        ArrayList<String> uris =
            MessageUtils.extractUris(msg.getSpans(0, msg.length(), URLSpan.class));
        mURLs.clear();
        menu.setHeaderTitle(R.string.message_options);
        if (text != null && text.length() > 0) {
            menu.add(0, MENU_COPY_MESSAGE_TEXT, 0, R.string.copy_message_text)
                    .setOnMenuItemClickListener(l);
            // add for select text copy
            menu.add(0, MENU_SELECT_TEXT, 0, R.string.select_text)
                    .setOnMenuItemClickListener(l);
            mCopyText = (String) text;
        }
        while (uris.size() > 0) {
            String uriString = uris.remove(0);
            // Remove any dupes so they don't get added to the menu multiple times
            while (uris.contains(uriString)) {
                uris.remove(uriString);
            }

            int sep = uriString.indexOf(":");
            String prefix = null;
            if (sep >= 0) {
                prefix = uriString.substring(0, sep);
                if ("mailto".equalsIgnoreCase(prefix) || "tel".equalsIgnoreCase(prefix)) {
                    uriString = uriString.substring(sep + 1);
                }
            }
            boolean addToContacts = false;
            if ("mailto".equalsIgnoreCase(prefix)) {
                String sendEmailString = mContext.getString(
                        R.string.menu_send_email).replace("%s", uriString);
                Intent intent = new Intent(Intent.ACTION_VIEW,
                        Uri.parse("mailto:" + uriString));
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                MmsLog.ipi(TAG, "send email number: " + sendEmailString);
                menu.add(0, MENU_SEND_EMAIL, 0, sendEmailString)
                    .setOnMenuItemClickListener(l)
                    .setIntent(intent);
                addToContacts = !MessageUtils.haveEmailContact(uriString, mContext);
            } else if ("tel".equalsIgnoreCase(prefix)) {
                String callBackString = mContext.getString(
                        R.string.menu_call_back).replace("%s", uriString);
                Intent intent = new Intent(Intent.ACTION_DIAL,
                        Uri.parse("tel:" + uriString));
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                MmsLog.ipi(TAG, "call back number: " + callBackString);
                menu.add(0, MENU_CALL_BACK, 0, callBackString)
                    .setOnMenuItemClickListener(l)
                    .setIntent(intent);

                if (text != null && text.toString().replaceAll("\\-", "").contains(uriString)) {
                    String sendSmsString = mContext.getString(
                        R.string.menu_send_sms).replace("%s", uriString);
                    Intent intentSms = new Intent(Intent.ACTION_SENDTO,
                        Uri.parse("smsto:" + uriString));
                    intentSms.setClassName(mContext, "com.android.mms.ui.SendMessageToActivity");
                    intentSms.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    MmsLog.dpi(TAG, "send sms number: " + sendSmsString);
                    /// M: disable when non-default sms
                    if (MmsConfig.isSmsEnabled(this)) {
                        menu.add(0, MENU_SEND_SMS, 0, sendSmsString)
                            .setOnMenuItemClickListener(l)
                            .setIntent(intentSms);
                    }
                }
                addToContacts = !isNumberInContacts(uriString);
            } else {
                //add URL to book mark
                if (mURLs.size() <= 0) {
                    menu.add(0, MENU_ADD_TO_BOOKMARK, 0, R.string.menu_add_to_bookmark)
                    .setOnMenuItemClickListener(l);
                }
                mURLs.add(uriString);
            }
            if (addToContacts) {
                Intent intent = ConversationList.createAddContactIntent(uriString);
                String addContactString = mContext.getString(
                        R.string.menu_add_address_to_contacts).replace("%s", uriString);
                menu.add(0, MENU_ADD_ADDRESS_TO_CONTACTS, 0, addContactString)
                    .setOnMenuItemClickListener(l)
                    .setIntent(intent);
            }
        }
    }

    private boolean isNumberInContacts(String phoneNumber) {
        return Contact.get(phoneNumber, true).existsInDatabase();
    }

    public interface OnDataSetChangedListener {
        void onDataSetChanged(MessageListAdapter adapter);
    }

    private void parseMsgUri() {
        MmsLog.dpi(TAG, "uri:" + mMsgUri.toString());
        Cursor c = null;
        try {
            c = SqliteWrapper.query(this, getContentResolver(), mMsgUri,
                            PDU_PROJECTION, null, null, null);
            if (c == null || c.getCount() != 1) {
                Log.w(TAG, "count is not 1!!");
                mParseMsgUriSuccess = false;
                return;
            }
            c.moveToFirst();
            mMsgBox = c.getInt(PDU_COLUMN_MESSAGE_BOX);
            mThreadId = c.getLong(PDU_COLUMN_THREAD_ID);
            mMsgId = c.getLong(PDU_COLUMN_ID);
            mSubject = c.getString(PDU_COLUMN_SUBJECT);
            mDeliveryReport = c.getInt(COLUMN_MMS_DELIVERY_REPORT);
            mReadReport = c.getInt(COLUMN_MMS_READ_REPORT);
            mDateLong = c.getLong(COLUMN_MMS_DATE) * 1000L;
            mSubId = c.getInt(COLUMN_MMS_SUB_ID);
            mLocked = c.getInt(COLUMN_MMS_LOCKED) > 0;
            if (!TextUtils.isEmpty(mSubject)) {
                EncodedStringValue v = new EncodedStringValue(
                        c.getInt(PDU_COLUMN_SUBJECT_CHARSET),
                        MtkPduPersister.getBytes(mSubject));
                mSubject = v.getString();
            }
            MmsLog.dpi(TAG, "msgbox:" + mMsgBox + ",threadId:" + mThreadId +
                                                ", subject:" + mSubject);
            // get the mms related numbers
            MtkPduPersister persister = MtkPduPersister.getPduPersister(this);
            MultimediaMessagePdu pdu = (MultimediaMessagePdu)persister.load(mMsgUri);
            if (mMsgBox == Mms.MESSAGE_BOX_INBOX) {
                if (pdu != null &&  pdu.getFrom() != null) {
                    mNumber = pdu.getFrom().getString();
                }
                MmsLog.dpi(TAG, "this mms is from:" + mNumber);
            } else {
                EncodedStringValue numbers[] = pdu.getTo();
                mNumber = EncodedStringValue.concat(numbers);
                MmsLog.dpi(TAG, "this mms is to:" + mNumber);
            }

            //check whether this mms was sent fail.
            if (isOutgoingMessage()) {
                checkSendFail();
            }
            mParseMsgUriSuccess = true;
        } catch (MmsException e) {
            Log.e(TAG, "MmsException:" + e.toString());
        } catch (ClassCastException e) {
            Log.e(TAG, "ClassCastException:" + e.toString());
        } finally {
            if (c != null) {
                c.close();
            }
        }
        // fill mms header view
        String date;
        date = MessageUtils.formatTimeStampString(this, mDateLong);
        if (mMsgBox == Mms.MESSAGE_BOX_INBOX) {
            //mName = getString(R.string.via_without_time_for_send) + ": " + mName;
            date = String.format(getString(R.string.received_on), date);
        } else {
            //mName = getString(R.string.via_without_time_for_recieve) + ": " + mName;
            date = String.format(getString(R.string.sent_on), date);
        }
        //mRecipient.setText("xx");//do this later.
        mDate.setText(date);
        mLockedInd.setVisibility(mLocked ? View.VISIBLE : View.GONE);
        setSubIconAndLabel(mSubId);
        //mark this mms as readed.
        markMmsRead();
        //update notification
        if (mMsgBox == Mms.MESSAGE_BOX_INBOX) {
            MessagingNotification.nonBlockingUpdateNewMessageIndicator(this, mThreadId, false);
        }
    }

    private void markMmsRead() {
        new Thread(new Runnable() {
            public void run() {
                final ContentValues values = new ContentValues(2);
                values.put("read", 1);
                values.put("seen", 1);
                SqliteWrapper.update(getApplicationContext(),
                        getContentResolver(), mMsgUri, values, null, null);
            }
        }).start();
    }

    public boolean isOutgoingMessage() {
        boolean isOutgoingMms =
            (mMsgBox == Mms.MESSAGE_BOX_OUTBOX || mMsgBox == Mms.MESSAGE_BOX_FAILED);
        return isOutgoingMms;
    }

    private void checkSendFail() {
        long msgId = ContentUris.parseId(mMsgUri);
        Log.d(TAG, "mms id:" + msgId);
        Uri.Builder uriBuilder = PendingMessages.CONTENT_URI.buildUpon();
        uriBuilder.appendQueryParameter("protocol", "mms");
        uriBuilder.appendQueryParameter("message", String.valueOf(msgId));

        Cursor cursor = SqliteWrapper.query(this, getContentResolver(),
                uriBuilder.build(), null, null, null, null);
        if (cursor != null) {
            try {
                if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                    int errorType =
                        cursor.getInt(cursor.getColumnIndexOrThrow(PendingMessages.ERROR_TYPE));
                    if (errorType >= MmsSms.ERR_TYPE_GENERIC_PERMANENT) {
                        mShowResend = true;
                    } else {
                        mShowResend = false;
                    }
                } else {
                    Log.e(TAG, "query result not 1.!!");
                }
            } finally {
                cursor.close();
            }
        } else {
            Log.d(TAG, "query PendingMessages get null!");
        }
    }

    private void forwardMms() {
        MtkSendReq sendReq = new MtkSendReq();
        String subject = getString(R.string.forward_prefix);
        if (mSubject != null) {
            subject += mSubject;
        }
        sendReq.setSubject(new EncodedStringValue(subject));
        sendReq.setBody(mSlideshow.makeCopy());

        Uri uri = null;
        try {
            MtkPduPersister persister = MtkPduPersister.getPduPersister(this);
            // Copy the parts of the message here.
            uri = persister.persist(sendReq, Mms.Draft.CONTENT_URI, true, false, null);
        } catch (MmsException e) {
            MmsLog.epi(TAG, "Failed to copy message: " + mMsgUri, e);
            MmsApp.getToastHandler().sendEmptyMessage(MmsApp.MSG_MMS_CAN_NOT_SAVE);
            return;
        }

        Intent intent = new Intent();
        intent.putExtra("forwarded_message", true);
        intent.putExtra("msg_uri", uri);
        intent.putExtra("subject", subject);
        intent.setClassName(this, "com.android.mms.ui.ForwardMessageActivity");
        startActivity(intent);
    }

    private void addToContact() {
        //int count = mContactList.size();
        //switch(count) {
        //case 0:
        //    Log.e(TAG, "add contact, mCount == 0!");
        //    break;
        //case 1:
        if (mIndex != -1) {
            Intent intent =
                ConversationList.createAddContactIntent(mContactList.get(mIndex).getNumber());
            startActivity(intent);
        }
            //MessageUtils.addNumberOrEmailtoContact(mNumber, 0, this);
        //    break;
        //default:
            //MultiRecipientsActivity.setContactList(mContactList);
            //final Intent i = new Intent(getApplicationContext(), MultiRecipientsActivity.class);
            //startActivity(i);
        //    break;
        //}
    }

    public void onClickDetails(View v) {
        //show mms details info.
        String messageDetails = MessageUtils.getMmsDetail(this, mMsgUri,
                mSlideshow.getCurrentSlideshowSize(), mMsgBox);
        new AlertDialog.Builder(this)
                .setTitle(R.string.message_details_title)
                .setMessage(messageDetails)
                .setPositiveButton(android.R.string.ok, null)
                .setCancelable(true)
                .show();
    }

    private void showDeliveryReport() {
        // delivery report activity is partially shown, do not clear the cache.
        // or the back activity may set ImageView to null, and image disappear.
        mClearCache = false;
        Intent intent = new Intent(this, DeliveryReportActivity.class);
        intent.putExtra("message_id", mMsgId);
        intent.putExtra("message_type", "mms");
        startActivity(intent);
    }

    // We don't want to show the "call" option unless there is only one
    // recipient and it's a phone number.
    private boolean isRecipientCallable() {
        if (mContactList == null) {
            if (mNumber != null) {
                mContactList = ContactList.getByNumbers(mNumber, true, false);
            } else {
                MmsLog.wpi(TAG, "isRecipientCallable(): no number with msgUri=" + mMsgUri);
                return false;
            }
        }
        return (mContactList.size() == 1 && !mContactList.containsEmail());
    }

    private void dialRecipient(boolean isVideoCall) {
        if (isRecipientCallable()) {
            String number = mContactList.get(0).getNumber();
            Intent dialIntent = new Intent(Intent.ACTION_DIAL, Uri.parse("tel:" + number));
            if (isVideoCall) {
                dialIntent.putExtra("com.android.phone.extra.video", true);
            }
            startActivity(dialIntent);
        }
    }
    //add for cmcc dir ui end

    /**
     * Looks to see if there are any valid parts of the attachment that can be copied to a SD card.
     * @param msgId
     */
    private boolean haveSomethingToCopyToSDCard(long msgId) {
        PduBody body = null;
        try {
            body = SlideshowModel.getPduBody(MmsPlayerActivity.this, mMessageUri);
        } catch (MmsException e) {
            Log.e(TAG, "SlideshowModel.getPduBody MmsException:" + e.toString());
        }

        if (body == null) {
            return false;
        }

        boolean result = false;
        int partNum = body.getPartsNum();
        for (int i = 0; i < partNum; i++) {
            PduPart part = body.getPart(i);
            String type = new String(part.getContentType());

            if (MmsContentType.isImageType(type) || MmsContentType.isVideoType(type) ||
                    MmsContentType.isAudioType(type) || "application/ogg".equalsIgnoreCase(type)) {
                result = true;
                break;
            }

            // add for vcard
            if (FileAttachmentModel.isSupportedFile(part)) {
                result = true;
                break;
            }
        }

        /// M: OP01, Justify weather there are attachments in parts but not in slides
        // SlideshowModel mSlideShowModel = mWorkingMessage.getSlideshow();
        if (mOpMmsPlayerActivityExt.haveSomethingToCopyToSDCard(
                this, body, mOpMmsPlayerActivityCallback)) {
            return true;
        }
        return result;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        boolean succeeded = false;
        if (data != null && data.hasExtra("multi_save_result")) {
            succeeded = data.getBooleanExtra("multi_save_result", false);
            final int resId =
                succeeded ? R.string.copy_to_sdcard_success : R.string.copy_to_sdcard_fail;
            Toast.makeText(MmsPlayerActivity.this, resId, Toast.LENGTH_SHORT).show();
        }
        return;
    }

    public void onUpdate(Contact updated) {
        MmsLog.dpi(TAG, "onUpdate,number & name:" + updated.getNumber() + "," + updated.getName());
        if ((mContactList != null && mContactList.size() > 0) &&
                (updated.getNumber().equals(mContactList.get(0).getNumber()))) {
            updateRecipient();
        }
    }

    private void updateRecipient() {
        mContactList = ContactList.getByNumbers(mNumber, true, false);
        //update name
        if (mContactList.size() == 1) {
            mName = mContactList.get(0).getName();
        } else {
            mName = mContactList.getFirstName(null);
        }
        MmsLog.dpi(TAG, "mName:" + mName);
        if (mMsgBox == Mms.MESSAGE_BOX_INBOX) {
            mName = getString(R.string.via_without_time_for_send) + ": " + mName;
        } else {
            mName = getString(R.string.via_without_time_for_recieve) + ": " + mName;
        }
        final String showString = mName;
        runOnUiThread(new Runnable() {
            public void run() {
                mRecipient.setText(showString);
            }
        });
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        boolean ret = false;
        if (mOpMmsPlayerActivityExt != null) {
            ret = mOpMmsPlayerActivityExt.dispatchTouchEvent(ev);
        }
        if (!ret) {
            ret = super.dispatchTouchEvent(ev);
        }
        return ret;
    }


    public void setTextSize(float size) {

        if (mListAdapter != null) {
            mListAdapter.setTextSize(size);
        }
        if (mListView != null) {
            int count = mListView.getChildCount();
            for (int i = 0; i < count; i++) {
                View view = mListView.getChildAt(i);
                if (view != null && view instanceof MmsPlayerActivityItem) {
                    MmsPlayerActivityItem item = (MmsPlayerActivityItem) view;
                    item.setTextSize(size);
                }
            }
        }
    }

    private void lockMessage(final boolean lock) {

        final ContentValues values = new ContentValues(1);
        values.put("locked", lock ? 1 : 0);

        mLocked = lock;
        new Thread(new Runnable() {
            public void run() {
                getContentResolver().update(mMsgUri, values, null, null);
                runOnUiThread(new Runnable() {

                    public void run() {
                        mLockedInd.setVisibility(lock ? View.VISIBLE : View.GONE);
                    }
                });
            }
        }, "lockMessage").start();
    }

    private void asyncLoadMms() {
        mAsyncDialog.clearPendingProgressDialog();
        mAsyncDialog.runAsync(new Runnable() {

            @Override
            public void run() {
                // backupGroud load...
                // add for cmcc dir ui begin
                try {
                    // body = SlideshowModel.getPduBody(MmsPlayerActivity.this,
                    // msg);
                    mSlideshow = SlideshowModel.createFromMessageUri(MmsPlayerActivity.this,
                            mMessageUri);
                } catch (MmsException e) {
                    return;
                }
                // add for cmcc dir ui end
            }
        }, new Runnable() {

            @Override
            public void run() {
                // after load finish

                initListAdapter(mMessageUri);
                setListTextSize();
                invalidateOptionsMenu();
            }
        }, R.string.sync_mms_to_db);
    }

    private void setListTextSize() {
            float textSize = MessageUtils.getPreferenceValueFloat(this,
                    SettingListActivity.TEXT_SIZE, 18);
            setTextSize(textSize);
    }

    private void setSubIconAndLabel(int subId) {
        // Log.i(TAG, "setSubIconAndLabel subId=" +  subId);
        SubscriptionInfo subInfo = SubscriptionManager.from(MmsApp.getApplication())
                .getActiveSubscriptionInfo(subId);
        MmsLog.dpi(TAG, "subInfo=" + subInfo);
        if (null != subInfo) {
            if (subInfo.getSimSlotIndex() == SubscriptionManager.SIM_NOT_INSERTED ||
                subInfo.getSimSlotIndex() == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                Log.i(TAG, "current not insert sim card");
                subName.setVisibility(View.GONE);
            } else {
                subName.setVisibility(View.VISIBLE);
                subName.setTextColor(subInfo.getIconTint());
                subName.setText(subInfo.getDisplayName().toString());
            }
        }
    }

    public IOpMmsPlayerActivityExt mOpMmsPlayerActivityExt = null;
    public IMmsPlayerActivityCallback mOpMmsPlayerActivityCallback
            = new OpMmsPlayerActivityCallback();
    public class OpMmsPlayerActivityCallback implements IMmsPlayerActivityCallback {

        public boolean hasAttachFiles(Context context, PduBody body) {
            SlideshowModel slideShow = null;
            try {
                slideShow = SlideshowModel.createFromPduBody(context, body);
            } catch (MmsException e) {
                MmsLog.e(TAG, "Create from pdubody exception!");
            }
            if (slideShow != null && slideShow.getAttachFiles() != null
                    && slideShow.getAttachFiles().size() != 0) {
                return true;
            }

            return false;
        }

    }
}
