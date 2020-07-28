package com.mediatek.common.multiwindow;

import android.app.PendingIntent;
import android.os.IBinder;
import android.os.WorkSource;
import android.content.ComponentName;


/**
 * System private API for talking with the alarm manager service.
 *
 * {@hide}
 */
interface IMWPhoneWindowCallback {
  ///for phone window 
  void setWindowType(in IBinder token,in int windowType); 
  void setFloatDecorVisibility(in int visibility);     
}


