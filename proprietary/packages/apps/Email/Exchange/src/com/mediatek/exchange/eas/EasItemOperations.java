package com.mediatek.exchange.eas;

import android.content.ContentValues;
import android.content.Context;
import android.text.TextUtils;

import com.android.emailcommon.internet.MimeUtility;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.emailcommon.provider.EmailContent.MessageColumns;
import com.android.emailcommon.provider.HostAuth;
import com.android.emailcommon.provider.Mailbox;
import com.android.emailcommon.service.EmailServiceStatus;
import com.android.exchange.Eas;
import com.android.exchange.EasResponse;
import com.android.exchange.adapter.Parser;
import com.android.exchange.adapter.Serializer;
import com.android.exchange.adapter.Tags;
import com.android.exchange.service.EasServerConnection;
import com.android.mail.utils.LogUtils;

import com.mediatek.exchange.adapter.ItemOperationsFetchParser;

import org.apache.http.HttpStatus;

import java.io.IOException;
import java.security.cert.CertificateException;

/**
 * Loads attachments from the Exchange server.
 * TODO: Add ability to call back to UI when this failed, and generally better handle error cases.
 */
public class EasItemOperations extends EasServerConnection {
    private static final String TAG = Eas.LOG_TAG;

    private static final String CMD = "ItemOperations";
    private EasItemOperations(final Context context, final Account account, final HostAuth hostAuth) {
        super(context, account, hostAuth);
    }

    /**
     * M: Added for Exchange Partial download request
     * @param messageId the Id of email to be completely fetched
     */
    public static void fetchMessage(final Context context, final long messageId) {
        final Message msg = Message.restoreMessageWithId(context, messageId);
        if (msg == null) {
            // TODO: Update Message load status or something to notify UI
            LogUtils.d(TAG, "Could not fetch message %d", messageId);
            return;
        }
        long accountId = msg.mAccountKey;
        final Account account = Account.restoreAccountWithId(context, accountId);
        if (account == null) {
            // TODO: Update Message load status or something to notify UI
            LogUtils.d(TAG, "Message %d has bad account key %d", msg.mId,
                    accountId);
            return;
        }
        final HostAuth hostAuth = HostAuth.restoreHostAuthWithId(context, account.mHostAuthKeyRecv);
        if (hostAuth == null) {
            // TODO: Update Message load status or something to notify UI
            LogUtils.d(TAG, "HostAuth is null");
            return;
        }
        // Error cases handled, do the operations.
        final EasItemOperations operations =
                new EasItemOperations(context, account, hostAuth);
        // TODO: Do error handling here ?
        operations.finishFetchMessage(msg, operations.fetch(msg));
    }

    private int fetch(Message message) {
        final EasResponse resp = performServerRequest(message);
        int result = EmailServiceStatus.SUCCESS;
        if (resp == null) {
            return EmailServiceStatus.CONNECTION_ERROR;
        }

        try {
            if (resp.getStatus() != HttpStatus.SC_OK || resp.isEmpty()) {
                return EmailServiceStatus.MESSAGE_NOT_FOUND;
            }
            result = handleResponse(resp, message);
        } catch (IOException e) {
            // M: TODO: Do error handling
            LogUtils.d(TAG, "fetch %d IOException: %s", message.mId, e);
            e.printStackTrace();
        } finally {
            resp.close();
        }
        return result;
    }

    /**
     * Read the {@link EasResponse} and extract the message data, saving it to the provider.
     * @param resp The (successful) {@link EasResponse} containing the message data.
     * @param message The {@link Message} with the message metadata.
     * @return A status code, from {@link EmailServiceStatus}, for this fetch.
     * @throws IOException
     */
    private int handleResponse(EasResponse resp, Message message) throws IOException {
        final Account account = Account.restoreAccountWithId(mContext, message.mAccountKey);
        if (account == null) {
            // TODO: Update Message load status or something to notify UI
            LogUtils.d(TAG, "Message %d has bad account key %d", message.mId,
                    message.mAccountKey);
            return EmailServiceStatus.ACCOUNT_UNINITIALIZED;
        }
        final Mailbox mailbox = Mailbox.restoreMailboxWithId(mContext, message.mMailboxKey);
        if (mailbox == null) {
            // TODO: Update Message load status or something to notify UI
            LogUtils.d(TAG, "Message %d has bad mailbox key %d", message.mId,
                    message.mMailboxKey);
            return EmailServiceStatus.MESSAGE_NOT_FOUND;
        }
        final ItemOperationsFetchParser parser = new ItemOperationsFetchParser(mContext, mContext.getContentResolver(),
                resp.getInputStream(), account, mailbox, message);
        try {
            parser.parse();
        } catch (final Parser.EmptyStreamException e) {
            // This indicates a compressed response which was empty, which is OK.
        }
        return 0;
    }

    private EasResponse performServerRequest(Message message) {
        try {
            // The method of attachment loading is different in EAS 14.0 than in earlier versions
            final byte[] bytes;
            if (getProtocolVersion() >= Eas.SUPPORTED_PROTOCOL_EX2007_DOUBLE) {
                final Serializer s = new Serializer();
                s.start(Tags.ITEMS_ITEMS).start(Tags.ITEMS_FETCH)
                        .data(Tags.ITEMS_STORE, "Mailbox");
                // If this is a search result, use the protocolSearchInfo field
                // to get the
                // correct remote location
                if (!TextUtils.isEmpty(message.mProtocolSearchInfo)) {
                    LogUtils.d(TAG,
                            "Fetch remote searched message: "
                                    + message.mProtocolSearchInfo);
                    s.data(Tags.SEARCH_LONG_ID, message.mProtocolSearchInfo);
                } else {
                    LogUtils.d(TAG, "Fetch local messages");
                    Mailbox mailbox = Mailbox.restoreMailboxWithId(mContext, message.mMailboxKey);
                    if (mailbox == null) {
                     // TODO: Update Message load status or something to notify UI
                        LogUtils.d(TAG, "Message %d has bad mailbox key %d", message.mId,
                                message.mMailboxKey);
                        return null;
                    }
                    s.data(Tags.SYNC_COLLECTION_ID, mailbox.mServerId).data(
                            Tags.SYNC_SERVER_ID, message.mServerId);
                }

                s.start(Tags.ITEMS_OPTIONS);
                if (getProtocolVersion() >= Eas.SUPPORTED_PROTOCOL_EX2007_DOUBLE) {
                    s.start(Tags.BASE_BODY_PREFERENCE);
                    s.data(Tags.BASE_TYPE, Eas.BODY_PREFERENCE_HTML);
                    // / M: limited the body fetch size to avoid database or
                    // binder buffer OOM.
                    s.data(Tags.BASE_TRUNCATION_SIZE,
                            String.valueOf(MimeUtility.FETCH_BODY_SIZE_LIMIT));
                    s.end();
                    LogUtils.d(TAG,
                            "Add Sync commands options for EX2007");
                } else {
                    s.data(Tags.SYNC_TRUNCATION, Eas.EAS2_5_TRUNCATION_SIZE);
                }
                s.end();
                s.end().end().done();
                bytes = s.toByteArray();
            } else {
                LogUtils.d(TAG,
                        "ItemOperations is not supported by EAS 2.5");
                // For Exchange 2003 (EAS 2.5), it is not supported
                bytes = null;
            }
            return sendHttpClientPost(CMD, bytes);
        } catch (final IOException e) {
            LogUtils.w(TAG, "IOException while fetching message from server: %s", e.getMessage());
            return null;
        } catch (CertificateException e) {
            ///M: Added for catch CeritifcationExcetion
            LogUtils.w(TAG, "CertificateException while ItemOperation fetch message from server: %s",
                    e.getMessage());
            return null;
        }
    }

    /**
     * M: Save sync status for notifying UI
     */
    private void finishFetchMessage(final Message message, final int status) {
        Message msg = message;
        // Only fetch failure need to be handled
        if (status != EmailServiceStatus.SUCCESS) {
            ContentValues values = new ContentValues();
            int flags = msg.changeMessageStateFlags(Message.FLAG_LOAD_STATUS_FAILED);
            values.put(MessageColumns.FLAGS, flags);
            msg.update(mContext, values);
            LogUtils.w(TAG, "Fetch message failed with error status: " + status);
        }
    }
}
