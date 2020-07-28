package com.mediatek.incallui.wfc;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.widget.Toast;

import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.android.incallui.Log;
import com.android.incallui.R;

import com.mediatek.incallui.plugin.ExtensionManager;
import com.mediatek.wfo.IMwiService;
import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.MwisConstants;
import com.mediatek.wfo.WifiOffloadManager;

import mediatek.telecom.MtkCall;

/**
 * RoveOutReceiver.
 */
public class RoveOutReceiver extends WifiOffloadManager.Listener {
  private static final String TAG = "RoveOutReceiver";
  private Context context;
  private Message msg = null;
  private static final int COUNT_TIMES = 3;
  private static final int EVENT_RESET_TIMEOUT = 1;
  private static final int CALL_ROVE_OUT_TIMER = 1800000;
  private IWifiOffloadService wfoService = null;

  /**
   * Constructor.
   * @param context context
   */
  public RoveOutReceiver(Context context) {
    this.context = context;
    IBinder wfoBinder = ServiceManager.getService(WifiOffloadManager.WFO_SERVICE);
    Log.d(TAG, "RoveOutReceiver constructor " + wfoBinder);
    if (wfoBinder != null) {
      wfoService = IWifiOffloadService.Stub.asInterface(wfoBinder);
    } else {
      wfoBinder = ServiceManager.getService(MwisConstants.MWI_SERVICE);
      try {
        if (wfoBinder != null) {
          wfoService = IMwiService.Stub.asInterface(wfoBinder)
              .getWfcHandlerInterface();
        } else {
          Log.d(TAG, "No MwiService exist");
        }
      } catch (RemoteException e) {
        Log.d(TAG, "can't get MwiService");
      }
    }
    Log.d(TAG, "wfoService is" + wfoService);
  }

  /**
   * register RoveOutReceiver for handover events.
   */
  public void register() {
    if (wfoService != null) {
      try {
        Log.d(TAG, "RoveOutReceiver register wfoService");
        wfoService.registerForHandoverEvent(this);
      } catch (RemoteException e) {
        Log.i(TAG, "RemoteException RoveOutReceiver()");
      }
    }
  }

  /**
   * unregister RoveOutReceiver.
   */
  public void unregister() {
    if (wfoService != null) {
      try {
        Log.d(TAG, "RoveOutReceiver unregister wfoService ");
        wfoService.unregisterForHandoverEvent(this);
      } catch (RemoteException e) {
        Log.i(TAG, "RemoteException RoveOutReceiver()");
      }
      WfcDialogActivity.sCount = 0;
      if (msg != null) {
        handler.removeMessages(msg.what);
      }
    }
  }

  @Override
  public void onHandover(int simIdx, int stage, int ratType) {
    Log.d(TAG, "onHandover stage: " + stage + "ratType : " + ratType);
    ExtensionManager.getInCallExt().showHandoverNotification(handler, stage, ratType);
    checkForVideoOverWifi(handler, stage, ratType);
  }

  @Override
  public void onRoveOut(int simIdx, boolean roveOut, int rssi) {
    Log.d(TAG, "onRoveOut: " + roveOut);
    DialerCall call = CallList.getInstance().getActiveOrBackgroundCall();
    if (roveOut) {
      if ((call != null && call.hasProperty(android.telecom.Call.Details.PROPERTY_WIFI))
          && (WfcDialogActivity.sCount < COUNT_TIMES)
          && !WfcDialogActivity.isShowing) {
        final Intent intent = new Intent(context, WfcDialogActivity.class);
        intent.putExtra(WfcDialogActivity.SHOW_WFC_ROVE_OUT_POPUP, true);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
        if (WfcDialogActivity.sCount == 0) {
          msg = handler.obtainMessage(EVENT_RESET_TIMEOUT);
          handler.removeMessages(msg.what);
          handler.sendMessageDelayed(msg, CALL_ROVE_OUT_TIMER);
          Log.i(TAG, "WfcSignalReceiver sendMessageDelayed ");
        }
      }
    }
  }

  private Handler handler = new Handler() {
    @Override
    public void handleMessage(Message msg) {
      switch (msg.what) {
        case EVENT_RESET_TIMEOUT:
          Log.i(TAG, "WfcSignalReceiver EVENT_RESET_TIMEOUT ");
          WfcDialogActivity.sCount = 0;
          break;
        default:
          Log.i(TAG, "Message not expected: ");
          break;
      }
    }
  };

  /**
   * M: check if video call over wifi is allowed or not.
   * if not allowed then show error toast to user and
   * convert video call to voice call on handover from LTE to WIFI
   *
   * @param handler handler
   * @param stage handover stage
   * @param ratType handover ratType
   */
  public void checkForVideoOverWifi(Handler handler, int stage, int ratType) {
    final DialerCall call = CallList.getInstance().getActiveOrBackgroundCall();
    if (call == null) {
      return;
    }
    if ((call.getVideoFeatures() == null) || call.isVideoCall() == false) {
      return;
    }
    boolean isVideoOverWifiDisabled =
        call.getVideoFeatures().disableVideoCallOverWifi();
    Log.d(TAG, "[WFC]checkForVideoOverWifi isVideoOverWifiDisabled = "
        + isVideoOverWifiDisabled);
    if (!isVideoOverWifiDisabled) {
      return;
    }
    if (call != null && !call.hasProperty(MtkCall.MtkDetails.MTK_PROPERTY_VOLTE)) {
      if (stage == WifiOffloadManager.HANDOVER_START &&
          ratType == WifiOffloadManager.RAN_TYPE_WIFI) {
        handler.post(new Runnable() {
          @Override
          public void run() {
            Toast.makeText(context, context.getResources().getString(
                R.string.video_over_wifi_not_available),
                Toast.LENGTH_SHORT).show();
            call.getVideoTech().downgradeToAudio();
          }
        });
      }
    }
  }
}

