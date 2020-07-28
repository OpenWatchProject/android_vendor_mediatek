package com.mediatek.engineermode.mdmdiagnosticinfo;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.TimeoutException;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.os.SystemService;

import com.mediatek.mdml.MONITOR_CMD_RESP;
import com.mediatek.mdml.MonitorCmdProxy;
import com.mediatek.mdml.MonitorTrapReceiver;
import com.mediatek.mdml.Msg;
import com.mediatek.mdml.PlainDataDecoder;
import com.mediatek.mdml.TRAP_TYPE;
import com.mediatek.mdml.TrapHandlerInterface;

import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_EM;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_ICD_RECORD;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_ICD_EVENT;
import static com.mediatek.mdml.MONITOR_CMD_RESP.*;

import com.mediatek.engineermode.Elog;

public class MDMCoreOperation {
    private static final String TAG = "EM_DiagnosticMetric_Info";
    private static final String SERVICE_NAME = "md_monitor";
    private static final String SZ_SERVER_NAME = "demo_receiver";

    private static final int WAIT_TIMEOUT = 3000;
    public static final int UPDATE_INTERVAL = 50;
    public static final int MSG_UPDATE_UI_URC_EM = 0;
    public static final int MSG_UPDATE_UI_URC_ICD = 1;
    public final static int LOADING_DONE = 0;
    public final static int SUBSCRIBE_DONE = 1;
    public final static int UPDATE_UI = 2;
    public static final int ENABLE_SUBSCRIBE_DONE = 3;
    public static final int UNSUBSCRIBE_DONE = 4;

    public static final int MDML_TRAP_TYPE_EM = 0;
    public static final int MDML_TRAP_TYPE_ICD_RECORD = 1;
    public static final int MDML_TRAP_TYPE_ICD_EVENT = 2;

    private PlainDataDecoder m_plainDataDecoder;
    private MonitorCmdProxy m_cmdProxy;
    private MonitorTrapReceiver m_trapReceiver;
    private long m_sid;
    private boolean isMsgShow = false;

    private List<String> mCheckedEmTypes = new ArrayList<String>();
    private List<Long> mCkeckedIcdRecordInt = new ArrayList<Long>();
    private List<Long> mCkeckedIcdInteger = new ArrayList<Long>();
    private List<MdmBaseComponent> components = new ArrayList<MdmBaseComponent>();
    private List<Long> mCheckedEmInteger = new ArrayList<Long>();

    private IDataTransaction transactionListener;
    private IDataUpdate dataUpdateListener;

    public MDMCoreOperation() {

    }

    public void setOnTransactionListener(IDataTransaction onTransactionListener){
        Elog.d(TAG, "[setOnTransactionListener]: " +
               onTransactionListener.getClass().getSimpleName());
        this.transactionListener = onTransactionListener;
        components = transactionListener.onItemSelect();
    }

    public void setOnDataUpdateListener(IDataUpdate onDataUpdateListener){
        Elog.d(TAG, "[setOnDataUpdateListener]: " +
               onDataUpdateListener.getClass().getSimpleName());
        this.dataUpdateListener = onDataUpdateListener;
    }

    private class DemoHandler implements TrapHandlerInterface {

        public void ProcessTrap(long timestamp, TRAP_TYPE type, int len, byte[] data, int offset)
        {
            Message msgToUi = mUpdateUiHandler.obtainMessage();
            // Decode trap by trap type
            switch (type) {
                case TRAP_TYPE_EM: {
                    msgToUi.what = MSG_UPDATE_UI_URC_EM;
                    Object objectMsg = m_plainDataDecoder.msgInfo_getMsg(data, offset);
                    if(! (objectMsg instanceof Msg)) return;
                    Msg msg = (Msg) objectMsg;
                    int msgID = (int) Utils.getIntFromByte(data, offset + 6, 2);
                    // MDML define the sim index from 1 to 4 but mSimType is from 0 to 3
                    if(msg == null){
                        Elog.d(TAG, "[TrapType][EM] Msg is null,Skip!");
                        return;
                    }
                    if (msg.getSimIdx() != (MDMComponentSelectActivity.mSimType+1)) {
                        Elog.d(TAG, "[TrapType][EM] ProcessTrap " + msg.getSimIdx()
                                + ", Selected SIM" + (MDMComponentSelectActivity.mSimType+1)
                                + ",Skip!");
                        return;
                    }
                    msgToUi.arg1 = msgID;
                    msgToUi.obj = msg;
                    mUpdateUiHandler.sendMessageDelayed(msgToUi,
                            UPDATE_INTERVAL);
                    break;
                }
                case TRAP_TYPE_ICD_RECORD:
                    /*
                    byte [] trap_data = Arrays.copyOfRange(data, offset, data.length);
                    ICDMsg recordMsg = ICDCodec.getICDMsg(trap_data);
                    if(recordMsg == null){
                        Elog.d(TAG, "[TrapType][ICD_RECORD] ICDMsg is null,Skip!");
                    }
                    if(recordMsg.headerInfo.return_code == 1){  // msg parse successfully
                        if(recordMsg.headerInfo.protocolID !=
                                (MDMComponentSelectActivity.mSimType+1)) {
                            Elog.d(TAG, "[TrapType][ICD_RECORD] ProcessTrap SIM"
                                    + recordMsg.headerInfo.protocolID + ",Selected SIM"
                                    + (MDMComponentSelectActivity.mSimType+1) + ",Skip!");
                            return;
                        }
                        msgToUi.what = MSG_UPDATE_UI_URC_ICD;
                        msgToUi.obj = recordMsg;
                        mUpdateUiHandler.sendMessageDelayed(msgToUi,
                                UPDATE_INTERVAL);
                    }else{
                        Elog.d(TAG, "[TrapType][ICD_RECORD] ICDMsg parse error, return code : "
                                + recordMsg.headerInfo.return_code);
                    }*/
                    break;
                case TRAP_TYPE_ICD_EVENT:
                    /*
                    byte [] trap_data = Arrays.copyOfRange(data, offset, data.length);
                    ICDMsg eventMsg = ICDCodec.getICDMsg(trap_data);
                    if(eventMsg != null){
                        Elog.d(TAG, "[TrapType][ICD_EVENT] ICDMsg is null,Skip!");
                    }
                    if(eventMsg.headerInfo.return_code == 1){  // msg parse successfully
                        if(eventMsg.headerInfo.protocolID !=
                                (MDMComponentSelectActivity.mSimType+1)) {
                            Elog.d(TAG, "[TrapType][ICD_EVENT] ProcessTrap with "
                                    + eventMsg.headerInfo.protocolID + ",Selected SIM"
                                    + (MDMComponentSelectActivity.mSimType+1) + ",Skip!");
                            return;
                        }
                        msgToUi.what = MSG_UPDATE_UI_URC_ICD;
                        msgToUi.obj = eventMsg;
                        mUpdateUiHandler.sendMessageDelayed(msgToUi,
                                UPDATE_INTERVAL);
                    }else{
                        Elog.d(TAG, "[TrapType][ICD_EVENT] ICDMsg parse error, return code : "
                                + eventMsg.headerInfo.return_code);
                    }
                    */
                    break;
                case TRAP_TYPE_DISCARDINFO:
                    Elog.d(TAG, "[Discard ... ]");
                    break;
                default:
                    Elog.d(TAG, "[Unknown Type, Error ...]");
                    break;
            }
        }
    };

    public Handler mUpdateUiHandler = new Handler() {
        public void handleMessage(Message msg) {
            int msgID = -1;
            switch (msg.what) {
                case MSG_UPDATE_UI_URC_EM:
                    msgID = msg.arg1;
                    String name  = m_plainDataDecoder.msgInfo_getMsgName(msgID);
                    Msg msgObj = (Msg) msg.obj;
                    if (msgObj != null) {
                        for (MdmBaseComponent c : components) {
                            if (((MdmComponent)c).hasTrapType(name, msgID)
                                    && isMsgShow == true) {
                                Elog.v(TAG, "[UpdateUI][EM][" + msgID + "]Update data to "
                                    + c.getGroup() + "[" + c.getName() + "], type = " + name);
                                c.update(name, msgObj);
                            }
                        }
                    } else {
                        Elog.e(TAG, "[UpdateUI][EM][" + msgID + "] MsgObj is null");
                    }
                    break;
                case MSG_UPDATE_UI_URC_ICD:
                   /* ICDMsg icdMsgObj = (ICDMsg) msg.obj;
                    msgID = icdMsgObj.headerInfo.code;
                    if(icdMsgObj != null) {
                        for (MdmBaseComponent c : components) {
                            if (((MdmComponent)c).hasTrapType(null, msgID) && isMsgShow == true) {
                                Elog.v(TAG, "[UpdateUI][ICD][" + msgID + "] Update data to "
                                    + c.getGroup() + "[" + c.getName() + "], type = " + msgID);
                                c.update(msgID, icdMsgObj);
                            }
                        }
                    } else {
                        Elog.e(TAG, "[UpdateUI][ICD][" + msgID + "] ICDMsgObj is null");
                    }*/
                    break;
                default:
                    break;
            }
        }
    };

    public void mdmlLoading(Context context) {
        new LoadingTask().execute(context);
    }

    /**
     * Background operation for attach.
     *
     */
    private class LoadingTask extends AsyncTask<Context, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Context... params) {
            // TODO Auto-generated method stub
            Elog.d(TAG, "[LoadingTask] before InitMDML");
                    // Setup ICDCodec
            String jsonPath = "/sdcard/Download/AllICD.json";
            //int ret = ICDCodec.LoadICDDatabase(jsonPath);
            //Elog.d(TAG, "[LoadingTask] LoadICDDatabase ret = " + ret);
            StartMDLService();
            InitMDML(params[0]);
            Elog.d(TAG, "[LoadingTask] after InitMDML");
            InitDecoder(params[0]);
            Elog.d(TAG, "[LoadingTask] after InitDecoder");
            return true;
        }
        @Override
        protected void onPostExecute(Boolean result) {
            transactionListener.onUpdateUI(LOADING_DONE);
        }
    };

    private void StartMDLService(){
        boolean isRunning = SystemService.isRunning(SERVICE_NAME);
        if (isRunning == false) {
            Elog.v(TAG, "start md_monitor prop");
            SystemProperties.set("ctl.start","md_monitor");
            try {
                SystemService.waitForState
                  (SERVICE_NAME, SystemService.State.RUNNING,  WAIT_TIMEOUT);
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

    /* decoder */
    private void InitDecoder(Context context) {
        try {
            m_plainDataDecoder = PlainDataDecoder.getInstance(null,context);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public void mdmlSubscribe() {
        new SubscribeTask().execute();
    }

    /**
     * Background operation for attach.
     *
     */
    private class SubscribeTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub
            if(transactionListener == null) {
                Elog.d(TAG, "onTransactionListener is null!");
                return false;
            }
            long msgId = 0;
            mCheckedEmTypes.clear();
            mCheckedEmInteger.clear();
            mCkeckedIcdInteger.clear();
            for (MdmBaseComponent com : components) {
                HashMap<String, Long> emComponentName =
                        (HashMap<String, Long>)com.getEmComponentMap();
                for (String name : emComponentName.keySet()) {
                    if (!mCheckedEmTypes.contains(name)) {
                        mCheckedEmTypes.add(name);
                        msgId = com.getEmComponentMap().get(name);
                        if(com.getTrapType() == MDML_TRAP_TYPE_EM) {
                            Integer msgIdInt =
                                m_plainDataDecoder.msgInfo_getMsgID(name);
                            if (msgIdInt != null && msgId == -1) {
                                msgId = Long.valueOf(msgIdInt);
                                emComponentName.put(name, msgId);
                            } else if(msgIdInt == null && msgId == -1){
                                Elog.e(TAG, "[SubscribeTask]The msg is not support, msgName="
                                        + name);
                                continue;
                            }
                            if (!mCheckedEmInteger.contains(msgId)) {
                                mCheckedEmInteger.add(msgId);
                                // Subscribe all EMs at beginning
                                Elog.d(TAG, "[SubscribeTask] onSubscribeTrap msg " + name);
                                m_cmdProxy.onSubscribeTrap(m_sid, TRAP_TYPE_EM, msgId);
                            }
                        } else if(com.getTrapType() == MDML_TRAP_TYPE_ICD_RECORD){
                            if (msgId == -1) {
                                Elog.e(TAG, "[SubscribeTask]The msg is not support, msgId="
                                        + msgId);
                                continue;
                            }
                            if (!mCkeckedIcdInteger.contains(msgId)) {
                                mCkeckedIcdInteger.add(msgId);
                                // Subscribe all EMs at beginning
                                Elog.d(TAG, "[SubscribeTask]onSubscribeTrap msg " + msgId);
                                m_cmdProxy.onSubscribeTrap(m_sid, TRAP_TYPE_ICD_RECORD, msgId);
                            }
                        } else if(com.getTrapType() == MDML_TRAP_TYPE_ICD_EVENT) {
                            if (msgId == -1) {
                                Elog.e(TAG, "[SubscribeTask]The msg is not support, msgId="
                                        + msgId);
                                continue;
                            }
                            if (!mCkeckedIcdInteger.contains(msgId)) {
                                mCkeckedIcdInteger.add(msgId);
                                // Subscribe all EMs at beginning
                                Elog.d(TAG, "[SubscribeTask] onSubscribeTrap msg " + msgId);
                                m_cmdProxy.onSubscribeTrap(m_sid, TRAP_TYPE_ICD_EVENT, msgId);
                            }
                        }

                    }
                }
            }
            return true;
        }
        @Override
        protected void onPostExecute(Boolean result) {
            if (result) {
                Elog.d(TAG, "init MDMComponentDetailActivity done");
                transactionListener.onUpdateUI(SUBSCRIBE_DONE);
            }
        }
    };

    public List<MdmBaseComponent> getSelectedComponents() {
        return components;
    }

    public void mdmlClosing() {
        new CloseMDMLTask().execute();
    }

    private class CloseMDMLTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub
            MONITOR_CMD_RESP ret = m_cmdProxy.onDisableTrap(m_sid);
            if (MONITOR_CMD_RESP_SUCCESS != ret) {
                Elog.d(TAG, "[CloseMDMLTask] Disable Trap fail");
            }
            ret = m_cmdProxy.onCloseSession(m_sid);
            if (MONITOR_CMD_RESP_SUCCESS != ret) {
                Elog.d(TAG, "[CloseMDMLTask] Close Session fail");
            }
            return true;
        }
    };

    public void mdmlUnSubscribe() {
        new UnSubscribeTask().execute();
    }

    private class UnSubscribeTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub
            unSubscribeTrap();
            return true;
        }
        @Override
        protected void onPostExecute(Boolean result) {
            dataUpdateListener.onDataUpdate(UNSUBSCRIBE_DONE);
        }
    }

    public void unSubscribeTrap() {
        int i = 0;
        long msgId = 0;
        Elog.d(TAG, "Before unSubscribeTrap..");
        mCheckedEmInteger.clear();
        mCkeckedIcdInteger.clear();
        for (MdmBaseComponent com : components) {
            HashMap<String, Long> emComponentName =
                    (HashMap<String, Long>)com.getEmComponentMap();
            for (String name : emComponentName.keySet()) {
                if (!mCheckedEmTypes.contains(name)) {
                    mCheckedEmTypes.add(name);
                    msgId = com.getEmComponentMap().get(name);
                    if(com.getTrapType() == MDML_TRAP_TYPE_EM) {
                        Integer msgIdInt =
                            m_plainDataDecoder.msgInfo_getMsgID((String)name);
                        if (msgIdInt != null && msgId == -1) {
                            msgId = Long.valueOf(msgIdInt);
                            emComponentName.put(name, msgId);
                        } else if(msgIdInt == null && msgId == -1){
                            Elog.e(TAG, "[unSubscribeTrap]The msgid is not support, msgName="
                                    + name);
                            continue;
                        }
                        if (!mCheckedEmInteger.contains(msgId)) {
                            mCheckedEmInteger.add(msgId);
                            // Subscribe all EMs at beginning
                            Elog.d(TAG, "[unSubscribeTrap]UnsubscribeTrap msg Name=" +
                                    name.toString());
                            m_cmdProxy.onUnsubscribeTrap(m_sid, TRAP_TYPE_EM, msgId);
                        }
                    } else if(com.getTrapType() == MDML_TRAP_TYPE_ICD_RECORD){
                        if (msgId == -1) {
                            Elog.e(TAG, "[unSubscribeTrap]The msgid is not support, msgId="
                                    + msgId);
                            continue;
                        }
                        if (!mCkeckedIcdInteger.contains(msgId)) {
                            mCkeckedIcdInteger.add(msgId);
                            // Subscribe all EMs at beginning
                            Elog.d(TAG, "[unSubscribeTrap]UnsubscribeTrap msg msgId=" + msgId);
                            m_cmdProxy.onUnsubscribeTrap(m_sid, TRAP_TYPE_ICD_RECORD, msgId);
                        }
                    } else if(com.getTrapType() == MDML_TRAP_TYPE_ICD_EVENT) {
                        if (msgId == -1) {
                            Elog.e(TAG, "[unSubscribeTrap]The msgid is not support, msgId="
                                    + msgId);
                            continue;
                        }
                        if (!mCkeckedIcdInteger.contains(msgId)) {
                            mCkeckedIcdInteger.add(msgId);
                            // Subscribe all EMs at beginning
                            Elog.d(TAG, "[unSubscribeTrap]UnsubscribeTrap msg msgId=" + msgId);
                            m_cmdProxy.onUnsubscribeTrap(m_sid, TRAP_TYPE_ICD_EVENT, msgId);
                        }
                    }
                    Elog.d(TAG, "[unSubscribeTrap] UnsubscribeTrap " + name + ":" +
                            emComponentName.get(name).toString());
                }
            }
        }
    }

    public void mdmlEnableSubscribe() {
        new EnableSubscribeTask().execute();
    }

    private class EnableSubscribeTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub
            try {
                m_cmdProxy.onEnableTrap(m_sid);
            } catch (Exception e) {
                Elog.e(TAG, "[EnableSubscribeTask] onEnableTrap failed ");
            }
            return true;
        }
        @Override
        protected void onPostExecute(Boolean result) {
            Elog.d(TAG, "[EnableSubscribeTask] onEnableTrap done");
            dataUpdateListener.onDataUpdate(ENABLE_SUBSCRIBE_DONE);
            isMsgShow = true;
        }
    };

    public interface IDataTransaction {

        public List<MdmBaseComponent> onItemSelect();
        public void onUpdateUI(int msg_id);
    };

    public  interface IDataUpdate {

        public void onDataUpdate(int msg_id);
    };
}

