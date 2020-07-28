package com.mediatek.email.mail.store;

import android.content.Context;
import android.os.Bundle;

import com.android.email.mail.Store;
import com.android.email.mail.transport.MailTransport;
import com.android.emailcommon.mail.Flag;
import com.android.emailcommon.mail.Folder;
import com.android.emailcommon.mail.Folder.OpenMode;
import com.android.emailcommon.mail.MessagingException;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.HostAuth;
import com.android.emailcommon.provider.Mailbox;
import com.android.mail.utils.LogUtils;


/**
 * M: This class represents a pop3 store object, which an account have its own pop3 store.
 * This is our own implemented pop3 store which supports multiple folder/connection operations.
 */
public class Pop3Store extends Store {
    // All flags defining debug or development code settings must be FALSE
    // when code is checked in or released.
    /// M: Mark as protected @{
    protected static final boolean DEBUG_FORCE_SINGLE_LINE_UIDL = false;
    protected static final boolean DEBUG_LOG_RAW_STREAM = false;
    protected static final Flag[] PERMANENT_FLAGS = { Flag.DELETED };
    /// @}
    /** The name of the only mailbox available to POP3 accounts */
    protected static final String POP3_MAILBOX_NAME = "INBOX"; // M: Mark as protected

    /**
     * Static named constructor.
     */
    public static Store newInstance(Account account, Context context) throws MessagingException {
        return new Pop3Store(context, account);
    }

    /**
     * Creates a new store for the given account.
     */
    private Pop3Store(Context context, Account account) throws MessagingException {
        mContext = context;
        mAccount = account;

        HostAuth recvAuth = account.getOrCreateHostAuthRecv(context);
        mTransport = new MailTransport(context, "POP3", recvAuth);
        String[] userInfoParts = recvAuth.getLogin();
        if (userInfoParts != null) {
            mUsername = userInfoParts[0];
            mPassword = userInfoParts[1];
        }
    }

    /**
     * For testing only.  Injects a different transport.  The transport should already be set
     * up and ready to use.  Do not use for real code.
     * @param testTransport The Transport to inject and use for all future communication.
     */
    /* package */ void setTransport(MailTransport testTransport) {
        mTransport = testTransport;
    }

    /**
     * M: Always return a new folder with its own connection
     */
    @Override
    public Folder getFolder(String name) throws MessagingException {
        if (name != null && name.equalsIgnoreCase(POP3_MAILBOX_NAME)) {
            name = POP3_MAILBOX_NAME;
        } else {
            LogUtils.e(LogUtils.TAG, "Only INBOX is availibe for pop3");
            throw new MessagingException("Folder does not exist");
        }
        /// M: Always return new Folder
        return new Pop3Folder(this, name);
    }

    /**
     * M: May throw messaging exception
     */
    @Override
    public Folder[] updateFolders() throws MessagingException {
        Mailbox mailbox = Mailbox.restoreMailboxOfType(mContext, mAccount.mId, Mailbox.TYPE_INBOX);
        if (mailbox == null) {
            mailbox = Mailbox.newSystemMailbox(mContext, mAccount.mId, Mailbox.TYPE_INBOX);
        }
        if (mailbox.isSaved()) {
            mailbox.update(mContext, mailbox.toContentValues());
        } else {
            mailbox.save(mContext);
        }
        return new Folder[] { getFolder(mailbox.mServerId) };
    }

    /**
     * Used by account setup to test if an account's settings are appropriate.  The definition
     * of "checked" here is simply, can you log into the account and does it meet some minimum set
     * of feature requirements?
     *
     * @throws MessagingException if there was some problem with the account
     */
    @Override
    public Bundle checkSettings() throws MessagingException {
        /// M: Create new pop3Folder
        Pop3Folder folder = new Pop3Folder(this, POP3_MAILBOX_NAME);
        Bundle bundle = null;
        try {
            folder.open(OpenMode.READ_WRITE);
            bundle = folder.checkSettings();
        } finally {
            folder.close(false);    // false == don't expunge anything
        }
        return bundle;
    }

    /**
     * M: Gets a new connection for pop3. note: A fact had been found that for
     * certain POP3 servers, reuse a connection to send "STATUS" command,
     * server would not respond correctly. So it should give up to use
     * connection pool for POP.
     */
    synchronized Pop3Connection getConnectionSync() {
           return new Pop3Connection(this, mUsername, mPassword);
    }
}
