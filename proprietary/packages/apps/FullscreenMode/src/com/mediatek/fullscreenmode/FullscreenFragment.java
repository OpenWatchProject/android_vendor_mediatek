package com.mediatek.fullscreenmode;

import android.app.Activity;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.widget.TextView;

import com.mediatek.fullscreenswitch.FullscreenMode;
import com.mediatek.fullscreenswitch.IFullscreenModeChangeListener;
import com.mediatek.fullscreenswitch.IFullscreenSwitchManager;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class FullscreenFragment extends PreferenceFragment {
    private static final String TAG = "FullscreenFragment";

    public Activity mActivity;
    public Resources mRes;
    public PreferenceScreen mPreferenceScreen;
    private Preference mDesPrf;
    private TextView mEmptyView;
    private IFullscreenSwitchManager mManager;
    private IFullscreenModeChangeListener mModeChangeListener = new ModeChangeListener();
    private Handler mHandler;

    public class ModeChangeListener extends IFullscreenModeChangeListener.Stub {
        public void onModeChanged() {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    updateUI();
                }
            });
        }
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.fullscreen_fragment);
        mActivity = getActivity();
        mPreferenceScreen = getPreferenceScreen();
        mRes = mActivity.getResources();
        mManager = IFullscreenSwitchManager.Stub.asInterface(ServiceManager
                .checkService("FullscreenSwitchService"));
        mHandler = new Handler();
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        setHasOptionsMenu(true);
    }

    @Override
    public void onStart() {
        super.onStart();
        updateUI();
        // register listener to update UI when some app installed or uninstalled
        try {
            mManager.registerModeChangeLister(mModeChangeListener);
        } catch (RemoteException e) {
            Log.e(TAG, "onStart error in " + e.toString());
        }
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onStop() {
        super.onStop();
        // unregister the listener
        try {
            mManager.unregisterModeChangeLister(mModeChangeListener);
        } catch (RemoteException e) {
            Log.e(TAG, "onStop error in " + e.toString());
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }

    /**get the data from FullscreenSwitchManager, then sort data, and add them
     * into preferences
     **/
    private void updateUI() {
        // get the init data from fwk
        List<FullscreenMode> modes = null;
        try {
            modes = mManager.getAllScreenSwitchModels();
            Log.d(TAG, " getAllScreenSwitchModels.. = " + modes);
        } catch (RemoteException e) {
            Log.e(TAG, "updateUI error in " + e.toString());
        }
        // rearrange the data, get its label from PMS
        List<LocalMode> localModes = new ArrayList<LocalMode>();
        LocalMode lm = null;
        for (FullscreenMode eachMode : modes) {
            lm = new LocalMode();
            lm.id = eachMode.id;
            lm.packageName = eachMode.packageName;
            lm.mode = eachMode.mode;
            lm.level = eachMode.level;
            lm.label = ViewUtils.getAppLabel(mActivity, eachMode.packageName);
            localModes.add(lm);
        }
        // Sort the data modes
        MyComparator mc = new MyComparator();
        Collections.sort(localModes, mc);
        // add apps to preferences
        addAllAppPreference(localModes);
    }

    private void addAllAppPreference(List<LocalMode> appList) {
        mPreferenceScreen.removeAll();
        for (LocalMode appRecord : appList) {
            if (appRecord != null) {
                AppListPreference prf = new AppListPreference(mActivity,
                        appRecord, mManager);
                mPreferenceScreen.addPreference(prf);
            } else {
                Log.w(TAG, "empty label for pkg: " + appRecord);
            }
        }
    }

    // comparator for sorting the app list
    class MyComparator implements Comparator {
        public int compare(Object o1, Object o2) {
            LocalMode object1 = (LocalMode) o1;
            LocalMode object2 = (LocalMode) o2;
            if (object1 == object2) {
                return 0;
            } else {
                if (object1.level < object2.level) {
                    return -1;
                } else if (object1.level > object2.level) {
                    return 1;
                } else {
                    if (0 < object1.label.compareTo(object2.label)) {
                        return 1;
                    } else {
                        return -1;
                    }
                }
            }
        }
    }

    public class LocalMode extends FullscreenMode {
        public String label;
    }
}
