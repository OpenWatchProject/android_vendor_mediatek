package com.mediatek.voicewakeup;

import android.app.AlertDialog;
import android.app.Activity;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.ComponentName;
import android.content.Context;
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

/**
 * For voice wake up's no speaker id mode
 * 
 */
public class VowNoSpeaker extends PreferenceActivity {

    @Override
    public Intent getIntent() {
        Intent modIntent = new Intent(super.getIntent());
        modIntent.putExtra(EXTRA_SHOW_FRAGMENT, VowNoSpeakerFragment.class.getName());
        modIntent.putExtra(EXTRA_NO_HEADERS, true);
        return modIntent;
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return (VowNoSpeakerFragment.class.getName().equals(fragmentName));
    }

    /**
     * Voice wakeup's no speaker id fragment.
     *
     */
    public static class VowNoSpeakerFragment extends SettingsPreferenceFragment implements
        Utils.VoiceServiceListener {

        static final String TAG = "VowNoSpeaker";
        // / { @ Same with voice ui
        // wakeup id 0: camera, 1: contacts, 2: googleNow
        public static final String KEY_COMMAND_ID = "command_id";
        // wakeup package name
        public static final String KEY_COMMAND_PACKAGENAME = "command_packagename";
        // wakeup class name
        public static final String KEY_COMMAND_CLASSNAME = "command_classname";
        // wakeup keyword
        public static final String KEY_COMMAND_KEYWORD = "command_keyword";
        // / @ }

        private static final int MSG_SERVICE_ERROR = 1;
        private static final int MSG_SERVICE_CHANGE_KEYPHRASE = 2;

        private static final int COMFIRM_RESET_DIALOG = 0;
        private static final int DYNAMIC_KEYPHRASE_DIALOG = 1;

        private static final String PREF_KEY_PLAY_COMMAND = "play_command";
        private static final String PREF_KEY_INFORCE_COMMAND = "reinforece_command";
        private static final String PREF_KEY_RESET_COMMAND = "reset_command";
        private static final String PREF_KEY_SELECT_KEYWORD_COMMAND = "select_keyword_command";

        private static boolean sNeedUpdate = false;
        private static int sCommandId;

        private Context mContext;
        private Preference mPlayPref;
        private Preference mResetPref;
        private Preference mSelectKeyPref;
        private Utils mUtils;

        private ComponentName[] mComponentArray;
        private String mCommandKeyword;
        private String[] mCmdKeywordArray;
        private String mComponetStr;
        private String mAppLabel;

        private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_SERVICE_ERROR:
                    handleServiceError((String) msg.obj);
                    break;
                case MSG_SERVICE_CHANGE_KEYPHRASE:
                    handleServiceChangeKeyphrase(msg.arg1, msg.arg2);
                    break;
                default:
                    break;
                }
            }
        };

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            Activity activity = getActivity();
            mContext = activity.getBaseContext();
            mUtils = Utils.getInstance();
            mUtils.setContext(mContext);

            sNeedUpdate = false;
            sCommandId = activity.getIntent().getIntExtra(KEY_COMMAND_ID, 0);
            mCmdKeywordArray = activity.getIntent().getStringArrayExtra(KEY_COMMAND_KEYWORD);
            mCommandKeyword = mCmdKeywordArray[sCommandId];
            Log.d("@M_" + TAG, "sCommandId = " + sCommandId
                + ", mCommandKeyword = " + mCommandKeyword);
            String[] packageStrArray =
                activity.getIntent().getStringArrayExtra(KEY_COMMAND_PACKAGENAME);
            String[] classStrArray =
                activity.getIntent().getStringArrayExtra(KEY_COMMAND_CLASSNAME);
            mComponentArray = mUtils.getComponentArray(classStrArray, packageStrArray);
            ComponentName component = null;
            if (mComponentArray != null || sCommandId > mComponentArray.length) {
                component = mComponentArray[sCommandId];
                mAppLabel = mUtils.getAppLabel(component);
                mComponetStr = component.flattenToShortString();
            } else {
                mAppLabel = "";
                mComponetStr = "";
            }
            Log.d("@M_" + TAG, "component = " + mComponetStr + "mAppLabel = " + mAppLabel);

            addPreferencesFromResource(R.xml.voice_wakeup);
            // set title
            CharSequence msg =
                mContext.getString(R.string.voice_wake_up_command_title, mAppLabel);
            ((PreferenceActivity) activity).showBreadCrumbs(msg, msg);
            initPreferences();
        }

        @Override
        public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
                Preference preference) {
            final String key = preference.getKey();
            Log.d("@M_" + TAG, "onPreferenceTreeClick commandId = " + key);

            switch (key) {
            case PREF_KEY_PLAY_COMMAND: {
                mUtils.playCommand(this, sCommandId, mContext.getString(
                        R.string.voice_wake_up_command_summary, mAppLabel),
                        Utils.VOW_NO_SPEAKER_MODE);
            }
            break;
            case PREF_KEY_INFORCE_COMMAND: {
                // no speaker id mode
                mUtils.updateCommand(this, sCommandId, mComponetStr,
                        mPlayPref.isEnabled() ? Utils.COMMAND_TYPE_MODIFY
                                : Utils.COMMAND_TYPE_RECORD, Utils.VOW_NO_SPEAKER_MODE,
                        mCommandKeyword);
            }
            break;
            case PREF_KEY_RESET_COMMAND: {
                showDialog(COMFIRM_RESET_DIALOG);
            }
            break;
            case PREF_KEY_SELECT_KEYWORD_COMMAND: {
                // no speaker id mode
                //Check if preference mismatch, then need modify
                showDialog(DYNAMIC_KEYPHRASE_DIALOG);
            }
            break;
            default:
                return false;
            }
            return true;
        }

        @Override
        public void onResume() {
        Log.v("@M_" + TAG, "onResume commandId =" + sCommandId);
            super.onResume();
            if (sNeedUpdate == true) {
                updateSummaryAndData(sCommandId);
            }
            updatePlayResetStatus();
            mUtils.setContext(mContext);
            mUtils.setOnChangedListener(this);
            mUtils.onResume();
        }

        @Override
        public void onPause() {
            Log.v("@M_" + TAG, "onPause commandId =" + sCommandId);
            mUtils.onPause();
            mUtils.setOnChangedListener(null);
            super.onPause();
            sNeedUpdate = false; ///TODO; If not already false, then too reset this state.
        }

        /**
         *  Initialize preferences.
         */
        private void initPreferences() {
            Log.d("@M_" + TAG, "initPreferences ");
            mPlayPref = new Preference(mContext);
            mPlayPref.setKey(PREF_KEY_PLAY_COMMAND);
            mPlayPref.setTitle(getString(R.string.play_command_action));
            mPlayPref.setSummary(mCommandKeyword);
            getPreferenceScreen().addPreference(mPlayPref);
            mResetPref = new Preference(mContext);
            mResetPref.setKey(PREF_KEY_RESET_COMMAND);
            mResetPref.setTitle(getString(R.string.reset_command_action));
            mResetPref.setSummary(mCommandKeyword);
            getPreferenceScreen().addPreference(mResetPref);
            Preference preference = new Preference(mContext);
            preference.setKey(PREF_KEY_INFORCE_COMMAND);
            preference.setTitle(getString(R.string.reinforce_command_action));
            preference.setSummary(getString(R.string.reinforce_command_action_summary));
            getPreferenceScreen().addPreference(preference);
            if (mCmdKeywordArray.length > 1) {
                mSelectKeyPref = new Preference(mContext);
                mSelectKeyPref.setKey(PREF_KEY_SELECT_KEYWORD_COMMAND);
                mSelectKeyPref.setTitle(getString(R.string.voice_command_select_title));
                mSelectKeyPref.setSummary(mCommandKeyword);
                getPreferenceScreen().addPreference(mSelectKeyPref);
            }
        }

        /**
         *  Set need update from across different flows.
         *
         * @param commandId
         *            Command id
         *
         */
        public static void setNeedUpdate(int commandId) {
            Log.v("@M_" + TAG, "setNeedUpdate commandId = " + commandId);
            sCommandId = commandId;
            sNeedUpdate = true;
        }

        /**
         *  Update play button and reset button states.
         */
        private void updatePlayResetStatus() {
            mPlayPref.setEnabled(MtkSettingsExt.System.getVoiceCommandValue(getContentResolver(),
                    MtkSettingsExt.System.BASE_VOICE_WAKEUP_COMMAND_KEY, sCommandId) != null);
            mResetPref.setEnabled(MtkSettingsExt.System.getVoiceCommandValue(getContentResolver(),
                    MtkSettingsExt.System.BASE_VOICE_WAKEUP_COMMAND_KEY, sCommandId) != null);
        }

        /**
         *  Update summary and data part of preferences.
         *
         * @param commandId
         *            Command id
         */
        public void updateSummaryAndData(int commandId) {
            Log.d("@M_" + TAG, "updateSummaryAndData commandId = " + commandId);
            ComponentName component = mComponentArray[commandId];

            mAppLabel = mUtils.getAppLabel(component);
            mComponetStr = component.flattenToShortString();
            sCommandId = commandId;

            mCommandKeyword =  mCmdKeywordArray[commandId];
            if (mSelectKeyPref != null) {
                mSelectKeyPref.setSummary(mCommandKeyword);
            }
            mPlayPref.setSummary(mCommandKeyword);
            mResetPref.setSummary(mCommandKeyword);
            CharSequence msg = mContext.getString(R.string.voice_wake_up_command_title, mAppLabel);
            ((PreferenceActivity) getActivity()).showBreadCrumbs(msg, msg);
            sNeedUpdate = false;
        }

        /**
         *  Send switch keyphrase command.
         *
         * @param commandId
         *            Command id
         */
        private void switchKeyphraseCommand(int commandId) {
            if (mUtils.mVCmdMgrService != null) {
                Bundle extra = new Bundle();
                Log.d("@M_" + TAG, "sendCommand TRAINING_SWITCH commandId = " + commandId);
                extra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, commandId);
                mUtils.sendVoiceCommand(mContext.getPackageName(),
                        VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                        VoiceCommandListener.ACTION_VOICE_TRAINING_CHANGE_KEYPHRASE, extra);
            }
        }

        @Override
        public Dialog onCreateDialog(int dialogId) {
            switch (dialogId) {
            case COMFIRM_RESET_DIALOG: {
                Log.d("@M_" + TAG, "[startConfirmResetDialog]");
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
            }
            case DYNAMIC_KEYPHRASE_DIALOG: {
                Log.d("@M_" + TAG, "[startDynamicKeywordsDialog]");
                SettingsPreferenceFragment fragContext = this;
                DialogInterface.OnClickListener clickListener =
                    new DialogInterface.OnClickListener() {
                  @Override
                  public void onClick(DialogInterface dialog, int item) {
                    if ((item < 0) || (item > mCmdKeywordArray.length)) {
                        Log.d("@M_" + TAG, "[startDynamicKeywordsDialog] Out of range item_id : "
                            + item);
                        item = sCommandId; //Override to avoid unexpected errors.
                    }
                    if (item != sCommandId) {
                        switchKeyphraseCommand(item);
                    } else {
                        Log.d("@M_" + TAG, "[startDynamicKeywordsDialog] selection is same."
                            + " CommandId : " + sCommandId);
                        if (!mPlayPref.isEnabled()) {
                            mUtils.updateCommand(fragContext, sCommandId, mComponetStr,
                                 Utils.COMMAND_TYPE_RECORD, Utils.VOW_NO_SPEAKER_MODE,
                                mCommandKeyword);
                        } else {
                            Toast.makeText(mContext, getString(R.string.vow_done_label),
                                Toast.LENGTH_SHORT);
                        }
                    }
                    dialog.dismiss();
                  }
                };

                AlertDialog dialog = new AlertDialog.Builder(getActivity())
                    .setTitle(R.string.voice_command_select_title)
                    .setSingleChoiceItems((CharSequence[]) mCmdKeywordArray, sCommandId,
                        clickListener)
                    .setCancelable(true)
                    .setPositiveButton(R.string.voice_unlock_ok_label,
                        clickListener).create();
                return dialog;
            }
            default:
                return null;
            }
        }

        /**
         *  Send Reset enrolled keyphrase command.
         */
        private void resetCommand() {
            if (mUtils.mVCmdMgrService != null) {
                int count = mUtils.countEnrolledModel(mCmdKeywordArray.length);
                // Write enable settings to reset state
                if (count == 1) {
                    Settings.System.putInt(getContentResolver(),
                            MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS, 0);
                }

                Bundle extra = new Bundle();
                Log.d("@M_" + TAG, "sendCommand TRAINING_RESET commandId = " + sCommandId);
                extra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, sCommandId);
                extra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, 1);
                mUtils.sendVoiceCommand(mContext.getPackageName(),
                        VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                        VoiceCommandListener.ACTION_VOICE_TRAINING_RESET, extra);

                MtkSettingsExt.System.setVoiceCommandValue(getContentResolver(),
                    MtkSettingsExt.System.BASE_VOICE_WAKEUP_COMMAND_KEY, sCommandId, null);
            }
            getActivity().finish();
        }

        /**
         *  Handle error part in case of service response.
         *
         * @param errorMsg
         *            error message
         */
        private void handleServiceError(String errorMsg) {
            Toast.makeText(mContext, errorMsg, Toast.LENGTH_SHORT);
        }

        /**
         *  Handle the service response when no error.
         *
         * @param status
         *            service response value for status
         * @param commandId
         *            Command Id
         */
        private void handleServiceChangeKeyphrase(int status, int commandId) {
            Log.d("@M_" + TAG, "handleServiceChangeKeyphrase status = " + status
                + ", commandId = " + commandId);
            if (status == 1) {
                Log.d("@M_" + TAG, "handleServiceChangeKeyphrase Training already done with "
                    + mCommandKeyword);
                Toast.makeText(mContext, getString(R.string.vow_done_label),
                                Toast.LENGTH_SHORT);
                updateSummaryAndData(commandId);
                updatePlayResetStatus();
            } else {
                ComponentName component = mComponentArray[commandId];
                String componentStr = component.flattenToShortString();
                String commandKeyword =  mCmdKeywordArray[commandId];

                mUtils.updateCommand(this, commandId, componentStr,
                        Utils.COMMAND_TYPE_RECORD, Utils.VOW_NO_SPEAKER_MODE,
                        commandKeyword);
            }
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
            } else if (result == VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS) {
                switch (subAction) {
                case VoiceCommandListener.ACTION_VOICE_TRAINING_RESET: {
                    Log.d("@M_" + TAG, "onNotified ACTION_VOICE_TRAINING_RESET");
                    int isNeedUpdateCommandId = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                    int commandId = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                    if (isNeedUpdateCommandId != 0) {
                        mHandler.sendMessage(mHandler.obtainMessage(MSG_SERVICE_CHANGE_KEYPHRASE,
                            isNeedUpdateCommandId, commandId));
                    }
                }
                break;

                case VoiceCommandListener.ACTION_VOICE_TRAINING_CHANGE_KEYPHRASE: {
                    Log.d("@M_" + TAG, "onNotified TRAINING_CHANGE_KEYPHRASE");
                    int checkStatus = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                    int commandId = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_SERVICE_CHANGE_KEYPHRASE,
                        checkStatus, commandId));
                }
                break;

                default:
                    break;
                }
            }
        }

        @Override
        public void onVoiceServiceConnect() {
            // do noting
        }
    }
}
