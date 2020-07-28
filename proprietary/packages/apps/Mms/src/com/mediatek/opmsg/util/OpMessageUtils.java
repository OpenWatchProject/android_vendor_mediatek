package com.mediatek.opmsg.util;

import android.content.Context;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.util.Log;

import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.data.WorkingMessage;
import com.android.mms.model.SlideshowModel;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.ui.ComposeMessageActivity;
import com.android.mms.ui.MessageUtils;
import com.android.mms.ui.SlideshowEditor;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.android.mms.util.Recycler;
import com.android.mms.widget.MmsWidgetProvider;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.PduBody;

//import com.mediatek.common.MPlugin;
import com.mediatek.mms.callback.IFileAttachmentModelCallback;
import com.mediatek.mms.callback.IMessageUtilsCallback;
import com.mediatek.mms.callback.ISlideshowModelCallback;
import com.mediatek.mms.ext.DefaultOpMessagePluginExt;
import com.mediatek.mms.ext.IOpMessagePluginExt;
import com.mediatek.mms.ext.OpMmsCustomizationUtils;
import com.mediatek.mms.model.FileAttachmentModel;
import com.mediatek.mms.model.VCardModel;
import com.mediatek.mms.util.VCardUtils;
import com.mediatek.wappush.WapPushMessagingNotification;

public class OpMessageUtils implements IMessageUtilsCallback {
    private static final String TAG = "Mms/opmsg/utils";

    /// M: add for opmessage {@
    public static IOpMessagePluginExt sOpMessagePluginExt = null;

    private static Context mContext;

    private static OpMessageUtils sOpMessageUtils;

    public static void init(Context context) {
        mContext = context;
        sOpMessageUtils = new OpMessageUtils();
    }

    public static synchronized IOpMessagePluginExt getOpMessagePlugin() {
        if (sOpMessagePluginExt == null) {
            initOpMessagePlugin(mContext);
        }
        return sOpMessagePluginExt;
    }

    private static synchronized void initOpMessagePlugin(Context context) {
        if (sOpMessagePluginExt == null) {
/*            sOpMessagePluginExt = (IOpMessagePluginExt) MPlugin.createInstance(
                    IOpMessagePluginExt.class.getName(), context);
        mContext = context;
            MmsLog.d(TAG, "sOpMessagePlugin = " + sOpMessagePluginExt);
            if (sOpMessagePluginExt == null) {
                sOpMessagePluginExt = new DefaultOpMessagePluginExt(context);
                MmsLog.d(TAG, "default sOpMessagePlugin = " + sOpMessagePluginExt);
            }
            sOpMessagePluginExt.setOpMessageUtilsCallback(sOpMessageUtils);
                */
            mContext = context;
            MmsLog.d(TAG, "sOpMessagePlugin = " + sOpMessagePluginExt);
            synchronized (IOpMessagePluginExt.class) {
                if (sOpMessagePluginExt == null) {
                    sOpMessagePluginExt = OpMmsCustomizationUtils.getOpFactory(mContext)
                            .makeOpMessagePluginExt();
                    MmsLog.i(TAG, "[getOpConversationExt]create ext_instance:"
                               + sOpMessagePluginExt);
                }
                sOpMessagePluginExt.setOpMessageUtilsCallback(sOpMessageUtils);
            }
        }
    }

    @Override
    public void showDiscardDraftConfirmDialogCallback(Context context, OnClickListener listener) {
        MessageUtils.showDiscardDraftConfirmDialog(context, listener);
    }

    @Override
    public int getMaxSlideNum() {
        return SlideshowEditor.MAX_SLIDE_NUM;
    }

    @Override
    public String getString(int id, Object... formatArgs) {
        String text = "";
        switch (id) {
            case IMessageUtilsCallback.save_attachment:
                text = mContext.getString(R.string.save_attachment);
                break;
            case IMessageUtilsCallback.save_single_attachment_notes:
                text = mContext.getString(R.string.save_single_attachment_notes);
                break;
            case IMessageUtilsCallback.save_multi_attachment_notes:
                text = mContext.getString(R.string.save_multi_attachment_notes);
                break;
            case  IMessageUtilsCallback.attachment_vcard_name:
                text = mContext.getString(R.string.file_attachment_vcard_name, formatArgs[0]);
                break;
            case IMessageUtilsCallback.attachment_vcalendar_name:
                text = mContext.getString(R.string.file_attachment_vcalendar_name, formatArgs[0]);
                break;
            case add_slide:
                text = mContext.getString(R.string.add_slide);
                break;
            case cannot_add_slide_anymore:
                text = mContext.getString(R.string.cannot_add_slide_anymore);
                break;
            case file_attachment_common_name:
                text = mContext.getString(R.string.file_attachment_common_name, formatArgs[0]);
                break;
            case file_attachment_vcard_name:
                text = mContext.getString(R.string.file_attachment_vcard_name);
                break;
            case file_attachment_vcalendar_name:
                text = mContext.getString(R.string.file_attachment_vcalendar_name);
                break;
            case save_single_supportformat_attachment_notes:
                text = mContext.getString(R.string.save_single_supportformat_attachment_notes);
                break;
            case invalid_contact_message:
                text = mContext.getString(R.string.invalid_contact_message);
                break;
            case copy_to_sdcard_success:
                text = mContext.getString(R.string.copy_to_sdcard_success);
                break;
            case copy_to_sdcard_fail:
                text = mContext.getString(R.string.copy_to_sdcard_fail);
                break;
            case multi_attach_files:
                text = mContext.getString(R.string.file_attachment_common_name,
                        mContext.getString(R.string.file_attachment_contains) + " "
                        + String.valueOf(formatArgs[0]) + " "
                        + mContext.getString(R.string.file_attachment_files));
                break;
            case from_label:
                text = mContext.getString(R.string.from_label);
                break;
            case subject_label:
                text = mContext.getString(R.string.subject_label);
                break;
            default:
                break;
        }
        return text;
    }

    @Override
    public Drawable getDrawable(int id) {
        Drawable drawable = null;
        switch (id) {
//            case IOpMessageUtilsCallback.save_attachment:
//                drawable = mContext.getResources()
//        .getDrawable(R.drawable.ipmsg_chat_contact_vcard);
//                break;
//            case IOpMessageUtilsCallback.save_single_attachment_notes:
//                drawable = mContext.getResources()
//        .getDrawable(R.drawable.ipmsg_chat_contact_vcard);
            case ic_menu_add_slide:
                drawable = mContext.getResources().getDrawable(R.drawable.ic_menu_add_slide);
                break;
            case ic_vcard_attach:
                drawable = mContext.getResources().getDrawable(R.drawable.ic_vcard_attach);
                break;
            case ic_vcalendar_attach:
                drawable = mContext.getResources().getDrawable(R.drawable.ic_vcalendar_attach);
                break;
            case unsupported_file:
                drawable = mContext.getResources().getDrawable(R.drawable.unsupported_file);
                break;
            case ipmsg_chat_contact_vcard:
                drawable = mContext.getResources().getDrawable(
                        R.drawable.ipmsg_chat_contact_vcard);
                break;
            case ipmsg_chat_contact_calendar:
                drawable = mContext.getResources().getDrawable(
                        R.drawable.ipmsg_chat_contact_calendar);
                break;
            default:
                break;
        }
        return drawable;

    }

    @Override
    public CharSequence getText(int resId) {
        CharSequence text = "";
        switch (resId) {
        case set:
            text = mContext.getText(R.string.set);
            break;
        case no:
            text = mContext.getString(R.string.no);
            break;
        default:
            break;
        }
        return text;
    }

    public String getStorageStatus() {
        return MessageUtils.getStorageStatus(mContext);
    }

    public int getRecyclerMessageLimit(boolean isMms) {
        if (isMms) {
            return Recycler.getMmsRecycler().getMessageLimit(mContext);
        } else {
            return Recycler.getSmsRecycler().getMessageLimit(mContext);
        }
    }

    public int getRecyclerMessageMinLimit(boolean isMms) {
        if (isMms) {
            return Recycler.getMmsRecycler().getMessageMinLimit();
        } else {
            return Recycler.getSmsRecycler().getMessageMinLimit();
        }
    }

    public int getRecyclerMessageMaxLimit(boolean isMms) {
        if (isMms) {
            return Recycler.getMmsRecycler().getMessageMaxLimit();
        } else {
            return Recycler.getSmsRecycler().getMessageMaxLimit();
        }
    }

    public void setRecyclerMessageLimit(boolean isMms, int limit) {
        if (isMms) {
            Recycler.getMmsRecycler().setMessageLimit(mContext, limit);
        } else {
            Recycler.getSmsRecycler().setMessageLimit(mContext, limit);
        }
    }

    public void deleteRecyclerOldMessages(boolean isMms) {
        if (isMms) {
            Recycler.getMmsRecycler().deleteOldMessages(mContext);
        } else {
            Recycler.getSmsRecycler().deleteOldMessages(mContext);
        }
    }

    public void deleteWapPushRecyclerOldMessages() {
        Recycler.getWapPushRecycler().deleteOldMessages(mContext);
    }

    public void blockingUpdateNewMessageIndicatorCallback(Context context, long newMsgThreadId) {
        WapPushMessagingNotification.blockingUpdateNewMessageIndicator(mContext, newMsgThreadId);
    }

    public void nonBlockingUpdateNewMessageIndicatorCallback(final Context context,
            final long newMsgThreadId, final boolean isStatusMessage) {
        MessagingNotification.nonBlockingUpdateNewMessageIndicator(context, newMsgThreadId,
                isStatusMessage);
    }

    public void notifyDatasetChangedCallback() {
        MmsWidgetProvider.notifyDatasetChanged(mContext);
    }

    public Intent getTransactionServiceIntent() {
        Intent intent = new Intent();
        intent.setClassName(MmsConfig.MMS_APP_SERVICE_PACKAGE, MmsConfig.TRANSACTION_SERVICE);
        return intent;
    }

    public Intent getComposeIntent() {
        return new Intent(mContext, ComposeMessageActivity.class);
    }

    public IFileAttachmentModelCallback createVCardModel(String filename, Uri dataUri) {
        try {
            return new VCardModel(mContext, MmsContentType.TEXT_VCARD, filename, dataUri);
        } catch (Exception e) {
            return null;
            // TODO: handle exception
        }
    }

    public void importVCard(IFileAttachmentModelCallback vcard) {
        VCardUtils.importVCard(mContext, (FileAttachmentModel) vcard);
    }

    public boolean isAlias(String number) {
        return MessageUtils.isAlias(number);
    }

    public String getEmailGateway() {
        return MmsConfig.getEmailGateway();
    }

    public boolean getDeviceStorageFullStatus() {
        return MmsConfig.getDeviceStorageFullStatus();
    }


    @Override
    public void setUserSetMmsSizeLimitCallback(int limit) {
        MmsConfig.setUserSetMmsSizeLimit(limit);
    }

    @Override
    public void updateCreationModeCallback(Context context) {
       WorkingMessage.updateCreationMode(context);
    }

    public boolean getGroupMmsEnabled() {
        return MmsConfig.getGroupMmsEnabled();
    }

    public ISlideshowModelCallback createFromPduBodyCallback(Context context, PduBody body) {
        try {
            return SlideshowModel.createFromPduBody(context, body);
        } catch (MmsException e) {
            Log.e(TAG, "createFromPduBodyCb Exception: " + e);
        }

        return null;
    }

    public PduBody getPduBodyCallback(Context context, Uri uri) {
        try {
            return SlideshowModel.getPduBody(context, uri);
        } catch (MmsException e) {
            Log.e(TAG, "createFromPduBodyCb Exception: " + e);
        }

        return null;
    }
}
