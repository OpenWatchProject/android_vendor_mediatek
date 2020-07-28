package com.mediatek.fullscreenmode;

import android.content.Context;
import android.os.RemoteException;
import android.preference.Preference;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Switch;

import com.mediatek.fullscreenmode.FullscreenFragment.LocalMode;
import com.mediatek.fullscreenswitch.FullscreenMode;
import com.mediatek.fullscreenswitch.IFullscreenSwitchManager;


/**
 * AppListPreference is the preference type. used to display white list manage
 * UI.
 */
public final class AppListPreference extends Preference implements
        View.OnClickListener {
    private static final String TAG = "AppListPreference";
    private Context mContext;
    private boolean mSwitchState;
    private Switch mSwitch;
    private IFullscreenSwitchManager mManager;
    private LocalMode mMode;

    public String mPkg;

    public AppListPreference(Context context, LocalMode appRecord,
            IFullscreenSwitchManager manager) {
        super(context);
        mContext = context;
        mMode = appRecord;
        mPkg = appRecord.packageName;
        mManager = manager;
        setLayoutResource(R.layout.preference_trust_list);
        // add switch
        setWidgetLayoutResource(R.layout.preference_switch);
        onDeviceAttributesChanged();
        try {
            int mode = mManager.getFullscreenMode(mPkg);
            if (mode == FullscreenMode.APP_CROP_SCREEN_MODE) {
                mSwitchState = false;
            } else {
                mSwitchState = true;
            }
        } catch (RemoteException e) {
            Log.e(TAG, "constructor error in " + e.toString());
        }
    }

    protected View onCreateView(ViewGroup parent) {
        View view = super.onCreateView(parent);
        mSwitch = (Switch) view.findViewById(R.id.status);
        return view;
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        mSwitch.setChecked(mSwitchState);
        view.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        mSwitchState = !mSwitch.isChecked();
        mSwitch.setChecked(mSwitchState);
        try {
            mManager.setFullscreenMode(mPkg, mSwitchState);
        } catch (RemoteException e) {
            Log.e(TAG, "onClick error in " + e.toString());
        }
        Log.d(TAG, "onClick()," + mPkg + ", support full screen ? "
                + mSwitchState);
    }

    /**
     * update the attributes.
     */
    public void onDeviceAttributesChanged() {
        setTitle(mMode.label);
        setIcon(ViewUtils.getAppDrawable(mContext, mPkg));
        setEnabled(mMode.level == FullscreenMode.APP_LEVEL_THIRD_PARTY);
    }
}
