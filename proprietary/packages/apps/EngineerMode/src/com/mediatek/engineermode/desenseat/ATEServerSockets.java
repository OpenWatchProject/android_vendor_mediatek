package com.mediatek.engineermode.desenseat;

import com.mediatek.engineermode.Elog;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;


/**
 * Client socket, used to connect with GPS server and communicate with PMTK command.
 *
 * @author mtk54046
 * @version 1.0
 */
public class ATEServerSockets {

    public static final String PMTK_CMD_CONNECTION_DONE = "PMTK2000";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_ITEMS = "PMTK2001";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_GNSS_BAND = "PMTK2002";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_MODE = "PMTK2003";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_START_API_TEST = "PMTK2004";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_API = "PMTK2005";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_RF_BAND_CONFIG = "PMTK2006";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_ON_OFF = "PMTK2007";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_SET_MODE = "PMTK2008";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_API_TEST_ON = "PMTK2009";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_RESULT = "PMTK2010";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_TRANSMIT_TEXT = "PMTK2011";
    public static final String PMTK_CMD_DESENSE_AUTO_TEST_QUERY_RF_CONFIG = "PMTK2012";
    public static final String PMTK_CMD_CLOCK_QUALITY_AUTO_TEST_ITEMS = "PMTK2020";
    public static final String PMTK_CMD_CLOCK_QUALITY_AUTO_TEST_CONFIG = "PMTK2021";
    public static final String PMTK_CMD_CLOCK_QUALITY_AUTO_TEST_ON_OFF = "PMTK2022";
    public static final String PMTK_CMD_CLOCK_QUALITY_AUTO_TEST_RESULT = "PMTK2023";
    public static final String PMTK_CMD_CLOCK_QUALITY_AUTO_TEST_TRANSMIT_TEXT = "PMTK2024";
    public static final String PMTK_ACK = "PMTK2030";
    public static final String PMTK_HEADER = "$PMTK";
    private static final int MASK_8_BIT = 0xFF;
    private static final int BUFFER_SIZE = 2048;
    public static boolean mAliveServer = false;
    public static boolean mAliveClient = false;
    private static ServerSocket mServerSocket = null;
    private final String TAG = "ATEServer";
    private final int SERVER_PORT = 9000;
    public Thread mExecuteThread = null;
    private BufferedReader mDataInputBuffered = null;
    private DataOutputStream mDataOutput = null;
    private String mCommand = null;
    private String mResponse = null;
    private BlockingQueue<String> mCommandQueue = null;
    private OnResponseListener mCallBack = null;
    private Thread mReceiveThread = null;
    private byte[] mDataBuffer = null;

    public ATEServerSockets(OnResponseListener callBack) {
        this.mCallBack = callBack;
        Elog.v(TAG, "ATEServerSockets --> init");

        if (mCommandQueue == null) {
            mCommandQueue = new LinkedBlockingQueue<String>();
        }
        if (mDataBuffer == null) {
            mDataBuffer = new byte[BUFFER_SIZE];
        }

        openServer();
        executeCmd();
    }

    private void executeCmd() {
        Elog.v(TAG, "ATEServerSockets --> enter executeCmd");
        mAliveServer = true;
        mExecuteThread = new Thread(new Runnable() {
            public void run() {
                while (mAliveServer) {
                    try {
                        mCommand = mCommandQueue.take();
                        Elog.v(TAG, "ATEServerSockets --> Queue take command:" + mCommand);
                    } catch (InterruptedException ie) {
                        Elog.w(TAG, "ATEServerSockets --> Take command interrupted:" + ie
                                .getMessage());
                        return;
                    }
                    if (ATEServerSockets.this.mCallBack != null) {
                        try {
                            ATEServerSockets.this.mCallBack.onResponse(trim(mCommand));
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    } else {
                        Elog.v(TAG, "ATEServerSockets --> mCallBack is null");
                    }
                    mCommand = null;
                }
            }
        });
        mExecuteThread.start();
    }

    public void openServer() {
        Elog.v(TAG, "ATEServerSockets --> enter startServer");
        mAliveServer = true;
        if (null != mServerSocket) {
            Elog.d(TAG, "ATEServerSockets --> Server Socket has started, return");
            return;
        }
        try {
            mServerSocket = new ServerSocket(SERVER_PORT);
        } catch (Exception e) {
            Elog.e(TAG, "ATEServerSockets --> mServerSocket new failed : " + e.getMessage());
        }

        mReceiveThread = new Thread(new Runnable() {
            Socket socket = null;

            public void run() {
                while (mAliveServer) {
                    Elog.d(TAG, "ATEServerSockets --> listening ...");
                    socket = null;
                    try {
                        socket = mServerSocket.accept();
                        socket.setSoTimeout(0);
                    } catch (Exception e) {
                        e.printStackTrace();
                        Elog.d(TAG, "ATEServerSockets --> accepted failed = " + e.getMessage());
                        return;
                    }
                    mAliveClient = true;
                    Elog.d(TAG, "ATEServerSockets --> accepted");
                    try {
                        mDataOutput = new DataOutputStream(socket.getOutputStream());
                        mDataInputBuffered = new BufferedReader(new InputStreamReader(socket
                                .getInputStream()));
                    } catch (IOException e) {
                        e.printStackTrace();
                        Elog.d(TAG, "ATEServerSockets --> IO failed" + e.getMessage());
                    }
                    while (mAliveClient) {
                        Elog.d(TAG, "ATEServerSockets --> waiting for cmd...");
                        String receiveCMD = readCMDFromSocket(mDataInputBuffered);
                        Elog.v(TAG, "ATEServerSockets --> received command: " + receiveCMD);

                        if (!socket.isConnected() || receiveCMD == null) {
                            Elog.w(TAG, "ATEServerSockets --> Disconnected from client");
                            mAliveClient = false;
                            break;
                        }
                        if (receiveCMD != null) {
                            sendAck(receiveCMD);
                            if (mCommandQueue.contains(receiveCMD)) {
                                Elog.v(TAG, "ATEServerSockets --> received return because hasn't " +
                                        "handle the same cmd");
                                return;
                            }
                            try {
                                mCommandQueue.put(receiveCMD);
                            } catch (InterruptedException ie) {
                                Elog.w(TAG, "ATEServerSockets --> send command interrupted:" + ie
                                        .getMessage());
                            }
                        }
                    }
                }
            }
        });
        mReceiveThread.start();
    }

    boolean checkSupportMsgID(int msgID) {
        return true;
    }

    void sendAck(String receiveCMD) {
        int type = 3;
        int cmdID = 0;
        if (receiveCMD.startsWith(PMTK_HEADER)) {
            String strTemp = receiveCMD.substring(PMTK_HEADER.length());
            int endIndex = strTemp.indexOf("*");
            String result = strTemp.substring(0, endIndex);
            try {
                cmdID = Integer.valueOf(result.split(",")[0]);
            } catch (Exception e) {
                mResponse = e.getMessage();
                Elog.w(TAG, "ATEServerSockets --> sendAck:" + e.getMessage());
            }
        }
        Elog.v(TAG, "ATEServerSockets --> cmdID = " + cmdID);
        if (checkSupportMsgID(cmdID)) {
            type = 3;
        } else {
            type = 1;
        }

        String sendCmd = PMTK_ACK + "," + cmdID +"," + type;
        Elog.v(TAG, "ATEServerSockets --> send ACK: " + sendCmd);
        sendCommand(sendCmd);
    }

    public String readCMDFromSocket(BufferedReader in) {
        String result = null;
        int count = -1;
        try {
            if (in != null) {
                result = in.readLine();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return result;
    }

    /**
     * Stop client socket and disconnect with server.
     */

    public void closeServer() {
        Elog.v(TAG, "ATEServerSockets -->enter closeClient");
        try {
            if (null != mDataInputBuffered) {
                mDataInputBuffered.close();
            }
            if (null != mDataOutput) {
                mDataOutput.close();
            }
            if (null != mServerSocket) {
                mServerSocket.close();
            }
        } catch (IOException e) {
            Elog.w(TAG, "ATEServerSockets -->closeClient IOException: " + e.getMessage());
        } finally {
            mServerSocket = null;
            mDataInputBuffered = null;
            mDataOutput = null;
            mAliveServer = false;
            mAliveClient = false;
        }
    }


    public void sendCommand(String command) {
        String sendComm = "$" + command + "*" + calcCS(command) + "\r" + "\n";
        Elog.v(TAG, "ATEServerSockets -->Send command: " + sendComm);
        if (mDataOutput != null && mAliveServer) {
            try {
                mDataOutput.writeBytes(sendComm);
                mDataOutput.flush();
            } catch (SocketException e) {
                Elog.w(TAG, "ATEServerSockets -->write data SocketException: " + e.getMessage());
            } catch (IOException e) {
                Elog.w(TAG, "ATEServerSockets -->write data IOException: " + e.getMessage());
                e.printStackTrace();
            }
        } else {
            Elog.d(TAG, "ATEServerSockets -->mDataOutput is null");
        }
    }

    /**
     * Calculate check sum for PMTK command.
     *
     * @param command The command need to send
     * @return The check sum string
     */
    private String calcCS(String command) {
        if (null == command || "".equals(command)) {
            return "";
        }
        byte[] ba = command.getBytes();
        int temp = 0;
        for (byte b : ba) {
            temp ^= b;
        }
        return String.format("%1$02x", temp & MASK_8_BIT).toUpperCase();
    }

    private String trim(String response) {
        String result = response.replaceAll(" ", "");
        return result;
    }

    /**
     * Interface to hand data from socket.
     */
    public interface OnResponseListener {
        /**
         * @param response data from socket
         */
        void onResponse(String response) throws InterruptedException;
    }
}
