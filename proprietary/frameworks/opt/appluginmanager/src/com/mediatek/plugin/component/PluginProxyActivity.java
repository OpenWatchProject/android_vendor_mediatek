package com.mediatek.plugin.component;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.content.res.Resources.Theme;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.WindowManager.LayoutParams;

import com.mediatek.plugin.Plugin;
import com.mediatek.plugin.PluginManager;
import com.mediatek.plugin.element.PluginDescriptor;
import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.ReflectUtils;

/**
 * PluginProxyActivity is the started activity. And proxy the relative activity.
 */

public class PluginProxyActivity extends Activity implements IProxy {
    private static final String TAG = "PluginManager/PluginProxyActivity";
    public static final String CLASS_PILGIN = "Class_plugin";
    public static final String PATH_APK = "Path_apk";
    public static final String URI_IMG = "Uri_img";
    private PluginBaseActivity mPluginActivity;
    private ClassLoader mClassLoader;
    private Resources mResources;
    private AssetManager mAsset;

    @Override
    public void attach(IPlugin plugin) {
        // TODO Auto-generated method stub
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        getActionBar();
        String pluginId = intent.getStringExtra(ComponentSupport.KEY_TARGET_PLUGIN_ID);
        String targetActivityName = intent.getStringExtra(ComponentSupport.KEY_TARGET_ACTVITY);
        Log.d(TAG, "<onCreate> bundle = " + pluginId);
        if (pluginId != null && targetActivityName != null) {
            PluginManager pluginManager = PluginManager.getInstance(this);
            Plugin plugin = pluginManager.getPlugin(pluginId);
            if (plugin == null) {
                finish();
                Log.e(TAG, "<onCreate> plugin == null Please check!!!");
                return;
            }
            PluginDescriptor descriptor = plugin.getDescriptor();
            mResources = descriptor.getResources();
            mAsset = descriptor.getAssetManager();
            mPluginActivity =
                    launchTargetActivity(targetActivityName, plugin.getClassLoader());
            if (mPluginActivity != null) {
                Log.d(TAG, "<onCreate> mPluginActivity = " + mPluginActivity);
                mPluginActivity.onCreate(savedInstanceState);
            } else {
                finish();
                Log.e(TAG, "<onCreate> mPluginActivity == null Please check!!!");
            }
        }
    }

    @Override
    protected void onStart() {
        mPluginActivity.onStart();
        super.onStart();
    }

    @Override
    protected void onRestart() {
        mPluginActivity.onRestart();
        super.onRestart();
    }

    @Override
    protected void onResume() {
        mPluginActivity.onResume();
        super.onResume();
    }

    @Override
    protected void onPause() {
        mPluginActivity.onPause();
        super.onPause();
    }

    @Override
    protected void onStop() {
        mPluginActivity.onStop();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        mPluginActivity.onDestroy();
        mPluginActivity = null;
        mClassLoader = null;
        super.onDestroy();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        if (mPluginActivity != null) {
            mPluginActivity.onSaveInstanceState(outState);
        }
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        if (mPluginActivity != null) {
            mPluginActivity.onRestoreInstanceState(savedInstanceState);
        }
        super.onRestoreInstanceState(savedInstanceState);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        if (mPluginActivity != null) {
            mPluginActivity.onNewIntent(intent);
        }
        super.onNewIntent(intent);
    }

    @Override
    public void onBackPressed() {
        if (mPluginActivity != null) {
            mPluginActivity.onBackPressed();
        }
        super.onBackPressed();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        super.onTouchEvent(event);
        return mPluginActivity.onTouchEvent(event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        super.onKeyUp(keyCode, event);
        return mPluginActivity.onKeyUp(keyCode, event);
    }

    @Override
    public void onWindowAttributesChanged(LayoutParams params) {
        if (mPluginActivity != null) {
            mPluginActivity.onWindowAttributesChanged(params);
        }
        super.onWindowAttributesChanged(params);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        if (mPluginActivity != null) {
            mPluginActivity.onWindowFocusChanged(hasFocus);
        }
        super.onWindowFocusChanged(hasFocus);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.d(TAG, " <onCreateOptionsMenu> ");
        if (mPluginActivity != null) {
            mPluginActivity.onCreateOptionsMenu(menu);
        }
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (mPluginActivity != null) {
            mPluginActivity.onPrepareOptionsMenu(menu);
        }

        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (mPluginActivity != null) {
            mPluginActivity.onOptionsItemSelected(item);
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        if (mPluginActivity != null) {
            mPluginActivity.onConfigurationChanged(newConfig);
        }
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public Resources getResources() {
        if (mResources != null) {
            return mResources;
        } else {
            return super.getResources();
        }
    }

    @Override
    public AssetManager getAssets() {
        if (mAsset != null) {
            return mAsset;
        } else {
            return super.getAssets();
        }
    }

    @Override
    public Theme getTheme() {
        return super.getTheme();
    }

    @Override
    public ClassLoader getClassLoader() {
        if (mClassLoader != null) {
            return mClassLoader;
        } else {
            return super.getClassLoader();
        }
    }

    @Override
    public MenuInflater getMenuInflater() {
        return new MenuInflater(mPluginActivity);
    }

    @Override
    public void setContentView(int layoutResID) {
        getWindow().setContentView(layoutResID);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        mPluginActivity.onActivityResult(requestCode, resultCode, data);
        super.onActivityResult(requestCode, resultCode, data);
    }

    private PluginBaseActivity launchTargetActivity(String classPath, ClassLoader classLoader) {
        PluginBaseActivity pluginActivity = null;
        Object instance = ReflectUtils.createInstance(classPath, classLoader);
        if (instance != null && instance instanceof PluginBaseActivity) {
            pluginActivity = (PluginBaseActivity) instance;
            pluginActivity.addProxyActivity(this);
            attach(pluginActivity);
        }
        return pluginActivity;
    }
}
