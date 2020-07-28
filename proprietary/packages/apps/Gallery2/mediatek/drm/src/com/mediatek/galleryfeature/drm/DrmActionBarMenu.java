package com.mediatek.galleryfeature.drm;

import android.app.ActionBar;
import android.content.Context;
import android.view.Menu;
import android.view.MenuItem;

import com.mediatek.gallerybasic.base.IActionBar;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.util.Log;

public class DrmActionBarMenu implements IActionBar {
    private final static String TAG = "MtkGallery/DrmActionBar";
    private int mMenuId;
    private MenuItem mMenu;
    private Context mContext;

    /**
     * @param context
     */
    public DrmActionBarMenu(Context context) {
        Log.d(TAG, "<DrmActionBar> context = " + context + " this class = "
                + this.getClass());
        mContext = context;
    }

    @Override
    public void onCreateOptionsMenu(ActionBar actionBar, Menu menu) {
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
        Log.d(TAG, "<onCreateOptionsMenu>");
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu, MediaData data) {
        if (mMenu != null) {
            if (data != null && data.filePath != null && data.filePath != ""
                    && data.mediaType.getMainType() == DrmMember.sType) {
                if (DrmHelper.isDataProtectionFile(data.filePath)) {
                    mMenu.setVisible(false);
                } else {
                    mMenu.setVisible(true);
                }
            } else {
                mMenu.setVisible(false);
            }
        }
        Log.d(TAG, "<onPrepareOptionsMenu> mMenu = " + mMenu + ", data = " + data);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item, MediaData data) {
        if (mMenu != null && data != null && data.uri != null) {
            if (item.getItemId() == mMenuId) {
                DrmHelper.showProtectionInfoDialog(mContext, data.uri);
            }
        }
        return false;
    }

}
