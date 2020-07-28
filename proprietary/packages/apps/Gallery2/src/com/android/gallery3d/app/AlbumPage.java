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
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore;
import android.view.HapticFeedbackConstants;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.Toast;

import com.android.gallery3d.R;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.ClusterAlbum;
import com.android.gallery3d.data.DataManager;
import com.android.gallery3d.data.MediaDetails;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.MediaObject;
import com.android.gallery3d.data.MediaSet;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.filtershow.crop.CropActivity;
import com.android.gallery3d.filtershow.crop.CropExtras;
import com.android.gallery3d.glrenderer.GLCanvas;
import com.android.gallery3d.ui.ActionModeHandler;
import com.android.gallery3d.ui.ActionModeHandler.ActionModeListener;
import com.android.gallery3d.ui.AlbumSlotRenderer;
import com.android.gallery3d.ui.DetailsHelper;
import com.android.gallery3d.ui.DetailsHelper.CloseListener;
import com.android.gallery3d.ui.GLRoot;
import com.android.gallery3d.ui.GLView;
import com.android.gallery3d.ui.PhotoFallbackEffect;
import com.android.gallery3d.ui.RelativePosition;
import com.android.gallery3d.ui.SelectionManager;
import com.android.gallery3d.ui.SlotView;
import com.android.gallery3d.ui.SynchronizedHandler;
import com.android.gallery3d.util.Future;
import com.android.gallery3d.util.GalleryUtils;
import com.android.gallery3d.util.MediaSetUtils;
import com.mediatek.gallery3d.layout.FancyHelper;
import com.mediatek.gallery3d.util.PermissionHelper;

import java.util.ArrayList;

/// M: [FEATURE.ADD] Gallery picker plugin @{

/// M: [BUG.MODIFY] leave selection mode when plug out sdcard @{
/*
public class AlbumPage extends ActivityState implements GalleryActionBar.ClusterRunner,
        SelectionManager.SelectionListener, MediaSet.SyncListener, GalleryActionBar.OnAlbumModeSelectedListener {
*/
public class AlbumPage extends ActivityState implements
        GalleryActionBar.ClusterRunner, SelectionManager.SelectionListener,
        MediaSet.SyncListener, GalleryActionBar.OnAlbumModeSelectedListener,
        AbstractGalleryActivity.EjectListener {
/// @}
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/AlbumPage";

    public static final String KEY_MEDIA_PATH = "media-path";
    public static final String KEY_PARENT_MEDIA_PATH = "parent-media-path";
    public static final String KEY_SET_CENTER = "set-center";
    public static final String KEY_AUTO_SELECT_ALL = "auto-select-all";
    public static final String KEY_SHOW_CLUSTER_MENU = "cluster-menu";
    public static final String KEY_EMPTY_ALBUM = "empty-album";
    public static final String KEY_RESUME_ANIMATION = "resume_animation";

    private static final int REQUEST_SLIDESHOW = 1;
    public static final int REQUEST_PHOTO = 2;
    private static final int REQUEST_DO_ANIMATION = 3;

    private static final int BIT_LOADING_RELOAD = 1;
    private static final int BIT_LOADING_SYNC = 2;

    private static final float USER_DISTANCE_METER = 0.3f;

    private boolean mIsActive = false;
    private AlbumSlotRenderer mAlbumView;
    private Path mMediaSetPath;
    private String mParentMediaSetString;
    private SlotView mSlotView;

    private AlbumDataLoader mAlbumDataAdapter;

    protected SelectionManager mSelectionManager;

    private boolean mGetContent;
    private boolean mShowClusterMenu;

    private ActionModeHandler mActionModeHandler;
    private int mFocusIndex = 0;
    private DetailsHelper mDetailsHelper;
    private MyDetailsSource mDetailsSource;
    private MediaSet mMediaSet;
    private boolean mShowDetails;
    private float mUserDistance; // in pixel
    private Future<Integer> mSyncTask = null;
    private boolean mLaunchedFromPhotoPage;
    private boolean mInCameraApp;
    private boolean mInCameraAndWantQuitOnPause;

    private int mLoadingBits = 0;
    private boolean mInitialSynced = false;
    private int mSyncResult;
    private boolean mLoadingFailed;
    private RelativePosition mOpenCenter = new RelativePosition();

    private Handler mHandler;
    private static final int MSG_PICK_PHOTO = 0;

    private PhotoFallbackEffect mResumeEffect;
    private PhotoFallbackEffect.PositionProvider mPositionProvider =
            new PhotoFallbackEffect.PositionProvider() {
        @Override
        public Rect getPosition(int index) {
            Rect rect = mSlotView.getSlotRect(index);
            Rect bounds = mSlotView.bounds();
            rect.offset(bounds.left - mSlotView.getScrollX(),
                    bounds.top - mSlotView.getScrollY());
            return rect;
        }

        @Override
        public int getItemIndex(Path path) {
            int start = mSlotView.getVisibleStart();
            int end = mSlotView.getVisibleEnd();
            for (int i = start; i < end; ++i) {
                MediaItem item = mAlbumDataAdapter.get(i);
                if (item != null && item.getPath() == path) return i;
            }
            return -1;
        }
    };

    @Override
    protected int getBackgroundColorId() {
        return R.color.album_background;
    }

    private final GLView mRootPane = new GLView() {
        private final float mMatrix[] = new float[16];

        @Override
        protected void onLayout(
                boolean changed, int left, int top, int right, int bottom) {

            int slotViewTop = mActivity.getGalleryActionBar().getHeight();
            int slotViewBottom = bottom - top;
            int slotViewRight = right - left;

            if (mShowDetails) {
                mDetailsHelper.layout(left, slotViewTop, right, bottom);
            } else {
                mAlbumView.setHighlightItemPath(null);
            }

            // Set the mSlotView as a reference point to the open animation
            mOpenCenter.setReferencePosition(0, slotViewTop);
            mSlotView.layout(0, slotViewTop, slotViewRight, slotViewBottom);
            GalleryUtils.setViewPointMatrix(mMatrix,
                    (right - left) / 2, (bottom - top) / 2, -mUserDistance);
        }

        @Override
        protected void render(GLCanvas canvas) {
            canvas.save(GLCanvas.SAVE_FLAG_MATRIX);
            canvas.multiplyMatrix(mMatrix, 0);
            super.render(canvas);

            if (mResumeEffect != null) {
                boolean more = mResumeEffect.draw(canvas);
                if (!more) {
                    mResumeEffect = null;
                    mAlbumView.setSlotFilter(null);
                }
                // We want to render one more time even when no more effect
                // required. So that the animated thumbnails could be draw
                // with declarations in super.render().
                invalidate();
            }
            canvas.restore();
        }
    };

    // This are the transitions we want:
    //
    // +--------+           +------------+    +-------+    +----------+
    // | Camera |---------->| Fullscreen |--->| Album |--->| AlbumSet |
    // |  View  | thumbnail |   Photo    | up | Page  | up |   Page   |
    // +--------+           +------------+    +-------+    +----------+
    //     ^                      |               |            ^  |
    //     |                      |               |            |  |         close
    //     +----------back--------+               +----back----+  +--back->  app
    //
    @Override
    protected void onBackPressed() {
        if (mShowDetails) {
            hideDetails();
        } else if (mSelectionManager.inSelectionMode()) {
            mSelectionManager.leaveSelectionMode();
        } else {
            if(mLaunchedFromPhotoPage) {
                mActivity.getTransitionStore().putIfNotPresent(
                        PhotoPage.KEY_ALBUMPAGE_TRANSITION,
                        PhotoPage.MSG_ALBUMPAGE_RESUMED);
            }
            // TODO: fix this regression
            // mAlbumView.savePositions(PositionRepository.getInstance(mActivity));
            if (mInCameraApp) {
                super.onBackPressed();
            } else {
                onUpPressed();
            }
        }
    }

    private void onUpPressed() {
        if (mInCameraApp) {
            GalleryUtils.startGalleryActivity(mActivity);
        } else if (mActivity.getStateManager().getStateCount() > 1) {
            super.onBackPressed();
        } else if (mParentMediaSetString != null) {
            Bundle data = new Bundle(getData());
            data.putString(AlbumSetPage.KEY_MEDIA_PATH, mParentMediaSetString);
            mActivity.getStateManager().switchState(
                    this, AlbumSetPage.class, data);
        }
    }

    private void onDown(int index) {
        mAlbumView.setPressedIndex(index);
    }

    private void onUp(boolean followedByLongPress) {
        if (followedByLongPress) {
            // Avoid showing press-up animations for long-press.
            mAlbumView.setPressedIndex(-1);
        } else {
            mAlbumView.setPressedUp();
        }
    }

    private void onSingleTapUp(int slotIndex) {
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<onSingleTapUp> slotIndex = " + slotIndex);
        /// @}
        if (!mIsActive) return;

        if (mSelectionManager.inSelectionMode()) {
            MediaItem item = mAlbumDataAdapter.get(slotIndex);
            if (item == null) return; // Item not ready yet, ignore the click
            /// M: [BUG.MODIFY] @{
            // If restore selection not done in selection mode,
            // after click one slot, show 'wait' toast
            // mSelectionManager.toggle(item.getPath());
            // mSlotView.invalidate();
            if (mRestoreSelectionDone) {
                /// M: [BUG.ADD] fix menu display abnormal @{
                if (mActionModeHandler != null) {
                    mActionModeHandler.closeMenu();
                }
                /// @}
                mSelectionManager.toggle(item.getPath());
                mSlotView.invalidate();
            } else {
                if (mWaitToast == null) {
                    mWaitToast = Toast.makeText(mActivity, R.string.wait, Toast.LENGTH_SHORT);
                }
                mWaitToast.show();
            }
            /// @}
        } else {
            // Render transition in pressed state
            mAlbumView.setPressedIndex(slotIndex);
            mAlbumView.setPressedUp();
            /// M: [PERF.MODIFY] send Message without 180ms delay for improving performance @{
            /*mHandler.sendMessageDelayed(mHandler.obtainMessage(MSG_PICK_PHOTO, slotIndex, 0),
                  FadeTexture.DURATION);*/
            mHandler.sendMessage(mHandler.obtainMessage(MSG_PICK_PHOTO, slotIndex, 0));
            /// @}
        }
    }

    private void pickPhoto(int slotIndex) {
        pickPhoto(slotIndex, false);
    }

    private void pickPhoto(int slotIndex, boolean startInFilmstrip) {
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<pickPhoto> slotIndex = " + slotIndex + ", startInFilmstrip = "
                + startInFilmstrip);
        /// @}
        if (!mIsActive) return;

        /// M: [BUG.MARK] @{
        /* if (!startInFilmstrip) {
            // Launch photos in lights out mode
            mActivity.getGLRoot().setLightsOutMode(true);
        }*/
        /// @}

        MediaItem item = mAlbumDataAdapter.get(slotIndex);
        if (item == null) return; // Item not ready yet, ignore the click

        /// M: [BUG.ADD] @{
        // setLightsOutMode after check if item is null
        if (!startInFilmstrip) {
            // Launch photos in lights out mode
            mActivity.getGLRoot().setLightsOutMode(true);
        }
        /// @}

        if (mGetContent) {
            onGetContent(item);
        } else if (mLaunchedFromPhotoPage) {
            TransitionStore transitions = mActivity.getTransitionStore();
            transitions.put(
                    PhotoPage.KEY_ALBUMPAGE_TRANSITION,
                    PhotoPage.MSG_ALBUMPAGE_PICKED);
            transitions.put(PhotoPage.KEY_INDEX_HINT, slotIndex);
            onBackPressed();
        } else {
            /// M:[FEATURE.ADD] play video directly. @{
            if (!startInFilmstrip && canBePlayed(item)) {
                Log.d(TAG, "<pickPhoto> item.getName()");
                playVideo(mActivity, item.getPlayUri(), item.getName());
                return;
            }
            /// @}
            // Get into the PhotoPage.
            // mAlbumView.savePositions(PositionRepository.getInstance(mActivity));
            Bundle data = new Bundle();
            data.putInt(PhotoPage.KEY_INDEX_HINT, slotIndex);
            data.putParcelable(PhotoPage.KEY_OPEN_ANIMATION_RECT,
                    mSlotView.getSlotRect(slotIndex, mRootPane));
            data.putString(PhotoPage.KEY_MEDIA_SET_PATH,
                    mMediaSetPath.toString());
            data.putString(PhotoPage.KEY_MEDIA_ITEM_PATH,
                    item.getPath().toString());
            data.putInt(PhotoPage.KEY_ALBUMPAGE_TRANSITION,
                    PhotoPage.MSG_ALBUMPAGE_STARTED);
            data.putBoolean(PhotoPage.KEY_START_IN_FILMSTRIP,
                    startInFilmstrip);
            data.putBoolean(PhotoPage.KEY_IN_CAMERA_ROLL, mMediaSet.isCameraRoll());
            /// M: [FEATURE.ADD] [Camera independent from Gallery] @{
            // Many times page switch lead to KEY_LAUNCH_FROM_CAMERA value changed.
            // So that PhotoPage behavior not be coincident.
            data.putBoolean(PhotoPage.KEY_LAUNCH_FROM_CAMERA, mInCameraApp);
            /// @}
            if (startInFilmstrip) {
                mActivity.getStateManager().switchState(this, FilmstripPage.class, data);
            } else {
                mActivity.getStateManager().startStateForResult(
                            SinglePhotoPage.class, REQUEST_PHOTO, data);
            }
        }
    }

    private void onGetContent(final MediaItem item) {
        DataManager dm = mActivity.getDataManager();
        Activity activity = mActivity;
        if (mData.getString(GalleryActivity.EXTRA_CROP) != null) {
            Uri uri = dm.getContentUri(item.getPath());
            Intent intent = new Intent(CropActivity.CROP_ACTION, uri)
                    .addFlags(Intent.FLAG_ACTIVITY_FORWARD_RESULT)
                    .putExtras(getData());
            if (mData.getParcelable(MediaStore.EXTRA_OUTPUT) == null) {
                intent.putExtra(CropExtras.KEY_RETURN_DATA, true);
            }
            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "<onGetContent> start CropActivity for extra crop, uri: " + uri);
            /// @}
            activity.startActivity(intent);
            activity.finish();
        } else {
            Intent intent = new Intent(null, item.getContentUri())
                .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            activity.setResult(Activity.RESULT_OK, intent);
            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "<onGetContent> return uri: " + item.getContentUri());
            /// @}
            activity.finish();
        }
    }

    public void onLongTap(int slotIndex) {
        if (mGetContent) return;
        MediaItem item = mAlbumDataAdapter.get(slotIndex);
        if (item == null) return;
        /// M: [BUG.ADD] fix menu display abnormal @{
        if (mActionModeHandler != null) {
            mActionModeHandler.closeMenu();
        }
        /// @}
        mSelectionManager.setAutoLeaveSelectionMode(true);
        mSelectionManager.toggle(item.getPath());
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
        String newPath = FilterUtils.newClusterPath(basePath, clusterType);
        Bundle data = new Bundle(getData());
        data.putString(AlbumSetPage.KEY_MEDIA_PATH, newPath);
        if (mShowClusterMenu) {
            Context context = mActivity.getAndroidContext();
            data.putString(AlbumSetPage.KEY_SET_TITLE, mMediaSet.getName());
            /// M: [BUG.MODIFY] @{
            // If get the mTitle/mSubTitle, they will not change when switch language.
            // data.putString(AlbumSetPage.KEY_SET_SUBTITLE,
            // GalleryActionBar.getClusterByTypeString(context, clusterType));
            data.putInt(AlbumSetPage.KEY_SELECTED_CLUSTER_TYPE, clusterType);
            /// @}
        }

        // mAlbumView.savePositions(PositionRepository.getInstance(mActivity));
        mActivity.getStateManager().startStateForResult(
                AlbumSetPage.class, REQUEST_DO_ANIMATION, data);
    }

    @Override
    protected void onCreate(Bundle data, Bundle restoreState) {
        super.onCreate(data, restoreState);
        mUserDistance = GalleryUtils.meterToPixel(USER_DISTANCE_METER);
        initializeViews();
        initializeData(data);
        mGetContent = data.getBoolean(GalleryActivity.KEY_GET_CONTENT, false);
        mShowClusterMenu = data.getBoolean(KEY_SHOW_CLUSTER_MENU, false);
        mDetailsSource = new MyDetailsSource();
        Context context = mActivity.getAndroidContext();

        if (data.getBoolean(KEY_AUTO_SELECT_ALL)) {
            mSelectionManager.selectAll();
        }

        mLaunchedFromPhotoPage =
                mActivity.getStateManager().hasStateClass(FilmstripPage.class);

        /// M: [FEATURE.MODIFY] [Camera independent from Gallery] @{
        /*mInCameraApp = data.getBoolean(PhotoPage.KEY_APP_BRIDGE, false);*/
        mInCameraApp = data.getBoolean(PhotoPage.KEY_LAUNCH_FROM_CAMERA, false);
        /// @}

        mHandler = new SynchronizedHandler(mActivity.getGLRoot()) {
            @Override
            public void handleMessage(Message message) {
                switch (message.what) {
                    case MSG_PICK_PHOTO: {
                        pickPhoto(message.arg1);
                        break;
                    }
                    default:
                        throw new AssertionError(message.what);
                }
            }
        };
    }

    @Override
    protected void onResume() {
        super.onResume();
        mIsActive = true;

        mResumeEffect = mActivity.getTransitionStore().get(KEY_RESUME_ANIMATION);
        if (mResumeEffect != null) {
            mAlbumView.setSlotFilter(mResumeEffect);
            mResumeEffect.setPositionProvider(mPositionProvider);
            mResumeEffect.start();
        }

        setContentPane(mRootPane);

        /// M: [BUG.MARK] put these code in onCreateActionBar @{
        /*
        boolean enableHomeButton = (mActivity.getStateManager().getStateCount() > 1) |
                mParentMediaSetString != null;
        GalleryActionBar actionBar = mActivity.getGalleryActionBar();
        actionBar.setDisplayOptions(enableHomeButton, false);
        if (!mGetContent) {
            actionBar.enableAlbumModeMenu(GalleryActionBar.ALBUM_GRID_MODE_SELECTED, this);
        }
        */
        /// @}

        // Set the reload bit here to prevent it exit this page in clearLoadingBit().
        setLoadingBit(BIT_LOADING_RELOAD);
        mLoadingFailed = false;
        /// M: [BUG.ADD] need to update selection manager if in selection mode when resume @{
        if (mSelectionManager.inSelectionMode()) {
            mNeedUpdateSelection = true;
            // set mRestoreSelectionDone as false if we need to retore selection
            mRestoreSelectionDone = false;
        } else {
            // set mRestoreSelectionDone as true there is no need to retore selection
            mRestoreSelectionDone = true;
        }
        /// @}
        mAlbumDataAdapter.resume();

        mAlbumView.resume();
        mAlbumView.setPressedIndex(-1);
        mActionModeHandler.resume();
        if (!mInitialSynced) {
            setLoadingBit(BIT_LOADING_SYNC);
            mSyncTask = mMediaSet.requestSync(this);
        }
        mInCameraAndWantQuitOnPause = mInCameraApp;
        /// M: [BUG.ADD] leave selection mode when plug out sdcard @{
        mActivity.setEjectListener(this);
        /// @}
    }

    @Override
    protected void onPause() {
        super.onPause();
        mIsActive = false;

        /// M: [BEHAVIOR.MARK] @{
        // Not exit selection mode when pause
        /*if (mSelectionManager.inSelectionMode()) {
         mSelectionManager.leaveSelectionMode();
         }*/
        /// @}

        mAlbumView.setSlotFilter(null);
        mActionModeHandler.pause();
        mAlbumDataAdapter.pause();
        mAlbumView.pause();
        DetailsHelper.pause();
        /// M: [BEHAVIOR.MODIFY] behavior change: display album title when share via BT @{
        /*
        if (!mGetContent) {
            mActivity.getGalleryActionBar().disableAlbumModeMenu(true);
        }
        */
        // need to remove AlbumModeListener when pause,
        // otherwise no response when doCluster in AlbumSetPage
        mActivity.getGalleryActionBar().removeAlbumModeListener();
        /// @}
        if (mSyncTask != null) {
            mSyncTask.cancel();
            mSyncTask = null;
            clearLoadingBit(BIT_LOADING_SYNC);
        }
        /// M: [BUG.ADD] leave selection mode when plug out sdcard @{
        mActivity.setEjectListener(null);
        /// @}
        /// M: [BUG.ADD] no need to update selection manager after pause @{
        if (mSelectionManager.inSelectionMode()) {
            mSelectionManager.saveSelection();
            mNeedUpdateSelection = false;
        }
        /// @}
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mAlbumDataAdapter != null) {
            mAlbumDataAdapter.setLoadingListener(null);
        }
        mActionModeHandler.destroy();
    }

    private void initializeViews() {
        mSelectionManager = new SelectionManager(mActivity, false);
        mSelectionManager.setSelectionListener(this);
        Config.AlbumPage config = Config.AlbumPage.get(mActivity);
        /// M: [FEATURE.MODIFY] fancy layout @{
        // mSlotView = new SlotView(mActivity, config.slotViewSpec);
        mSlotView = new SlotView(mActivity, config.slotViewSpec,
                FancyHelper.isFancyLayoutSupported());
        if (FancyHelper.isFancyLayoutSupported()) {
            mSlotView.switchLayout(FancyHelper.DEFAULT_LAYOUT);
        }
        /// @}

        mAlbumView = new AlbumSlotRenderer(mActivity, mSlotView,
                mSelectionManager, config.placeholderColor);
        mSlotView.setSlotRenderer(mAlbumView);
        mRootPane.addComponent(mSlotView);
        mSlotView.setListener(new SlotView.SimpleListener() {
            @Override
            public void onDown(int index) {
                AlbumPage.this.onDown(index);
            }

            @Override
            public void onUp(boolean followedByLongPress) {
                AlbumPage.this.onUp(followedByLongPress);
            }

            @Override
            public void onSingleTapUp(int slotIndex) {
                AlbumPage.this.onSingleTapUp(slotIndex);
            }

            @Override
            public void onLongTap(int slotIndex) {
                AlbumPage.this.onLongTap(slotIndex);
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
    }

    private void initializeData(Bundle data) {
        mMediaSetPath = Path.fromString(data.getString(KEY_MEDIA_PATH));
        mParentMediaSetString = data.getString(KEY_PARENT_MEDIA_PATH);
        mMediaSet = mActivity.getDataManager().getMediaSet(mMediaSetPath);
        if (mMediaSet == null) {
            Utils.fail("MediaSet is null. Path = %s", mMediaSetPath);
        }
        mSelectionManager.setSourceMediaSet(mMediaSet);
        mAlbumDataAdapter = new AlbumDataLoader(mActivity, mMediaSet);
        mAlbumDataAdapter.setLoadingListener(new MyLoadingListener());
        mAlbumView.setModel(mAlbumDataAdapter);
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

    private void hideDetails() {
        mShowDetails = false;
        mDetailsHelper.hide();
        mAlbumView.setHighlightItemPath(null);
        mSlotView.invalidate();
    }

    @Override
    protected boolean onCreateActionBar(Menu menu) {
        GalleryActionBar actionBar = mActivity.getGalleryActionBar();
        MenuInflater inflator = getSupportMenuInflater();
        /// M: [BUG.ADD] @{
        // put code related with ActionBar here, not in onResume
        boolean enableHomeButton = (mActivity.getStateManager().getStateCount() > 1)
                | mParentMediaSetString != null;
        actionBar.setDisplayOptions(enableHomeButton, false);
        /// @}

        if (mGetContent) {
            inflator.inflate(R.menu.pickup, menu);
            int typeBits = mData.getInt(GalleryActivity.KEY_TYPE_BITS,
                    DataManager.INCLUDE_IMAGE);
            actionBar.setTitle(GalleryUtils.getSelectionModePrompt(typeBits));
        } else {
            inflator.inflate(R.menu.album, menu);
            /// M: [BUG.ADD] @{
            Log.d(TAG, "<onCreateActionBar> setTitle:" + mMediaSet.getName());
            /// @}
            actionBar.setTitle(mMediaSet.getName());
            /// M: [BUG.ADD] @{
            // put code related with ActionBar here, not in onResume
            // After gallery was killed when in AlbumPage, when restart,
            // the title of ActionBar shows not right,
            // so we have to enableAlbumModeMenu after set right title
            actionBar.enableAlbumModeMenu(
                    GalleryActionBar.ALBUM_GRID_MODE_SELECTED, this);
            /// @}
            FilterUtils.setupMenuItems(actionBar, mMediaSetPath, true);

            menu.findItem(R.id.action_group_by).setVisible(mShowClusterMenu);
            menu.findItem(R.id.action_camera).setVisible(
                    MediaSetUtils.isCameraSource(mMediaSetPath)
                    && GalleryUtils.isCameraAvailable(mActivity));

        }
        actionBar.setSubtitle(null);

        return true;
    }

    private void prepareAnimationBackToFilmstrip(int slotIndex) {
        if (mAlbumDataAdapter == null || !mAlbumDataAdapter.isActive(slotIndex)) return;
        MediaItem item = mAlbumDataAdapter.get(slotIndex);
        if (item == null) return;
        TransitionStore transitions = mActivity.getTransitionStore();
        transitions.put(PhotoPage.KEY_INDEX_HINT, slotIndex);
        transitions.put(PhotoPage.KEY_OPEN_ANIMATION_RECT,
                mSlotView.getSlotRect(slotIndex, mRootPane));
    }

    private void switchToFilmstrip() {
        if (mAlbumDataAdapter.size() < 1) return;
        int targetPhoto = mSlotView.getVisibleStart();
        prepareAnimationBackToFilmstrip(targetPhoto);
        if(mLaunchedFromPhotoPage) {
            onBackPressed();
        } else {
            pickPhoto(targetPhoto, true);
        }
    }

    @Override
    protected boolean onItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home: {
                onUpPressed();
                return true;
            }
            case R.id.action_cancel:
                mActivity.getStateManager().finishState(this);
                return true;
            case R.id.action_select:
                mSelectionManager.setAutoLeaveSelectionMode(false);
                mSelectionManager.enterSelectionMode();
                return true;
            case R.id.action_group_by: {
                mActivity.getGalleryActionBar().showClusterDialog(this);
                return true;
            }
            case R.id.action_slideshow: {
                mInCameraAndWantQuitOnPause = false;
                Bundle data = new Bundle();
                data.putString(SlideshowPage.KEY_SET_PATH,
                        mMediaSetPath.toString());
                data.putBoolean(SlideshowPage.KEY_REPEAT, true);
                mActivity.getStateManager().startStateForResult(
                        SlideshowPage.class, REQUEST_SLIDESHOW, data);
                return true;
            }
            case R.id.action_details: {
                if (mShowDetails) {
                    hideDetails();
                } else {
                    showDetails();
                }
                return true;
            }
            case R.id.action_camera: {
                GalleryUtils.startCameraActivity(mActivity);
                return true;
            }
            default:
                return false;
        }
    }

    @Override
    protected void onStateResult(int request, int result, Intent data) {
        switch (request) {
            case REQUEST_SLIDESHOW: {
                // data could be null, if there is no images in the album
                if (data == null) return;
                mFocusIndex = data.getIntExtra(SlideshowPage.KEY_PHOTO_INDEX, 0);
                mSlotView.setCenterIndex(mFocusIndex);
                break;
            }
            case REQUEST_PHOTO: {
                if (data == null) return;
                mFocusIndex = data.getIntExtra(PhotoPage.KEY_RETURN_INDEX_HINT, 0);
                mSlotView.makeSlotVisible(mFocusIndex);
                break;
            }
            case REQUEST_DO_ANIMATION: {
                mSlotView.startRisingAnimation();
                break;
            }
        }
    }

    @Override
    public void onSelectionModeChange(int mode) {
        switch (mode) {
            case SelectionManager.ENTER_SELECTION_MODE: {
                mActionModeHandler.startActionMode();
                performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
                break;
            }
            case SelectionManager.LEAVE_SELECTION_MODE: {
                mActionModeHandler.finishActionMode();
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
        int count = mSelectionManager.getSelectedCount();
        String format = mActivity.getResources().getQuantityString(
                R.plurals.number_of_items_selected, count);
        mActionModeHandler.setTitle(String.format(format, count));
        mActionModeHandler.updateSupportedOperation(path, selected);
    }

    @Override
    public void onSyncDone(final MediaSet mediaSet, final int resultCode) {
        Log.d(TAG, "onSyncDone: " + Utils.maskDebugInfo(mediaSet.getName()) + " result="
                + resultCode);
        ((Activity) mActivity).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                GLRoot root = mActivity.getGLRoot();
                root.lockRenderThread();
                mSyncResult = resultCode;
                try {
                    if (resultCode == MediaSet.SYNC_RESULT_SUCCESS) {
                        mInitialSynced = true;
                    }
                    clearLoadingBit(BIT_LOADING_SYNC);
                    showSyncErrorIfNecessary(mLoadingFailed);
                } finally {
                    root.unlockRenderThread();
                }
            }
        });
    }

    // Show sync error toast when all the following conditions are met:
    // (1) both loading and sync are done,
    // (2) sync result is error,
    // (3) the page is still active, and
    // (4) no photo is shown or loading fails.
    private void showSyncErrorIfNecessary(boolean loadingFailed) {
        if ((mLoadingBits == 0) && (mSyncResult == MediaSet.SYNC_RESULT_ERROR) && mIsActive
                && (loadingFailed || (mAlbumDataAdapter.size() == 0))) {
            Toast.makeText(mActivity, R.string.sync_album_error,
                    Toast.LENGTH_LONG).show();
        }
    }

    private void setLoadingBit(int loadTaskBit) {
        mLoadingBits |= loadTaskBit;
    }

    private void clearLoadingBit(int loadTaskBit) {
        mLoadingBits &= ~loadTaskBit;
        if (mLoadingBits == 0 && mIsActive) {
            if (mAlbumDataAdapter.size() == 0) {
                Intent result = new Intent();
                result.putExtra(KEY_EMPTY_ALBUM, true);
                setStateResult(Activity.RESULT_OK, result);
                mActivity.getStateManager().finishState(this);
            }
        }
    }

    private class MyLoadingListener implements LoadingListener {
        @Override
        public void onLoadingStarted() {
            setLoadingBit(BIT_LOADING_RELOAD);
            mLoadingFailed = false;
        }

        @Override
        public void onLoadingFinished(boolean loadingFailed) {
            clearLoadingBit(BIT_LOADING_RELOAD);
            mLoadingFailed = loadingFailed;
            showSyncErrorIfNecessary(loadingFailed);

            /// M: [BEHAVIOR.ADD] Restore selection status after load finish @{
            // We have to notify SelectionManager about data change,
            // and this is the most proper place we could find till now
            boolean inSelectionMode = (mSelectionManager.inSelectionMode());
            int itemCount = mMediaSet != null ? mMediaSet.getMediaItemCount()
                    : 0;
            Log.d(TAG, "onLoadingFinished: item count=" + itemCount);
            mSelectionManager.onSourceContentChanged();
            boolean restore = false;
            if (itemCount > 0 && inSelectionMode) {
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
            // ClusterAlbum's name is designed to be localized, and the
            // localized name is calculated in reload(). Therefore we may obtain
            // a miss-localized name which is corresponding to the obsolete
            // (previous) Locale from ClusterAlbum if it is not finished
            // reloading. Here we re-get its name after it finished reloading,
            // and set the title of action bar to be the obtained name.
            if (!inSelectionMode && (mMediaSet instanceof ClusterAlbum)) {
                GalleryActionBar actionBar = mActivity.getGalleryActionBar();
                String title = mMediaSet.getName();
                Log.d(TAG, "<onLoadingFinished> setTitle:" + title);
                actionBar.setTitle(title);
                actionBar.notifyDataSetChanged();
            }
            /// @}
        }
    }

    private class MyDetailsSource implements DetailsHelper.DetailsSource {
        private int mIndex;

        @Override
        public int size() {
            return mAlbumDataAdapter.size();
        }

        @Override
        public int setIndex() {
            /// M: [BUG.MODIFY] check arraylist size @{
            //Path id = mSelectionManager.getSelected(false).get(0);
            //mIndex = mAlbumDataAdapter.findItem(id);
            ArrayList<Path> paths = mSelectionManager.getSelected(false);
            if (paths != null && paths.size() > 0) {
                Path id = mSelectionManager.getSelected(false).get(0);
                mIndex = mAlbumDataAdapter.findItem(id);
                return mIndex;
            } else {
                return -1;
            }
            /// @}
        }

        @Override
        public MediaDetails getDetails() {
            // this relies on setIndex() being called beforehand
            MediaObject item = mAlbumDataAdapter.get(mIndex);
            if (item != null) {
                mAlbumView.setHighlightItemPath(item.getPath());
                return item.getDetails();
            } else {
                return null;
            }
        }
    }

    @Override
    public void onAlbumModeSelected(int mode) {
        if (mode == GalleryActionBar.ALBUM_FILMSTRIP_MODE_SELECTED) {
            switchToFilmstrip();
        }
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    // Flag to specify whether mSelectionManager.restoreSelection task has done
    private boolean mRestoreSelectionDone;
    // Save selection for onPause/onResume
    private boolean mNeedUpdateSelection = false;
    // If restore selection not done in selection mode,after click one slot, show 'wait' toast
    private Toast mWaitToast = null;

    public void onSelectionRestoreDone() {
        if (!mIsActive) {
            return;
        }
        mRestoreSelectionDone = true;
        // Update selection menu after restore done @{
        mActionModeHandler.updateSupportedOperation();
        mActionModeHandler.updateSelectionMenu();
    }

    /// M: [BUG.ADD] leave selection mode when plug out sdcard @{
    @Override
    public void onEjectSdcard() {
        if (mSelectionManager.inSelectionMode()) {
            Log.d(TAG, "<onEjectSdcard> leaveSelectionMode");
            mSelectionManager.leaveSelectionMode();
        }
    }
    /// @}

    /// M:[FEATURE.ADD] play video directly. @{
    public void playVideo(Activity activity, Uri uri, String title) {
        Log.d(TAG, "<playVideo> enter playVideo");
        try {
            Intent intent = new Intent(Intent.ACTION_VIEW)
                    .setDataAndType(uri, "video/*")
                    .putExtra(Intent.EXTRA_TITLE, title)
                    .putExtra(MovieActivity.KEY_TREAT_UP_AS_BACK, true);
            activity.startActivityForResult(intent, PhotoPage.REQUEST_PLAY_VIDEO);
        } catch (ActivityNotFoundException e) {
            Toast.makeText(activity, activity.getString(R.string.video_err),
                    Toast.LENGTH_SHORT).show();
        }
    }

    private boolean canBePlayed(MediaItem item) {
        int supported = item.getSupportedOperations();
        return ((supported & MediaItem.SUPPORT_PLAY) != 0
                && MediaObject.MEDIA_TYPE_VIDEO == item.getMediaType());
    }
    /// @}

    /// M: [PERF.ADD] add for delete many files performance improve @{
    @Override
    public void setProviderSensive(boolean isProviderSensive) {
        mAlbumDataAdapter.setSourceSensive(isProviderSensive);
    }
    @Override
    public void fakeProviderChange() {
        mAlbumDataAdapter.fakeSourceChange();
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
}
