package com.log.handler.connection;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import com.log.handler.LogHandlerUtils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author MTK81255
 *
 */
public class LogSocketConnection extends AbstractLogConnection {
    private static final String TAG = LogHandlerUtils.TAG + "/LogSocketConnection";

    private LocalSocket mSocket;
    private OutputStream mOutputStream;
    private InputStream mInputStream;

    private SocketListenHandler mSocketListenHandler;
    public static final int MSG_SOCKET_START_LISTEN = 1;
    private SocketMessageHandler mSocketMessageHandler;
    public static final int MSG_SOCKET_READ = 1;
    /**
     * Get at most this number of byte from socket. Since modem log may return a log folder path
     * through socket, this response maybe a long string
     */
    private static final int BUFFER_SIZE = 1024;

    /**
     * @param serverName
     *            String
     */
    public LogSocketConnection(String serverName) {
        super(serverName);
        HandlerThread listenHandlerThread = new HandlerThread("Socket_Listen_Handler");
        listenHandlerThread.start();
        mSocketListenHandler = new SocketListenHandler(listenHandlerThread.getLooper());

        HandlerThread socketMessageHandlerThread = new HandlerThread("Socket_Message_Handler");
        socketMessageHandlerThread.start();
        mSocketMessageHandler = new SocketMessageHandler(socketMessageHandlerThread.getLooper());
    }

    @Override
    public boolean connect() {
        mSocket = new LocalSocket();
        try {
            LocalSocketAddress socketAddress =
                    new LocalSocketAddress(mServerName, LocalSocketAddress.Namespace.ABSTRACT);
            mSocket.connect(socketAddress);
            mOutputStream = mSocket.getOutputStream();
            mInputStream = mSocket.getInputStream();
            mSocketListenHandler.sendEmptyMessage(MSG_SOCKET_START_LISTEN);
        } catch (IOException ex) {
            LogHandlerUtils.logw(TAG,
                    "Exception happens when connect to socket server : " + mServerName);
            disConnect();
            return false;
        }
        return true;
    }

    private void listen() {
        int count;
        byte[] buffer = new byte[BUFFER_SIZE];
        LogHandlerUtils.logi(TAG, "Socket listen start");
        while (true) {
            try {
                count = mInputStream.read(buffer, 0, BUFFER_SIZE);
                if (count < 0) {
                    LogHandlerUtils.logw(TAG,
                            "Get a empty response from native layer," + " socket connection lost!");
                    break;
                }
                LogHandlerUtils.logd(TAG, "Response from native byte size = " + count);
                byte[] resp = new byte[count];
                System.arraycopy(buffer, 0, resp, 0, count);
                mSocketMessageHandler.obtainMessage(MSG_SOCKET_READ, new String(resp))
                        .sendToTarget();
            } catch (IOException ex) {
                LogHandlerUtils.loge(TAG, "read failed", ex);
                break;
            }
        }
        disConnect();
    }

    @Override
    public boolean isConnection() {
        return mSocket != null;
    }

    @Override
    protected boolean sendDataToServer(String data) {
        LogHandlerUtils.logd(TAG,
                "sendDataToServer() mServerName = " + mServerName + ", data = " + data);
        boolean sendSuccess = false;
        try {
            Thread.sleep(50);
            mOutputStream.write((data + "\0").getBytes());
            mOutputStream.flush();
            sendSuccess = true;
        } catch (IOException e) {
            LogHandlerUtils.loge(TAG, "IOException while sending command to native.", e);
            disConnect();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        LogHandlerUtils.logd(TAG, "sendToServer done! sendSuccess = " + sendSuccess);
        return sendSuccess;
    }

    @Override
    public synchronized void disConnect() {
        if (mSocket != null) {
            try {
                mSocket.shutdownInput();
                mSocket.shutdownOutput();
                mSocket.close();
            } catch (IOException e) {
                LogHandlerUtils.loge(TAG, "Exception happended while closing socket: " + e);
            }
        }
        mSocket = null;
        super.disConnect();
    }

    /**
     * @author MTK81255
     *
     */
    class SocketMessageHandler extends Handler {

        public SocketMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;
            Object obj = msg.obj;
            LogHandlerUtils.logd(TAG,
                    "SocketMessageHandler receive message," + " what = " + what + ",obj = " + obj);
            switch (what) {
            case MSG_SOCKET_READ:
                if (obj != null && obj instanceof String) {
                    setResponseFromServer((String) obj);
                }
                break;
            default:
                break;
            }
        }
    }

    /**
     * @author MTK81255
     *
     */
    class SocketListenHandler extends Handler {

        public SocketListenHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;
            LogHandlerUtils.logi(TAG,
                    "SocketListenHandler receive message," + " what = " + what);
            switch (what) {
            case MSG_SOCKET_START_LISTEN:
                listen();
                break;
            default:
                break;
            }
        }
    }

}
