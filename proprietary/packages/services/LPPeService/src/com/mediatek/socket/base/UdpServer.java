package com.mediatek.socket.base;

import java.io.DataInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.net.LocalSocketAddress.Namespace;

import com.mediatek.socket.base.SocketUtils.BaseBuffer;
import com.mediatek.socket.base.SocketUtils.UdpServerInterface;

import com.mediatek.socket.base.SocketUtils.ProtocolHandler;
import vendor.mediatek.hardware.lbs.V1_0.ILbs;
import vendor.mediatek.hardware.lbs.V1_0.ILbsCallback;
import android.os.RemoteException;
import android.os.HwBinder;
import java.util.ArrayList;

public class UdpServer implements UdpServerInterface {

    private BaseBuffer mBuff;
    private boolean mIsLocalSocket;

    // Network
    private int mPort;
    private DatagramSocket mNetSocket;
    private DatagramPacket mPacket;

    // Local
    private LocalSocket mLocalSocket;
    private DataInputStream mIn;
    private String mChannelName;
    private Namespace mNamespace;

    //TODO HIDL start
    ILbs mLbsHidlClient;

    public static void covertArrayListToByteArray(ArrayList<Byte> in, byte[] out) {
        for(int i = 0; i < in.size() && i < out.length; i++) {
            out[i] = in.get(i);
        }
    }

    private LbsHidlCallback mLbsHidlCallback = new LbsHidlCallback();
    class LbsHidlCallback extends ILbsCallback.Stub {
        public boolean callbackToClient(ArrayList<Byte> data) {
            if(mReceiver != null) {
                //TODO set data to server
                covertArrayListToByteArray(data, mBuff.getBuff());
                mReceiver.decode(UdpServer.this);
            }
            return true;
        }
    };

    private LbsHidlDeathRecipient mLLbsHidlDeathRecipient = new LbsHidlDeathRecipient();
    class LbsHidlDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            mLbsHidlClient = null;
            boolean ret = false;
            while(!ret) {
                msleep(300);
                ret = lbsHidlInit(mChannelName);
            }
        }
    }
    
    private boolean lbsHidlInit(String name) {
        try {
            mLbsHidlClient = ILbs.getService(name);
            mLbsHidlClient.linkToDeath(mLLbsHidlDeathRecipient, 0);
            // cannot just new LbsHidlCallback() or it will cause runtime exception
            mLbsHidlClient.setCallback(mLbsHidlCallback);
        } catch (RemoteException | RuntimeException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    ProtocolHandler mReceiver;

    public void setReceiver(ProtocolHandler receiver) {
        mReceiver = receiver;
    }
    //TODO HIDL end

    public UdpServer(int port, int recvBuffSize) {
        mIsLocalSocket = false;
        mBuff = new BaseBuffer(recvBuffSize);
        mPort = port;
    }

    public UdpServer(String channelName, Namespace namespace, int recvBuffSize) {
        mIsLocalSocket = true;
        mBuff = new BaseBuffer(recvBuffSize);
        mChannelName = channelName;
        mNamespace = namespace;
        lbsHidlInit(channelName);
    }

    public boolean bind() {
        for (int i = 0; i < 5; i++) {
            if (mIsLocalSocket) {
                try {
                    mLocalSocket = new LocalSocket(LocalSocket.SOCKET_DGRAM);
                    mLocalSocket.bind(new LocalSocketAddress(mChannelName,
                            mNamespace));
                    mIn = new DataInputStream(mLocalSocket.getInputStream());
                    return true;
                } catch (IOException e) {
                    if (i == 4) {
                        throw new RuntimeException(e);
                    }
                    msleep(200);
                }
            } else {
                try {
                    mNetSocket = new DatagramSocket(mPort);
                    mPacket = new DatagramPacket(mBuff.getBuff(),
                            mBuff.getBuff().length);
                    return true;
                } catch (SocketException e) {
                    msleep(200);
                    if (i == 4) {
                        throw new RuntimeException(e);
                    }
                }
            }
        }
        return false;
    }

    @Override
    public boolean read() {
        mBuff.clear();
        if (mIsLocalSocket) {
            return true;
        } else {
            try {
                mNetSocket.receive(mPacket);
                return true;
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    @Override
    public BaseBuffer getBuff() {
        return mBuff;
    }

    public void close() {
        if (mIsLocalSocket) {
        } else {
            mNetSocket.close();
        }
    }

    public int available() {
        if (mIsLocalSocket) {
            try {
                return mIn.available();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            try {
                throw new RuntimeException(
                        "Network Type does not support available() API");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return -1;
    }

    public boolean setSoTimeout(int timeout) {
        if (mIsLocalSocket) {
            try {
                mLocalSocket.setSoTimeout(timeout);
                return true;
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            try {
                mNetSocket.setSoTimeout(timeout);
                return true;
            } catch (SocketException e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    private void msleep(long milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

}
