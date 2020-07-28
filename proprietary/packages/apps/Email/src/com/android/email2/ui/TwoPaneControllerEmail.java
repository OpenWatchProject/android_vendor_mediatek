package com.android.email2.ui;

import com.android.email.browse.ConversationPagerControllerEmail;
import com.android.mail.browse.ConversationPagerController;
import com.android.mail.ui.MailActivity;
import com.android.mail.ui.TwoPaneController;
import com.android.mail.ui.ViewMode;

public final class TwoPaneControllerEmail extends TwoPaneController {

    public TwoPaneControllerEmail(MailActivity activity, ViewMode viewMode) {
        super(activity, viewMode);
    }

    @Override
    protected ConversationPagerController createConversationPagerController() {
        return new ConversationPagerControllerEmail(mActivity, this);
    }

    /** M: We prefer not show the first conversation and make the behavior same with JellyBean.
     * @see com.android.mail.ui.TwoPaneController#shouldShowFirstConversation()
     */
    @Override
    public boolean shouldShowFirstConversation() {
        return false;
    }
}
