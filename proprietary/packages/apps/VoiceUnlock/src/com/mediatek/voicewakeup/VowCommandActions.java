package com.mediatek.voicewakeup;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.voiceunlock.R;
import com.mediatek.voiceunlock.SettingsPreferenceFragment;

public class VowCommandActions extends PreferenceActivity {
    @Override
    public Intent getIntent() {
        Intent modIntent = new Intent(super.getIntent());
        modIntent.putExtra(EXTRA_SHOW_FRAGMENT, VowCommandActionsFragment.class.getName());
        modIntent.putExtra(EXTRA_NO_HEADERS, true);
        return modIntent;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return (VowCommandActionsFragment.class.getName().equals(fragmentName));
    }

    public static class VowCommandActionsFragment extends SettingsPreferenceFragment implements
            Utils.VoiceServiceListener {

        private static final String TAG = "VowCommandActions";

        private static final String PREF_KEY_PLAY_COMMAND = "play_command";
        private static final String PREF_KEY_MODIFY_COMMAND = "modify_command";
        private static final String PREF_KEY_RESET_COMMAND = "reset_command";

        private static final int COMFIRM_RESET_DIALOG = 0;
        private static final int MSG_SERVICE_ERROR = 0;

        private int mCommandId;
        private String mCommandSummary;
        private static String mCommandTitle;

        Context mContext;
        private String mPkgName;
        private Utils mUtils;

        private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_SERVICE_ERROR:
                    handleServiceError((String) msg.obj);
                    break;
                default:
                    break;
                }
            }
        };

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            addPreferencesFromResource(R.xml.voice_command_actions);
            mContext = getActivity().getBaseContext();
            mPkgName = mContext.getPackageName();
            mUtils = Utils.getInstance();
            mCommandId = getActivity().getIntent().getIntExtra(Utils.KEY_COMMAND_ID, -1);
            if (mCommandId == -1) {
                getActivity().finish();
            }
            mCommandSummary = getActivity().getIntent().getStringExtra(Utils.KEY_COMMAND_SUMMARY);
            mCommandTitle = getActivity().getIntent().getStringExtra(Utils.KEY_COMMAND_TITLE);
            // set title , like Wake up *** App
            getActivity().setTitle(mCommandTitle);
        }

        @Override
        public void onResume() {
            super.onResume();
            mUtils.setContext(mContext);
            mUtils.setOnChangedListener(this);
            mUtils.onResume();
        }

        @Override
        public void onPause() {
            mUtils.onPause();
            mUtils.setOnChangedListener(null);
            super.onPause();
        }

        @Override
        public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
                Preference preference) {
            final String key = preference.getKey();
            boolean handled = true;
            if (PREF_KEY_PLAY_COMMAND.equals(key)) {
                mUtils.playCommand(this, mCommandId, mCommandSummary, Utils.VOW_WITH_SPEAKER_MODE);
            } else if (PREF_KEY_MODIFY_COMMAND.equals(key)) {
                // modify and with speaker id mode
                mUtils.updateCommand(this, mCommandId,
                        mUtils.mKeyCommandInfoMap.get(mCommandId).mLaunchedApp
                                .flattenToShortString(), Utils.COMMAND_TYPE_MODIFY,
                        Utils.VOW_WITH_SPEAKER_MODE, null);
            } else if (PREF_KEY_RESET_COMMAND.equals(key)) {
                showDialog(COMFIRM_RESET_DIALOG);
            } else {
                handled = false;
            }
            return handled;
        }

        @Override
        public Dialog onCreateDialog(int dialogId) {
            switch (dialogId) {
            case COMFIRM_RESET_DIALOG:
                AlertDialog dialog = new AlertDialog.Builder(getActivity()).setTitle(
                        R.string.reset_command_title).setNegativeButton(
                        R.string.voice_unlock_cancel_label, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                            }
                        }).setPositiveButton(R.string.voice_unlock_ok_label,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                resetCommand();
                            }
                        }).create();
                dialog.setMessage(getResources().getString(R.string.reset_command_prompt));
                return dialog;
            default:
                return null;
            }
        }

        private void resetCommand() {
            if (mUtils.mVCmdMgrService != null) {
                Bundle extra = new Bundle();
                Log.d("@M_" + TAG, "sendCommand TRAINING_RESET commandId = " + mCommandId);
                extra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, mCommandId);
                extra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, 2);
                mUtils.sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                        VoiceCommandListener.ACTION_VOICE_TRAINING_RESET, extra);
            }
            if (mUtils.isLastResetCommand()) {
                // Write enable settings to disable
                Settings.System.putInt(getContentResolver(),
                        MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS, 0);
            }
            MtkSettingsExt.System.setVoiceCommandValue(getContentResolver(),
                    MtkSettingsExt.System.BASE_VOICE_WAKEUP_COMMAND_KEY, mCommandId, null);
            getActivity().finish();
        }

        private void handleServiceError(String errorMsg) {
            Toast.makeText(getActivity(), errorMsg, Toast.LENGTH_SHORT);
        }

        @Override
        public void handleVoiceCommandNotified(int mainAction, int subAction, Bundle extraData) {
            int result = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT);
            Log.d("@M_" + TAG, "onNotified result = " + result);
            if (result == VoiceCommandListener.ACTION_EXTRA_RESULT_ERROR) {
                String errorMsg = extraData
                        .getString(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                Log.d("@M_" + TAG, "onNotified RESULT_ERROR errorMsg = " + errorMsg);
                mHandler.sendMessage(mHandler.obtainMessage(MSG_SERVICE_ERROR, errorMsg));
            }
        }

        @Override
        public void onVoiceServiceConnect() {
            // do noting
        }
    }
}
