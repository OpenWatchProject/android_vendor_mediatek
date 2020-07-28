package com.mediatek.gallery3d.video;

import android.net.Uri;
import android.view.Menu;
import android.view.MenuItem;

import com.android.gallery3d.R;
import com.mediatek.gallery3d.util.Log;

public class LoopVideoHooker extends MovieHooker {
    private static final String TAG = "VP_LoopVideoHooker";
    private static final boolean LOG = true;

    private static final int MENU_LOOP = 1;

    private MenuItem mMenuLoopButton;
    private boolean mNeedShow = true;

    @Override
    public boolean onCreateOptionsMenu(final Menu menu) {
        super.onCreateOptionsMenu(menu);
        mMenuLoopButton = menu.add(MENU_HOOKER_GROUP_ID,
                                   getMenuActivityId(MENU_LOOP), 0, R.string.loop);
        return true;
    }
    @Override
    public boolean onPrepareOptionsMenu(final Menu menu) {
        super.onPrepareOptionsMenu(menu);
        updateLoop();
        return true;
    }
    @Override
    public boolean onOptionsItemSelected(final MenuItem item) {
        super.onOptionsItemSelected(item);
        switch(getMenuOriginalId(item.getItemId())) {
        case MENU_LOOP:
            getPlayer().setLoop(!getPlayer().getLoop());
            updateLoop();
            return true;
        default:
            return false;
        }
    }
    @Override
    public void setVisibility(boolean visible) {
        if (mMenuLoopButton != null) {
            mMenuLoopButton.setVisible(visible);
            mNeedShow = visible;
            Log.v(TAG, "setVisibility() visible=" + visible);
        }
    }


    private void updateLoop() {
        Log.v(TAG, "updateLoop() mLoopButton=" + mMenuLoopButton);
        if (mMenuLoopButton != null && mNeedShow) {
            Uri uri = getMovieItem().getUri();
            if (MovieUtils.isLocalFile(uri, getMovieItem().getMimeType())) {
                mMenuLoopButton.setVisible(true);
            } else {
                mMenuLoopButton.setVisible(false);
            }
            boolean newLoop = false;
            if (getPlayer() != null) {
                newLoop = getPlayer().getLoop();
            }
            if (newLoop) {
                mMenuLoopButton.setTitle(R.string.single);
                mMenuLoopButton.setIcon(R.drawable.m_ic_menu_unloop);
            } else {
                mMenuLoopButton.setTitle(R.string.loop);
                mMenuLoopButton.setIcon(R.drawable.m_ic_menu_loop);
            }
        }
    }
}
