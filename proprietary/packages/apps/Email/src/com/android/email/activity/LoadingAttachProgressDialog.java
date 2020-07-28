package com.android.email.activity;

import android.app.Activity;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.net.Uri;
import android.os.Bundle;

import com.android.email.R;
import com.android.emailcommon.utility.AsyncTask;
import com.android.emailcommon.utility.Utility;
import com.android.mail.analytics.Analytics;
import com.android.mail.compose.AttachmentsView;
import com.android.mail.compose.AttachmentsView.AttachmentFailureException;
import com.android.mail.providers.Attachment;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.Utils;

import java.util.ArrayList;
import java.util.List;

/**
 * M: Loading attachment Progress dialog
 */
public class LoadingAttachProgressDialog extends DialogFragment {
    public interface Callback {
        void onAttachmentsLoadingComplete(AttachmentFailureException exception,
                List<Attachment> attachments);
    }

    public static final String TAG = "LoadingAttachProgressDialog";
    private static final String BUNDLE_KEY_ATTACHMENTS_URI = "bundleKeyAttachmentsUri";
    private AttachmentLoadTask mLoadingTask = null;
    private ArrayList<Uri> mAttachmentsUri;

    /**
     * Create a dialog for Loading attachment asynctask.
     */
    public static LoadingAttachProgressDialog newInstance(Fragment parentFragment,
            ArrayList<Uri> uriList) {
        LoadingAttachProgressDialog f = new LoadingAttachProgressDialog();
        f.setTargetFragment(parentFragment, 0);
        Bundle b = new Bundle();
        b.putParcelableArrayList(BUNDLE_KEY_ATTACHMENTS_URI, uriList);
        f.setArguments(b);
        return f;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRetainInstance(true);
        mAttachmentsUri = getArguments().getParcelableArrayList(BUNDLE_KEY_ATTACHMENTS_URI);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Context context = getActivity();

        ProgressDialog dialog = new ProgressDialog(context);
        dialog.setIndeterminate(true);
        dialog.setMessage(getString(R.string.loading_attachment));
        dialog.setCancelable(true);
        dialog.setCanceledOnTouchOutside(false);

        return dialog;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        if (mLoadingTask == null) {
            LogUtils.d(TAG, "onActivityCreated : execute loading task...");
            mLoadingTask = (AttachmentLoadTask) new AttachmentLoadTask(getActivity(), this)
                    .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, mAttachmentsUri);
        }
    }

    /**
     * Listen for cancellation, which can happen from places other than the negative
     * button (e.g. touching outside the dialog), and stop the checker
     */
    @Override
    public void onCancel(DialogInterface dialog) {
        super.onCancel(dialog);
        LogUtils.d(TAG, "LoadingAttachProgressDialog onCancel %s", mLoadingTask);
        Activity activity = getActivity();
        if (activity instanceof Callback) {
            // dismiss dialog and the onDestroy would be called. AsyncTask cancelled.
            ((Callback) activity).onAttachmentsLoadingComplete(null, null);
        }

    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        LogUtils.d(TAG, "LoadingAttachProgressDialog : onDismiss be intercepted %s", mLoadingTask);
    }

    /**
     * This is called when the fragment is going away.  It is NOT called
     * when the fragment is being propagated between activity instances.
     */
    @Override
    public void onDestroy() {
        if (mLoadingTask != null) {
            LogUtils.d(TAG, "LoadingAttachProgressDialog onDestroy, mLoadingTask will be canceled");
            Utility.cancelTaskInterrupt(mLoadingTask);
            mLoadingTask = null;
        }
        super.onDestroy();
    }

    private static class AttachmentLoadTask extends
            AsyncTask<ArrayList<Uri>, Void, List<Attachment>> {
        private AttachmentFailureException mAttachException = null;
        final LoadingAttachProgressDialog mCallback;
        final Context mContext;

        public AttachmentLoadTask(Context context, LoadingAttachProgressDialog callback) {
            mCallback = callback;
            mContext = context;
        }

        @Override
        protected List<Attachment> doInBackground(ArrayList<Uri>... params) {
            ArrayList<Uri> attachUris = params[0];
            List<Attachment> attachments = new ArrayList<Attachment>();
            for (Uri uri : attachUris) {
                if (uri != null) {
                    try {
                        final Attachment attachment = AttachmentsView.generateLocalAttachment(uri,
                                mContext);
                        attachments.add(attachment);

                        Analytics.getInstance().sendEvent("send_intent_attachment",
                                Utils.normalizeMimeType(attachment.getContentType()), null,
                                attachment.size);
                    } catch (AttachmentFailureException e) {
                        LogUtils.e(TAG, e, "Error adding attachment uri [%s]", uri);
                        mAttachException = e;
                    } catch (IllegalStateException e) {
                        // / M: Maybe this Exception happen when the file of the URI doesn't exsit
                        LogUtils.e(TAG, e, "Error adding attachment uri [%s]", uri);
                        mAttachException = new AttachmentFailureException(
                                "Failed add attachment IllegalStateException ", e);
                    }
                }
            }
            return attachments;
        }

        @Override
        protected void onPostExecute(List<Attachment> attachments) {
            if (isCancelled()) {
                return;
            }
            LogUtils.d(TAG, "loadAttachmentsInBackground : onPostExecute");
            Activity activity = mCallback.getActivity();
            if (activity instanceof Callback) {
                ((Callback) activity).onAttachmentsLoadingComplete(mAttachException, attachments);
            }
        }

        @Override
        protected void onCancelled() {
            LogUtils.d(TAG, "loadAttachmentsInBackground : onCancelled");
        }
    }
}
