package com.mediatek.gallery3d.video;

import android.view.Menu;
import android.view.MenuItem;

import com.android.gallery3d.R;

public class StopVideoHooker extends MovieHooker {
    private static final int MENU_STOP = 1;
    private MenuItem mMenuStop;

    @Override
    public boolean onCreateOptionsMenu(final Menu menu) {
        super.onCreateOptionsMenu(menu);
        mMenuStop = menu.add(MENU_HOOKER_GROUP_ID, getMenuActivityId(MENU_STOP), 0, R.string.stop);
        return true;
    }
    @Override
    public void setVisibility(boolean visible) {
        if (mMenuStop != null) {
            mMenuStop.setVisible(visible);
        }
    }
    @Override
    public boolean onPrepareOptionsMenu(final Menu menu) {
        super.onPrepareOptionsMenu(menu);
        updateStop();
        return true;
    }
    @Override
    public boolean onOptionsItemSelected(final MenuItem item) {
        super.onOptionsItemSelected(item);
        switch(getMenuOriginalId(item.getItemId())) {
        case MENU_STOP:
            getPlayer().stopVideo();
            return true;
        default:
            return false;
        }
    }
    private void updateStop() {
        if (getPlayer() != null && mMenuStop != null) {
            mMenuStop.setEnabled(getPlayer().canStop());
        }
    }
}