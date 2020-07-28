package com.mediatek.email.mail.store;

import com.android.email.DebugUtils;
import com.android.email.mail.transport.MailTransport;
import com.android.emailcommon.Logging;
import com.android.emailcommon.mail.AuthenticationFailedException;
import com.android.emailcommon.mail.MessagingException;
import com.android.mail.utils.LogUtils;

import java.io.IOException;
import java.io.InputStream;

/**
 * M: This class stands for a pop3 connection, which could be used by a pop3 folder to do sync/network
 * actions.
 */
public class Pop3Connection {
    // The transport for this connection
    private MailTransport mTransport;
    // The Store
    private Pop3Store mPop3Store;
    // The username/password
    private String mUsername;
    private String mPassword;
    private Pop3Capabilities mCapabilities;

    public Pop3Connection(Pop3Store store, String username, String password) {
        setStore(store, username, password);
    }

    void setStore(Pop3Store store, String username, String password) {
        if (username != null && password != null) {
            mUsername = username;
            mPassword = password;
        }
        mPop3Store = store;
    }

    void open() throws MessagingException {
        if (null != mTransport && mTransport.isOpen()) {
            return;
        }

        try {
            // copy configuration into a clean transport, if necessary
            if (mTransport == null) {
                mTransport = mPop3Store.cloneTransport();
            }
            mTransport.open();

            // Eat the banner
            executeSimpleCommand(null);

            mCapabilities = getCapabilities();

            if (mTransport.canTryTlsSecurity()) {
                if (mCapabilities.stls) {
                    executeSimpleCommand("STLS");
                    mTransport.reopenTls();
                } else {
                    if (DebugUtils.DEBUG) {
                        LogUtils.d(Logging.LOG_TAG, "TLS not supported but required");
                    }
                    throw new MessagingException(MessagingException.TLS_REQUIRED);
                }
            }

            try {
                executeSensitiveCommand("USER " + mUsername, "USER /redacted/");
                executeSensitiveCommand("PASS " + mPassword, "PASS /redacted/");
            } catch (MessagingException me) {
                if (DebugUtils.DEBUG) {
                    LogUtils.d(Logging.LOG_TAG, me.toString());
                }
                throw new AuthenticationFailedException(null, me);
            }
        } catch (IOException ioe) {
            mTransport.close();
            if (DebugUtils.DEBUG) {
                LogUtils.d(Logging.LOG_TAG, ioe.toString());
            }
            throw new MessagingException(MessagingException.IOERROR, ioe.toString());
        }

    }

    public boolean isOpen() {
        return (null != mTransport) && (mTransport.isOpen());
    }

    /// CHECKSTYLE:OFF
    private Pop3Capabilities getCapabilities() throws IOException {
        Pop3Capabilities capabilities = new Pop3Capabilities();
        try {
            String response = executeSimpleCommand("CAPA");
            while ((response = mTransport.readLine(true)) != null) {
                if (response.equals(".")) {
                    break;
                } else if (response.equalsIgnoreCase("STLS")) {
                    capabilities.stls = true;
                }
            }
        } catch (MessagingException me) {
            /*
             * The server may not support the CAPA command, so we just eat this Exception
             * and allow the empty capabilities object to be returned.
             */
        }
        return capabilities;
    }
    // CHECKSTYLE:ON

    /**
     * Send a single command and wait for a single line response.  Reopens the connection,
     * if it is closed.  Leaves the connection open.
     *
     * @param command The command string to send to the server.
     * @return Returns the response string from the server.
     */
    public String executeSimpleCommand(String command) throws IOException, MessagingException {
        return executeSensitiveCommand(command, null);
    }

    /**
     * Send a single command and wait for a single line response.  Reopens the connection,
     * if it is closed.  Leaves the connection open.
     *
     * @param command The command string to send to the server.
     * @param sensitiveReplacement If the command includes sensitive data (e.g. authentication)
     * please pass a replacement string here (for logging).
     * @return Returns the response string from the server.
     */
    private String executeSensitiveCommand(String command, String sensitiveReplacement)
            throws IOException, MessagingException {
        open();

        if (command != null) {
            mTransport.writeLine(command, sensitiveReplacement);
        }

        String response = mTransport.readLine(true);

        if (response.length() > 1 && response.charAt(0) == '-') {
            throw new MessagingException(response);
        }

        return response;
    }

    /**
     * Reads a single line from the server, using either \r\n or \n as the delimiter.  The
     * delimiter char(s) are not included in the result.
     */
    String readLine(boolean loggable) throws IOException {
        return mTransport.readLine(loggable);
    }

    public InputStream getInputStream() {
        return mTransport.getInputStream();
    }

    /**
     * Closes the connection and releases all resources. This connection can not be used again
     * until {@link #setStore(ImapStore, String, String)} is called.
     */
    // CHECKSTYLE:OFF
    void close() {
        try {
            executeSimpleCommand("QUIT");
        } catch (Exception e) {
            // ignore any problems here - just continue closing
        }
        if (mTransport != null) {
            mTransport.close();
            mTransport = null;
        }
        mPop3Store = null;
    }
    // CHECKSTYLE:ON

    /**
     * POP3 Capabilities as defined in RFC 2449.  This is not a complete list of CAPA
     * responses - just those that we use in this client.
     */
    class Pop3Capabilities {
        /** The STLS (start TLS) command is supported */
        public boolean stls;

        @Override
        public String toString() {
            return String.format("STLS %b", stls);
        }
    }
}
