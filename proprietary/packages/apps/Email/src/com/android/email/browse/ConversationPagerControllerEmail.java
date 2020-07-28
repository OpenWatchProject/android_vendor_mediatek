package com.android.email.browse;

import com.android.mail.browse.ConversationPagerAdapter;
import com.android.mail.browse.ConversationPagerController;
import com.android.mail.providers.Account;
import com.android.mail.providers.Conversation;
import com.android.mail.providers.Folder;
import com.android.mail.ui.ActivityController;
import com.android.mail.ui.RestrictedActivity;

public final class ConversationPagerControllerEmail extends ConversationPagerController {

    public ConversationPagerControllerEmail(RestrictedActivity activity,
            ActivityController controller) {
        super(activity, controller);
    }

    @Override
    protected ConversationPagerAdapter createPagerAdapter(Account account, Folder folder,
            Conversation initialConversation) {
        return new ConversationPagerAdapterEmail(mPager.getContext(), mFragmentManager,
                account, folder, initialConversation);
    }
}
