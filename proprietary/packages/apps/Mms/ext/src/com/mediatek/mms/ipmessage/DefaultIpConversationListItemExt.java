package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.net.Uri;
import android.widget.ImageView;
import android.widget.QuickContactBadge;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class DefaultIpConversationListItemExt implements IIpConversationListItemExt {

    @Override
    public void onIpSyncView(Context context,
            ImageView fullIntegrationModeView, QuickContactBadge avatarView) {

    }

    @Override
    public String onIpFormatMessage(IIpContactExt ipContact,
            long threadId, String number, String name) {
        return name;
    }

    @Override
    public boolean updateIpAvatarView(IIpContactExt ipContact,
            String number, QuickContactBadge avatarView, Uri uri) {
        return false;
    }

    @Override
    public boolean updateIpAvatarView(IIpConversationExt ipConv, QuickContactBadge avatarView,
            ImageView selectIcon) {
        return false;
    }

    @Override
    public boolean onIpBind(IIpConversationExt ipConv,
            boolean isActionMode, boolean isChecked, int convType,
            RelativeLayout conversationItem, TextView fromView,
            TextView subjectView, TextView dateView) {
        return false;
    }

    public void onIpDetachedFromWindow() {
        return;
    }

    public void onIpUnbind() {
        return;
    }
}
