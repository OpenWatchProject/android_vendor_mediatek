package com.mediatek.settings;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.drm.DrmManagerClient;
import android.os.Bundle;
import androidx.preference.Preference;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.R;
import com.android.settings.SettingsPreferenceFragment;

public class DrmSettings extends SettingsPreferenceFragment {
    private static final String TAG = "DrmSettings";
    private static final String DRM_RESET = "drm_settings";
    private static Preference sPreferenceReset;
    private static final int DIALOG_RESET = 1000;
    private static DrmManagerClient sClient;
    private Context mContext;
    private View mContentView;

    private SettingsDialogFragment mDialogFragment;

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.PRIVACY;
    }

    @Override
    public int getDialogMetricsCategory(int dialogId) {
        return MetricsEvent.PRIVACY;
    }

    protected void showDialog(int dialogId) {
        if (mDialogFragment != null) {
            Log.e(TAG, "Old dialog fragment not null!");
        }
        mDialogFragment = SettingsDialogFragment.newInstance(this, dialogId);
        mDialogFragment.show(getActivity().getSupportFragmentManager(), Integer.toString(dialogId));
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.drm_settings);

        sPreferenceReset = findPreference(DRM_RESET);
        mContext = getActivity();
        sClient = new DrmManagerClient(mContext);
    }

    @Override
    public Dialog onCreateDialog(int id) {
        Builder builder = new AlertDialog.Builder(mContext);
        AlertDialog dialog;
        switch (id) {
        case DIALOG_RESET:
            builder.setMessage(getResources().getString(
                    R.string.drm_reset_dialog_msg));
            builder.setTitle(getResources().getString(
                    R.string.drm_settings_title));
            builder.setIcon(android.R.drawable.ic_dialog_alert);
            builder.setPositiveButton(android.R.string.ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {
                            if (sClient != null) {
                                int result = sClient.removeAllRights();
                                if (result == sClient.ERROR_NONE) {
                                    Toast.makeText(mContext,
                                            R.string.drm_reset_toast_msg,
                                            Toast.LENGTH_SHORT).show();
                                    sPreferenceReset.setEnabled(false);
                                } else {
                                    Log.d(TAG, "removeAllRights fail!");
                                }
                                sClient = null;
                            }
                        }
                    });
            builder.setNegativeButton(android.R.string.cancel, null);
            dialog = builder.create();
            return dialog;
        default:
            return null;
        }
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        if (preference == sPreferenceReset) {
            showDialog(DIALOG_RESET);
        }
        return super.onPreferenceTreeClick(preference);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        sClient = null;
    }

}
