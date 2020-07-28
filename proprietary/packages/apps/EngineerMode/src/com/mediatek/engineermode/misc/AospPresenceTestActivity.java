package com.mediatek.engineermode.misc;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;

import android.app.Activity;
import android.os.Bundle;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.widget.ListView;
import android.widget.EditText;
import android.widget.Button;
import android.widget.Toast;
import android.widget.AdapterView;
import android.view.View.OnClickListener;
import android.view.View;
import android.content.SharedPreferences;
import android.content.Context;

import java.util.List;
import java.util.ArrayList;

public class AospPresenceTestActivity extends Activity {
    public static final String TAG = "AospPresenceTestActivity";
    public static final String FUNCTION_UCE_STARTSERVICE = "[UCE]startService";
    public static final String FUNCTION_UCE_STOPSERVICE = "[UCE]stopService";
    public static final String FUNCTION_UCE_ISSERVICESTARTED = "[UCE]isServiceStarted";
    public static final String FUNCTION_UCE_CREATEOPTIONSSERVICE = "[UCE]createOptionsService";
    public static final String FUNCTION_UCE_DESTROYOPTIONSSERVICE = "[UCE]destroyOptionsService";
    public static final String FUNCTION_UCE_CREATEPRESENCESERVICE = "[UCE]createPresenceService";
    public static final String FUNCTION_UCE_DESTROYPRESENCESERVICE = "[UCE]destroyPresenceService";
    public static final String FUNCTION_UCE_GETSERVICESTATUS = "[UCE]getServiceStatus";
    public static final String FUNCTION_UCE_GETPRESENCESERVICE = "[UCE]getPresenceService";
    public static final String FUNCTION_UCE_GETOPTIONSSERVICE = "[UCE]getOptionsService";
    public static final String FUNCTION_PRESENCE_GETVERSION = "[PRESENCE]getVersion";
    public static final String FUNCTION_PRESENCE_ADDLISTENER = "[PRESENCE]addListener";
    public static final String FUNCTION_PRESENCE_REMOVELISTENER = "[PRESENCE]removeListener";
    public static final String FUNCTION_PRESENCE_REENABLESERVICE = "[PRESENCE]reenableService";
    public static final String FUNCTION_PRESENCE_PUBLISHMYCAP = "[PRESENCE]publishMyCap";
    public static final String FUNCTION_PRESENCE_GETCONTACTCAP = "[PRESENCE]getContactCap";
    public static final String FUNCTION_PRESENCE_GETCONTACTLISTCAP = "[PRESENCE]getContactListCap";
    public static final String FUNCTION_OPTIONS_GETVERSION = "[OPTIONS]getVersion";
    public static final String FUNCTION_OPTIONS_ADDLISTENER = "[OPTIONS]addListener";
    public static final String FUNCTION_OPTIONS_REMOVELISTENER = "[OPTIONS]removeListener";
    public static final String FUNCTION_OPTIONS_SETMYINFO = "[OPTIONS]setMyInfo";
    public static final String FUNCTION_OPTIONS_GETMYINFO = "[OPTIONS]getMyInfo";
    public static final String FUNCTION_OPTIONS_GETCONTACTCAP = "[OPTIONS]getContactCap";
    public static final String FUNCTION_OPTIONS_GETCONTACTLISTCAP = "[OPTIONS]getContactListCap";
    public static final String FUNCTION_OPTIONS_RESPONSEINCOMINGOPTIONS
            = "[OPTIONS]responseIncomingOptions";
    public static final String PREF_NAME = "PhoneNum";
    public static final String PREF_KEY_CONTACT1 = "Contact1PhoneNum";
    public static final String PREF_KEY_CONTACT2 = "Contact2PhoneNum";
    private ListView mListView;
    private EditText mEditTextContact1;
    private EditText mEditTextContact2;
    private Button mButtonContact1;
    private Button mButtonContact2;
    private List<String> mFunctionList;
    private SharedPreferences sharedPreferences;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.aosp_presence_listview);
        mListView = (ListView)findViewById(R.id.Aosp_Presence_ListView);
        mButtonContact1 = (Button) findViewById(R.id.Contact1_Button);
        mButtonContact2 = (Button) findViewById(R.id.Contact2_Button);
        mEditTextContact1 = (EditText) findViewById(R.id.Contact1_EditText);
        mEditTextContact2 = (EditText) findViewById(R.id.Contact2_EditText);
        sharedPreferences = getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
        initClickListener();
        initFunctionList();
        Elog.d(TAG, "onCreate");
        AospPresenceListAdapter adapter = new AospPresenceListAdapter(mFunctionList);
        mListView.setAdapter(adapter);
    }

    private void initFunctionList() {
        mFunctionList = new ArrayList<String>();
        mFunctionList.add(FUNCTION_UCE_STARTSERVICE);
        mFunctionList.add(FUNCTION_UCE_STOPSERVICE);
        mFunctionList.add(FUNCTION_UCE_ISSERVICESTARTED);
        mFunctionList.add(FUNCTION_UCE_CREATEOPTIONSSERVICE);
        mFunctionList.add(FUNCTION_UCE_DESTROYOPTIONSSERVICE);
        mFunctionList.add(FUNCTION_UCE_CREATEPRESENCESERVICE);
        mFunctionList.add(FUNCTION_UCE_DESTROYPRESENCESERVICE);
        mFunctionList.add(FUNCTION_UCE_GETSERVICESTATUS);
        mFunctionList.add(FUNCTION_UCE_GETPRESENCESERVICE);
        mFunctionList.add(FUNCTION_UCE_GETOPTIONSSERVICE);
        mFunctionList.add(FUNCTION_PRESENCE_ADDLISTENER);
        mFunctionList.add(FUNCTION_PRESENCE_REMOVELISTENER);
        mFunctionList.add(FUNCTION_PRESENCE_GETVERSION);
        mFunctionList.add(FUNCTION_PRESENCE_REENABLESERVICE);
        mFunctionList.add(FUNCTION_PRESENCE_PUBLISHMYCAP);
        mFunctionList.add(FUNCTION_PRESENCE_GETCONTACTCAP);
        mFunctionList.add(FUNCTION_PRESENCE_GETCONTACTLISTCAP);
        mFunctionList.add(FUNCTION_OPTIONS_ADDLISTENER);
        mFunctionList.add(FUNCTION_OPTIONS_REMOVELISTENER);
        mFunctionList.add(FUNCTION_OPTIONS_GETVERSION);
        mFunctionList.add(FUNCTION_OPTIONS_SETMYINFO);
        mFunctionList.add(FUNCTION_OPTIONS_GETMYINFO);
        mFunctionList.add(FUNCTION_OPTIONS_GETCONTACTCAP);
        mFunctionList.add(FUNCTION_OPTIONS_GETCONTACTLISTCAP);
        mFunctionList.add(FUNCTION_OPTIONS_RESPONSEINCOMINGOPTIONS);
    }

    private void initClickListener() {
        mButtonContact1.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                String contact1PhoneNum = mEditTextContact1
                        .getText().toString();
                sharedPreferences.edit().putString(
                        PREF_KEY_CONTACT1, contact1PhoneNum).apply();
            }
        });

        mButtonContact2.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                String contact2PhoneNum = mEditTextContact2
                        .getText().toString();
                sharedPreferences.edit().putString(
                        PREF_KEY_CONTACT2, contact2PhoneNum).apply();
            }
        });
    }

}
