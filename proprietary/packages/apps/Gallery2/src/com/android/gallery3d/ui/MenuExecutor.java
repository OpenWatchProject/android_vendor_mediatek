/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
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

package com.android.gallery3d.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.support.v4.print.PrintHelper;
import android.view.Menu;
import android.view.MenuItem;

import android.widget.Toast;

import com.android.gallery3d.R;
import com.android.gallery3d.app.AbstractGalleryActivity;
import com.android.gallery3d.app.ActivityState;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.ClusterAlbumSet;
import com.android.gallery3d.data.DataManager;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.MediaObject;
import com.android.gallery3d.data.MediaSet;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.filtershow.crop.CropActivity;
import com.android.gallery3d.util.Future;
import com.android.gallery3d.util.GalleryUtils;
import com.android.gallery3d.util.ThreadPool.Job;
import com.android.gallery3d.util.ThreadPool.JobContext;

import java.util.ArrayList;

public class MenuExecutor {
    private static final String TAG = "Gallery2/MenuExecutor";

    private static final int MSG_TASK_COMPLETE = 1;
    private static final int MSG_TASK_UPDATE = 2;
    private static final int MSG_TASK_START = 3;
    private static final int MSG_DO_SHARE = 4;

    public static final int EXECUTION_RESULT_SUCCESS = 1;
    public static final int EXECUTION_RESULT_FAIL = 2;
    public static final int EXECUTION_RESULT_CANCEL = 3;

    private ProgressDialog mDialog;
    private Future<?> mTask;
    // wait the operation to finish when we want to stop it.
    private boolean mWaitOnStop;
    private boolean mPaused;

    private final AbstractGalleryActivity mActivity;
    private final SelectionManager mSelectionManager;
    private final Handler mHandler;

    /// M: [BUG.ADD] @{
    private volatile boolean mIsMultiOperation;
    private volatile boolean mHasCancelMultiOperation;
    private boolean mIsOnDestory = false;
    private MediaOperation mMediaOperation = null;
    /// @}

    /// M: [BUG.MODIFY] @{
    /*
    private static ProgressDialog createProgressDialog(
            Context context, int titleId, int progressMax) {
     */
    private ProgressDialog createProgressDialog(
            Context context, int titleId, int progressMax) {
        /// @}
        ProgressDialog dialog = new ProgressDialog(context);
        dialog.setTitle(titleId);
        dialog.setMax(progressMax);
        dialog.setCancelable(false);
        dialog.setIndeterminate(false);
        /// M: [BUG.ADD] @{
        //while mIsMultiOperation is true,should add "cancel" button for stopping the operation.
        if (mIsMultiOperation) {
            dialog.setButton(context.getString(R.string.cancel), new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    // TODO Auto-generated method stub
                    mHasCancelMultiOperation = true;
                }
            });
        }
        /// @}
        if (progressMax > 1) {
            dialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        }
        return dialog;
    }

    public interface ProgressListener {
        public void onConfirmDialogShown();
        public void onConfirmDialogDismissed(boolean confirmed);
        public void onProgressStart();
        public void onProgressUpdate(int index);
        public void onProgressComplete(int result);
    }

    public MenuExecutor(
            AbstractGalleryActivity activity, SelectionManager selectionManager) {
        mActivity = Utils.checkNotNull(activity);
        mSelectionManager = Utils.checkNotNull(selectionManager);
        mHandler = new SynchronizedHandler(mActivity.getGLRoot()) {
            @Override
            public void handleMessage(Message message) {
                switch (message.what) {
                    case MSG_TASK_START: {
                        if (message.obj != null) {
                            ProgressListener listener = (ProgressListener) message.obj;
                            listener.onProgressStart();
                        }
                        break;
                    }
                    case MSG_TASK_COMPLETE: {
                        /// M: [BUG.MODIFY] if mTask be modified, we can not cancel mTask @{
                        //stopTaskAndDismissDialog();
                        stopTaskAndDismissDialog(message.arg2);
                        /// @}
                        if (message.obj != null) {
                            ProgressListener listener = (ProgressListener) message.obj;
                            listener.onProgressComplete(message.arg1);
                        }
                        mSelectionManager.leaveSelectionMode();
                        break;
                    }
                    case MSG_TASK_UPDATE: {
                        if (mDialog != null && !mPaused) mDialog.setProgress(message.arg1);
                        if (message.obj != null) {
                            ProgressListener listener = (ProgressListener) message.obj;
                            listener.onProgressUpdate(message.arg1);
                        }
                        break;
                    }
                    case MSG_DO_SHARE: {
                        ((Activity) mActivity).startActivity((Intent) message.obj);
                        break;
                    }
                }
            }
        };
    }

    /// M: [BUG.MODIFY] if mTask be modified, we can not cancel mTask @{
    /*private void stopTaskAndDismissDialog() {*/
    private void stopTaskAndDismissDialog(int jobHashCode) {
        if (jobHashCode != 0 && mMediaOperation != null
                && mMediaOperation.hashCode() != jobHashCode) {
            Log.d(TAG, "<stopTaskAndDismissDialog> jobHashCode : " + jobHashCode
                    + ", mMediaOperation : " + mMediaOperation + ", mMediaOperation.hashCode() : "
                    + mMediaOperation.hashCode());
            return;
        }
        /// @}
        /// M: [BUG.ADD] @{
        // if mIsMultiOperation == true ,should not stop the task. so after press
        // home key. The task still run on background.
        if (mIsMultiOperation && !mIsOnDestory) {
            return;
        }
        /// @}
        if (mTask != null) {
            if (!mWaitOnStop) mTask.cancel();
            if (mDialog != null && mDialog.isShowing()) {
                /// M: [BUG.MODIFY] The windows maybe lost @{
                /*  mDialog.dismiss();
                 */
                try {
                    mDialog.dismiss();
                } catch (IllegalArgumentException e) {
                    Log.e(TAG, "While stopTaskAndDismissDialog catch :", e);
                }
                /// @}
            }
            mDialog = null;
            mTask = null;
        }
    }

    public void resume() {
        mPaused = false;
        if (mDialog != null) mDialog.show();
    }

    public void pause() {
        mPaused = true;
        if (mDialog != null && mDialog.isShowing()) mDialog.hide();
    }

    public void destroy() {
        /// M: [BUG.ADD]  on destroy state should dismiss Dialog.@{
        mIsOnDestory = true;
        /// @}
        /// M: [BUG.MODIFY] if mTask be modified, we can not cancel mTask @{
        /*stopTaskAndDismissDialog();*/
        stopTaskAndDismissDialog(0);
        /// @}
        /// M: [BUG.ADD]@{
        mIsOnDestory = false;
        /// @}
    }

    private void onProgressUpdate(int index, ProgressListener listener) {
        mHandler.sendMessage(
                mHandler.obtainMessage(MSG_TASK_UPDATE, index, 0, listener));
    }

    private void onProgressStart(ProgressListener listener) {
        mHandler.sendMessage(mHandler.obtainMessage(MSG_TASK_START, listener));
    }

    /// M: [BUG.MODIFY] if mTask be modified, we can not cancel mTask @{
    /*private void onProgressComplete(int result, ProgressListener listener) {
     mHandler.sendMessage(mHandler.obtainMessage(MSG_TASK_COMPLETE, result, 0, listener));
     }*/
    private void onProgressComplete(int result, ProgressListener listener, int jobHashCode) {
        mHandler.sendMessage(mHandler.obtainMessage(MSG_TASK_COMPLETE, result, jobHashCode,
                listener));
    }
    /// @}

    public static void updateMenuOperation(Menu menu, int supported) {
        boolean supportDelete = (supported & MediaObject.SUPPORT_DELETE) != 0;
        boolean supportRotate = (supported & MediaObject.SUPPORT_ROTATE) != 0;
        boolean supportCrop = (supported & MediaObject.SUPPORT_CROP) != 0;
        boolean supportTrim = (supported & MediaObject.SUPPORT_TRIM) != 0;
        boolean supportMute = (supported & MediaObject.SUPPORT_MUTE) != 0;
        boolean supportShare = (supported & MediaObject.SUPPORT_SHARE) != 0;
        boolean supportSetAs = (supported & MediaObject.SUPPORT_SETAS) != 0;
        boolean supportShowOnMap = (supported & MediaObject.SUPPORT_SHOW_ON_MAP) != 0;
        boolean supportCache = (supported & MediaObject.SUPPORT_CACHE) != 0;
        boolean supportEdit = (supported & MediaObject.SUPPORT_EDIT) != 0;
        boolean supportInfo = (supported & MediaObject.SUPPORT_INFO) != 0;
        boolean supportPrint = (supported & MediaObject.SUPPORT_PRINT) != 0;

        supportPrint &= PrintHelper.systemSupportsPrint();

        setMenuItemVisible(menu, R.id.action_delete, supportDelete);
        setMenuItemVisible(menu, R.id.action_rotate_ccw, supportRotate);
        setMenuItemVisible(menu, R.id.action_rotate_cw, supportRotate);
        setMenuItemVisible(menu, R.id.action_crop, supportCrop);
        setMenuItemVisible(menu, R.id.action_trim, supportTrim);
        setMenuItemVisible(menu, R.id.action_mute, supportMute);
        // Hide panorama until call to updateMenuForPanorama corrects it
        setMenuItemVisible(menu, R.id.action_share_panorama, false);
        setMenuItemVisible(menu, R.id.action_share, supportShare);
        setMenuItemVisible(menu, R.id.action_setas, supportSetAs);
        setMenuItemVisible(menu, R.id.action_show_on_map, supportShowOnMap);
        setMenuItemVisible(menu, R.id.action_edit, supportEdit);
        // setMenuItemVisible(menu, R.id.action_simple_edit, supportEdit);
        setMenuItemVisible(menu, R.id.action_details, supportInfo);
        setMenuItemVisible(menu, R.id.print, supportPrint);
    }

    public static void updateMenuForPanorama(Menu menu, boolean shareAsPanorama360,
            boolean disablePanorama360Options) {
        setMenuItemVisible(menu, R.id.action_share_panorama, shareAsPanorama360);
        if (disablePanorama360Options) {
            setMenuItemVisible(menu, R.id.action_rotate_ccw, false);
            setMenuItemVisible(menu, R.id.action_rotate_cw, false);
        }
    }

    private static void setMenuItemVisible(Menu menu, int itemId, boolean visible) {
        MenuItem item = menu.findItem(itemId);
        if (item != null) item.setVisible(visible);
    }

    private Path getSingleSelectedPath() {
        ArrayList<Path> ids = mSelectionManager.getSelected(true);
        Utils.assertTrue(ids.size() == 1);
        return ids.get(0);
    }

    private Intent getIntentBySingleSelectedPath(String action) {
        DataManager manager = mActivity.getDataManager();
        Path path = getSingleSelectedPath();
        String mimeType = getMimeType(manager.getMediaType(path));
        return new Intent(action).setDataAndType(manager.getContentUri(path), mimeType);
    }

    private void onMenuClicked(int action, ProgressListener listener) {
        onMenuClicked(action, listener, false, true);
    }

    public void onMenuClicked(int action, ProgressListener listener,
            boolean waitOnStop, boolean showDialog) {
        int title;
        switch (action) {
            case R.id.action_select_all:
                if (mSelectionManager.inSelectAllMode()) {
                    mSelectionManager.deSelectAll();
                } else {
                    mSelectionManager.selectAll();
                }
                return;
            case R.id.action_crop: {
                Intent intent = getIntentBySingleSelectedPath(CropActivity.CROP_ACTION);
                ((Activity) mActivity).startActivity(intent);
                return;
            }
            case R.id.action_edit: {
                Intent intent = getIntentBySingleSelectedPath(Intent.ACTION_EDIT)
                        .setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                /// M: [BUG.ADD] create new task when launch photo editor @{
                // same behavior to that in PhotoPage @{
                intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
                                | Intent.FLAG_ACTIVITY_CLEAR_TOP
                                | Intent.FLAG_ACTIVITY_NEW_TASK);
                /// @}
                ((Activity) mActivity).startActivity(Intent.createChooser(intent, null));
                return;
            }
            case R.id.action_setas: {
                Intent intent = getIntentBySingleSelectedPath(Intent.ACTION_ATTACH_DATA)
                        .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                intent.putExtra("mimeType", intent.getType());
                Activity activity = mActivity;
                activity.startActivity(Intent.createChooser(
                        intent, activity.getString(R.string.set_as)));
                return;
            }
            case R.id.action_delete:
                title = R.string.delete;
                break;
            case R.id.action_rotate_cw:
                title = R.string.rotate_right;
                break;
            case R.id.action_rotate_ccw:
                title = R.string.rotate_left;
                break;
            case R.id.action_show_on_map:
                title = R.string.show_on_map;
                break;
            default:
                return;
        }
        startAction(action, title, listener, waitOnStop, showDialog);
    }

    private class ConfirmDialogListener implements OnClickListener, OnCancelListener {
        private final int mActionId;
        private final ProgressListener mListener;

        public ConfirmDialogListener(int actionId, ProgressListener listener) {
            mActionId = actionId;
            mListener = listener;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mListener != null) {
                    mListener.onConfirmDialogDismissed(true);
                }
                onMenuClicked(mActionId, mListener);
            } else {
                if (mListener != null) {
                    mListener.onConfirmDialogDismissed(false);
                }
            }
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            if (mListener != null) {
                mListener.onConfirmDialogDismissed(false);
            }
        }
    }

    public void onMenuClicked(MenuItem menuItem, String confirmMsg,
            final ProgressListener listener) {
        final int action = menuItem.getItemId();

        if (confirmMsg != null) {
            if (listener != null) listener.onConfirmDialogShown();
            ConfirmDialogListener cdl = new ConfirmDialogListener(action, listener);
            new AlertDialog.Builder(mActivity.getAndroidContext())
                    .setMessage(confirmMsg)
                    .setOnCancelListener(cdl)
                    .setPositiveButton(R.string.ok, cdl)
                    .setNegativeButton(R.string.cancel, cdl)
                    .create().show();
        } else {
            onMenuClicked(action, listener);
        }
    }

    public void startAction(int action, int title, ProgressListener listener) {
        startAction(action, title, listener, false, true);
    }

    public void startAction(int action, int title, ProgressListener listener,
            boolean waitOnStop, boolean showDialog) {
        ArrayList<Path> ids = mSelectionManager.getSelected(false);
        /// M: [BUG.MODIFY] if mTask be modified, we can not cancel mTask @{
        /*stopTaskAndDismissDialog();*/
        stopTaskAndDismissDialog(0);
        /// @}
        /// M: [BUG.ADD] @{
        // if ids.size() > 1, this is a multi operation.
        mIsMultiOperation = (ids.size() > 1) ? true : false;
        /// @}
        Activity activity = mActivity;
        if (showDialog) {
            mDialog = createProgressDialog(activity, title, ids.size());
            /// M: [BEHAVIOR.ADD] @{
            // when operation target is only one, show target name on progress
            // dialog
            appendMessageForSingleId(mDialog, ids);
            /// @}
            mDialog.show();
        } else {
            mDialog = null;
        }
        /// M: [BUG.MODIFY] if mTask be modified, we can not cancel mTask @{
        /*MediaOperation mediaOperation = new MediaOperation(action, ids, listener);
        mTask = mActivity.getBatchServiceThreadPoolIfAvailable().submit(mediaOperation, null);*/
        mMediaOperation = new MediaOperation(action, ids, listener);
        mTask = mActivity.getBatchServiceThreadPoolIfAvailable().submit(mMediaOperation, null);
        /// @}
        mWaitOnStop = waitOnStop;
    }

    public void startSingleItemAction(int action, Path targetPath) {
        ArrayList<Path> ids = new ArrayList<Path>(1);
        ids.add(targetPath);
        mDialog = null;
        /// M: [BUG.MODIFY] if mTask be modified, we can not cancel mTask @{
        /*MediaOperation operation = new MediaOperation(action, ids, null);
        mTask = mActivity.getBatchServiceThreadPoolIfAvailable().submit(operation, null);*/
        mMediaOperation = new MediaOperation(action, ids, null);
        mTask = mActivity.getBatchServiceThreadPoolIfAvailable().submit(mMediaOperation, null);
        /// @}
        mWaitOnStop = false;
    }

    public static String getMimeType(int type) {
        switch (type) {
            case MediaObject.MEDIA_TYPE_IMAGE :
                return GalleryUtils.MIME_TYPE_IMAGE;
            case MediaObject.MEDIA_TYPE_VIDEO :
                return GalleryUtils.MIME_TYPE_VIDEO;
            default: return GalleryUtils.MIME_TYPE_ALL;
        }
    }

    private boolean execute(
            DataManager manager, JobContext jc, int cmd, Path path) {
        boolean result = true;
        Log.v(TAG, "Execute cmd: " + cmd + " for " + path);
        long startTime = System.currentTimeMillis();

        switch (cmd) {
            case R.id.action_delete:
                manager.delete(path);
                break;
            case R.id.action_rotate_cw:
                manager.rotate(path, 90);
                break;
            case R.id.action_rotate_ccw:
                manager.rotate(path, -90);
                break;
            case R.id.action_toggle_full_caching: {
                MediaObject obj = manager.getMediaObject(path);
                int cacheFlag = obj.getCacheFlag();
                if (cacheFlag == MediaObject.CACHE_FLAG_FULL) {
                    cacheFlag = MediaObject.CACHE_FLAG_SCREENNAIL;
                } else {
                    cacheFlag = MediaObject.CACHE_FLAG_FULL;
                }
                obj.cache(cacheFlag);
                break;
            }
            case R.id.action_show_on_map: {
                MediaItem item = (MediaItem) manager.getMediaObject(path);
                double latlng[] = new double[2];
                item.getLatLong(latlng);
                if (GalleryUtils.isValidLocation(latlng[0], latlng[1])) {
                    GalleryUtils.showOnMap(mActivity, latlng[0], latlng[1]);
                }
                break;
            }
            default:
                throw new AssertionError();
        }
        Log.v(TAG, "It takes " + (System.currentTimeMillis() - startTime) +
                " ms to execute cmd for " + path);
        return result;
    }

    private class MediaOperation implements Job<Void> {
        private final ArrayList<Path> mItems;
        private final int mOperation;
        private final ProgressListener mListener;

        public MediaOperation(int operation, ArrayList<Path> items,
                ProgressListener listener) {
            mOperation = operation;
            mItems = items;
            mListener = listener;
        }

        @Override
        public Void run(JobContext jc) {
            int index = 0;
            DataManager manager = mActivity.getDataManager();
            int result = EXECUTION_RESULT_SUCCESS;
            /// M: [BUG.ADD] @{
            boolean isDelete = (mOperation == R.id.action_delete);
            /// @}
            /// M: [PERF.ADD] add for delete many files performance improve @{
            ActivityState topState = null;
            if (mActivity.getStateManager().getStateCount() >= 1) {
                topState = mActivity.getStateManager().getTopState();
                topState.setProviderSensive(false);
            }
            /// @}
            try {
                onProgressStart(mListener);
                for (Path id : mItems) {
                    /// M: [BUG.ADD] @{
                    // if mHasCancelMultiOperation is true, should break the operation.
                    if (jc.isCancelled() || mHasCancelMultiOperation) {
                        result = EXECUTION_RESULT_CANCEL;
                        break;
                    }
                    if (isDelete) {
                        if ("cluster".equals(id.getPrefix())) {
                            /// M: this is cluster object, use special logic during delete operation
                            // to avoid delete fail issue
                            Log.w(TAG, "deleting cluster, use special logic for culster object!");
                            ClusterAlbumSet.setClusterDeleteOperation(true);
                        }
                    }
                    /// @}
                    if (jc.isCancelled()) {
                        result = EXECUTION_RESULT_CANCEL;
                        break;
                    }
                    if (!execute(manager, jc, mOperation, id)) {
                        result = EXECUTION_RESULT_FAIL;
                    }
                    /// M: [BUG.MODIFY] while on pause should not send messager @{
                    /*
                     * onProgressUpdate(index++, mListener);
                    */
                    if (!mPaused) {
                        onProgressUpdate(index++, mListener);
                    }
                    /// @}
                }
            } catch (Throwable th) {
                Log.e(TAG, "failed to execute operation " + mOperation
                        + " : " + th);
            } finally {
                /// M: [BUG.ADD] @{
                if (isDelete) {
                    /// M: For cluster object delete operation. should refreshAll at last.
                    boolean isDeleteOperation = ClusterAlbumSet.getClusterDeleteOperation();
                    ClusterAlbumSet.setClusterDeleteOperation(false);
                    if (isDeleteOperation) {
                        Log.w(TAG, "deleting cluster complete, force reload all!");
                        manager.forceRefreshAll();
                    }
                }
                 mHasCancelMultiOperation = false;
                 mIsMultiOperation = false;
                /// @}
                 /// M: [BUG.MODIFY] if mTask be modified, we can not cancel mTask @{
                 /* onProgressComplete(result, mListener); */
                 onProgressComplete(result, mListener, this.hashCode());
                 /// @}
                 /// M: [PERF.ADD] add for delete many files performance improve @{
                if (topState != null) {
                    topState.setProviderSensive(true);
                    topState.fakeProviderChange();
                }
                 /// @}
            }
            return null;
        }
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    public static void updateSupportedMenuEnabled(Menu menu, int supported, boolean enabled) {
        boolean supportDelete = (supported & MediaObject.SUPPORT_DELETE) != 0;
        boolean supportRotate = (supported & MediaObject.SUPPORT_ROTATE) != 0;
        boolean supportCrop = (supported & MediaObject.SUPPORT_CROP) != 0;
        boolean supportShare = (supported & MediaObject.SUPPORT_SHARE) != 0;
        boolean supportSetAs = (supported & MediaObject.SUPPORT_SETAS) != 0;
        boolean supportShowOnMap = (supported & MediaObject.SUPPORT_SHOW_ON_MAP) != 0;
        boolean supportCache = (supported & MediaObject.SUPPORT_CACHE) != 0;
        boolean supportEdit = (supported & MediaObject.SUPPORT_EDIT) != 0;
        boolean supportInfo = (supported & MediaObject.SUPPORT_INFO) != 0;
        //add for Bluetooth Print feature
        boolean supportPrint = (supported & MediaObject.SUPPORT_PRINT) != 0;

        if (supportDelete) {
            setMenuItemEnable(menu, R.id.action_delete, enabled);
        }
        if (supportRotate) {
            setMenuItemEnable(menu, R.id.action_rotate_ccw, enabled);
            setMenuItemEnable(menu, R.id.action_rotate_cw, enabled);
        }
        if (supportCrop) {
            setMenuItemEnable(menu, R.id.action_crop, enabled);
        }
        if (supportShare) {
            setMenuItemEnable(menu, R.id.action_share, enabled);
        }
        if (supportSetAs) {
            setMenuItemEnable(menu, R.id.action_setas, enabled);
        }
        if (supportShowOnMap) {
            setMenuItemEnable(menu, R.id.action_show_on_map, enabled);
        }
        if (supportEdit) {
            setMenuItemEnable(menu, R.id.action_edit, enabled);
        }
        if (supportInfo) {
            setMenuItemEnable(menu, R.id.action_details, enabled);
        }
    }

    private static void setMenuItemEnable(
            Menu menu, int id, boolean enabled) {
        MenuItem item = menu.findItem(id);
        if (item != null) {
            item.setEnabled(enabled);
        }
    }

    /// M: [FEATURE.ADD] Support BlueTooth print feature.@{
    private final static String BT_PRINT_ACTION = "mediatek.intent.action.PRINT";
    private final static String IMAGE_MIME = "image/*";

    private void printImageViaBT(Uri uri) {
        if (uri == null) {
            Log.d(TAG, "<printImageViaBT> uri is null, return!!!");
            return;
        }
        Intent intent = new Intent();
        intent.setAction(BT_PRINT_ACTION);
        intent.addCategory(Intent.CATEGORY_ALTERNATIVE);
        intent.setType(IMAGE_MIME);
        intent.putExtra(Intent.EXTRA_STREAM, uri);
        try {
            Log.d(TAG, "<printImageViaBT> uri: " + uri);
            mActivity.startActivity(Intent.createChooser(intent,
                    mActivity.getText(R.string.print_image)));
        } catch (ActivityNotFoundException e) {
            Toast.makeText(mActivity, R.string.no_way_to_print,
                    Toast.LENGTH_SHORT).show();
        }
    }
    /// @}

    /// M: [BEHAVIOR.ADD] @{
    // when operation target is only one, show target name on progress
    // dialog
    private void appendMessageForSingleId(ProgressDialog dialog, ArrayList<Path> ids) {
        if (ids.size() == 1) {
            String message = null;
            MediaObject obj = mActivity.getDataManager().getMediaObject(ids.get(0));
            if (obj == null) {
                return;
            }
            if (obj instanceof MediaItem) {
                message = ((MediaItem) obj).getName();
            } else if (obj instanceof MediaSet) {
                message = ((MediaSet) obj).getName();
            }
            if (message != null && mDialog != null) {
                mDialog.setMessage(message);
            }
        }
    }
    /// @}
}
