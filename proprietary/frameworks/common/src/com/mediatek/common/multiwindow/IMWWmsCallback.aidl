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
interface IMWWmsCallback {

  ///for WMS
  /// called  by window manager policy
  void moveFloatingWindow(in int disX, in int disY);
  
  /// called  by window manager policy
  void resizeFloatingWindow(in int direction, in int deltaX, in int deltaY);
  
  /// called  by window manager policy
  void enableFocusedFrame(in boolean enable);
  
  /// called by ActivityStackSupervisor
  void miniMaxTask(in int taskId); 
}


