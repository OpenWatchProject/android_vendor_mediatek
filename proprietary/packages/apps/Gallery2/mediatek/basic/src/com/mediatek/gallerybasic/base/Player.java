package com.mediatek.gallerybasic.base;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MTexture;
import com.mediatek.gallerybasic.util.Log;

import java.util.ArrayList;

public abstract class Player {
    private static final String TAG = "MtkGallery2/Player";

    private static final LooperThread sLooperThread;
    static {
        sLooperThread = new LooperThread("Player-LooperThread");
        sLooperThread.start();
    }

   static class LooperThread extends Thread {
        private Looper mLooper;
        public LooperThread(String name) {
            super(name);
        }
        public void run() {
            Log.d(TAG, "<LooperThread.run>");
            Looper.prepare();
            mLooper = Looper.myLooper();
            Looper.loop();
        }
        public Looper getLooper() {
            return mLooper;
        }
    }

    public enum State {
        PREPARED, PLAYING, RELEASED
    }

    public enum OutputType {
        TEXTURE, BITMAP
    }

    public interface TaskCanceller {
        public boolean isCancelled();
    }

    // message send to PlayListener
    public static final int MSG_NOTIFY = 0;
    // message send to OnFrameAvailableListener
    public static final int MSG_FRAME_AVAILABLE = 1;
    // message used to call onPlayFrame on main thread
    public static final int MSG_PLAY_FRAME = 2;

    protected OutputType mOutputType = OutputType.TEXTURE;
    protected ArrayList<PlayListener> mPlayListeners;
    protected OnFrameAvailableListener mFrameAvailableListener;
    protected Context mContext;
    protected MediaData mMediaData;
    protected EventHandler mMainThreadHandler;
    protected EventHandler mThreadHandler;

    private volatile State mState = State.RELEASED;
    protected TaskCanceller mTaskCanceller;

    public Player(Context context, MediaData data, OutputType outputType) {
        mContext = context;
        mMainThreadHandler = new EventHandler(this, Looper.getMainLooper());
        Looper looper = sLooperThread.getLooper();
        // if LooperThread run slowly, looper may be null, wait until looper != null
        while (looper == null) {
            Log.d(TAG, "<Player> looper is null, wait 5 ms");
            try {
                Thread.sleep(5);
            } catch (InterruptedException e) {
                Log.d(TAG, "<Player> Thread.sleep InterruptedException");
            }
            looper = sLooperThread.getLooper();
        }
        mThreadHandler = new EventHandler(this, looper);
        mMediaData = data;
        mPlayListeners = new ArrayList<PlayListener>();
        mOutputType = outputType;
    }

    public Context getContext() {
        return mContext;
    }

    public State getState() {
        return mState;
    }

    public interface OnFrameAvailableListener {
        public void onFrameAvailable(Player player);
    }

    public MTexture getTexture(MGLCanvas canvas) {
        return null;
    }

    public Bitmap getBitmap() {
        return null;
    }

    public static interface PlayListener {
        void onChange(Player player, int what, int arg, Object obj);
    }

    public void registerPlayListener(PlayListener listener) {
        mPlayListeners.add(listener);
    }

    public void unRegisterPlayListener(PlayListener listener) {
        mPlayListeners.remove(listener);
    }

    public void clearAllPlayListener() {
        mPlayListeners.clear();
    }

    public OutputType getOutputType() {
        return mOutputType;
    }

    // return width relative to rotation
    public int getOutputWidth() {
        return 0;
    }

    // return height relative to rotation
    public int getOutputHeight() {
        return 0;
    }

    public boolean isSkipAnimationWhenUpdateSize() {
        return false;
    }

    protected void sendNotify(int what, int arg, Object data) {
        Message msg = new Message();
        msg.what = MSG_NOTIFY;
        msg.arg1 = what;
        msg.arg2 = arg;
        msg.obj = data;
        mMainThreadHandler.sendMessage(msg);
    }

    protected void sendNotify(int what) {
        sendNotify(what, 0, null);
    }

    protected void sendFrameAvailable() {
        if (mFrameAvailableListener == null)
            return;
        mMainThreadHandler.removeMessages(MSG_FRAME_AVAILABLE);
        mMainThreadHandler.sendEmptyMessage(MSG_FRAME_AVAILABLE);
    }

    protected void sendPlayFrameDelayed(int delay) {
        // As onPlayFrame() can be called in other thread,
        // in order to decrease loading of main thread,
        // we do not post message to mMainThreadHander
        if (delay == 0) {
            mThreadHandler.sendEmptyMessage(MSG_PLAY_FRAME);
        } else {
            mThreadHandler.sendEmptyMessageDelayed(MSG_PLAY_FRAME, delay);
        }
    }

    protected void removeAllMessages() {
        mMainThreadHandler.removeMessages(MSG_PLAY_FRAME);
        mMainThreadHandler.removeMessages(MSG_FRAME_AVAILABLE);
        mMainThreadHandler.removeMessages(MSG_NOTIFY);
        mThreadHandler.removeMessages(MSG_PLAY_FRAME);
        mThreadHandler.removeMessages(MSG_FRAME_AVAILABLE);
        mThreadHandler.removeMessages(MSG_NOTIFY);
    }

    protected void onPlayFrame() {

    }

    protected class EventHandler extends Handler {
        private Player mPlayer;

        public EventHandler(Player player, Looper looper) {
            super(looper);
            mPlayer = player;
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_NOTIFY:
                for (PlayListener listener : mPlayListeners) {
                    listener.onChange(mPlayer, msg.arg1, msg.arg2, msg.obj);
                }
                return;
            case MSG_FRAME_AVAILABLE:
                if (mFrameAvailableListener != null) {
                    mFrameAvailableListener.onFrameAvailable(Player.this);
                }
                return;
            case MSG_PLAY_FRAME:
                onPlayFrame();
                return;
            default:
                throw new IllegalArgumentException("Invalid message.what = "
                        + msg.what);
            }
        }
    }

    public void setOnFrameAvailableListener(OnFrameAvailableListener lis) {
        mFrameAvailableListener = lis;
    }

    public boolean prepare() {
        boolean success = onPrepare();
        if (success)
            mState = State.PREPARED;
        return success;
    }

    public boolean start() {
        boolean success = onStart();
        if (success)
            mState = State.PLAYING;
        return success;
    }

    public boolean pause() {
        return onPause();
    }

    public boolean stop() {
        boolean success = onStop();
        if (success)
            mState = State.PREPARED;
        return success;
    }

    public void release() {
        onRelease();
        mState = State.RELEASED;
    }

    public void setBuffer(byte[] buffer) {
    }

    public void setTaskCanceller(TaskCanceller canceller) {
        mTaskCanceller = canceller;
    }

    protected abstract boolean onPrepare();

    protected abstract boolean onStart();

    protected abstract boolean onPause();

    protected abstract boolean onStop();

    protected abstract void onRelease();

    // When onPrepare/onStart/onPause/onStop is running,
    // and mTaskCanceller.isCancled() become true from false, onCancel will be called
    // This function will be called in main thread,
    // so please not do time-consuming things here
    public void onCancel() {
    }
}
