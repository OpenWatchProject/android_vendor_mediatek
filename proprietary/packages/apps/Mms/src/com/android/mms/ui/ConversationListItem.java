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

import android.content.Context;
import android.graphics.Typeface;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.net.Uri;
import android.os.Handler;
import android.os.RemoteException;
import android.provider.Telephony.Mms;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.style.ForegroundColorSpan;
import android.text.style.StyleSpan;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Display;
import android.view.IWindowManager;
import android.view.WindowManagerGlobal;
import android.view.View;
import android.view.ViewStub;
import android.widget.Checkable;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.QuickContactBadge;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.android.mms.MmsApp;
import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.data.Conversation;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsLog;
import com.mediatek.cb.cbmsg.CbMessage;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.ext.IOpConversationListItemExt;
import com.mediatek.mms.ipmessage.IIpConversationListItemExt;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.List;

import mediatek.telephony.MtkTelephony;


/**
 * This class manages the view for given conversation.
 */
public class ConversationListItem extends RelativeLayout implements Contact.UpdateListener,
            Checkable {
    private static final String TAG = "ConversationListItem";
    private static final boolean DEBUG = false;

    private TextView mSubjectView;
    private TextView mFromView;
    private TextView mDateView;
    private View mAttachmentView;
    private ImageView mErrorIndicator;
    /// M:
    private QuickContactBadge mAvatarView;
    private static Drawable sDefaultContactImage;
    private static Drawable sDefaultSelectedImage;

    // For posting UI update Runnables from other threads:
    private Handler mHandler = new Handler();

    private Conversation mConversation;

    public static final StyleSpan STYLE_BOLD = new StyleSpan(Typeface.BOLD);

    private Context mContext;
    /// M: add for new common feature.
    private View mMuteView;
    private TextView mUnreadView;
    private static final int MAX_UNREAD_MESSAGES_COUNT = 999;
    private static final String MAX_UNREAD_MESSAGES_STRING = "999+";
    private static final int MAX_READ_MESSAGES_COUNT = 9999;
    private static final String MAX_READ_MESSAGES_STRING = "9999+";

    // M: add for op
    private IOpConversationListItemExt mOpConversationListItemExt = null;

    /// M: New feature for rcse, adding IntegrationMode. @{
    private ImageView mFullIntegrationModeView;
    /// @}

    //add for ipmessage
    public IIpConversationListItemExt mIpConvListItem;

    public ConversationListItem(Context context) {
        super(context);
        mContext = context;
        mOpConversationListItemExt = OpMessageUtils.getOpMessagePlugin()
                .getOpConversationListItemExt();
    }

    public ConversationListItem(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        if (sDefaultContactImage == null) {
            sDefaultContactImage =
                context.getResources().getDrawable(R.drawable.ic_default_contact);
        }
        if (sDefaultSelectedImage == null) {
            sDefaultSelectedImage =
                context.getResources().getDrawable(R.drawable.ic_selected_item);
        }
        mOpConversationListItemExt = OpMessageUtils.getOpMessagePlugin()
                .getOpConversationListItemExt();
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mFromView = (TextView) findViewById(R.id.from);
        mSubjectView = (TextView) findViewById(R.id.subject);
        mDateView = (TextView) findViewById(R.id.date);
        mAttachmentView = findViewById(R.id.attachment);
        mErrorIndicator = (ImageView) findViewById(R.id.error);
        mAvatarView = (QuickContactBadge) findViewById(R.id.avatar);
        mAvatarView.setOverlay(null);
        /// M: add for ipmessage
        mMuteView = findViewById(R.id.mute);
        mUnreadView = (TextView) findViewById(R.id.unread);
        /// M: New feature for rcse, adding IntegrationMode. @{
        ViewStub stub = (ViewStub) findViewById(R.id.fullintegrationmode_stub);
        stub.inflate();
        mFullIntegrationModeView = (ImageView) findViewById(R.id.fullintegrationmode);

        mIpConvListItem = IpMessageUtils.getIpMessagePlugin(mContext).getIpConversationListItem();
        mIpConvListItem.onIpSyncView(mContext, mFullIntegrationModeView, mAvatarView);
        /// @}
        // add for op
        mOpConversationListItemExt.onFinishInflate(mSubjectView);
    }

    public Conversation getConversation() {
        return mConversation;
    }

    /**
     * Only used for header binding.
     */
    public void bind(String title, String explain) {
        mFromView.setText(title);
        mSubjectView.setText(explain);
    }

    private CharSequence formatMessage() {
        final int color = android.R.styleable.Theme_textColorSecondary;
        /// M: Code analyze 029, For new feature ALPS00111828, add CellBroadcast feature . @{
        String from = null;
        if (mConversation.getType() == MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD) {
            from = formatCbMessage();
        } else {
            if (mConversation.getRecipients() != null
                    && mConversation.getRecipients().size() > 0) {
                Contact contact = mConversation.getRecipients().get(0);
                from = mIpConvListItem.onIpFormatMessage(contact.getIpContact(mContext),
                        mConversation.getThreadId(), contact.getNumber(),
                        mConversation.getRecipients().formatNames(", "));
            } else {
                from = mIpConvListItem.onIpFormatMessage(null,
                        mConversation.getThreadId(), null, null);
            }

            if (TextUtils.isEmpty(from)) {
                from = mConversation.getRecipients().formatNames(", ");
            }
        }

        if (TextUtils.isEmpty(from)) {
            from = mContext.getString(android.R.string.unknownName);
        }
        /// @}

        SpannableStringBuilder buf = new SpannableStringBuilder(from);

        /// M:
        int before = buf.length();
        if (!mConversation.hasUnreadMessages()) {
            int count = mConversation.getMessageCount();
            if (count > 1) {
                if (count > MAX_READ_MESSAGES_COUNT) {
                    buf.append("  " + MAX_READ_MESSAGES_STRING);
                } else {
                    buf.append("  " + count);
                }
                buf.setSpan(new ForegroundColorSpan(
                        mContext.getResources().getColor(R.color.message_count_color)),
                        before, buf.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
            }
        }
/** M: Remove Google default code
        if (mConversation.hasDraft()) {
           // buf.append(mContext.getResources().getString(R.string.draft_separator));
            int before = buf.length();
            int size;
            buf.append(",  " + mContext.getResources().getString(R.string.has_draft));
            size = android.R.style.TextAppearance_Small;
            buf.setSpan(new TextAppearanceSpan(mContext, size, color), before + 1,
                    buf.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
            buf.setSpan(new ForegroundColorSpan(
                    mContext.getResources().getColor(R.drawable.text_color_red)),
                    before + 1, buf.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
        }

 */
        // Unread messages are shown in bold
        if (mConversation.hasUnreadMessages()) {
            buf.setSpan(STYLE_BOLD, 0, buf.length(),
                    Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
        }

        return buf;
    }

    private void updateAvatarView() {
        Drawable avatarDrawable;
        ConversationList conversationList = (ConversationList) ConversationList.getContext();
        if ((conversationList != null) && conversationList.isActionMode() && mConversation.isChecked()) {
            mAvatarView.setImageDrawable(sDefaultSelectedImage);
            mAvatarView.assignContactUri(null);
        } else {
            if (mIpConvListItem.updateIpAvatarView(mConversation.getIpConv(mContext), mAvatarView,
                    mAvatarView)) {
                return;
            }
            Uri photoUri = null;
            if (mConversation.getRecipients().size() == 1) {
                final Contact contact = mConversation.getRecipients().get(0);
                photoUri = contact.getPhotoUri();
                boolean isJoynNumber = mIpConvListItem.updateIpAvatarView(
                        contact.getIpContact(mContext),
                        contact.getNumber(), mAvatarView, contact.getUri());
                avatarDrawable = contact.getAvatar(mContext,
                        sDefaultContactImage, mConversation.getThreadId());

                /// M: fix bug ALPS00400483, clear all data of mAvatarView firstly. @{
                mAvatarView.assignContactUri(null);
                /// @}
                /// M: Code analyze 030, For new feature ALPS00241750, Add email address
                /// to email part in contact . @{
                String number = contact.getNumber();
                // add for joyn converged inbox mode
                if (isJoynNumber) {
                    number = number.substring(4);
                }
                if (Mms.isEmailAddress(number)) {
                    mAvatarView.assignContactFromEmail(number, true);
                } else {
                    if (contact.existsInDatabase()) {
                        mAvatarView.assignContactUri(contact.getUri());
                    } else {
                        mAvatarView.assignContactFromPhone(number, true);
                    }
                    /// @}
                }
            } else {
                // TODO get a multiple recipients asset (or do something else)
                avatarDrawable = sDefaultContactImage;
                mAvatarView.assignContactUri(null);
            }

            if ((conversationList != null) && conversationList.isActionMode()) {
                mAvatarView.assignContactUri(null);
            }
            mAvatarView.setImageDrawable(createCircleDrawable(avatarDrawable));
        }
    }

    private void updateFromView() {
        mFromView.setText(formatMessage());
        updateAvatarView();
    }

    private Runnable mUpdateFromViewRunnable = new Runnable() {
        public void run() {
            updateFromView();
        }
    };

    public void onUpdate(Contact updated) {
        //if (Log.isLoggable(LogTag.CONTACT, Log.DEBUG)) {
        if (updated == null || mConversation.getRecipients() == null
                || mConversation.getRecipients().size() < 1) {
            return;
        }
        Contact contact = mConversation.getRecipients().get(0);
        if (!updated.getNumber().equals(contact.getNumber())) {
            return;
        }
        MmsLog.ipi(TAG, "onUpdate: " + this + " contact: " + updated);
        //}
        /// M: fix blank screen issue. if there are 1000 threads, 1 recipient each thread,
        /// and 8 list items in each screen, onUpdate() will be called 8000 times.
        /// mUpdateFromViewRunnable run in UI thread will blocking the other things.
        /// remove blocked mUpdateFromViewRunnable.
        mHandler.removeCallbacks(mUpdateFromViewRunnable);
        mHandler.post(mUpdateFromViewRunnable);
    }

    public final void bind(Context context, final Conversation conversation) {
        //if (DEBUG) Log.v(TAG, "bind()");

        mConversation = conversation;

        /// M: change for op01, draft icon @{
        if (!mOpConversationListItemExt.bind(mContext, mDateView, mUnreadView,
                (ImageView) findViewById(R.id.sim_type_conv),
                conversation.hasDraft(), (LinearLayout) findViewById(R.id.iconlist))) {
            /// M: update read view
            if (mConversation.hasUnreadMessages()) {
                int unreadCount = mConversation.getUnreadMessageCount();
                String unreadString = null;
                if (unreadCount > MAX_UNREAD_MESSAGES_COUNT) {
                    unreadString = MAX_UNREAD_MESSAGES_STRING;
                } else {
                    unreadString = "" + unreadCount;
                }
                mUnreadView.setText(unreadString);
                mUnreadView.setVisibility(View.VISIBLE);
            } else {
                mUnreadView.setVisibility(View.GONE);
            }

            /// M: Code analyze 031, For bug ALPS00235723, The crolling performance of message . @{
            // Date
            mDateView.setVisibility(VISIBLE);
            String dateStr = MessageUtils.formatTimeStampStringExtend(
                    context, conversation.getDate());
            mDateView.setText(dateStr);
        }
        /// @}

        mConversation = conversation;

        updateBackground(conversation);

//        LayoutParams attachmentLayout = (LayoutParams)mAttachmentView.getLayoutParams();
        boolean hasError = conversation.hasError();
        // When there's an error icon, the attachment icon is left of the error icon.
        // When there is not an error icon, the attachment icon is left of the date text.
        // As far as I know, there's no way to specify that relationship in xml.
        /// M @{
        // if (hasError) {
        //     attachmentLayout.addRule(RelativeLayout.LEFT_OF, R.id.error);
        // } else {
        //     attachmentLayout.addRule(RelativeLayout.LEFT_OF, R.id.date);
        // }
        /// @}

        boolean hasAttachment = conversation.hasAttachment();
        mAttachmentView.setVisibility(hasAttachment ? VISIBLE : GONE);

        // set mSubjectView text before mIpConvListItem.onIpBind()
        if (mConversation.hasUnreadMessages()) {
            String subject = conversation.getSnippet();
            SpannableStringBuilder buf = new SpannableStringBuilder(subject);
            buf.setSpan(STYLE_BOLD, 0, buf.length(),
                    Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
            mSubjectView.setText(buf);
        } else {
            mSubjectView.setText(conversation.getSnippet());
        }

        // From.
        mFromView.setVisibility(VISIBLE);
        ConversationList conversationList = (ConversationList) ConversationList.getContext();
        boolean isActionMode = false;

        if (conversationList != null) {
            isActionMode = conversationList.isActionMode();
        }
        if (!mIpConvListItem.onIpBind(mConversation.getIpConv(mContext),
                isActionMode,
                conversation.isChecked(), mConversation.getType(), this,
                mFromView, mSubjectView, mDateView)) {
            mFromView.setText(formatMessage());
            updateAvatarView();
            /// M:
            mMuteView.setVisibility(View.GONE);

        /// M: this local variable has never been used. delete google default code.
        // Register for updates in changes of any of the contacts in this conversation.
        // ContactList contacts = conversation.getRecipients();

            if (DEBUG) {
                MmsLog.vpi(TAG, "bind: contacts.addListeners " + this);
            }
            Contact.addListener(this);
        }
        /// M: Code analyze 031, For bug ALPS00235723, The crolling performance of message .
        mSubjectView.setVisibility(VISIBLE);
 //       LayoutParams subjectLayout = (LayoutParams)mSubjectView.getLayoutParams();
 //       // We have to make the subject left of whatever optional items are shown on the right.
 //       subjectLayout.addRule(RelativeLayout.LEFT_OF, hasAttachment ? R.id.attachment :
  //          (hasError ? R.id.error : R.id.date));

        // Transmission error indicator.
        /// M: Code analyze 032, For new feature ALPS00347707, add for wap push error icon . @{
        if (FeatureOption.MTK_WAPPUSH_SUPPORT && hasError) {
            if (conversation.getType() == MtkTelephony.MtkThreads.WAPPUSH_THREAD) {
                mErrorIndicator.setImageResource(R.drawable.alert_wappush_si_expired);
            } else {
                mErrorIndicator.setImageResource(R.drawable.ic_list_alert_sms_failed);
            }
        }
        /// @}
        mErrorIndicator.setVisibility(hasError ? VISIBLE : GONE);
    }

    private void updateBackground(Conversation conversation) {
        int backgroundId;
        /// M: fix bug ALPS00998351, solute the issue "All of the threads still
        /// highlight after you back to all thread view". @{
        ConversationList conversationList = (ConversationList) ConversationList.getContext();
        if (conversationList == null) {
            return;
        }
        /// @}
        if (conversationList.isActionMode() && conversation.isChecked()) {
            backgroundId = R.drawable.conversation_item_background_unread;
        } else if (conversation.hasUnreadMessages()) {
            backgroundId = R.drawable.conversation_item_background_unread;
        } else {
            backgroundId = R.drawable.conversation_item_background_unread;
        }
        Drawable background = mContext.getResources().getDrawable(backgroundId);
        setBackgroundDrawable(background);
    }

    public final void unbind() {
        if (DEBUG) {
            Log.v(TAG, "unbind: contacts.removeListeners " + this);
        }
        // Unregister contact update callbacks.
        Contact.removeListener(this);
        mIpConvListItem.onIpUnbind();
        mOpConversationListItemExt.unbind();
    }
    public void setChecked(boolean checked) {
        mConversation.setIsChecked(checked);
        updateBackground(mConversation);
        updateAvatarView();
    }

    public boolean isChecked() {
        return mConversation.isChecked();
    }

    public void toggle() {
        mConversation.setIsChecked(!mConversation.isChecked());
    }

    /// M: Code analyze 031, For bug ALPS00235723, The crolling performance of message . @{
    public void bindDefault(Conversation conversation) {
        MmsLog.d(TAG, "bindDefault().");
        if (conversation  != null) {
            updateBackground(conversation);
        }
        mAttachmentView.setVisibility(GONE);
        mDateView.setVisibility(View.GONE);
        mFromView.setText(R.string.refreshing);
        mSubjectView.setVisibility(GONE);
        mUnreadView.setVisibility(View.GONE);
        mErrorIndicator.setVisibility(GONE);
        mAvatarView.setImageDrawable(sDefaultContactImage);
        /// M:
        mMuteView.setVisibility(View.GONE);
        mOpConversationListItemExt.bindDefault((ImageView) findViewById(R.id.sim_type_conv));
    }
    /// @}

    /// M: Make sure listeners are removed so that ConversationList instance can be released @{
    @Override
    protected void onDetachedFromWindow() {
        Log.i(TAG, "onDetachedFromWindow!!!");
        super.onDetachedFromWindow();
        Contact.removeListener(this);
        mIpConvListItem.onIpDetachedFromWindow();
    }
    /// @}

    private String formatCbMessage() {
        int channelId = 0;
        String from = null;
        if (mConversation.getRecipients().size() == 0) {
            return null;
        }
        MmsLog.dpi(TAG, "recipients = " + mConversation.getRecipients().formatNames(", "));
        String number = mConversation.getRecipients().get(0).getNumber();
        if (!TextUtils.isEmpty(number)) {
            try {
                channelId = Integer.parseInt(number);
            } catch (NumberFormatException e) {
                MmsLog.e(TAG, "format number error!");
            }
        }

        String name = "";
        List<SubscriptionInfo> subInfoList
                = SubscriptionManager.from(mContext).getActiveSubscriptionInfoList();
        int subCount = (subInfoList != null && !subInfoList.isEmpty()) ? subInfoList.size() : 0;
        for (int i = 0; i < subCount; i++) {
            name = CbMessage.getCBChannelName(subInfoList.get(i).getSubscriptionId(), channelId);
            if (!name.equals(mContext.getString(R.string.cb_default_channel_name))) {
                break;
            }
        }

        if (TextUtils.isEmpty(name)) {
            name = MmsApp.getApplication().getApplicationContext()
                    .getString(R.string.cb_default_channel_name);
        }
        try {
            from = name + "(" + channelId + ")";
        } catch (NumberFormatException e) {
            MmsLog.e(TAG, "format recipient number error!");
        }
        return from;
    }

    private Drawable createCircleDrawable(Drawable drawable) {
        BitmapDrawable bd = (BitmapDrawable)drawable;
        Bitmap bitmap = bd.getBitmap();
        bitmap = createCircleImage(bitmap);
        Drawable newDrawable = new BitmapDrawable(bitmap);
        return newDrawable;
    }

    private Bitmap createCircleImage(Bitmap bitmap) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int currentDpi = mContext.getResources().getConfiguration().densityDpi;
        int defaultDpi = getDefaultDisplayDensity();
        if (defaultDpi != -1 && currentDpi != defaultDpi) {
            width = width * currentDpi / defaultDpi;
            height = height * currentDpi / defaultDpi;
        }
        int diameter = width < height ? width : height;

        Bitmap output = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(output);
        final Paint paint = new Paint();
        paint.setColor(Color.WHITE);
        paint.setAntiAlias(true);
        canvas.drawCircle(width/2, height/2, diameter/2, paint);
        paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_IN));
        canvas.drawBitmap(bitmap, 0, 0, paint);
        return output;
    }

    private int getDefaultDisplayDensity() {
        try {
            final IWindowManager wm = WindowManagerGlobal.getWindowManagerService();
            return wm.getInitialDisplayDensity(Display.DEFAULT_DISPLAY);
        } catch (RemoteException e) {
            return -1;
        }
    }
}
