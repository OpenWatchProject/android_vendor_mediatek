package com.mediatek.socket.base;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.net.LocalSocketAddress.Namespace;

import com.mediatek.socket.base.SocketUtils.BaseBuffer;

import vendor.mediatek.hardware.lbs.V1_0.ILbs;
import vendor.mediatek.hardware.lbs.V1_0.ILbsCallback;
import android.os.RemoteException;
import android.os.HwBinder;
import java.util.ArrayList;

public class UdpClient {

    private BaseBuffer mBuff;
    private boolean mIsLocalSocket;

    // Network
    private String mHost;
    private int mPort;
    private InetAddress mInetAddress;
    private DatagramSocket mNetSocket;
    private DatagramPacket mPacket;

    // Local
    private String mChannelName;
    private Namespace mNamespace;
    private LocalSocket mLocalSocket;
    private DataOutputStream mOut;

    public UdpClient(String host, int port, int sendBuffSize) {
        mIsLocalSocket = false;
        mBuff = new BaseBuffer(sendBuffSize);
        mHost = host;
        mPort = port;
    }

    public UdpClient(String channelName, Namespace namesapce, int sendBuffSize) {
        mIsLocalSocket = true;
        mBuff = new BaseBuffer(sendBuffSize);
        mChannelName = channelName;
        mNamespace = namesapce;
    }

    //HIDL start
    ILbs mLbsHidlClient = null;

    LbsHidlDeathRecipient mLLbsHidlDeathRecipient = new LbsHidlDeathRecipient();
    class LbsHidlDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            mLbsHidlClient = null;
        }
    }

    public boolean lbsHidlInit(String name) {
        if(mLbsHidlClient != null) {
            //already have the HIDL client, do not get it again
            return true;
        }
        try {
            mLbsHidlClient = ILbs.getService(name);
            mLbsHidlClient.linkToDeath(mLLbsHidlDeathRecipient, 0);
        } catch (RemoteException | RuntimeException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public static ArrayList<Byte> convertByteArrayToArrayList(byte[] data, int size) {
        if (data == null) {
            return null;
        }
        int max = (size < data.length)? size : data.length;
        ArrayList<Byte> ret = new ArrayList<Byte>();
        for (int i = 0; i < max; i++) {
            ret.add(data[i]);
        }
        return ret;
    }
    //HIDL end

    public boolean connect() {
        if (mIsLocalSocket) {
            return lbsHidlInit(mChannelName);
        } else {
            try {
                mNetSocket = new DatagramSocket();
                if (mInetAddress == null) {
                    mInetAddress = InetAddress.getByName(mHost);
                }
                mPacket = new DatagramPacket(mBuff.getBuff(),
                        mBuff.getBuff().length, mInetAddress, mPort);
                return true;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    public BaseBuffer getBuff() {
        return mBuff;
    }

    public boolean write() {
        if (mIsLocalSocket) {
            try {
                ArrayList<Byte> data = convertByteArrayToArrayList(mBuff.getBuff(), mBuff.getOffset());
                mLbsHidlClient.sendToServer(data);
                mBuff.setOffset(0);
                return true;
            } catch (RemoteException | RuntimeException e) {
                e.printStackTrace();
            }
        } else {
            try {
                mPacket.setLength(mBuff.getOffset());
                mNetSocket.send(mPacket);
                mBuff.setOffset(0);
                return true;
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    public void close() {
        if (mIsLocalSocket) {
			// do nothing
        } else {
            if (mNetSocket != null) {
                mNetSocket.close();
            }
        }
    }

}
