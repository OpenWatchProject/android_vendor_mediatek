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

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.os.Handler;
import android.util.TypedValue;
import android.view.ActionMode;
import android.view.ActionMode.Callback;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ShareActionProvider;
import android.widget.Toast;

import com.android.gallery3d.R;
import com.android.gallery3d.app.AbstractGalleryActivity;
import com.android.gallery3d.app.ActivityState;
import com.android.gallery3d.app.AlbumSetPage;
import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.DataManager;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.MediaObject;
import com.android.gallery3d.data.MediaObject.PanoramaSupportCallback;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.ui.MenuExecutor.ProgressListener;
import com.android.gallery3d.util.Future;
import com.android.gallery3d.util.GalleryUtils;
import com.android.gallery3d.util.JobLimiter;
import com.android.gallery3d.util.ThreadPool.Job;
import com.android.gallery3d.util.ThreadPool.JobContext;

import com.mediatek.gallery3d.adapter.FeatureManager;
import com.mediatek.gallerybasic.base.IActionMode;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.galleryportable.ActivityChooserModelWrapper;
import com.mediatek.galleryportable.ActivityChooserModelWrapper.OnChooseActivityListenerWrapper;
import com.mediatek.galleryportable.ActivityChooserViewWrapper;

import java.util.ArrayList;

public class ActionModeHandler implements Callback, PopupList.OnPopupItemClickListener {

    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/ActionModeHandler";

    private static final int MAX_SELECTED_ITEMS_FOR_SHARE_INTENT = 300;
    private static final int MAX_SELECTED_ITEMS_FOR_PANORAMA_SHARE_INTENT = 10;

    private static final int SUPPORT_MULTIPLE_MASK = MediaObject.SUPPORT_DELETE
            | MediaObject.SUPPORT_ROTATE | MediaObject.SUPPORT_SHARE
            | MediaObject.SUPPORT_CACHE;

    public interface ActionModeListener {
        public boolean onActionItemClicked(MenuItem item);
        /// M: [BEHAVIOR.ADD] When return false, show wait @{
        public boolean onPopUpItemClicked(int itemId);
        /// @}
    }

    private final AbstractGalleryActivity mActivity;
    private final MenuExecutor mMenuExecutor;
    private final SelectionManager mSelectionManager;
    private final NfcAdapter mNfcAdapter;
    private Menu mMenu;
    private MenuItem mSharePanoramaMenuItem;
    private MenuItem mShareMenuItem;
    private ShareActionProvider mSharePanoramaActionProvider;
    private ShareActionProvider mShareActionProvider;
    private SelectionMenu mSelectionMenu;
    private ActionModeListener mListener;
    private Future<?> mMenuTask;
    private final Handler mMainHandler;
    private ActionMode mActionMode;
    /// M: [FEATURE.ADD] @{
    private ActivityChooserModelWrapper mDataModel;
    private ActivityChooserViewWrapper mActivityChooserView;
    /// @}
    /// M: [BUG.ADD] @{
    private JobLimiter mComputerShareItemsJobLimiter;
    /// @}

    private static class GetAllPanoramaSupports implements PanoramaSupportCallback {
        private int mNumInfoRequired;
        private JobContext mJobContext;
        public boolean mAllPanoramas = true;
        public boolean mAllPanorama360 = true;
        public boolean mHasPanorama360 = false;
        private Object mLock = new Object();

        public GetAllPanoramaSupports(ArrayList<MediaObject> mediaObjects, JobContext jc) {
            mJobContext = jc;
            mNumInfoRequired = mediaObjects.size();
            for (MediaObject mediaObject : mediaObjects) {
                mediaObject.getPanoramaSupport(this);
            }
        }

        @Override
        public void panoramaInfoAvailable(MediaObject mediaObject, boolean isPanorama,
                boolean isPanorama360) {
            synchronized (mLock) {
                mNumInfoRequired--;
                mAllPanoramas = isPanorama && mAllPanoramas;
                mAllPanorama360 = isPanorama360 && mAllPanorama360;
                mHasPanorama360 = mHasPanorama360 || isPanorama360;
                if (mNumInfoRequired == 0 || mJobContext.isCancelled()) {
                    mLock.notifyAll();
                }
            }
        }

        public void waitForPanoramaSupport() {
            synchronized (mLock) {
                while (mNumInfoRequired != 0 && !mJobContext.isCancelled()) {
                    try {
                        mLock.wait();
                    } catch (InterruptedException e) {
                        // May be a cancelled job context
                    }
                }
            }
        }
    }

    public ActionModeHandler(
            AbstractGalleryActivity activity, SelectionManager selectionManager) {
        mActivity = Utils.checkNotNull(activity);
        mSelectionManager = Utils.checkNotNull(selectionManager);
        mMenuExecutor = new MenuExecutor(activity, selectionManager);
        mMainHandler = new Handler(activity.getMainLooper());
        mNfcAdapter = NfcAdapter.getDefaultAdapter(mActivity.getAndroidContext());
    }

    public void startActionMode() {
        Activity a = mActivity;
        mActionMode = a.startActionMode(this);
        View customView = LayoutInflater.from(a).inflate(
                R.layout.action_mode, null);
        mActionMode.setCustomView(customView);
        mSelectionMenu = new SelectionMenu(a,
                (Button) customView.findViewById(R.id.selection_menu), this);
        updateSelectionMenu();
    }

    public void finishActionMode() {
        mActionMode.finish();
        /// M: [BUG.ADD] @{
        // Cancel menutask if action mode finish
        if (mMenuTask != null) {
            mMenuTask.cancel();
            mMenuTask = null;
        }
        /// @}
        /// M: [BUG.ADD] deselect set uri null @{
        setNfcBeamPushUris(null);
        /// @}
    }

    public void setTitle(String title) {
        mSelectionMenu.setTitle(title);
    }

    public void setActionModeListener(ActionModeListener listener) {
        mListener = listener;
    }

    private WakeLockHoldingProgressListener mDeleteProgressListener;

    @Override
    public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
        GLRoot root = mActivity.getGLRoot();
        root.lockRenderThread();
        try {
            boolean result;
            // Give listener a chance to process this command before it's routed to
            // ActionModeHandler, which handles command only based on the action id.
            // Sometimes the listener may have more background information to handle
            // an action command.
            if (mListener != null) {
                result = mListener.onActionItemClicked(item);
                if (result) {
                    mSelectionManager.leaveSelectionMode();
                    return result;
                }
                /// M: [FEATURE.ADD] @{
                for (IActionMode actionMode : mActionModeExt) {
                    result = actionMode.onActionItemClicked(mode, item);
                    if (result) {
                        mSelectionManager.leaveSelectionMode();
                        return result;
                    }
                }
                /// @}
            }
            ProgressListener listener = null;
            String confirmMsg = null;
            int action = item.getItemId();
            if (action == R.id.action_delete) {
                confirmMsg = mActivity.getResources().getQuantityString(
                        R.plurals.delete_selection, mSelectionManager.getSelectedCount());
                if (mDeleteProgressListener == null) {
                    mDeleteProgressListener = new WakeLockHoldingProgressListener(mActivity,
                            "Gallery Delete Progress Listener");
                }
                listener = mDeleteProgressListener;
            }
            mMenuExecutor.onMenuClicked(item, confirmMsg, listener);
        } finally {
            root.unlockRenderThread();
        }
        return true;
    }

    @Override
    public boolean onPopupItemClick(int itemId) {
        GLRoot root = mActivity.getGLRoot();
        root.lockRenderThread();
        try {
            if (itemId == R.id.action_select_all) {
                /// M: [FEATURE.MODIFY] @{
                /*
                 updateSupportedOperation();
                 mMenuExecutor.onMenuClicked(itemId, null, false, true);
                */
                if (mListener.onPopUpItemClicked(itemId)) {
                    mMenuExecutor.onMenuClicked(itemId, null, false, true);
                    updateSupportedOperation();
                    updateSelectionMenu();
                } else {
                    if (mWaitToast == null) {
                        mWaitToast = Toast.makeText(mActivity, R.string.wait, Toast.LENGTH_SHORT);
                    }
                    mWaitToast.show();
                }
                /// @}
            }
            return true;
        } finally {
            root.unlockRenderThread();
        }
    }

    /// M: [BUG.MODIFY] @{
    /*private void updateSelectionMenu() {*/
    public void updateSelectionMenu() {
    /// @}
        // update title
        int count = mSelectionManager.getSelectedCount();
        /// M: [BUG.MODIFY] @{
        /*
         String format = mActivity.getResources().getQuantityString(
         R.plurals.number_of_items_selected, count);
         setTitle(String.format(format, count));
        */
        /// @}
        // M: if current state is AlbumSetPage, title maybe albums/groups,
        // so getSelectedString from AlbumSetPage
        String title = null;
        ActivityState topState = null;
        // add empty state check to avoid JE
        if (mActivity.getStateManager().getStateCount() != 0) {
            topState = mActivity.getStateManager().getTopState();
        }
        if (topState != null && topState instanceof AlbumSetPage) {
            title = ((AlbumSetPage) topState).getSelectedString();
        } else {
            String format = mActivity.getResources().getQuantityString(
                    R.plurals.number_of_items_selected, count);
            title = String.format(format, count);
        }
        setTitle(title);

        // For clients who call SelectionManager.selectAll() directly, we need to ensure the
        // menu status is consistent with selection manager.
        mSelectionMenu.updateSelectAllMode(mSelectionManager.inSelectAllMode());
    }

    /// M: [FEATURE.MARK] @{
    /*private final OnShareTargetSelectedListener mShareTargetSelectedListener =
            new OnShareTargetSelectedListener() {
        @Override
        public boolean onShareTargetSelected(ShareActionProvider source, Intent intent) {
            /// M: [BEHAVIOR.ADD] @{
            Log.i(TAG, "<onShareTargetSelected> intent=" + intent);
            // if the intent is not ready intent, and show wait toast @{
            if (isNotReadyIntent(intent)) {
                showWaitToast();
                Log.i(TAG, "<onShareTargetSelected> still not ready, wait!");
                return true;
                // if current selected is more than 300, show toast @{
            } else if (isMoreThanMaxIntent(intent)) {
                Toast.makeText(mActivity, R.string.share_limit, Toast.LENGTH_SHORT).show();
                Log.i(TAG, "<onShareTargetSelected> shared too many item, abort!");
                return true;
            }
            /// @}
            mSelectionManager.leaveSelectionMode();
            return false;
        }
    };*/
    /// @}

    @Override
    public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
        /// M: [FEATURE.ADD] @{
        for (IActionMode actionMode : mActionModeExt) {
            actionMode.onPrepareActionMode(mode, menu);
        }
        /// @}
        return false;
    }

    @Override
    public boolean onCreateActionMode(ActionMode mode, Menu menu) {
        mode.getMenuInflater().inflate(R.menu.operation, menu);

        /// M: [FEATURE.ADD] @{
        if (mActionModeExt == null) {
            mActionModeExt =
                    (IActionMode[]) FeatureManager.getInstance().getImplement(
                            IActionMode.class, mActivity);
        }
        for (IActionMode actionMode : mActionModeExt) {
            actionMode.onCreateActionMode(mode, menu);
        }
        /// @}

        mMenu = menu;
        /// M: [BEHAVIOR.MARK] mask panorama share menu @{
        /*
        mSharePanoramaMenuItem = menu.findItem(R.id.action_share_panorama);
        if (mSharePanoramaMenuItem != null) {
            mSharePanoramaActionProvider = (ShareActionProvider) mSharePanoramaMenuItem
                .getActionProvider();
            mSharePanoramaActionProvider.setOnShareTargetSelectedListener(
                    mShareTargetSelectedListener);
            mSharePanoramaActionProvider.setShareHistoryFileName("panorama_share_history.xml");
        }
        */
        /// @}
        mShareMenuItem = menu.findItem(R.id.action_share);
        if (mShareMenuItem != null) {
            mShareActionProvider = (ShareActionProvider) mShareMenuItem
                .getActionProvider();
            /// M: [FEATURE.MODIFY] @{
            /*
            mShareActionProvider.setOnShareTargetSelectedListener(
                mShareTargetSelectedListener);
            mShareActionProvider.setShareHistoryFileName("share_history.xml");
            */
            mActivityChooserView = new ActivityChooserViewWrapper(mShareMenuItem);
            mShareActionProvider.setShareHistoryFileName("share_history.xml");
            mDataModel = new ActivityChooserModelWrapper(mActivity, "share_history.xml");
            if (mDataModel != null) {
                mDataModel.setOnChooseActivityListener(mChooseActivityListener);
            }
            /// @}
        }

        /// M: [FEATURE.ADD] Set the expand action icon resource. @{
        TypedValue outTypedValue = new TypedValue();
        mActivity.getTheme().resolveAttribute(
                mActivity.getResources().getIdentifier("actionModeShareDrawable", "attr",
                        "com.android.internal"), outTypedValue, true);
        mActivityChooserView.setExpandActivityOverflowButtonDrawable(mActivity
                .getApplicationContext().getResources().getDrawable(
                        R.drawable.ic_menu_share_holo_light));
        /// @}
        return true;
    }

    @Override
    public void onDestroyActionMode(ActionMode mode) {
        /// M: [FEATURE.ADD] @{
        for (IActionMode actionMode : mActionModeExt) {
            actionMode.onDestroyActionMode(mode);
        }
        /// @}
        mSelectionManager.leaveSelectionMode();
        /// M: [BUG.ADD] @{
        if (mSelectionMenu != null) {
            mSelectionMenu.finish();
        }
        /// @}
    }

    private ArrayList<MediaObject> getSelectedMediaObjects(JobContext jc) {
        ArrayList<Path> unexpandedPaths = mSelectionManager.getSelected(false);
        if (unexpandedPaths.isEmpty()) {
            // This happens when starting selection mode from overflow menu
            // (instead of long press a media object)
            return null;
        }
        ArrayList<MediaObject> selected = new ArrayList<MediaObject>();
        DataManager manager = mActivity.getDataManager();
        for (Path path : unexpandedPaths) {
            if (jc.isCancelled()) {
                return null;
            }
            selected.add(manager.getMediaObject(path));
        }

        return selected;
    }
    // Menu options are determined by selection set itself.
    // We cannot expand it because MenuExecuter executes it based on
    // the selection set instead of the expanded result.
    // e.g. LocalImage can be rotated but collections of them (LocalAlbum) can't.
    private int computeMenuOptions(ArrayList<MediaObject> selected) {
        /// M: [BUG.ADD] @{
        if (selected == null) {
            return 0;
        }
        /// @}
        int operation = MediaObject.SUPPORT_ALL;
        int type = 0;
        for (MediaObject mediaObject: selected) {
            int support = mediaObject.getSupportedOperations();
            type |= mediaObject.getMediaType();
            operation &= support;
        }

        switch (selected.size()) {
            case 1:
                final String mimeType = MenuExecutor.getMimeType(type);
                if (!GalleryUtils.isEditorAvailable(mActivity, mimeType)) {
                    operation &= ~MediaObject.SUPPORT_EDIT;
                }
                break;
            default:
                operation &= SUPPORT_MULTIPLE_MASK;
        }

        return operation;
    }

    @TargetApi(ApiHelper.VERSION_CODES.JELLY_BEAN)
    private void setNfcBeamPushUris(Uri[] uris) {
        /// M: [FEATURE.MODIFY] @{
        /*if (mNfcAdapter != null && ApiHelper.HAS_SET_BEAM_PUSH_URIS) {*/
        if (mNfcAdapter != null && !mActivity.isDestroyed() && ApiHelper.HAS_SET_BEAM_PUSH_URIS) {
            /// @}
            mNfcAdapter.setBeamPushUrisCallback(null, mActivity);
            mNfcAdapter.setBeamPushUris(uris, mActivity);
            /// M: [BUG.ADD] when no item selected, send NdefMessage for
            //launch other device gallery @{
            if (uris == null) {
                String pkgName = mActivity.getPackageName();
                NdefRecord appUri = NdefRecord.createUri(Uri
                        .parse("http://play.google.com/store/apps/details?id=" + pkgName
                                + "&feature=beam"));
                NdefRecord appRecord = NdefRecord.createApplicationRecord(pkgName);
                NdefMessage message = new NdefMessage(new NdefRecord[] {appUri, appRecord });
                mNfcAdapter.setNdefPushMessage(message, mActivity);
            }
            /// @}
        }
    }

    // Share intent needs to expand the selection set so we can get URI of
    // each media item
    private Intent computePanoramaSharingIntent(JobContext jc, int maxItems) {
        ArrayList<Path> expandedPaths = mSelectionManager.getSelected(true, maxItems);
        if (expandedPaths == null || expandedPaths.size() == 0) {
            return new Intent();
        }
        final ArrayList<Uri> uris = new ArrayList<Uri>();
        DataManager manager = mActivity.getDataManager();
        final Intent intent = new Intent();
        for (Path path : expandedPaths) {
            if (jc.isCancelled()) return null;
            uris.add(manager.getContentUri(path));
        }

        final int size = uris.size();
        if (size > 0) {
            if (size > 1) {
                intent.setAction(Intent.ACTION_SEND_MULTIPLE);
                intent.setType(GalleryUtils.MIME_TYPE_PANORAMA360);
                intent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, uris);
            } else {
                intent.setAction(Intent.ACTION_SEND);
                intent.setType(GalleryUtils.MIME_TYPE_PANORAMA360);
                intent.putExtra(Intent.EXTRA_STREAM, uris.get(0));
            }
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        }

        return intent;
    }

    private Intent computeSharingIntent(JobContext jc, int maxItems) {
        /// M: [BUG.MODIFY] In order to cancel getSelected quickly, using new function @{
        /* ArrayList<Path> expandedPaths = mSelectionManager.getSelected(true, maxItems); */
        ArrayList<Path> expandedPaths = mSelectionManager.getSelected(jc, true, maxItems);
        /// @}
        /// M: [BUG.ADD] @{
        if (jc.isCancelled()) {
            Log.d(TAG, "<computeSharingIntent> jc.isCancelled() - 1");
            return null;
        }
        /// @}
        /// M: [BUG.ADD] share 300 items at most @{
        if (expandedPaths == null) {
            setNfcBeamPushUris(null);
            Log.d(TAG, "<computeSharingIntent> selected items exceeds max number!");
            return createMoreThanMaxIntent();
        }
        /// @}
        if (expandedPaths == null || expandedPaths.size() == 0) {
            setNfcBeamPushUris(null);
            return new Intent();
        }
        /// M: [FEATURE.ADD] @{
        setCurrentSelectDatas(expandedPaths);
        for (IActionMode mode : mActionModeExt) {
            mode.onSelectionChange(mCurrentSelectDatas);
        }
        /// @}
        final ArrayList<Uri> uris = new ArrayList<Uri>();
        DataManager manager = mActivity.getDataManager();
        /// M: [BUG.ADD] Fix JE when sharing too many items @{
        int totalUriSize = 0;
        /// @}
        int type = 0;
        final Intent intent = new Intent();
        for (Path path : expandedPaths) {
            /// M: [DEBUG.MODIFY] @{
            /* if (jc.isCancelled()) return null; */
            /// @}
            if (jc.isCancelled()) {
                Log.d(TAG, "<computeSharingIntent> jc.isCancelled() - 2");
                return null;
            }
            int support = manager.getSupportedOperations(path);
            /// M: [BUG.MODIFY] @{
            /*type |= manager.getMediaType(path);*/
            /// @}

            if ((support & MediaObject.SUPPORT_SHARE) != 0) {
                uris.add(manager.getContentUri(path));
                /// M: [BUG.ADD] Fix JE when sharing too many items @{
                totalUriSize += manager.getContentUri(path).toString().length();
                // Only check type of media which support share
                type |= manager.getMediaType(path);
                /// @}
            }
            /// M: [BUG.ADD] Fix JE when sharing too many items @{
            if (totalUriSize > SHARE_URI_SIZE_LIMITATION) {
                Log.d(TAG, "<computeSharingIntent> totalUriSize > SHARE_URI_SIZE_LIMITATION");
                break;
            }
            /// @}
        }

        final int size = uris.size();
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<computeSharingIntent> total share items = " + size);
        /// @}
        if (size > 0) {
            final String mimeType = MenuExecutor.getMimeType(type);
            if (size > 1) {
                intent.setAction(Intent.ACTION_SEND_MULTIPLE).setType(mimeType);
                intent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, uris);
            } else {
                intent.setAction(Intent.ACTION_SEND).setType(mimeType);
                intent.putExtra(Intent.EXTRA_STREAM, uris.get(0));
            }
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            setNfcBeamPushUris(uris.toArray(new Uri[uris.size()]));
        } else {
            setNfcBeamPushUris(null);
            /// M: [BUG.ADD] @{
            return null;
            /// @}
        }

        return intent;
    }

    public void updateSupportedOperation(Path path, boolean selected) {
        // TODO: We need to improve the performance
        updateSupportedOperation();
    }

    public void updateSupportedOperation() {
        // Interrupt previous unfinished task, mMenuTask is only accessed in main thread
        /// M: [BUG.MODIFY] @{
        /* if (mMenuTask != null) mMenuTask.cancel();*/
        if (mMenuTask != null && !mMenuTask.isDone()) {
            mMenuTask.cancel();
        }
        /// @}
        updateSelectionMenu();

        // Disable share actions until share intent is in good shape
        if (mSharePanoramaMenuItem != null) mSharePanoramaMenuItem.setEnabled(false);
        if (mShareMenuItem != null) mShareMenuItem.setEnabled(false);

        /// M: [BUG.ADD] Replace old share intent with NotReadyIntent @{
        if (mShareActionProvider != null) {
            setShareIntent(createNotReadyIntent());
        }
        /// @}

        // Generate sharing intent and update supported operations in the background
        // The task can take a long time and be canceled in the mean time.
        /// M: [BUG.MODIFY] joblimiter replace threadpool, limit only one job in thread @{
        /*mMenuTask = mActivity.getThreadPool().submit(new Job<Void>() {*/
        if (mComputerShareItemsJobLimiter == null) {
            mComputerShareItemsJobLimiter = new JobLimiter(mActivity.getThreadPool(), 1);
        }
        mMenuTask = mComputerShareItemsJobLimiter.submit(new Job<Void>() {
        /// @}
            @Override
            public Void run(final JobContext jc) {
                // Pass1: Deal with unexpanded media object list for menu operation.
                /// M: [BUG.ADD] @{
                // Temporarily disable the menu to avoid mis-operation
                // during menu compute
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (jc.isCancelled()) {
                            return;
                        }
                        MenuExecutor.updateSupportedMenuEnabled(mMenu,
                                MediaObject.SUPPORT_ALL, false);
                    }
                });
                /// @}
                /// M: [BUG.MODIFY] @{
                /* ArrayList<MediaObject> selected = getSelectedMediaObjects(jc); */
                final ArrayList<MediaObject> selected = getSelectedMediaObjects(jc);
                /// @}
                if (selected == null) {
                    mMainHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            /// M: [BUG.MARK] @{
                            // Current task maybe not mMenuTask,
                            // so we do not set mMenuTask = null
                            /* mMenuTask = null; */
                            /// @}
                            if (jc.isCancelled()) return;
                            // Disable all the operations when no item is selected
                            MenuExecutor.updateMenuOperation(mMenu, 0);
                        }
                    });
                    /// M: [DEBUG.ADD] @{
                    Log.d(TAG, "<updateSupportedOperation> selected == null, task done, return");
                    /// @}
                    return null;
                }
                final int operation = computeMenuOptions(selected);
                if (jc.isCancelled()) {
                    /// M: [DEBUG.ADD] @{
                     Log.d(TAG, "<updateSupportedOperation> task is cancelled after " +
                             "computeMenuOptions, return");
                    /// @}
                    return null;
                }
                /// M: [BUG.ADD] @{
                final boolean supportShare = (operation & MediaObject.SUPPORT_SHARE) != 0;
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (jc.isCancelled()) {
                            return;
                        }
                        MenuExecutor.updateMenuOperation(mMenu, operation);
                        // Re-enable menu after compute and update finished
                        MenuExecutor.updateSupportedMenuEnabled(mMenu, MediaObject.SUPPORT_ALL,
                                true);
                        if (mShareMenuItem != null) {
                            if (selected == null || selected.size() == 0 || !supportShare) {
                                mShareMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
                                mShareMenuItem.setEnabled(false);
                                mShareMenuItem.setVisible(false);
                                setShareIntent(null);
                            } else {
                                mShareMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
                                mShareMenuItem.setEnabled(false);
                                mShareMenuItem.setVisible(true);
                                // When share intent is not ready, set it to INVALID_INTENT,
                                // when click share icon, set SHARE_TARGET_SELECTION_IGNORE_ACTION
                                // as true in onShareTargertSelected, and show a wait toast
                                // Add if condition to fix share history flash when selected items
                                // > 300
                                if (selected.size() <= MAX_SELECTED_ITEMS_FOR_SHARE_INTENT) {
                                    setShareIntent(createNotReadyIntent());
                                }
                            }
                        }
                }
                });
                if (mShareMenuItem == null || selected == null || selected.size() == 0) {
                    return null;
                }
                /// @}
                int numSelected = selected.size();
                final boolean canSharePanoramas =
                        numSelected < MAX_SELECTED_ITEMS_FOR_PANORAMA_SHARE_INTENT;
                /// M: [BUG.MODIFY] @{
                /*
                final boolean canShare =
                    numSelected < MAX_SELECTED_ITEMS_FOR_SHARE_INTENT;
                */
                final boolean canShare =
                    numSelected <= MAX_SELECTED_ITEMS_FOR_SHARE_INTENT;
                /// @}

                final GetAllPanoramaSupports supportCallback = canSharePanoramas ?
                        new GetAllPanoramaSupports(selected, jc)
                        : null;

                // Pass2: Deal with expanded media object list for sharing operation.
                final Intent share_panorama_intent = canSharePanoramas ?
                        computePanoramaSharingIntent(jc, MAX_SELECTED_ITEMS_FOR_PANORAMA_SHARE_INTENT)
                        : new Intent();
                /// M: [BUG.MODIFY] @{
                /*
                 final Intent share_intent = canShare ?
                        computeSharingIntent(jc, MAX_SELECTED_ITEMS_FOR_SHARE_INTENT)
                         : new Intent();
                */
                Log.d(TAG, "<updateSupportedOperation> computeSharingIntent begin");
                final Intent intent = canShare ?
                        computeSharingIntent(jc, MAX_SELECTED_ITEMS_FOR_SHARE_INTENT)
                        : createMoreThanMaxIntent();
                Log.d(TAG, "<updateSupportedOperation> computeSharingIntent end");
                /// @}

                /// M: [FEATURE.MARK] Not support this feature @{
                /*
                 if (canSharePanoramas) {
                 supportCallback.waitForPanoramaSupport(); }
                */
                /// @}
                if (jc.isCancelled()) {
                    /// M: [DEBUG.ADD] @{
                    Log.d(TAG, "<updateSupportedOperation> task is cancelled after " +
                            "computeSharingIntent, return");
                    /// @}
                    return null;
                }
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        /// M: [BUG.MARK] @{
                        // Current task maybe not mMenuTask,
                        // so we do not set mMenuTask = null
                        /* mMenuTask = null; */
                        if (jc.isCancelled()) return;
                        /// M: [BUG.MODIFY] @{
                        /*
                        MenuExecutor.updateMenuOperation(mMenu, operation);
                        MenuExecutor.updateMenuForPanorama(mMenu,
                                canSharePanoramas && supportCallback.mAllPanorama360,
                                canSharePanoramas && supportCallback.mHasPanorama360);
                        if (mSharePanoramaMenuItem != null) {
                            mSharePanoramaMenuItem.setEnabled(true);
                            if (canSharePanoramas && supportCallback.mAllPanorama360) {
                                mShareMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_NEVER);
                                mShareMenuItem.setTitle(
                                    mActivity.getResources().getString(R.string.share_as_photo));
                            } else {
                                mSharePanoramaMenuItem.setVisible(false);
                                mShareMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
                                mShareMenuItem.setTitle(
                                    mActivity.getResources().getString(R.string.share));
                            }
                            mSharePanoramaActionProvider.setShareIntent(share_panorama_intent);
                        }
                        if (mShareMenuItem != null) {
                            mShareMenuItem.setEnabled(canShare);
                            mShareActionProvider.setShareIntent(share_intent);
                        }
                        */
                        mShareMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
                        if (intent != null) {
                            mShareMenuItem.setEnabled(true);
                            mShareMenuItem.setVisible(true);
                        } else {
                            mShareMenuItem.setEnabled(false);
                            mShareMenuItem.setVisible(false);
                        }
                        setShareIntent(intent);
                        /// M: [BUG.ADD] close menu when computeSharingIntent end. @{
                        if (operation == 0
                                || operation == MediaObject.SUPPORT_DELETE
                                || operation == (MediaObject.SUPPORT_DELETE |
                                        MediaObject.SUPPORT_SHARE)) {
                            Log.d(TAG, "<updateSupportedOperation> close menu, " +
                                        "operation " + operation);
                            closeMenu();
                        }
                        /// @}
                        /// @}
                    }
                });
                /// M: [DEBUG.ADD] @{
                Log.d(TAG, "<updateSupportedOperation> task done, return");
                /// @}
                return null;
            }
        /// M: [BUG.MODIFY] @{
        /*});*/
        }, null);
        /// @}
    }

    public void pause() {
        if (mMenuTask != null) {
            mMenuTask.cancel();
            mMenuTask = null;
        }
        mMenuExecutor.pause();
        /// M: [BUG.ADD] @{
        // Disable menu on pause to avoid click but not response when resume
        if (mSelectionManager != null && mSelectionManager.inSelectionMode()) {
            MenuExecutor.updateSupportedMenuEnabled(mMenu,
                    MediaObject.SUPPORT_ALL, false);
        }
        /// @}
    }

    public void destroy() {
        mMenuExecutor.destroy();
    }

    public void resume() {
        /// M: [BUG.MODIFY] Send NdefMessage when not in selection mode@{
        /*if (mSelectionManager.inSelectionMode()) updateSupportedOperation();*/
        if (mSelectionManager.inSelectionMode()) {
            updateSupportedOperation();
        } else {
            setNfcBeamPushUris(null);
        }
        /// @}
        /// M: [BUG.ADD] @{
        // Resume share target select listener
        // in most cases, mShareTargetSelectedListener would be refreshed in onCreateActionMode()
        // but onCreateActionMode() could be missed invoking in
        // "select some items -> pause gallery -> resume gallery" operation sequence
        /*if (mShareActionProvider != null) {
            mShareActionProvider
                    .setOnShareTargetSelectedListener(mShareTargetSelectedListener);
        }*/
        if (mDataModel != null) {
            mDataModel.setOnChooseActivityListener(mChooseActivityListener);
        }
        /// @}
        mMenuExecutor.resume();
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    // Fix JE when sharing too many items
    private final static int SHARE_URI_SIZE_LIMITATION = 30000;
    // When not ready, show wait toast
    private Toast mWaitToast = null;

    private void showWaitToast() {
        if (mWaitToast == null) {
            mWaitToast = Toast.makeText(mActivity, R.string.wait, Toast.LENGTH_SHORT);
        }
        mWaitToast.show();
    }

    // Add for selected item > 300
    private static final String INTENT_MORE_THAN_MAX = "more than max";

    private Intent createMoreThanMaxIntent() {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_SEND_MULTIPLE).setType(
                GalleryUtils.MIME_TYPE_ALL);
        intent.putExtra(INTENT_MORE_THAN_MAX, true);
        return intent;
    }

    private boolean isMoreThanMaxIntent(Intent intent) {
        return null != intent.getExtras()
                && intent.getExtras().getBoolean(INTENT_MORE_THAN_MAX, false);
    }

    // Add for intent is not ready
    private static final String INTENT_NOT_READY = "intent not ready";

    private Intent createNotReadyIntent() {
        Intent intent = mIntent;
        if (intent == null) {
            intent = new Intent();
            intent.setAction(Intent.ACTION_SEND_MULTIPLE).setType(
                    GalleryUtils.MIME_TYPE_ALL);
        }
        intent.putExtra(INTENT_NOT_READY, true);
        return intent;
    }

    private boolean isNotReadyIntent(Intent intent) {
        return null != intent.getExtras()
                && intent.getExtras().getBoolean(INTENT_NOT_READY, false);
    }

    private OnChooseActivityListenerWrapper mChooseActivityListener =
            new OnChooseActivityListenerWrapper() {
                @Override
                public boolean onChooseActivity(ActivityChooserModelWrapper host,
                                                Intent intent) {
                    /// M: [BEHAVIOR.ADD] @{
                    Log.i(TAG, "<onChooseActivity> intent=" + intent);
                    // if the intent is not ready intent, and show wait toast
                    if (isNotReadyIntent(intent)) {
                        showWaitToast();
                        Log.i(TAG, "<onChooseActivity> still not ready, wait!");
                        return true;
                        // if current selected is more than 300, show toast
                    } else if (isMoreThanMaxIntent(intent)) {
                        Toast.makeText(mActivity, R.string.share_limit,
                                Toast.LENGTH_SHORT).show();
                        Log.i(TAG, "<onChooseActivity> shared too many item, abort!");
                        return true;
                    }
                    /// @}
                    Log.i(TAG, "<onChooseActivity> start share");
                    mActivity.startActivity(intent);
                    /// M: [BUG.MODIFY] NFC share not leave selection mode @{
                    // mSelectionManager.leaveSelectionMode();
                    if (!(intent.getComponent() != null && intent.getComponent()
                            .getPackageName().indexOf("nfc") != -1)) {
                        mSelectionManager.leaveSelectionMode();
                    }
                    /// @}
                    return true;
                }
            };

    /// M: [BUG.ADD] fix menu display abnormal @{
    public void closeMenu() {
        if (mMenu != null) {
            mMenu.close();
        }
    }
    /// @}

    /// M: [BUG.ADD] save last intent for not ready@{
    private Intent mIntent = null;

    private void setShareIntent(Intent intent) {
        if (intent != null) {
            mShareActionProvider.setShareIntent(intent);
            mIntent = (Intent) intent.clone();
        } else {
            mIntent = null;
        }
    }
    /// @}

    private IActionMode[] mActionModeExt;
    private MediaData[] mCurrentSelectDatas;

    private void setCurrentSelectDatas(ArrayList<Path> pathList) {
        ArrayList<MediaData> dataList = new ArrayList<MediaData>();
        DataManager manager = mActivity.getDataManager();
        MediaObject mediaObject;
        MediaData mediaData;
        for (Path path : pathList) {
            mediaObject = manager.getMediaObject(path);
            if (mediaObject instanceof MediaItem) {
                mediaData = ((MediaItem) (mediaObject)).getMediaData();
                if (mediaData != null) {
                    dataList.add(mediaData);
                }
            }
        }
        mCurrentSelectDatas = new MediaData[dataList.size()];
        dataList.toArray(mCurrentSelectDatas);
    }
}
