/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008-2009 Marc Blank
 * Licensed to The Android Open Source Project.
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

package com.android.exchange.adapter;

import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.net.Uri;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.TransactionTooLargeException;

import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.MailboxColumns;
import com.android.emailcommon.provider.Mailbox;
import com.android.exchange.CommandStatusException;
import com.android.exchange.Exchange;
import com.android.exchange.ExchangePreferences;
import com.android.exchange.CommandStatusException.CommandStatus;
import com.android.exchange.adapter.AbstractSyncAdapter.Operation;
import com.android.exchange.Eas;
import com.android.mail.utils.LogUtils;
import com.google.common.annotations.VisibleForTesting;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

/**
 * Base class for the Email and PIM sync parsers
 * Handles the basic flow of syncKeys, looping to get more data, handling errors, etc.
 * Each subclass must implement a handful of methods that relate specifically to the data type
 *
 */
public abstract class AbstractSyncParser extends Parser {
    private static final String TAG = Eas.LOG_TAG;

    protected Mailbox mMailbox;
    protected Account mAccount;
    protected Context mContext;
    protected ContentResolver mContentResolver;

    private boolean mLooping;

    public AbstractSyncParser(final Context context, final ContentResolver resolver,
            final InputStream in, final Mailbox mailbox, final Account account) throws IOException {
        super(in);
        init(context, resolver, mailbox, account);
    }

    public AbstractSyncParser(InputStream in, AbstractSyncAdapter adapter) throws IOException {
        super(in);
        init(adapter);
    }

    public AbstractSyncParser(Parser p, AbstractSyncAdapter adapter) throws IOException {
        super(p);
        init(adapter);
    }

    public AbstractSyncParser(final Parser p, final Context context, final ContentResolver resolver,
        final Mailbox mailbox, final Account account) throws IOException {
        super(p);
        init(context, resolver, mailbox, account);
    }

    private void init(final AbstractSyncAdapter adapter) {
        init(adapter.mContext, adapter.mContext.getContentResolver(), adapter.mMailbox,
                adapter.mAccount);
    }

    private void init(final Context context, final ContentResolver resolver, final Mailbox mailbox,
            final Account account) {
        mContext = context;
        mContentResolver = resolver;
        mMailbox = mailbox;
        mAccount = account;
    }

    /**
     * Read, parse, and act on incoming commands from the Exchange server
     * @throws IOException if the connection is broken
     * @throws CommandStatusException
     */
    public abstract void commandsParser() throws IOException, CommandStatusException;

    /**
     * Read, parse, and act on server responses
     * @throws IOException
     */
    public abstract void responsesParser() throws IOException;

    /**
     * Commit any changes found during parsing
     * @throws IOException
     */
    public abstract void commit() throws IOException, RemoteException,
            OperationApplicationException;

    public boolean isLooping() {
        return mLooping;
    }

    /**
     * Skip through tags until we reach the specified end tag
     * @param endTag the tag we end with
     * @throws IOException
     */
    public void skipParser(int endTag) throws IOException {
        while (nextTag(endTag) != END) {
            skipTag();
        }
    }

    /**
     * Loop through the top-level structure coming from the Exchange server
     * Sync keys and the more available flag are handled here, whereas specific data parsing
     * is handled by abstract methods implemented for each data class (e.g. Email, Contacts, etc.)
     * @throws CommandStatusException
     */
    @Override
    public boolean parse() throws IOException, CommandStatusException {
        int status;
        boolean moreAvailable = false;
        boolean newSyncKey = false;
        mLooping = false;
        // If we're not at the top of the xml tree, throw an exception
        if (nextTag(START_DOCUMENT) != Tags.SYNC_SYNC) {
            throw new EasParserException();
        }

        boolean mailboxUpdated = false;
        ContentValues cv = new ContentValues();

        // Loop here through the remaining xml
        while (nextTag(START_DOCUMENT) != END_DOCUMENT) {
            if (tag == Tags.SYNC_COLLECTION || tag == Tags.SYNC_COLLECTIONS) {
                // Ignore these tags, since we've only got one collection syncing in this loop
            } else if (tag == Tags.SYNC_STATUS) {
                // Status = 1 is success; everything else is a failure
                status = getValueInt();
                if (status != 1) {
                    if (status == 3 || CommandStatus.isBadSyncKey(status)) {
                        // Must delete all of the data and start over with syncKey of "0"
                        mMailbox.mSyncKey = "0";
                        newSyncKey = true;

                        /** M: Store bad sync key mailbox id in Preference in case of user rebooting the device
                            or Exchange process crashes during the recovery, or else we would loss the BSK status.
                            We ought to know there is still an unfinished bad sync key recovery and have to
                            go on finish it. We do not deal with Calendar & Contacts folders with the recovery
                            process at present @{ */
                        if (mMailbox.mType < Mailbox.TYPE_NOT_EMAIL) {
                            LogUtils.i(Eas.BSK_TAG, "Bad sync key occurs");
                            Exchange.sBadSyncKeyMailboxId = mMailbox.mId;
                            ExchangePreferences pref = ExchangePreferences.getPreferences(mContext);
                            pref.setBadSyncKeyMailboxId(mMailbox.mId);
                        }
                        /** @} */
                        wipe();
                        // Indicate there's more so that we'll start syncing again
                        moreAvailable = true;
                    } else if (status == 16 || status == 5) {
                        // Status 16 indicates a transient server error (indeterminate state)
                        // Status 5 indicates "server error"; this tends to loop for a while so
                        // throwing IOException will at least provide backoff behavior
                        throw new IOException();
                    } else if (status == 8) {
                        // Status 8 is Bad; it means the server doesn't recognize the serverId it
                        // sent us.
                        // We don't have any provision for telling the user "wait a minute while
                        // we sync folders"...
                        throw new IOException();
                    } else if (status == 12) {
                        // 12 means that we're being asked to refresh the folder list.
                        // We'll do that with 8 also...
                        // TODO: reloadFolderList simply sets all mailboxes to hold.
                        //ExchangeService.reloadFolderList(mContext, mAccount.mId, true);
                        /// M: We do a folder sync immediately right here. @{
                        final Bundle extras = new Bundle();
                        extras.putBoolean(Mailbox.SYNC_EXTRA_ACCOUNT_ONLY, true);
                        extras.putBoolean(ContentResolver.SYNC_EXTRAS_EXPEDITED, true);
                        ContentResolver.requestSync(new android.accounts.Account(
                                mAccount.mEmailAddress, Eas.EXCHANGE_ACCOUNT_MANAGER_TYPE),
                                EmailContent.AUTHORITY, extras);
                        /// @}
                    } else if (status == 7) {
                        // TODO: Fix this. The handling here used to be pretty bogus, and it's not
                        // obvious that simply forcing another resync makes sense here.
                        moreAvailable = true;
                    } else {
                        LogUtils.e(LogUtils.TAG, "Sync: Unknown status: " + status);
                        // Access, provisioning, transient, etc.
                        throw new CommandStatusException(status);
                    }
                }
            } else if (tag == Tags.SYNC_COMMANDS) {
                commandsParser();
            } else if (tag == Tags.SYNC_RESPONSES) {
                responsesParser();
            } else if (tag == Tags.SYNC_MORE_AVAILABLE) {
                moreAvailable = true;
            } else if (tag == Tags.SYNC_SYNC_KEY) {
                if (mMailbox.mSyncKey.equals("0")) {
                    moreAvailable = true;
                }
                String newKey = getValue();
                userLog("Parsed key for ", mMailbox.mDisplayName, ": ", newKey);
                if (!newKey.equals(mMailbox.mSyncKey)) {
                    mMailbox.mSyncKey = newKey;
                    cv.put(MailboxColumns.SYNC_KEY, newKey);
                    mailboxUpdated = true;
                    newSyncKey = true;
                }
           } else {
                skipTag();
           }
        }

        // If we don't have a new sync key, ignore moreAvailable (or we'll loop)
        if (moreAvailable && !newSyncKey) {
            LogUtils.e(TAG, "Looping detected");
            mLooping = true;
        }

        // Commit any changes
        try {
            commit();
            if (mailboxUpdated) {
                mMailbox.update(mContext, cv);
            }
        } catch (RemoteException e) {
            LogUtils.e(TAG, "Failed to commit changes", e);
        } catch (OperationApplicationException e) {
            LogUtils.e(TAG, "Failed to commit changes", e);
        }
        // Let the caller know that there's more to do
        if (moreAvailable) {
            userLog("MoreAvailable");
        }
        return moreAvailable;
    }

    abstract protected void wipe();

    void userLog(String ...strings) {
        // TODO: Convert to other logging types?
        //mService.userLog(strings);
    }

    void userLog(String string, int num, String string2) {
        // TODO: Convert to other logging types?
        //mService.userLog(string, num, string2);
    }

    /**
     * M: The following function came from CalendarSyncParser, move here just
     * only for re-used them on other place, such as ContactsSyncParser.
     *
     * @{
     *
     */

    /**
     * We apply the batch of CPO's here.  We synchronize on the service to avoid thread-nasties,
     * and we just return quickly if the service has already been stopped.
     */
    private static ContentProviderResult[] execute(final ContentResolver contentResolver,
            final String authority, final ArrayList<ContentProviderOperation> ops)
            throws RemoteException, OperationApplicationException {
        if (!ops.isEmpty()) {
            ContentProviderResult[] result = contentResolver.applyBatch(authority, ops);
            //mService.userLog("Results: " + result.length);
            return result;
        }
        return new ContentProviderResult[0];
    }

    /**
     * Convert an Operation to a CPO; if the Operation has a back reference, apply it with the
     * passed-in offset
     */
    @VisibleForTesting
    static ContentProviderOperation operationToContentProviderOperation(Operation op, int offset) {
        if (op.mOp != null) {
            return op.mOp;
        } else if (op.mBuilder == null) {
            throw new IllegalArgumentException("Operation must have CPO.Builder");
        }
        ContentProviderOperation.Builder builder = op.mBuilder;
        if (op.mColumnName != null) {
            builder.withValueBackReference(op.mColumnName, op.mOffset - offset);
        }
        return builder.build();
    }

    /**
     * Create a list of CPOs from a list of Operations, and then apply them in a batch
     */
    private static ContentProviderResult[] applyBatch(final ContentResolver contentResolver,
            final String authority, final ArrayList<Operation> ops, final int offset)
            throws RemoteException, OperationApplicationException {
        // Handle the empty case
        if (ops.isEmpty()) {
            return new ContentProviderResult[0];
        }
        ArrayList<ContentProviderOperation> cpos = new ArrayList<ContentProviderOperation>();
        for (Operation op: ops) {
            cpos.add(operationToContentProviderOperation(op, offset));
        }
        return execute(contentResolver, authority, cpos);
    }

    /**
     * Apply the list of CPO's in the provider and copy the "mini" result into our full result array
     */
    private static void applyAndCopyResults(final ContentResolver contentResolver,
            final String authority, final ArrayList<Operation> mini,
            final ContentProviderResult[] result, final int offset) throws RemoteException {
        // Empty lists are ok; we just ignore them
        if (mini.isEmpty()) return;
        try {
            ContentProviderResult[] miniResult = applyBatch(contentResolver, authority, mini,
                    offset);
            // Copy the results from this mini-batch into our results array
            System.arraycopy(miniResult, 0, result, offset, miniResult.length);
        } catch (OperationApplicationException e) {
            // Not possible since we're building the ops ourselves
        }
    }

    /**
     * Called by a sync adapter to execute a list of Operations in the ContentProvider handling
     * the passed-in authority.  If the attempt to apply the batch fails due to a too-large
     * binder transaction, we split the Operations as directed by separators.  If any of the
     * "mini" batches fails due to a too-large transaction, we're screwed, but this would be
     * vanishingly rare.  Other, possibly transient, errors are handled by throwing a
     * RemoteException, which the caller will likely re-throw as an IOException so that the sync
     * can be attempted again.
     *
     * Callers MAY leave a dangling separator at the end of the list; note that the separators
     * themselves are only markers and are not sent to the provider.
     */
    protected static ContentProviderResult[] safeExecute(final ContentResolver contentResolver,
            final String authority, final ArrayList<Operation> ops) throws RemoteException {
        //mService.userLog("Try to execute ", ops.size(), " CPO's for " + authority);
        ContentProviderResult[] result = null;
        try {
            // Try to execute the whole thing
            return applyBatch(contentResolver, authority, ops, 0);
        } catch (TransactionTooLargeException e) {
            // Nope; split into smaller chunks, demarcated by the separator operation
            //mService.userLog("Transaction too large; spliting!");
            ArrayList<Operation> mini = new ArrayList<Operation>();
            // Build a result array with the total size we're sending
            result = new ContentProviderResult[ops.size()];
            int count = 0;
            int offset = 0;
            for (Operation op: ops) {
                if (op.mSeparator) {
                    //mService.userLog("Try mini-batch of ", mini.size(), " CPO's");
                    applyAndCopyResults(contentResolver, authority, mini, result, offset);
                    mini.clear();
                    // Save away the offset here; this will need to be subtracted out of the
                    // value originally set by the adapter
                    offset = count + 1; // Remember to add 1 for the separator!
                } else {
                    mini.add(op);
                }
                count++;
            }
            // Check out what's left; if it's more than just a separator, apply the batch
            int miniSize = mini.size();
            if ((miniSize > 0) && !(miniSize == 1 && mini.get(0).mSeparator)) {
                applyAndCopyResults(contentResolver, authority, mini, result, offset);
            }
        } catch (RemoteException e) {
            throw e;
        } catch (OperationApplicationException e) {
            // Not possible since we're building the ops ourselves
            LogUtils.e(TAG, "problem inserting %s during server update", authority, e);
        }
        return result;
    }

    protected static void addSeparatorOperation(ArrayList<Operation> ops, Uri uri) {

    }

    /** @} */
}
