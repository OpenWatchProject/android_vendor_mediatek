package com.mediatek.fullscreenmode;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.os.Bundle;
import android.util.Log;


public class FullscreenModeActivity extends Activity {
    private static final String TAG = "FullscreenModeActivity";
    private int mMainContentId = R.id.main_content;

    @Override
    protected void onCreate(Bundle savedState) {
        super.onCreate(savedState);
        if (isMonkeyRunning()) {
           finish();
        }
        // set up empty UI
        setContentView(R.layout.fullscreen);
        switchToFragment(FullscreenFragment.class.getName(), null, false);
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    /**
     * switch To Fragment.
     * @param fragmentName switch to fragment class name
     * @param args Bundle , can be null
     * @param backStack  put it to back stack or not
     * @return fragment will switch to
     */
    public Fragment switchToFragment(String fragmentName, Bundle args,
            boolean backStack) {
        Log.d(TAG, "switchToFragment = " + fragmentName);
        Fragment f = Fragment.instantiate(this, fragmentName, args);
        FragmentTransaction transaction = getFragmentManager()
                .beginTransaction();
        transaction.replace(mMainContentId, f);
        transaction.setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN);
        if (backStack) {
            transaction.addToBackStack("FullscreenMode");
        }
        transaction.commitAllowingStateLoss();
        return f;
    }

    /**
     * Returns true if Monkey is running.
     */
    public boolean isMonkeyRunning() {
        return ActivityManager.isUserAMonkey();
    }
}
