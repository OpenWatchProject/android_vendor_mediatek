package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.net.Uri;
import android.widget.ImageView;
import android.widget.QuickContactBadge;
import android.widget.RelativeLayout;
import android.widget.TextView;

public interface IIpConversationListItemExt {

    /**
     * M: called onFinishInflate
     * @param context: context
     * @param fullIntegrationModeView: fullIntegrationModeView
     * @param avatarView: avatarView
     * @internal
     */
    public void onIpSyncView(Context context,
            ImageView fullIntegrationModeView, QuickContactBadge avatarView);

    /**
     * M: called formatMessage
     * @param ipContact: IpContact
     * @param threadId: threadId
     * @param number: number
     * @param name: name
     * @return boolean
     * @internal
     */
    public String onIpFormatMessage(IIpContactExt ipContact,
            long threadId, String number, String name);

    /**
     * M: called updateAvatarView
     * @param ipContact: IpContact
     * @param number: number
     * @param avatarView: avatarView
     * @param uri: uri
     * @return boolean
     * @internal
     */
    public boolean updateIpAvatarView(IIpContactExt ipContact,
            String number, QuickContactBadge avatarView, Uri uri);

    /**
     * M: called updateAvatarView
     * @param ipContact: IpContact
     * @param avatarView: avatarView
     * @param selectIcon: ImageView, when the conversationItem is checked, show this view.
     * @return if process
     * @internal
     */
    public boolean updateIpAvatarView(IIpConversationExt ipConv, QuickContactBadge avatarView,
            ImageView selectIcon);

    /**
     * M: called bind
     * @param ipConv: IpConversation
     * @param convType: convType
     * @param fromView: fromView
     * @param subjectView: subjectView
     * @param dateView: dateView
     * @return boolean
     * @internal
     */
    public boolean onIpBind(IIpConversationExt ipConv,
            boolean isActionMode, boolean isChecked, int convType,
            RelativeLayout conversationItem, TextView fromView,
            TextView subjectView, TextView dateView);

    /**
     * called in onDetachedFromWindow in ConversationListItem
     * @internal
     */
    public void onIpDetachedFromWindow();

    /**
     * called in unbind in ConversationListItem
     * @internal
     */
    public void onIpUnbind();
}
