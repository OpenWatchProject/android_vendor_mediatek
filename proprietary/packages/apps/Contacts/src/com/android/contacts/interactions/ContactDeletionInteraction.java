/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.contacts.interactions;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.LoaderManager;
import android.app.LoaderManager.LoaderCallbacks;
import android.content.Context;
import android.content.CursorLoader;
import android.content.DialogInterface;
import android.content.DialogInterface.OnShowListener;
import android.content.Intent;
import android.content.DialogInterface.OnDismissListener;
import android.content.Loader;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Contacts.Entity;
import android.text.TextUtils;
//import android.util.Log;
import android.widget.Toast;

import com.android.contacts.ContactSaveService;
import com.android.contacts.R;
import com.android.contacts.model.AccountTypeManager;
import com.android.contacts.model.account.AccountType;
import com.android.contacts.preference.ContactsPreferences;
import com.android.contacts.util.ContactDisplayUtils;
import com.android.internal.telephony.PhoneConstants;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.ContactsConstants;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.simservice.SimDeleteProcessor;
import com.mediatek.provider.MtkContactsContract;

import java.util.HashSet;
import java.util.List;

import com.mediatek.contacts.eventhandler.BaseEventHandlerFragment;
import com.mediatek.contacts.eventhandler.GeneralEventHandler;
import com.mediatek.contacts.interactions.ContactDeletionInteractionUtils;

/**
 * An interaction invoked to delete a contact.
 */
 /* M: [Sim Contact Flow] support delete sim contact.
  * extends BaseEventHandlerFragment insteads of Fragment,
  * implements SimDeleteProcessor.Listener.
  * M: [Google Issue][ALPS04211444] add OnShowListener. 1/4 @{ */
public class ContactDeletionInteraction extends BaseEventHandlerFragment
        implements LoaderCallbacks<Cursor>, OnDismissListener, OnShowListener,
        SimDeleteProcessor.Listener { /* @} */

    private static final String TAG = "ContactDeletion";
    private static final String FRAGMENT_TAG = "deleteContact";

    private static final String KEY_ACTIVE = "active";
    private static final String KEY_CONTACT_URI = "contactUri";
    private static final String KEY_FINISH_WHEN_DONE = "finishWhenDone";
    public static final String ARG_CONTACT_URI = "contactUri";
    public static final int RESULT_CODE_DELETED = 3;

    /* M: [Sim Contact Flow] Add some columns for sim contact.
     * Original code: @{
    private static final String[] ENTITY_PROJECTION = new String[] {
        Entity.RAW_CONTACT_ID, //0
        Entity.ACCOUNT_TYPE, //1
        Entity.DATA_SET, // 2
        Entity.CONTACT_ID, // 3
        Entity.LOOKUP_KEY, // 4
        Entity.DISPLAY_NAME, // 5
        Entity.DISPLAY_NAME_ALTERNATIVE, // 6
    };
     * @}
     * New code: @{ */
    private static final String[] ENTITY_PROJECTION;
    private static final String[] ENTITY_PROJECTION_INTERNAL = new String[] {
        Entity.RAW_CONTACT_ID, //0
        Entity.ACCOUNT_TYPE, //1
        Entity.DATA_SET, // 2
        Entity.CONTACT_ID, // 3
        Entity.LOOKUP_KEY, // 4
        Entity.DISPLAY_NAME, // 5
        Entity.DISPLAY_NAME_ALTERNATIVE, // 6
    };
    static {
        List<String> projectionList = Lists.newArrayList(ENTITY_PROJECTION_INTERNAL);
        if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
            projectionList.add(MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM); //7
            projectionList.add(MtkContactsContract.RawContactsColumns.INDEX_IN_SIM);         //8
        }
        ENTITY_PROJECTION = projectionList.toArray(new String[projectionList.size()]);
    }
    /// @}

    private static final int COLUMN_INDEX_RAW_CONTACT_ID = 0;
    private static final int COLUMN_INDEX_ACCOUNT_TYPE = 1;
    private static final int COLUMN_INDEX_DATA_SET = 2;
    private static final int COLUMN_INDEX_CONTACT_ID = 3;
    private static final int COLUMN_INDEX_LOOKUP_KEY = 4;
    private static final int COLUMN_INDEX_DISPLAY_NAME = 5;
    private static final int COLUMN_INDEX_DISPLAY_NAME_ALT = 6;
    /// M: [Sim Contact Flow] Add some columns for sim contact. @{
    private static final int COLUMN_INDEX_INDICATE_PHONE_SIM = 7;
    private static final int COLUMN_INDEX_IN_SIM = 8;
    /// @}

    private boolean mActive;
    private Uri mContactUri;
    private String mDisplayName;
    private String mDisplayNameAlt;
    private boolean mFinishActivityWhenDone;
    private Context mContext;
    private AlertDialog mDialog;

    /** This is a wrapper around the fragment's loader manager to be used only during testing. */
    private TestLoaderManagerBase mTestLoaderManager;

    @VisibleForTesting
    int mMessageId;

    /**
     * Starts the interaction.
     *
     * @param activity the activity within which to start the interaction
     * @param contactUri the URI of the contact to delete
     * @param finishActivityWhenDone whether to finish the activity upon completion of the
     *        interaction
     * @return the newly created interaction
     */
    public static ContactDeletionInteraction start(
            Activity activity, Uri contactUri, boolean finishActivityWhenDone) {
        Log.d(FRAGMENT_TAG, "[start] contactUri=" + contactUri);
        return startWithTestLoaderManager(activity, contactUri, finishActivityWhenDone, null);
    }

    /**
     * Starts the interaction and optionally set up a {@link TestLoaderManagerBase}.
     *
     * @param activity the activity within which to start the interaction
     * @param contactUri the URI of the contact to delete
     * @param finishActivityWhenDone whether to finish the activity upon completion of the
     *        interaction
     * @param testLoaderManager the {@link TestLoaderManagerBase} to use to load the data, may be null
     *        in which case the default {@link LoaderManager} is used
     * @return the newly created interaction
     */
    @VisibleForTesting
    static ContactDeletionInteraction startWithTestLoaderManager(
            Activity activity, Uri contactUri, boolean finishActivityWhenDone,
            TestLoaderManagerBase testLoaderManager) {
        if (contactUri == null || activity.isDestroyed()) {
            return null;
        }

        FragmentManager fragmentManager = activity.getFragmentManager();
        ContactDeletionInteraction fragment =
                (ContactDeletionInteraction) fragmentManager.findFragmentByTag(FRAGMENT_TAG);
        if (fragment == null) {
            fragment = new ContactDeletionInteraction();
            fragment.setTestLoaderManager(testLoaderManager);
            fragment.setContactUri(contactUri);
            fragment.setFinishActivityWhenDone(finishActivityWhenDone);
            fragmentManager.beginTransaction().add(fragment, FRAGMENT_TAG)
                    .commitAllowingStateLoss();
        } else {
            fragment.setTestLoaderManager(testLoaderManager);
            fragment.setContactUri(contactUri);
            fragment.setFinishActivityWhenDone(finishActivityWhenDone);
        }
        /* M: [Sim Contact Flow] add for sim contact @ { */
        fragment.mSimUri = null;
        fragment.mSimIndex = -1;
        /* @ } */
        return fragment;
    }

    @Override
    public LoaderManager getLoaderManager() {
        // Return the TestLoaderManager if one is set up.
        LoaderManager loaderManager = super.getLoaderManager();
        if (mTestLoaderManager != null) {
            // Set the delegate: this operation is idempotent, so let's just do it every time.
            mTestLoaderManager.setDelegate(loaderManager);
            return mTestLoaderManager;
        } else {
            return loaderManager;
        }
    }

    /** Sets the TestLoaderManager that is used to wrap the actual LoaderManager in tests. */
    private void setTestLoaderManager(TestLoaderManagerBase mockLoaderManager) {
        mTestLoaderManager = mockLoaderManager;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        mContext = activity;
        /// M: [Sim Contact Flow] Add for SIM Service refactory @{
        SimDeleteProcessor.registerListener(this);
        /// @}
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        if (mDialog != null && mDialog.isShowing()) {
            mDialog.setOnDismissListener(null);
            mDialog.dismiss();
            mDialog = null;
        }
        /// M: [Sim Contact Flow] Add for SIM Service refactory @{
        SimDeleteProcessor.unregisterListener(this);
        /// @}
    }

    public void setContactUri(Uri contactUri) {
        mContactUri = contactUri;
        mActive = true;
        if (isStarted()) {
            Bundle args = new Bundle();
            args.putParcelable(ARG_CONTACT_URI, mContactUri);
            getLoaderManager().restartLoader(R.id.dialog_delete_contact_loader_id, args, this);
        }
    }

    private void setFinishActivityWhenDone(boolean finishActivityWhenDone) {
        this.mFinishActivityWhenDone = finishActivityWhenDone;

    }

    /* Visible for testing */
    boolean isStarted() {
        return isAdded();
    }

    @Override
    public void onStart() {
        if (mActive) {
            Bundle args = new Bundle();
            args.putParcelable(ARG_CONTACT_URI, mContactUri);
            getLoaderManager().initLoader(R.id.dialog_delete_contact_loader_id, args, this);
        }
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
        if (mDialog != null) {
            mDialog.hide();
        }
    }

    @Override
    public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        Uri contactUri = args.getParcelable(ARG_CONTACT_URI);
        return new CursorLoader(mContext,
                Uri.withAppendedPath(contactUri, Entity.CONTENT_DIRECTORY), ENTITY_PROJECTION,
                null, null, null);
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor cursor) {
        if (mDialog != null) {
            mDialog.dismiss();
            mDialog = null;
        }

        if (!mActive) {
            return;
        }

        if (cursor == null || cursor.isClosed()) {
            Log.e(TAG, "Failed to load contacts");
            return;
        }

        long contactId = 0;
        String lookupKey = null;

        // This cursor may contain duplicate raw contacts, so we need to de-dupe them first
        HashSet<Long>  readOnlyRawContacts = Sets.newHashSet();
        HashSet<Long>  writableRawContacts = Sets.newHashSet();

        AccountTypeManager accountTypes = AccountTypeManager.getInstance(getActivity());
        cursor.moveToPosition(-1);
        while (cursor.moveToNext()) {
            final long rawContactId = cursor.getLong(COLUMN_INDEX_RAW_CONTACT_ID);
            final String accountType = cursor.getString(COLUMN_INDEX_ACCOUNT_TYPE);
            final String dataSet = cursor.getString(COLUMN_INDEX_DATA_SET);
            contactId = cursor.getLong(COLUMN_INDEX_CONTACT_ID);
            lookupKey = cursor.getString(COLUMN_INDEX_LOOKUP_KEY);
            mDisplayName = cursor.getString(COLUMN_INDEX_DISPLAY_NAME);
            mDisplayNameAlt = cursor.getString(COLUMN_INDEX_DISPLAY_NAME_ALT);
            /// M: [Sim Contact Flow] @{
            if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                mSubId = cursor.getInt(COLUMN_INDEX_INDICATE_PHONE_SIM);
                mSimIndex = cursor.getInt(COLUMN_INDEX_IN_SIM);
                mSimUri = SubInfoUtils.getIccProviderUri(mSubId);
            }
            /// @}
            AccountType type = accountTypes.getAccountType(accountType, dataSet);
            boolean writable = type == null || type.areContactsWritable();
            if (writable) {
                writableRawContacts.add(rawContactId);
            } else {
                readOnlyRawContacts.add(rawContactId);
            }
        }
        if (TextUtils.isEmpty(lookupKey)) {
            Log.e(TAG, "Failed to find contact lookup key");
            getActivity().finish();
            return;
        }

        int readOnlyCount = readOnlyRawContacts.size();
        int writableCount = writableRawContacts.size();
        int positiveButtonId = android.R.string.ok;
        if (readOnlyCount > 0 && writableCount > 0) {
            mMessageId = R.string.readOnlyContactDeleteConfirmation;
        } else if (readOnlyCount > 0 && writableCount == 0) {
            mMessageId = R.string.readOnlyContactWarning;
            positiveButtonId = R.string.readOnlyContactWarning_positive_button;
        } else if (readOnlyCount == 0 && writableCount > 1) {
            mMessageId = R.string.multipleContactDeleteConfirmation;
            positiveButtonId = R.string.deleteConfirmation_positive_button;
        } else {
            mMessageId = R.string.deleteConfirmation;
            positiveButtonId = R.string.deleteConfirmation_positive_button;
        }

        /// M: [ALPS02619600] Forbid user to delete the read only account contacts in Contacts AP.
        //  The delete flow is not suit for these accounts.
        // ToDo:
        // Change the dialog message, because we can't make it in-visible or delete it clearly. @{
        if (readOnlyCount > 0) {
            showReadonlyDialog();
        } else { /// @}
            final Uri contactUri = Contacts.getLookupUri(contactId, lookupKey);
            showDialog(mMessageId, positiveButtonId, contactUri);
        }

        // We don't want onLoadFinished() calls any more, which may come when the database is
        // updating.
        getLoaderManager().destroyLoader(R.id.dialog_delete_contact_loader_id);
    }

    @Override
    public void onLoaderReset(Loader<Cursor> loader) {
    }

    private void showDialog(int messageId, int positiveButtonId, final Uri contactUri) {
        mDialog = new AlertDialog.Builder(getActivity())
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(messageId)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(positiveButtonId,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int whichButton) {
                            doDeleteContact(contactUri);
                        }
                    }
                )
                .create();

        mDialog.setOnDismissListener(this);
        /// M: [Google Issue][ALPS04211444] 3/4 @{
        mDialog.setOnShowListener(this);
        /// @}
        mDialog.show();
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        mActive = false;
        mDialog = null;
    }

    /// M: [Google Issue][ALPS04211444]mActive is true which represents dialog is shown up. 2/4 @{
    @Override
    public void onShow(DialogInterface dialog) {
        mActive = true;
    }
    /// @}

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putBoolean(KEY_ACTIVE, mActive);
        outState.putParcelable(KEY_CONTACT_URI, mContactUri);
        /** M: [Sim Contact Flow] to save sim_uri and sim_index to delete @{ */
        outState.putParcelable(KEY_CONTACT_SIM_URI, mSimUri);
        outState.putInt(KEY_CONTACT_SIM_INDEX, mSimIndex);
        outState.putInt(KEY_CONTACT_SUB_ID, mSubId);
        /**@}*/
        outState.putBoolean(KEY_FINISH_WHEN_DONE, mFinishActivityWhenDone);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        if (savedInstanceState != null) {
            mActive = savedInstanceState.getBoolean(KEY_ACTIVE);
            mContactUri = savedInstanceState.getParcelable(KEY_CONTACT_URI);
            /** M: [Sim Contact Flow] to get sim_uri and sim_index to delete @{ */
            mSimUri = savedInstanceState.getParcelable(KEY_CONTACT_SIM_URI);
            mSimIndex = savedInstanceState.getInt(KEY_CONTACT_SIM_INDEX);
            mSubId = savedInstanceState.getInt(KEY_CONTACT_SUB_ID);
            /**@}*/
            mFinishActivityWhenDone = savedInstanceState.getBoolean(KEY_FINISH_WHEN_DONE);
        }
    }

    protected void doDeleteContact(Uri contactUri) {
        /* M: [Sim Contact Flow] Add for SIM Contact @{ */
        if (!isAdded() || ContactDeletionInteractionUtils.doDeleteSimContact(
                mContext, contactUri, mSimUri, mSimIndex, mSubId, this)) {
            return;
        }
        /* @} */

        mContext.startService(ContactSaveService.createDeleteContactIntent(mContext, contactUri));
        if (isAdded() && mFinishActivityWhenDone) {
            Log.d(FRAGMENT_TAG, "[doDeleteContact] finished");
            getActivity().setResult(RESULT_CODE_DELETED);
            getActivity().finish();
            final String deleteToastMessage;
            final String name = ContactDisplayUtils.getPreferredDisplayName(mDisplayName,
                    mDisplayNameAlt, new ContactsPreferences(mContext));
            if (TextUtils.isEmpty(name)) {
                deleteToastMessage = getResources().getQuantityString(
                        R.plurals.contacts_deleted_toast, /* quantity */ 1);
            } else {
                deleteToastMessage = getResources().getString(
                        R.string.contacts_deleted_one_named_toast, name);
            }
            Toast.makeText(mContext, deleteToastMessage, Toast.LENGTH_LONG).show();
        }
    }

    /** M: [Sim Contact Flow] below code is for sim contact @{ */

    private Uri mSimUri = null;
    private int mSimIndex = -1;
    private int mSubId = SubInfoUtils.getInvalidSubId();
    private static final String KEY_CONTACT_SIM_URI = "contact_sim_uri";
    private static final String KEY_CONTACT_SIM_INDEX = "contact_sim_index";
    private static final String KEY_CONTACT_SUB_ID = "contact_sub_id";

    @Override
    public void onSIMDeleteFailed() {
        if (isAdded()) {
            getActivity().finish();
        }
        return;
    }

    @Override
    public void onSIMDeleteCompleted() {
        if (isAdded() && mFinishActivityWhenDone) {
            getActivity().setResult(RESULT_CODE_DELETED);
            /// M: Fix JE: Conn't show toast in non UI thread. @{
            final String deleteToastMessage = getResources().getQuantityString(R.plurals
                    .contacts_deleted_toast, /* quantity */ 1);
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(mContext, deleteToastMessage, Toast.LENGTH_LONG).show();
                }
            });
            getActivity().finish();
            /// @}
        }
        return;
    }

    /**
     * M: refactor phb state change
     */
    @Override
    public void onReceiveEvent(String eventType, Intent extraData) {
        int stateChangeSubId = extraData.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                ContactsConstants.ERROR_SUB_ID);
        Log.i(TAG , "[onReceiveEvent] eventType: " + eventType
                + ", extraData: " + extraData.toString()
                + ",stateChangeSubId: " + stateChangeSubId
                + ",mSubId: " + mSubId);
        if (GeneralEventHandler.EventType.PHB_STATE_CHANGE_EVENT.equals(eventType)
                && (mSubId == stateChangeSubId)) {
            Log.i(TAG, "[onReceiveEvent] phb state change,finish EditorActivity ");
            getActivity().setResult(RESULT_CODE_DELETED);
            getActivity().finish();
            return;
        }
    }

    /**
     * M: add for alert the read only contact can not be delete in Contact APP.
     */
    private void showReadonlyDialog() {
        mDialog = new AlertDialog.Builder(getActivity())
                .setTitle(R.string.deleteConfirmation_title)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(R.string.readOnlyContactWarning)
                .setPositiveButton(android.R.string.ok, null).create();
        mDialog.setOnDismissListener(this);
        /// M: [Google Issue][ALPS04211444] 4/4 @{
        mDialog.setOnShowListener(this);
        /// @}
        mDialog.show();
    }

    /** M: [Sim Contact Flow] above code is for sim contact @} */
}
