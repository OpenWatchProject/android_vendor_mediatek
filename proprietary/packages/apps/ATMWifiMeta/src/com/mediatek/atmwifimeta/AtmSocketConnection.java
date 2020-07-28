package com.mediatek.atmwifimeta;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.util.Log;

/**
 * Class which will communicate with native layer. Send command to native by
 * local socket, then monitor response code, and feed that back to user
 */
public class AtmSocketConnection {
    private static final String TAG = "atmwifimeta/AtmSocketConnection";

    private Thread mListenThread = null;

    LocalSocket mSocket;
    LocalSocketAddress mAddress;

    private OutputStream mOutputStream;
    private InputStream mInputStream;
    private static final int BUFFER_SIZE = 32;

    private String mRecvCommand = null;

    /**
     * Flag for whether local monitor thread should keep running.
     */
    private boolean mShouldStop = false;

    /**
     * Constructor with default socket name space.
     *
     * @param sockname
     *            String
     */

    public AtmSocketConnection(String sockname) {
        mSocket = new LocalSocket();
        mAddress = new LocalSocketAddress(sockname, LocalSocketAddress.Namespace.ABSTRACT);
    }

    /**
     * @return boolean
     */
    public boolean connect() {
        Log.d(TAG, "[Connect] Socket name=" + mAddress.getName());
        if (mSocket == null) {
            Log.w(TAG, "[Connect] mSocket = null, just return.");
            return false;
        }
        try {
            mSocket.connect(mAddress);
            mOutputStream = mSocket.getOutputStream();
            mInputStream = mSocket.getInputStream();
        } catch (IOException ex) {
            Log.w(TAG, "[Connect] Communications error,"
                    + " Exception happens when connect to socket server" + ex.toString());
            stop();
            return false;
        }
        mListenThread = new Thread() {
            public void run() {
                listen();
            }
        };
        mListenThread.start();

        Log.d(TAG, "[Connect] Connect to native socket OK. And start local monitor thread now");
        return true;
    }

    /**
     * @return boolean
     */
    public boolean isConnected() {
        boolean isConnectedNow = (mSocket != null && mSocket.isConnected());
        return isConnectedNow;
    }

    /**
     * @param cmd String
     * @return boolean
     */
    public boolean sendCmd(String cmd) {
        Log.d(TAG, "[Send] Send command: (" + cmd + ") to (" + mAddress.getName() + ")");
        boolean success = false;
        synchronized (this) {
            if (mOutputStream == null) {
                Log.e(TAG, "[Send] No connection, outputstream is null.");
                stop();
            } else {
                StringBuilder builder = new StringBuilder(cmd);
                builder.append('\0');
                try {
                    mOutputStream.write(builder.toString().getBytes());
                    mOutputStream.flush();
                    success = true;
                } catch (IOException ex) {
                    Log.e(TAG, "[Send] IOException while sending command to native.", ex);
                    mOutputStream = null;
                    stop();
                }
            }
        }
        Log.i(TAG, "[Send] Send command : (" + cmd + ") done");
        return success;
    }

    /**
     * void return.
     */
    public void listen() {
        int count;
        byte[] buffer = new byte[BUFFER_SIZE];

        Log.d(TAG, "[Listen] Start to listen socket command");
        while (!mShouldStop/* true */) {
            try {
                count = mInputStream.read(buffer, 0, BUFFER_SIZE);
                if (count < 0) {
                    Log.e(TAG, "[Listen] Got a empty response, stop listening.");
                    break;
                }
                byte[] resp = new byte[count];
                System.arraycopy(buffer, 0, resp, 0, count);
                mRecvCommand = new String(resp);
                Log.v(TAG, "[Listen] Got command (" + mRecvCommand +"), size = " + count);
            } catch (IOException ex) {
                Log.e(TAG, "[Listen] Read failed : ", ex);
                break;
            }
        }
        return;
    }

    /**
     * return void.
     */
    public void stop() {
        Log.i(TAG, "[Stop] Enter stop flow");
        mShouldStop = true;

        if (mSocket == null) {
            return;
        }

        try {
            mSocket.shutdownInput();
            mSocket.shutdownOutput();
            mSocket.close();
        } catch (IOException e) {
            Log.e(TAG, "[Stop] Exception happended while closing socket: " + e);
        }
        mListenThread = null;
        mSocket = null;
        mRecvCommand = null;
    }

    public LocalSocket getSocket() {
        return this.mSocket;
    }

    public String getRecvCommand() {
        return mRecvCommand;
    }
}
