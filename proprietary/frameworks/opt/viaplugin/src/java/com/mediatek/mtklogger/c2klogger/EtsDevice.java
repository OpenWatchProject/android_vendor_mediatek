package com.mediatek.mtklogger.c2klogger;

import android.system.OsConstants;
import android.util.Log;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.ConcurrentModificationException;
import java.util.List;

import libcore.io.IoBridge;

/**
 * @author MTK81255
 *
 */
public abstract class EtsDevice {

    private File mTtyDev = null;

    private FileDescriptor mFileDescriptor;
    private OutputStream mOutStream;
    private InputStream mInSteam;

    private ReadThread mReadThread = null;
    private EtsMsgQueue mMsgCache = new EtsMsgQueue();

    protected String mPathDev = null;

    protected boolean mWorking = false;
    protected boolean mStopLogrecord = true;

    // public static final int DEFAULT_FILE_SIZE =
    // C2KLogUtils.DEFAULT_CONIFG_PERLOGSIZE * 1024 * 1024;
    // public static final int DEFAULT_FILE_SIZE = 10 * 1024;
    protected int mPerLogSize = C2KLogUtils.DEFAULT_CONIFG_PERLOGSIZE * 1024 * 1024;
    public static final int TTY_DEV_CACHE_SIZE = 256 * 1024;

    private int mWriteTimeout = 0;

    /**
     * @author MTK81255
     *
     */
    public enum ErrorCode {
        DevUnvalid, Unknown
    }

    /**
     * @author MTK81255
     *
     */
    public enum CBPStatus {
        Boot, CP, Unknown
    }

    /**
     */
    public EtsDevice() {
        Log.i("via_ets", "version: 1.0.0");
    }

    List<EtsMsg> mMsgTotalList = new ArrayList<EtsMsg>();

    /**
     * @author MTK81255
     *
     */
    private class ReadThread extends Thread {

        @Override
        public void run() {
            super.run();
            Log.i("via_ets", "read thread start");
            byte[] bufRead = new byte[mPerLogSize]; // total readed data
            byte[] buf = new byte[TTY_DEV_CACHE_SIZE]; // readed data this time
            int sizeTotal = 0;
            int logOutputCaculate = 0;
            while (!mStopLogrecord) {
                try {
                    if (mInSteam == null) {
                        Log.w("via_ets", "in_steam is null");
                        break;
                    }

                    int size = mInSteam.read(buf);
                    logOutputCaculate++;
                    if (size > 0) {
                        System.arraycopy(buf, 0, bufRead, sizeTotal, size);
                        sizeTotal += size;
//                        Log.i("via_ets", "receive: " + size + " bytes");
                        if (size == (TTY_DEV_CACHE_SIZE - 1)) {
                            Log.w("via_ets", "receive: " + size + " bytes");
                        } else if (logOutputCaculate > 100) {
                            Log.v("via_ets", "receive " + logOutputCaculate
                                    + " sizeTotal = " + sizeTotal + " bytes");
                            logOutputCaculate = 0;
                        } else if (logOutputCaculate == 1) {
                            Log.v("via_ets", "receive " + logOutputCaculate
                                    + " sizeTotal = " + sizeTotal + " bytes");
                        }

                        if (sizeTotal > mPerLogSize - TTY_DEV_CACHE_SIZE
                                || mStopLogrecord) {
                            byte[] bufWrite = new byte[mPerLogSize];
                            System.arraycopy(bufRead, 0, bufWrite, 0, sizeTotal);
                            writeToFile(bufWrite, sizeTotal);
                            sizeTotal = 0;
                            logOutputCaculate = 0;
                            try {
                                bufRead = new byte[mPerLogSize];
                            } catch (OutOfMemoryError e) {
                                Log.e("via_ets", "Memory free bytes < " + mPerLogSize);
                                return;
                            }
                            if (mStopLogrecord) {
                                break;
                            }
                        }
                    } else {
                        Log.w("via_ets", "read " + size + " byte");

                        // you should break it?
                        if (!mWorking) {
                            break;
                        }

                        if (size < 0) {
                            if (!onError(ErrorCode.DevUnvalid)) {
                                break;
                            }
                        }
                    }

                } catch (IOException e) {
                    Log.e("via_ets", "Something error happend for IO read!");
//                    writeToFile(bufRead, sizeTotal);
                    e.printStackTrace();
                    if (!onError(ErrorCode.Unknown)) {
                        break;
                    }
                }
            }
            Log.i("via_ets", "read thread exit! _working ? " + mStopLogrecord);
            writeToFile(bufRead, sizeTotal);

        }
    }

    private void writeToFile(final byte[] buffer, final int sizeTotal) {
        onDataReceived(buffer, sizeTotal);
    }

    /**
     * @param pathDev String
     * @return Boolean
     */
    protected Boolean create(String pathDev) {
        Log.i("via_ets", "create device on " + pathDev);
        try {
            openDevice(pathDev);
        } catch (SecurityException e) {
            e.printStackTrace();
            mTtyDev = null;
            return false;
        } catch (InvalidParameterException e) {
            e.printStackTrace();
            mTtyDev = null;
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            mTtyDev = null;
            return false;
        }

        mWorking = true;
        return true;
    }

    /**
     *
     */
    public void startReadThread() {
        mReadThread = new ReadThread();
        mReadThread.start();
    }

    protected void destroy() {
        Log.i("via_ets", "destroy device");

        mWorking = false;
        closeDevice();
    }

    /**
     * @param pathDev String
     * @throws SecurityException SecurityException
     * @throws IOException IOException
     * @throws InvalidParameterException InvalidParameterException
     */
    protected void openDevice(String pathDev) throws SecurityException,
            IOException, InvalidParameterException {
        if (mTtyDev != null) {
            Log.w("via_ets", "device already opened");
            return;
        }

        if (pathDev == null) {
            pathDev = mPathDev;
        }

        if (pathDev == null) {
            pathDev = C2KLogUtils.DEFAULT_CONIFG_DEVICEPATH;
            mPathDev = C2KLogUtils.DEFAULT_CONIFG_DEVICEPATH;
        }
        /* Open the port */
        mTtyDev = new File(pathDev);
        mFileDescriptor = IoBridge.open(mTtyDev.getAbsolutePath(),
                OsConstants.O_RDWR | OsConstants.O_APPEND);
        mOutStream = new FileOutputStream(mFileDescriptor);
        mInSteam = new FileInputStream(mFileDescriptor);

        mPathDev = pathDev;
    }

    protected void closeDevice() {
        Log.i("via_ets", "closeDevice()");
        if (!isOpened()) {
            return;
        }

        try {
            if (mOutStream != null) {
                mOutStream.close();
                mOutStream = null;
            }
            if (mInSteam != null) {
                mInSteam.close();
                mInSteam = null;
            }
            if (mFileDescriptor != null) {
                IoBridge.closeAndSignalBlockedThreads(mFileDescriptor);
//                IoBridge.closeSocket(mFileDescriptor);
                mFileDescriptor = null;
            }
            Log.i("via_ets", "in_steam & out_stream closed");
        } catch (IOException e) {
            e.printStackTrace();
        }

        mTtyDev = null;
        Log.i("via_ets", "closeDevice() done");
    }

    protected Boolean isOpened() {
        return mTtyDev != null;

    }

    private void onDataReceived(byte[] buffer, int sizeTotal) {
        int newBufferSize = EtsMsg.removeErrorBuffer(buffer, sizeTotal);
        onEtsMsgReceived(buffer, newBufferSize);
    }

    /*
     * private void onDataReceived(byte[] buffer, int totalSize){ List<EtsMsg>
     * msgs = EtsMsg.parse(buffer, totalSize); onEtsMsgReceived(msgs); }
     */

    protected void onEtsMsgReceived(EtsMsg msg) {
        mMsgCache.offer(msg);
        // Log.v("via_ets", "Offer a msg to cache");
    }

    protected void onEtsMsgReceived(byte[] buffer, int size) {
    }

    protected void onEtsMsgReceived(List<EtsMsg> msgs) {
    }

    protected boolean onError(ErrorCode code) {
        Log.e("via_ets", "Error:" + code.toString());
        return false;
    }

    /**
     * @param msg EtsMsg
     * @throws NullPointerException e
     * @throws ConcurrentModificationException e
     */
    public void write(EtsMsg msg) throws NullPointerException, ConcurrentModificationException {
        try {
            byte[] buf = msg.getBuf();
            if (buf != null) {
                mOutStream.write(buf);
            }
        } catch (IOException e) {
            mWriteTimeout ++;
            if (mWriteTimeout > 5) {
                Log.e("via_ets", "Send a msg, id = " + msg.getId() + " is failed!"
                        + " Throw ConcurrentModificationException to stop write!");
                mWriteTimeout = 0;
                throw new ConcurrentModificationException();
            }
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e1) {
                e1.printStackTrace();
            }
            write(msg);
            return;
        }
        mWriteTimeout = 0;
    }

    /**
     * @param data1 byte[]
     * @param data2 byte[]
     * @return boolean
     */
    public boolean cmpBytes(byte[] data1, byte[] data2) {
        if (data1.length != data2.length) {
            return false;
        }

        for (int i = 0; i < data1.length; ++i) {
            if (data1[i] != data2[i]) {
                return false;
            }
        }

        return true;
    }

    /**
     * @param id short
     * @param timeout long
     * @return EtsMsg
     */
    public EtsMsg waitForMsg(short id, long timeout) {
        return mMsgCache.waitForMsg(id, timeout);
    }

    protected EtsMsg sendAndWait(EtsMsg msgReq, short id, long timeout) {
        write(msgReq);
        return waitForMsg(id, timeout);
    }

    /**
     * feature loopback.
     * @return boolean
     */
    public boolean loopback() {
        Log.v("via_ets", "do loopback");

        short id = 0x0000;
        byte[] data = new byte[] { (byte) (System.currentTimeMillis() & 0xFF) };

        EtsMsg msg = sendAndWait(new EtsMsg(id, data), id, 2000);
        if (msg == null) {
            Log.e("via_ets", "failed, no response");
            return false;
        }

        Log.i("via_ets", "success");
        return true;
    }

    private CBPStatus getCbpStatus() {
        CBPStatus ret = CBPStatus.Unknown;

        short id = 0x0120;
        EtsMsg msg = sendAndWait(new EtsMsg(id, null), id, 1000);
        if (msg != null) {
            byte[] data = msg.getData();
            if (data[0] == 0) {
                ret = CBPStatus.Boot;
            } else {
                ret = CBPStatus.CP;
            }
        }

        Log.v("via_ets", ret.name() + " mode");
        return ret;
    }

    /**
     * @param timeoutS long
     * @return boolean
     */
    public boolean waitForCP(long timeoutS) {

        for (int i = 0; i < timeoutS; ++i) {
            if (getCbpStatus() == CBPStatus.CP) {
                return true;
            }

            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        return false;
    }

}
