package com.mediatek.powerhalmgr;

import android.os.IRemoteCallback;
import com.mediatek.powerhalmgr.DupLinkInfo;

/** @hide */
interface IPowerHalMgr {
    int scnReg();
    oneway void scnConfig(int handle, int cmd, int param_1, int param_2, int param_3, int param_4);
    oneway void scnUnreg(int handle);
    oneway void scnEnable(int handle, int timeout);
    oneway void scnDisable(int handle);
    oneway void scnUltraCfg(int handle, int ultracmd, int param_1, int param_2, int param_3, int param_4);
    oneway void mtkCusPowerHint(int hint, int data);
    oneway void getCpuCap();
    oneway void getGpuCap();
    oneway void getGpuRTInfo();
    oneway void getCpuRTInfo();
    oneway void UpdateManagementPkt(int type, String packet);
    oneway void setForegroundSports();
    oneway void setSysInfo(int type, String data);

    // M: DPP @{
    boolean startDuplicatePacketPrediction();
    boolean stopDuplicatePacketPrediction();
    boolean isDupPacketPredictionStarted();
    boolean registerDuplicatePacketPredictionEvent(in IRemoteCallback listener);
    boolean unregisterDuplicatePacketPredictionEvent(in IRemoteCallback listener);
    boolean updateMultiDuplicatePacketLink(in DupLinkInfo[] linkList);
    // @}

    oneway void setPredictInfo(String pack_name, int uid);
    int perfLockAcquire(int handle, int duration, in int[] list);
    oneway void perfLockRelease(int handle);
    int querySysInfo(int cmd, int param);
}
