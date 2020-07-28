package com.mediatek.rcse.plugin.message;

import java.util.ArrayList;
import java.util.List;

import android.R.mipmap;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources.NotFoundException;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.ThumbnailUtils;
import android.net.Uri;
import android.provider.MediaStore.Video.Thumbnails;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.mms.ipmessage.DefaultIpDialogModeActivityExt;
import com.mediatek.mms.callback.IDialogModeActivityCallback;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IpMessageConsts.*;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcs.R;

public class RcseDialogModeActivity extends DefaultIpDialogModeActivityExt
implements INotificationsListener {
    private static String TAG = "RcseDialogModeActivity";
    
    /// M: add for ipmessage
    private TextView mGroupSender;
    private final ArrayList<Uri> mIpMessageUris;
    private List<Uri> mUris;
    private TextView mBodyTextView;
    private View mMmsView;
    private View mIpView;
    
    /// M: add for image and video
    private View mIpImageView; // ip_image
    private ImageView mImageContent; // image_content
    private View mIpImageSizeBg; // image_size_bg
    private ImageView mActionButton; // action_btn
    private TextView mContentSize; // content_size
    private ProgressBar mImageDownloadProgressBar; // image_downLoad_progress
    private View mCaptionSeparator; // caption_separator
    private TextView mCaption; // text_caption
    private ImageView mMediaPlayView;
    /// M: add for audio
    private View mIpAudioView; // ip_audio
    private ImageView mAudioIcon; // ip_audio_icon
    private TextView mAudioInfo; // audio_info
    private ProgressBar mAudioDownloadProgressBar; // audio_downLoad_progress
    
    /// M: add for vCard
    private View mIpVCardView;
    private ImageView mVCardIcon;
    private TextView mVCardInfo;
    /// M: add for vcalendar
    private View mIpVCalendarView;
    private TextView mVCalendarInfo;
    
    private Activity mContext;
    private Context mRcseContext;
    private Cursor mCursor;
    private IDialogModeActivityCallback mCallback;
    
    private static final int SMS_ADDR = 2;

    public RcseDialogModeActivity(Context context) {
        mIpMessageUris = new ArrayList<Uri>();
        mRcseContext = context;
    }

    @Override
    public boolean onIpInitDialogView(Activity context, List<Uri> uris, View mmsView,
            Cursor cursor, IDialogModeActivityCallback callback, EditText replyEditor,
            TextView smsContentText, View ipView, TextView groupSender , ImageView contactImage) {
        mContext = context;
        mUris = uris;
        mMmsView = mmsView;
        mCursor = cursor;
        mCallback = callback;

        // / M: add for ipmessage
        /// M: if ipmessage is enabled. show another hint
        if (IpMmsConfig.isServiceEnabled(mContext)) {
            LayoutInflater pluginInflater = LayoutInflater.from(mRcseContext);             
           /* mIpView = IpMessageResourceMananger.getInstance(mContext).inflateView(
                    R.layout.ip_dlg_view, (ViewGroup) ipView, true);*/
            mIpView =  pluginInflater.inflate(R.layout.ip_dlg_view, null,false);           
            ((ViewGroup)ipView).addView(mIpView);
            ipView.setVisibility(View.VISIBLE);
            mIpView.setVisibility(View.VISIBLE);
            Logger.d(TAG, "onIpInitDialogView: mIpView" + mIpView + "ipview:" + ipView);
            replyEditor.setHint(IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(IpMessageConsts.string.ipmsg_type_to_compose_text));
            /// M: add for ipmessage, notification listener
            IpMessageUtils.addIpMsgNotificationListeners(mContext, this);
            
            /// M: add for image and video
            mIpImageView = (View) mIpView.findViewById(R.id.dlg_ip_image);
            mImageContent = (ImageView) mIpView.findViewById(R.id.dlg_image_content);
            mIpImageSizeBg = (View) mIpView.findViewById(R.id.dlg_image_size_bg);
            mActionButton = (ImageView) mIpView.findViewById(R.id.dlg_action_btn);
            mContentSize = (TextView) mIpView.findViewById(R.id.dlg_content_size);
            mImageDownloadProgressBar = (ProgressBar) mIpView
                    .findViewById(R.id.dlg_image_downLoad_progress);
            mCaptionSeparator = (View) mIpView.findViewById(R.id.dlg_caption_separator);
            mCaption = (TextView) mIpView.findViewById(R.id.dlg_text_caption);
            mMediaPlayView = (ImageView) mIpView.findViewById(R.id.dlg_video_media_play);
            /// M: add for audio
            mIpAudioView = (View) mIpView.findViewById(R.id.dlg_ip_audio);
            mAudioIcon = (ImageView) mIpView.findViewById(R.id.dlg_ip_audio_icon);
            mAudioInfo = (TextView) mIpView.findViewById(R.id.dlg_audio_info);
            mAudioDownloadProgressBar = (ProgressBar) mIpView
                    .findViewById(R.id.dlg_audio_downLoad_progress);
            
            /// M: add for vCard
            mIpVCardView = (View) mIpView.findViewById(R.id.dlg_ip_vcard);
            mVCardIcon = (ImageView) mIpView.findViewById(R.id.dlg_ip_vcard_icon);
            mVCardInfo = (TextView) mIpView.findViewById(R.id.dlg_vcard_info);
            /// M: add for vCalendar
            mIpVCalendarView = (View) mIpView.findViewById(R.id.dlg_ip_vcalendar);
            mVCalendarInfo = (TextView) mIpView.findViewById(R.id.dlg_vcalendar_info);
        }

        mGroupSender = groupSender;
        mBodyTextView = smsContentText;
        return true;
    }
    
    @Override
    public boolean onIpAddNewUri(Intent intent, Uri newUri) {
        if (intent.getBooleanExtra("ipmessage", false)) {
            Logger.d(TAG, "receiver a ipmessage,uri:" + newUri.toString());
            mIpMessageUris.add(newUri);
        }
        return true;
    }

    @Override
    public boolean onIpDestroy() {
        IpMessageUtils.removeIpMsgNotificationListeners(mContext, this);
        return true;
    }

   public String onIpSetDialogView() {
       Logger.d(TAG, "onIpSetDialogView entry:");
        // Add for joyn message
        String simStatus = IpMessageResourceMananger.getInstance(mContext).getSimStatus();
        /*if (simStatus != null) {
            simStatus = "   " + simStatus + "  ";
            (new SpannableStringBuilder(simCharSequence)).replace(0, simCharSequence.length(),
                    simStatus);
        }*/

        // add for ipmessage
        if (isCurIpMessage()) {
            showIpMessage();
        } else {
            Logger.d(TAG, "onIpSetDialogView else entry:");
            // hide ipmessage views and show sms view.
            mGroupSender.setVisibility(View.GONE);
            if (mIpImageView != null) {
                mIpImageView.setVisibility(View.GONE);                
            }
            if (mCaptionSeparator != null) {
                mCaptionSeparator.setVisibility(View.GONE);                
            }
            if (mCaption != null) {
                mCaption.setVisibility(View.GONE);       
            }
            if (mIpAudioView != null) {
                mIpAudioView.setVisibility(View.GONE);    
            }
            /// M: add for ipmessage, hide audio or vcard view
            if (mIpVCardView != null) {
                mIpVCardView.setVisibility(View.GONE);      
            }
            if (mIpVCalendarView != null) {
                mIpVCalendarView.setVisibility(View.GONE);
            }
            if (mBodyTextView != null) {
                mBodyTextView.setVisibility(View.VISIBLE);
            }
        }
        return simStatus;
    }

    @Override
    public String onIpGetSenderString() {
        if (isCurGroupIpMessage()) {
            return getCurGroupIpMessageName();
        }
        // Add for joyn
        if (isCurJoynConvergedIpMessage()) {
            return getCurJoynIpMessageName();
        }
        return null;
    }
    
    @Override
    public String onIpGetSenderNumber() {
        if (isCurGroupIpMessage()) {
            return getCurGroupIpMessageNumber();
        }
        return null;
    }
    
    @Override
    public boolean onIpClick(long threadId) {
        if (isCurGroupIpMessage()) {
            /// M: open group chat
            openIpMsgThread(threadId);
            return true;
        } else {
            return false;
        }
    }
    
    @Override
    public boolean onIpSendReplySms(String body, String to) {
        if (isCurGroupIpMessage()) {
            Logger.d(TAG, "onIpSendReplySms, to:" + to + ",body:" + body);
            sendIpTextMessage(body, to);
            Uri curUri = (Uri) mUris.get(mCallback.getCurUriIdx());
            mCallback.markIpAsRead(curUri);
            return true;
        } else {
            return false;
        }
    }
    
    @Override
    public boolean onIpSendMessage(String body, String to) {
        /// M: add for ipmessage
        if (isCurIpMessage() && IpMmsConfig.isServiceEnabled(mContext)) {
            Logger.d(TAG, "onIpSendMessage, to:" + to + ",body:" + body);
            sendIpTextMessage(body, to);
            Uri curUri = (Uri) mUris.get(mCallback.getCurUriIdx());
            mCallback.markIpAsRead(curUri);
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpUpdateSendButtonState(ImageButton sendButton) {
        if (isCurIpMessage() && sendButton.isEnabled()) {
            Logger.d(TAG, "onIpUpdateSendButtonState entry, " );
            sendButton.setImageDrawable(IpMessageResourceMananger.getInstance(
                                    mContext).getSingleDrawable(
                                    IpMessageConsts.drawable.enter_joyn_chat));
            return true;
        }
        return false;
    }

    // / M: add for ipmessage
    private boolean isCurIpMessage() {
        boolean result = false;
        Uri curUri;
        if(mUris == null)
            return false;
        if (mUris.size() <= 0) {
            mCallback.setCurUriIdx(0, null);
            return false;
        }
        curUri = (Uri) mUris.get(mCallback.getCurUriIdx());
        if (curUri != null) {
            result = mIpMessageUris.contains(curUri);
            Logger.d(TAG, "check uri:" + curUri.toString());
        }
        Logger.d(TAG, "result:" + result);
        return result;
    }

    private long getCurIpMessageId() {
        long id = 0;
        Uri curUri;
        if (mUris.size() <= 0) {
            mCallback.setCurUriIdx(0, null);
            return id;
        }
        curUri = (Uri) mUris.get(mCallback.getCurUriIdx());
        if (curUri != null) {
            id = Long.parseLong(curUri.getLastPathSegment());
        } else {
            Logger.w(TAG, "mCurUri is null!");
        }
        Logger.d(TAG, "id:" + id);
        return id;
    }

    private void showIpMessage() {
        Logger.d(TAG, "showIpMessage entry:");
        if (mMmsView != null) {
            Logger.d(TAG, "Hide MMS views");
            mMmsView.setVisibility(View.GONE);
        }
        if (isCurGroupIpMessage() && mCursor != null && !mCursor.isClosed()) {
            String name = mCursor.getString(SMS_ADDR);
            Logger.d(TAG, "group sender address:" + name);
            name = IpMessageContactManager.getInstance(mContext).getNameByNumber(name) + ":";
            Logger.d(TAG, "group sender name:" + name);
            mGroupSender.setText(name);
            mGroupSender.setVisibility(View.VISIBLE);
        } else {
            mGroupSender.setVisibility(View.GONE);
        }
        long id = getCurIpMessageId();
        IpMessage ipMessage = IpMessageManager.getInstance(mContext).getIpMsgInfo(id);
        Logger.d(TAG, "showIpMessage. id:" + id + ",type:" + ipMessage.getType());
        switch (ipMessage.getType()) {
            case IpMessageType.TEXT:
                setIpTextItem((IpTextMessage) ipMessage);
                break;
            case IpMessageType.PICTURE:
                setIpImageItem((IpImageMessage) ipMessage);
                break;
            case IpMessageType.VOICE:
                setIpVoiceItem((IpVoiceMessage) ipMessage);
                break;
            case IpMessageType.VCARD:
                setIpVCardItem((IpVCardMessage) ipMessage);
                break;
            case IpMessageType.VIDEO:
                setIpVideoItem((IpVideoMessage) ipMessage);
                break;
            case IpMessageType.CALENDAR:
                setIpVCalendarItem((IpVCalendarMessage) ipMessage);
                break;
            case IpMessageType.UNKNOWN_FILE:
            case IpMessageType.COUNT:
                Logger.w(TAG, "Unknown IP message type. type = " + ipMessage.getType());
                break;
            case IpMessageType.GROUP_CREATE_CFG:
            case IpMessageType.GROUP_ADD_CFG:
            case IpMessageType.GROUP_QUIT_CFG:
                // / M: group chat type
                Logger.w(TAG, "Group IP message type. type = " + ipMessage.getType());
                break;
            default:
                Logger.w(TAG, "Error IP message type. type = " + ipMessage.getType());
                break;
        }
    }

    private void setIpTextItem(IpTextMessage textMessage) {
        Logger.d(TAG, "setIpTextItem()");
        if (TextUtils.isEmpty(textMessage.getBody())) {
            Logger.w(TAG, "setIpTextItem(): No message content!");
            return;
        }

        mIpImageView.setVisibility(View.GONE);
        mCaptionSeparator.setVisibility(View.GONE);
        mCaption.setVisibility(View.GONE);
        // / M: add for ipmessage, hide audio or vcard view
        mIpAudioView.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
        CharSequence formattedMessage = formatMessage(textMessage.getBody());
        mIpImageView.setVisibility(View.GONE);
        mBodyTextView.setText(formattedMessage);
        mBodyTextView.setVisibility(View.VISIBLE);
    }

    private CharSequence formatMessage(String body) {
        SpannableStringBuilder buf = new SpannableStringBuilder();
        buf.append(body);
        return buf;
    }

    private void updateIpMessageImageOrVideoView(IpAttachMessage message, long msgId) {
        mActionButton.setVisibility(View.VISIBLE);
        mActionButton.setImageResource(R.drawable.ipmsg_chat_download_selector);

        if (null != mImageDownloadProgressBar) {
            mImageDownloadProgressBar.setVisibility(View.GONE);
            mActionButton.setVisibility(View.GONE);
        }
        mContentSize.setVisibility(View.VISIBLE);
        mContentSize.setText(IpMessageUtils.formatFileSize(message.getSize()));
    }

    private void setIpImageItem(IpImageMessage imageMessage) {
        Logger.d(TAG, "setIpImageItem()");
        mIpImageView.setVisibility(View.VISIBLE);
        mMediaPlayView.setVisibility(View.INVISIBLE);
        final long msgId = getCurIpMessageId();

        if (!setPicView(imageMessage.getThumbPath())) {
            setPicView(imageMessage.getPath());
        }
        mIpImageSizeBg.setBackgroundDrawable(null);
        mActionButton.setVisibility(View.GONE);
        if (null != mImageDownloadProgressBar) {
            mImageDownloadProgressBar.setVisibility(View.GONE);
        }
        mActionButton.setClickable(false);
        mContentSize.setVisibility(View.GONE);

        if (TextUtils.isEmpty(imageMessage.getCaption())) {
            mCaptionSeparator.setVisibility(View.GONE);
            mCaption.setVisibility(View.GONE);
        } else {
            mCaptionSeparator.setVisibility(View.VISIBLE);
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setText(imageMessage.getCaption());
        }

        // / M: add for ipmessage, hide text view
        mBodyTextView.setVisibility(View.GONE);
        // / M: add for ipmessage, hide audio or vcard view
        mIpAudioView.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
    }

    private void setIpVoiceItem(IpVoiceMessage voiceMessage) {
        long msgId = getCurIpMessageId();
        Logger.d(TAG, "setIpVoiceItem(): message Id = " + msgId);

        if (null != mAudioDownloadProgressBar) {
            mAudioDownloadProgressBar.setVisibility(View.GONE);
        }
        mAudioInfo.setVisibility(View.VISIBLE);
        mAudioInfo.setText(IpMessageUtils.formatAudioTime(voiceMessage.getDuration()));

        if (TextUtils.isEmpty(voiceMessage.getCaption())) {
            mCaptionSeparator.setVisibility(View.GONE);
            mCaption.setVisibility(View.GONE);
        } else {
            mCaptionSeparator.setVisibility(View.VISIBLE);
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setText(voiceMessage.getCaption());
        }

        // / M: add for ipmessage, show audio view
        mIpAudioView.setVisibility(View.VISIBLE);
        mIpVCardView.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
        // / M: add for ipmessage, hide text view
        mBodyTextView.setVisibility(View.GONE);
        // / M: add for ipmessage, hide image view or video view
        mIpImageView.setVisibility(View.GONE);
    }

    private void setIpVCardItem(IpVCardMessage vCardMessage) {
        long msgId = getCurIpMessageId();
        Logger.d(TAG, "setIpVCardItem(): message Id = " + msgId);
        String name = vCardMessage.getName();
        /*if (name != null && name.lastIndexOf(".") != -1) {
            name = name.substring(0, name.lastIndexOf("."));
        }*/
        name = extractFileNameFromPath(name);
        mVCardInfo.setText(name);
        mVCardIcon.setVisibility(View.VISIBLE);
        mVCardInfo.setVisibility(View.VISIBLE);
        mIpVCardView.setVisibility(View.VISIBLE);
        mIpAudioView.setVisibility(View.GONE);
        // / M: add for ipmessage, hide text view
        mBodyTextView.setVisibility(View.GONE);
        // / M: add for ipmessage, hide image view or video view
        mIpImageView.setVisibility(View.GONE);
        mCaptionSeparator.setVisibility(View.GONE);
        mCaption.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
    }

    /**
     * @param filePath Path of file to be transferred .
     * @return return file path .
     */
    public static String extractFileNameFromPath(String filePath) {
        if (null != filePath) {
            int lastDashIndex = filePath.lastIndexOf("/");
            if (-1 != lastDashIndex
                    && lastDashIndex < filePath.length() - 1) {
                String fileName = filePath.substring(lastDashIndex + 1);
                return fileName;
            } else {
                Logger.e(TAG,
                        "extractFileNameFromPath() invalid file path:"
                                + filePath);
                return filePath;
            }
        } else {
            Logger.e(TAG, "extractFileNameFromPath() filePath is null");
            return null;
        }
    }

    private void setIpVCalendarItem(IpVCalendarMessage vCalendarMessage) {
        long msgId = getCurIpMessageId();
        Logger.d(TAG, "setIpVCalendarItem(): message Id = " + msgId);
        String summary = vCalendarMessage.getSummary();
        if (summary != null && summary.lastIndexOf(".") != -1) {
            summary = summary.substring(0, summary.lastIndexOf("."));
        }
        mVCalendarInfo.setText(summary);
        mIpVCalendarView.setVisibility(View.VISIBLE);
        mBodyTextView.setVisibility(View.GONE);
        mIpImageView.setVisibility(View.GONE);
        mIpAudioView.setVisibility(View.GONE);
        mCaptionSeparator.setVisibility(View.GONE);
        mCaption.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
    }

    private void setIpVideoItem(IpVideoMessage videoMessage) {
        final long msgId = getCurIpMessageId();
        Logger.d(TAG, "setIpVideoItem(): message Id = " + msgId);
        mIpImageView.setVisibility(View.VISIBLE);
        mMediaPlayView.setVisibility(View.VISIBLE);

        if (!setPicView(videoMessage.getThumbPath())) {
            setVideoView(videoMessage.getPath());
        }
        mIpImageSizeBg.setBackgroundDrawable(null);
        mActionButton.setVisibility(View.GONE);
        if (null != mImageDownloadProgressBar) {
            mImageDownloadProgressBar.setVisibility(View.GONE);
        }
        mActionButton.setClickable(false);
        mContentSize.setVisibility(View.GONE);

        if (TextUtils.isEmpty(videoMessage.getCaption())) {
            mCaptionSeparator.setVisibility(View.GONE);
            mCaption.setVisibility(View.GONE);
        } else {
            mCaptionSeparator.setVisibility(View.VISIBLE);
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setText(videoMessage.getCaption());
        }

        // / M: add for ipmessage, hide text view
        mBodyTextView.setVisibility(View.GONE);
        // / M: add for ipmessage, hide audio or vcard view
        mIpAudioView.setVisibility(View.GONE);
        mIpVCardView.setVisibility(View.GONE);
        mIpVCalendarView.setVisibility(View.GONE);
    }

    private boolean setPicView(String filePath) {
        try {
        Logger.d(TAG, "setPicView(): filePath = " + filePath + ", imageView = " + mImageContent);
        if (TextUtils.isEmpty(filePath) || null == mImageContent) {
            return false;
        }
        mIpImageSizeBg.setVisibility(View.GONE);
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        Bitmap bitmap = BitmapFactory.decodeFile(filePath, options);
        options.inJustDecodeBounds = false;

        int l = Math.max(options.outHeight, options.outWidth);
        int be = (int) (l / 500);
        if (be <= 0) {
            be = 1;
        }
        options.inSampleSize = be;
        bitmap = BitmapFactory.decodeFile(filePath, options);

            int mWidth = IpMessageResourceMananger.getInstance(MediatekFactory.getApplicationContext()).getRcseResource().getDimensionPixelSize(R.dimen.img_minwidth);
        Logger.d(
                TAG,
                "setPicView(): before set layout IpImageSizeBg.width = "
                        + mIpImageSizeBg.getWidth());
        if (bitmap != null) {
            bitmap = IpMessageUtils.resizeImage(bitmap, mWidth, bitmap.getHeight() * mWidth
                    / bitmap.getWidth(), true);
            mImageContent.setImageBitmap(bitmap);
            mIpImageSizeBg.setVisibility(View.VISIBLE);
            return true;
        } else {
            mImageContent.setImageResource(R.drawable.ic_missing_thumbnail_picture);
            mIpImageSizeBg.setVisibility(View.GONE);
            return false;
        }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return false;
        }
    }

    private boolean setVideoView(String filePath) {
        Logger.d(TAG, "setVideoView(): filePath = " + filePath + ", imageView = " + mImageContent);
        if (TextUtils.isEmpty(filePath) || null == mImageContent) {
            return false;
        }
        mMediaPlayView.setVisibility(View.VISIBLE);
        Bitmap bitmap = ThumbnailUtils.createVideoThumbnail(filePath, Thumbnails.MICRO_KIND);
        if (null != bitmap) {
            mImageContent.setImageBitmap(bitmap);
            mIpImageSizeBg.setVisibility(View.VISIBLE);
            return true;
        } else {
            mImageContent.setImageResource(R.drawable.ic_missing_thumbnail_picture);
            mIpImageSizeBg.setVisibility(View.GONE);
            return false;
        }
    }

    private void sendIpTextMessage(String body, String to) {
        final IpTextMessage ipMessage = new IpTextMessage();
        ipMessage.setBody(body);
        ipMessage.setType(IpMessageType.TEXT);
        ipMessage.setTo(to);
        int sendModeType = IpMessageSendMode.AUTO;
        final int sendMode = sendModeType;
        mCallback.onPreMessageSent();
        new Thread(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "sendIpTextMessage(): calling API: saveIpMsg().");
                int ret = -1;
                ret = IpMessageManager.getInstance(mContext).saveIpMsg(
                        ipMessage, sendMode);
                if (ret == -1) {
                    Logger.w(TAG, "sendIpTextMessage failed! ");
                } else {
                    // success.
                    mCallback.onMessageSent();
                }
            }
        }).start();
    }

    private boolean isCurGroupIpMessage() {
        boolean result = false;
        if (isCurIpMessage()) {
            String number = mCallback.getNumber();
            Logger.d(TAG, "number:" + number);
            if (number != null && number.startsWith(IpMessageConsts.GROUP_START)) {
                // this is group message
                result = true;
            }
        }
        Logger.d(TAG, "is group message:" + result);
        return result;
    }

    private boolean isCurJoynConvergedIpMessage() {
        boolean result = false;
        if (isCurIpMessage()) {
            String number = mCallback.getNumber();
            Logger.d(TAG, "number:" + number);
            if (number != null && number.startsWith(IpMessageConsts.JOYN_START)) {
                // this is joyn converged message
                result = true;
            }
        }
        Logger.d(TAG, "is converaged message:" + result);
        return result;
    }

    private String getCurGroupIpMessageNumber() {
        String number = "";
        if (isCurIpMessage()) {
            String num = mCallback.getNumber();
            Logger.d(TAG, "number:" + num);
            if (num != null && num.startsWith(IpMessageConsts.GROUP_START)) {
                // this is group message
                number = num;
            }

        }
        Logger.d(TAG, "group message number:" + number);
        return number;
    }

    private String getCurGroupIpMessageName() {
        String name = "";
        if (isCurIpMessage()) {
            String nam = mCallback.getName();
            Logger.d(TAG, "name:" + nam);
            if (nam != null) {
                // this is group message name
                name = IpMessageContactManager.getInstance(mContext).getNameByThreadId(
                        mCallback.getIpThreadId(),mCallback.getNumber());
            }

        }
        Logger.d(TAG, "group message name:" + name);
        return name;
    }

    private String getCurJoynIpMessageName() {
        String name = "";
        if (isCurIpMessage()) {
            String num = mCallback.getNumber();
            Logger.d(TAG, "number:" + num);
            if (num != null) {
                // this is joyn message name
                name = IpMessageContactManager.getInstance(mContext).getNameByNumber(num);
            }

        }
        Logger.d(TAG, "joyn message name:" + name);
        return name;
    }

    @Override
    public void notificationsReceived(Intent intent) {
        Logger.d(TAG, "DialogModeActivity, notificationReceived: intent = " + intent);
        String action = intent.getAction();
        if (TextUtils.isEmpty(action)) {
            return;
        }
        if (mContext.isFinishing()) {
            Logger.d(TAG, "activity is being destroied, ignore notification.");
            return;
        }
        Logger.d(TAG, "DialogModeActivity, action:" + action);
        if (!isCurIpMessage()) {
            Logger.d(TAG, "showing one is not ipmessage, ignore action.");
            return;
        }
        long msgId = 0L;
        switch (IpMessageUtils.getActionTypeByAction(action)) {
            case IpMessageUtils.IPMSG_DOWNLOAD_ATTACH_STATUS_ACTION:
            case IpMessageUtils.IPMSG_IP_MESSAGE_STATUS_ACTION:
                try {
                    if (IpMessageUtils.getActionTypeByAction(action) == IpMessageUtils.IPMSG_DOWNLOAD_ATTACH_STATUS_ACTION) {
                        msgId = intent.getLongExtra(DownloadAttachStatus.DOWNLOAD_MSG_ID, 0);
                    } else {
                        msgId = intent.getLongExtra(IpMessageStatus.IP_MESSAGE_ID, 0);
                    }
                    if (getCurIpMessageId() != msgId) {
                        Logger.d(TAG, "current ipmessage is not this:" + msgId + ",current:"
                                + getCurIpMessageId());
                        return;
                    }
                    mContext.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            showIpMessage(); // just refresh all element.
                        }
                    });
                } catch (NullPointerException e) {
                    // TODO: handle exception
                    Logger.d(TAG, "catch a NullPointerExcpetion?");
                }
                break;
            default:
                Logger.d(TAG, "DialogModeActivity. ignore notification.");
                return;
        }
    }

    private void openIpMsgThread(final long threadId) {
        Intent intent = new Intent(RemoteActivities.CHAT_DETAILS_BY_THREAD_ID);
        intent.putExtra(RemoteActivities.KEY_THREAD_ID, threadId);
        IpMessageUtils.startRemoteActivity(mContext, intent);
        mContext.finish();
    }
}
