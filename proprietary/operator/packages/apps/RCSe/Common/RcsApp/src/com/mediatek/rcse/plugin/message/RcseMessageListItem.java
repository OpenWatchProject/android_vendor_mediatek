package com.mediatek.rcse.plugin.message;

import java.io.File;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Callable;

import com.mediatek.mms.ipmessage.DefaultIpMessageListItemExt;
import com.mediatek.mms.ipmessage.IIpMessageItemExt;
import com.mediatek.mms.callback.IMessageListItemCallback;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IpMessageConsts.DownloadAttachStatus;
import com.mediatek.rcse.plugin.message.IpMessageConsts.FeatureId;
import com.mediatek.rcse.plugin.message.IpMessageConsts.IpMessageStatus;
import com.mediatek.rcse.plugin.message.IpMessageConsts.IpMessageType;
import com.mediatek.rcse.plugin.message.IpMessageConsts.RemoteActivities;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;
import com.mediatek.storage.StorageManagerEx;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.media.ThumbnailUtils;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore.Video.Thumbnails;
import android.provider.Telephony.Sms;
import android.support.v4.content.FileProvider;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcs.R;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;

public class RcseMessageListItem extends DefaultIpMessageListItemExt
implements INotificationsListener {
    private static String TAG = "RcseMessageListItem";
    
    // / M: add for ip message, download file, accept or reject
    private View mIpmsgFileDownloadContrller; // ipmsg_file_downloading_controller_view
    private TextView mIpmsgResendButton; // ipmsg_resend
    private Button mIpmsgAcceptButton; // ipmsg_accept
    private Button mIpmsgRejectButton; // ipmsg_reject
    private View mIpmsgFileDownloadView; // ipmsg_file_download
    private TextView mIpmsgFileSize; // ipmsg_download_file_size
    private ImageView mIpmsgCancelDownloadButton; // ipmsg_download_file_cancel
    private ImageView mIpmsgPauseResumeButton; // ipmsg_download_file_resume
    private ProgressBar mIpmsgDownloadFileProgress; // ipmsg_download_file_progress

    // / M: add for ip message
    // / M: add for image and video
    private View mIpImageView; // ip_image
    private ImageView mImageContent; // image_content
    private View mIpImageSizeBg; // image_size_bg
    private ImageView mActionButton; // action_btn
    private TextView mContentSize; // content_size
    // private View mCaptionSeparator; // caption_separator
    private TextView mCaption; // text_caption
    private ImageView mMediaPlayView;
    // private View mVideoCaptionSeparator; // caption_separator
    // private TextView mVideoCaption; // text_caption
    // / M: add for audio
    private View mIpAudioView; // ip_audio
    private ImageView mAudioIcon; // ip_audio_icon
    private TextView mAudioInfo; // audio_info
    // / M: add for vcard
    private View mIpVCardView;
    private TextView mVCardInfo;
    // / M: add for vcalendar
    private View mIpVCalendarView;
    private TextView mVCalendarInfo;

    private TextView mBodyTextView;

    private TextView mSimStatus;

    private ImageView mSendingIndicator;
    private ImageView mSentIndicator;
    private ImageView mDeliveredIndicator;
    private ImageView mDisplayedIndicator;
    private ImageView mFailedIndicator;
    private ImageView mAccountIcon;

    private boolean isRegistNotificationListener = false;

    private long mIpMessageId;
    private long mMsgId;
    private Handler mHandler;
    private boolean isFileResumed = false;

    public static final int MSG_LIST_RESEND_IPMSG = 20;
    static final int MSG_LIST_NEED_REFRASH = 100;
    public static final int MESSAGE_LIST_REFRASH_WITH_CLEAR_CACHE = 1;

    private final static float MAX_SCALE = 0.4f;
    private final static float MIN_SCALE = 0.3f;
    private final static float COMP_NUMBER = 0.5f;

    public static final int text_view = 1;
    public static final int date_view = 2;
    public static final int sim_status = 3;
    public static final int account_icon = 4;
    public static final int locked_indicator = 5;
    public static final int delivered_indicator = 6;
    public static final int details_indicator = 7;
    public static final int avatar = 8;
    public static final int message_block = 9;
    public static final int select_check_box = 10;
    public static final int time_divider = 11;
    public static final int time_divider_str = 12;
    public static final int unread_divider = 13;
    public static final int unread_divider_str = 14;
    public static final int on_line_divider = 15;
    public static final int on_line_divider_str = 16;
    public static final int sim_divider = 17;
    public static final int text_expire = 18;
    public static final int sender_name = 19;
    public static final int sender_name_separator = 20;
    public static final int sender_photo = 21;
    public static final int send_time_txt = 22;
    public static final int double_time_layout = 23;
    public static final int mms_file_attachment_view_stub = 24;
    public static final int file_attachment_view = 25;
    public static final int file_attachment_thumbnail = 26;
    public static final int file_attachment_name_info = 27;
    public static final int file_attachment_name_info2 = 28;
    public static final int file_attachment_thumbnail2 = 29;
    public static final int file_attachment_size_info = 30;
    public static final int mms_view = 31;
    public static final int mms_layout_view_stub = 32;
    public static final int image_view = 33;
    public static final int play_slideshow_button = 34;
    public static final int mms_downloading_view_stub = 35;
    public static final int btn_download_msg = 36;
    public static final int label_downloading = 37;
    public static final int mms_download_controls = 38;
    public static final int status_panel = 39;
    public static final int delivered_sending = 40;
    public static final int delivered_failed = 41;
    public static final int delivered_displayed = 42;
    public static final int delivered_sent = 43;

    private static final int OUTGOING_ITEM_TYPE_IPMSG = 3;

    private static Timer mResumeTimer = new Timer();

    public IMessageListItemCallback mIpMessageItemCallback;
    public Context mContext;
    public RcseMessageItem mRcseMessageItem;
    public LinearLayout mItemView;

    private static final long FIRST_RETRY_TIME = 600000; // 10 Minutes
    private static final long SECOND_RETRY_TIME = 3600000; // 1 hour
    private static final long THIRD_RETRY_TIME = 86400000; // 24 Hours
    private static final long FOURTH_RETRY_TIME = 172800000; // 48 Hours
    private static final long FIFTH_RETRY_TIME = 259200000; // 72 Hours
    
    private boolean FIRST_RETRY_FINISHED = false;
    private boolean SECOND_RETRY_FINISHED = false;
    private boolean THIRD_RETRY_FINISHED = false;
    private boolean FOURH_RETRY_FINISHED = false;
    private boolean FIFTH_RETRY_FINISHED = false;

    public boolean IpMessageListItemInit() {

        return true;
    }

    @Override
    public boolean onIpFinishInflate(Context context, TextView bodyTextView,
            IMessageListItemCallback ipMessageItemCallback,
            Handler handler, LinearLayout itemView) {
    	Log.d(TAG, "onIpFinishInflate(): bodyTextView = " + bodyTextView + "itemView-" + itemView);
        mContext = context;
        mIpMessageItemCallback = ipMessageItemCallback;
        mHandler = handler;
        mItemView = itemView;
        mBodyTextView = (TextView)mItemView.findViewById(R.id.text_view);
        // / M: add for ip message
        // / M: add for image and video
        mIpImageView = (View) mItemView.findViewById(R.id.ip_image);
        mImageContent = (ImageView) mItemView.findViewById(R.id.image_content);
        mIpImageSizeBg = (View) mItemView.findViewById(R.id.image_size_bg);
        mActionButton = (ImageView) mItemView.findViewById(R.id.action_btn);
        mContentSize = (TextView) mItemView.findViewById(R.id.content_size);
        // mCaptionSeparator = (View) findViewById(R.id.caption_separator);
        mCaption = (TextView) mItemView.findViewById(R.id.text_caption);
        mMediaPlayView = (ImageView) mItemView.findViewById(R.id.video_media_paly);
        // / M: add for audio
        mIpAudioView = (View) mItemView.findViewById(R.id.ip_audio);
        mAudioIcon = (ImageView) mItemView.findViewById(R.id.ip_audio_icon);
        mAudioInfo = (TextView) mItemView.findViewById(R.id.audio_info);
        // / M: add for vCard
        mIpVCardView = (View) mItemView.findViewById(R.id.ip_vcard);
        mVCardInfo = (TextView) mItemView.findViewById(R.id.vcard_info);
        // / M: add for vCalendar
        mIpVCalendarView = (View) mItemView.findViewById(R.id.ip_vcalendar);
        mVCalendarInfo = (TextView) mItemView.findViewById(R.id.vcalendar_info);
        mSimStatus = (TextView) mItemView.findViewById(R.id.sim_status);
        Log.d(TAG, "onIpFinishInflate(): mSimStatus = " + mSimStatus);
        
        mSendingIndicator = (ImageView) mItemView.findViewById(R.id.delivered_sending);
        mSentIndicator = (ImageView) mItemView.findViewById(R.id.delivered_success);
        mDeliveredIndicator = (ImageView) mItemView.findViewById(R.id.delivered_indicator);
        mDisplayedIndicator = (ImageView) mItemView.findViewById(R.id.displayed_indicator);
        mFailedIndicator = (ImageView) mItemView.findViewById(R.id.delivered_failed);
        if(mFailedIndicator!=null)
            mFailedIndicator.setVisibility(View.GONE);
        mAccountIcon = (ImageView) mItemView.findViewById(R.id.account_icon);
        
        LinearLayout contentView = (LinearLayout) mItemView.findViewById(R.id.mms_layout_view_parent);
        if(RcsSettings.getInstance().isSupportOP08() && contentView != null) {
            contentView.setBackground(IpMessageResourceMananger.getInstance(
                                    mContext).getSingleDrawable(
                                    IpMessageConsts.drawable.mms_send_msg_background));
        }
        
        // / M: add for ip message, file download
        if (IpMessageServiceMananger.getInstance(mContext).isFeatureSupported(
                FeatureId.FILE_TRANSACTION)) {
            mIpmsgFileDownloadContrller = (View) mItemView.findViewById(R.id.ipmsg_file_downloading_controller_view);
            mIpmsgResendButton = (TextView) mItemView.findViewById(R.id.ipmsg_resend);
            mIpmsgAcceptButton = (Button) mItemView.findViewById(R.id.ipmsg_accept);
            mIpmsgRejectButton = (Button) mItemView.findViewById(R.id.ipmsg_reject);
            mIpmsgFileDownloadView = (View) mItemView.findViewById(R.id.ipmsg_file_download);
            mIpmsgFileSize = (TextView) mItemView.findViewById(R.id.ipmsg_download_file_size);
            mIpmsgCancelDownloadButton = (ImageView) mItemView.findViewById(R.id.ipmsg_download_file_cancel);
            mIpmsgPauseResumeButton = (ImageView) mItemView.findViewById(R.id.ipmsg_download_file_resume);
            mIpmsgDownloadFileProgress = (ProgressBar) mItemView.findViewById(R.id.ipmsg_download_file_progress);
        }
        return true;
    }

    @Override
    public boolean onIpBind(IIpMessageItemExt ipMessageItem, long msgId, long ipMessageId,
            boolean isDeleteMode) {
        Logger.v(TAG, "bindView(): IpMessageId = " + ipMessageId);
        mRcseMessageItem = (RcseMessageItem) ipMessageItem;
        if (ipMessageId > 0) {
            mIpMessageId = ipMessageId;
            mMsgId = msgId;
            bindIpmsg(false);
            return true;
        }
        return false;
    }
    
    @Override
    public boolean onIpDetachedFromWindow() {
        /// M: add for ip message, remove notification listener
        if (isRegistNotificationListener) {
            Logger.v(TAG, "listItem.unbind(): remove noti listener.");
            IpMessageUtils.removeIpMsgNotificationListeners(mContext, this);
            isRegistNotificationListener = false;
            return true;
        }
        return false;
    }

    public static String getIpTextMessageDetails(Context context, RcseMessageItem msgItem) {
        StringBuilder details = new StringBuilder();
        Resources res = context.getResources();
        Logger.v(TAG, "getIpTextMessageDetails(): entry " + msgItem.mAddress);
        // Message Type: Text message.
        details.append("Type:");
        details.append("Joyn Message");
        if (msgItem.mAddress.startsWith(IpMessageConsts.JOYN_START)) {
            msgItem.mAddress = msgItem.mAddress.substring(4);
        }

        // Address: ***
        int smsType = msgItem.mBoxId;
        if (msgItem.mBoxId == Sms.MESSAGE_TYPE_INBOX) {
            details.append('\n');
            details.append("From:");
            details.append(msgItem.mAddress);
        } else {           
                details.append('\n');
                details.append("To:");
                details.append(msgItem.mAddress);
           
        }
        /*
        if (msgItem.mSentDate > 0 && smsType == Sms.MESSAGE_TYPE_INBOX) {
            details.append('\n');
            details.append(res.getString(R.string.sent_label));
            String dateStr = "";
            /// M: For OP09 @{
            if (MmsConfig.isFormatDateAndTimeStampEnable()) {
                dateStr = mmsUtils.formatDateAndTimeStampString(context, 0, msgItem.mSmsSentDate, true, dateStr);
            /// @}
            } else {
                dateStr = MessageUtils.formatTimeStampString(context, msgItem.mSmsSentDate, true);
            }

            details.append(dateStr);
        }
        */
        if (msgItem.mDate > 0L) {
            details.append('\n');
            if (smsType == Sms.MESSAGE_TYPE_INBOX) {
                details.append("Received:");
            } else {
                details.append("Sent:");
            }
            String dateStr = IpMessageUtils.formatIpTimeStampString( msgItem.mDate, true, context);
            details.append(dateStr);
        }
        Logger.v(TAG, "getIpTextMessageDetails(): exit " +  details.toString());
        return details.toString();
    }

    @Override
    public boolean onIpMessageListItemClick() {
        // / M: add for ipmessage
        IpMessage ipMessage = IpMessageManager.getInstance(mContext).getIpMsgInfo(mMsgId);
        if (null != ipMessage) {
            if (ipMessage.getType() != IpMessageType.TEXT) {
                Logger.v(TAG, "onMessageListItemClick(): open IP message media. msgId = " + mMsgId);
                openMedia(ipMessage);
                return true;
            }
        }
        return false;
    }

    @Override
    public boolean onIpSetBodyTextSize(float size) {
        if (mCaption != null && mCaption.getVisibility() == View.VISIBLE) {
            mCaption.setTextSize(size);
            return true;
        }
        return false;
    }
    
    public boolean onIpBindDefault() {
      /// M: add for ip message
        if (mIpImageView != null) {
            mIpImageView.setVisibility(View.GONE);
        }
        if (mCaption != null) {
            mCaption.setVisibility(View.GONE);
        }
        if (mIpAudioView != null) {
            mIpAudioView.setVisibility(View.GONE);
        }
        if (mIpVCardView != null) {
            mIpVCardView.setVisibility(View.GONE);
        }
        if (mIpVCalendarView != null) {
            mIpVCalendarView.setVisibility(View.GONE);
        }

        /// M: hide file transfer view
        if (mIpmsgFileDownloadContrller != null) {
            mIpmsgFileDownloadContrller.setVisibility(View.GONE);
        }
        if (mIpmsgFileDownloadView != null) {
            mIpmsgFileDownloadView.setVisibility(View.GONE);
        }
        return true;
    }

    @Override
    public boolean onIpSetMmsImage() {
        if (null != mMediaPlayView && mMediaPlayView.getVisibility() == View.VISIBLE) {
            mMediaPlayView.setVisibility(View.GONE);
        }
        return true;
    }

    @Override
    public View findIpView(int id, View parentView) {
        switch (id) {
            case text_view:
                return parentView.findViewById(R.id.text_view);
            case date_view:
                return parentView.findViewById(R.id.date_view);
            case sim_status:
                return parentView.findViewById(R.id.sim_status);
            case account_icon:
            	 Log.d(TAG, "findIpView(): icon = " + parentView.findViewById(R.id.account_icon));
                return parentView.findViewById(R.id.account_icon);
            case locked_indicator:
                return parentView.findViewById(R.id.locked_indicator);
            case delivered_indicator:
                return parentView.findViewById(R.id.delivered_indicator);
            case details_indicator:
                return parentView.findViewById(R.id.details_indicator);
            case avatar:
                return parentView.findViewById(R.id.avatar);
            case select_check_box:
                return parentView.findViewById(R.id.select_check_box);
            case time_divider:
                return parentView.findViewById(R.id.time_divider);
            case time_divider_str:
                return parentView.findViewById(R.id.time_divider_str);
            case unread_divider:
                return parentView.findViewById(R.id.unread_divider);
            case unread_divider_str:
                return parentView.findViewById(R.id.unread_divider_str);
            case on_line_divider:
                return parentView.findViewById(R.id.on_line_divider);
            case on_line_divider_str:
                return parentView.findViewById(R.id.on_line_divider_str);
            case sim_divider:
                return parentView.findViewById(R.id.sim_divider);
            case text_expire:
                return parentView.findViewById(R.id.text_expire);
            case sender_name:
                return parentView.findViewById(R.id.sender_name);
            case sender_name_separator:
                return parentView.findViewById(R.id.sender_name_separator);
            case sender_photo:
                return parentView.findViewById(R.id.sender_photo);
            case send_time_txt:
                return parentView.findViewById(R.id.send_time_txt);
            case double_time_layout:
                return parentView.findViewById(R.id.double_time_layout);
            case mms_file_attachment_view_stub:
                return parentView.findViewById(R.id.mms_file_attachment_view_stub);
            case file_attachment_view:
                return parentView.findViewById(R.id.file_attachment_view);
            case file_attachment_thumbnail:
                return parentView.findViewById(R.id.file_attachment_thumbnail);
            case file_attachment_name_info:
                return parentView.findViewById(R.id.file_attachment_name_info);
            case file_attachment_name_info2:
                return parentView.findViewById(R.id.file_attachment_name_info2);
            case file_attachment_thumbnail2:
                return parentView.findViewById(R.id.file_attachment_thumbnail2);
            case file_attachment_size_info:
                return parentView.findViewById(R.id.file_attachment_size_info);
            case mms_view:
                return parentView.findViewById(R.id.mms_view);
            case mms_layout_view_stub:
                return parentView.findViewById(R.id.mms_layout_view_stub);
            case image_view:
                return parentView.findViewById(R.id.image_view);
            case play_slideshow_button:
                return parentView.findViewById(R.id.play_slideshow_button);
            case mms_downloading_view_stub:
                return parentView.findViewById(R.id.mms_downloading_view_stub);
            case btn_download_msg:
                return parentView.findViewById(R.id.btn_download_msg);
            case label_downloading:
                return parentView.findViewById(R.id.label_downloading);
            case mms_download_controls:
                return parentView.findViewById(R.id.mms_download_controls);
            case status_panel:
                return parentView.findViewById(R.id.status_panel);
            default:
        }
        Log.d(TAG, "findIpView(): icon = null return");
        return null;
    }

    // / M: add for ipmessage
    public boolean bindIpmsg(boolean isDeleteMode) {
        // / M: add for ipmessage, notification listener
        Log.d(TAG, "bindIpmsg(): msgId = " + mMsgId);
        IpMessage ipMessage = IpMessageManager.getInstance(mContext).getIpMsgInfo(mMsgId);
        if (null == ipMessage) {
            Logger.v(TAG, "bindIpmsg(): ip message is null!");
            return false;
        }
        if (!isRegistNotificationListener) {
            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                    "listItem.bindIpmsg(): regist noti listener.");
            isRegistNotificationListener = true;
            IpMessageUtils.addIpMsgNotificationListeners(mContext, this);
        }

        // / M: hide file transfer view
        if (mIpmsgFileDownloadContrller != null) {
            mIpmsgFileDownloadContrller.setVisibility(View.GONE);
        }
        if (mIpmsgFileDownloadView != null) {
            mIpmsgFileDownloadView.setVisibility(View.GONE);
        }
        int ipMsgStatus = ipMessage.getStatus();
        boolean isFileTransferStatus = isFileTransferStatus(ipMsgStatus);
        boolean showContent = isIpMessageShowContent(ipMsgStatus);
        Logger.v(TAG, "bindIpmsg(): ipMsgStatus! " + ipMsgStatus);
        switch (ipMessage.getType()) {
            case IpMessageType.TEXT:
                setIpTextItem(ipMessage, isDeleteMode);
                break;
            case IpMessageType.PICTURE:
                setIpImageItem(ipMessage, isDeleteMode, isFileTransferStatus, showContent);
                break;
            case IpMessageType.VOICE:
                setIpVoiceItem(ipMessage, isDeleteMode, isFileTransferStatus, showContent);
                break;
            case IpMessageType.VCARD:
                setIpVCardItem(ipMessage, isDeleteMode, isFileTransferStatus, showContent);
                break;
            case IpMessageType.VIDEO:
                setIpVideoItem(ipMessage, isDeleteMode, isFileTransferStatus, showContent);
                break;
            case IpMessageType.CALENDAR:
                setIpVCalendarItem(ipMessage, isDeleteMode, isFileTransferStatus, showContent);
                break;
            case IpMessageType.UNKNOWN_FILE:
            case IpMessageType.COUNT:
                Logger.e(TAG,
                        "bindIpmsg(): Unknown IP message type. type = " + ipMessage.getType());
                setIpUnknownItem(ipMessage, isDeleteMode, isFileTransferStatus, showContent);
                break;
            case IpMessageType.GROUP_CREATE_CFG:
            case IpMessageType.GROUP_ADD_CFG:
            case IpMessageType.GROUP_QUIT_CFG:
                // / M: group chat type
                Logger.v(TAG,
                        "bindIpmsg(): Group IP message type. type = " + ipMessage.getType());
                break;
            default:
                Logger.v(TAG,
                        "bindIpmsg(): Error IP message type. type = " + ipMessage.getType());
                break;
        }

        if (isFileTransferStatus) {
            drawDownloadFileView(ipMessage, ipMsgStatus, ipMessage.getType());
        }
        String subStatus = IpMessageResourceMananger.getInstance(mContext).getSimStatus();
        if(mSimStatus != null)
        {
        	mSimStatus.setText(subStatus);
            Logger.v(TAG,
                    "bindIpmsg(): mSimStatus : subStatus : " + subStatus + "mAccountIcon:" + mAccountIcon);
        }
        mIpMessageItemCallback.setSubDateView(subStatus);
        if(RcsSettings.getInstance().isSupportOP08()) {
            mAccountIcon.setVisibility(View.GONE);
        }
		if (mAccountIcon != null){
			mAccountIcon.setImageDrawable(IpMessageResourceMananger
					.getInstance(mContext).getSingleDrawable(
							IpMessageConsts.drawable.ipmsg_jump_to_joyn));
		}
		if(ipMsgStatus != -1)
        drawRightStatusIndicator(ipMsgStatus);
        return true;
    }

    private void drawRightStatusIndicator(int status) {
        // Locked icon
    	Log.i(TAG, "drawRightStatusIndicator status =" + status );       
        if (status == IpMessageConsts.IpMessageStatus.OUTBOX || status == IpMessageConsts.IpMessageStatus.MO_INVITE) {
        	Log.i(TAG, "drawRightStatusIndicator=SENDING" );
        	 if(mSendingIndicator!=null)
            mSendingIndicator.setVisibility(View.VISIBLE);
        	 if(mDeliveredIndicator!=null)
            mDeliveredIndicator.setVisibility(View.GONE);
        	 if(mDisplayedIndicator!=null)
            mDisplayedIndicator.setVisibility(View.GONE);
        	 if(mFailedIndicator!=null)
            mFailedIndicator.setVisibility(View.GONE);
            if(mSentIndicator!=null)
                mSentIndicator.setVisibility(View.GONE);
        } else if (status == IpMessageConsts.IpMessageStatus.MO_SENT || status == IpMessageConsts.IpMessageStatus.SENT) {
        	Log.i(TAG, "drawRightStatusIndicator=SENT" );
            /// M: @{
        	if(mSentIndicator!=null)
               mSentIndicator.setVisibility(View.VISIBLE);
        	if(mDeliveredIndicator!=null)
            mDeliveredIndicator.setClickable(false); 
        	if(mDisplayedIndicator!=null)
            mDisplayedIndicator.setVisibility(View.GONE);
            if(mSendingIndicator!=null)
            mSendingIndicator.setVisibility(View.GONE);
            if(mDeliveredIndicator!=null)
            mDeliveredIndicator.setVisibility(View.GONE);
            if(mFailedIndicator!=null)
            mFailedIndicator.setVisibility(View.GONE);
        } else if (status == IpMessageConsts.IpMessageStatus.DELIVERED || status == IpMessageConsts.IpMessageStatus.MO_DELIVERED) {
        	Log.i(TAG, "drawRightStatusIndicator=DELIVERED" );
            /// M: @{
            if(mDeliveredIndicator!=null) {
                Resources mResource = null;
                try {
            mDeliveredIndicator.setClickable(false); 
                    mResource = MediatekFactory.getApplicationContext()
                            .getPackageManager().getResourcesForApplication(CoreApplication.APP_NAME);
                } catch (NameNotFoundException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                mDeliveredIndicator.setImageDrawable(mResource.getDrawable(IpMessageUtils.
                getIpMessageStatusResourceId(IpMessageStatus.DELIVERED)));
                }
            if(mDisplayedIndicator!=null)
            mDisplayedIndicator.setVisibility(View.GONE);
            if(mSentIndicator!=null)
                mSentIndicator.setVisibility(View.GONE);
            if(mSendingIndicator!=null)
            mSendingIndicator.setVisibility(View.GONE);
            if(mDeliveredIndicator!=null)
            mDeliveredIndicator.setVisibility(View.VISIBLE);
            if(mFailedIndicator!=null)
            mFailedIndicator.setVisibility(View.GONE);
        }
        else if (status == IpMessageConsts.IpMessageStatus.VIEWED || status == IpMessageConsts.IpMessageStatus.MO_VIEWED) {
        	Log.i(TAG, "drawRightStatusIndicator=DISPLAYED" );
            /// M: @{
        	if(mSendingIndicator!=null)
        	mSendingIndicator.setVisibility(View.GONE);           
        	 if(mSentIndicator!=null)
                 mSentIndicator.setVisibility(View.GONE);
        	if(mDeliveredIndicator!=null)
            mDeliveredIndicator.setVisibility(View.GONE);
        	if(mDisplayedIndicator!=null)
            mDisplayedIndicator.setVisibility(View.VISIBLE);
        	if(mFailedIndicator!=null)
            mFailedIndicator.setVisibility(View.GONE);
        }else if (status == IpMessageConsts.IpMessageStatus.FAILED || status == IpMessageConsts.IpMessageStatus.MO_REJECTED
                    || status == IpMessageConsts.IpMessageStatus.MO_CANCEL || status == IpMessageConsts.IpMessageStatus.MO_TIMEOUT || status == IpMessageConsts.IpMessageStatus.MO_FALLBACK
                    || status == IpMessageConsts.IpMessageStatus.MT_NOT_RECEIVED) {
        	Log.i(TAG, "drawRightStatusIndicator=FAILED" );
        	 if(mSentIndicator!=null)
                 mSentIndicator.setVisibility(View.GONE);
        	if(mSendingIndicator!=null)
        	mSendingIndicator.setVisibility(View.GONE);           
            if(mDeliveredIndicator!=null)
            mDeliveredIndicator.setVisibility(View.GONE);
            if(mDisplayedIndicator!=null)
            mDisplayedIndicator.setVisibility(View.GONE);
            if(mFailedIndicator!=null)
            mFailedIndicator.setVisibility(View.VISIBLE);
        }else if (status == IpMessageConsts.IpMessageStatus.MO_SENDING) {
            Log.i(TAG, "drawRightStatusIndicator=MO_SENDING_TRANSFERRING" );
            if(mSentIndicator!=null)
                mSentIndicator.setVisibility(View.GONE);
            /// M: @{
            if(mSendingIndicator!=null)
            mSendingIndicator.setVisibility(View.GONE);           
            if(mDeliveredIndicator!=null)
            mDeliveredIndicator.setVisibility(View.GONE);
            if(mDisplayedIndicator!=null)
            mDisplayedIndicator.setVisibility(View.GONE);
            if(mFailedIndicator!=null)
            mFailedIndicator.setVisibility(View.GONE);
        }
        else {
        	Log.i(TAG, "drawRightStatusIndicator=ELSE" );
        	 if(mSentIndicator!=null)
                 mSentIndicator.setVisibility(View.GONE);
        	if(mSendingIndicator!=null)
        	    mSendingIndicator.setVisibility(View.GONE);           
        	if(mDeliveredIndicator!=null)
                mDeliveredIndicator.setVisibility(View.GONE);
        	if(mDisplayedIndicator!=null)
                mDisplayedIndicator.setVisibility(View.GONE);
        	if(mFailedIndicator!=null)
        	    mFailedIndicator.setVisibility(View.GONE);
             	
        }     
       
    }
    

    private void setIpTextItem(IpMessage ipMessage, boolean isDeleteMode) {
        Logger.v(TAG, "setIpTextItem(): ipMessage = " + ipMessage);
        IpTextMessage textMessage = (IpTextMessage) ipMessage;
        if (TextUtils.isEmpty(textMessage.getBody())) {
            Logger.w(TAG, "setIpTextItem(): No message content!");
            return;
        }

        mIpMessageItemCallback.setTextMessage(textMessage.getBody());

        // / M: add for ip message, hide audio, vCard, vCalendar
        mIpImageView.setVisibility(View.GONE);
        mIpAudioView.setVisibility(View.GONE);
        // mCaptionSeparator.setVisibility(View.GONE);
        mCaption.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
    }

    private void setIpImageItem(IpMessage ipMessage, boolean isDeleteMode,
            boolean isFileTransferStatus, boolean showContent) {
        IpImageMessage imageMessage = (IpImageMessage) ipMessage;
        Logger.v(TAG, "setIpImageItem():" + " ipThumbPath:"
                + imageMessage.getThumbPath() + " imagePath:" + imageMessage.getPath());
        mIpImageView.setVisibility(View.VISIBLE);

        // joyn add for show thumbnail when recieved image message
        if (!TextUtils.isEmpty(imageMessage.getThumbPath())) {
            showContent = true;
        }
        if (isFileTransferStatus && !showContent) {
            mIpImageView.setVisibility(View.GONE);
        } else if (!setPicView(ipMessage, imageMessage.getPath())) {
            setPicView(ipMessage, imageMessage.getThumbPath());
        }

        mIpImageSizeBg.setVisibility(View.GONE);

        if (!TextUtils.isEmpty(imageMessage.getCaption())) {
            // mCaptionSeparator.setVisibility(View.VISIBLE);
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setText(imageMessage.getCaption());
        } else {
            // mCaptionSeparator.setVisibility(View.GONE);
            mCaption.setVisibility(View.GONE);
        }

        // / M: add for ip message, hide text, audio, vCard, vCalendar
        mBodyTextView.setVisibility(View.GONE);
        mIpAudioView.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
        mMediaPlayView.setVisibility(View.GONE);
    }

    private void setIpVoiceItem(IpMessage ipMessage, boolean isDeleteMode,
            boolean isFileTransferStatus, boolean showContent) {
        Logger.v(TAG, "setIpVoiceItem():");
        IpVoiceMessage voiceMessage = (IpVoiceMessage) ipMessage;
        // mAudioOrVcardIcon.setImageResource(R.drawable.ic_soundrecorder);
        // / M: add for ip message, show audio view
        mIpAudioView.setVisibility(View.VISIBLE);

        if (TextUtils.isEmpty(voiceMessage.getCaption())) {
            // mCaptionSeparator.setVisibility(View.GONE);
            mCaption.setVisibility(View.GONE);
        } else {
            // mCaptionSeparator.setVisibility(View.VISIBLE);
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setText(voiceMessage.getCaption());
        }

        // / M: add for ip message, hide text, image, audio, vCard, vCalendar
        mBodyTextView.setVisibility(View.GONE);
        mIpImageView.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
    }

    private void setIpVCardItem(IpMessage ipMessage, boolean isDeleteMode,
            boolean isFileTransferStatus, boolean showContent) {
        Logger.v(TAG, "setIpVCardItem():");
        IpVCardMessage vCardMessage = (IpVCardMessage) ipMessage;
        String name = vCardMessage.getName();
        if (name != null && name.lastIndexOf(".") != -1) {
            name = name.substring(0, name.lastIndexOf("."));
        }
        if (isFileTransferStatus && !showContent) {
            mIpVCardView.setVisibility(View.GONE);
        } else {
            mVCardInfo.setText(name);
            mIpVCardView.setVisibility(View.VISIBLE);
        }

        // / M: add for ip message, hide text, image, audio, vCalendar
        mBodyTextView.setVisibility(View.GONE);
        mIpImageView.setVisibility(View.GONE);
        mIpAudioView.setVisibility(View.GONE);
        // mCaptionSeparator.setVisibility(View.GONE);
        mCaption.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
    }

    private void setIpVCalendarItem(IpMessage ipMessage, boolean isDeleteMode,
            boolean isFileTransferStatus, boolean showContent) {
        Logger.v(TAG, "setIpVCalendarItem():");
        IpVCalendarMessage vCalendarMessage = (IpVCalendarMessage) ipMessage;
        String summary = vCalendarMessage.getSummary();
        if (summary != null && summary.lastIndexOf(".") != -1) {
            summary = summary.substring(0, summary.lastIndexOf("."));
        }
        if (isFileTransferStatus && !showContent) {
            mIpVCalendarView.setVisibility(View.GONE);
        } else {
            mVCalendarInfo.setText(summary);
            mIpVCalendarView.setVisibility(View.VISIBLE);
        }

        mBodyTextView.setVisibility(View.GONE);
        mIpImageView.setVisibility(View.GONE);
        mIpAudioView.setVisibility(View.GONE);
        // mCaptionSeparator.setVisibility(View.GONE);
        mCaption.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
    }

    private void setIpVideoItem(IpMessage ipMessage, boolean isDeleteMode,
            boolean isFileTransferStatus, boolean showContent) {
        Logger.v(TAG, "setIpVideoItem():");
        mIpImageView.setVisibility(View.VISIBLE);
        mMediaPlayView.setVisibility(View.VISIBLE);
        IpVideoMessage videoMessage = (IpVideoMessage) ipMessage;

        if (isFileTransferStatus && !showContent) {
            mIpImageView.setVisibility(View.GONE);
            mMediaPlayView.setVisibility(View.GONE);
        } else {
            setVideoView(videoMessage.getPath(), videoMessage.getThumbPath());
        }
        mIpImageSizeBg.setVisibility(View.GONE);

        if (TextUtils.isEmpty(videoMessage.getCaption())) {
            // mCaptionSeparator.setVisibility(View.GONE);
            mCaption.setVisibility(View.GONE);
        } else {
            // mCaptionSeparator.setVisibility(View.VISIBLE);
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setText(videoMessage.getCaption());
        }

        // / M: add for ip message, hide text, audio, vCard, vCalendar
        mBodyTextView.setVisibility(View.GONE);
        mIpAudioView.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
    }

    private void setIpUnknownItem(IpMessage ipMessage, boolean isDeleteMode,
            boolean isFileTransferStatus, boolean showContent) {
        Logger.v(TAG, "setIpUnknownItem():" + isFileTransferStatus);
        mIpImageView.setVisibility(View.VISIBLE);
        mMediaPlayView.setVisibility(View.GONE);
        mIpImageSizeBg.setVisibility(View.GONE);  
        mBodyTextView.setVisibility(View.GONE);
        mIpAudioView.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
    }

    private boolean setPicView(IpMessage ipMessage, String filePath) {
        Logger.v(TAG, "setPicView(): filePath = " + filePath + ", imageView = "
                + mImageContent);
        if (TextUtils.isEmpty(filePath) || null == mImageContent) {
            return false;
        }
        Bitmap bitmap = mRcseMessageItem.getIpMessageBitmap();
        if (null == bitmap) {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            bitmap = BitmapFactory.decodeFile(filePath, options);
            int width = options.outWidth;
            int height = options.outHeight;
            int w = options.outWidth;

            // / M: get screen width
            DisplayMetrics dm = new DisplayMetrics();
            int screenWidth = 0;
            WindowManager wmg = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
            wmg.getDefaultDisplay().getMetrics(dm);
            if (dm.heightPixels > dm.widthPixels) {
                screenWidth = dm.widthPixels;
            } else {
                screenWidth = dm.heightPixels;
            }
            // / M: the returned bitmap's w/h is different with the input!
            if (width > screenWidth * MAX_SCALE) {
                w = (int) (screenWidth * MAX_SCALE);
                bitmap = IpMessageUtils.getBitmapByPath(filePath, options, w, height * w / width);
                mRcseMessageItem.setIpMessageBitmapSize(w, height * w / width);
            } else if (width > screenWidth * MIN_SCALE) {
                w = (int) (screenWidth * MIN_SCALE);
                bitmap = IpMessageUtils.getBitmapByPath(filePath, options, w, height * w / width);
                mRcseMessageItem.setIpMessageBitmapSize(w, height * w / width);
            } else {
                bitmap = IpMessageUtils.getBitmapByPath(filePath, options, width, height);
                mRcseMessageItem.setIpMessageBitmapSize(width, height);
            }

            mRcseMessageItem.setIpMessageBitmapCache(bitmap);
        }

        if (null != bitmap) {
            ViewGroup.LayoutParams params = (ViewGroup.LayoutParams) mImageContent
                    .getLayoutParams();
            params.height = mRcseMessageItem.getIpMessageBitmapHeight();
            params.width = mRcseMessageItem.getIpMessageBitmapWidth();
            mImageContent.setLayoutParams(params);
            mImageContent.setImageBitmap(bitmap);
            return true;
        } else {
            mImageContent.setImageResource(R.drawable.ic_missing_thumbnail_picture);
            return false;
        }
    }

    public void setVideoView(String path, String bakPath) {
        Bitmap bp = null;
        int degree = 0;
        mMediaPlayView.setVisibility(View.VISIBLE);

        if (!TextUtils.isEmpty(path)) {
            bp = ThumbnailUtils.createVideoThumbnail(path, Thumbnails.MICRO_KIND);
            degree = IpMessageUtils.getExifOrientation(path);
        }

        if (null == bp) {
            if (!TextUtils.isEmpty(bakPath)) {
                BitmapFactory.Options options = IpMessageUtils.getOptions(bakPath);
                options.inJustDecodeBounds = true;
                BitmapFactory.decodeFile(bakPath, options);
                bp = IpMessageUtils.getBitmapByPath(bakPath, IpMessageUtils.getOptions(bakPath),
                        options.outWidth, options.outHeight);
                degree = IpMessageUtils.getExifOrientation(bakPath);
            }
        }
        /**
         * M: we use the same view show image/big video snap, but they should
         * have different property. image layout change to a dynamic size, big
         * video snap is still wrap_content we change ipmessage image layout to
         * keep uniform with group chat activity.
         */
        ViewGroup.LayoutParams params = (ViewGroup.LayoutParams) mImageContent.getLayoutParams();
        params.height = ViewGroup.LayoutParams.WRAP_CONTENT;
        params.width = ViewGroup.LayoutParams.WRAP_CONTENT;
        mImageContent.setLayoutParams(params);

        if (null != bp) {
            if (degree != 0) {
                bp = IpMessageUtils.rotate(bp, degree);
            }
            mImageContent.setImageBitmap(bp);
        } else {
            mImageContent.setImageResource(R.drawable.ic_missing_thumbnail_picture);
        }
    }

    @Override
    public void notificationsReceived(Intent intent) {
        Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                "listItem.notificationsReceived(): intent = " + intent);
        String action = intent.getAction();
        if (TextUtils.isEmpty(action)) {
            return;
        }
        long msgId = 0L;
        switch (IpMessageUtils.getActionTypeByAction(action)) {
            case IpMessageUtils.IPMSG_DOWNLOAD_ATTACH_STATUS_ACTION:
                Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "listItem.notificationsReceived():"
                        + " download status notification.");
                if (null != mRcseMessageItem) {
                    try {
                        int downloadStatus = intent.getIntExtra(
                                DownloadAttachStatus.DOWNLOAD_MSG_STATUS,
                                DownloadAttachStatus.STARTING);
                        Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                                "notificationsReceived(): downloadStatus = " + downloadStatus);
                        if (downloadStatus == DownloadAttachStatus.DONE) {
                            Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                                    "notificationsReceived(): call UI thread notify data set change.");
                            final Message msg = Message.obtain(mHandler, MSG_LIST_NEED_REFRASH,
                                    MESSAGE_LIST_REFRASH_WITH_CLEAR_CACHE, 0);
                            msg.sendToTarget();
                            return;
                        }
                        msgId = intent.getLongExtra(DownloadAttachStatus.DOWNLOAD_MSG_ID, 0);
                        mIpMessageItemCallback.updateMessageItemState(msgId);
                    } catch (NullPointerException e) {
                        Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                                "NullPointerException:" + e.toString());
                    }
                }
                break;
            case IpMessageUtils.IPMSG_IP_MESSAGE_STATUS_ACTION:
                Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                        "listItem.notificationsReceived(): download status notification.");
                if (null != mRcseMessageItem) {
                    try {
                        msgId = intent.getLongExtra(IpMessageStatus.IP_MESSAGE_ID, 0);
                        mIpMessageItemCallback.updateMessageItemState(msgId);
                    } catch (NullPointerException e) {
                        Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                                "NullPointerException:" + e.toString());
                    }
                }
                break;
            default:
                Logger.v(IpMessageUtils.IPMSG_NOTIFICATION_TAG,
                        "listItem.notificationsReceived(): ignore notification.");
                return;
        }
    }

    private void openMedia(IpMessage ipmessage) {
        Logger.v(TAG, "openMedia(): msgId = " + mMsgId);
        if (ipmessage.getType() == IpMessageType.VCARD) {
            IpVCardMessage msg = (IpVCardMessage) ipmessage;
            if (TextUtils.isEmpty(msg.getPath())) {
                Logger.e(TAG, "openMedia(): open vCard failed.");
                return;
            }
            if (!IpMessageUtils.getSDCardStatus()) {
                IpMessageUtils.createLoseSDCardNotice(
                        mContext,
                        IpMessageResourceMananger.getInstance(mContext).getSingleString(
                                IpMessageConsts.string.ipmsg_cant_share));
                return;
            }
            if (IpMessageUtils.getAvailableBytesInFileSystemAtGivenRoot(StorageManagerEx
                    .getDefaultPath()) < msg.getSize()) {
                Toast.makeText(mContext, mContext.getString(R.string.export_disk_problem),
                        Toast.LENGTH_LONG).show();
            }
            String dest = IpMessageUtils.getCachePath(mContext) + "temp"
                    + msg.getPath().substring(msg.getPath().lastIndexOf(".vcf"));
            IpMessageUtils.copy(msg.getPath(), dest);
            File vcardFile = new File(dest);
            //Uri vcardUri = PluginUtils.getUriFromPath(dest);
            Uri vcardUri = FileProvider.getUriForFile(
                mContext, "com.android.mms.files", vcardFile);
            Logger.d(TAG, "openMedia(): vcardUri = " + vcardUri);
            Intent i = new Intent();
            i.setAction(android.content.Intent.ACTION_VIEW);
            i.setDataAndType(vcardUri, "text/x-vcard");
            i.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION |
                Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            try {
                mContext.startActivity(i);
            } catch(Exception e) {
                e.printStackTrace();
            }
        } else if (ipmessage.getType() == IpMessageType.CALENDAR) {
            IpVCalendarMessage msg = (IpVCalendarMessage) ipmessage;
            if (TextUtils.isEmpty(msg.getPath())) {
                Logger.e(TAG, "openMedia(): open vCalendar failed.");
                return;
            }
            if (!IpMessageUtils.getSDCardStatus()) {
                IpMessageUtils.createLoseSDCardNotice(
                        mContext,
                        IpMessageResourceMananger.getInstance(mContext).getSingleString(
                                IpMessageConsts.string.ipmsg_cant_share));
                return;
            }
            if (IpMessageUtils.getAvailableBytesInFileSystemAtGivenRoot(StorageManagerEx
                    .getDefaultPath()) < msg.getSize()) {
                Toast.makeText(mContext, mContext.getString(R.string.export_disk_problem),
                        Toast.LENGTH_LONG).show();
            }
            String dest = IpMessageUtils.getCachePath(mContext) + "temp"
                    + msg.getPath().substring(msg.getPath().lastIndexOf(".vcs"));
            IpMessageUtils.copy(msg.getPath(), dest);
            File calendarFile = new File(dest);
            Uri calendarUri = PluginUtils.getUriFromPath(dest);
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
            intent.putExtra(RemoteActivities.KEY_MESSAGE_ID, mMsgId);
            IpMessageUtils.startRemoteActivity(mContext, intent);
        }
    }

    private boolean isFileTransferStatus(int ipMsgStatus) {
        switch (ipMsgStatus) {
            case IpMessageStatus.MO_INVITE:
            case IpMessageStatus.MO_TIMEOUT:
            case IpMessageStatus.MO_SENDING:
            case IpMessageStatus.MO_REJECTED:
            case IpMessageStatus.MO_SENT:
            case IpMessageStatus.MO_DELIVERED:
            case IpMessageStatus.MO_VIEWED:
            case IpMessageStatus.MO_CANCEL:
            case IpMessageStatus.MT_INVITED:
            case IpMessageStatus.MT_REJECT:
            case IpMessageStatus.MT_RECEIVING:
            case IpMessageStatus.MT_RECEIVED:
            case IpMessageStatus.MT_CANCEL:
            case IpMessageStatus.MO_PAUSE:
            case IpMessageStatus.MO_RESUME:
            case IpMessageStatus.MT_PAUSE:
            case IpMessageStatus.MT_RESUME:
            case IpMessageStatus.MT_NOT_RECEIVED:
                return true;
            default:
                return false;
        }
    }

    private boolean isIpMessageShowContent(int ipMsgStatus) {
        switch (ipMsgStatus) {
            case IpMessageStatus.MT_RESUME:
                isFileResumed = true;
                return true;
            case IpMessageStatus.MO_INVITE:
            case IpMessageStatus.MO_SENDING:
            case IpMessageStatus.MO_REJECTED:
            case IpMessageStatus.MO_SENT:
            case IpMessageStatus.MT_RECEIVED:
            case IpMessageStatus.MO_PAUSE:
            case IpMessageStatus.MO_DELIVERED:
            case IpMessageStatus.MO_VIEWED:
            case IpMessageStatus.MO_RESUME:
                return true;
            case IpMessageStatus.MO_CANCEL:
            case IpMessageStatus.MT_INVITED:
            case IpMessageStatus.MT_REJECT:
            case IpMessageStatus.MT_RECEIVING:
            case IpMessageStatus.MT_CANCEL:
            case IpMessageStatus.MT_PAUSE:
                return true;
            default:
                return true;
        }
    }

    private void drawDownloadFileView(final IpMessage ipMessage, final int ipMsgStatus,
            int ipMsgType) {
        Logger.v(TAG, "drawDownloadFileView(): mMsgId = " + mMsgId
                + ", ipMsgStatus = " + ipMsgStatus + ", ipMsgType = " + ipMsgType);
        if (mIpmsgFileDownloadContrller == null || mIpmsgFileDownloadView == null) {
            Logger.v(TAG, "drawDownloadFileView(): mIpmsgFileDownloadContrller is NULL!");
            return;
        }
        // / M: show IP message status string
        String statusText = IpMessageManager.getInstance(mContext).getIpMessageStatusString(
mMsgId);
        if (!TextUtils.isEmpty(statusText)) {
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setText(statusText);
        } else {
            mCaption.setVisibility(View.GONE);
        }

        IpAttachMessage ipAttachMessage = (IpAttachMessage) ipMessage;
        showIpMessageThumb(ipMsgType, ipMsgStatus, ipAttachMessage);
        switch (ipMsgStatus) {
            case IpMessageStatus.MT_RECEIVING:
                // mCaptionSeparator.setVisibility(View.GONE);
                if (mIpmsgResendButton != null) {
                    mIpmsgResendButton.setVisibility(View.GONE);
                }
            case IpMessageStatus.MO_INVITE:
            case IpMessageStatus.MO_SENDING:
            case IpMessageStatus.MO_RESUME:
            case IpMessageStatus.MO_PAUSE:
            case IpMessageStatus.MT_RESUME:
            case IpMessageStatus.MT_PAUSE:
                int progress = IpMessageManager.getInstance(mContext).getDownloadProcess(mMsgId);
                mIpmsgDownloadFileProgress.setProgress(progress);
                mIpmsgFileSize.setText((progress * ipAttachMessage.getSize() / 100) + "/"
                        + ipAttachMessage.getSize() + "K");
                mIpmsgFileDownloadContrller.setVisibility(View.GONE);
                mIpmsgFileDownloadView.setVisibility(View.VISIBLE);
                if (mIpmsgCancelDownloadButton != null) {
                    Drawable imageCancel = IpMessageResourceMananger.getInstance(mContext)
                            .getSingleDrawable(IpMessageConsts.drawable.ipmsg_file_transfer_cancel);
                    mIpmsgCancelDownloadButton.setImageDrawable(imageCancel);
                    mIpmsgCancelDownloadButton.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            Logger.v(TAG, "drawDownloadFileView(): cancel button OnClick.");
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MO_CANCEL);
                        }
                    });
                }

                if (mIpmsgPauseResumeButton != null) {
                    Logger.v(TAG, "mIpmsgPauseResumeButton = " + mIpmsgPauseResumeButton
                            + ", ipMsgStatus = " + ipMsgStatus);
                    if (ipMsgStatus == IpMessageStatus.MO_INVITE) {
                        mIpmsgPauseResumeButton.setVisibility(View.GONE);
                    } else {
                        mIpmsgPauseResumeButton.setVisibility(View.GONE);
                        if (ipMsgStatus != IpMessageStatus.MO_PAUSE
                                && ipMsgStatus != IpMessageStatus.MT_PAUSE) {
                            Drawable imagePause = IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleDrawable(
                                            IpMessageConsts.drawable.ipmsg_file_transfer_pause);
                            Logger.v(TAG, "imagePause = " + imagePause);
                            mIpmsgPauseResumeButton.setBackgroundDrawable(imagePause);
                        } else {
                            Drawable imageResume = IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleDrawable(
                                            IpMessageConsts.drawable.ipmsg_file_transfer_resume);
                            Logger.v(TAG, "imageResume = " + imageResume);
                            mIpmsgPauseResumeButton.setBackgroundDrawable(imageResume);
                        }
                    }

                    mIpmsgPauseResumeButton.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            Logger.v(TAG, "drawDownloadFileView(): pause button OnClick.");
                            if (ipMsgStatus == IpMessageStatus.MO_PAUSE
                                    || ipMsgStatus == IpMessageStatus.MT_PAUSE) {
                                IpMessageManager.getInstance(mContext).setIpMessageStatus(
mMsgId,
                                        IpMessageStatus.MO_RESUME);
                            } else {
                                IpMessageManager.getInstance(mContext).setIpMessageStatus(
mMsgId,
                                        IpMessageStatus.MO_PAUSE);
                            }
                        }
                    });
                }

                break;
            case IpMessageStatus.MO_REJECTED:
                mIpmsgFileDownloadContrller.setVisibility(View.VISIBLE);
                mIpmsgFileDownloadView.setVisibility(View.GONE);
                if (mIpmsgResendButton != null) {
                    Logger.v(TAG,
                            "drawDownloadFileView(): Set resend button OnClickListener.");
                    mIpmsgResendButton.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            Logger.v(TAG, "drawDownloadFileView(): Resend button OnClick.");
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MO_INVITE);
                        }
                    });
                }
                // mCaptionSeparator.setVisibility(View.VISIBLE);
                break;
            case IpMessageStatus.MT_NOT_RECEIVED:
                mIpmsgFileDownloadContrller.setVisibility(View.GONE);
                mIpmsgFileDownloadView.setVisibility(View.GONE);
                if (mIpmsgResendButton != null) {
                    Logger.v(TAG,
                            "drawDownloadFileView(): Set resume receiver button OnClickListener.");
                    mIpmsgResendButton.setVisibility(View.VISIBLE);
//                    mIpmsgResendButton.setText(mContext.getString(R.string.file_type_resume));
                    mIpmsgResendButton.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            Logger.v(TAG, "drawDownloadFileView(): Resume receiver button OnClick.");
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MT_RESUME);
                        }
                    });
                }
                // mCaptionSeparator.setVisibility(View.VISIBLE);
                break;
            case IpMessageStatus.MT_INVITED:
                mIpmsgFileDownloadContrller.setVisibility(View.VISIBLE);
                mIpmsgFileDownloadView.setVisibility(View.GONE);
                // mCaptionSeparator.setVisibility(View.GONE);
                Logger.v(TAG,
                        "drawDownloadFileView(): Set accept and reject button OnClickListener.");
                if (mIpmsgAcceptButton != null) {
                    mIpmsgAcceptButton.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            Logger.v(TAG, "drawDownloadFileView(): Accept button OnClick.");
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MT_RECEIVING);
                        }
                    });
                }
                if (mIpmsgRejectButton != null) {
                    mIpmsgRejectButton.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            Logger.v(TAG, "drawDownloadFileView(): Reject button OnClick.");
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MT_REJECT);
                        }
                    });
                }
                break;
            case IpMessageStatus.MT_REJECT:
            case IpMessageStatus.MT_CANCEL:
                // mCaptionSeparator.setVisibility(View.GONE);
            case IpMessageStatus.MO_CANCEL:
                /*mIpmsgFileDownloadContrller.setVisibility(View.GONE);
                mIpmsgFileDownloadView.setVisibility(View.GONE);

                mBodyTextView.setVisibility(View.GONE);
                mIpImageView.setVisibility(View.GONE);
                // / M: add for ip message, hide audio, vCard, vCalendar
                mIpAudioView.setVisibility(View.GONE);
                // mCaptionSeparator.setVisibility(View.GONE);
                mIpVCardView.setVisibility(View.GONE);
                mIpVCalendarView.setVisibility(View.GONE);*/
                if (mIpmsgResendButton != null) {
                    Logger.v(TAG,
                            "drawDownloadFileView(): Set resend button OnClickListener.");
                    mIpmsgResendButton.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            Logger.v(TAG, "drawDownloadFileView(): Resend button OnClick.");
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MO_INVITE);
                        }
                    });
                }
                break;
            case IpMessageStatus.MO_SENT:
            case IpMessageStatus.MO_DELIVERED:
            case IpMessageStatus.MO_VIEWED:
            case IpMessageStatus.MT_RECEIVED:
                mIpmsgFileDownloadContrller.setVisibility(View.GONE);
                mIpmsgFileDownloadView.setVisibility(View.GONE);
                break;
            case IpMessageStatus.MO_TIMEOUT:
                if(isFileResumed && !FIRST_RETRY_FINISHED) {
                    Logger.v(TAG, "drawDownloadFileView(): Add to Auto Retry first");
                    mResumeTimer.schedule(new TimerTask() {          
                        @Override
                        public void run() {
                            FIRST_RETRY_FINISHED = true;
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MT_RESUME);
                            mResumeTimer.cancel();
                        }
                    }, FIRST_RETRY_TIME);                    
                } else if(isFileResumed && !SECOND_RETRY_FINISHED) {
                    Logger.v(TAG, "drawDownloadFileView(): Add to Auto Retry second");
                    mResumeTimer.schedule(new TimerTask() {          
                        @Override
                        public void run() {
                            SECOND_RETRY_FINISHED = true;
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MT_RESUME);
                            mResumeTimer.cancel();
                        }
                    }, SECOND_RETRY_TIME - FIRST_RETRY_TIME);                    
                } else if(isFileResumed && !THIRD_RETRY_FINISHED) {
                    Logger.v(TAG, "drawDownloadFileView(): Add to Auto Retry third");
                    mResumeTimer.schedule(new TimerTask() {          
                        @Override
                        public void run() {
                            THIRD_RETRY_FINISHED = true;
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MT_RESUME);
                            mResumeTimer.cancel();
                        }
                    }, THIRD_RETRY_TIME - SECOND_RETRY_TIME);                    
                } else if(isFileResumed && !FOURH_RETRY_FINISHED) {
                    Logger.v(TAG, "drawDownloadFileView(): Add to Auto Retry fourth");
                    mResumeTimer.schedule(new TimerTask() {          
                        @Override
                        public void run() {
                            FOURH_RETRY_FINISHED = true;
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MT_RESUME);
                            mResumeTimer.cancel();
                        }
                    }, FOURTH_RETRY_TIME - THIRD_RETRY_TIME);                    
                } else if(isFileResumed && !FIFTH_RETRY_FINISHED) {
                    Logger.v(TAG, "drawDownloadFileView(): Add to Auto Retry fifth");
                    mResumeTimer.schedule(new TimerTask() {          
                        @Override
                        public void run() {
                            FIFTH_RETRY_FINISHED = true;
                            IpMessageManager.getInstance(mContext).setIpMessageStatus(mMsgId,
                                    IpMessageStatus.MT_RESUME);
                            mResumeTimer.cancel();
                        }
                    }, FIFTH_RETRY_TIME - FOURTH_RETRY_TIME);                    
                }
                break;
            default:
                break;
        }
    }

    private void showIpMessageThumb(int ipMsgType, int ipMsgStatus, IpAttachMessage ipAttachMessage) {
        Drawable thumb = null;
        Logger.v(TAG, "showIpMessageThumb(): entry");
        if (ipMsgStatus >= IpMessageStatus.MO_INVITE && ipMsgStatus <= IpMessageStatus.MO_CANCEL
                && ipMsgType != IpMessageType.UNKNOWN_FILE) {
            Logger.v(TAG, "showIpMessageThumb(): UNKNOWN_FILE");
            //return;
        }
        if (ipMsgType == IpMessageType.PICTURE) {
            String path = ((IpImageMessage) ipAttachMessage).getThumbPath();
            if (path != null) {
                thumb = Drawable.createFromPath(path);
            }
        } else {
            thumb = ipAttachMessage.getFileTypeIcon();
        }
        if (thumb != null) {
            mIpAudioView.setVisibility(View.GONE);
            mIpVCalendarView.setVisibility(View.GONE);
            mIpVCardView.setVisibility(View.GONE);
            mIpImageView.setVisibility(View.VISIBLE);
            mImageContent.setVisibility(View.VISIBLE);
            mIpImageSizeBg.setVisibility(View.GONE);
            mImageContent.setImageDrawable(thumb);
        }
    }


}
