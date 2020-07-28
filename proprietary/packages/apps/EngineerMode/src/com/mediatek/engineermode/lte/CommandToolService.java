package com.mediatek.engineermode.lte;

import android.app.Service;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

/*
 * AT command tool should be able to run commands at background.
 */
public class CommandToolService extends Service {
    private static final String TAG = "CommandToolService";
    private static final int MSG_SEND_NEXT_COMMAND = 1;
    private static final int MSG_AT_COMMAND = 2;

    private List<String> mCommands = new ArrayList<String>();
    private int mInterval = 1;
    private String mOutput = new String();
    private boolean mSending = false;

    private OnUpdateResultListener mOnUpdateResultListener;
    private final CommandToolServiceBinder mBinder = new CommandToolServiceBinder();

    private final Handler mHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Elog.d(TAG, "handleMessage() " + msg.what);
            switch (msg.what) {
            case MSG_SEND_NEXT_COMMAND:
                if (!mSending) {
                    return;
                }
                if (mCommands.size() > 0) {
                    sendAtCommand(mCommands.remove(0), MSG_AT_COMMAND);
                    mHander.sendEmptyMessageDelayed(MSG_SEND_NEXT_COMMAND, mInterval * 1000);
                } else {
                    mSending = false;
                    updateResult("Finished\n");
                }
                break;
            default:
                break;
            }
        }
    };

    private final Handler mAtCmdHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Elog.d(TAG, "handleMessage() " + msg.what);
            switch (msg.what) {
            case MSG_AT_COMMAND:
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Object result = ar.result;
                    if (result != null ) {
                        if (result instanceof String[]) {
                            String[] data = (String[]) result;
                            Elog.d(TAG, "Result(String): " + data.toString());
                            if (data.length > 0) {
                                updateResult("Return: ");
                                for (int i = 0; i < data.length; i++) {
                                    updateResult(data[i] + "\n");
                                }
                            }
                        } else if(result instanceof byte[]) {
                            byte[] rawData = (byte[]) ar.result;
                            try {
                                String txt = new String(rawData, "UTF-8");
                                Elog.d(TAG, "Result(byte): " + txt);
                                updateResult("Return: ");
                                updateResult(txt + "\n");
                            }  catch (NullPointerException e) {
                                updateResult("ERROR! \n");
                                e.printStackTrace();
                            } catch (UnsupportedEncodingException ee) {
                                updateResult("ERROR! \n");
                                ee.printStackTrace();
                            }
                        }
                    }
                } else {
                    Elog.e(TAG, "Exception: " + ar.exception);
                    updateResult("Exception: " + ar.exception + "\n");
                }
                break;
            default:
                break;
            }
        }
    };

    public interface OnUpdateResultListener {
        void onUpdateResult();
    }

    public class CommandToolServiceBinder extends Binder {
        CommandToolService getService() {
            return CommandToolService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Elog.v(TAG, "Enter onStartCommand");
        return START_NOT_STICKY;
    }

    public void startTest(List<String> commands, int interval) {
        Elog.v(TAG, "startTest");
        mCommands = commands;
        mInterval = interval;
        mOutput = "";
        mSending = true;
        mHander.sendEmptyMessage(MSG_SEND_NEXT_COMMAND);
    }

    public void stopTest() {
        Elog.v(TAG, "stopTest");
        if (mSending) {
            mSending = false;
            updateResult("Stopped\n");
        }
    }

    private void sendAtCommand(String str, int message) {
        Elog.d(TAG, "sendAtCommand() " + str);
        updateResult("Send " + str + " to Sim" + (CommandTool.mAtCmdSentToIndex +1) +"\n");
        byte[] rawData = str.getBytes();
        byte[] cmdByte = new byte[rawData.length + 1];
        System.arraycopy(rawData, 0, cmdByte, 0, rawData.length);
        cmdByte[cmdByte.length - 1] = 0;
        EmUtils.invokeOemRilRequestRawEm(CommandTool.mAtCmdSentToIndex,
                cmdByte, mAtCmdHander.obtainMessage(message));
    }

    private void updateResult(String result) {
        mOutput += result;
        if (mOnUpdateResultListener != null) {
            mOnUpdateResultListener.onUpdateResult();
        }
    }

    public String getOutput() {
        return mOutput;
    }

    public boolean isRunning() {
        return mSending;
    }

    public void setOnUpdateResultListener(OnUpdateResultListener listener) {
        mOnUpdateResultListener = listener;
    }
}

