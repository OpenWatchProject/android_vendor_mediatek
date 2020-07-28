package com.mediatek.engineermode.otaairplanemode;

import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.os.SystemService;

import com.mediatek.engineermode.Elog;
import com.mediatek.mdml.MONITOR_CMD_RESP;
import com.mediatek.mdml.MonitorCmdProxy;
import com.mediatek.mdml.MonitorTrapReceiver;
import com.mediatek.mdml.Msg;
import com.mediatek.mdml.PlainDataDecoder;
import com.mediatek.mdml.TRAP_TYPE;
import com.mediatek.mdml.TrapHandlerInterface;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.TimeoutException;

import static com.mediatek.mdml.MONITOR_CMD_RESP.*;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_EM;


public class MDMCoreOperation {
    public final static int MDM_SERVICE_INIT = 0;
    public final static int SUBSCRIBE_DONE = 1;
    public static final int UNSUBSCRIBE_DONE = 4;
    public static final int MSG_UPDATE_UI_URC = 7;
    public static final int UPDATE_INTERVAL = 50;
    private static final String TAG = "MDMCoreOperation";
    private static final String SERVICE_NAME = "md_monitor";
    private static final String SZ_SERVER_NAME = "demo_receiver";
    private static final int WAIT_TIMEOUT = 3000;
    private static MDMCoreOperation single = null;
    private PlainDataDecoder m_plainDataDecoder;
    private MonitorCmdProxy m_cmdProxy;
    private MonitorTrapReceiver m_trapReceiver;
    private long m_sid;
    private IMDMSeiviceInterface mMDMChangedListener;
    public Handler mUpdateUiHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_UI_URC:
                    int msgID = (int) msg.arg1;
                    String name = m_plainDataDecoder.msgInfo_getMsgName(msgID);
                    Msg msgObj = (Msg) msg.obj;
                    if (msgObj != null) {
                        mMDMChangedListener.onUpdateMDMData(name, msgObj);
                    } else {
                        Elog.e(TAG, "msgObj is null");
                    }
                    break;
                default:
            }
        }
    };
    private int mSimTypeShow = 0;
    private List<MdmBaseComponent> mComponents = null;
    private List<Long> mCheckedEmInteger = new ArrayList<Long>();

    public MDMCoreOperation() {

    }

    public static synchronized MDMCoreOperation getInstance() {
        if (single == null) {
            single = new MDMCoreOperation();
        }
        return single;
    }

    public void mdmParametersSeting(List<MdmBaseComponent> components, int simTypeShow) {
        mComponents = components;
        mSimTypeShow = simTypeShow;
    }

    public void setOnMDMChangedListener(IMDMSeiviceInterface onMDMChangedListener) {
        mMDMChangedListener = onMDMChangedListener;
    }

    public void mdmInitialize(Context context) {
        new LoadingTask().execute(context);
    }

    public void mdmlSubscribe() {
        new SubscribeTask().execute();
    }

    public void mdmlUnSubscribe() {
        new UnSubscribeTask().execute();
    }

    public void mdmlEnableSubscribe() {
        try {
            m_cmdProxy.onEnableTrap(m_sid);
        } catch (Exception e) {
            Elog.e(TAG, "MdmCoreOperation.m_cmdProxy.onEnableTrap failed ");
        }
    }

    public List<MdmBaseComponent> getSelectedComponents() {
        return mComponents;
    }

    public boolean mdmlClosing() {
        MONITOR_CMD_RESP ret = ret = m_cmdProxy.onCloseSession(m_sid);
        if (MONITOR_CMD_RESP_SUCCESS != ret) {
            Elog.e(TAG, "Close Session fail");
            return false;
        } else {
            Elog.d(TAG, "Close Session succeed");
        }
        return true;
    }

    private void StartMDLService() {
        boolean isRunning = SystemService.isRunning(SERVICE_NAME);
        if (isRunning == false) {
            Elog.v(TAG, "start md_monitor prop");
            SystemProperties.set("ctl.start","md_monitor");
            try {
                SystemService.waitForState
                        (SERVICE_NAME, SystemService.State.RUNNING, WAIT_TIMEOUT);
            } catch (TimeoutException e) {
                e.printStackTrace();
            }
            isRunning = SystemService.isRunning(SERVICE_NAME);
            if (isRunning == false) {
                Elog.e(TAG, "start md_monitor failed time out");
            }
        }
    }

    private void InitMDML(Context context) {
        m_cmdProxy = new MonitorCmdProxy(context);
        m_sid = m_cmdProxy.onCreateSession();
        m_trapReceiver = new MonitorTrapReceiver(m_sid, SZ_SERVER_NAME);
        m_trapReceiver.SetTrapHandler(new DemoHandler());
        m_cmdProxy.onSetTrapReceiver(m_sid, SZ_SERVER_NAME);
    }

    private void InitDecoder(Context context) {
        try {
            m_plainDataDecoder = PlainDataDecoder.getInstance(null,context);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    private void subscribeTrap() {
        long msgId;
        mCheckedEmInteger.clear();
        for (MdmBaseComponent com : mComponents) {
            String[] emComponentName = com.getEmComponentName();
            for (int j = 0; j < emComponentName.length; j++) {
                Integer msgIdInt = m_plainDataDecoder.msgInfo_getMsgID(emComponentName[j]);
                if (msgIdInt != null) {
                    msgId = msgIdInt.longValue();
                } else {
                    Elog.e(TAG, "The msgid is not support, msgName = " + emComponentName[j]);
                    continue;
                }
                if (!mCheckedEmInteger.contains(msgId)) {
                    mCheckedEmInteger.add(msgId);
                    Elog.d(TAG, "onSubscribeTrap msg Name = " + emComponentName[j]);
                }
            }
        }
        int size = mCheckedEmInteger.size();
        Long[] array = (Long[]) mCheckedEmInteger.toArray(new Long[size]);
        Elog.d(TAG, "onSubscribeTrap msg id = " + Arrays.toString(array));
        m_cmdProxy.onSubscribeMultiTrap(m_sid, TRAP_TYPE_EM, toPrimitives(array));
    }

    public long[] toPrimitives(Long... objects) {
        long[] primitives = new long[objects.length];
        for (int i = 0; i < objects.length; i++)
            primitives[i] = objects[i];
        return primitives;
    }

    private void unSubscribeTrap() {
        Elog.d(TAG, "Before unSubscribeTrap..");
        int size = mCheckedEmInteger.size();
        Long[] array = (Long[]) mCheckedEmInteger.toArray(new Long[size]);
        Elog.d(TAG, "unSubscribeTrap msg id = " + Arrays.toString(array));
        m_cmdProxy.onUnsubscribeMultiTrap(m_sid, TRAP_TYPE_EM, toPrimitives(array));

        Elog.d(TAG, "onDisableTrap..");
        MONITOR_CMD_RESP ret = m_cmdProxy.onDisableTrap(m_sid);
        if (MONITOR_CMD_RESP_SUCCESS != ret) {
            Elog.e(TAG, "Disable Trap fail");
            return;
        } else {
            Elog.d(TAG, "Disable Trap succeed");
        }
        SystemProperties.set("ctl.stop","md_monitor");
        Elog.d(TAG, "Disable mdm monitor");
    }

    public int getFieldValue(Msg data, String msgName) {
        byte[] bData = data.getFieldValue(msgName);
        int iData = (int) com.mediatek.engineermode.mdmcomponent.Utils.getIntFromByte(bData);
        return iData;
    }

    public int getFieldValue(Msg data, String msgName, boolean sign) {
        if (!sign) {
            return getFieldValue(data, msgName);
        }
        byte[] bData = data.getFieldValue(msgName);
        int iData = (int) com.mediatek.engineermode.mdmcomponent.Utils.getIntFromByte(bData, true);
        return iData;
    }

    public interface IMDMSeiviceInterface {
        public void onUpdateMDMStatus(int msg_id);

        public void onUpdateMDMData(String name, Msg data);
    }

    private class DemoHandler implements TrapHandlerInterface {

        public void ProcessTrap(long timestamp, TRAP_TYPE type, int len, byte[] data, int offset) {
            Message msgToUi = mUpdateUiHandler.obtainMessage();
            msgToUi.what = MSG_UPDATE_UI_URC;
            int msgID = (int) Utils.getIntFromByte(data, offset + 6, 2);
            Msg msg = m_plainDataDecoder.msgInfo_getMsg(data, offset);
            // MDML define the sim index from 1 to 4 but mSimType is from 0 to 3
            if ((msg == null) || (msg.getSimIdx() != (mSimTypeShow + 1))) {
                Elog.d(TAG, "msg is null in ProcessTrap");
                return;
            }
            //Elog.d(TAG, "frame is incoming: len = [" + len + "], msgID = [" + msgID + "]");
            msgToUi.arg1 = msgID;
            msgToUi.obj = msg;
            mUpdateUiHandler.sendMessageDelayed(msgToUi, UPDATE_INTERVAL);

        }
    }

    private class LoadingTask extends AsyncTask<Context, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Context... params) {
            // TODO Auto-generated method stub
            Elog.d(TAG, "before InitMDML");
            StartMDLService();
            InitMDML(params[0]);
            Elog.d(TAG, "after InitMDML");
            InitDecoder(params[0]);
            Elog.d(TAG, "after InitDecoder");
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            Elog.d(TAG, "LoadingTask done");
            mMDMChangedListener.onUpdateMDMStatus(MDM_SERVICE_INIT);
        }
    }

    private class SubscribeTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... params) {
            subscribeTrap();
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            if (result) {
                Elog.d(TAG, "onSubscribeTrap done");
                mMDMChangedListener.onUpdateMDMStatus(SUBSCRIBE_DONE);
            }
        }
    }

    private class UnSubscribeTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... params) {
            unSubscribeTrap();
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            Elog.d(TAG, "onUnSubscribeTrap done");
            mMDMChangedListener.onUpdateMDMStatus(UNSUBSCRIBE_DONE);
        }
    }
}

