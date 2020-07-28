package com.mediatek.engineermode.boot;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;

import com.mediatek.engineermode.Elog;

import java.util.HashMap;
import java.util.Map;

/**
 * a start service of EM boot.
 * @author: mtk81238
 */
public class EmBootStartService extends Service {
    private static final String TAG = "BootStartService";
    private static final String KEY_REQ_START_SERV = "req_start_serv";
    private static final int MSG_CHECK_STOP_SERVICE = 111;

    private static Map<String, IBootServiceHandler> sStartReqHandlerMap
            = new HashMap<String, IBootServiceHandler>();

    private Handler mMainHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_CHECK_STOP_SERVICE:
                synchronized (EmBootStartService.class) {
                    Elog.d(TAG, "size: " + sStartReqHandlerMap.size());
                    if (sStartReqHandlerMap.size() == 0) {
                        stopSelf();
                    }
                }
                break;
            default:
                break;
            }
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }


    /**
     * stop started service.
     * @param handler IBootServiceHandler
     */
    private void stopStartedService(IBootServiceHandler handler) {
        if (handler != null) {
            String handlerName = handler.getClass().getCanonicalName();
            synchronized (EmBootStartService.class) {
                sStartReqHandlerMap.remove(handlerName);
            }
        }
        mMainHandler.sendEmptyMessage(MSG_CHECK_STOP_SERVICE);
    }

    private int onHandleRequest(String handlerName) {
        IBootServiceHandler handler = sStartReqHandlerMap.get(handlerName);
        if (handler == null) {
            return IBootServiceHandler.HANDLE_INVALID;
        }
        return handler.handleStartRequest(this);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null) {
            String handlerName = intent.getStringExtra(KEY_REQ_START_SERV);
            int ret = onHandleRequest(handlerName);
            if (ret != IBootServiceHandler.HANDLE_ONGOING) {
                stopStartedService(sStartReqHandlerMap.get(handlerName));
            }
        }
        return START_NOT_STICKY;
    }

}
