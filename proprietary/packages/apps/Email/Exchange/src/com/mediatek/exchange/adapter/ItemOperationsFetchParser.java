/* Copyright (C) 2011 The Android Open Source Project.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.exchange.adapter;

import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.os.RemoteException;

import com.android.emailcommon.internet.MimeUtility;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.Body;
import com.android.emailcommon.provider.EmailContent.BodyColumns;
import com.android.emailcommon.provider.Mailbox;
import com.android.emailcommon.utility.HtmlConverter;
import com.android.emailcommon.utility.StringCompressor;
import com.android.emailcommon.utility.Utility;
import com.android.exchange.Eas;
import com.android.exchange.adapter.Parser;
import com.android.exchange.adapter.Tags;
import com.android.mail.utils.LogUtils;
import com.mediatek.emailcommon.EmailFeatureOptions;


import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

/**
 * Parse the result of an ItemOperations command; we use this to parse message in EAS 12.0+
 */
public class ItemOperationsFetchParser extends Parser {

    private static final String TAG = Eas.LOG_TAG;

    private int mStatusCode = 0;

    protected EmailContent.Message mMessage;
    protected Mailbox mMailbox;
    protected Account mAccount;
    protected Context mContext;
    protected ContentResolver mContentResolver;

    public ItemOperationsFetchParser(Context context, ContentResolver contentResolver,
            final InputStream in, Account account, Mailbox mailbox, EmailContent.Message message) throws IOException {
        super(in);
        init(context, contentResolver, account, mailbox, message);
    }

    private void init(final Context context, final ContentResolver resolver, final Account account,
            final Mailbox mailbox, final EmailContent.Message message) {
        mContext = context;
        mContentResolver = resolver;
        mMessage = message;
        mMailbox = mailbox;
        mAccount = account;
    }

    public int getStatusCode() {
        return mStatusCode;
    }

    private void bodyParser(EmailContent.Message msg) throws IOException {
        String bodyType = Eas.BODY_PREFERENCE_TEXT;
        String body = "";
        /// M: change tag from EAMIL_BODY to BASE_BODY
        while (nextTag(Tags.BASE_BODY) != END) {
            LogUtils.d(TAG, "ITEMS_PROPERTIES parseProperties BODY PARSER: " + tag);
            switch (tag) {
                case Tags.BASE_TYPE:
                    bodyType = getValue();
                    break;
                case Tags.BASE_DATA:
                    body = getValue();
                    LogUtils.d(TAG, "_____________ Fetched body length: " + body.length());
                    break;
                case Tags.BASE_TRUNCATED:
                    // Message is partial loaded if AirSyncBaseBody::Truncated is True;
                    // Otherwise message is complete loaded if False
                    String bodyTruncated = getValue();
                    if ("1".equals(bodyTruncated) || "true".equals(bodyTruncated)) {
                        msg.mFlagLoaded = EmailContent.Message.FLAG_LOADED_PARTIAL;
                    } else {
                        msg.mFlagLoaded = EmailContent.Message.FLAG_LOADED_COMPLETE;
                    }
                    LogUtils.d(TAG, "_____________ Fetch for EAS 12+ body truncated: " + bodyTruncated);
                    break;
                default:
                    skipTag();
            }
        }
        // We always ask for TEXT or HTML; there's no third option
        if (bodyType.equals(Eas.BODY_PREFERENCE_HTML)) {
            msg.mHtml = body;
            /** M: Generate the text for supporting local search. @{*/
            if (body != null) {
                msg.mText = HtmlConverter.htmlToText(body);
            }
            /** @} */
        } else {
            msg.mText = body;
            /// M: Generate the html contents, cause it's neccessary for setting msg.mFlagLoaded @{
            msg.mHtml = body;
            /// @}
        }
    }

    public void contentParser(EmailContent.Message msg, int endingTag) throws IOException {
        while (nextTag(endingTag) != END) {
            switch (tag) {
                case Tags.BASE_BODY:
                    LogUtils.d(TAG, "ITEMS_PROPERTIES parseProperties BASE_BODY");
                    bodyParser(msg);
                    break;
                default:
                    skipTag();
            }
        }
    }

    private void parseProperties() throws IOException {
        EmailContent.Message msg = mMessage;
        int bodySize = (msg.mHtml != null) ? msg.mHtml.length() : 0;
        LogUtils.d(TAG, "ITEMS_PROPERTIES parseProperties message: " + msg);
        // Parse Email Content
        contentParser(msg, tag);

        /** M: Sometimes the server would not send "TRUNCATED = false" to indicate
         * the body is download completed. We should calculate it by the body size.
         * Set load flag to FLAG_LOADED_COMPLETE when it is FLAG_LOADED_PARTIAL and
         * the body increased but not exceed the FETCH_BODY_SIZE_LIMIT @{ */
        int msgSize = (msg.mHtml != null) ? msg.mHtml.length() : 0;
        if (msg.mFlagLoaded == EmailContent.Message.FLAG_LOADED_PARTIAL
                && bodySize < msgSize
                && msgSize < MimeUtility.FETCH_BODY_SIZE_LIMIT) {
            LogUtils.d(TAG, "Fetched body completely but no TRUNCATED flag recevied");
            msg.mFlagLoaded = EmailContent.Message.FLAG_LOADED_COMPLETE;
        }
        /** @} */
        /**
         *  if the message's flagLoaded was still FLAG_LOADED_PARTIAL, that indicated
         *  this message's body was to large
         */
        if (msg.mFlagLoaded == EmailContent.Message.FLAG_LOADED_PARTIAL) {
            LogUtils.d(TAG, "Exchange Fetch messageId: %d is too large", msg.mId);
            msg.mFlags |= EmailContent.Message.FLAG_BODY_TOO_LARGE;
            msg.mFlagLoaded = EmailContent.Message.FLAG_LOADED_COMPLETE;
        }

        if (msg.mFlagLoaded == EmailContent.Message.FLAG_LOADED_COMPLETE) {
            msg.changeMessageStateFlags(EmailContent.Message.FLAG_LOAD_STATUS_SUCCESS);
        } else {
            // Just reset loaded flags if still Partial loaded, basically not happened
            msg.changeMessageStateFlags(EmailContent.Message.FLAG_LOAD_STATUS_NONE);
        }
        LogUtils.d(TAG, "Fetch messageId: %d with flagLoaded: %d" , msg.mId, msg.mFlagLoaded);
        // Update message's body, flags and flagLoaded
        final ArrayList<ContentProviderOperation> ops =
                new ArrayList<ContentProviderOperation>();
        // Create and save the body
        ContentValues cv = new ContentValues();

        /**
         * M: [LBE] Large body enhancement, major changes:
         * 1. Check if fetched mail body larger than @MimeUtility.LRGE_BODY_SIZE.
         * 2. Write body file instead of ContentValues.
         * 3. Note that, skip update body table since default update body table
         *    focus write file and notify UI refresh.
         *    The message update will notify UI refresh too.
         **/
        boolean isLargBody = false;
        int length = 0;
        if (msg.mText != null) {
            length += msg.mText.length();
        }
        if (msg.mHtml != null) {
            length += msg.mHtml.length();
        }
        if (length > MimeUtility.NEED_COMPRESS_BODY_SIZE) {
            if (EmailFeatureOptions.LAEGE_BODY_ENHANCEMENT) {
                // Write body file if msg.mHtml or msg.mText is not null, to avoid NPE.
                if (msg.mHtml != null) {
                    Utility.writeBodyFile(mContentResolver, msg.mId, true, msg.mHtml);
                }
                if (msg.mText != null) {
                    Utility.writeBodyFile(mContentResolver, msg.mId, false, msg.mText);
                }
                LogUtils.d(TAG, "[LBE] Exchange write message [%d] body file sucessfully!",
                        msg.mId);
            } else {
                if (msg.mText != null) {
                    cv.put(BodyColumns.TEXT_CONTENT, StringCompressor.compressToBytes(msg.mText));
                }
                if (msg.mHtml != null) {
                    cv.put(BodyColumns.HTML_CONTENT, StringCompressor.compressToBytes(msg.mHtml));
                }
                msg.mFlags |= EmailContent.Message.FLAG_BODY_COMPRESSED;
                LogUtils.d(TAG, "Exchange Fetch messageId: %d is compressed", msg.mId);
            }
            isLargBody = true;
        } else {
            if (msg.mText != null) {
                cv.put(BodyColumns.TEXT_CONTENT, msg.mText);
            }
            if (msg.mHtml != null) {
                cv.put(BodyColumns.HTML_CONTENT, msg.mHtml);
            }
        }
        /** @} */
        String[] msgKey = new String[1];
        msgKey[0] = String.valueOf(msg.mId);
        /**
         * M: [LBE] Large body enhancement. if
         * @EmailFeatureOptions.LAEGE_BODY_ENHANCEMENT true, ignore update body
         * because has write body file. @{
         **/
        if (isLargBody) {
            if (!EmailFeatureOptions.LAEGE_BODY_ENHANCEMENT) {
                ops.add(ContentProviderOperation.newUpdate(Body.CONTENT_LARGE_URI)
                   .withSelection(BodyColumns.MESSAGE_KEY + "=?", msgKey)
                   .withValues(cv).build());
            }
        } else {
            ops.add(ContentProviderOperation.newUpdate(Body.CONTENT_URI)
                    .withSelection(BodyColumns.MESSAGE_KEY + "=?", msgKey)
                    .withValues(cv).build());
        }
        /** @} */
        // commit fetched mails via EAS here to trigger MessageView reload
        ops.add(ContentProviderOperation.newUpdate(ContentUris.withAppendedId(
                EmailContent.Message.CONTENT_URI, msg.mId))
                .withValue(EmailContent.MessageColumns.FLAGS, msg.mFlags)
                .withValue(EmailContent.MessageColumns.FLAG_LOADED, msg.mFlagLoaded)
                .build());
        try {
            ContentProviderResult[] results = mContentResolver.applyBatch(EmailContent.AUTHORITY, ops);
            LogUtils.d(TAG, "ITEMS_FETCH Save successfully for " + ops.size()
                     + " operations.");
            for (ContentProviderResult result : results) {
                LogUtils.d(TAG, "ITEMS_FETCH Save successfully: " + result.toString());
            }
        } catch (RemoteException e) {
            LogUtils.d(TAG, "RemoteException while saving search results.");
        } catch (OperationApplicationException e) {
            LogUtils.d(TAG, "OperationApplicationException while saving search results.");
        }
    }

    private void parseFetch() throws IOException {
        while (nextTag(Tags.ITEMS_FETCH) != END) {
            if (tag == Tags.ITEMS_STATUS) {
                String status = getValue();
                LogUtils.d(TAG, "ITEMS_STATUS:" + status);
            } else if (tag == Tags.SYNC_SERVER_ID) {
                String serverId = getValue();
                LogUtils.d(TAG, "ITEMOPERATIONS_FETCH SERVER_ID: " + serverId);
            } else if (tag == Tags.SEARCH_LONG_ID) {
                String protocolSearchInfo = getValue();
                LogUtils.d(TAG, "ITEMOPERATIONS_FETCH LONGID: " + protocolSearchInfo);
            } else if (tag == Tags.ITEMS_PROPERTIES) {
                LogUtils.d(TAG, "ITEMS_PROPERTIES");
                parseProperties();
            } else {
                skipTag();
            }
        }
    }

    private void parseResponse() throws IOException {
        while (nextTag(Tags.ITEMS_RESPONSE) != END) {
            if (tag == Tags.ITEMS_FETCH) {
                parseFetch();
            } else {
                skipTag();
            }
        }
    }

    @Override
    public boolean parse() throws IOException {
        boolean res = false;
        if (nextTag(START_DOCUMENT) != Tags.ITEMS_ITEMS) {
            throw new IOException();
        }
        while (nextTag(START_DOCUMENT) != END_DOCUMENT) {
            if (tag == Tags.ITEMS_STATUS) {
                // Save the status code
                mStatusCode = getValueInt();
            } else if (tag == Tags.ITEMS_RESPONSE) {
                parseResponse();
            } else {
                skipTag();
            }
        }
        return res;
    }
}
