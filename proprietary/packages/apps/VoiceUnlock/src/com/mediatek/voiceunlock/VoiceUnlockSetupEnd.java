package com.mediatek.voiceunlock;

import android.content.Intent;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.mediatek.voicewakeup.Utils;

public class VoiceUnlockSetupEnd extends PreferenceActivity {

    // required constructor for fragments
    public VoiceUnlockSetupEnd() {

    }

    @Override
    public Intent getIntent() {
        Intent modIntent = new Intent(super.getIntent());
        modIntent.putExtra(EXTRA_SHOW_FRAGMENT, VoiceUnlockSetupEndFragment.class.getName());
        modIntent.putExtra(EXTRA_NO_HEADERS, true);
        return modIntent;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        CharSequence msg = getText(R.string.voice_unlock_setup_intro_header);
        showBreadCrumbs(msg, msg);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        if (VoiceUnlockSetupEndFragment.class.getName().equals(fragmentName)) {
            return true;
        }
        return false;
    }

    public static class VoiceUnlockSetupEndFragment extends SettingsPreferenceFragment implements
            View.OnClickListener {
        private View mOkButton;
        private TextView mPromptTxt;
        private Utils mUtils;
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            mUtils = Utils.getInstance();
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            View view = inflater.inflate(R.layout.voice_unlock_setup_end, null);
            mPromptTxt = (TextView)view.findViewById(R.id.txt_prompt);
            /* M: Not required as we already have separate res for tablet
            ** Moreover, voice unlock feature is not being used now, rather only voiceUnlock
            *if ("tablet".equals(android.os.SystemProperties.get("ro.build.characteristics"))) {
            *    String str = getActivity().getResources()
            *                  .getString(R.string.voice_unlock_setup_end_prompt);
            *    str = str.replace("phone", "tablet");
            *    mPromptTxt.setText(str);
            *}
            **/
            mOkButton = view.findViewById(R.id.ok_button);
            mOkButton.setOnClickListener(this);
            // to over use the same file for voice unlock and voice wake up.
            int wakeupMode = mUtils.getWakeupMode(getActivity());
            // wake up , reset the description
            if (wakeupMode == mUtils.VOICE_WAKEUP_ANYONE ||
                     wakeupMode == mUtils.VOICE_WAKEUP_COMMAND) {
                mPromptTxt.setText(R.string.voice_wakeup_setup_end_prompt);
            }

            return view;
        }

        public void onClick(View v) {
            getActivity().finish();
        }
    }
}
