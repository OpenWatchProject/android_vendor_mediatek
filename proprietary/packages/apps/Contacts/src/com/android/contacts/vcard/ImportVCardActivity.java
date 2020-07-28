/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.android.contacts.vcard;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.ProgressDialog;
import android.content.ClipData;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.PowerManager;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import android.text.format.DateUtils;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.style.RelativeSizeSpan;
//import android.util.Log;
import android.widget.Button;
import android.widget.Toast;

import com.android.contacts.R;
import com.android.contacts.activities.RequestImportVCardPermissionsActivity;
import com.android.contacts.model.AccountTypeManager;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contactsbind.FeedbackHelper;
import com.android.vcard.VCardEntryCounter;
import com.android.vcard.VCardParser;
import com.android.vcard.VCardParser_V21;
import com.android.vcard.VCardParser_V30;
import com.android.vcard.VCardSourceDetector;
import com.android.vcard.exception.VCardException;
import com.android.vcard.exception.VCardNestedException;
import com.android.vcard.exception.VCardVersionException;

import com.mediatek.contacts.eventhandler.BaseEventHandlerActivity;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.ImportExportUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.MtkToast;
import com.mediatek.contacts.util.VcardUtils;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.channels.Channels;
import java.nio.channels.ReadableByteChannel;
import java.nio.channels.WritableByteChannel;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Vector;

/**
 * The class letting users to import vCard. This includes the UI part for letting them select
 * an Account and posssibly a file if there's no Uri is given from its caller Activity.
 *
 * Note that this Activity assumes that the instance is a "one-shot Activity", which will be
 * finished (with the method {@link Activity#finish()}) after the import and never reuse
 * any Dialog in the instance. So this code is careless about the management around managed
 * dialogs stuffs (like how onCreateDialog() is used).
 */
public class ImportVCardActivity extends BaseEventHandlerActivity
                                 implements ImportVCardDialogFragment.Listener {
    private static final String LOG_TAG = "VCardImport";

    private static final int SELECT_ACCOUNT = 0;

    /// M: [MTK IMPORT/EXPORT] @{
    /* package */ static final String VCARD_URI_ARRAY = "vcard_uri";
    /* package */ static final String ESTIMATED_VCARD_TYPE_ARRAY = "estimated_vcard_type";
    /* package */ static final String ESTIMATED_CHARSET_ARRAY = "estimated_charset";
    /* package */ static final String VCARD_VERSION_ARRAY = "vcard_version";
    /* package */ static final String ENTRY_COUNT_ARRAY = "entry_count";

    private static final String SECURE_DIRECTORY_NAME = ".android_secure";
    /// @}

    /* package */ final static int VCARD_VERSION_AUTO_DETECT = 0;
    /* package */ final static int VCARD_VERSION_V21 = 1;
    /* package */ final static int VCARD_VERSION_V30 = 2;

    private static final int REQUEST_OPEN_DOCUMENT = 100;

    /**
     * Notification id used when error happened before sending an import request to VCardServer.
     */
    private static final int FAILURE_NOTIFICATION_ID = 1;

    private static final String LOCAL_TMP_FILE_NAME_EXTRA =
            "com.android.contacts.vcard.LOCAL_TMP_FILE_NAME";

    private static final String SOURCE_URI_DISPLAY_NAME =
            "com.android.contacts.vcard.SOURCE_URI_DISPLAY_NAME";

    private static final String STORAGE_VCARD_URI_PREFIX = "file:///storage";

    private AccountWithDataSet mAccount;

    private ProgressDialog mProgressDialogForCachingVCard;

    private VCardCacheThread mVCardCacheThread;
    private ImportRequestConnection mConnection;
    /* package */ VCardImportExportListener mListener;

    private String mErrorMessage;

    private Handler mHandler = new Handler();

    /// M: [MTK IMPORT/EXPORT] @{
    private ProgressDialog mProgressDialogForScanVCard;
    private List<VCardFile> mAllVCardFileList;
    private VCardScanThread mVCardScanThread;
    private boolean mIsScanThreadStarted;
    // For Vcard file select dialog.
    private AlertDialog mVcardFileSelectDialog;

    private static class VCardFile {
        private final String mName;
        private final String mCanonicalPath;
        private final long mLastModified;

        public VCardFile(String name, String canonicalPath, long lastModified) {
            mName = name;
            mCanonicalPath = canonicalPath;
            mLastModified = lastModified;
        }

        public String getName() {
            return mName;
        }

        public String getCanonicalPath() {
            return mCanonicalPath;
        }

        public long getLastModified() {
            return mLastModified;
        }
    }
    /// @}


    // Runs on the UI thread.
    private class DialogDisplayer implements Runnable {
        private final int mResId;
        public DialogDisplayer(int resId) {
            mResId = resId;
        }
        public DialogDisplayer(String errorMessage) {
            mResId = R.id.dialog_error_with_message;
            mErrorMessage = errorMessage;
        }
        @Override
        public void run() {
            if (!isFinishing()) {
                showDialog(mResId);
            }
        }
    }

    private class CancelListener
        implements DialogInterface.OnClickListener, DialogInterface.OnCancelListener {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            /// M: Bug Fix CR ID: ALPS00110214 @{
            setResult(Activity.RESULT_CANCELED);
            /// @}
            finish();
        }
        @Override
        public void onCancel(DialogInterface dialog) {
            /// M: Bug Fix CR ID: ALPS00110214 @{
            setResult(Activity.RESULT_CANCELED);
            /// @}
            finish();
        }
    }

    private CancelListener mCancelListener = new CancelListener();

    private class ImportRequestConnection implements ServiceConnection {
        private VCardService mService;

        public void sendImportRequest(final List<ImportRequest> requests) {
            Log.i(LOG_TAG, "Send an import request");
            mService.handleImportRequest(requests, mListener);
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            mService = ((VCardService.MyBinder) binder).getService();
            /// M: Error handling, Activity killed might cause mVCardCacheThread un-inited. @{
            if (mVCardCacheThread == null) {
                Log.e(LOG_TAG,
                        "[onServiceConnected]mVCardCacheThread is null, some error happens.");
                VcardUtils.showErrorInfo(R.string.vcard_import_request_rejected_message,
                        ImportVCardActivity.this);
                return;
            }
            /// @}
            Log.i(LOG_TAG,
                    String.format("Connected to VCardService. Kick a vCard cache thread (uri: %s)",
                            Arrays.toString(mVCardCacheThread.getSourceUris())));
            mVCardCacheThread.start();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.i(LOG_TAG, "Disconnected from VCardService");
        }

        /// M: [MTK IMPORT/EXPORT] @{
        /**
         * M: @return true if the service has been binded,false else.
         */
        public boolean isServiceBinded() {
            return mService != null;
        }

        /**
         * M: ALPS01022668 first vcard import finished, second vcard still
         * caching, can't stop service in this case.
         */
        public void setVCardCaching(boolean cache) {
            Log.d(LOG_TAG, "[setVCardCaching] cache:" + cache);
            mService.mCaching = cache;
        }
        /// @}
    }

    /**
     * Caches given vCard files into a local directory, and sends actual import request to
     * {@link VCardService}.
     *
     * We need to cache given files into local storage. One of reasons is that some data (as Uri)
     * may have special permissions. Callers may allow only this Activity to access that content,
     * not what this Activity launched (like {@link VCardService}).
     */
    private class VCardCacheThread extends Thread
            implements DialogInterface.OnCancelListener {
        private boolean mCanceled;
        private PowerManager.WakeLock mWakeLock;
        private VCardParser mVCardParser;
        ///M: [Google Issue][ALPS03298275] add for import multi vcf file @{
        private List<VCardFile> mVCardFileList = null;
        private Uri[] mSourceUris;  // Given from a caller.
        private String[] mSourceDisplayNames; // Display names for each Uri in mSourceUris.
        /// @}
        private final byte[] mSource;
        private final String mDisplayName;

        public VCardCacheThread(final Uri[] sourceUris, String[] sourceDisplayNames) {
            mSourceUris = sourceUris;
            mSourceDisplayNames = sourceDisplayNames;
            mSource = null;
            final Context context = ImportVCardActivity.this;
            final PowerManager powerManager =
                    (PowerManager)context.getSystemService(Context.POWER_SERVICE);
            mWakeLock = powerManager.newWakeLock(
                    PowerManager.SCREEN_DIM_WAKE_LOCK |
                    PowerManager.ON_AFTER_RELEASE, LOG_TAG);
            mDisplayName = null;
        }

        ///M: [Google Issue][ALPS03298275] add for import multi vcf file @{
        public VCardCacheThread(final List<VCardFile> selectedVCardFileList) {
            mVCardFileList = selectedVCardFileList;
            mSource = null;
            final Context context = ImportVCardActivity.this;
            final PowerManager powerManager =
                    (PowerManager)context.getSystemService(Context.POWER_SERVICE);
            mWakeLock = powerManager.newWakeLock(
                    PowerManager.SCREEN_DIM_WAKE_LOCK |
                    PowerManager.ON_AFTER_RELEASE, LOG_TAG);
            mDisplayName = null;
        }
        /// @}

        @Override
        public void finalize() {
            if (mWakeLock != null && mWakeLock.isHeld()) {
                Log.w(LOG_TAG, "WakeLock is being held.");
                mWakeLock.release();
            }
        }

        @Override
        public void run() {
            Log.i(LOG_TAG, "vCard cache thread starts running.");
            if (mConnection == null) {
                throw new NullPointerException("vCard cache thread must be launched "
                        + "after a service connection is established");
            }

            mWakeLock.acquire();
            try {
                /// M: ALPS01022668 first vcard import finished,
                //  second vcard still caching, can't stop service in this case. @{
                mConnection.setVCardCaching(true);
                /// @}
                if (mCanceled == true) {
                    Log.i(LOG_TAG, "vCard cache operation is canceled.");
                    return;
                }

                final Context context = ImportVCardActivity.this;
                // Uris given from caller applications may not be opened twice: consider when
                // it is not from local storage (e.g. "file:///...") but from some special
                // provider (e.g. "content://...").
                // Thus we have to once copy the content of Uri into local storage, and read
                // it after it.
                //
                // We may be able to read content of each vCard file during copying them
                // to local storage, but currently vCard code does not allow us to do so.
                int cache_index = 0;
                ArrayList<ImportRequest> requests = new ArrayList<ImportRequest>();
                if (mSource != null) {
                    try {
                        requests.add(constructImportRequest(mSource, null, mDisplayName));
                    } catch (VCardException e) {
                        FeedbackHelper.sendFeedback(ImportVCardActivity.this, LOG_TAG,
                                "Failed to cache vcard", e);
                        Log.e(LOG_TAG, "Maybe the file is in wrong format", e);
                        showFailureNotification(R.string.fail_reason_not_supported);
                        return;
                    }
                } else {
                    ///M: [Google Issue][ALPS03298275] add for import multi vcf file @{
                    if (mSourceUris == null && mVCardFileList != null) {
                        final int size = mVCardFileList.size();
                        Log.d(LOG_TAG, "parse from file list. size=" + size);
                        final Uri[] srcUri = new Uri[size];
                        mSourceUris = new Uri[size];
                        mSourceDisplayNames = new String[size];
                        int index = 0;
                        for (VCardFile vcardFile : mVCardFileList) {
                            srcUri[index] = Uri.parse(
                                    getEncodeUriString(vcardFile.getCanonicalPath()));
                            mSourceUris[index] = readUriToLocalUri(srcUri[index]);
                            mSourceDisplayNames[index] = getDisplayName(srcUri[index]);
                            if ((mCanceled == true) || (mSourceUris[index] == null)) {
                                Log.i(LOG_TAG, "vCard cache break: mCanceled=" + mCanceled +
                                        ", uri=" + Log.anonymize(mSourceUris[index]));
                                for (final String fileName : fileList()) {
                                    if (fileName.startsWith(VCardService.CACHE_FILE_PREFIX)) {
                                        deleteFile(fileName);
                                    }
                                }
                                return;
                            }
                            index++;
                            if (index == Integer.MAX_VALUE) {
                                throw new RuntimeException("Exceeded cache limit");
                            }
                        }
                        Log.e(LOG_TAG, "parse from file list done. size=" + index);
                    }
                    /// @}
                    int i = 0;
                    for (Uri sourceUri : mSourceUris) {
                        if (mCanceled) {
                            Log.i(LOG_TAG, "vCard cache operation is canceled.");
                            break;
                        }

                        String sourceDisplayName = mSourceDisplayNames[i++];

                        final ImportRequest request;
                        try {
                            request = constructImportRequest(null, sourceUri, sourceDisplayName);
                        } catch (VCardException e) {
                            FeedbackHelper.sendFeedback(ImportVCardActivity.this, LOG_TAG,
                                    "Failed to cache vcard", e);
                            /**
                             * M: Fixed cr ALPS00598462
                             * Ori code:
                             *     showFailureNotification(R.string.fail_reason_not_supported);
                             *     return;
                             * new code: @{ */
                            String reason = getString(R.string.fail_reason_not_supported);
                            VcardUtils.showFailureNotification(ImportVCardActivity.this, reason ,
                                    sourceDisplayName , cache_index, mHandler);
                            continue;
                            /** @} */
                        /// Bug fix ALPS00318987 @{
                        } catch (IllegalArgumentException e) {
                            FeedbackHelper.sendFeedback(ImportVCardActivity.this, LOG_TAG,
                                    "Failed to cache vcard", e);
                            String reason = getString(R.string.fail_reason_not_supported);
                            VcardUtils.showFailureNotification(ImportVCardActivity.this, reason ,
                                    sourceDisplayName , cache_index, mHandler);
                            continue;
                        /// @}
                        } catch (IOException e) {
                            FeedbackHelper.sendFeedback(ImportVCardActivity.this, LOG_TAG,
                                    "Failed to cache vcard", e);
                            /**
                             * M: Fixed cr ALPS00598462
                             * Ori code:
                             *     showFailureNotification(R.string.fail_reason_io_error);
                             *     return;
                             * new code: @{ */
                            String reason = getString(R.string.fail_reason_io_error);
                            VcardUtils.showFailureNotification(ImportVCardActivity.this, reason ,
                                    sourceDisplayName , cache_index, mHandler);
                            continue;
                            /** @} */
                        }
                        if (mCanceled) {
                            Log.i(LOG_TAG, "vCard cache operation is canceled.");
                            return;
                        }
                        requests.add(request);
                    }
                }
                if (!requests.isEmpty()) {
                    mConnection.sendImportRequest(requests);
                } else {
                    Log.w(LOG_TAG, "Empty import requests. Ignore it.");
                }
            } catch (OutOfMemoryError e) {
                FeedbackHelper.sendFeedback(ImportVCardActivity.this, LOG_TAG,
                        "OutOfMemoryError occured during caching vCard", e);
                System.gc();
                runOnUiThread(new DialogDisplayer(
                        getString(R.string.fail_reason_low_memory_during_import)));
            /// M: Fix ALPS00874343,if the vcf file is not exist,toast @{
            } catch (FileNotFoundException e) {
                Log.w(LOG_TAG, "[run] the vcf file is not found when import! exception:" + e);
                VcardUtils.showErrorInfo(R.string.vcard_import_failed,
                        ImportVCardActivity.this);
            /// @}
            } catch (IOException e) {
                FeedbackHelper.sendFeedback(ImportVCardActivity.this, LOG_TAG,
                        "IOException during caching vCard", e);
                runOnUiThread(new DialogDisplayer(
                        getString(R.string.fail_reason_io_error)));
            } finally {
                Log.i(LOG_TAG, "Finished caching vCard.");
                /// M: ALPS01022668 first vcard import finished,
                //  second vcard still caching, can't stop service in this case. @{
                mConnection.setVCardCaching(false);
                /// @}
                mWakeLock.release();
                /// M: @{
                if (!ImportVCardActivity.this.isFinishing() && (null != mConnection) &&
                        mConnection.isServiceBinded()) {
                /// @}
                    try {
                       unbindService(mConnection);
                       mConnection = null;
                    } catch (IllegalArgumentException e) {
                        FeedbackHelper.sendFeedback(ImportVCardActivity.this, LOG_TAG,
                                "Cannot unbind service connection", e);
                    }
                /// M: @{
                } else {
                    Log.d(LOG_TAG, "in VcardCacheThread, Run(), mConnection==null !!! ");
                }
                /// @}
                mProgressDialogForCachingVCard.dismiss();
                mProgressDialogForCachingVCard = null;
                finish();
            }
        }

        /**
         * Reads localDataUri (possibly multiple times) and constructs {@link ImportRequest} from
         * its content.
         *
         * @arg localDataUri Uri actually used for the import. Should be stored in
         * app local storage, as we cannot guarantee other types of Uris can be read
         * multiple times. This variable populates {@link ImportRequest#uri}.
         * @arg displayName Used for displaying information to the user. This variable populates
         * {@link ImportRequest#displayName}.
         */
        private ImportRequest constructImportRequest(final byte[] data,
                final Uri localDataUri, final String displayName)
                throws IOException, VCardException {
            final ContentResolver resolver = ImportVCardActivity.this.getContentResolver();
            VCardEntryCounter counter = null;
            VCardSourceDetector detector = null;
            int vcardVersion = VCARD_VERSION_V21;
            try {
                boolean shouldUseV30 = false;
                InputStream is;
                if (data != null) {
                    is = new ByteArrayInputStream(data);
                } else {
                    is = resolver.openInputStream(localDataUri);
                }
                mVCardParser = new VCardParser_V21();
                try {
                    counter = new VCardEntryCounter();
                    detector = new VCardSourceDetector();
                    mVCardParser.addInterpreter(counter);
                    mVCardParser.addInterpreter(detector);
                    mVCardParser.parse(is);
                } catch (VCardVersionException e1) {
                    try {
                        is.close();
                    } catch (IOException e) {
                    }

                    shouldUseV30 = true;
                    if (data != null) {
                        is = new ByteArrayInputStream(data);
                    } else {
                        is = resolver.openInputStream(localDataUri);
                    }
                    mVCardParser = new VCardParser_V30();
                    try {
                        counter = new VCardEntryCounter();
                        detector = new VCardSourceDetector();
                        mVCardParser.addInterpreter(counter);
                        mVCardParser.addInterpreter(detector);
                        mVCardParser.parse(is);
                    } catch (VCardVersionException e2) {
                        throw new VCardException("vCard with unspported version.");
                    }
                } finally {
                    if (is != null) {
                        try {
                            is.close();
                        } catch (IOException e) {
                        }
                    }
                }

                vcardVersion = shouldUseV30 ? VCARD_VERSION_V30 : VCARD_VERSION_V21;
            } catch (VCardNestedException e) {
                Log.w(LOG_TAG, "Nested Exception is found (it may be false-positive).");
                // Go through without throwing the Exception, as we may be able to detect the
                // version before it
            }
            return new ImportRequest(mAccount,
                    data, localDataUri, displayName,
                    detector.getEstimatedType(),
                    detector.getEstimatedCharset(),
                    vcardVersion, counter.getCount());
        }

        public Uri[] getSourceUris() {
            return mSourceUris;
        }

        public void cancel() {
            mCanceled = true;
            if (mVCardParser != null) {
                mVCardParser.cancel();
            }
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            Log.i(LOG_TAG, "Cancel request has come. Abort caching vCard.");
            cancel();
        }
    }

    /// M: [MTK IMPORT/EXPORT] @{
    private class ImportTypeSelectedListener implements
            DialogInterface.OnClickListener {
        public static final int IMPORT_ONE = 0;
        public static final int IMPORT_MULTIPLE = 1;
        public static final int IMPORT_ALL = 2;
        public static final int IMPORT_TYPE_SIZE = 3;

        private int mCurrentIndex;

        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                switch (mCurrentIndex) {
                case IMPORT_ALL:
                    importVCardFromSDCard(mAllVCardFileList);
                    break;
                case IMPORT_MULTIPLE:
                    showDialog(R.id.dialog_select_multiple_vcard);
                    /**
                     * M: Default show dialog_select_multiple_vcard,should
                     * disable the OK button @{
                     */
                    Button btnOK = mVcardFileSelectDialog
                            .getButton(DialogInterface.BUTTON_POSITIVE);
                    btnOK.setEnabled(false);
                    /** @} */
                    break;
                default:
                    showDialog(R.id.dialog_select_one_vcard);
                    break;
                }
                /** M: Bug Fix, CR ID: ALPS00110214 */
                setResult(ImportExportUtils.RESULT_CODE);
            } else if (which == DialogInterface.BUTTON_NEGATIVE) {
                finish();
            } else {
                mCurrentIndex = which;
            }
        }
    }

    private class VCardSelectedListener implements
            DialogInterface.OnClickListener, DialogInterface.OnMultiChoiceClickListener {
        private int mCurrentIndex;
        private Set<Integer> mSelectedIndexSet;

        public VCardSelectedListener(boolean multipleSelect) {
            mCurrentIndex = 0;
            if (multipleSelect) {
                mSelectedIndexSet = new HashSet<Integer>();
            }
        }

        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mSelectedIndexSet != null) {
                    List<VCardFile> selectedVCardFileList = new ArrayList<VCardFile>();
                    final int size = mAllVCardFileList.size();
                    // We'd like to sort the files by its index, so we do not use Set iterator.
                    for (int i = 0; i < size; i++) {
                        if (mSelectedIndexSet.contains(i)) {
                            selectedVCardFileList.add(mAllVCardFileList.get(i));
                        }
                    }
                    importVCardFromSDCard(selectedVCardFileList);
                } else {
                    importVCardFromSDCard(mAllVCardFileList.get(mCurrentIndex));
                }
                /** M: Bug Fix CR ID: ALPS00110214 */
                setResult(ImportExportUtils.RESULT_CODE);
            } else if (which == DialogInterface.BUTTON_NEGATIVE) {
                finish();
            } else {
                // Some file is selected.
                mCurrentIndex = which;
                if (mSelectedIndexSet != null) {
                    /** M: When selected file is none, should disable the OK button @{ */
                    Button btnOK = mVcardFileSelectDialog.
                            getButton(DialogInterface.BUTTON_POSITIVE);
                    boolean enabled = true;
                    if (mSelectedIndexSet.contains(which)) {
                        mSelectedIndexSet.remove(which);
                        if (mSelectedIndexSet.size() == 0) {
                            enabled = false;
                        }
                    } else {
                        mSelectedIndexSet.add(which);
                    }
                    btnOK.setEnabled(enabled);
                    /** @} */
                }
            }
        }

        public void onClick(DialogInterface dialog, int which, boolean isChecked) {
            if (mSelectedIndexSet == null || (mSelectedIndexSet.contains(which) == isChecked)) {
                Log.e(LOG_TAG, String.format("Inconsist state in index %d (%s)", which,
                        Log.anonymize(mAllVCardFileList.get(which).getCanonicalPath())));
            } else {
                onClick(dialog, which);
            }
        }
    }

    /**
     * Thread scanning VCard from SDCard. After scanning, the dialog which lets a user select
     * a vCard file is shown. After the choice, VCardReadThread starts running.
     */
    private class VCardScanThread extends Thread implements OnCancelListener, OnClickListener {
        private boolean mCanceled;
        private boolean mGotIOException;
        private File mRootDirectory;

        // To avoid recursive link.
        private Set<String> mCheckedPaths;
        private PowerManager.WakeLock mWakeLock;

        private class CanceledException extends Exception {
        }

        public VCardScanThread(File sdcardDirectory) {
            mCanceled = false;
            mGotIOException = false;
            mRootDirectory = sdcardDirectory;
            mCheckedPaths = new HashSet<String>();
            PowerManager powerManager = (PowerManager)ImportVCardActivity.this.getSystemService(
                    Context.POWER_SERVICE);
            mWakeLock = powerManager.newWakeLock(
                    PowerManager.SCREEN_DIM_WAKE_LOCK |
                    PowerManager.ON_AFTER_RELEASE, LOG_TAG);
        }

        @Override
        public void run() {
            mAllVCardFileList = new Vector<VCardFile>();
            try {
                mWakeLock.acquire();
                getVCardFileRecursively(mRootDirectory);
            } catch (CanceledException e) {
                mCanceled = true;
            } catch (IOException e) {
                mGotIOException = true;
            } finally {
                mWakeLock.release();
            }

            if (mCanceled) {
                mAllVCardFileList = null;
            }

            mProgressDialogForScanVCard.dismiss();
            mProgressDialogForScanVCard = null;

            if (mGotIOException) {
                runOnUiThread(new DialogDisplayer(R.id.dialog_io_exception));
            } else if (mCanceled) {
                finish();
            } else {
                int size = mAllVCardFileList.size();
                final Context context = ImportVCardActivity.this;
                if (size == 0) {
                    runOnUiThread(new DialogDisplayer(R.id.dialog_vcard_not_found));
                } else {
                    startVCardSelectAndImport();
                }
            }
        }

        private void getVCardFileRecursively(File directory)
                throws CanceledException, IOException {
            if (mCanceled) {
                throw new CanceledException();
            }

            // e.g. secured directory may return null toward listFiles().
            final File[] files = directory.listFiles();
            if (files == null) {
                final String currentDirectoryPath = directory.getCanonicalPath();
                final String secureDirectoryPath =
                        mRootDirectory.getCanonicalPath().concat(SECURE_DIRECTORY_NAME);
                if (!TextUtils.equals(currentDirectoryPath, secureDirectoryPath)) {
                    Log.w(LOG_TAG, "listFiles() returned null (directory: " + Log.anonymize(directory) + ")");
                }
                return;
            }
            try {
                for (File file : directory.listFiles()) {
                    if (mCanceled) {
                        throw new CanceledException();
                    }
                    String canonicalPath = file.getCanonicalPath();
                    if (mCheckedPaths.contains(canonicalPath)) {
                        continue;
                    }

                    mCheckedPaths.add(canonicalPath);

                    if (file.isDirectory()) {
                        getVCardFileRecursively(file);
                    } else if (canonicalPath.toLowerCase().endsWith(".vcf") &&
                            file.canRead()) {
                        String fileName = file.getName();
                        VCardFile vcardFile = new VCardFile(
                                fileName, canonicalPath, file.lastModified());
                        mAllVCardFileList.add(vcardFile);
                    }
                }
            } catch (NullPointerException e) {
                Log.e(LOG_TAG, "Null pointer file path:" + Log.anonymize(directory));
                return;
            }
        }

        public void onCancel(DialogInterface dialog) {
            mCanceled = true;
        }

        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_NEGATIVE) {
                mCanceled = true;
            }
        }
    }

    /// M: new feature, import multi vcf file from storage @{
    private void startVCardSelectAndImport() {
        int size = mAllVCardFileList.size();
        if (getResources().getBoolean(R.bool.config_import_all_vcard_from_sdcard_automatically) ||
                size == 1) {
            /** M: Bug Fix, CR ID: ALPS00120656 @{ */
            if (size == 1) {
                setResult(ImportExportUtils.RESULT_CODE);
            }
            /** @} */
            importVCardFromSDCard(mAllVCardFileList);
        } else if (getResources().getBoolean(R.bool.config_allow_users_select_all_vcard_import)) {
            runOnUiThread(new DialogDisplayer(R.id.dialog_select_import_type));
        } else {
            runOnUiThread(new DialogDisplayer(R.id.dialog_select_one_vcard));
        }
    }

    private void importVCardFromSDCard(final List<VCardFile> selectedVCardFileList) {
        Log.d(LOG_TAG, "[importVCardFromSDCard]");
        /** M: [Google Issue][ALPS03298275] add for import multi vcf file
         * ori code:
        final int size = selectedVCardFileList.size();
        final Uri[] srcUri = new Uri[size];
        final Uri[] copyUri = new Uri[size];
        final String[] sourceDisplayNames = new String[size];
        int i = 0;
        for (VCardFile vcardFile : selectedVCardFileList) {
            srcUri[i] = Uri.parse(getEncodeUriString(vcardFile.getCanonicalPath()));
            copyUri[i] = readUriToLocalUri(srcUri[i]);
            ///M: ALPS03263987: readUriToLocalUri will return null if SDCard been plug out.
            if (copyUri[i] == null) {
                Log.w(LOG_TAG, "No local URI for vCard import");
                finish();
                return;
            }
            /// @}
            sourceDisplayNames[i] = getDisplayName(srcUri[i]);
            i++;
            if (i == Integer.MAX_VALUE) {
                throw new RuntimeException("Exceeded cache limit");
            }
        }
        importVCard(copyUri, sourceDisplayNames);
         * new code @{ */
        importVCard(selectedVCardFileList);
        /* @} */
    }

    private void importVCardFromSDCard(final VCardFile vcardFile) {
        Log.d(LOG_TAG, "[importVCardFromSDCard:vcardFile]");
        Uri fileUri = Uri.parse(getEncodeUriString(vcardFile.getCanonicalPath()));
        Uri copyUri = readUriToLocalUri(fileUri);
        String sourceDisplayName = getDisplayName(fileUri);
        ///M: ALPS03263987: readUriToLocalUri will return null if SDCard been plug out.
        if (copyUri != null) {
            importVCard(new Uri[]{copyUri}, new String[]{sourceDisplayName});
        } else {
            Log.w(LOG_TAG, "No local URI for vCard import");
            finish();
        }
        /// @}
    }

    ///M: [Google Issue][ALPS03298275] add for import multi vcf file
    private void importVCard(final List<VCardFile> selectedVCardFileList) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!isFinishing() && mResumed) {
                    mVCardCacheThread = new VCardCacheThread(selectedVCardFileList);
                    mListener = new NotificationImportExportListener(ImportVCardActivity.this);
                    showDialog(R.id.dialog_cache_vcard);
                }
            }
        });
    }
    /// @}

    private void importVCard(final Uri uri, final String sourceDisplayName) {
        importVCard(new Uri[] {uri}, new String[] {sourceDisplayName});
    }

    private void importVCard(final Uri[] uris, final String[] sourceDisplayNames) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!isFinishing()) {
                    mVCardCacheThread = new VCardCacheThread(uris, sourceDisplayNames);
                    mListener = new NotificationImportExportListener(ImportVCardActivity.this);
                    showDialog(R.id.dialog_cache_vcard);
                }
            }
        });
    }

    private String getDisplayName(Uri sourceUri) {
        if (sourceUri == null) {
            return null;
        }
        final ContentResolver resolver = ImportVCardActivity.this.getContentResolver();
        String displayName = null;
        Cursor cursor = null;
        // Try to get a display name from the given Uri. If it fails, we just
        // pick up the last part of the Uri.
        try {
            cursor = resolver.query(sourceUri,
                    new String[] { OpenableColumns.DISPLAY_NAME },
                    null, null, null);
            if (cursor != null && cursor.getCount() > 0 && cursor.moveToFirst()) {
                if (cursor.getCount() > 1) {
                    Log.w(LOG_TAG, "Unexpected multiple rows: "
                            + cursor.getCount());
                }
                int index = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (index >= 0) {
                    displayName = cursor.getString(index);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        /// M: ALPS02743192. when not get display name will using lastPathSegement,check
        /// whether it`s vcf suffix.@{
        if (TextUtils.isEmpty(displayName)) {
            //Log.d(LOG_TAG,"[getDisplayName] displayName is null,will get displayName!");
            if (isVcfSuffixFileUri(sourceUri)) {//get lastPathSegment as display name like "xx.vcf"
                displayName = sourceUri.getLastPathSegment();
            } else {//query mediaProvider to get displayName.like uri from FileManager
                displayName = getDisplayNameByQueryMediaProvider(sourceUri);
            }
        }
        /// @}
        Log.d(LOG_TAG, "[getDisplayName] sourceUri: " + Log.anonymize(sourceUri) + ",displayName : " +
                Log.anonymize(displayName));
        return displayName;
    }

    /**
     * Copy the content of sourceUri to the destination.
     */
    private Uri copyTo(final Uri sourceUri, String filename) throws IOException {
        Log.i(LOG_TAG, String.format("Copy a Uri to app local storage (%s -> %s)",
                sourceUri, filename));
        final Context context = ImportVCardActivity.this;
        final ContentResolver resolver = context.getContentResolver();
        ReadableByteChannel inputChannel = null;
        WritableByteChannel outputChannel = null;
        Uri destUri = null;
        try {
            inputChannel = Channels.newChannel(resolver.openInputStream(sourceUri));
            destUri = Uri.parse(context.getFileStreamPath(filename).toURI().toString());
            outputChannel = context.openFileOutput(filename, Context.MODE_PRIVATE).getChannel();
            final ByteBuffer buffer = ByteBuffer.allocateDirect(8192);
            while (inputChannel.read(buffer) != -1) {
                buffer.flip();
                outputChannel.write(buffer);
                buffer.compact();
            }
            buffer.flip();
            while (buffer.hasRemaining()) {
                outputChannel.write(buffer);
            }
        } finally {
            if (inputChannel != null) {
                try {
                    inputChannel.close();
                } catch (IOException e) {
                    Log.w(LOG_TAG, "Failed to close inputChannel.");
                }
            }
            if (outputChannel != null) {
                try {
                    outputChannel.close();
                } catch(IOException e) {
                    Log.w(LOG_TAG, "Failed to close outputChannel");
                }
            }
        }
        return destUri;
    }

    /**
     * Reads the file from {@param sourceUri} and copies it to local cache file.
     * Returns the local file name which stores the file from sourceUri.
     */
    private String readUriToLocalFile(Uri sourceUri) {
        // Read the uri to local first.
        int cache_index = 0;
        String localFilename = null;
        // Note: caches are removed by VCardService.
        while (true) {
            localFilename = VCardService.CACHE_FILE_PREFIX + cache_index + ".vcf";
            final File file = getFileStreamPath(localFilename);
            if (!file.exists()) {
                break;
            } else {
                if (cache_index == Integer.MAX_VALUE) {
                    throw new RuntimeException("Exceeded cache limit");
                }
                cache_index++;
            }
        }
        try {
            copyTo(sourceUri, localFilename);
        } catch (IOException|SecurityException e) {
            FeedbackHelper.sendFeedback(this, LOG_TAG, "Failed to copy vcard to local file", e);
            showFailureNotification(R.string.fail_reason_io_error);
            return null;
        }
        Log.d(LOG_TAG,"[readUriToLocalFile] sourceUri: " + Log.anonymize(sourceUri) + ",localFilename : "+
                Log.anonymize(localFilename));
        if (localFilename == null) {
            Log.e(LOG_TAG, "Cannot load uri to local storage.");
            showFailureNotification(R.string.fail_reason_io_error);
            return null;
        }

        return localFilename;
    }

    private Uri readUriToLocalUri(Uri sourceUri) {
        final String fileName = readUriToLocalFile(sourceUri);
        if (fileName == null) {
            return null;
        }
        return Uri.parse(getFileStreamPath(fileName).toURI().toString());
    }

    // Returns true if uri is from Storage.
    private boolean isStorageUri(Uri uri) {
        return uri != null && uri.toString().startsWith(STORAGE_VCARD_URI_PREFIX);
    }

    ///M: [MTK IMPORT/EXPORT]
    private Dialog getSelectImportTypeDialog() {
        /// M: [ALPS03261435] Don't show this dialog if mAllVCardFileList is null.
        /// M: [ALPS02413700] Don't show this dialog if vcard list is empty.  @{
        final int size = (mAllVCardFileList == null) ? 0 : mAllVCardFileList.size();
        if (size == 0) {
            Log.w(LOG_TAG, "[getSelectImportTypeDialog] size: " + size);
            return null;
        }
        /// @}

        final DialogInterface.OnClickListener listener = new ImportTypeSelectedListener();
        final AlertDialog.Builder builder = new AlertDialog.Builder(this)
                .setTitle(R.string.select_vcard_title)
                .setPositiveButton(android.R.string.ok, listener)
                .setOnCancelListener(mCancelListener)
                .setNegativeButton(android.R.string.cancel, mCancelListener);

        final String[] items = new String[ImportTypeSelectedListener.IMPORT_TYPE_SIZE];
        items[ImportTypeSelectedListener.IMPORT_ONE] =
                getString(R.string.import_one_vcard_string);
        items[ImportTypeSelectedListener.IMPORT_MULTIPLE] =
                getString(R.string.import_multiple_vcard_string);
        items[ImportTypeSelectedListener.IMPORT_ALL] =
                getString(R.string.import_all_vcard_string);
        builder.setSingleChoiceItems(items, ImportTypeSelectedListener.IMPORT_ONE, listener);
        return builder.create();
    }

    private Dialog getVCardFileSelectDialog(boolean multipleSelect) {
        /// M: [ALPS03261435] Don't show this dialog if mAllVCardFileList is null. @{
        final int size = (mAllVCardFileList == null) ? 0 : mAllVCardFileList.size();
        /// @}
        /// M: ALPS02413700 Don't show this dialog if vcard list is empty. @{
        if (size == 0) {
            Log.w(LOG_TAG, "[getVCardFileSelectDialog] size: " + size);
            return null;
        }
        // @}

        final VCardSelectedListener listener = new VCardSelectedListener(multipleSelect);
        final AlertDialog.Builder builder =
                new AlertDialog.Builder(this)
                        .setTitle(R.string.select_vcard_title)
                        .setPositiveButton(android.R.string.ok, listener)
                        .setOnCancelListener(mCancelListener)
                        .setNegativeButton(android.R.string.cancel, mCancelListener);

        CharSequence[] items = new CharSequence[size];
        //DateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        for (int i = 0; i < size; i++) {
            VCardFile vcardFile = mAllVCardFileList.get(i);
            SpannableStringBuilder stringBuilder = new SpannableStringBuilder();
            stringBuilder.append(vcardFile.getName());
            stringBuilder.append('\n');
            int indexToBeSpanned = stringBuilder.length();
            // Smaller date text looks better, since each file name becomes easier to read.
            // The value set to RelativeSizeSpan is arbitrary. You can change it to any other
            // value (but the value bigger than 1.0f would not make nice appearance :)

            /// M:fix ALPS02371510.resolved show file time not sync setting @{
            stringBuilder.append(
                    "(" + DateUtils.formatDateTime (this,vcardFile.getLastModified(),
                            DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME |
                            DateUtils.FORMAT_SHOW_YEAR) + ")");
            //stringBuilder.append(
            //      "(" + dateFormat.format(new Date(vcardFile.getLastModified())) + ")");
            // @}

            stringBuilder.setSpan(
                    new RelativeSizeSpan(0.7f), indexToBeSpanned, stringBuilder.length(),
                    Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            items[i] = stringBuilder;
        }
        if (multipleSelect) {
            builder.setMultiChoiceItems(items, (boolean[])null, listener);
        } else {
            builder.setSingleChoiceItems(items, 0, listener);
        }
        return builder.create();
    }
    /// @}

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);

        Uri sourceUri = getIntent().getData();
        Log.d(LOG_TAG, "[onCreate] sourceUri: " + Log.anonymize(sourceUri));
        // Reading uris from non-storage needs the permission granted from the source intent,
        // instead of permissions from RequestImportVCardPermissionActivity. So skipping requesting
        // permissions from RequestImportVCardPermissionActivity for uris from non-storage source.

        /// M: ALPS02743192.using isStorageUriEx replace of isStorageUri.@{
        if (isStorageUriEx(sourceUri) /// @}
                && RequestImportVCardPermissionsActivity
                .startPermissionActivity(this, isCallerSelf(this))) {
            Log.i(LOG_TAG,"[onCreate] it is storage uri and need request permission!");
            return;
        }

        String sourceDisplayName = null;
        if (sourceUri != null) {
            // Read the uri to local first.
            String localTmpFileName = getIntent().getStringExtra(LOCAL_TMP_FILE_NAME_EXTRA);
            sourceDisplayName = getIntent().getStringExtra(SOURCE_URI_DISPLAY_NAME);
            if (TextUtils.isEmpty(localTmpFileName)) {
                localTmpFileName = readUriToLocalFile(sourceUri);
                sourceDisplayName = getDisplayName(sourceUri);
                if (localTmpFileName == null) {
                    Log.e(LOG_TAG, "Cannot load uri to local storage.");
                    showFailureNotification(R.string.fail_reason_io_error);
                    return;
                }
                getIntent().putExtra(LOCAL_TMP_FILE_NAME_EXTRA, localTmpFileName);
                getIntent().putExtra(SOURCE_URI_DISPLAY_NAME, sourceDisplayName);
            }
            sourceUri = Uri.parse(getFileStreamPath(localTmpFileName).toURI().toString());
            Log.d(LOG_TAG, "[onCreate] localTmpFileName: " + Log.anonymize(localTmpFileName) +
                    ",sourceDisplayName: " + Log.anonymize(sourceDisplayName) + ",sourceUri: " +
                    Log.anonymize(sourceUri));
        }

        // Always request required permission for contacts before importing the vcard.
        if (RequestImportVCardPermissionsActivity.startPermissionActivity(this,
                isCallerSelf(this))) {
            Log.w(LOG_TAG,"[onCreate] need request permission!");
            return;
        }

        String accountName = null;
        String accountType = null;
        String dataSet = null;
        final Intent intent = getIntent();
        if (intent != null) {
            accountName = intent.getStringExtra(SelectAccountActivity.ACCOUNT_NAME);
            accountType = intent.getStringExtra(SelectAccountActivity.ACCOUNT_TYPE);
            dataSet = intent.getStringExtra(SelectAccountActivity.DATA_SET);
            /// M:[ALPS00276020] New Feature, support multiple storages for import/export. @{
            mSourcePath = intent.getStringExtra("source_path");
            /// @}
        } else {
            Log.e(LOG_TAG, "intent does not exist");
        }

        if (!TextUtils.isEmpty(accountName) && !TextUtils.isEmpty(accountType)) {
            mAccount = new AccountWithDataSet(accountName, accountType, dataSet);
        } else {
            final AccountTypeManager accountTypes = AccountTypeManager.getInstance(this);
            final List<AccountWithDataSet> accountList = accountTypes.blockForWritableAccounts();
            if (accountList.size() == 0) {
                mAccount = null;
            } else if (accountList.size() == 1) {
                mAccount = accountList.get(0);
            } else {
                /** M: Bug Fix for CR: ALPS00403117 @{ */
                int validAccountSize = accountList.size();
                for (AccountWithDataSet account : accountList) {
                    if (AccountTypeUtils.isAccountTypeIccCard(account.type)) {
                        validAccountSize--;
                    } else {
                        mAccount = account;
                    }
                }
                if (validAccountSize > 1) {
                    startActivityForResult(new Intent(this, SelectAccountActivity.class)
                            .setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP), SELECT_ACCOUNT);
                    return ;
                }
                /** @} */
            }
        }

        if (isCallerSelf(this)) {
            startImport(sourceUri, sourceDisplayName);
        } else {
            ImportVCardDialogFragment.show(this, sourceUri, sourceDisplayName);
        }
    }

    ///M: [ALPS03689588] check for whether we are in background @{
    private boolean mResumed = false;

    @Override
    protected void onPause() {
        mResumed = false;
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mResumed = true;
    }
    ///M: @}

    private static boolean isCallerSelf(Activity activity) {
        // {@link Activity#getCallingActivity()} is a safer alternative to
        // {@link Activity#getCallingPackage()} that works around a
        // framework bug where getCallingPackage() can sometimes return null even when the
        // current activity *was* in fact launched via a startActivityForResult() call.
        //
        // (The bug happens if the task stack needs to be re-created by the framework after
        // having been killed due to memory pressure or by the "Don't keep activities"
        // developer option; see bug 7494866 for the full details.)
        //
        // Turns out that {@link Activity#getCallingActivity()} *does* return correct info
        // even in the case where getCallingPackage() is broken, so the workaround is simply
        // to get the package name from getCallingActivity().getPackageName() instead.
        final ComponentName callingActivity = activity.getCallingActivity();
        if (callingActivity == null) return false;
        final String packageName = callingActivity.getPackageName();
        if (packageName == null) return false;
        return packageName.equals(activity.getApplicationContext().getPackageName());
    }

    @Override
    public void onImportVCardConfirmed(Uri sourceUri, String sourceDisplayName) {
        startImport(sourceUri, sourceDisplayName);
    }

    @Override
    public void onImportVCardDenied() {
        finish();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent intent) {
        Log.i(LOG_TAG, "[onActivityResult] requestCode:"+ requestCode +
                ",resultCode: " + resultCode + ",intent: " + intent);
        if (requestCode == SELECT_ACCOUNT) {
            if (resultCode == Activity.RESULT_OK) {
                mAccount = new AccountWithDataSet(
                        intent.getStringExtra(SelectAccountActivity.ACCOUNT_NAME),
                        intent.getStringExtra(SelectAccountActivity.ACCOUNT_TYPE),
                        intent.getStringExtra(SelectAccountActivity.DATA_SET));
                ///M:[Google Issue][ALPS03632327]Check if account exists after account selected @{
                final List<AccountWithDataSet> accountList =
                        AccountTypeManager.getInstance(this).blockForWritableAccounts();
                boolean exist = false;
                if (accountList != null && accountList.size() >= 1) {
                    for (int i = 0; i < accountList.size(); i++) {
                        if (mAccount.equals(accountList.get(i))) {
                            exist = true;
                            break;
                        }
                    }
                    if (!exist) {
                        MtkToast.toast(ImportVCardActivity.this,
                                getString(R.string.vcard_import_failed), Toast.LENGTH_LONG);
                        Log.e(LOG_TAG, "[onActivityResult] "+ mAccount + " doesn't existed !");
                        finish();
                    }
                }
                ///@}
                final Uri sourceUri = getIntent().getData();
                if (sourceUri == null) {
                    startImport(sourceUri, /* sourceDisplayName =*/ null);
                } else {
                    final String sourceDisplayName = getIntent().getStringExtra(
                            SOURCE_URI_DISPLAY_NAME);
                    final String localFileName = getIntent().getStringExtra(
                            LOCAL_TMP_FILE_NAME_EXTRA);
                    final Uri localUri = Uri.parse(
                            getFileStreamPath(localFileName).toURI().toString());
                    startImport(localUri, sourceDisplayName);
                }
            } else {
                if (resultCode != Activity.RESULT_CANCELED) {
                    Log.w(LOG_TAG, "Result code was not OK nor CANCELED: " + resultCode);
                }
                finish();
            }
        } else if (requestCode == REQUEST_OPEN_DOCUMENT) {
            if (resultCode == Activity.RESULT_OK) {
                final ClipData clipData = intent.getClipData();
                if (clipData != null) {
                    final ArrayList<Uri> uris = new ArrayList<>();
                    final ArrayList<String> sourceDisplayNames = new ArrayList<>();
                    for (int i = 0; i < clipData.getItemCount(); i++) {
                        ClipData.Item item = clipData.getItemAt(i);
                        final Uri uri = item.getUri();
                        if (uri != null) {
                            final Uri localUri = readUriToLocalUri(uri);
                            if (localUri != null) {
                                final String sourceDisplayName = getDisplayName(uri);
                                uris.add(localUri);
                                sourceDisplayNames.add(sourceDisplayName);
                            }
                        }
                    }
                    if (uris.isEmpty()) {
                        Log.w(LOG_TAG, "No vCard was selected for import");
                        finish();
                    } else {
                        Log.i(LOG_TAG, "Multiple vCards selected for import: " + uris);
                        importVCard(uris.toArray(new Uri[0]),
                                sourceDisplayNames.toArray(new String[0]));
                    }
                } else {
                    final Uri uri = intent.getData();
                    if (uri != null) {
                        Log.i(LOG_TAG, "vCard selected for import: " + uri);
                        final Uri localUri = readUriToLocalUri(uri);
                        if (localUri != null) {
                            final String sourceDisplayName = getDisplayName(uri);
                            importVCard(localUri, sourceDisplayName);
                        } else {
                            Log.w(LOG_TAG, "No local URI for vCard import");
                            finish();
                        }
                    } else {
                        Log.w(LOG_TAG, "No vCard was selected for import");
                        finish();
                    }
                }
            } else {
                if (resultCode != Activity.RESULT_CANCELED) {
                    Log.w(LOG_TAG, "Result code was not OK nor CANCELED" + resultCode);
                }
                finish();
            }
        }
    }

    private void startImport(Uri uri, String sourceDisplayName) {
        Log.d(LOG_TAG, "[startImport] uri: " + Log.anonymize(uri) + ",sourceDisplayName: " + Log.anonymize(sourceDisplayName));
        // Handle inbound files
        if (uri != null) {
            Log.i(LOG_TAG, "Starting vCard import using Uri " + uri);
            importVCard(uri, sourceDisplayName);
        } else {
            Log.i(LOG_TAG, "Start vCard without Uri. The user will select vCard manually.");
           /**
             * M: [MTK IMPORT/EXPORT] @{
            final Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType(VCardService.X_VCARD_MIME_TYPE);
            intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
            startActivityForResult(intent, REQUEST_OPEN_DOCUMENT);
             @}*/
            doScanExternalStorageAndImportVCard();
        }
    }

    /** M:[MTK IMPORT/EXPORT]: mark google code:
    @Override
    protected Dialog onCreateDialog(int resId, Bundle bundle) {
        if (resId == R.id.dialog_cache_vcard) {
            if (mProgressDialogForCachingVCard == null) {
                final String title = getString(R.string.caching_vcard_title);
                final String message = getString(R.string.caching_vcard_message);
                mProgressDialogForCachingVCard = new ProgressDialog(this);
                mProgressDialogForCachingVCard.setTitle(title);
                mProgressDialogForCachingVCard.setMessage(message);
                mProgressDialogForCachingVCard.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                mProgressDialogForCachingVCard.setOnCancelListener(mVCardCacheThread);
                startVCardService();
            }
            return mProgressDialogForCachingVCard;
        } else if (resId == R.id.dialog_error_with_message) {
            String message = mErrorMessage;
            if (TextUtils.isEmpty(message)) {
                Log.e(LOG_TAG, "Error message is null while it must not.");
                message = getString(R.string.fail_reason_unknown);
            }
            final AlertDialog.Builder builder = new AlertDialog.Builder(this)
                .setTitle(getString(R.string.reading_vcard_failed_title))
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(message)
                .setOnCancelListener(mCancelListener)
                .setPositiveButton(android.R.string.ok, mCancelListener);
            return builder.create();
        }

        return super.onCreateDialog(resId, bundle);
    }
    * MTK solution: @{ */
    @Override
    protected Dialog onCreateDialog(int resId, Bundle bundle) {
        switch (resId) {
            ///M:@{
            case R.id.dialog_searching_vcard: {
                Log.d(LOG_TAG, "[onCreateDialog]dialog_search_vcard");
                ///Change for CR ALPS00683398. This will be called from
                //  Activity.restoreManagedDialogs() when the contacts process has been killed;
                //  So add checking "mVCardScanThread != null" before use it.
                if (mProgressDialogForScanVCard == null && mVCardScanThread != null) {
                    /// M: [ALPS03785068] ignore duplciate create dialog by restoreManagedDialogs.
                    if (mIsScanThreadStarted) {
                        Log.w(LOG_TAG, "[onCreateDialog] Ignore !!!", new Exception());
                        return null;
                    }
                    String message = getString(R.string.searching_vcard_message);
                    mProgressDialogForScanVCard =
                        ProgressDialog.show(this, "", message, true, false);
                    mProgressDialogForScanVCard.setOnCancelListener(mVCardScanThread);
                    mVCardScanThread.start();
                    mIsScanThreadStarted = true;
                }
                return mProgressDialogForScanVCard;
            }
            case R.id.dialog_sdcard_not_found: {
                Log.d(LOG_TAG, "[onCreateDialog]dialog_sdcard_not_found");
                AlertDialog.Builder builder = new AlertDialog.Builder(this)
                    .setMessage(R.string.no_sdcard_message)
                    .setOnCancelListener(mCancelListener)
                    .setPositiveButton(android.R.string.ok, mCancelListener);
                return builder.create();
            }
            case R.id.dialog_vcard_not_found: {
                Log.d(LOG_TAG, "[onCreateDialog]dialog_vcard_not_found");
                final String message = getString(R.string.import_no_vcard_dialog_text, mVolumeName);
                AlertDialog.Builder builder = new AlertDialog.Builder(this)
                        .setMessage(message)
                        .setOnCancelListener(mCancelListener)
                        .setPositiveButton(android.R.string.ok, mCancelListener);
                return builder.create();
            }
            case R.id.dialog_select_import_type: {
                Log.d(LOG_TAG, "[onCreateDialog]dialog_select_import_type");
                return getSelectImportTypeDialog();
            }
            case R.id.dialog_select_multiple_vcard: {
                Log.d(LOG_TAG, "[onCreateDialog]dialog_select_multiple_vcard");
                mVcardFileSelectDialog = (AlertDialog) getVCardFileSelectDialog(true);
                return mVcardFileSelectDialog;
            }
            case R.id.dialog_select_one_vcard: {
                Log.d(LOG_TAG, "[onCreateDialog]dialog_select_one_vcard");
                return getVCardFileSelectDialog(false);
            }
            case R.id.dialog_io_exception: {
                Log.d(LOG_TAG, "[onCreateDialog]dialog_io_exception");
                /// M: Todo Add for migration build error, need change @{
                String message = (getString(R.string.fail_reason_unknown,
                        getString(R.string.fail_reason_io_error)));
                AlertDialog.Builder builder = new AlertDialog.Builder(this)
                    .setMessage(message)
                    .setOnCancelListener(mCancelListener)
                    .setPositiveButton(android.R.string.ok, mCancelListener);
                return builder.create();
            }
            ///M: @}
            case R.id.dialog_cache_vcard: {
                Log.d(LOG_TAG, "[onCreateDialog]dialog_cache_vcard");
                if (mProgressDialogForCachingVCard == null) {
                    Log.d(LOG_TAG, "[onCreateDialog][dialog_cache_vcard]Dialog first created");
                    final String title = getString(R.string.caching_vcard_title);
                    final String message = getString(R.string.caching_vcard_message);
                    mProgressDialogForCachingVCard = new ProgressDialog(this);
                    mProgressDialogForCachingVCard.setTitle(title);
                    mProgressDialogForCachingVCard.setMessage(message);
                    mProgressDialogForCachingVCard.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                    mProgressDialogForCachingVCard.setOnCancelListener(mVCardCacheThread);
                    startVCardService();
                }
                return mProgressDialogForCachingVCard;
            }
            case R.id.dialog_error_with_message: {
                Log.d(LOG_TAG, "[onCreateDialog]dialog_error_with_message");
                String message = mErrorMessage;
                if (TextUtils.isEmpty(message)) {
                    Log.e(LOG_TAG, "Error message is null while it must not.");
                    message = getString(R.string.fail_reason_unknown);
                }
                final AlertDialog.Builder builder = new AlertDialog.Builder(this)
                    .setTitle(getString(R.string.reading_vcard_failed_title))
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setMessage(message)
                    .setOnCancelListener(mCancelListener)
                    .setPositiveButton(android.R.string.ok, mCancelListener);
                return builder.create();
            }
            default: {
                Log.w(LOG_TAG, "[onCreateDialog]res id is invalid: " + resId);
            }
        }

        return super.onCreateDialog(resId, bundle);
    }
    /* @} */

    /* package */ void startVCardService() {
        mConnection = new ImportRequestConnection();

        Log.i(LOG_TAG, "Bind to VCardService.");
        // We don't want the service finishes itself just after this connection.
        Intent intent = new Intent(this, VCardService.class);
        startService(intent);
        bindService(new Intent(this, VCardService.class),
                mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        if (mProgressDialogForCachingVCard != null) {
            Log.i(LOG_TAG, "Cache thread is still running. Show progress dialog again.");
            showDialog(R.id.dialog_cache_vcard);
        }
    }

    ///M:[MTK IMPORT/EXPORT] {
    /**
     *  M:Scans vCard in external storage (typically SDCard) and tries to import it.
     * - When there's no SDCard available, an error dialog is shown.
     * - When multiple vCard files are available, asks a user to select one.
     */
    private void doScanExternalStorageAndImportVCard() {
        // TODO: should use getExternalStorageState().
        /// M: New Feature ALPS00276020, multiple storages for import/export. @{
        String path = VcardUtils.getExternalPath(mSourcePath);
        Log.d(LOG_TAG,"[doScanExternalStorageAndImportVCard]path : " + Log.anonymize(path));
        final File file = VcardUtils.getDirectory(path,
                Environment.getExternalStorageDirectory().toString());
        /// @}
        /** M: Bug Fix, CR ID: ALPS00301464 */
        mVolumeName = VcardUtils.getVolumeName(path, this);

        if (!file.exists() || !file.isDirectory() || !file.canRead()) {
            showDialog(R.id.dialog_sdcard_not_found);
        } else {
            mIsScanThreadStarted = false;
            mVCardScanThread = new VCardScanThread(file);
            showDialog(R.id.dialog_searching_vcard);
        }
    }
    /// @}

    /* package */ void showFailureNotification(int reasonId) {
        final NotificationManager notificationManager =
                (NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE);
        final Notification notification =
                NotificationImportExportListener.constructImportFailureNotification(
                        ImportVCardActivity.this,
                        getString(reasonId));
        notificationManager.notify(NotificationImportExportListener.FAILURE_NOTIFICATION_TAG,
                FAILURE_NOTIFICATION_ID, notification);
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                /// M: To modify the toast last a long time issue.
                MtkToast.toast(ImportVCardActivity.this, getString(R.string.vcard_import_failed),
                        Toast.LENGTH_LONG);
            }
        });
    }

    /** M: [MTK IMPORT/EXPORT]@{ */
    private String mSourcePath = null;
    private String mVolumeName = null;

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(LOG_TAG, "[onDestroy]");

        if (mVCardCacheThread != null && mVCardCacheThread.isAlive()) {
            Log.w(LOG_TAG, "[onDestroy]should not finish Activity when work did not finished");
            mVCardCacheThread.cancel();
        }
    }

    /**
     * M:ALPS02671394. encode vcf fileName.
     * because some vcf filename include some special letter,such as tmp#.vcf,tmp%.vcf.
     * "#","%" cannot encode right,will make FileNotFoundException.
     * full filePathString = filePath + "/"+ uri.encode(fileName).
     */
    private String getEncodeUriString(String filePathString) {
        if (TextUtils.isEmpty(filePathString)) {
            Log.e(LOG_TAG, "[getEncodeUriString] filePathString is wrong !");
            return null;
        }
        int lastSeparatorIndex = filePathString.lastIndexOf(File.separator);
        StringBuilder rstPathString = new StringBuilder("file://").append(
                filePathString.substring(0, lastSeparatorIndex)).append(File.separator);
        String encodeFileName = Uri.encode(filePathString.substring(lastSeparatorIndex + 1,
                filePathString.length()));
        rstPathString.append(encodeFileName);
        return rstPathString.toString();
    }

    /*
     * M: ALPS02763024. it should expand storage check field.the uri which from media provider
     * should also request permission
     */
    private static final String MEDIA_VCARD_URI_PREFIX = "content://media";
    private boolean isStorageUriEx(Uri uri) {
        Log.d(LOG_TAG, "[isStorageUriEx] uri :" + Log.anonymize(uri));
        return uri != null && (uri.toString().startsWith(STORAGE_VCARD_URI_PREFIX)||
                uri.toString().startsWith(MEDIA_VCARD_URI_PREFIX));
    }

    /*
     * M: ALPS02743192.when using getDisplayName method to get display name fail.it will using
     * last Path Segment as the display name.add a check that if it`s not a vcf uri form.using
     * default toast to tip user.
     */
    private boolean isVcfSuffixFileUri(Uri sourceUri) {
        boolean rst = false;
        if (sourceUri != null && (sourceUri.toString().endsWith(".vcf"))) {
            rst = true;
        } else {
            Log.d(LOG_TAG, "[isVcfSuffixFileUri] rst: " + rst);
        }
        return rst;
    }

    /**
     * some app like FileManager,we receive the uri string that is not meaningful.to get
     * file name we should query using MediaStore.MediaColumns.DATA column
     */
    private String getDisplayNameByQueryMediaProvider(Uri sourceUri) {
        Log.d(LOG_TAG, "[getDisplayNameByQueryMediaProvider] sourceUri: " + Log.anonymize(sourceUri));
        if (sourceUri == null) {
            Log.w(LOG_TAG, "[getDisplayNameByQueryMediaProvider] sourceUri is null,reteurn");
            return null;
        }
        final ContentResolver resolver = ImportVCardActivity.this.getContentResolver();
        String displayName = null;
        Cursor cursor = null;
        String filePath = null;
        try {
            cursor = resolver.query(sourceUri,
                    new String[]{MediaStore.MediaColumns.DATA},
                    null, null, null);
            if (cursor != null && cursor.getCount() > 0 && cursor.moveToFirst()) {
                if (cursor.getCount() > 1) {
                    Log.w(LOG_TAG, "[[getDisplayNameByQueryMediaProvider]] Unexpected multiple" +
                            "rows: " + cursor.getCount());
                }
                int index = cursor.getColumnIndex(MediaStore.MediaColumns.DATA);
                if (index >= 0) {
                    filePath = cursor.getString(index);
                    displayName = Uri.parse(filePath).getLastPathSegment();
                }
                Log.i(LOG_TAG, "[getDisplayNameByQueryMediaProvider] index:" + index +
                        ", filePath:" + Log.anonymize(filePath) + ",displayName: " + Log.anonymize(displayName));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        Log.i(LOG_TAG, "[getDisplayNameByQueryMediaProvider] displayName:" + Log.anonymize(displayName));
        return displayName;
    }
    /** @} */
}
