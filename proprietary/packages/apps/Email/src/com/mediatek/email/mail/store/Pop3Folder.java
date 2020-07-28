package com.mediatek.email.mail.store;

import android.content.Context;
import android.os.Bundle;

import com.android.email.DebugUtils;
import com.android.emailcommon.Logging;
import com.android.emailcommon.internet.MimeMessage;
import com.android.emailcommon.mail.FetchProfile;
import com.android.emailcommon.mail.Flag;
import com.android.emailcommon.mail.Folder;
import com.android.emailcommon.mail.Folder.FolderType;
import com.android.emailcommon.mail.Folder.MessageRetrievalListener;
import com.android.emailcommon.mail.Folder.MessageUpdateCallbacks;
import com.android.emailcommon.mail.Folder.OpenMode;
import com.android.emailcommon.mail.Message;
import com.android.emailcommon.mail.MessagingException;
import com.android.emailcommon.service.EmailServiceProxy;
import com.android.emailcommon.service.SearchParams;
import com.android.emailcommon.utility.LoggingInputStream;
import com.android.emailcommon.utility.Utility;
import com.android.mail.utils.LogUtils;
import com.google.common.annotations.VisibleForTesting;

import org.apache.james.mime4j.EOLConvertingInputStream;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Locale;

/**
 * M: This class stands for a pop3 folder, which could be got from a pop3 store to do network actions,
 * One account can only have one pop3 store, meanwhile, a pop3 store can have multiple pop3 folder
 * to do things in parallel
 */
public class Pop3Folder extends Folder {
    private final HashMap<String, Pop3Message> mUidToMsgMap
            = new HashMap<String, Pop3Message>();
    private final HashMap<Integer, Pop3Message> mMsgNumToMsgMap
            = new HashMap<Integer, Pop3Message>();
    private final HashMap<String, Integer> mUidToMsgNumMap = new HashMap<String, Integer>();
    private final Message[] mOneMessage = new Message[1];

    /// M: Whether we need to expunge the connection of this folder, when set true, we should just
    // close the connection rather than pool it
    private boolean mNeedExpungeConn;
    private final String mName;
    private int mMessageCount;
    // Store
    private Pop3Store mStore;
    // Connection for this folder
    private Pop3Connection mConnection;

    public Pop3Folder(Pop3Store store, String name) {
        if (name.equalsIgnoreCase(Pop3Store.POP3_MAILBOX_NAME)) {
            mName = Pop3Store.POP3_MAILBOX_NAME;
        } else {
            mName = name;
        }
        mStore = store;
        mNeedExpungeConn = false;
    }

    /**
     * Used by account setup to test if an account's settings are appropriate.  Here, we run
     * an additional test to see if UIDL is supported on the server. If it's not we
     * can't service this account.
     *
     * @return Bundle containing validation data (code and, if appropriate, error message)
     * @throws MessagingException if the account is not going to be useable
     */
    public Bundle checkSettings() throws MessagingException {
        Bundle bundle = new Bundle();
        int result = MessagingException.NO_ERROR;
        try {
            UidlParser parser = new UidlParser();
            mConnection.executeSimpleCommand("UIDL");
            // drain the entire output, so additional communications don't get confused.
            String response;
            while ((response = mConnection.readLine(false)) != null) {
                parser.parseMultiLine(response);
                if (parser.mEndOfMessage) {
                    break;
                }
            }
        } catch (IOException ioe) {
            mConnection.close();
            result = MessagingException.IOERROR;
            bundle.putString(EmailServiceProxy.VALIDATE_BUNDLE_ERROR_MESSAGE,
                    ioe.getMessage());
        }
        bundle.putInt(EmailServiceProxy.VALIDATE_BUNDLE_RESULT_CODE, result);
        return bundle;
    }

    @Override
    public void open(OpenMode mode) throws MessagingException {
        if (!mName.equalsIgnoreCase(Pop3Store.POP3_MAILBOX_NAME)) {
            throw new MessagingException("Folder does not exist");
        }

        if (null == mConnection) {
            mConnection = mStore.getConnectionSync();
        }
        mConnection.open();

        Exception statException = null;
        try {
            String response = mConnection.executeSimpleCommand("STAT");
            String[] parts = response.split(" ");
            if (parts.length < 2) {
                statException = new IOException();
            } else {
                mMessageCount = Integer.parseInt(parts[1]);
            }
        } catch (MessagingException me) {
            statException = me;
        } catch (IOException ioe) {
            statException = ioe;
        } catch (NumberFormatException nfe) {
            statException = nfe;
        }
        if (statException != null) {
            mConnection.close();
            if (DebugUtils.DEBUG) {
                LogUtils.d(Logging.LOG_TAG, statException.toString());
            }
            throw new MessagingException("POP3 STAT", statException);
        }

        mUidToMsgMap.clear();
        mMsgNumToMsgMap.clear();
        mUidToMsgNumMap.clear();
    }

    @Override
    public OpenMode getMode() {
        return OpenMode.READ_WRITE;
    }

    /**
     * Close the folder (and the transport below it).
     *
     * MUST NOT return any exceptions.
     *
     * @param expunge If true all deleted messages will be expunged
     */
    @Override
    public void close(boolean expunge) {
        mMessageCount = -1;
        mConnection.close();
        mConnection = null;
        mNeedExpungeConn = false;
    }

    @Override
    public String getName() {
        return mName;
    }

    // POP3 does not folder creation
    @Override
    public boolean canCreate(FolderType type) {
        return false;
    }

    @Override
    public boolean create(FolderType type) {
        return false;
    }

    @Override
    public boolean exists() {
        return mName.equalsIgnoreCase(Pop3Store.POP3_MAILBOX_NAME);
    }

    @Override
    public int getMessageCount() {
        return mMessageCount;
    }

    @Override
    public int getUnreadMessageCount() {
        return -1;
    }

    @Override
    public Message getMessage(String uid) throws MessagingException {
        if (mUidToMsgNumMap.size() == 0) {
            try {
                indexMsgNums(1, mMessageCount);
            } catch (IOException ioe) {
                mConnection.close();
                if (DebugUtils.DEBUG) {
                    LogUtils.d(Logging.LOG_TAG, "Unable to index during getMessage " + ioe);
                }
                throw new MessagingException("getMessages", ioe);
            }
        }
        Pop3Message message = mUidToMsgMap.get(uid);
        return message;
    }

    @Override
    public Pop3Message[] getMessages(int start, int end, MessageRetrievalListener listener)
            throws MessagingException {
        return null;
    }

    @Override
    public Pop3Message[] getMessages(long startDate, long endDate,
            MessageRetrievalListener listener) throws MessagingException {
        return null;
    }

    public Pop3Message[] getMessages(int end, final int limit)
            throws MessagingException {
        try {
            indexMsgNums(1, end);
        } catch (IOException ioe) {
            mConnection.close();
            if (DebugUtils.DEBUG) {
                LogUtils.d(Logging.LOG_TAG, ioe.toString());
            }
            throw new MessagingException("getMessages", ioe);
        }
        ArrayList<Message> messages = new ArrayList<Message>();
        for (int msgNum = end; msgNum > 0 && (messages.size() < limit); msgNum--) {
            Pop3Message message = mMsgNumToMsgMap.get(msgNum);
            if (message != null) {
                messages.add(message);
            }
        }
        return messages.toArray(new Pop3Message[messages.size()]);
    }

    /**
     * Ensures that the given message set (from start to end inclusive)
     * has been queried so that uids are available in the local cache.
     * @param start
     * @param end
     * @throws MessagingException
     * @throws IOException
     */
    private void indexMsgNums(int start, int end)
            throws MessagingException, IOException {
        if (!mMsgNumToMsgMap.isEmpty()) {
            return;
        }
        UidlParser parser = new UidlParser();
        if (Pop3Store.DEBUG_FORCE_SINGLE_LINE_UIDL || (mMessageCount > 5000)) {
            /*
             * In extreme cases we'll do a UIDL command per message instead of a bulk
             * download.
             */
            for (int msgNum = start; msgNum <= end; msgNum++) {
                Pop3Message message = mMsgNumToMsgMap.get(msgNum);
                if (message == null) {
                    String response = mConnection.executeSimpleCommand("UIDL " + msgNum);
                    if (!parser.parseSingleLine(response)) {
                        throw new IOException();
                    }
                    message = new Pop3Message(parser.mUniqueId, this);
                    indexMessage(msgNum, message);
                }
            }
        } else {
            String response = mConnection.executeSimpleCommand("UIDL");
            while ((response = mConnection.readLine(false)) != null) {
                if (!parser.parseMultiLine(response)) {
                    throw new IOException();
                }
                if (parser.mEndOfMessage) {
                    break;
                }
                int msgNum = parser.mMessageNumber;
                if (msgNum >= start && msgNum <= end) {
                    Pop3Message message = mMsgNumToMsgMap.get(msgNum);
                    if (message == null) {
                        message = new Pop3Message(parser.mUniqueId, this);
                        indexMessage(msgNum, message);
                    }
                }
            }
        }
    }

    /**
     * Simple parser class for UIDL messages.
     *
     * <p>NOTE:  In variance with RFC 1939, we allow multiple whitespace between the
     * message-number and unique-id fields.  This provides greater compatibility with some
     * non-compliant POP3 servers, e.g. mail.comcast.net.
     */
    /* package */ class UidlParser {

        /**
         * Caller can read back message-number from this field
         */
        public int mMessageNumber;
        /**
         * Caller can read back unique-id from this field
         */
        public String mUniqueId;
        /**
         * True if the response was "end-of-message"
         */
        public boolean mEndOfMessage;
        /**
         * True if an error was reported
         */
        public boolean mErr;

        /**
         * Construct & Initialize
         */
        public UidlParser() {
            mErr = true;
        }

        /**
         * Parse a single-line response.  This is returned from a command of the form
         * "UIDL msg-num" and will be formatted as: "+OK msg-num unique-id" or
         * "-ERR diagnostic text"
         *
         * @param response The string returned from the server
         * @return true if the string parsed as expected (e.g. no syntax problems)
         */
        public boolean parseSingleLine(String response) {
            mErr = false;
            if (response == null || response.length() == 0) {
                return false;
            }
            char first = response.charAt(0);
            if (first == '+') {
                String[] uidParts = response.split(" +");
                if (uidParts.length >= 3) {
                    try {
                        mMessageNumber = Integer.parseInt(uidParts[1]);
                    } catch (NumberFormatException nfe) {
                        return false;
                    }
                    mUniqueId = uidParts[2];
                    mEndOfMessage = true;
                    return true;
                }
            } else if (first == '-') {
                mErr = true;
                return true;
            }
            return false;
        }

        /**
         * Parse a multi-line response.  This is returned from a command of the form
         * "UIDL" and will be formatted as: "." or "msg-num unique-id".
         *
         * @param response The string returned from the server
         * @return true if the string parsed as expected (e.g. no syntax problems)
         */
        public boolean parseMultiLine(String response) {
            mErr = false;
            if (response == null || response.length() == 0) {
                return false;
            }
            char first = response.charAt(0);
            if (first == '.') {
                mEndOfMessage = true;
                return true;
            } else {
                String[] uidParts = response.split(" +");
                if (uidParts.length >= 2) {
                    try {
                        mMessageNumber = Integer.parseInt(uidParts[0]);
                    } catch (NumberFormatException nfe) {
                        return false;
                    }
                    mUniqueId = uidParts[1];
                    mEndOfMessage = false;
                    return true;
                }
            }
            return false;
        }
    }

    private void indexMessage(int msgNum, Pop3Message message) {
        mMsgNumToMsgMap.put(msgNum, message);
        mUidToMsgMap.put(message.getUid(), message);
        mUidToMsgNumMap.put(message.getUid(), msgNum);
    }

    @Override
    public Message[] getMessages(String[] uids, MessageRetrievalListener listener) {
        throw new UnsupportedOperationException(
                "Pop3Folder.getMessage(MessageRetrievalListener)");
    }

    /**
     * M: Add for IMAP partial download.
     */
    @Override
    public void fetch(Message[] messages, FetchProfile fp,
            MessageRetrievalListener listener, int fetchSize) throws MessagingException {
    }

    /**
     * Fetch the items contained in the FetchProfile into the given set of
     * Messages in as efficient a manner as possible.
     * @param messages
     * @param fp
     * @throws MessagingException
     */
    @Override
    public void fetch(Message[] messages, FetchProfile fp, MessageRetrievalListener listener)
            throws MessagingException {
        throw new UnsupportedOperationException(
                "Pop3Folder.fetch(Message[], FetchProfile, MessageRetrievalListener)");
    }

    /**
     * Fetches the body of the given message, limiting the stored data
     * to the specified number of lines. If lines is -1 the entire message
     * is fetched. This is implemented with RETR for lines = -1 or TOP
     * for any other value. If the server does not support TOP it is
     * emulated with RETR and extra lines are thrown away.
     *
     * @param message
     * @param lines
     * @param optional callback that reports progress of the fetch
     */
    // CHECKSTYLE:OFF
    public void fetchBody(Pop3Message message, int lines,
            EOLConvertingInputStream.Callback callback) throws IOException, MessagingException {
        String response = null;
        int messageId = mUidToMsgNumMap.get(message.getUid());

        /// M: we must obtain the message size here by LIST command. Cause not all server
        /// support return size by RETR command. @{
        String listResponse = mConnection.executeSimpleCommand(String.format(Locale.US, "LIST %d", messageId));
        try {
            // For null response, throw IOException.
            if (listResponse == null) {
                LogUtils.e(Logging.LOG_TAG,
                        ": >>> Error happened, LIST response is null");
                throw new IOException();
            }
            String[] listParts = listResponse.split(" ");
            // Normal response, "+OK 756 4538".
            // what if some server send strange response? throw IOException.
            if (listParts.length < 3) {
                LogUtils.e(Logging.LOG_TAG,
                        ": >>> Error happened, LIST command response [%s]", listResponse);
                throw new IOException();
            }
            int msgSize = Integer.parseInt(listParts[2]);
            message.setSize(msgSize);
        } catch (NumberFormatException nfe) {
            throw new IOException();
        }
        /// @}

        if (lines == -1) {
            // Fetch entire message
            response = mConnection.executeSimpleCommand(String.format(Locale.US, "RETR %d", messageId));
        } else {
            // Fetch partial message.  Try "TOP", and fall back to slower "RETR" if necessary
            try {
                response = mConnection.executeSimpleCommand(
                        String.format(Locale.US, "TOP %d %d", messageId,  lines));
            } catch (MessagingException me) {
                try {
                    response = mConnection.executeSimpleCommand(
                            String.format(Locale.US, "RETR %d", messageId));
                } catch (MessagingException e) {
                    LogUtils.w(Logging.LOG_TAG, e, "Can't read message %s, %s", messageId,
                            e.toString());
                    /// M: finally, if it failed while fetching message from server. Just abort the
                    /// fetching instead of saving null message to database.
                    throw e;
                }
            }
        }
        if (response != null)  {
            try {
                InputStream in = mConnection.getInputStream();
                if (Pop3Store.DEBUG_LOG_RAW_STREAM && DebugUtils.DEBUG) {
                    in = new LoggingInputStream(in);
                }
                message.parse(new Pop3ResponseInputStream(in), callback);
            } catch (MessagingException me) {
                /*
                 * If we're only downloading headers it's possible
                 * we'll get a broken MIME message which we're not
                 * real worried about. If we've downloaded the body
                 * and can't parse it we need to let the user know.
                 */
                if (lines == -1) {
                    throw me;
                }
            }
        }
    }
 // CHECKSTYLE:ON

    @Override
    public Flag[] getPermanentFlags() {
        return Pop3Store.PERMANENT_FLAGS;
    }

    @Override
    public void appendMessage(Context context, Message message, final boolean noTimeout) {
    }

    @Override
    public void delete(boolean recurse) {
    }

    /**
     * M: Expunge implementation.
     * Here we just re-open the connection to expunge.
     */
    @Override
    public Message[] expunge() throws MessagingException {
        if (mNeedExpungeConn) {
            close(true);
            open(OpenMode.READ_WRITE);
        }
        mNeedExpungeConn = false;
        return null;
    }

    public void deleteMessage(Message message) throws MessagingException {
        mOneMessage[0] = message;
        setFlags(mOneMessage, Pop3Store.PERMANENT_FLAGS, true);
    }

    // CHECKSTYLE:OFF
    @Override
    public void setFlags(Message[] messages, Flag[] flags, boolean value)
            throws MessagingException {
        if (!value || !Utility.arrayContains(flags, Flag.DELETED)) {
            /*
             * The only flagging we support is setting the Deleted flag.
             */
            return;
        }
        /// M: When we set flags, we should close connection to make it works actually when we close the folder
        mNeedExpungeConn = true;
        try {
            for (Message message : messages) {
                try {
                    String uid = message.getUid();
                    int msgNum = mUidToMsgNumMap.get(uid);
                    mConnection.executeSimpleCommand(String.format(Locale.US, "DELE %s", msgNum));
                    // Remove from the maps
                    mMsgNumToMsgMap.remove(msgNum);
                    mUidToMsgNumMap.remove(uid);
                } catch (MessagingException e) {
                    // A failed deletion isn't a problem
                }
            }
        } catch (IOException ioe) {
            mConnection.close();
            if (DebugUtils.DEBUG) {
                LogUtils.d(Logging.LOG_TAG, ioe.toString());
            }
            throw new MessagingException("setFlags()", ioe);
        }
    }
    // CHECKSTYLE:ON

    @Override
    public void copyMessages(Message[] msgs, Folder folder, MessageUpdateCallbacks callbacks) {
        throw new UnsupportedOperationException("copyMessages is not supported in POP3");
    }


    // CHECKSTYLE:OFF
    @Override
    public boolean equals(Object o) {
        if (o instanceof Pop3Folder) {
            return ((Pop3Folder) o).mName.equals(mName);
        }
        return super.equals(o);
    }
    // CHECKSTYLE:ON

    @Override
    @VisibleForTesting
    public boolean isOpen() {
        return (null != mConnection) && (mConnection.isOpen());
    }

    @Override
    public Message createMessage(String uid) {
        return new Pop3Message(uid, this);
    }

    @Override
    public Message[] getMessages(SearchParams params, MessageRetrievalListener listener) {
        return null;
    }

    public static class Pop3Message extends MimeMessage {
        public Pop3Message(String uid, Pop3Folder folder) {
            mUid = uid;
            mFolder = folder;
            mSize = -1;
        }

        public void setSize(int size) {
            mSize = size;
        }

        @Override
        public void parse(InputStream in) throws IOException, MessagingException {
            super.parse(in);
        }

        @Override
        public void setFlag(Flag flag, boolean set) throws MessagingException {
            super.setFlag(flag, set);
            mFolder.setFlags(new Message[] { this }, new Flag[] { flag }, set);
        }
    }

    // TODO figure out what is special about this and merge it into MailTransport
    class Pop3ResponseInputStream extends InputStream {
        private final InputStream mIn;
        private boolean mStartOfLine = true;
        private boolean mFinished;

        public Pop3ResponseInputStream(InputStream in) {
            mIn = in;
        }

        @Override
        public int read() throws IOException {
            if (mFinished) {
                return -1;
            }
            int d = mIn.read();
            if (mStartOfLine && d == '.') {
                d = mIn.read();
                if (d == '\r') {
                    mFinished = true;
                    mIn.read();
                    return -1;
                }
            }

            mStartOfLine = (d == '\n');

            return d;
        }
    }

}

