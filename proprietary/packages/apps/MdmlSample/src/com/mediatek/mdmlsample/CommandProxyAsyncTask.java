package com.mediatek.mdmlsample;

import android.os.AsyncTask;
import android.os.SystemProperties;
import android.os.SystemService;
import android.util.Log;

import com.mediatek.mdml.MONITOR_CMD_RESP;
import static com.mediatek.mdml.MONITOR_CMD_RESP.*;
import com.mediatek.mdml.MonitorCmdProxy;

import java.lang.ref.WeakReference;
import java.util.concurrent.TimeoutException;
import java.util.HashMap;

/**
 * Created by MTK03685 on 2015/9/14.
 */
public class CommandProxyAsyncTask extends AsyncTask<CommandProxyAction, Integer, Integer> {
    private static final String TAG = "CommandProxyAsyncTask";
    private static final String SERVICE_NAME = "md_monitor";
    private static final String MDM_PROP = "persist.vendor.mdmmonitor";
    private static final int WAIT_TIMEOUT = 3000;

    // resource objects
    private MonitorCmdProxy m_cmdProxy;
    private TrapReceiverInitListener m_receiverInitListener;
    private ConnectionState m_connectState;
    // UI updater for each registered activity (use WeakReference to avoid activity die)
    private HashMap<Integer , WeakReference<CommandProxyListener>> m_listener_map;  // progress and result listener

    public CommandProxyAsyncTask(MonitorCmdProxy cmdproxy,
            TrapReceiverInitListener receiverInitListener,
            ConnectionState connectState,
            HashMap<Integer , WeakReference<CommandProxyListener>> listener_map){
        m_cmdProxy = cmdproxy;
        m_receiverInitListener = receiverInitListener;
        m_connectState = connectState;
        m_listener_map = listener_map;
    }

    @Override
    protected void onPreExecute() {
        super.onPreExecute();
        Log.d(TAG, "onPreExecute() : sid = " + m_connectState.m_sid);

        // update pre-exe state for each listener in hash map
        for(WeakReference<CommandProxyListener> listener : m_listener_map.values()) {
            if (listener.get() != null) {
                listener.get().onCommandPreExecute();
            }
        }
    }

    @Override
    protected Integer doInBackground(CommandProxyAction... params) {
        int commands_count = params.length;  // commands number for this asynctask
        int overall_ret = 0;
        // set processing state in connection state
        m_connectState.m_bCommandProcessing = true;

        //execute commands one by one
        for(int commands_i = 0 ; commands_i < commands_count ; commands_i++) {
            if(params[commands_i] != null){
                int ret = DoCommandProxyOperation(params[commands_i]);
                if(ret != 0){
                    overall_ret = ret;
                }
            }

            // update progress status
            publishProgress((int)((commands_i/(float)commands_count)*100));
        }

        // set processing done in connection state
        m_connectState.m_bCommandProcessing = false;

        return overall_ret;
    }

    @Override
    protected void onProgressUpdate(Integer... values) {
        super.onProgressUpdate(values);

        // Update progress bar for each listener in hash map
        for(WeakReference<CommandProxyListener> listener : m_listener_map.values()) {
            if (listener.get() != null) {
                listener.get().onCommandProgressUpdate(values[0]);
            }
        }
    }

    @Override
    protected void onPostExecute(Integer result) {
        super.onPostExecute(result);

        // check there is no other asynctask in exe
        if(m_connectState.m_bCommandProcessing == false) {
            // update finish state for each listener in hash map
            for (WeakReference<CommandProxyListener> listener : m_listener_map.values()) {
                if (listener.get() != null) {
                    listener.get().onCommandFinishUpdate(result);
                }
            }
        }
    }

    private int DoCommandProxyOperation(CommandProxyAction action){
        switch (action.actionType){
            case ACTION_TYPE_ENABLE_MDM:
            {
                boolean isRunning = SystemService.isRunning(SERVICE_NAME);
                if (isRunning == false) {
                    Log.v(TAG, "start md_monitor prop");
                    SystemProperties.set(MDM_PROP, "1");
                    try {
                        SystemService.waitForState(SERVICE_NAME, SystemService.State.RUNNING,  WAIT_TIMEOUT);
                    } catch (TimeoutException e) {
                        e.printStackTrace();
                    }
                    isRunning = SystemService.isRunning(SERVICE_NAME);
                    if (isRunning == false) {
                        Log.v(TAG, "start md_monitor failed time out");
                    }
                }
                break;
            }
            case ACTION_TYPE_CREATE_SESSION:
            {
                Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_CREATE_SESSION ");
                long sid = m_cmdProxy.onCreateSession();
                // update to ConnectionState
                Log.d(TAG, "Got session id: " + sid);
                m_connectState.m_sid = sid;
                m_connectState.m_bConnected = true;
                break;
            }
            case ACTION_TYPE_CLOSE_SESSION: {
                Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_CLOSE_SESSION ");
                m_cmdProxy.onCloseSession(action.sessionID);
                // update to ConnectionState
                m_connectState.m_sid = 0;
                m_connectState.m_bConnected = false;
                break;
            }
            case ACTION_TYPE_SET_TRAP_RECEIVER: {
                Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_SET_TRAP_RECEIVER , sid :" + m_connectState.m_sid + " server name : " + action.serverName);
                m_receiverInitListener.onTrapReceiverInit(m_connectState.m_sid, action.serverName);
                m_cmdProxy.onSetTrapReceiver(m_connectState.m_sid, action.serverName);
                break;
            }
            case ACTION_TYPE_CREATE_SESSION_AND_SET_TRAP_RECEIVER:{
                Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_CREATE_SESSION_AND_SET_TRAP_RECEIVER , server name : " + action.serverName);
                long sid = m_cmdProxy.onCreateSession();
                // update to ConnectionState
                Log.d(TAG, "Got session id: " + sid);
                m_connectState.m_sid = sid;
                m_connectState.m_bConnected = true;
                Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_SET_TRAP_RECEIVER ");
                m_receiverInitListener.onTrapReceiverInit(m_connectState.m_sid, action.serverName);
                m_cmdProxy.onSetTrapReceiver(sid, action.serverName);
                break;
            }
            case ACTION_TYPE_ENABLE_TRAP: {
                Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_ENABLE_TRAP , sid :" + m_connectState.m_sid);
                MONITOR_CMD_RESP ret = m_cmdProxy.onEnableTrap(m_connectState.m_sid);
                // error check
                if (MONITOR_CMD_RESP_SUCCESS != ret) {
                    Log.e(TAG, "Failed to Enable Trap. err = [" + ret + "]");
                    // TODO: 2015/9/15
                    return 1;
                }
                // update to ConnectionState
                m_connectState.m_bTrapEnabled = true;
                break;
            }
            case ACTION_TYPE_DISABLE_TRAP: {
                Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_DISABLE_TRAP , sid :" +m_connectState.m_sid);
                MONITOR_CMD_RESP ret = m_cmdProxy.onDisableTrap(m_connectState.m_sid);
                // error check
                if (MONITOR_CMD_RESP_SUCCESS != ret) {
                    Log.e(TAG, "Failed to Disable Trap. err = [" + ret + "]");
                    // TODO: 2015/9/15
                    return 1;
                }
                // update to ConnectionState
                m_connectState.m_bTrapEnabled = false;
                break;
            }
            case ACTION_TYPE_SUBSCRIBE_TRAP: {
                //Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_SUBSCRIBE_TRAP , sid :" + m_connectState.m_sid + "type :" + action.trapType + "msgID :" + action.msgID);
                MONITOR_CMD_RESP ret;
                if (action.msgIDArray == null){
                    ret = m_cmdProxy.onSubscribeTrap(m_connectState.m_sid, action.trapType, action.msgID);
                } else {
                    ret = m_cmdProxy.onSubscribeMultiTrap(m_connectState.m_sid, action.trapType, action.msgIDArray);
                }
                // error check
                if (MONITOR_CMD_RESP_SUCCESS != ret) {
                    Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_SUBSCRIBE_TRAP , sid :" + m_connectState.m_sid + "type :" + action.trapType + "msgID :" + action.msgID);
                    Log.e(TAG, "Failed to Subscribe Trap. err = [" + ret + "]");
                    // TODO: 2015/9/15
                    return 1;
                }
                break;
            }
            case ACTION_TYPE_UNSUBSCRIBE_TRAP: {
                //Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_UNSUBSCRIBE_TRAP , sid :" + m_connectState.m_sid + "type :" + action.trapType + "msgID :" + action.msgID);
                MONITOR_CMD_RESP ret;
                if (action.msgIDArray == null){
                    ret = m_cmdProxy.onUnsubscribeTrap(m_connectState.m_sid, action.trapType, action.msgID);
                } else {
                    ret = m_cmdProxy.onUnsubscribeMultiTrap(m_connectState.m_sid, action.trapType, action.msgIDArray);
                }
                // error check
                if (MONITOR_CMD_RESP_SUCCESS != ret) {
                    Log.d(TAG, "DoCommandProxyOperation() : Do ACTION_TYPE_UNSUBSCRIBE_TRAP , sid :" + m_connectState.m_sid + "type :" + action.trapType + "msgID :" + action.msgID);
                    Log.e(TAG, "Failed to UnSubscribe Trap. err = [" + ret + "]");
                    // TODO: 2015/9/15
                    return 1;
                }
                break;
            }
            default:
                Log.e(TAG, "DoCommandProxyOperation() : Unkonwn operation type ");
                return 1;
        }
        return 0; // success
    }
}
