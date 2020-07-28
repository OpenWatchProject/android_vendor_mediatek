package com.android.camera.externaldevice;

public interface IExternalDeviceCtrl {

    public interface Listener {
        void onStateChanged(boolean enabled);
    }

    public boolean onCreate();
    public boolean onResume();
    public boolean onPause();
    public boolean onDestory();
    public boolean onOrientationChanged(int orientation);
    public void addListener(Object listenr);
    public void removeListener(Object listenr);

}
