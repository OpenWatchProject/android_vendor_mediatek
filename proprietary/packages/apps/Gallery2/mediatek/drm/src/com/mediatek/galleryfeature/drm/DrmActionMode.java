package com.mediatek.galleryfeature.drm;

import android.content.Context;
import android.os.Handler;
import android.view.ActionMode;
import android.view.Menu;
import android.view.MenuItem;

import com.mediatek.gallerybasic.base.IActionMode;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.util.Log;

public class DrmActionMode implements IActionMode {
    private final static String TAG = "MtkGallery/DrmActionMode";
    private int mMenuId;
    private MenuItem mMenu;
    private Context mContext;
    private MediaData mData;
    private Handler mMainHandler;
    private UpdateOperation mUpdateOperation;

    public DrmActionMode(Context context) {
        Log.d(TAG, "<DrmActionMode> context = " + context + " this class = "
                + this.getClass());
        if (context == null) {
            throw new IllegalArgumentException("context can not be null");
        }
        mContext = context;
        mMainHandler = new Handler(context.getMainLooper());
    }

    @Override
    public void onCreateActionMode(ActionMode actionMode, Menu menu) {
        if (!DrmHelper.sSupportDrm) {
            return;
        }
        mMenuId = Menu.FIRST + 1;
        String title = mContext.getResources().getString(com.mediatek.internal.R.string
                .drm_protectioninfo_title);
        if (title != null) {
            mMenu = menu.add(Menu.NONE, mMenuId, 1, title);
            mMenu.setShowAsActionFlags(MenuItem.SHOW_AS_ACTION_NEVER);
            mMenu.setVisible(false);
        }
        Log.d(TAG, "<onCreateActionMode> create drm action mode");
    }

    @Override
    public void onPrepareActionMode(ActionMode actionMode, Menu menu) {
        Log.d(TAG, "<onPrepareActionMode> prepare action mode");
    }

    @Override
    public void onDestroyActionMode(ActionMode actionMode) {
        if (mUpdateOperation != null) {
            mMainHandler.removeCallbacks(mUpdateOperation);
        }
    }

    @Override
    public boolean onActionItemClicked(ActionMode actionMode, MenuItem item) {
        if (mMenu != null && mData != null) {
            if (item.getItemId() == mMenuId) {
                DrmHelper.showProtectionInfoDialog(mContext, mData.uri);
            }
        }
        return false;
    }

    @Override
    public void onSelectionChange(MediaData[] data) {
        if (mMenu != null) {
            mUpdateOperation = new UpdateOperation(data);
            mMainHandler.post(mUpdateOperation);

        }
    }

    private class UpdateOperation implements Runnable {
        MediaData[] mMediaData;
        public UpdateOperation(MediaData[] mediaData) {
            mMediaData = mediaData;
        }
        @Override
        public void run() {
            if (mMenu == null || mMediaData == null || mMediaData.length > 1 || mMediaData[0] ==
                    null) {
                mMenu.setVisible(false);
                return;
            }
            mData = mMediaData[0];
            Log.d(TAG, "<UpdateOperation> filePath = " + mData.filePath);
            if (mData != null && mData.mediaType.getMainType() ==
                    DrmMember.sType) {
                if (DrmHelper.isDataProtectionFile(mData.filePath)) {
                    mMenu.setVisible(false);
                } else {
                    mMenu.setVisible(true);
                }
            } else {
                mMenu.setVisible(false);
            }
        }
    }
}
