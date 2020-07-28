package com.debug.loggerui.framework;

import android.content.Intent;
import android.net.LocalServerSocket;
import android.net.LocalSocket;

import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public class AEEConnection {
    private static final String AEE_CLIENT = "1,";
    private static final String MODEM_LOG_CLIENT = "2,";

    private static final String TAG = Utils.TAG + "/AEEConnection";
    private static final int BUFFER_SIZE = 4096;

    private ServerSocketThread mServerSocketThread;
    private List<InteractClientSocketThread> mClientSocketThreadList
            = new ArrayList<InteractClientSocketThread>();

    private static AEEConnection sInstance;

    private AEEConnection() {
    }

    /**
     * @return AEEConnection
     */
    public synchronized static AEEConnection getInstance() {
        if (sInstance == null) {
            sInstance = new AEEConnection();
        }
        return sInstance;
    }

    /**
     * void return.
     */
    public void startSocketServer() {
        Utils.logd(TAG, "startSocketServer");
        if (mServerSocketThread == null || !mServerSocketThread.isAlive()) {
            Utils.logd(TAG, "SocketServer is not running, start it!");
            mServerSocketThread = new ServerSocketThread();
            mServerSocketThread.start();
        }
    }

    /**
     * void return.
     */
    public void stopSocketServer() {
        Utils.logd(TAG, "stopSocketServer");
        if (mServerSocketThread != null && !mServerSocketThread.isInterrupted()) {
            mServerSocketThread.stopRun();
            mServerSocketThread.interrupt();
        }
        mClientSocketThreadList.clear();
    }

    /**
     * @author MTK81255
     *
     */
    private class ServerSocketThread extends Thread {
        private LocalServerSocket mServerSocket;

        public void stopRun() {
            if (mServerSocket != null) {
                try {
                    mServerSocket.close();
                    Utils.logd(TAG, "serverSocket closed !");
                } catch (IOException e) {
                    e.printStackTrace();
                    Utils.loge(TAG, "There is some thing error happend when serverSocket closed!");
                }
            }
        }

        @Override
        public void run() {
            try {
                mServerSocket = new LocalServerSocket(Utils.ACTION_EXP_HAPPENED);
            } catch (IOException e) {
                e.printStackTrace();
                Utils.loge(TAG, "There is some thing error happend when new LocalServerSocket!");
                return;
            }

            while (!this.isInterrupted()) {
                Utils.logd(TAG, "wait for new client coming !");
                try {
                    LocalSocket interactClientSocket = mServerSocket.accept();
                    if (mClientSocketThreadList.size() >= 3) {
                        Utils.logd(TAG, "The max for client is 3!");
                        continue;
                    }
                    if (!this.isInterrupted()) {
                        Utils.logd(TAG, "new client coming !");
                        InteractClientSocketThread clientSocketThread
                            = new InteractClientSocketThread(interactClientSocket);
                        clientSocketThread.start();
                        mClientSocketThreadList.add(clientSocketThread);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    this.isInterrupted();
                }
            }

            Utils.logd(TAG, "ServerSocketThread exit !");

            if (mServerSocket != null) {
                try {
                    mServerSocket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                    Utils.loge(TAG, "There is some thing error happend when serverSocket closed!");
                }
            }
        }
    }

    /**
     * @author MTK81255
     *
     */
    private class InteractClientSocketThread extends Thread {
        private LocalSocket mInteractClientSocket;

        public InteractClientSocketThread(LocalSocket interactClientSocket) {
            this.mInteractClientSocket = interactClientSocket;
        }

        @Override
        public void run() {
            Utils.logd(TAG, "Monitor thread running");
            InputStream inputStream = null;
            try {
                inputStream = mInteractClientSocket.getInputStream();
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                char[] buf = new char[BUFFER_SIZE];
                int readBytes = -1;
                while ((readBytes = inputStreamReader.read(buf)) != -1) {
                    String tempPath = new String(buf, 0, readBytes);
                    Utils.logi(TAG, "Receive String Value from client is : " + tempPath);
                    if (tempPath.startsWith(MODEM_LOG_CLIENT)) {
                        // 2,com.mediatek.mdlogger.AUTOSTART_COMPLETE,modem_index=4
                        Utils.logd(TAG, "Client from Modem_log");
                        String[] modemCmd = tempPath.split(",");
                        if (modemCmd.length != 3) {
                            Utils.loge(TAG, "The command from mdlog is error : " + tempPath);
                            break;
                        }
                        if (!Utils.ACTION_MDLOGGER_RESTART_DONE.equals(modemCmd[1])) {
                            Utils.loge(TAG, "The resart done command from mdlog is error : "
                                    + tempPath);
                            break;
                        }
                        Intent intent = new Intent(Utils.ACTION_MDLOGGER_RESTART_DONE);
                        intent.setClassName("com.debug.loggerui",
                                "com.debug.loggerui.framework.LogReceiver");
                        String[] extra = modemCmd[2].split("=");
                        if (extra.length != 2) {
                            Utils.loge(TAG, "The extra from mdlog is error : " + modemCmd[2]);
                        } else {
                            intent.putExtra(extra[0], extra[1]);
                        }
                        Utils.sendBroadCast(intent);
                        Utils.logd(TAG, "Sent out broadcast : " + " am broadcast -a "
                                + Utils.ACTION_MDLOGGER_RESTART_DONE);
                    } else if (tempPath.startsWith(AEE_CLIENT)) {
                        Utils.logd(TAG, "Client from AEE");
                    } else {
                        if (tempPath.endsWith("/")) {
                            tempPath = tempPath.substring(0, tempPath.length() - 1);
                        }
                        String dbFolderPath = tempPath + "/";
                        String dbFileName =
                                tempPath.substring(tempPath.lastIndexOf(File.separator) + 1)
                                        + ".dbg";
                        String zzFileName = "ZZ_INTERNAL";

                        Intent intent = new Intent(Utils.ACTION_EXP_HAPPENED);
                        intent.putExtra(Utils.EXTRA_KEY_EXP_PATH, dbFolderPath);
                        intent.putExtra(Utils.EXTRA_KEY_EXP_NAME, dbFileName);
                        intent.putExtra(Utils.EXTRA_KEY_EXP_ZZ, zzFileName);
                        Utils.sendBroadCast(intent);
                        Utils.logd(TAG, "Sent out broadcast : " + " am broadcast -a "
                                + Utils.ACTION_EXP_HAPPENED + " -e " + Utils.EXTRA_KEY_EXP_PATH
                                + " " + dbFolderPath + " -e " + Utils.EXTRA_KEY_EXP_NAME + " "
                                + dbFileName + " -e " + Utils.EXTRA_KEY_EXP_ZZ + " " + zzFileName);
                    }
                    break;
                }
            } catch (IOException e) {
                e.printStackTrace();
                Utils.logd(TAG, "resolve data error !");
            } finally {
                if (inputStream != null) {
                    try {
                        inputStream.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                mClientSocketThreadList.remove(this);
            }
        }
    }
}