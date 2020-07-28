package com.mediatek.gallery3d.ext;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

/**
 * Activity action hooker class.
 * Host app's activity will call this hooker's functions in its lifecycle.
 * For example: HostActivity.onCreate()-->hooker.onCreate().
 * But void init(Activity context, Intent intent) will be called before other functions.
 * <br/>
 * IActivityHooker objects may show menus, but we should give a unique menu id to every menus.
 * Hooker can call getMenuActivityId(int) to get a global unique menu id to be used in menu.add(),
 * and can call getMenuOriginalId(int) to get the original menu id.
 * the example:
 * class Hooker implements IActivityHooker {
 *   private static final int MENU_EXAMPLE = 1;
 *   @Override
 *   public boolean onCreateOptionsMenu(Menu menu) {
 *       super.onCreateOptionsMenu(menu);
 *       menu.add(0, getMenuActivityId(MENU_EXAMPLE), 0, android.R.string.ok);
 *       return true;
 *   }
 *   @Override
 *   public boolean onOptionsItemSelected(MenuItem item) {
 *       switch(getMenuOriginalId(item.getItemId())) {
 *       case MENU_EXAMPLE:
 *          //do something
 *           return true;
 *       default:
 *           return false;
 *       }
 *   }
 * }
 */
public interface IActivityHooker {
    /**
     * Will be called in Host Activity.onCreate(Bundle savedInstanceState)
     * @param savedInstanceState
     * @internal
     */
    void onCreate(Bundle savedInstanceState);
    /**
     * Will be called in Host Activity.onStart()
     * @internal
     */
    void onStart();
    /**
     * Will be called in Host Activity.onStop()
     * @internal
     */
    void onStop();
    /**
     * Will be called in Host Activity.onPause()
     * @internal
     */
    void onPause();
    /**
     * Will be called in Host Activity.onResume()
     * @internal
     */
    void onResume();
    /**
     * Will be called in Host Activity.onDestroy()
     * @internal
     */
    void onDestroy();
    /**
     * Will be called in Host Activity.onCreateOptionsMenu(Menu menu)
     * @param menu
     * @return
     * @internal
     */
    boolean onCreateOptionsMenu(Menu menu);
    /**
     * Will be called in Host Activity.onPrepareOptionsMenu(Menu menu)
     * @param menu
     * @return
     * @internal
     */
    boolean onPrepareOptionsMenu(Menu menu);
    /**
     * Will be called in Host Activity.onOptionsItemSelected(MenuItem item)
     * @param item
     * @return
     * @internal
     */
    boolean onOptionsItemSelected(MenuItem item);

    /**
     * Should be called before any other functions.
     * @param context
     * @param intent
     * @internal
     */
    void init(Activity context, Intent intent);

    /**
     * Set Menu visibility.
     * @param visible
     * @return
     * @internal
     */
    void setVisibility(boolean visible);
    /**
     * Host activity will call this function to set parameter to hooker activity.
     * @param key
     * @param value
     * @internal
     */
    void setParameter(String key, Object value);
}
