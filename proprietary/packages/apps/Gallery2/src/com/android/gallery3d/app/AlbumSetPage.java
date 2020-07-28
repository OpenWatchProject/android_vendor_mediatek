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

package com.android.gallery3d.app;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.view.HapticFeedbackConstants;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.android.gallery3d.R;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.ClusterAlbumSet;
import com.android.gallery3d.data.DataManager;
import com.android.gallery3d.data.MediaDetails;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.MediaObject;
import com.android.gallery3d.data.MediaSet;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.glrenderer.GLCanvas;
import com.android.gallery3d.ui.ActionModeHandler;
import com.android.gallery3d.ui.ActionModeHandler.ActionModeListener;
import com.android.gallery3d.ui.AlbumSetSlotRenderer;
import com.android.gallery3d.ui.DetailsHelper;
import com.android.gallery3d.ui.DetailsHelper.CloseListener;
import com.android.gallery3d.ui.GLRoot;
import com.android.gallery3d.ui.GLView;
import com.android.gallery3d.ui.SelectionManager;
import com.android.gallery3d.ui.SlotView;
import com.android.gallery3d.ui.SynchronizedHandler;
import com.android.gallery3d.util.Future;
import com.android.gallery3d.util.GalleryUtils;
import com.android.gallery3d.util.HelpUtils;
import com.mediatek.gallery3d.layout.FancyHelper;
import com.mediatek.gallery3d.layout.Layout.DataChangeListener;
import com.mediatek.gallery3d.util.PermissionHelper;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/// M: [BUG.MODIFY] leave selection mode when plug out sdcard @{
/*
public class AlbumSetPage extends ActivityState implements
        SelectionManager.SelectionListener, GalleryActionBar.ClusterRunner,
        EyePosition.EyePositionListener, MediaSet.SyncListener {
*/
public class AlbumSetPage extends ActivityState implements
        SelectionManager.SelectionListener, GalleryActionBar.ClusterRunner,
        EyePosition.EyePositionListener, MediaSet.SyncListener,
        AbstractGalleryActivity.EjectListener {
/// @}
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/AlbumSetPage";

    private static final int MSG_PICK_ALBUM = 1;

    public static final String KEY_MEDIA_PATH = "media-path";
    public static final String KEY_SET_TITLE = "set-title";
    public static final String KEY_SET_SUBTITLE = "set-subtitle";
    public static final String KEY_SELECTED_CLUSTER_TYPE = "selected-cluster";

    private static final int DATA_CACHE_SIZE = 256;
    private static final int REQUEST_DO_ANIMATION = 1;

    private static final int BIT_LOADING_RELOAD = 1;
    private static final int BIT_LOADING_SYNC = 2;

    private boolean mIsActive = false;
    private SlotView mSlotView;
    private AlbumSetSlotRenderer mAlbumSetView;
    private Config.AlbumSetPage mConfig;

    private MediaSet mMediaSet;
    private String mTitle;
    private String mSubtitle;
    private boolean mShowClusterMenu;
    private GalleryActionBar mActionBar;
    private int mSelectedAction;

    protected SelectionManager mSelectionManager;
    private AlbumSetDataLoader mAlbumSetDataAdapter;

    private boolean mGetContent;
    private boolean mGetAlbum;
    private ActionModeHandler mActionModeHandler;
    private DetailsHelper mDetailsHelper;
    private MyDetailsSource mDetailsSource;
    private boolean mShowDetails;
    private EyePosition mEyePosition;
    private Handler mHandler;

    // The eyes' position of the user, the origin is at the center of the
    // device and the unit is in pixels.
    private float mX;
    private float mY;
    private float mZ;

    private Future<Integer> mSyncTask = null;

    private int mLoadingBits = 0;
    private boolean mInitialSynced = false;

    private Button mCameraButton;
    private boolean mShowedEmptyToastForSelf = false;
    /// M: [BUG.ADD]  if get the mTitle/mSubTitle,they will not change when switch language@{
    private int mClusterType = -1;
    /// @}

    /// M: [PERF.ADD] for performance auto test@{
    public boolean mLoadingFinished = false;
    public boolean mInitialized = false;
    /// @}

    @Override
    protected int getBackgroundColorId() {
        return R.color.albumset_background;
    }

    private final GLView mRootPane = new GLView() {
        private final float mMatrix[] = new float[16];

        @Override
        protected void onLayout(
                boolean changed, int left, int top, int right, int bottom) {
            mEyePosition.resetPosition();

            int slotViewTop = mActionBar.getHeight() + mConfig.paddingTop;
            int slotViewBottom = bottom - top - mConfig.paddingBottom;
            int slotViewRight = right - left;

            if (mShowDetails) {
                mDetailsHelper.layout(left, slotViewTop, right, bottom);
            } else {
                mAlbumSetView.setHighlightItemPath(null);
            }

            mSlotView.layout(0, slotViewTop, slotViewRight, slotViewBottom);
        }

        @Override
        protected void render(GLCanvas canvas) {
            canvas.save(GLCanvas.SAVE_FLAG_MATRIX);
            GalleryUtils.setViewPointMatrix(mMatrix,
                    getWidth() / 2 + mX, getHeight() / 2 + mY, mZ);
            canvas.multiplyMatrix(mMatrix, 0);
            super.render(canvas);
            canvas.restore();
        }
    };

    @Override
    public void onEyePositionChanged(float x, float y, float z) {
        mRootPane.lockRendering();
        mX = x;
        mY = y;
        mZ = z;
        /// M: [FEATURE.ADD] fancy layout @{
        if (FancyHelper.isFancyLayoutSupported() && !mActivity.isInMultiWindowMode()) {
            Log.d(TAG, "<onEyePositionChanged> <Fancy> enter");
            mIsInMultiWindowMode = false;
            // need to update screen width height for screen rotation
            DisplayMetrics reMetrics = getDisplayMetrics();
            FancyHelper.doFancyInitialization(reMetrics.widthPixels,
                    reMetrics.heightPixels);
            int layoutType = mEyePosition.getLayoutType();
            if (mLayoutType != layoutType) {
                mLayoutType = layoutType;
                // need clear content window cache before setting visible range
                // otherwise covers may not be loaded
                Log.d(TAG,
                        "<onEyePositionChanged> <Fancy> begin to switchLayout");
                mAlbumSetView.onEyePositionChanged(mLayoutType);
                mSlotView.switchLayout(mLayoutType);
            }
        }
        /// @}
        mRootPane.unlockRendering();
        mRootPane.invalidate();
    }

    @Override
    public void onBackPressed() {
        if (mShowDetails) {
            hideDetails();
        } else if (mSelectionManager.inSelectionMode()) {
            mSelectionManager.leaveSelectionMode();
        } else {
            super.onBackPressed();
        }
    }

    private void getSlotCenter(int slotIndex, int center[]) {
        Rect offset = new Rect();
        mRootPane.getBoundsOf(mSlotView, offset);
        Rect r = mSlotView.getSlotRect(slotIndex);
        int scrollX = mSlotView.getScrollX();
        int scrollY = mSlotView.getScrollY();
        center[0] = offset.left + (r.left + r.right) / 2 - scrollX;
        center[1] = offset.top + (r.top + r.bottom) / 2 - scrollY;
    }

    public void onSingleTapUp(int slotIndex) {
        if (!mIsActive) return;

        if (mSelectionManager.inSelectionMode()) {
            MediaSet targetSet = mAlbumSetDataAdapter.getMediaSet(slotIndex);
            if (targetSet == null) return; // Content is dirty, we shall reload soon
            if (mRestoreSelectionDone) {
                /// M: [BUG.ADD] fix menu display abnormal @{
                if (mActionModeHandler != null) {
                    mActionModeHandler.closeMenu();
                }
                /// @}
                mSelectionManager.toggle(targetSet.getPath());
                mSlotView.invalidate();
            } else {
                if (mWaitToast == null) {
                    mWaitToast = Toast.makeText(mActivity, R.string.wait, Toast.LENGTH_SHORT);
                }
                mWaitToast.show();
            }
        } else {
            /// M: [BUG.ADD] check slotIndex valid or not @{
            if (!mAlbumSetDataAdapter.isActive(slotIndex)) {
                Log.d(TAG, "<onSingleTapUp> slotIndex " + slotIndex
                        + " is not active, return!");
                return;
            }
            /// @}
            // Show pressed-up animation for the single-tap.
            mAlbumSetView.setPressedIndex(slotIndex);
            mAlbumSetView.setPressedUp();
            /// M: [PERF.MODIFY] send Message without 180ms delay for improving performance. @{
            //mHandler.sendMessageDelayed(mHandler.obtainMessage(
            //        MSG_PICK_ALBUM, slotIndex, 0), FadeTexture.DURATION);
            Log.d(TAG, "onSingleTapUp() at " + System.currentTimeMillis());
            mHandler.sendMessage(mHandler.obtainMessage(MSG_PICK_ALBUM, slotIndex, 0));
            mAlbumSetView.setPressedIndex(-1);
            /// @}
        }
    }

    private static boolean albumShouldOpenInFilmstrip(MediaSet album) {
        int itemCount = album.getMediaItemCount();
        ArrayList<MediaItem> list = (itemCount == 1) ? album.getMediaItem(0, 1) : null;
        // open in film strip only if there's one item in the album and the item exists
        return (list != null && !list.isEmpty());
    }

    WeakReference<Toast> mEmptyAlbumToast = null;

    private void showEmptyAlbumToast(int toastLength) {
        Toast toast;
        if (mEmptyAlbumToast != null) {
            toast = mEmptyAlbumToast.get();
            if (toast != null) {
                toast.show();
                return;
            }
        }
        toast = Toast.makeText(mActivity, R.string.empty_album, toastLength);
        mEmptyAlbumToast = new WeakReference<Toast>(toast);
        toast.show();
    }

    private void hideEmptyAlbumToast() {
        if (mEmptyAlbumToast != null) {
            Toast toast = mEmptyAlbumToast.get();
            if (toast != null) toast.cancel();
        }
    }

    private void pickAlbum(int slotIndex) {
        if (!mIsActive) return;
        /// M: [BUG.ADD] check if slotIndex is active before getMediaSet @{
        if (!mAlbumSetDataAdapter.isActive(slotIndex)) {
            return;
        }
        /// @}

        MediaSet targetSet = mAlbumSetDataAdapter.getMediaSet(slotIndex);
        if (targetSet == null) return; // Content is dirty, we shall reload soon
        if (targetSet.getTotalMediaItemCount() == 0) {
            showEmptyAlbumToast(Toast.LENGTH_SHORT);
            return;
        }
        hideEmptyAlbumToast();

        String mediaPath = targetSet.getPath().toString();

        Bundle data = new Bundle(getData());
        int[] center = new int[2];
        getSlotCenter(slotIndex, center);
        data.putIntArray(AlbumPage.KEY_SET_CENTER, center);
        if (mGetAlbum && targetSet.isLeafAlbum()) {
            Activity activity = mActivity;
            Intent result = new Intent()
                    .putExtra(AlbumPicker.KEY_ALBUM_PATH, targetSet.getPath().toString());
            activity.setResult(Activity.RESULT_OK, result);
            activity.finish();
        } else if (targetSet.getSubMediaSetCount() > 0) {
            data.putString(AlbumSetPage.KEY_MEDIA_PATH, mediaPath);
            mActivity.getStateManager().startStateForResult(
                    AlbumSetPage.class, REQUEST_DO_ANIMATION, data);
        } else {
            if (!mGetContent && albumShouldOpenInFilmstrip(targetSet)) {
                data.putParcelable(PhotoPage.KEY_OPEN_ANIMATION_RECT,
                        mSlotView.getSlotRect(slotIndex, mRootPane));
                data.putInt(PhotoPage.KEY_INDEX_HINT, 0);
                data.putString(PhotoPage.KEY_MEDIA_SET_PATH,
                        mediaPath);
                data.putBoolean(PhotoPage.KEY_START_IN_FILMSTRIP, true);
                data.putBoolean(PhotoPage.KEY_IN_CAMERA_ROLL, targetSet.isCameraRoll());
                /// M: [BUG.ADD] @{
                data.putString(PhotoPage.KEY_MEDIA_ITEM_PATH, null);
                /// @}
                mActivity.getStateManager().startStateForResult(
                        FilmstripPage.class, AlbumPage.REQUEST_PHOTO, data);
                /// M: [BUG.ADD] avoid show selected icon when back from album page @{
                mAlbumSetView.setPressedIndex(-1);
                /// @}
                return;
            }
            data.putString(AlbumPage.KEY_MEDIA_PATH, mediaPath);

            // We only show cluster menu in the first AlbumPage in stack
            boolean inAlbum = mActivity.getStateManager().hasStateClass(AlbumPage.class);
            data.putBoolean(AlbumPage.KEY_SHOW_CLUSTER_MENU, !inAlbum);
            mActivity.getStateManager().startStateForResult(
                    AlbumPage.class, REQUEST_DO_ANIMATION, data);
        }
    }

    private void onDown(int index) {
        mAlbumSetView.setPressedIndex(index);
    }

    private void onUp(boolean followedByLongPress) {
        if (followedByLongPress) {
            // Avoid showing press-up animations for long-press.
            mAlbumSetView.setPressedIndex(-1);
        } else {
            mAlbumSetView.setPressedUp();
        }
    }

    public void onLongTap(int slotIndex) {
        if (mGetContent || mGetAlbum) return;
        MediaSet set = mAlbumSetDataAdapter.getMediaSet(slotIndex);
        if (set == null) return;
        /// M: [BUG.ADD] fix menu display abnormal @{
        if (mActionModeHandler != null) {
            mActionModeHandler.closeMenu();
        }
        /// @}
        mSelectionManager.setAutoLeaveSelectionMode(true);
        mSelectionManager.toggle(set.getPath());
        mSlotView.invalidate();
    }

    @Override
    public void doCluster(int clusterType) {
        /// M: [FEATURE.ADD] [Runtime permission] @{
        if (clusterType == FilterUtils.CLUSTER_BY_LOCATION
                && !PermissionHelper.checkAndRequestForLocationCluster(mActivity)) {
            Log.d(TAG, "<doCluster> permission not granted");
            mNeedDoClusterType = clusterType;
            return;
        }
        /// @}
        String basePath = mMediaSet.getPath().toString();
        String newPath = FilterUtils.switchClusterPath(basePath, clusterType);
        Bundle data = new Bundle(getData());
        data.putString(AlbumSetPage.KEY_MEDIA_PATH, newPath);
        data.putInt(KEY_SELECTED_CLUSTER_TYPE, clusterType);
        mActivity.getStateManager().switchState(this, AlbumSetPage.class, data);
    }

    @Override
    public void onCreate(Bundle data, Bundle restoreState) {
        super.onCreate(data, restoreState);
        initializeViews();
        initializeData(data);

        /// M: [PERF.ADD] for performance auto test@{
        mInitialized = true;
        /// @}
        Context context = mActivity.getAndroidContext();
        mGetContent = data.getBoolean(GalleryActivity.KEY_GET_CONTENT, false);
        mGetAlbum = data.getBoolean(GalleryActivity.KEY_GET_ALBUM, false);
        mTitle = data.getString(AlbumSetPage.KEY_SET_TITLE);
        /// M: [BUG.MODIFY] @{
        // Get clustertype here, if get the mTitle/mSubTitle,
        // they will not change when switch language.
        //mSubtitle = data.getString(AlbumSetPage.KEY_SET_SUBTITLE);
        mClusterType = data.getInt(AlbumSetPage.KEY_SELECTED_CLUSTER_TYPE);
        /// @}
        mEyePosition = new EyePosition(context, this);
        mDetailsSource = new MyDetailsSource();
        mActionBar = mActivity.getGalleryActionBar();
        /// M: [FEATURE.ADD] fancy layout @{
        if (mSlotView != null) {
            mSlotView.setActionBar(mActionBar);
        }
        /// @}
        mSelectedAction = data.getInt(AlbumSetPage.KEY_SELECTED_CLUSTER_TYPE,
                FilterUtils.CLUSTER_BY_ALBUM);

        mHandler = new SynchronizedHandler(mActivity.getGLRoot()) {
            @Override
            public void handleMessage(Message message) {
                switch (message.what) {
                    case MSG_PICK_ALBUM: {
                        pickAlbum(message.arg1);
                        break;
                    }
                    default: throw new AssertionError(message.what);
                }
            }
        };
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        cleanupCameraButton();
        mActionModeHandler.destroy();
    }

    private boolean setupCameraButton() {
        if (!GalleryUtils.isCameraAvailable(mActivity)) return false;
        RelativeLayout galleryRoot = (RelativeLayout) ((Activity) mActivity)
                .findViewById(R.id.gallery_root);
        if (galleryRoot == null) return false;

        mCameraButton = new Button(mActivity);
        mCameraButton.setText(R.string.camera_label);
        mCameraButton.setCompoundDrawablesWithIntrinsicBounds(0, R.drawable.frame_overlay_gallery_camera, 0, 0);
        mCameraButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                GalleryUtils.startCameraActivity(mActivity);
            }
        });
        RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        lp.addRule(RelativeLayout.CENTER_IN_PARENT);
        galleryRoot.addView(mCameraButton, lp);
        return true;
    }

    private void cleanupCameraButton() {
        if (mCameraButton == null) return;
        RelativeLayout galleryRoot = (RelativeLayout) ((Activity) mActivity)
                .findViewById(R.id.gallery_root);
        if (galleryRoot == null) return;
        galleryRoot.removeView(mCameraButton);
        mCameraButton = null;
    }

    private void showCameraButton() {
        if (mCameraButton == null && !setupCameraButton()) return;
        mCameraButton.setVisibility(View.VISIBLE);
    }

    private void hideCameraButton() {
        if (mCameraButton == null) return;
        mCameraButton.setVisibility(View.GONE);
    }

    private void clearLoadingBit(int loadingBit) {
        mLoadingBits &= ~loadingBit;
        if (mLoadingBits == 0 && mIsActive) {
            if (mAlbumSetDataAdapter.size() == 0) {
                // If this is not the top of the gallery folder hierarchy,
                // tell the parent AlbumSetPage instance to handle displaying
                // the empty album toast, otherwise show it within this
                // instance
                if (mActivity.getStateManager().getStateCount() > 1) {
                    Intent result = new Intent();
                    result.putExtra(AlbumPage.KEY_EMPTY_ALBUM, true);
                    setStateResult(Activity.RESULT_OK, result);
                    mActivity.getStateManager().finishState(this);
                } else {
                    mShowedEmptyToastForSelf = true;
                    showEmptyAlbumToast(Toast.LENGTH_LONG);
                    mSlotView.invalidate();
                    showCameraButton();
                }
                return;
            }
        }
        // Hide the empty album toast if we are in the root instance of
        // AlbumSetPage and the album is no longer empty (for instance,
        // after a sync is completed and web albums have been synced)
        if (mShowedEmptyToastForSelf) {
            mShowedEmptyToastForSelf = false;
            hideEmptyAlbumToast();
            hideCameraButton();
        }
    }

    private void setLoadingBit(int loadingBit) {
        mLoadingBits |= loadingBit;
    }

    @Override
    public void onPause() {
        super.onPause();
        /// M: [BUG.ADD] when user exits from current page, UpdateContent() in data loader may
        // be executed in main handler, it may cause seldom JE. @{
        if (FancyHelper.isFancyLayoutSupported()) {
            mAlbumSetDataAdapter.setFancyDataChangeListener(null);
            Log.d(TAG, "<onPause> set FancyDataChangeListener as null");
        }
        /// @}
        mIsActive = false;
        /// M: [BEHAVIOR.ADD] @{
        if (mSelectionManager != null && mSelectionManager.inSelectionMode()) {
            mSelectionManager.saveSelection();
            mNeedUpdateSelection = false;
        }
        /// @}
        mAlbumSetDataAdapter.pause();
        mAlbumSetView.pause();
        mActionModeHandler.pause();
        mEyePosition.pause();
        DetailsHelper.pause();
        // Call disableClusterMenu to avoid receiving callback after paused.
        // Don't hide menu here otherwise the list menu will disappear earlier than
        // the action bar, which is janky and unwanted behavior.
        mActionBar.disableClusterMenu(false);
        if (mSyncTask != null) {
            mSyncTask.cancel();
            mSyncTask = null;
            clearLoadingBit(BIT_LOADING_SYNC);
        }
        /// M: [BUG.ADD] leave selection mode when plug out sdcard @{
        mActivity.setEjectListener(null);
        /// @}

        /// M: [FEATURE.ADD] Multi-window. @{
        mActivity.setMultiWindowModeListener(null);
        /// @}
    }

    @Override
    public void onResume() {
        super.onResume();
        /// M: [BUG.ADD] when user exits from current page, UpdateContent() in data loader may
        // be executed in main handler, it may cause seldom JE. @{
        if (FancyHelper.isFancyLayoutSupported()) {
            mAlbumSetDataAdapter.setFancyDataChangeListener((DataChangeListener) mSlotView);
            Log.d(TAG, "<onResume> reset FancyDataChangeListener");
        }
        /// @}
        mIsActive = true;
        setContentPane(mRootPane);

        // Set the reload bit here to prevent it exit this page in clearLoadingBit().
        setLoadingBit(BIT_LOADING_RELOAD);
        /// M: [BEHAVIOR.ADD] @{
        if (mSelectionManager != null && mSelectionManager.inSelectionMode()) {
            mNeedUpdateSelection = true;
            // set mRestoreSelectionDone as false if we need to restore selection
            mRestoreSelectionDone = false;
        } else {
            // set mRestoreSelectionDone as true there is no need to restore selection
            mRestoreSelectionDone = true;
        }
        /// @}
        mAlbumSetDataAdapter.resume();

        mAlbumSetView.resume();
        mEyePosition.resume();
        mActionModeHandler.resume();
        if (mShowClusterMenu) {
            mActionBar.enableClusterMenu(mSelectedAction, this);
        }
        if (!mInitialSynced) {
            setLoadingBit(BIT_LOADING_SYNC);
            mSyncTask = mMediaSet.requestSync(AlbumSetPage.this);
        }
        /// M: [BUG.ADD] leave selection mode when plug out sdcard @{
        mActivity.setEjectListener(this);
        /// @}
        /// M: [FEATURE.ADD] [Runtime permission] @{
        if (mClusterType == FilterUtils.CLUSTER_BY_LOCATION
                && !PermissionHelper.checkLocationPermission(mActivity)) {
            Log.d(TAG, "<onResume> CLUSTER_BY_LOCATION, permisison not granted, finish");
            PermissionHelper.showDeniedPrompt(mActivity);
            mActivity.getStateManager().finishState(AlbumSetPage.this);
            return;
        }
        /// @}

        /// M: [FEATURE.ADD] Multi-window. @{
        mActivity.setMultiWindowModeListener(mMultiWindowListener);
        mMultiWindowListener.onMultiWindowModeChanged(mActivity.isInMultiWindowMode());
        /// @}
    }

    private void initializeData(Bundle data) {
        String mediaPath = data.getString(AlbumSetPage.KEY_MEDIA_PATH);
        mMediaSet = mActivity.getDataManager().getMediaSet(mediaPath);
        mSelectionManager.setSourceMediaSet(mMediaSet);
        mAlbumSetDataAdapter = new AlbumSetDataLoader(
                mActivity, mMediaSet, DATA_CACHE_SIZE);
        mAlbumSetDataAdapter.setLoadingListener(new MyLoadingListener());
        mAlbumSetView.setModel(mAlbumSetDataAdapter);
        /// M: [FEATURE.ADD] fancy layout @{
        if (FancyHelper.isFancyLayoutSupported()) {
            mAlbumSetDataAdapter.setFancyDataChangeListener((DataChangeListener) mSlotView);
            // initialize fancy thumbnail size with DisplayMetrics
            FancyHelper.initializeFancyThumbnailSizes(getDisplayMetrics());
        }
        /// @}
    }

    private void initializeViews() {
        mSelectionManager = new SelectionManager(mActivity, true);
        mSelectionManager.setSelectionListener(this);

        mConfig = Config.AlbumSetPage.get(mActivity);
        /// M: [FEATURE.MODIFY] fancy layout @{
        // mSlotView = new SlotView(mActivity, mConfig.slotViewSpec);
        mSlotView = new SlotView(mActivity, mConfig.slotViewSpec,
                FancyHelper.isFancyLayoutSupported());
        if (FancyHelper.isFancyLayoutSupported()) {
            mSlotView.setPaddingSpec(mConfig.paddingTop, mConfig.paddingBottom);
            /// M: [FEATURE.ADD] Multi-window. @{
            Config.AlbumSetPage config = Config.AlbumSetPage.getConfigInMultiWindow(mActivity);
            mSlotView.setMultiWindowSpec(config.slotViewSpec);
            /// @}
        }
        /// @}
        mAlbumSetView = new AlbumSetSlotRenderer(
                mActivity, mSelectionManager, mSlotView, mConfig.labelSpec,
                mConfig.placeholderColor);
        mSlotView.setSlotRenderer(mAlbumSetView);
        mSlotView.setListener(new SlotView.SimpleListener() {
            @Override
            public void onDown(int index) {
                AlbumSetPage.this.onDown(index);
            }

            @Override
            public void onUp(boolean followedByLongPress) {
                AlbumSetPage.this.onUp(followedByLongPress);
            }

            @Override
            public void onSingleTapUp(int slotIndex) {
                AlbumSetPage.this.onSingleTapUp(slotIndex);
            }

            @Override
            public void onLongTap(int slotIndex) {
                AlbumSetPage.this.onLongTap(slotIndex);
            }
        });

        mActionModeHandler = new ActionModeHandler(mActivity, mSelectionManager);
        mActionModeHandler.setActionModeListener(new ActionModeListener() {
            @Override
            public boolean onActionItemClicked(MenuItem item) {
                return onItemSelected(item);
            }
            /// M: [BEHAVIOR.ADD] @{
            public boolean onPopUpItemClicked(int itemId) {
                // return if restoreSelection has done
                return mRestoreSelectionDone;
            }
            /// @}
        });
        mRootPane.addComponent(mSlotView);
    }

    @Override
    protected boolean onCreateActionBar(Menu menu) {
        Activity activity = mActivity;
        final boolean inAlbum = mActivity.getStateManager().hasStateClass(AlbumPage.class);
        MenuInflater inflater = getSupportMenuInflater();

        if (mGetContent) {
            inflater.inflate(R.menu.pickup, menu);
            int typeBits = mData.getInt(
                    GalleryActivity.KEY_TYPE_BITS, DataManager.INCLUDE_IMAGE);
            mActionBar.setTitle(GalleryUtils.getSelectionModePrompt(typeBits));
        } else  if (mGetAlbum) {
            inflater.inflate(R.menu.pickup, menu);
            mActionBar.setTitle(R.string.select_album);
        } else {
            inflater.inflate(R.menu.albumset, menu);
            boolean wasShowingClusterMenu = mShowClusterMenu;
            mShowClusterMenu = !inAlbum;
            /// M: [BUG.MODIFY] @{
            /* boolean selectAlbums = !inAlbum &&
                    mActionBar.getClusterTypeAction() == FilterUtils.CLUSTER_BY_ALBUM;
            MenuItem selectItem = menu.findItem(R.id.action_select);
            selectItem.setTitle(activity.getString(
                    selectAlbums ? R.string.select_album : R.string.select_group));
             */
            MenuItem selectItem = menu.findItem(R.id.action_select);
            if (selectItem != null) {
                int clusterType = (mSelectedAction != 0 ? mSelectedAction : mActionBar
                        .getClusterTypeAction());
                boolean selectAlbums = !inAlbum &&
                        clusterType == FilterUtils.CLUSTER_BY_ALBUM;
                if (selectAlbums) {
                    selectItem.setTitle(R.string.select_album);
                } else {
                    selectItem.setTitle(R.string.select_group);
                }
            }
            /// @}
            MenuItem cameraItem = menu.findItem(R.id.action_camera);
            cameraItem.setVisible(GalleryUtils.isCameraAvailable(activity));

            FilterUtils.setupMenuItems(mActionBar, mMediaSet.getPath(), false);

            Intent helpIntent = HelpUtils.getHelpIntent(activity);

            MenuItem helpItem = menu.findItem(R.id.action_general_help);
            helpItem.setVisible(helpIntent != null);
            if (helpIntent != null) helpItem.setIntent(helpIntent);
            /// M: [BUG.ADD] if get the mTitle/mSubTitle,they will not change @{
            // when switch language.
            if (mTitle != null) {
                mTitle = mMediaSet.getName();
                mSubtitle = GalleryActionBar.getClusterByTypeString(mActivity, mClusterType);
            }
            /// @}
            mActionBar.setTitle(mTitle);
            mActionBar.setSubtitle(mSubtitle);
            if (mShowClusterMenu != wasShowingClusterMenu) {
                if (mShowClusterMenu) {
                    mActionBar.enableClusterMenu(mSelectedAction, this);
                } else {
                    mActionBar.disableClusterMenu(true);
                }
            }
        }
        return true;
    }

    @Override
    protected boolean onItemSelected(MenuItem item) {
        Activity activity = mActivity;
        switch (item.getItemId()) {
            case R.id.action_cancel:
                activity.setResult(Activity.RESULT_CANCELED);
                activity.finish();
                return true;
            case R.id.action_select:
                mSelectionManager.setAutoLeaveSelectionMode(false);
                mSelectionManager.enterSelectionMode();
                return true;
            case R.id.action_details:
                if (mAlbumSetDataAdapter.size() != 0) {
                    if (mShowDetails) {
                        hideDetails();
                    } else {
                        showDetails();
                    }
                } else {
                    Toast.makeText(activity,
                            activity.getText(R.string.no_albums_alert),
                            Toast.LENGTH_SHORT).show();
                }
                return true;
            case R.id.action_camera: {
                GalleryUtils.startCameraActivity(activity);
                return true;
            }
            /// M: [BUG.MARK] @{
            //comment out unavailable options for non-GMS version Gallery2
            /*case R.id.action_manage_offline: {
             Bundle data = new Bundle();
             String mediaPath = mActivity.getDataManager().getTopSetPath(
             DataManager.INCLUDE_ALL);
             data.putString(AlbumSetPage.KEY_MEDIA_PATH, mediaPath);
             mActivity.getStateManager().startState(ManageCachePage.class, data);
             return true;
             }
             case R.id.action_sync_picasa_albums: {
             PicasaSource.requestSync(activity);
             return true;
             }
             case R.id.action_settings: {
             activity.startActivity(new Intent(activity, GallerySettings.class));
             return true;
             }*/
            /// @}
            default:
                return false;
        }
    }

    @Override
    protected void onStateResult(int requestCode, int resultCode, Intent data) {
        /// M: [BUG.MARK] no need show toast @{
        /*if (data != null && data.getBooleanExtra(AlbumPage.KEY_EMPTY_ALBUM, false)) {
         showEmptyAlbumToast(Toast.LENGTH_SHORT);
         }*/
        /// @}
        switch (requestCode) {
            case REQUEST_DO_ANIMATION: {
                mSlotView.startRisingAnimation();
            }
        }
    }

    /// M: [BUG.MODIFY] @{
    /* private String getSelectedString() { */
    public String getSelectedString() {
    /// @}
        int count = mSelectionManager.getSelectedCount();
        int action = mActionBar.getClusterTypeAction();
        int string = action == FilterUtils.CLUSTER_BY_ALBUM
                ? R.plurals.number_of_albums_selected
                : R.plurals.number_of_groups_selected;
        String format = mActivity.getResources().getQuantityString(string, count);
        return String.format(format, count);
    }

    @Override
    public void onSelectionModeChange(int mode) {
        switch (mode) {
            case SelectionManager.ENTER_SELECTION_MODE: {
                mActionBar.disableClusterMenu(true);
                mActionModeHandler.startActionMode();
                performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
                break;
            }
            case SelectionManager.LEAVE_SELECTION_MODE: {
                mActionModeHandler.finishActionMode();
                if (mShowClusterMenu) {
                    mActionBar.enableClusterMenu(mSelectedAction, this);
                }
                mRootPane.invalidate();
                break;
            }
            /// M: [BEHAVIOR.ADD] @{
            // when click deselect all in menu, not leave selection mode
            case SelectionManager.DESELECT_ALL_MODE:
            /// @}
            case SelectionManager.SELECT_ALL_MODE: {
                mActionModeHandler.updateSupportedOperation();
                mRootPane.invalidate();
                break;
            }
        }
    }

    @Override
    public void onSelectionChange(Path path, boolean selected) {
        mActionModeHandler.setTitle(getSelectedString());
        mActionModeHandler.updateSupportedOperation(path, selected);
    }

    private void hideDetails() {
        mShowDetails = false;
        mDetailsHelper.hide();
        mAlbumSetView.setHighlightItemPath(null);
        mSlotView.invalidate();
    }

    private void showDetails() {
        mShowDetails = true;
        if (mDetailsHelper == null) {
            mDetailsHelper = new DetailsHelper(mActivity, mRootPane, mDetailsSource);
            mDetailsHelper.setCloseListener(new CloseListener() {
                @Override
                public void onClose() {
                    hideDetails();
                }
            });
        }
        mDetailsHelper.show();
    }

    @Override
    public void onSyncDone(final MediaSet mediaSet, final int resultCode) {
        if (resultCode == MediaSet.SYNC_RESULT_ERROR) {
            Log.d(TAG, "onSyncDone: " + Utils.maskDebugInfo(mediaSet.getName()) + " result="
                    + resultCode);
        }
        ((Activity) mActivity).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                GLRoot root = mActivity.getGLRoot();
                root.lockRenderThread();
                try {
                    if (resultCode == MediaSet.SYNC_RESULT_SUCCESS) {
                        mInitialSynced = true;
                    }
                    clearLoadingBit(BIT_LOADING_SYNC);
                    if (resultCode == MediaSet.SYNC_RESULT_ERROR && mIsActive) {
                        Log.w(TAG, "failed to load album set");
                    }
                } finally {
                    root.unlockRenderThread();
                }
            }
        });
    }

    private class MyLoadingListener implements LoadingListener {
        @Override
        public void onLoadingStarted() {
            /// M: [PERF.ADD] for performance auto test@{
            mLoadingFinished = false;
            /// @}
            setLoadingBit(BIT_LOADING_RELOAD);
        }

        @Override
        public void onLoadingFinished(boolean loadingFailed) {
            /// M: [PERF.ADD] for performance auto test@{
            mLoadingFinished = true;
            /// @}
            clearLoadingBit(BIT_LOADING_RELOAD);
            /// M: [BEHAVIOR.ADD] @{
            // We have to notify SelectionManager about data change,
            // and this is the most proper place we could find till now
            boolean inSelectionMode = (mSelectionManager != null && mSelectionManager
                    .inSelectionMode());
            int setCount = mMediaSet != null ? mMediaSet.getSubMediaSetCount() : 0;
            Log.d(TAG, "<onLoadingFinished> set count=" + setCount);
            Log.d(TAG, "<onLoadingFinished> inSelectionMode=" + inSelectionMode);
            mSelectionManager.onSourceContentChanged();
            boolean restore = false;
            if (setCount > 0 && inSelectionMode) {
                if (mNeedUpdateSelection) {
                    mNeedUpdateSelection = false;
                    restore = true;
                    mSelectionManager.restoreSelection();
                }
                mActionModeHandler.updateSupportedOperation();
                mActionModeHandler.updateSelectionMenu();
            }
            if (!restore) {
                mRestoreSelectionDone = true;
            }
            /// @}
            /// M: [BUG.ADD] @{
            // ClusterAlbumSet name is designed to be localized, and the
            // localized name is calculated in reload(). Therefore we may obtain
            // a miss-localized name which is corresponding to the obsolete
            // (previous) Locale from ClusterAlbumSet if it is not finished
            // reloading. Here we re-get its name after it finished reloading,
            // and set the title of action bar to be the obtained name.
            if (mTitle != null && mActionBar != null && !inSelectionMode
                    && (mMediaSet instanceof ClusterAlbumSet)) {
                String title = mMediaSet.getName();
                String subtitle =
                        GalleryActionBar.getClusterByTypeString(mActivity, mClusterType);
                if (!mTitle.equalsIgnoreCase(title)
                        || (subtitle != null && !subtitle.equalsIgnoreCase(mSubtitle))) {
                    mTitle = title;
                    mSubtitle = subtitle;
                    mActionBar.setTitle(mTitle);
                    mActionBar.setSubtitle(mSubtitle);
                    Log.d(TAG, "<onLoadingFinished> mTitle:" + mTitle + "mSubtitle = "
                            + mSubtitle);
                    mActionBar.notifyDataSetChanged();
                }
            }
            /// @}
        }
    }

    private class MyDetailsSource implements DetailsHelper.DetailsSource {
        private int mIndex;

        @Override
        public int size() {
            return mAlbumSetDataAdapter.size();
        }

        @Override
        public int setIndex() {
            Path id = mSelectionManager.getSelected(false).get(0);
            mIndex = mAlbumSetDataAdapter.findSet(id);
            return mIndex;
        }

        @Override
        public MediaDetails getDetails() {
            MediaObject item = mAlbumSetDataAdapter.getMediaSet(mIndex);
            if (item != null) {
                mAlbumSetView.setHighlightItemPath(item.getPath());
                return item.getDetails();
            } else {
                return null;
            }
        }
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    // Flag to specify whether mSelectionManager.restoreSelection task has done
    private boolean mRestoreSelectionDone;
    // Save selection for onPause/onResume
    private boolean mNeedUpdateSelection = false;
    // If restore selection not done in selection mode,
    // after click one slot, show 'wait' toast
    private Toast mWaitToast = null;

    /// M: [BUG.ADD] leave selection mode when plug out sdcard @{
    @Override
    public void onEjectSdcard() {
        if (mSelectionManager != null && mSelectionManager.inSelectionMode()) {
            Log.d(TAG, "<onEjectSdcard> leaveSelectionMode");
            mSelectionManager.leaveSelectionMode();
        }
    }
    /// @}

    public void onSelectionRestoreDone() {
        if (!mIsActive) {
            return;
        }
        mRestoreSelectionDone = true;
        // Update selection menu after restore done @{
        mActionModeHandler.updateSupportedOperation();
        mActionModeHandler.updateSelectionMenu();
    }

    /// M: [FEATURE.ADD] fancy layout @{
    private int mLayoutType = -1;
    private DisplayMetrics getDisplayMetrics() {
        WindowManager wm = (WindowManager) mActivity.getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics reMetrics = new DisplayMetrics();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            wm.getDefaultDisplay().getRealMetrics(reMetrics);
        } else {
            wm.getDefaultDisplay().getMetrics(reMetrics);
        }
        Log.d(TAG, "<getDisplayMetrix> <Fancy> Display Metrics: " + reMetrics.widthPixels
                + " x " + reMetrics.heightPixels);
        return reMetrics;
    }
    /// @}

    /// M: [BUG.ADD] Save dataManager object.
    @Override
    protected void onSaveState(Bundle outState) {
        // keep record of current DataManager object.
        String dataManager = mActivity.getDataManager().toString();
        String processId = String.valueOf(android.os.Process.myPid());
        outState.putString(KEY_DATA_OBJECT, dataManager);
        outState.putString(KEY_PROCESS_ID, processId);
        Log.d(TAG, "<onSaveState> dataManager = " + dataManager
                + ", processId = " + processId);
    }
    /// @}

    /// M: [PERF.ADD] add for delete many files performance improve @{
    @Override
    public void setProviderSensive(boolean isProviderSensive) {
        mAlbumSetDataAdapter.setSourceSensive(isProviderSensive);
    }
    @Override
    public void fakeProviderChange() {
        mAlbumSetDataAdapter.fakeSourceChange();
    }
    /// @}

    /// M: [FEATURE.ADD] [Runtime permission] @{
    private int mNeedDoClusterType = 0;

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions,
            int[] grantResults) {
        if (PermissionHelper.isAllPermissionsGranted(permissions, grantResults)) {
            doCluster(mNeedDoClusterType);
        } else {
            PermissionHelper.showDeniedPromptIfNeeded(mActivity,
                    Manifest.permission.ACCESS_FINE_LOCATION);
        }
    }
    /// @}

    /// M: [FEATURE.ADD] Multi-window. @{
    private boolean mIsInMultiWindowMode = false;
    private MultiWindowListener mMultiWindowListener = new MultiWindowListener();

    /**
     * Use MultiWindowListener to monitor entering or leaving multi-window.
     */
    private class MultiWindowListener implements
            AbstractGalleryActivity.MultiWindowModeListener {

        @Override
        public void onMultiWindowModeChanged(boolean isInMultiWindowMode) {
            if (mIsInMultiWindowMode == isInMultiWindowMode) {
                return;
            }
            mRootPane.lockRendering();
            Log.d(TAG, "<onMultiWindowModeChanged> isInMultiWindowMode: "
                    + isInMultiWindowMode);
            mIsInMultiWindowMode = isInMultiWindowMode;
            if (mIsInMultiWindowMode) {
                Log.d(TAG, "<onMultiWindowModeChanged> switch to MULTI_WINDOW_LAYOUT");
                mLayoutType = FancyHelper.MULTI_WINDOW_LAYOUT;
                mAlbumSetView.onEyePositionChanged(mLayoutType);
                mSlotView.switchLayout(mLayoutType);
            } else {
                Log.d(TAG, "<onMultiWindowModeChanged> <Fancy> enter");
                DisplayMetrics reMetrics = getDisplayMetrics();
                FancyHelper.doFancyInitialization(reMetrics.widthPixels,
                        reMetrics.heightPixels);
                mLayoutType = mEyePosition.getLayoutType();
                Log.d(TAG, "<onMultiWindowModeChanged> <Fancy> begin to switchLayout");
                mAlbumSetView.onEyePositionChanged(mLayoutType);
                mSlotView.switchLayout(mLayoutType);
            }
            mRootPane.unlockRendering();
        }
    }
    /// @}
}
