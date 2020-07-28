package com.mediatek.settings.inputmethod;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.android.settings.R;
import com.android.settings.password.ChooseLockSettingsHelper;

public class VowKeyguardConfirm extends Activity {

    private static final String TAG = "VowKeyguardConfirm";
    private static final int KEYGUARD_REQUEST = 55;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        if (!runKeyguardConfirmation(KEYGUARD_REQUEST)) {
            // if no pass word, finish this confirm activity
            setResult(Activity.RESULT_OK);
            finish();
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.d(TAG, "onActivityResult: requestCode = " + requestCode + ", resultCode = "
                + resultCode);
        if (requestCode != KEYGUARD_REQUEST) {
            return;
        }

        // If the user entered a valid keyguard trace, present the final
        // confirmation prompt; otherwise, go back to the initial state.
        if (resultCode == Activity.RESULT_OK) {
            setResult(Activity.RESULT_OK);
            finish();
        } else {
            setResult(Activity.RESULT_CANCELED);
            finish();
        }
    }

    /**
     * Keyguard validation is run using the standard {@link ConfirmLockPattern}
     * component as a subactivity
     *
     * @param request
     *            the request code to be returned once confirmation finishes
     * @return true if confirmation launched
     */
    private boolean runKeyguardConfirmation(int request) {
        return new ChooseLockSettingsHelper(this).launchConfirmationActivity(request, getIntent()
                .getCharSequenceExtra("title"), null, null, 0);
    }
}
