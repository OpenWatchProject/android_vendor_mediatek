package com.android.email.browse;

import android.app.FragmentManager;
import android.content.Context;

import com.android.mail.browse.ConversationPagerAdapter;
import com.android.mail.providers.Account;
import com.android.mail.providers.Conversation;
import com.android.mail.providers.Folder;
import com.android.mail.ui.AbstractConversationViewFragment;
import com.mediatek.email.attachment.AttachmentAutoClearController;

public final class ConversationPagerAdapterEmail extends ConversationPagerAdapter {

    public ConversationPagerAdapterEmail(Context cont, FragmentManager fm, Account account,
            Folder folder, Conversation initialConversation) {
        super(cont, fm, account, folder, initialConversation);
    }

    @Override
    protected AbstractConversationViewFragment getConversationViewFragment(Conversation c) {
        /// M: Record the opened msg id @{
        AttachmentAutoClearController.recordConversationMsgIdAsync(c);
        /// @}
        return super.getConversationViewFragment(c);
    }
}
