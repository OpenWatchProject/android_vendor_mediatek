package com.mediatek.mdmlsample;

import android.content.Context;

import com.mediatek.mdml.MonitorCmdProxy;

import java.lang.ref.WeakReference;
import java.util.HashMap;

/**
 * Created by MTK03685 on 2015/9/16.
 */
public class CommandProxy {
    // MDML command proxy object
    private MonitorCmdProxy m_cmdProxy;     // create inside
    private TrapReceiverInitListener m_receiverInitListener;
    private ConnectionState m_connectState; // reference outside

    // CommandProxyListener hashmap
    private HashMap<Integer , WeakReference<CommandProxyListener>> m_listener_map;

    // constructor
    public CommandProxy(Context context) {
        m_cmdProxy = new MonitorCmdProxy(context);
        m_listener_map = new HashMap<Integer , WeakReference<CommandProxyListener>>();
    }

    // set connect state object for updating
    void SetConnectionState(ConnectionState state){
        m_connectState = state;
    }

    // set CommandProxyListener reference (weak reference)
    WeakReference<CommandProxyListener> m_listener;

    void RegisterCommandProxyListener(int hashcode, CommandProxyListener listener){
        WeakReference<CommandProxyListener> listener_ref = new WeakReference<CommandProxyListener>(listener);
        m_listener_map.put(hashcode, listener_ref);
    }

    void UnRegisterCommandProxyListener(int hashcode){
        m_listener_map.remove(hashcode);
    }

    void setTrapReceiverInitListener(TrapReceiverInitListener l) {
        m_receiverInitListener = l;
    }

    // command operation method
    void ExecuteCommand(CommandProxyAction... params) {
        new CommandProxyAsyncTask(m_cmdProxy, m_receiverInitListener,
                m_connectState, m_listener_map).execute(params);
    }
}
