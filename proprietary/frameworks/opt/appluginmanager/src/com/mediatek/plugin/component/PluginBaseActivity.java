package com.mediatek.plugin.component;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;

import com.mediatek.plugin.utils.Log;

/**
 * The base class of activity in plugin apk, it hides the implement of proxy-plugin, and makes the
 * plugin activity like normal acitivty as much as possible. <br>
 * It saves the reference to proxy activity and transfers the calling from plugin activity to the
 * proxy activity.
 */
public class PluginBaseActivity extends Activity implements IPlugin {
    private static final String TAG = "PluginManager/PluginBaseActivity";

    // The reference to real activity:
    // Case 1: When PluginBaseActivity is launched as normal activity, <mThis> refers to the
    // PluginBaseActivity itself.
    // Case 2: When PluginBaseActivity is launched as plugin activity, <mThis> refers to the proxy
    // activity.
    protected Activity mThis;

    private boolean mLaunchAsPlugin;

    /**
     * Set proxy Activity.
     * @param proxyActivity
     *            The proxy activity.
     */
    public void addProxyActivity(Activity proxyActivity) {
        mLaunchAsPlugin = true;
        mThis = proxyActivity;
        initActivityInfo();
    }

    /**
     * Constructor when launch as normal activity.
     */
    public PluginBaseActivity() {
        super();
        mLaunchAsPlugin = false;
        mThis = this;
    }

    @Override
    public void setContentView(View view) {
        Log.d(TAG, "<setContentView> mLaunchAsPlugin444 = " + mLaunchAsPlugin);
        if (!mLaunchAsPlugin) {
            super.setContentView(view);
        } else {
            mThis.setContentView(view);
        }
    }

    @Override
    public void setContentView(View view, LayoutParams params) {
        if (!mLaunchAsPlugin) {
            super.setContentView(view, params);
        } else {
            mThis.setContentView(view, params);
        }
    }

    @Override
    public void setContentView(int layoutResID) {
        if (!mLaunchAsPlugin) {
            super.setContentView(layoutResID);
        } else {
            mThis.setContentView(layoutResID);
        }
    }

    @Override
    public void addContentView(View view, LayoutParams params) {
        if (!mLaunchAsPlugin) {
            super.addContentView(view, params);
        } else {
            mThis.addContentView(view, params);
        }
    }

    @Override
    public View findViewById(int id) {
        if (!mLaunchAsPlugin) {
            return super.findViewById(id);
        } else {
            return mThis.findViewById(id);
        }
    }

    @Override
    public Intent getIntent() {
        if (!mLaunchAsPlugin) {
            return super.getIntent();
        } else {
            return mThis.getIntent();
        }
    }

    @Override
    public ClassLoader getClassLoader() {
        if (!mLaunchAsPlugin) {
            return super.getClassLoader();
        } else {
            return mThis.getClassLoader();
        }
    }

    @Override
    public Resources getResources() {
        if (!mLaunchAsPlugin) {
            return super.getResources();
        } else {
            return mThis.getResources();
        }
    }

    @Override
    public String getPackageName() {
        if (!mLaunchAsPlugin) {
            return super.getPackageName();
        } else {
            return mThis.getPackageName();
        }
    }

    @Override
    public LayoutInflater getLayoutInflater() {
        if (!mLaunchAsPlugin) {
            return super.getLayoutInflater();
        } else {
            return mThis.getLayoutInflater();
        }
    }

    @Override
    public MenuInflater getMenuInflater() {
        if (!mLaunchAsPlugin) {
            return super.getMenuInflater();
        } else {
            return mThis.getMenuInflater();
        }
    }

    @Override
    public SharedPreferences getSharedPreferences(String name, int mode) {
        if (!mLaunchAsPlugin) {
            return super.getSharedPreferences(name, mode);
        } else {
            return mThis.getSharedPreferences(name, mode);
        }
    }

    @Override
    public Context getApplicationContext() {
        if (!mLaunchAsPlugin) {
            return super.getApplicationContext();
        } else {
            return mThis.getApplicationContext();
        }
    }

    @Override
    public Context getBaseContext() {
        if (!mLaunchAsPlugin) {
            return super.getBaseContext();
        } else {
            return mThis.getBaseContext();
        }
    }

    @Override
    public WindowManager getWindowManager() {
        if (!mLaunchAsPlugin) {
            return super.getWindowManager();
        } else {
            return mThis.getWindowManager();
        }
    }

    @Override
    public Window getWindow() {
        if (!mLaunchAsPlugin) {
            return super.getWindow();
        } else {
            return mThis.getWindow();
        }
    }

    @Override
    public Object getSystemService(String name) {
        if (!mLaunchAsPlugin) {
            return super.getSystemService(name);
        } else {
            return mThis.getSystemService(name);
        }
    }

    @Override
    public void finish() {
        if (!mLaunchAsPlugin) {
            super.finish();
        } else {
            mThis.finish();
        }
    }

    @Override
    public void onBackPressed() {
        if (!mLaunchAsPlugin) {
            super.onBackPressed();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (!mLaunchAsPlugin) {
            super.onActivityResult(requestCode, resultCode, data);
        }
    }

    @Override
    protected void onStart() {
        if (!mLaunchAsPlugin) {
            super.onStart();
        }
    }

    @Override
    public void onRestart() {
        if (!mLaunchAsPlugin) {
            super.onRestart();
        }
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        if (!mLaunchAsPlugin) {
            super.onRestoreInstanceState(savedInstanceState);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        if (!mLaunchAsPlugin) {
            super.onSaveInstanceState(outState);
        }
    }

    @Override
    public void onNewIntent(Intent intent) {
        if (!mLaunchAsPlugin) {
            super.onNewIntent(intent);
        }
    }

    @Override
    protected void onResume() {
        if (!mLaunchAsPlugin) {
            super.onResume();
        }
    }

    @Override
    protected void onPause() {
        if (!mLaunchAsPlugin) {
            super.onPause();
        }
    }

    @Override
    protected void onStop() {
        if (!mLaunchAsPlugin) {
            super.onStop();
        }
    }

    @Override
    protected void onDestroy() {
        if (!mLaunchAsPlugin) {
            super.onDestroy();
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (!mLaunchAsPlugin) {
            return super.onTouchEvent(event);
        }
        return false;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (!mLaunchAsPlugin) {
            return super.onKeyUp(keyCode, event);
        }
        return false;
    }

    @Override
    public void onWindowAttributesChanged(WindowManager.LayoutParams params) {
        if (!mLaunchAsPlugin) {
            super.onWindowAttributesChanged(params);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        if (!mLaunchAsPlugin) {
            super.onWindowFocusChanged(hasFocus);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.d(TAG, "<onCreateOptionsMenu> mLaunchAsPlugin = " + mLaunchAsPlugin);
        if (!mLaunchAsPlugin) {
            return super.onCreateOptionsMenu(menu);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (!mLaunchAsPlugin) {
            return onOptionsItemSelected(item);
        }
        return false;
    }

    private void initActivityInfo() {
        // TODO Get PackageInfo, Resources, AssetManager, ClassLoader from PluginManger
    }

    @Override
    public void onAttach(IProxy proxy) {
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        if (!mLaunchAsPlugin) {
            super.onCreate(savedInstanceState);
        }
    }

    @Override
    public Resources.Theme getTheme() {
        if (!mLaunchAsPlugin) {
            return super.getTheme();
        } else {
            return mThis.getTheme();
        }
    }

    @Override
    public ActionBar getActionBar() {
        if (!mLaunchAsPlugin) {
            return super.getActionBar();
        } else {
            return mThis.getActionBar();
        }

    }
}
