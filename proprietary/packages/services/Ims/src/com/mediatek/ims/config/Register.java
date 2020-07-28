package com.mediatek.ims.config;

import android.content.ContentUris;
import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;

import java.util.HashSet;
import java.util.List;

/**
 * Base class to register Ims related configuration listener.
 */
public abstract class Register {
    protected String argType = null;
    protected HashSet<Integer> mRegArgs = new HashSet<Integer>();
    protected Context mContext = null;
    protected int mPhoneId;
    protected IArgsChangeListener mArgsListener = null;
    protected ArgsObserver mArgsObserver = null;
    private Handler mHandler = null;
    private HandlerThread mHandlerThread = null;

    private Register() {};

    public Register(Context context, int phoneId) {
        this(context, phoneId, null);
    }

    public Register(Context context, int phoneId, Handler handler) {
        mContext = context;
        mPhoneId = phoneId;
        if (context == null) {
            throw new IllegalArgumentException("Null context!");
        }
        // Caller's main thread is preferred, otherwise,start a new thread to handle it.
        if (handler != null) {
            mHandler = handler;
        } else {
            mHandlerThread = new HandlerThread("Ims" + argType + "Reg");
            mHandlerThread.start();
            mHandler = new Handler(mHandlerThread.getLooper());
        }
    }

    public Register addArg(int argId) throws IllegalArgumentException {
        mRegArgs.add(argId);
        return this;
    }

    public void register(IArgsChangeListener listener) throws IllegalArgumentException {
        mArgsListener = listener;
        if (mRegArgs.isEmpty()) {
            throw new IllegalArgumentException("Empty " + argType + " for register");
        }
        if (listener == null) {
            throw new IllegalArgumentException("Null listener for " + argType + " register");
        }
        mArgsObserver = new ArgsObserver(mArgsListener);
    }

    public void unregister() {
        if (mArgsObserver != null) {
            mContext.getContentResolver().unregisterContentObserver(mArgsObserver);
        }
    }

    public interface IArgsChangeListener {
        void onChange(int argId);
    }

    public class ArgsObserver extends ContentObserver {
        private IArgsChangeListener mArgsObserver = null;
        public ArgsObserver(IArgsChangeListener listener) {
            super(mHandler);
            mArgsObserver = listener;
        }
        @Override
        public void onChange(boolean selfChange) {
            onChange(selfChange, null);
        }
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            int argId = -1;
            if (uri == null) {
                return;
            }

            switch (argType) {
                case "config":
                    String configName = uri.getLastPathSegment();
                    argId = ImsConfigContract.configNameToId(configName);
                    mArgsObserver.onChange(argId);
                    break;
                case "feature":
                    // content://com.mediatek.ims.config.provider/tb_feature/$phoneId/$featureId/$network
                    //                                                 0        1          2        3
                    List<String> list = uri.getPathSegments();
                    argId = Integer.parseInt(list.get(2));
                    mArgsObserver.onChange(argId);
                    break;
                default:
                    throw new RuntimeException("Invalid Register class: " + argType);
            }

        }
    }
}
