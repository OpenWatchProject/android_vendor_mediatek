package com.mediatek.incallui.wfc;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.graphics.drawable.ColorDrawable;
import android.media.AudioManager;
import android.media.ToneGenerator;
import android.os.Bundle;
import android.view.WindowManager;

import com.android.incallui.Log;
import com.android.incallui.R;

/**
 * M: To Display WFC related dialogs.
 */
public class WfcDialogActivity extends Activity {
  private static final String TAG = "WfcDialogActivity";
  private AlertDialog generalErrorDialog;
  private ToneGenerator toneGenerator;
  private static final int DIAL_TONE_STREAM_TYPE = AudioManager.STREAM_VOICE_CALL;
  private static final int TONE_RELATIVE_VOLUME = 80;
  public static int sCount = 0;
  public static boolean isShowing = false;
  public static final String WFC_ERROR_LABEL = "label";
  public static final String WFC_ERROR_DECRIPTION = "description";

  public static final String SHOW_WFC_CALL_ERROR_POPUP = "show_wfc_call_error_popup";
  public static final String SHOW_WFC_ROVE_OUT_POPUP = "show_wfc_rove_out_popup";
  public static final String ACTION_IMS_SETTING = "android.settings.WIFI_SETTINGS";

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    getWindow().setBackgroundDrawable(new ColorDrawable(0));
  }

  /**
   * When some other activity covered on this dialog activity
   * we simply finish it to avoid the dialog flick.
   */
  @Override
  protected void onPause() {
    super.onPause();
    finish();
    stopTone();
  }

  @Override
  public void finish() {
    super.finish();
    // Don't show the return to previous task animation to avoid showing a black screen.
    // Just dismiss the dialog and undim the previous activity immediately.
    overridePendingTransition(0, 0);
    isShowing = false;
  }

  @Override
  protected void onNewIntent(Intent intent) {
    setIntent(intent);
  }

  @Override
  protected void onResume() {
    super.onResume();
    if (generalErrorDialog == null) {
      showErrorDialog();
      isShowing = true;
    }
  }

  @Override
  protected void onStop() {
    super.onStop();
    if (generalErrorDialog != null) {
      generalErrorDialog.dismiss();
      generalErrorDialog = null;
    }
  }

  private void showErrorDialog() {
    boolean showWfcRoveOutPopup = getIntent().getBooleanExtra(SHOW_WFC_ROVE_OUT_POPUP, false);
    if (showWfcRoveOutPopup) {
      showWfcRoveOutError();
      return;
    }
    boolean showWfcCallErrorPopup =
        getIntent().getBooleanExtra(SHOW_WFC_CALL_ERROR_POPUP, false);
    if (showWfcCallErrorPopup) {
      showWfcErrorDialog();
      return;
    }
  }

  private void showWfcRoveOutError() {
    if (generalErrorDialog == null) {
      generalErrorDialog = new AlertDialog.Builder(this)
          .setTitle(R.string.call_drop)
          .setMessage(R.string.call_drop_message)
          .setPositiveButton(R.string.ok, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
              onDialogDismissed();
              finish();
            }
          })
          .setOnCancelListener(new OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
              onDialogDismissed();
              finish();
            }
          })
          .create();
      generalErrorDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
      Log.d(TAG, "showWfcRoveOutError sCount" + sCount);
      generalErrorDialog.show();
      playTone();
      sCount++;
    }
  }

  /* For Showing wifi related popups on wfc call disconnects with error*/
  private void showWfcErrorDialog() {
    int id = 0;
    CharSequence label = getIntent().getCharSequenceExtra(WFC_ERROR_LABEL);
    CharSequence description = getIntent().getCharSequenceExtra(WFC_ERROR_DECRIPTION);
    CharSequence wfcText = this.getResources().getString(R.string.wfc_wifi_call_drop_summary);
    if (wfcText.equals(description)) {
      id = R.string.no_thanks;
    } else {
      id = R.string.close;
    }
    Log.i(TAG, "showWfcErrorDialog " + label);
    Log.i(TAG, "showWfcErrorDialog " + description);
    generalErrorDialog = new AlertDialog.Builder(this)
        .setTitle(label)
        .setMessage(description)
        .setPositiveButton(R.string.view_networks, new OnClickListener() {
          @Override
          public void onClick(DialogInterface dialog, int which) {
            enableWifi();
          }
        })
        .setNegativeButton(id, new DialogInterface.OnClickListener() {
          @Override
          public void onClick(DialogInterface dialog, int which) {
            onDialogDismissed();
            finish();
          }
        })
        .create();
    generalErrorDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
    generalErrorDialog.show();
  }

  private void enableWifi() {
    Intent intent = new Intent();
    intent.setAction(ACTION_IMS_SETTING);
    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
    this.startActivity(intent);
    finish();
  }

  private void playTone() {
    if (toneGenerator == null) {
      toneGenerator = new ToneGenerator(DIAL_TONE_STREAM_TYPE, TONE_RELATIVE_VOLUME);
    }
    toneGenerator.startTone(ToneGenerator.TONE_PROP_BEEP2);
  }

  private void stopTone() {
    if (toneGenerator != null) {
      toneGenerator.stopTone();
      toneGenerator.release();
      toneGenerator = null;
      Log.i(this, "[WFC]onPause tonegenrator stopped ");
    }
  }

  private void onDialogDismissed() {
    if (generalErrorDialog != null) {
      generalErrorDialog.dismiss();
      generalErrorDialog = null;
    }
  }
}

