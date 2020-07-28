package com.mediatek.common.multiwindow;

import android.app.PendingIntent;
import com.mediatek.common.multiwindow.IMWAmsCallback;
import com.mediatek.common.multiwindow.IMWPhoneWindowCallback;
import com.mediatek.common.multiwindow.IMWWmsCallback;
import com.mediatek.common.multiwindow.IMWSystemUiCallback;
import android.os.IBinder;
import android.os.WorkSource;
import android.content.ComponentName;


/**
 * System private API for talking with the alarm manager service.
 *
 * {@hide}
 */
interface IMultiWindowManager {

    void moveActivityTaskToFront(in IBinder token);
    void closeWindow(in IBinder token);
    void restoreWindow(in IBinder token, in boolean toMax);
    void setAMSCallback(IMWAmsCallback cb);
    void setSystemUiCallback(IMWSystemUiCallback cb);    
    void stickWindow(in IBinder token, in boolean isSticky);                
    boolean isFloatingStack(in int stackId);
       
    void setFloatingStack(in int stackId); 
    
    // for Window Manager Service
    void setWMSCallback(IMWWmsCallback cb); 
    boolean isStickStack(in int stackId);
    boolean isInMiniMax(in int taskId);
    
    /// called  by window manager policy
    void moveFloatingWindow(in int disX, in int disY);
    void resizeFloatingWindow(in int direction, in int deltaX, in int deltaY);
    void enableFocusedFrame(in boolean enable);
  
    /// called by ActivityStackSupervisor
    void miniMaxTask(in int taskId); 

    // for showing restore button on systemUI module
    void showRestoreButton(in boolean flag); 
   
    /// black name list
    boolean matchConfigNotChangeList(in String packageName);
    boolean matchDisableFloatPkgList(in String packageName);
    boolean matchDisableFloatActivityList(in String ActivityName);
    boolean matchDisableFloatWinList(in String winName);
    List<String> getDisableFloatPkgList();
    List<String> getDisableFloatComponentList();
    boolean matchMinimaxRestartList(in String packageName);
    boolean matchConfigChangeList(in String packageName);
    
    void addDisableFloatPkg(in String packageName);
    void addConfigNotChangePkg(in String packageName);
    void addMiniMaxRestartPkg(in String packageName);
    int appErrorHandling(in String packageName, in boolean inMaxOrRestore, in boolean defaultChangeConfig);

    boolean isSticky(in IBinder token);

    void activityCreated(in IBinder token);
    void taskAdded(in int taskId);
    void taskRemoved(in int taskId);


}


