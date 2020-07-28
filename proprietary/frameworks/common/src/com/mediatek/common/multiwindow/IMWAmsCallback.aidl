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
interface IMWAmsCallback {
  ///for AMS
  void restoreStack(in IBinder token,in boolean toMax); 
  String findProcessNameByToken(in IBinder token);
  boolean moveActivityTaskToFront(in IBinder token);
  int findStackIdByToken(in IBinder token);
  int findStackIdByTask(in int taskId);
}


