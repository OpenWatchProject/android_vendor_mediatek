package com.mediatek.mms.plugin;

import android.content.Context;

import com.mediatek.mms.callback.IMessageUtilsCallback;
import com.mediatek.mms.ext.DefaultOpMessagePluginExt;
import com.mediatek.mms.ext.IOpAttachmentEditorExt;
import com.mediatek.mms.ext.IOpComposeExt;
import com.mediatek.mms.ext.IOpConversationListExt;
import com.mediatek.mms.ext.IOpConversationListItemExt;
import com.mediatek.mms.ext.IOpFileAttachmentModelExt;
import com.mediatek.mms.ext.IOpFileAttachmentUtilsExt;
import com.mediatek.mms.ext.IOpGeneralPreferenceActivityExt;
import com.mediatek.mms.ext.IOpManageSimMessagesExt;
import com.mediatek.mms.ext.IOpMessageListAdapterExt;
import com.mediatek.mms.ext.IOpMessageListItemExt;
import com.mediatek.mms.ext.IOpMessageUtilsExt;
import com.mediatek.mms.ext.IOpMessagingNotificationExt;
import com.mediatek.mms.ext.IOpMmsConfigExt;
import com.mediatek.mms.ext.IOpMmsPlayerActivityAdapterExt;
import com.mediatek.mms.ext.IOpMmsPlayerActivityExt;
import com.mediatek.mms.ext.IOpMmsPreferenceActivityExt;
import com.mediatek.mms.ext.IOpMultiDeleteActivityExt;
import com.mediatek.mms.ext.IOpMultiSaveActivityExt;
import com.mediatek.mms.ext.IOpNotificationTransactionExt;
import com.mediatek.mms.ext.IOpPushReceiverExt;
import com.mediatek.mms.ext.IOpRecipientsEditorExt;
import com.mediatek.mms.ext.IOpSearchActivityExt;
import com.mediatek.mms.ext.IOpSlideEditorActivityExt;
import com.mediatek.mms.ext.IOpSlideViewExt;
import com.mediatek.mms.ext.IOpSlideshowEditActivityExt;
import com.mediatek.mms.ext.IOpSlideshowModelExt;
import com.mediatek.mms.ext.IOpSmsPreferenceActivityExt;
import com.mediatek.mms.ext.IOpSmsReceiverServiceExt;
import com.mediatek.mms.ext.IOpSmsSingleRecipientSenderExt;
import com.mediatek.mms.ext.IOpStatusBarSelectorCreatorExt;
import com.mediatek.mms.ext.IOpSubSelectActivityExt;
import com.mediatek.mms.ext.IOpWPMessageActivityExt;
import com.mediatek.mms.ext.IOpWappushMessagingNotificationExt;
import com.mediatek.mms.ext.IOpWorkingMessageExt;
import com.mediatek.mms.ext.IOpSmilHelperExt;

/**
 * Op01MessagePluginExt. Used to get pluin instances.
 *
 */
public class Op01MessagePluginExt extends DefaultOpMessagePluginExt {
    public static IMessageUtilsCallback sMessageUtilsCallback = null;

    /**
     * Construction.
     * @param context Context
     */
    public Op01MessagePluginExt(Context context) {
        super(context);
    }

    public IOpManageSimMessagesExt getOpManageSimMessagesExt() {
        return new Op01ManageSimMessagesExt(mContext);
    }

    public IOpMessageListItemExt getOpMessageListItemExt() {
        return new Op01MessageListItemExt(mContext);
    }

    public IOpMessageUtilsExt getOpMessageUtilsExt() {
        return new Op01MessageUtilsExt(mContext);
    }

    public IOpMessagingNotificationExt getOpMessagingNotificationExt() {
        return new Op01MessagingNotificationExt(mContext);
    }
    /* q0 migration, phase out open url dialog*/
    /*
    public IOpMmsPlayerActivityAdapterExt getOpMmsPlayerActivityAdapterExt() {
        return new Op01MmsPlayerActivityAdapterExt(mContext);
    }
*/
    /* q0 migration, phase out*/
    /*
    public IOpPushReceiverExt getOpPushReceiverExt() {
        return new Op01PushReceiverExt();
    }
*/
    public IOpRecipientsEditorExt getOpRecipientsEditorExt() {
        return new Op01RecipientsEditorExt(mContext);
    }

    public IOpSlideViewExt getOpSlideViewExt() {
        return new Op01SlideViewExt(mContext);
    }

    public IOpSlideshowEditActivityExt getOpSlideshowEditActivityExt() {
        return new Op01SlideshowEditActivityExt(mContext);
    }

    public IOpWappushMessagingNotificationExt getOpWappushMessagingNotificationExt() {
        return new Op01WappushMessagingNotificationExt(mContext);
    }

    public IOpComposeExt getOpComposeExt() {
        return new Op01ComposeExt(mContext);
    }

    public IOpConversationListExt getOpConversationListExt() {
        return new Op01ConversationListExt(mContext);
    }

    /* q0 migration, phase out attach icon*/
    /*
    public IOpConversationListItemExt getOpConversationListItemExt() {
        return new Op01ConversationListItemExt(mContext);
    }
*/
    /* q0 migration, phase out multi-forward*/
    /*
    public IOpMultiDeleteActivityExt getOpMultiDeleteActivityExt() {
        return new Op01MultiDeleteActivityExt(mContext);
    }
*/
    public IOpGeneralPreferenceActivityExt getOpGeneralPreferenceActivityExt() {
        return new Op01GeneralPreferenceActivityExt(mContext);
    }
    /* q0 migration, phase out multi-forward*/
    /*
    public IOpMessageListAdapterExt getOpMessageListAdapterExt() {
        return new Op01MessageListAdapterExt(mContext);
    }
*/
    /* q0 migration, phase out attach enhance*/
    /*
    public IOpMmsPlayerActivityExt getOpMmsPlayerActivityExt() {
        return new Op01MmsPlayerActivityExt(mContext);
    }
*/
    public IOpMmsPreferenceActivityExt getOpMmsPreferenceActivityExt() {
        return new Op01MmsPreferenceActivityExt(mContext);
    }
    /* q0 migration, phase out*/
    /*
    public IOpNotificationTransactionExt getOpNotificationTransactionExt() {
        return new Op01NotificationTransactionExt(mContext);
    }
    */
    /* q0 migration, phase out text size adjust*/
    /*
    public IOpSlideEditorActivityExt getOpSlideEditorActivityExt() {
        return new Op01SlideEditorActivityExt(mContext);
    }
*/
    public IOpSmsReceiverServiceExt getOpSmsReceiverServiceExt() {
        return new Op01SmsReceiverServiceExt(mContext);
    }
    /* q0 migration, phase out*/
    /*
    public IOpStatusBarSelectorCreatorExt getOpStatusBarSelectorCreatorExt() {
        return new Op01StatusBarSelectorCreatorExt(mContext);
    }
*/
    public IOpSubSelectActivityExt getOpSubSelectActivityExt() {
        return new Op01SubSelectActivityExt(mContext);
    }
    /* q0 migration, phase out text size adjust*/
    /*
    public IOpWPMessageActivityExt getOpWPMessageActivityExt() {
        return new Op01WPMessageActivityExt(mContext);
    }
*/
    public void setOpMessageUtilsCallback(IMessageUtilsCallback callback) {
        sMessageUtilsCallback = callback;
    }

    public IOpWorkingMessageExt getOpWorkingMessageExt() {
        return new Op01WorkingMessageExt();
    }

    public IOpSlideshowModelExt getOpSlideshowModelExt() {
        return new Op01SlideshowModelExt();
    }

    public IOpMmsConfigExt getOpMmsConfigExt() {
        return new Op01MmsConfigExt();
    }

    /* q0 migration, phase out attach enhance*/
    /*
    public IOpFileAttachmentModelExt getOpFileAttachmentModelExt() {
        return new Op01FileAttachmentModelExt();
    }

    public IOpFileAttachmentUtilsExt getOpFileAttachmentUtilsExt() {
        return new Op01FileAttachmentUtilsExt(mContext);
    }
    */

    /* q0 migration, phase out*/
    /*
    @Override
    public IOpAttachmentEditorExt getOpAttachmentEditorExt() {
        return new Op01AttachmentEditorExt();
    }
*/
    /* q0 migration, phase out sms validate*/
    /*
    @Override
    public IOpSmsPreferenceActivityExt getOpSmsPreferenceActivityExt() {
        return new Op01SmsPreferenceActivityExt(mContext);
    }
    */
    /* q0 migration, phase out attach enhance*/
    /*
    public IOpMultiSaveActivityExt getOpMultiSaveActivityExt() {
        return new Op01MultiSaveActivityExt();
    }
*/
    public IOpSearchActivityExt getOpSearchActivityExt() {
        return new Op01SearchActivityExt();
    }

    public IOpSmsSingleRecipientSenderExt getOpSmsSingleRecipientSenderExt() {
        return new Op01SmsSingleRecipientSenderExt();
    }

    public IOpSmilHelperExt getOpSmilHelperExt() {
        return new Op01SmilHelperExt();
    }
}
