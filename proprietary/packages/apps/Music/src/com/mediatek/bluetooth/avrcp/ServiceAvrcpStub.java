
package com.mediatek.bluetooth.avrcp;

import android.os.RemoteException;
import android.os.RemoteCallbackList;

import com.android.music.MediaPlaybackService;
import com.android.music.MusicLogUtils;

public class ServiceAvrcpStub extends IBTAvrcpMusic.Stub {

    private final static String TAG = "SERVICE_AVRCP_STUB";

    private MediaPlaybackService mService;

    private int mRepeatMode = 1;

    private int mShuffleMode = 1;

    public ServiceAvrcpStub(MediaPlaybackService mediaPlaybackService) {
        this.mService = mediaPlaybackService;
    }

    final RemoteCallbackList<IBTAvrcpMusicCallback> mAvrcpCallbacksList =
     new RemoteCallbackList<IBTAvrcpMusicCallback>();

    public void registerCallback(IBTAvrcpMusicCallback callback) {

        MusicLogUtils.d(TAG, "[AVRCP] ServiceAvrcpStub. registerCallback");
        if (callback != null) {
            mAvrcpCallbacksList.register(callback);
        }
        getRepeatMode();
        getShuffleMode();
    }

    public void unregisterCallback(IBTAvrcpMusicCallback callback) {
        MusicLogUtils.d(TAG, "[AVRCP] ServiceAvrcpStub. unregisterCallback");
        if (callback != null) {
            mAvrcpCallbacksList.unregister(callback);
        }
    }

    public boolean regNotificationEvent(byte eventId, int interval) {
        switch (eventId) {
            case 0x01: // playstatus
                bPlaybackFlag = true;
                MusicLogUtils.v(TAG, "[AVRCP] bPlaybackFlag flag is " + bPlaybackFlag);
                return true;
            case 0x02: // track change
                bTrackchangeFlag = true;
                MusicLogUtils.v(TAG, "[AVRCP] bTrackchange flag is " + bTrackchangeFlag);
                return bTrackchangeFlag;
            case 0x09: // playing content
                bTrackNowPlayingChangedFlag = true;
                return true;
            default:
                MusicLogUtils.e(TAG, "[AVRCP] MusicApp doesn't support eventId:" + eventId);
                break;
        }

        return false;
    }

    public boolean setPlayerApplicationSettingValue(byte attrId, byte value) {
        return false;
    }

    public byte[] getCapabilities() {
        return null;
    }

    public void stop() {
        mService.stop();
    }

    public void pause() {
        mService.pause();
    }

    public void resume() {
    }

    public void play() {
        mService.play();
    }

    public void prev() {
        mService.prev();
    }

    public void next() {
        mService.gotoNext(true);
    }

    public void nextGroup() {
        mService.gotoNext(true);
    }

    public void prevGroup() {
        mService.prev();
    }

    public byte getPlayStatus() {
        if (true == mService.isPlaying()) {
            return 1; // playing
        } else {
            if (!mService.isCursorNull()) {
                return 2; // pause
            }
            return 0; // stop
        }
    }

    public long getAudioId() {
        return mService.getAudioId();
    }

    public String getTrackName() {
        return mService.getTrackName();
    }

    public String getAlbumName() {
        return mService.getAlbumName();
    }

    public long getAlbumId() {
        return mService.getAlbumId();
    }

    public String getArtistName() {
        return mService.getArtistName();
    }

    public long position() {
        return mService.position();
    }

    public long duration() {
        return mService.duration();
    }

    public boolean setEqualizeMode(int equalizeMode) {
        return false;
    }

    public int getEqualizeMode() {
        return 0;
    }

    public boolean setShuffleMode(int shufflemode) {
        int mode = 0;
        switch (shufflemode) {
            case 1: // SHUFFLE_NONE
                mode = 0;
                break;
            case 2: // SHUFFLE_NORMAL
                mode = 1;
                break;
            default:
                return false;
        }
        MusicLogUtils.d(TAG, "[AVRCP] setShuffleMode music_mode:" + mode);
        mService.setShuffleMode(mode);
        // sendBTDelaySetting( 2, mode);
        return true;
    }

    public int getShuffleMode() {
        mShuffleMode = (mService.getShuffleMode() + 1);
        return mShuffleMode;
    }

    public boolean setRepeatMode(int repeatmode) {
        // avrcp repeat mode to local mode TODO: delay this
        // REPEAT_NONE = 0;
        // REPEAT_CURRENT = 1;
        // REPEAT_ALL = 2;
        int mode = 0;
        switch (repeatmode) {
            case 1: // REPEAT_MODE_OFF
                mode = 0;
                break;
            case 2: // REPEAT_MODE_SINGLE_TRACK
                mode = 1;
                break;
            case 3: // REPEAT_MODE_ALL_TRACK
                mode = 2;
                break;
            default:
                return false;
        }
        MusicLogUtils.d(TAG, String.format("[AVRCP] setRepeatMode musid_mode:%d", mode));
        mService.setRepeatMode(mode);
        // sendBTDelaySetting( 3, mode);
        return true;
    }

    public int getRepeatMode() {
        mRepeatMode = (mService.getRepeatMode() + 1);
        return mRepeatMode;
    }

    public boolean setScanMode(int scanMode) {
        return false;
    }

    public int getScanMode() {
        return 0;
    }

    public boolean informDisplayableCharacterSet(int charset) {
        if (charset == 0x6a) {
            return true;
        }
        return false;
    }

    public boolean informBatteryStatusOfCT() {
        return true;
    }

    public void enqueue(long[] list, int action) {
        mService.enqueue(list, action);
    }

    public long[] getNowPlaying() {
        return mService.getQueue();
    }

    public String getNowPlayingItemName(long id) {
        return null;
    }

    public void open(long[] list, int position) {
        mService.open(list, position);
    }

    public int getQueuePosition() {
        return mService.getQueuePosition();
    }

    public void setQueuePosition(int index) {
        mService.setQueuePosition(index);
    }

    protected boolean bPlaybackFlag = false;

    protected boolean bTrackchangeFlag = false;

    protected boolean bTrackReachStartFlag = false;

    protected boolean bTrackReachEndFlag = false;

    protected boolean bTrackPosChangedFlag = false;

    protected boolean bTrackAppSettingChangedFlag = false;

    protected boolean bTrackNowPlayingChangedFlag = false;

    public void notifyBTAvrcp(String s) {

        MusicLogUtils.v(TAG, "[AVRCP] notifyBTAvrcp " + s);
        if (MediaPlaybackService.PLAYSTATE_CHANGED.equals(s)) {
            notifyPlaybackStatus(getPlayStatus());
        }
        if (MediaPlaybackService.PLAYBACK_COMPLETE.equals(s)) {
            notifyTrackChanged();
            // notifyTrackReachEnd();
        }
        if (MediaPlaybackService.QUEUE_CHANGED.equals(s)) {
            notifyTrackChanged();
            notifyNowPlayingContentChanged();
        }
        if (MediaPlaybackService.META_CHANGED.equals(s)) {
            notifyTrackChanged();
        }
    }

    /* AVRCP callback interface */
    protected void notifyPlaybackStatus(byte status) {
        // check the register & callback it back
        // if( true != bPlaybackFlag ){
        // MusicLogUtils.v(TAG, "notifyPlaybackStatus ignore bPlaybackFlag:" +
        // bPlaybackFlag);
        // return;
        // }
        bPlaybackFlag = false;

        final int N = mAvrcpCallbacksList.beginBroadcast();
        MusicLogUtils.d(TAG, "[AVRCP] notifyPlaybackStatus " + status + " N= " + N);
        for (int i = 0; i < N; i++) {
            IBTAvrcpMusicCallback listener = mAvrcpCallbacksList.getBroadcastItem(i);
            try {
                listener.notifyPlaybackStatus(status);
            } catch (RemoteException ex) {
                // The RemoteCallbackList will take care of removing the
                // dead listeners.
                MusicLogUtils.e(TAG, "Error:" + ex);
            }
        }
        mAvrcpCallbacksList.finishBroadcast();

    }

    protected void notifyTrackChanged() {
        // check the register & callback it back
        // if( true != bTrackchangeFlag ){
        // return;
        // }
        bTrackchangeFlag = false;

        final int N = mAvrcpCallbacksList.beginBroadcast();
        MusicLogUtils.d(TAG, "[AVRCP] notifyTrackChanged " + " N= " + N);
        for (int i = 0; i < N; i++) {
            IBTAvrcpMusicCallback listener = mAvrcpCallbacksList.getBroadcastItem(i);
            try {
                listener.notifyTrackChanged(getAudioId());
            } catch (RemoteException ex) {
                // The RemoteCallbackList will take care of removing the
                // dead listeners.
                MusicLogUtils.e(TAG, "Error:" + ex);
            }
        }
        mAvrcpCallbacksList.finishBroadcast();
    }

    protected void notifyTrackReachStart() {
        // check the register & callback it back
        if (true != bTrackReachStartFlag) {
            return;
        }

        // Default Music Player dones't support this
    }

    protected void notifyTrackReachEnd() {
        // check the register & callback it back
        if (true != bTrackReachEndFlag) {
            return;
        }

        // Default Music Player dones't support this
    }

    protected void notifyPlaybackPosChanged() {
        if (true != bTrackPosChangedFlag || null == mAvrcpCallbacksList) {
            return;
        }

        // Default Music Player dones't support this
    }

    protected void notifyAppSettingChanged() {
        if (true != bTrackAppSettingChangedFlag || null == mAvrcpCallbacksList) {
            return;
        }
        bTrackAppSettingChangedFlag = false;

        // check the register & callback it back
        final int N = mAvrcpCallbacksList.beginBroadcast();
        for (int i = 0; i < N; i++) {
            IBTAvrcpMusicCallback listener = mAvrcpCallbacksList.getBroadcastItem(i);
            try {
                listener.notifyAppSettingChanged();
            } catch (RemoteException ex) {
                // The RemoteCallbackList will take care of removing the
                // dead listeners.
                MusicLogUtils.e(TAG, "Error:" + ex);
            }
        }
        mAvrcpCallbacksList.finishBroadcast();
    }

    protected void notifyNowPlayingContentChanged() {
        MusicLogUtils.v(TAG, "[AVRCP] notifyNowPlayingContentChanged ");
        if (null == mAvrcpCallbacksList) {
            return;
        }
        bTrackNowPlayingChangedFlag = false;

        // check the register & callback it back
        final int N = mAvrcpCallbacksList.beginBroadcast();

        MusicLogUtils.d(TAG, "[AVRCP] notifyNowPlayingContentChanged " + " N= " + N);

        for (int i = 0; i < N; i++) {
            IBTAvrcpMusicCallback listener = mAvrcpCallbacksList.getBroadcastItem(i);
            try {
                listener.notifyNowPlayingContentChanged();
            } catch (RemoteException ex) {
                // The RemoteCallbackList will take care of removing the
                // dead listeners.
                MusicLogUtils.e(TAG, "Error:" + ex);
            }
        }
        mAvrcpCallbacksList.finishBroadcast();
    }

    protected void notifyVolumehanged(byte volume) {
        MusicLogUtils.v(TAG, "[AVRCP] notifyVolumehanged " + volume);
    }
}
