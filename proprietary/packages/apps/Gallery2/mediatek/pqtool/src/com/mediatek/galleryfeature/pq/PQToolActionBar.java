package com.mediatek.galleryfeature.pq;

import android.app.ActionBar;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.Menu;
import android.view.MenuItem;

import com.mediatek.gallerybasic.base.IActionBar;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.plugin.component.ComponentSupport;

import java.io.File;

public class PQToolActionBar implements IActionBar {
    private final static String TAG = "MtkGallery/PQToolActionBar";
    private static boolean SUPPORT_PQ = (new File(Environment.getExternalStorageDirectory(),
            "SUPPORT_PQ")).exists();
    private final static String PLUGIN_ID = "PQTool";
    private int mMenuId;
    private MenuItem mMenu;
    private Context mContext;

    public PQToolActionBar(Context context) {
        Log.d(TAG, "<PQToolActionBar> context = " + context + " this class = "
                + this.getClass());
        mContext = context;
    }

    @Override
    public void onCreateOptionsMenu(ActionBar actionBar, Menu menu) {
        if (SUPPORT_PQ) {
            mMenuId = Menu.FIRST + 1;
            mMenu = menu.add(Menu.NONE, mMenuId, 1, "PQ");
        }
        Log.d(TAG, "<onCreateOptionsMenu>");
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu, MediaData data) {
        if (mMenu != null) {
            if (data != null && "image/jpeg".equalsIgnoreCase(data.mimeType)) {
                Log.d(TAG, "<onPrepareOptionsMenu> data mimeType = " + data.mimeType);
                mMenu.setVisible(true);
            } else {
                mMenu.setVisible(false);
            }
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item, MediaData data) {
        if (mMenu != null && data != null) {
            if (item.getItemId() == mMenuId) {
                Intent intent = new Intent();
                intent.setClassName(mContext,
                        "com.mediatek.galleryfeature.pq.PictureQualityActivity");
                Bundle pqBundle = new Bundle();
                pqBundle.putString("PQUri", data.uri.toString());
                pqBundle.putString("PQMineType", data.mimeType);
                intent.putExtras(pqBundle);
                ComponentSupport.startActivity(mContext, intent, PLUGIN_ID);
                Log.d(TAG, "<onOptionsItemSelected>  id == Menu.FIRST");
            }
        }
        Log.d(TAG, "<onOptionsItemSelected>");
        return false;
    }

}
