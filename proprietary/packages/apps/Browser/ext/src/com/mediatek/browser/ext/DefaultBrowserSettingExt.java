package com.mediatek.browser.ext;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.os.Environment;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.preference.Preference.OnPreferenceChangeListener;
import android.webkit.WebSettings;

import com.mediatek.search.SearchEngineManager;
import com.mediatek.common.search.SearchEngine;
import com.mediatek.storage.StorageManagerEx;

public class DefaultBrowserSettingExt implements IBrowserSettingExt {

    private static final String TAG = "DefaultBrowserSettingsExt";

    private static final String DEFAULT_DOWNLOAD_DIRECTORY = "/storage/sdcard0/Download";
    private static final String DEFAULT_MY_FAVORITE_FOLDER = "/Download";

    private static final String PREF_SEARCH_ENGINE = "search_engine";
    private static final String DEFAULT_SEARCH_ENGIN = "google";

    @Override
    public void customizePreference(int index, PreferenceScreen prefSc,
                    OnPreferenceChangeListener onPreferenceChangeListener,
                    SharedPreferences sharedPref, PreferenceFragment prefFrag) {
        Log.i("@M_" + TAG, "Enter: " + "customizePreference" + " --default implement");
        return;
    }

    @Override
    public boolean updatePreferenceItem(Preference pref, Object objValue) {
        Log.i("@M_" + TAG, "Enter: " + "updatePreferenceItem" + " --default implement");
        return false;
    }

    @Override
    public String getCustomerHomepage() {
        Log.i("@M_" + TAG, "Enter: " + "getCustomerHomepage" + " --default implement");
        return null;
    }

    @Override
    public String getDefaultDownloadFolder() {
        Log.i("@M_" + TAG, "Enter: " + "getDefaultDownloadFolder()" + " --default implement");
        String defaultDownloadPath = DEFAULT_DOWNLOAD_DIRECTORY;
        String defaultStorage = Environment.getExternalStorageDirectory().getAbsolutePath();
        if (null != defaultStorage) {
            defaultDownloadPath = defaultStorage + DEFAULT_MY_FAVORITE_FOLDER;
        }
        Log.v("@M_" + TAG, "device default storage is: " + defaultStorage +
               " defaultPath is: " + defaultDownloadPath);
        return defaultDownloadPath;
    }

    @Override
    public String getOperatorUA(String defaultUA) {
        Log.i("@M_" + TAG, "Enter: " + "getOperatorUA" + " --default implement");
        return null;
    }

    @Override
    public String getSearchEngine(SharedPreferences pref, Context context) {
        Log.i("@M_" + TAG, "Enter: " + "getSearchEngine" + " --default implement");
        SearchEngineManager searchEngineManager = (SearchEngineManager)
            context.getSystemService(SearchEngineManager.SEARCH_ENGINE_SERVICE);
        SearchEngine info = searchEngineManager.getDefault();
        String defaultSearchEngine = DEFAULT_SEARCH_ENGIN;
        if (info != null) {
            defaultSearchEngine = info.getName();
        }
        return pref.getString(PREF_SEARCH_ENGINE, defaultSearchEngine);
    }

    @Override
    public void setOnlyLandscape(SharedPreferences pref, Activity activity) {
        Log.i("@M_" + TAG, "Enter: " + "setOnlyLandscape" + " --default implement");
        if (activity != null) {
            activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        }
    }

    @Override
    public void setStandardFontFamily(WebSettings settings, SharedPreferences pref) {
        Log.i("@M_" + TAG, "Enter: " + "setStandardFontFamily" + " --default implement");
    }

    @Override
    public void setTextEncodingChoices(ListPreference pref) {
        Log.i("@M_" + TAG, "Enter: " + "setTextEncodingChoices" + " --default implement");
    }

}
