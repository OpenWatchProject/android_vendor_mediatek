package com.mediatek.email.util;

import android.content.Context;
import android.os.Handler;
import android.widget.Toast;

import com.android.email.provider.EmailProvider;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.StorageLowState;
import com.android.mail.utils.StorageLowState.LowStorageHandler;
import com.android.mail.R;
import com.mediatek.email.attachment.AttachmentAutoClearController;

/**
 * M: Low storage handler for Email
 *
 */
public class EmailLowStorageHandler implements LowStorageHandler {
    public static final String TAG = LogUtils.TAG;
    /** The context which we use to register the handler */
    private Context mContext;
    /** The handler we can post runnable on */
    private Handler mHandler = new Handler();

    public EmailLowStorageHandler(Context context) {
        mContext = context;
    }

    /* (non-Javadoc)
     * @see com.android.mail.utils.StorageLowState.LowStorageHandler#onStorageLow()
     */
    @Override
    public void onStorageLow() {
        LogUtils.d(TAG, "onStorageLow");
        // call this to make toast
        onStorageLowAfterCheck(StorageLowState.CHECK_FOR_GENERAL);
        EmailProvider.startOrStopEmailServices(mContext, false);
        // Try to recover from the low storage state
        AttachmentAutoClearController.clearMessageIdsSync();
        AttachmentAutoClearController.actionClearOnce(mContext);
    }

    /* (non-Javadoc)
     * @see com.android.mail.utils.StorageLowState.LowStorageHandler#onStorageOk()
     */
    @Override
    public void onStorageOk() {
        LogUtils.d(TAG, "onStorageOK");
        mHandler.post(new Runnable() {

            @Override
            public void run() {
                Toast.makeText(mContext, R.string.low_storage_service_start, Toast.LENGTH_SHORT).show();
            }
        });
        EmailProvider.startOrStopEmailServices(mContext, true);
    }

    @Override
    public void onStorageLowAfterCheck(final int checkReason) {
        LogUtils.d(TAG, "onStorageLowAfterCheck");
        mHandler.post(new Runnable() {

            @Override
            public void run() {
                switch (checkReason) {
                case StorageLowState.CHECK_FOR_VIEW_ATTACHMENT:
                    Toast.makeText(mContext, R.string.low_storage_attachment_cannot_view, Toast.LENGTH_SHORT).show();
                    Toast.makeText(mContext, R.string.low_storage_hint_delete_mail, Toast.LENGTH_SHORT).show();
                    break;
                case StorageLowState.CHECK_FOR_DOWNLOAD_ATTACHMENT:
                    Toast.makeText(mContext, R.string.low_storage_attachment_cannot_download, Toast.LENGTH_SHORT).show();
                    Toast.makeText(mContext, R.string.low_storage_hint_delete_mail, Toast.LENGTH_SHORT).show();
                    break;
                case StorageLowState.CHECK_FOR_GENERAL:
                default:
                    Toast.makeText(mContext, R.string.low_storage_service_stop, Toast.LENGTH_SHORT).show();
                    Toast.makeText(mContext, R.string.low_storage_hint_delete_mail, Toast.LENGTH_SHORT).show();
                    break;
                }
            }
        });
    }

}
