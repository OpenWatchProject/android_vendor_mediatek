package com.mediatek.mdmlsample;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.mdml.TRAP_TYPE;
import com.mediatek.mdml.PlainDataDecoder;

import java.util.ArrayList;
import java.util.Arrays;

import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_EM;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_OTA;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_ICD_RECORD;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_ICD_EVENT;

public class MainActivity extends Activity {
    private static final String TAG = "MDMLSample";
    // UI
    protected Switch m_swtEnable;
    protected Button m_btnEM;
    protected Button m_btnGLOBAL;
    protected Button m_btnOTA;

    protected TextView m_textviewResult;
    protected ScrollView m_scrollview;
    protected ProgressBar m_progressCircle;
    protected ProgressBar m_progressBar;
    //
    ConnectionState m_connectState;
    CommandProxy m_cmdProxy;
    commandUIUpdater m_cmdListener;
    TrapReceiverService m_trapReceiverService; // trap services reference
    boolean mBound = false; // trap services attach flag
    DemoListener m_trapListener;

    // used to update command proxy related UI widget
    public class commandUIUpdater implements CommandProxyListener {
        public void onCommandPreExecute() {
            Log.d(TAG, "onCommandPreExecute ! sid = " + m_connectState.m_sid);
            // disable related widget
            m_swtEnable.setClickable(false);
            m_btnEM.setClickable(false);
            m_btnGLOBAL.setClickable(false);
            m_btnOTA.setClickable(false);
            m_swtEnable.setEnabled(false);
            m_btnEM.setEnabled(false);
            m_btnGLOBAL.setEnabled(false);
            m_btnOTA.setEnabled(false);
            // enable progress bar
            m_progressCircle.setVisibility(View.VISIBLE);
            m_progressBar.setVisibility(View.VISIBLE);
        }

        public void onCommandProgressUpdate(int progressPercentage) {
            Log.d(TAG, "onCommandProgressUpdate ! sid = "
                    + m_connectState.m_sid);
            m_progressBar.setProgress(progressPercentage);
        }

        public void onCommandFinishUpdate(int commandResult) {
            Log.d(TAG, "onCommandFinishUpdate !");
            // toast message about execution result
            if (commandResult == 0) {
                Toast.makeText(MainActivity.this,
                        "Command proxy async task done", Toast.LENGTH_SHORT)
                        .show();
            } else {
                Toast.makeText(MainActivity.this,
                        "Command proxy async task failed", Toast.LENGTH_SHORT)
                        .show();
            }

            // set UI widget visible
            m_swtEnable.setClickable(true);
            m_btnEM.setClickable(true);
            m_btnGLOBAL.setClickable(true);
            m_btnOTA.setClickable(true);
            m_swtEnable.setEnabled(true);
            m_btnEM.setEnabled(true);
            m_btnGLOBAL.setEnabled(true);
            m_btnOTA.setEnabled(true);
            m_progressCircle.setVisibility(View.INVISIBLE);
            m_progressBar.setVisibility(View.INVISIBLE);

        }
    }

    private class DemoListener implements NewTrapListener {
        public void NewTrapArrival() {
            Log.d(TAG, "frame is incoming:");
            runOnUiThread(new UIUpdater());
            Log.d(TAG, "frame is incoming... done!");
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LoadMDML();
        InitUI();
    }

    private void LoadMDML() {
        InitCmdProxy();
        InitTrapReceiverService();
    }

    private void InitCmdProxy() {
        // Get MDML objects and connection state from app object
        AppApplication app = (AppApplication) getApplicationContext();
        m_connectState = new ConnectionState();
        m_cmdProxy = new CommandProxy(this);
        m_cmdProxy.SetConnectionState(m_connectState);

        // set proxy command state listener to global CommandProxy object
        m_cmdListener = new commandUIUpdater();
        m_cmdProxy.RegisterCommandProxyListener(hashCode(), m_cmdListener);
    }

    private void InitTrapReceiverService() {
        // bind to services
        Intent intent = new Intent(this, TrapReceiverService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    /** Defines callbacks for service binding, passed to bindService() */
    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            // We've bound to LocalService, cast the IBinder and get
            // LocalService instance
            TrapReceiverService.LocalBinder binder = (TrapReceiverService.LocalBinder) service;
            m_trapReceiverService = binder.getService();
            mBound = true;
            Log.d(TAG, "onServiceConnected()");
            m_trapListener = new DemoListener();
            m_trapReceiverService.registerLogUpdater(m_trapListener);

            ConnectMDM();
            InitDecoder();
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mBound = false;
        }
    };

    TrapFileReader trapFileReader = null;

    // inner log updater class
    public class UIUpdater implements Runnable {
        public void run() {
            if (trapFileReader == null)
                trapFileReader = new TrapFileReader(getApplicationContext());
            Log.d(TAG, "UIUpdater() : Run");
            // Read the latest trap text from file
            StringBuilder trapText = trapFileReader.GetLastTrapContext();
            if (trapText.length() != 0) {
                int trapIndex = trapFileReader.GetCurrentTrapOrder();
                m_textviewResult.setText("[" + trapIndex + "]\n"
                        + trapText.toString());
            }
        }
    }

    private void ConnectMDM() {
        // Check connection is ready or not. If not, do connection
        if (m_connectState.m_bConnected == false) {
            Log.d(TAG, "Start connection()");
            // create session and setup trap receiver in one operation
            CommandProxyAction openable = new CommandProxyAction();
            openable.actionType = CommandProxyActionType.ACTION_TYPE_ENABLE_MDM;
            m_cmdProxy.ExecuteCommand(openable);
            CommandProxyAction op = new CommandProxyAction();
            op.actionType = CommandProxyActionType.ACTION_TYPE_CREATE_SESSION_AND_SET_TRAP_RECEIVER;
            op.serverName = m_connectState.m_serverName;
            m_cmdProxy.setTrapReceiverInitListener(m_trapReceiverService);
            m_cmdProxy.ExecuteCommand(op);
        }
    }

    private void InitUI() {
        Log.d(TAG, "InitUI()");
        setContentView(R.layout.layout_main);
        m_btnGLOBAL = (Button) findViewById(R.id.btnGLOBAL);
        m_btnGLOBAL.setOnClickListener(new ButtonClickHandler());

        m_btnEM = (Button) findViewById(R.id.btnEM);
        m_btnEM.setOnClickListener(new ButtonClickHandler());

        m_btnOTA = (Button) findViewById(R.id.btnOTA);
        m_btnOTA.setOnClickListener(new ButtonClickHandler());

        m_btnEM.setClickable(false);
        m_btnGLOBAL.setClickable(false);
        m_btnOTA.setClickable(false);
        m_btnEM.setEnabled(false);
        m_btnGLOBAL.setEnabled(false);
        m_btnOTA.setEnabled(false);

        m_progressCircle = (ProgressBar) findViewById(R.id.progressBar_Circle);
        m_progressBar = (ProgressBar) findViewById(R.id.progressBar_Horizontal);
        m_progressCircle.setVisibility(View.INVISIBLE);
        m_progressBar.setVisibility(View.INVISIBLE);
        m_progressBar.setMax(100);

        m_textviewResult = (TextView) findViewById(R.id.textviewResult);
        m_scrollview = ((ScrollView) findViewById(R.id.scrollview));
        m_swtEnable = ((Switch) findViewById(R.id.swEnable));
        m_swtEnable.setClickable(false);
        m_swtEnable.setEnabled(false);
        m_swtEnable
                .setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(CompoundButton buttonView,
                            boolean isChecked) {
                        if (isChecked) {
                            CommandProxyAction op = new CommandProxyAction();
                            op.actionType = CommandProxyActionType.ACTION_TYPE_ENABLE_TRAP;
                            op.sessionID = m_connectState.m_sid;
                            m_cmdProxy.ExecuteCommand(op);
                        } else {
                            CommandProxyAction op = new CommandProxyAction();
                            op.actionType = CommandProxyActionType.ACTION_TYPE_DISABLE_TRAP;
                            op.sessionID = m_connectState.m_sid;
                            m_cmdProxy.ExecuteCommand(op);
                        }
                    }
                });
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        CommandProxyAction op = new CommandProxyAction();
        op.actionType = CommandProxyActionType.ACTION_TYPE_DISABLE_TRAP;
        op.sessionID = m_connectState.m_sid;
        m_cmdProxy.ExecuteCommand(op);

        CommandProxyAction opclose = new CommandProxyAction();
        opclose.actionType = CommandProxyActionType.ACTION_TYPE_CLOSE_SESSION;
        opclose.sessionID = m_connectState.m_sid;
        m_cmdProxy.ExecuteCommand(opclose);

        // Unregister command listener
        m_cmdProxy.UnRegisterCommandProxyListener(hashCode());
        // unbind TrapReceiverService
        if (null != m_trapReceiverService) {
            m_trapReceiverService = null;
        }
        if (null != mConnection) {
            unbindService(mConnection);
            mConnection = null;
        }
    }

    /* decoder */
    private PlainDataDecoder m_plainDataDecoder;

    private void InitDecoder()
    {
        int i;
        // Get decoder object from app object
        AppApplication app = (AppApplication) getApplicationContext();
        m_plainDataDecoder = app.GetPlainDataDecoder();
        if (m_plainDataDecoder == null) {
            Toast.makeText(MainActivity.this, "Please open MDM service first!",
                    Toast.LENGTH_SHORT).show();
            finish();
            return;
        }

        m_GLOBAL_options = m_plainDataDecoder.globalId_getList();
        m_EM_options = m_plainDataDecoder.msgInfo_getMsgList();
        m_OTA_options = m_plainDataDecoder.otaInfo_getMsgList();

        int total_id_size = m_GLOBAL_options.length + m_EM_options.length;
        CommandProxyAction[] subscribeCmdArray = new CommandProxyAction[total_id_size];
        int CmdArrayIndex = 0;


        m_GLOBAL_selections = new boolean[m_GLOBAL_options.length];
        m_GLOBAL_selections_lasttime = new boolean[m_GLOBAL_options.length];
        m_OTA_selections = new boolean[m_OTA_options.length];
        Arrays.fill(m_GLOBAL_selections, true);   // fill true
        Arrays.fill(m_GLOBAL_selections_lasttime, true);   // fill true
        Arrays.fill(m_OTA_selections, false);   // fill false
        m_GLOBAL_id = new long[m_GLOBAL_options.length];
        for (i = 0; i < m_GLOBAL_options.length; ++i) {
            m_GLOBAL_id[i] = m_plainDataDecoder.globalId_getValue(m_GLOBAL_options[i]);
        }
        subscribeCmdArray[CmdArrayIndex] = new CommandProxyAction();
        subscribeCmdArray[CmdArrayIndex].sessionID = m_connectState.m_sid;
        subscribeCmdArray[CmdArrayIndex].trapType = TRAP_TYPE_OTA;
        subscribeCmdArray[CmdArrayIndex].msgIDArray = m_GLOBAL_id; // subscribe all OTA in
        subscribeCmdArray[CmdArrayIndex].actionType = CommandProxyActionType.ACTION_TYPE_SUBSCRIBE_TRAP;
        CmdArrayIndex++;

        m_EM_options = m_plainDataDecoder.msgInfo_getMsgList();
        m_EM_selections = new boolean[m_EM_options.length];
        m_EM_selections_lasttime = new boolean[m_EM_options.length];
        Arrays.fill(m_EM_selections, true);   // fill true
        Arrays.fill(m_EM_selections_lasttime, true);   // fill true
        m_EM_id = new long[m_EM_options.length];
        for (i = 0; i < m_EM_options.length; ++i) {
            m_EM_id[i] = m_plainDataDecoder.msgInfo_getMsgID(m_EM_options[i]);
        }

        subscribeCmdArray[CmdArrayIndex] = new CommandProxyAction();
        subscribeCmdArray[CmdArrayIndex].actionType = CommandProxyActionType.ACTION_TYPE_SUBSCRIBE_TRAP;
        subscribeCmdArray[CmdArrayIndex].sessionID = m_connectState.m_sid;
        subscribeCmdArray[CmdArrayIndex].trapType = TRAP_TYPE_EM;
        subscribeCmdArray[CmdArrayIndex].msgIDArray = m_EM_id;  // subscribe all EM in m_EM_id array
        CmdArrayIndex++;

        // Temp Way: hard code enable all ICD subscription
        ArrayList<Long> recordList = new ArrayList<Long>();
        ArrayList<Long> eventList = new ArrayList<Long>();

        // for record
        recordList.add(0x700AL);
        recordList.add(0x7000L);
        recordList.add(0x7007L);
        recordList.add(0x7008L);
        recordList.add(0x7002L);
        recordList.add(0x700CL);
        recordList.add(0x700EL);
        recordList.add(0x700FL);
        recordList.add(0x700DL);
        recordList.add(0x7010L);
        recordList.add(0x7018L);
        recordList.add(0x700BL);
        recordList.add(0x7004L);
        recordList.add(0x7013L);
        recordList.add(0x7012L);
        recordList.add(0x7011L);
        recordList.add(0x7016L);
        recordList.add(0x7015L);
        recordList.add(0x7014L);
        recordList.add(0x7006L);
        recordList.add(0x7001L);
        recordList.add(0x7009L);
        recordList.add(0x7005L);
        recordList.add(0x7017L);
        recordList.add(0x7003L);
        recordList.add(0x7090L);
        recordList.add(0x7091L);
        recordList.add(0x7083L);
        recordList.add(0x7082L);
        recordList.add(0x7080L);
        recordList.add(0x7081L);
        recordList.add(0x7084L);
        recordList.add(0x7085L);
        recordList.add(0x708DL);
        recordList.add(0x708FL);
        recordList.add(0x708BL);
        recordList.add(0x708CL);
        recordList.add(0x708EL);
        recordList.add(0x7088L);
        recordList.add(0x7089L);
        recordList.add(0x7086L);
        recordList.add(0x7087L);
        recordList.add(0x708AL);
        recordList.add(0x7180L);
        recordList.add(0x7105L);
        recordList.add(0x7103L);
        recordList.add(0x7108L);
        recordList.add(0x7107L);
        recordList.add(0x7106L);
        recordList.add(0x7101L);
        recordList.add(0x7102L);
        recordList.add(0x7100L);
        recordList.add(0x7104L);
        eventList.add(0x7701L);
        eventList.add(0x7700L);
        eventList.add(0x7503L);
        eventList.add(0x7501L);
        eventList.add(0x7502L);
        eventList.add(0x7500L);
        eventList.add(0x758AL);
        eventList.add(0x7589L);
        eventList.add(0x7584L);
        eventList.add(0x7581L);
        eventList.add(0x7582L);
        eventList.add(0x7585L);
        eventList.add(0x7583L);
        eventList.add(0x7580L);
        eventList.add(0x7587L);
        eventList.add(0x7588L);
        eventList.add(0x7586L);
        eventList.add(0x758BL);
        eventList.add(0x7689L);
        eventList.add(0x7686L);
        eventList.add(0x7685L);
        eventList.add(0x7688L);
        eventList.add(0x7687L);
        eventList.add(0x7680L);
        eventList.add(0x7684L);
        eventList.add(0x7681L);
        eventList.add(0x7683L);
        eventList.add(0x7682L);
        eventList.add(0x7605L);
        eventList.add(0x760DL);
        eventList.add(0x760AL);
        eventList.add(0x760CL);
        eventList.add(0x760BL);
        eventList.add(0x7607L);
        eventList.add(0x7609L);
        eventList.add(0x7608L);
        eventList.add(0x760FL);
        eventList.add(0x7601L);
        eventList.add(0x7603L);
        eventList.add(0x7600L);
        eventList.add(0x7604L);
        eventList.add(0x760EL);
        eventList.add(0x7602L);
        eventList.add(0x7606L);

        long[] icd_record_id= recordList.stream().mapToLong(l -> l).toArray();
        long[] icd_event_id= eventList.stream().mapToLong(l -> l).toArray();

        subscribeCmdArray[CmdArrayIndex] = new CommandProxyAction();
        subscribeCmdArray[CmdArrayIndex].actionType = CommandProxyActionType.ACTION_TYPE_SUBSCRIBE_TRAP;
        subscribeCmdArray[CmdArrayIndex].sessionID = m_connectState.m_sid;
        subscribeCmdArray[CmdArrayIndex].trapType = TRAP_TYPE_ICD_EVENT;
        subscribeCmdArray[CmdArrayIndex].msgIDArray = icd_event_id;
        CmdArrayIndex++;

        subscribeCmdArray[CmdArrayIndex] = new CommandProxyAction();
        subscribeCmdArray[CmdArrayIndex].actionType = CommandProxyActionType.ACTION_TYPE_SUBSCRIBE_TRAP;
        subscribeCmdArray[CmdArrayIndex].sessionID = m_connectState.m_sid;
        subscribeCmdArray[CmdArrayIndex].trapType = TRAP_TYPE_ICD_RECORD;
        subscribeCmdArray[CmdArrayIndex].msgIDArray = icd_record_id;
        CmdArrayIndex++;

        // call CommandProxyAsyncTask to operate commands array
        if(m_connectState.m_bSetupDefaultSubscribe == false) {
            m_connectState.m_bSetupDefaultSubscribe = true;
            m_cmdProxy.ExecuteCommand(subscribeCmdArray);
        }

        /* temp code for verifying PSTIME trap type */
        // subscribe TRAP_TYPE_PSTIME
        CommandProxyAction subsPstimeCmd = new CommandProxyAction();
        subsPstimeCmd.actionType = CommandProxyActionType.ACTION_TYPE_SUBSCRIBE_TRAP;
        subsPstimeCmd.sessionID = m_connectState.m_sid;
        subsPstimeCmd.trapType = TRAP_TYPE.TRAP_TYPE_PSTIME;
        subsPstimeCmd.msgID = 0;
        m_cmdProxy.ExecuteCommand(subsPstimeCmd);
        /* temp code for verifying PSTIME trap type */
    }

    /* Trap Info */
    protected String[] m_GLOBAL_options;
    protected boolean[] m_GLOBAL_selections;
    protected boolean[] m_GLOBAL_selections_lasttime;
    protected long[] m_GLOBAL_id;
    protected String[] m_EM_options;
    protected boolean[] m_EM_selections;
    protected boolean[] m_EM_selections_lasttime;
    protected long[] m_EM_id;
    protected String[] m_OTA_options;
    protected boolean[] m_OTA_selections;

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        // noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    /* Button click */
    public class ButtonClickHandler implements View.OnClickListener {
        public void onClick(View view) {
            switch (view.getId()) {
            case R.id.btnEM:
            case R.id.btnGLOBAL:
            case R.id.btnOTA:
                showDialog(view.getId());
                break;
            }
        }
    }

    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case R.id.btnEM:
            return new SubscribeDialog(this, "EM", m_EM_options,
                    m_EM_selections, new DialogButtonClickHandler(TRAP_TYPE_EM))
                    .buildDialog();
        case R.id.btnGLOBAL:
            return new SubscribeDialog(this, "GLOBAL", m_GLOBAL_options,
                    m_GLOBAL_selections, new DialogButtonClickHandler(
                            TRAP_TYPE_OTA)).buildDialog();
        case R.id.btnOTA:
            return new SubscribeDialog(this, "OTA", m_OTA_options,
                    m_OTA_selections, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int clicked) {
                            if (clicked == DialogInterface.BUTTON_POSITIVE) {
                                ArrayList<String> subscribeTraceId = new ArrayList<String>();
                                for (int i = 0; i < m_OTA_selections.length; i++) {
                                    if (m_OTA_selections[i]) {
                                        subscribeTraceId.add(m_OTA_options[i]);
                                    }
                                }
                                int[] traceId = new int[subscribeTraceId.size()];
                                for (int i = 0; i < subscribeTraceId.size(); i++) {
                                    String traceName = subscribeTraceId.get(i);
                                    traceId[i] = m_plainDataDecoder
                                            .otaInfo_getMsgID(traceName);
                                }
                                m_trapReceiverService
                                        .setOtaTraceIdEnable(traceId);
                            }
                        }
                    }).buildDialog();
        default:
            return null;
        }
    }

    public class DialogButtonClickHandler implements
            DialogInterface.OnClickListener {
        private TRAP_TYPE m_type;

        DialogButtonClickHandler(TRAP_TYPE type) {
            m_type = type;
        }

        public void onClick(DialogInterface dialog, int clicked) {
            switch (clicked) {
            case DialogInterface.BUTTON_POSITIVE:
                sendSubscriptionCmd(m_type);
                break;
            }
        }
    }

    protected void sendSubscriptionCmd(TRAP_TYPE type){
        try {
            ArrayList<CommandProxyAction> subscribeCmdArray = new ArrayList<CommandProxyAction>(); // dynamic array to store commands

            if (type == TRAP_TYPE_EM) {
                ArrayList<Long> subList = new ArrayList<Long>();
                ArrayList<Long> unSubList = new ArrayList<Long>();
                for (int i = 0; i < m_EM_selections.length; ++i) {
                    if (m_EM_selections[i] == m_EM_selections_lasttime[i]) {
                        continue;
                    }

                    if (m_EM_selections[i]) { // subscribe
                        // create subscribe command
                        subList.add(m_EM_id[i]);
                    } else {    // unsubscribe
                        // create unsubscribe command
                        unSubList.add(m_EM_id[i]);
                    }
                    m_EM_selections_lasttime[i] = m_EM_selections[i];
                }

                if (subList.size() != 0) {
                    CommandProxyAction command = new CommandProxyAction();
                    command.actionType = CommandProxyActionType.ACTION_TYPE_SUBSCRIBE_TRAP;
                    command.sessionID = m_connectState.m_sid;
                    command.trapType = TRAP_TYPE_EM;
                    command.msgIDArray = subList.stream().mapToLong(l -> l).toArray();
                    subscribeCmdArray.add(command);
                }

                if (unSubList.size() != 0) {
                    CommandProxyAction command = new CommandProxyAction();
                    command.actionType = CommandProxyActionType.ACTION_TYPE_UNSUBSCRIBE_TRAP;
                    command.sessionID = m_connectState.m_sid;
                    command.trapType = TRAP_TYPE_EM;
                    command.msgIDArray = unSubList.stream().mapToLong(l -> l).toArray();
                    subscribeCmdArray.add(command);
                }
            } else if (type == TRAP_TYPE_OTA) {
                ArrayList<Long> subList = new ArrayList<Long>();
                ArrayList<Long> unSubList = new ArrayList<Long>();
                for (int i = 0; i < m_GLOBAL_selections.length; ++i) {
                    if (m_GLOBAL_selections[i] == m_GLOBAL_selections_lasttime[i]) {
                        continue;
                    }
                    if (m_GLOBAL_selections[i]) { // subscribe
                        // create subscribe command
                        subList.add(m_GLOBAL_id[i]);
                    } else {    // unsubscribe
                        // create unsubscribe command
                        unSubList.add(m_GLOBAL_id[i]);
                    }
                    m_GLOBAL_selections_lasttime[i] = m_GLOBAL_selections[i];
                }

                if (subList.size() != 0) {
                    CommandProxyAction command = new CommandProxyAction();
                    command.actionType = CommandProxyActionType.ACTION_TYPE_SUBSCRIBE_TRAP;
                    command.sessionID = m_connectState.m_sid;
                    command.trapType = TRAP_TYPE_OTA;
                    command.msgIDArray = subList.stream().mapToLong(l -> l).toArray();
                    subscribeCmdArray.add(command);
                }

                if (unSubList.size() != 0) {
                    CommandProxyAction command = new CommandProxyAction();
                    command.actionType = CommandProxyActionType.ACTION_TYPE_UNSUBSCRIBE_TRAP;
                    command.sessionID = m_connectState.m_sid;
                    command.trapType = TRAP_TYPE_OTA;
                    command.msgIDArray = unSubList.stream().mapToLong(l -> l).toArray();
                    subscribeCmdArray.add(command);
                }
            }

            // call CommandProxyAsyncTask to operate commands array
            if(subscribeCmdArray.size() > 0) {
                // execute commands package by m_cmdProxy
                m_cmdProxy.ExecuteCommand(subscribeCmdArray.toArray(new CommandProxyAction[subscribeCmdArray.size()]));
            }

        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}
