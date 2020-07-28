package com.mediatek.mms.callback;

import android.content.Context;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.net.Uri;

import com.google.android.mms.pdu.PduBody;

public interface IMessageUtilsCallback {

    // String
    public static int save_attachment = 1;
    public static int save_single_attachment_notes = 2;
    public static int save_multi_attachment_notes = 3;
    public static int attachment_vcard_name = 4;
    public static int attachment_vcalendar_name = 5;
    public static int add_slide = 6;
    public static int cannot_add_slide_anymore = 7;
    public static int file_attachment_common_name = 8;
    public static int file_attachment_vcard_name = 9;
    public static int file_attachment_vcalendar_name = 10;
    public static int save_single_supportformat_attachment_notes = 11;
    public static int invalid_contact_message = 12;
    public static int copy_to_sdcard_success = 13;
    public static int copy_to_sdcard_fail = 14;
    public static int multi_attach_files = 15;
    public static int from_label = 16;
    public static int subject_label = 17;

    // Drawable
    public static int ipmsg_chat_contact_vcard = 1;
    public static int ipmsg_chat_contact_calendar = 2;
    public static int ic_menu_add_slide = 3;
    public static int ic_vcard_attach = 4;
    public static int ic_vcalendar_attach = 5;
    public static int unsupported_file = 6;

    // text
    public static int set = 1;
    public static int no = 2;

    public static final long THREAD_ALL = -1;
    public static final long THREAD_NONE = -2;

    void showDiscardDraftConfirmDialogCallback(Context context,
            OnClickListener listener);
    int getMaxSlideNum();

    String getString(int resId, Object... formatArgs);
    Drawable getDrawable(int id);
    CharSequence getText(int resId);

    String getStorageStatus();

    int getRecyclerMessageLimit(boolean isMms);
    int getRecyclerMessageMinLimit(boolean isMms);
    int getRecyclerMessageMaxLimit(boolean isMms);
    void setRecyclerMessageLimit(boolean isMms, int limit);
    void deleteRecyclerOldMessages(boolean isMms);
    void deleteWapPushRecyclerOldMessages();
    void blockingUpdateNewMessageIndicatorCallback(Context context, long newMsgThreadId);
    IFileAttachmentModelCallback createVCardModel(String filename, Uri dataUri);
    void importVCard(IFileAttachmentModelCallback vcard);
    boolean isAlias(String number);
    String getEmailGateway();
    boolean getDeviceStorageFullStatus();
    void setUserSetMmsSizeLimitCallback(int limit);
    void updateCreationModeCallback(Context context);
    public void nonBlockingUpdateNewMessageIndicatorCallback(final Context context,
            final long newMsgThreadId, final boolean isStatusMessage);
    public void notifyDatasetChangedCallback();
    public Intent getTransactionServiceIntent();
    public Intent getComposeIntent();
    boolean getGroupMmsEnabled();
    ISlideshowModelCallback createFromPduBodyCallback(Context context, PduBody body);
    PduBody getPduBodyCallback(Context context, Uri uri);

}
