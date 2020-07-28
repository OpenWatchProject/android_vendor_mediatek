package com.mediatek.gallery3d.ext;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import com.mediatek.gallery3d.video.IMovieItem;

/**
 * Default implemention class of IActivityHooker.
 */
public class DefaultActivityHooker implements IActivityHooker {
    public static final int MENU_HOOKER_GROUP_ID = 999;
    private static final int MENU_MAX_NUMBER = 100;
    private static int sMenuId = 1;
    private int mMenuId;
    private static Object sMenuLock = new Object();
    private Activity mContext;
    private Intent mIntent;
    private IMovieItem mMovieItem;

    public DefaultActivityHooker() {
        synchronized (sMenuLock) {
            sMenuId++;
            mMenuId = sMenuId * MENU_MAX_NUMBER;
        }
    }

    public int getMenuActivityId(int id) {
        return mMenuId + id;
    };

    public int getMenuOriginalId(int id) {
        return id - mMenuId;
    }

    @Override
    public void init(Activity context, Intent intent) {
        mContext = context;
        mIntent = intent;
    }

    public Activity getContext() {
        return mContext;
    }

    public Intent getIntent() {
        return mIntent;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
    }

    @Override
    public void onStart() {
    }

    @Override
    public void onResume() {
    }

    @Override
    public void onPause() {
    }

    @Override
    public void onStop() {
    }

    @Override
    public void onDestroy() {
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        return false;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        return false;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        return false;
    }

    @Override
    public void setVisibility(boolean visible) {
    }

    @Override
    public void setParameter(String key, Object value) {
        if (value instanceof IMovieItem) {
            mMovieItem = (IMovieItem) value;
            onMovieItemChanged(mMovieItem);
        }
    }

    public IMovieItem getMovieItem() {
        return mMovieItem;
    }

    public void onMovieItemChanged(final IMovieItem item) {
    }

}
