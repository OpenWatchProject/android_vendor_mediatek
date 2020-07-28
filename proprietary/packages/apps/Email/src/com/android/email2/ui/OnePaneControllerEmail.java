package com.android.email2.ui;

import com.android.email.activity.setup.AccountSecurity;
import com.android.email.browse.ConversationPagerControllerEmail;
import com.android.mail.browse.ConversationPagerController;
import com.android.mail.providers.Account;
import com.android.mail.ui.MailActivity;
import com.android.mail.ui.OnePaneController;
import com.android.mail.ui.ViewMode;

public final class OnePaneControllerEmail extends OnePaneController {

    public OnePaneControllerEmail(MailActivity activity, ViewMode viewMode) {
        super(activity, viewMode);
    }

    @Override
    protected ConversationPagerController createConversationPagerController() {
        return new ConversationPagerControllerEmail(mActivity, this);
    }

    @Override
    public void changeAccount(Account account) {
        super.changeAccount(account);
        ///M: If the switch target account is Security Hold, just show the security need dialog. @{
        long accountId = Long.parseLong(account.uri.getLastPathSegment());
        if (com.android.emailcommon.provider.Account.isSecurityHold(mContext, accountId)) {
            mActivity.startActivity(AccountSecurity.actionUpdateSecurityIntent(mContext, accountId, true));
        }
        ///M: @}
    }
}
