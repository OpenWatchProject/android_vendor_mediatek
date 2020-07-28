package com.mediatek.camera.portability;

import android.os.HwBinder;
import android.os.RemoteException;

import vendor.mediatek.hardware.camera.bgservice.V1_0.IBGService;
import vendor.mediatek.hardware.camera.bgservice.V1_0.IEventCallback;

/**
 * The background service interface.
 */
public class BGService {
    public interface HwBinderDeathRecipient {
        void serviceDied(long cookie);
    }

    /**
     *  The service callback.
     */
    public interface IEventCallbackStub {
        boolean onCompleted(int imgReaderId, int frameNumber, int status);
    }

    private IBGService mBGService;

    private BGService(IBGService bg) {
        mBGService = bg;
    }

    /**
     * Get BG service.
     * @param the service param
     * @param the service param
     * @return BG service instance.
     */
    public static BGService getService(String arg0, boolean arg1) {
        try {
            return new BGService(IBGService.getService("internal/0", true));
        } catch (RemoteException e) {
            return null;
        }
    }

    /**
     * Regist the service death listener.
     * @param arg0 the service param
     * @param arg1 the service param.
     */
    public void linkToDeath(HwBinderDeathRecipient arg0, int arg1) {
        try {
            mBGService.linkToDeath(new HwBinder.DeathRecipient() {
                @Override
                public void serviceDied(long cookie) {
                    arg0.serviceDied(cookie);
                }
            }, arg1);
        } catch (RemoteException e) {

        }
    }

    /**
     * Set service event callback
     * @param arg0
     * @param arg1  the callback.
     */
    public void setEventCallback(int arg0, IEventCallbackStub arg1) {
        try {
            mBGService.setEventCallback(arg0, new IEventCallback.Stub() {

                @Override
                public boolean onCompleted(int imgReaderId, int frameNumber, int status) {
                    return arg1.onCompleted(imgReaderId, frameNumber, status);
                }
            });
        } catch (RemoteException e) {

        }
    }
}