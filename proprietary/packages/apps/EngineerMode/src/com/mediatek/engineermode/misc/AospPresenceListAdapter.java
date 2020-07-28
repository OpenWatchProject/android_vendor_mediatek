package com.mediatek.engineermode.misc;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;

import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Button;
import android.widget.BaseAdapter;
import android.content.Context;
import android.content.SharedPreferences;
import android.view.View;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.os.RemoteException;
import android.os.Looper;
import android.os.Handler;
import android.os.Message;
import android.os.Bundle;
import android.telephony.TelephonyManager;


import com.android.ims.internal.uce.uceservice.ImsUceManager;
import com.android.ims.internal.uce.uceservice.IUceService;
import com.android.ims.internal.uce.uceservice.IUceListener;
import com.android.ims.internal.uce.presence.IPresenceService;
import com.android.ims.internal.uce.presence.IPresenceListener;
import com.android.ims.internal.uce.presence.PresPublishTriggerType;
import com.android.ims.internal.uce.presence.PresCmdStatus;
import com.android.ims.internal.uce.presence.PresCapInfo;
import com.android.ims.internal.uce.presence.PresSipResponse;
import com.android.ims.internal.uce.presence.PresTupleInfo;
import com.android.ims.internal.uce.presence.PresResInstanceInfo;
import com.android.ims.internal.uce.presence.PresResInfo;
import com.android.ims.internal.uce.presence.PresRlmiInfo;
import com.android.ims.internal.uce.options.IOptionsService;
import com.android.ims.internal.uce.options.IOptionsListener;
import com.android.ims.internal.uce.options.OptionsSipResponse;
import com.android.ims.internal.uce.options.OptionsCapInfo;
import com.android.ims.internal.uce.options.OptionsCmdStatus;
import com.android.ims.internal.uce.common.StatusCode;
import com.android.ims.internal.uce.common.UceLong;
import com.android.ims.internal.uce.common.CapInfo;

import java.util.List;
import java.lang.StringBuilder;


public class AospPresenceListAdapter extends BaseAdapter implements View.OnClickListener {
    private Context mContext;
    private List<String> mData;
    private ImsUceManager mImsUceManager;
    private IUceService mUceService;
    private IUceListener mUceListener;
    private int mOptionsServiceHandle = -1;
    private int mPresenceServiceHandle = -1;
    private IOptionsService mOptionsService;
    private IPresenceService mPresenceService;
    private IPresenceListener mPresenceListener;
    private IOptionsListener mOptionsListener;
    private UceLong mPresenceServiceListenerHdl;
    private UceLong mOptionsServiceListenerHdl;
    private final int mUserData = 10;
    private final int EVENT_SHOW_TOAST = 0;
    private final int mId = 1;
    private final int mSipResponseCode = 200;
    private final String mReasonPhrase = "no reason phrase";
    private final String TOASTINFO = "toastInfo";
    private String [] mContactUriList = new String[2];
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if(msg.what == EVENT_SHOW_TOAST) {
                String info = msg.getData().getString(TOASTINFO);
                showToast(info);
            }
        }
    };

    private SharedPreferences.OnSharedPreferenceChangeListener prefListener =
            new SharedPreferences.OnSharedPreferenceChangeListener() {
        public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
            String value = prefs.getString(key, "-1");
            showToast("Key changed: " + key);
            if (key.equals(AospPresenceTestActivity.PREF_KEY_CONTACT1)) {
                mContactUriList[0] = "tel:" + value;
            } else if (key.equals(AospPresenceTestActivity.PREF_KEY_CONTACT2)) {
                mContactUriList[1] = "tel:" + value;
            }
        }
    };

    public AospPresenceListAdapter(List<String> data){
        mData = data;
    }

    @Override
    public int getCount() {
        return mData == null ? 0 : mData.size();
    }
    @Override
    public Object getItem(int position) {
        return mData.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View view, ViewGroup viewGroup) {
        ViewHolder viewHolder = null;
        if(mContext == null) {
            mContext = viewGroup.getContext();
            initContactUriList();
            initUce();
        }
        if(view == null){
            view = LayoutInflater.from(viewGroup.getContext()).inflate(
                    R.layout.aosp_presence_listview_item , null);
            viewHolder = new ViewHolder();
            viewHolder.mTv = (TextView)view.findViewById(R.id.mTv);
            viewHolder.mBtn = (Button)view.findViewById(R.id.mBtn);
            view.setTag(viewHolder);
        }

        viewHolder = (ViewHolder)view.getTag();
        viewHolder.mBtn.setText("Test");
        viewHolder.mBtn.setTag(R.id.btn, mData.get(position));
        viewHolder.mBtn.setOnClickListener(this);
        viewHolder.mTv.setText(mData.get(position));
        return view;
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()){
            case R.id.mBtn:
                String apiName = (String) view.getTag(R.id.btn);
                startTestApi(apiName);
                break;
        }
    }

    static class ViewHolder{
        TextView mTv;
        Button mBtn;
    }

    private void initContactUriList() {
        TelephonyManager telephonyManager = TelephonyManager.from(mContext);
        String msisdn = telephonyManager.getMsisdn();
        if (msisdn == null) {
            msisdn = "";
        } else if (msisdn.startsWith("+")) {
            msisdn = msisdn.substring(1);
        }
        SharedPreferences sharedPreferences = mContext.getSharedPreferences(
                AospPresenceTestActivity.PREF_NAME, Context.MODE_PRIVATE);
        String contact1PhoneNum = sharedPreferences.getString(
                AospPresenceTestActivity.PREF_KEY_CONTACT1, msisdn);
        String contact2PhoneNum = sharedPreferences.getString(
                AospPresenceTestActivity.PREF_KEY_CONTACT2, msisdn);
        mContactUriList[0] = "tel:" + contact1PhoneNum;
        mContactUriList[1] = "tel:" + contact2PhoneNum;
        sharedPreferences.registerOnSharedPreferenceChangeListener(prefListener);

    }

    private void initUce() {
        mImsUceManager = ImsUceManager.getInstance(mContext, 0);
        mImsUceManager.createUceService(true);
        mUceService = mImsUceManager.getUceServiceInstance();
        mUceListener = new IUceListener.Stub() {
            @Override
            public void setStatus(int serviceStatusValue) throws RemoteException {
                String info = "SUCCESS!! [setStatus] Service status value = " + serviceStatusValue;
                sendMessageToHandler(info);
            }
        };

        mPresenceListener = new IPresenceListener.Stub() {
            @Override
            public void getVersionCb(String version) {
                String info = "SUCCESS!! [getVersionCb] Presence service version = " + version;
                sendMessageToHandler(info);
            }

            @Override
            public void serviceAvailable(StatusCode statusCode) {
                String info = "SUCCESS!! [serviceAvailable] StatusCode = " + statusCode;
                sendMessageToHandler(info);
            }

            @Override
            public void serviceUnAvailable(StatusCode statusCode) {
                String info = "SUCCESS!! [serviceUnAvailable] StatusCode = " + statusCode;
                sendMessageToHandler(info);
            }

            @Override
            public void publishTriggering(PresPublishTriggerType type) {
                String info = "SUCCESS!! [publishTriggering] Type = "
                        + type.getPublishTrigeerType();
                sendMessageToHandler(info);
            }

            @Override
            public void cmdStatus(PresCmdStatus cmdStatus) {
                String info = "SUCCESS!! [CmdStatus] CmdId = " + cmdStatus.getCmdId().getCmdId()
                        + ", StatusCode = " + cmdStatus.getStatus().getStatusCode()
                        + ", UserData = " + cmdStatus.getUserData()
                        + ", RequestId = " + cmdStatus.getRequestId();
                sendMessageToHandler(info);
            }

            @Override
            public void sipResponseReceived(PresSipResponse response) {
                String info = "SUCCESS!! [sipResponseReceived] CmdId = "
                        + response.getCmdId().getCmdId()
                        + ", RequestId = " + response.getRequestId()
                        + ", SipResponseCode = " + response.getSipResponseCode()
                        + ", RetryAfter time = " + response.getRetryAfter()
                        + ", ReasonPharse = " + response.getReasonPhrase();
                sendMessageToHandler(info);
            }

            @Override
            public void capInfoReceived(String contactUri, PresTupleInfo[] tupleInfo) {
                StringBuilder tupleInfoDetail = new StringBuilder();
                for (int i = 0; i < tupleInfo.length; ++i) {
                    tupleInfoDetail.append("[i] Feature tag = "  + tupleInfo[i].getFeatureTag());
                    tupleInfoDetail.append("[i] Contact uri in tupleInfo = "
                            + tupleInfo[i].getContactUri());
                    tupleInfoDetail.append("[i] Timestamp = " + tupleInfo[i].getTimestamp());
                }
                String info = "SUCCESS!! [capInfoReceived] Contact uri = " + contactUri
                        + tupleInfoDetail.toString();
                sendMessageToHandler(info);
            }

            @Override
            public void listCapInfoReceived(PresRlmiInfo rlmiInfo, PresResInfo[] resInfo) {
                StringBuilder resInfoDetail = new StringBuilder();
                for (int i = 0; i < resInfo.length; ++i) {
                    resInfoDetail.append("[" + i + "] ResUri = " + resInfo[i].getResUri());
                    resInfoDetail.append("[" + i + "] DisPlayName = "
                            + resInfo[i].getDisplayName());
                    resInfoDetail.append("[" + i + "] ResInstanceState = "
                            + resInfo[i].getInstanceInfo().getResInstanceState());
                    resInfoDetail.append("[" + i + "] ResId = "
                            + resInfo[i].getInstanceInfo().getResId());
                    resInfoDetail.append("[" + i + "] Reason = "
                            + resInfo[i].getInstanceInfo().getReason());
                    resInfoDetail.append("[" + i + "] Uri = "
                            + resInfo[i].getInstanceInfo().getPresentityUri());
                    PresTupleInfo[] tupleInfo = resInfo[i].getInstanceInfo().getTupleInfo();
                    for (int j = 0; j < tupleInfo.length; ++i) {
                        resInfoDetail.append("[" + i + "][" + j + "] Feature tag = "
                                + tupleInfo[j].getFeatureTag());
                        resInfoDetail.append("[" + i + "][" + j + "] Contact uri in tupleInfo = "
                                + tupleInfo[j].getContactUri());
                        resInfoDetail.append("[" + i + "][" + j + "] Timestamp = "
                                + tupleInfo[j].getTimestamp());
                    }
                }

                String info = "SUCCESS!! [listCapInfoReceived]  Rlmi uri = " + rlmiInfo.getUri()
                        + ", Version = " + rlmiInfo.getVersion()
                        + ", FullState = " + rlmiInfo.isFullState()
                        + ", ListName = " + rlmiInfo.getListName()
                        + ", RequestId = " + rlmiInfo.getRequestId()
                        + ", SubscrptionState = "
                        + rlmiInfo.getPresSubscriptionState().getPresSubscriptionStateValue()
                        + ", SubscriptionExpireTime = " + rlmiInfo.getSubscriptionExpireTime()
                        + ", SubscriptionTerminatedReason = "
                        + rlmiInfo.getSubscriptionTerminatedReason()
                        + resInfoDetail.toString();
                sendMessageToHandler(info);
            }

            @Override
            public void unpublishMessageSent() {
                String info = "SUCCESS!! [unpublishMessageSent] ";
                sendMessageToHandler(info);
            }
        };

        mOptionsListener = new IOptionsListener.Stub() {
            @Override
            public void getVersionCb(String version) {
                String info = "SUCCESS!! [getVersionCb] Options service version = " + version;
                sendMessageToHandler(info);
            }

            @Override
            public void serviceAvailable(StatusCode statusCode) {
                String info = "SUCCESS!! [serviceAvailable] StatusCode = " + statusCode;
                sendMessageToHandler(info);
            }

            @Override
            public void serviceUnavailable(StatusCode statusCode) {
                String info = "SUCCESS!! [serviceUnAvailable] StatusCode = " + statusCode;
                sendMessageToHandler(info);
            }

            @Override
            public void sipResponseReceived(String uri, OptionsSipResponse response,
                    OptionsCapInfo optionsCapInfo) {
                String info = "SUCCESS!! [sipResponseReceived] CmdId = "
                        + response.getCmdId().getCmdId()
                        + ", RequestId = " + response.getRequestId()
                        + ", SipResponseCode = " + response.getSipResponseCode()
                        + ", RetryAfter time = " + response.getRetryAfter()
                        + ", ReasonPharse = " + response.getReasonPhrase()
                        + ", sdp = " + optionsCapInfo.getSdp();
                info += transferCapInfoToString(optionsCapInfo.getCapInfo());
                sendMessageToHandler(info);
            }

            @Override
            public void cmdStatus(OptionsCmdStatus cmdStatus) {
                String info = "SUCCESS!! [CmdStatus] CmdId = " + cmdStatus.getCmdId().getCmdId()
                        + ", StatusCode = " + cmdStatus.getStatus().getStatusCode()
                        + ", UserData = " + cmdStatus.getUserData();
                info += transferCapInfoToString(cmdStatus.getCapInfo());
                sendMessageToHandler(info);
            }

            @Override public void incomingOptions(String uri,
                    OptionsCapInfo optionsCapInfo, int tId) {
                String info = "SUCCESS!! [incomingOptions] uri = " + uri
                        + ", tId = " + tId;
                info += transferCapInfoToString(optionsCapInfo.getCapInfo());
                sendMessageToHandler(info);
            }
        };

    }

    private void startTestApi(String apiName) {
        switch (apiName) {
            case AospPresenceTestActivity.FUNCTION_UCE_STARTSERVICE:
                testUceStartService();
                break;
            case AospPresenceTestActivity.FUNCTION_UCE_STOPSERVICE:
                testUceStopService();
                break;
            case AospPresenceTestActivity.FUNCTION_UCE_ISSERVICESTARTED:
                testUceIsServiceStarted();
                break;
            case AospPresenceTestActivity.FUNCTION_UCE_CREATEOPTIONSSERVICE:
                testUceCreateOptionsService();
                break;
            case AospPresenceTestActivity.FUNCTION_UCE_DESTROYOPTIONSSERVICE:
                testUceDestroyOptionsService();
                break;
            case AospPresenceTestActivity.FUNCTION_UCE_CREATEPRESENCESERVICE:
                testUceCreatePresenceService();
                break;
            case AospPresenceTestActivity.FUNCTION_UCE_DESTROYPRESENCESERVICE:
                testUceDestroyPresenceService();
                break;
            case AospPresenceTestActivity.FUNCTION_UCE_GETSERVICESTATUS:
                testUceGetServiceStatus();
                break;
            case AospPresenceTestActivity.FUNCTION_UCE_GETPRESENCESERVICE:
                testUceGetPresenceService();
                break;
            case AospPresenceTestActivity.FUNCTION_UCE_GETOPTIONSSERVICE:
                testUceGetOptionsService();
                break;
            case AospPresenceTestActivity.FUNCTION_PRESENCE_GETVERSION:
                testPresenceGetVersion();
                break;
            case AospPresenceTestActivity.FUNCTION_PRESENCE_ADDLISTENER:
                testPresenceAddLitener();
                break;
            case AospPresenceTestActivity.FUNCTION_PRESENCE_REMOVELISTENER:
                testPresenceRemoveLitener();
                break;
            case AospPresenceTestActivity.FUNCTION_PRESENCE_REENABLESERVICE:
                testPresenceReenableService();
                break;
            case AospPresenceTestActivity.FUNCTION_PRESENCE_PUBLISHMYCAP:
                testPresencePublshMyCap();
                break;
            case AospPresenceTestActivity.FUNCTION_PRESENCE_GETCONTACTCAP:
                testPresenceGetContactCap();
                break;
            case AospPresenceTestActivity.FUNCTION_PRESENCE_GETCONTACTLISTCAP:
                testPresenceGetContactListCap();
                break;
            case AospPresenceTestActivity.FUNCTION_OPTIONS_GETVERSION:
                testOptionsGetVersion();
                break;
            case AospPresenceTestActivity.FUNCTION_OPTIONS_ADDLISTENER:
                testOptionsAddListener();
                break;
            case AospPresenceTestActivity.FUNCTION_OPTIONS_REMOVELISTENER:
                testOptionsRemoveListener();
                break;
            case AospPresenceTestActivity.FUNCTION_OPTIONS_SETMYINFO:
                testOptionsSetMyInfo();
                break;
            case AospPresenceTestActivity.FUNCTION_OPTIONS_GETMYINFO:
                testOptionsGetMyInfo();
                break;
            case AospPresenceTestActivity.FUNCTION_OPTIONS_GETCONTACTCAP:
                testOptionsGetContactCap();
                break;
            case AospPresenceTestActivity.FUNCTION_OPTIONS_GETCONTACTLISTCAP:
                testOptionsGetContactListCap();
                break;
            case AospPresenceTestActivity.FUNCTION_OPTIONS_RESPONSEINCOMINGOPTIONS:
                testOptionsResponseIncomingOptions();
                break;
        }
    }

    private void testUceStartService() {
        try {
            boolean ret = mUceService.startService(mUceListener);
            if (ret) {
                showToast("SUCCESS!! [startService]");
            } else {
                showToast("FAIL!! [startService]");
            }
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testUceStopService() {
        try {
            boolean ret = mUceService.stopService();
            if (ret) {
                showToast("SUCCESS!! [stopService]");
            } else {
                showToast("FAIL!! [stopService]");
            }
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testUceIsServiceStarted() {
        try {
            boolean ret = mUceService.isServiceStarted();
            showToast("SUCCESS!! [isServiceStarted] ret = " + ret);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testUceCreateOptionsService() {
        try {
            mOptionsServiceHandle = mUceService.createOptionsService(null, null);
            showToast("SUCCESS!! [createOptionsService]  ret = " + mOptionsServiceHandle);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testUceDestroyOptionsService() {
        try {
            mUceService.destroyOptionsService(mOptionsServiceHandle);
            showToast("SUCCESS!! [destroyOptionsService]");
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testUceCreatePresenceService() {
        try {
            mPresenceServiceHandle = mUceService.createPresenceService(null, null);
            showToast("SUCCESS!! [createPresenceService] presenceServiceHandle = "
                    + mPresenceServiceHandle);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testUceDestroyPresenceService() {
        try {
            mUceService.destroyPresenceService(mOptionsServiceHandle);
            showToast( "SUCCESS!! [destroyPresenceService] ");
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testUceGetServiceStatus() {
        try {
            boolean ret = mUceService.getServiceStatus();
            showToast("SUCCESS!! [getServiceStatus] ret = " + ret);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testUceGetPresenceService() {
        try {
            mPresenceService = mUceService.getPresenceService();
            if (mPresenceService != null) {
                showToast("SUCCESS!! [GetPresenceService] mPresenceService is not null");
            } else {
                showToast("SUCCESS!! [GetPresenceService] mPresenceService is null");
            }
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testUceGetOptionsService() {
        try {
            mOptionsService = mUceService.getOptionsService();
            if (mOptionsService != null) {
                showToast("SUCCESS!! [GetOptionsService] mOptionsService is not null");
            } else {
                showToast("SUCCESS!! [GetOptionsService] mOptionsService is null");
            }
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testPresenceAddLitener() {
        try {
            if (mPresenceService == null) {
                showToast("FAIL!! Please Step1. Click createPresenceService\n"
                        + "Step2. Click getPresenceService");
                return;
            }
            mPresenceServiceListenerHdl = new UceLong();
            mPresenceServiceListenerHdl.setUceLong(100);
            StatusCode ret = mPresenceService.addListener(mPresenceServiceHandle,
                    mPresenceListener, mPresenceServiceListenerHdl);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("AddListener", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testPresenceRemoveLitener() {
        try {
            if (mPresenceService == null) {
                showToast("FAIL!! Please Step1. Click createPresenceService\n"
                        + "Step2. Click getPresenceService");
                return;
            }
            StatusCode ret = mPresenceService.removeListener(mPresenceServiceHandle,
                    mPresenceServiceListenerHdl);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("RemoveListener", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testPresenceGetVersion() {
        try {
            if (mPresenceService == null) {
                showToast("FAIL!! Please Step1. Click createPresenceService\n"
                        + "Step2. Click getPresenceService");
                return;
            }
            StatusCode ret = mPresenceService.getVersion(mPresenceServiceHandle);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("GetVersion", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testPresenceReenableService() {
        try {
            if (mPresenceService == null) {
                showToast("FAIL!! Please Step1. Click createPresenceService\n"
                        + "Step2. Click getPresenceService");
                return;
            }
            StatusCode ret = mPresenceService.reenableService(mPresenceServiceHandle, mUserData);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("ReenableService", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testPresencePublshMyCap() {
        try {
            if (mPresenceService == null) {
                showToast("FAIL!! Please Step1. Click createPresenceService\n"
                        + "Step2. Click getPresenceService");
                return;
            }
            PresCapInfo presCapInfo = createPresCapInfo();
            StatusCode ret = mPresenceService.publishMyCap(mPresenceServiceHandle,
                    presCapInfo, mUserData);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("PublishMyCap", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testPresenceGetContactCap() {
        try {
            if (mPresenceService == null) {
                showToast("FAIL!! Please Step1. Click createPresenceService\n"
                        + "Step2. Click getPresenceService");
                return;
            }
            StatusCode ret = mPresenceService.getContactCap(mPresenceServiceHandle,
                    mContactUriList[0], mUserData);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("GetContactCap", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testPresenceGetContactListCap() {
        try {
            if (mPresenceService == null) {
                showToast("FAIL!! Please Step1. Click createPresenceService\n"
                        + "Step2. Click getPresenceService");
                return;
            }
            StatusCode ret = mPresenceService.getContactListCap(mPresenceServiceHandle,
                    mContactUriList, mUserData);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("GetContactListCap", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testOptionsAddListener() {
        try {
            if (mOptionsService == null) {
                showToast("FAIL!! Please Step1. Click createOptionsService\n"
                        + "Step2. Click getOptionsService");
                return;
            }
            mOptionsServiceListenerHdl = new UceLong();
            mOptionsServiceListenerHdl.setUceLong(100);
            StatusCode ret = mOptionsService.addListener(mOptionsServiceHandle,
                    mOptionsListener, mOptionsServiceListenerHdl);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("AddListener", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testOptionsRemoveListener() {
        try {
            if (mOptionsService == null) {
                showToast("FAIL!! Please Step1. Click createOptionsService\n"
                        + "Step2. Click getOptionsService");
                return;
            }
            StatusCode ret = mOptionsService.removeListener(mOptionsServiceHandle,
                    mOptionsServiceListenerHdl);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("RemoveListener", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testOptionsGetVersion() {
        try {
            if (mOptionsService == null) {
                showToast("FAIL!! Please Step1. Click createOptionsService\n"
                        + "Step2. Click getOptionsService");
                return;
            }
            StatusCode ret = mOptionsService.getVersion(mOptionsServiceHandle);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("GetVersion", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testOptionsSetMyInfo() {
        try {
            if (mOptionsService == null) {
                showToast("FAIL!! Please Step1. Click createOptionsService\n"
                        + "Step2. Click getOptionsService");
                return;
            }
            CapInfo capInfo = new CapInfo();
            StatusCode ret = mOptionsService.setMyInfo(mOptionsServiceHandle, capInfo, mUserData);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("SetMyInfo", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testOptionsGetMyInfo() {
        try {
            if (mOptionsService == null) {
                showToast("FAIL!! Please Step1. Click createOptionsService\n"
                        + "Step2. Click getOptionsService");
                return;
            }
            CapInfo capInfo = new CapInfo();
            StatusCode ret = mOptionsService.getMyInfo(mOptionsServiceHandle, mUserData);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("GetMyInfo", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testOptionsGetContactCap() {
        try {
            if (mOptionsService == null) {
                showToast("FAIL!! Please Step1. Click createOptionsService\n"
                        + "Step2. Click getOptionsService");
                return;
            }
            StatusCode ret = mOptionsService.getContactCap(mOptionsServiceHandle,
                    mContactUriList[0], mUserData);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("GetContactCap", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testOptionsGetContactListCap() {
        try {
            if (mOptionsService == null) {
                showToast("FAIL!! Please Step1. Click createOptionsService\n"
                        + "Step2. Click getOptionsService");
                return;
            }
            StatusCode ret = mOptionsService.getContactListCap(mOptionsServiceHandle,
                    mContactUriList, mUserData);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("GetContactListCap", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private void testOptionsResponseIncomingOptions() {
        try {
            if (mOptionsService == null) {
                showToast("FAIL!! Please Step1. Click createOptionsService\n"
                        + "Step2. Click getOptionsService");
                return;
            }
            OptionsCapInfo optionsCapInfo = createOptionsCapInfo();
            StatusCode ret = mOptionsService.responseIncomingOptions(mOptionsServiceHandle, mId,
                    mSipResponseCode, mReasonPhrase, optionsCapInfo, false);
            int statusCode = ret.getStatusCode();
            showToastWithStatusCode("ResponseIncomingOptions", statusCode);
        } catch (RemoteException e) {
            showToast("FAIL!! The connection between client and server is lost");
        }
    }

    private PresCapInfo createPresCapInfo() {
        PresCapInfo presCapInfo = new PresCapInfo();
        CapInfo capInfo = new CapInfo();
        presCapInfo.setCapInfo(capInfo);
        presCapInfo.setContactUri("test");
        return presCapInfo;
    }

    private OptionsCapInfo createOptionsCapInfo() {
        OptionsCapInfo optionsCapInfo = new OptionsCapInfo();
        CapInfo capInfo = new CapInfo();
        optionsCapInfo.setSdp("test");
        optionsCapInfo.setCapInfo(capInfo);
        return optionsCapInfo;
    }

    private void showToast(String info) {
        Toast.makeText(mContext, info, Toast.LENGTH_SHORT).show();
    }

    private void showToastWithStatusCode(String functionName, int statusCode) {
        if (statusCode == StatusCode.UCE_SUCCESS) {
            Toast.makeText(mContext, "SUCCESS!! [ " + functionName + "] statusCode = " + statusCode,
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(mContext, "FAIL!! [ " + functionName + "] statusCode = " + statusCode,
                    Toast.LENGTH_SHORT).show();
        }
    }

    private void sendMessageToHandler(String info) {
        Message msg = new Message();
        msg.what = EVENT_SHOW_TOAST;
        Bundle bundle = new Bundle();
        bundle.putString(TOASTINFO, info);
        msg.setData(bundle);
        mHandler.sendMessage(msg);
    }

    private String transferCapInfoToString(CapInfo capInfo) {
        String info = ", isImsSupported = " + capInfo.isImSupported()
                + ", isFtThumbSupported = " + capInfo.isFtThumbSupported()
                + ", isFtSnFSupported = " + capInfo.isFtSnFSupported()
                + ", isFtHttpSupported = " + capInfo.isFtHttpSupported()
                + ", isFtSupported = " + capInfo.isFtSupported()
                + ", isIsSupported = " + capInfo.isIsSupported()
                + ", isVsDuringCSSupported = " + capInfo.isVsDuringCSSupported()
                + ", isVsSupported = " + capInfo.isVsSupported()
                + ", isSpSupported = " + capInfo.isSpSupported()
                + ", isCdViaPresenceSupported = " + capInfo.isCdViaPresenceSupported()
                + ", isIpVideoSupported = " + capInfo.isIpVideoSupported()
                + ", isGeoPullFtSupported = " + capInfo.isGeoPullFtSupported()
                + ", isGeoPullSupported = " + capInfo.isGeoPullSupported()
                + ", isGeoPushSupported = " + capInfo.isGeoPushSupported()
                + ", isSmSupported = " + capInfo.isSmSupported()
                + ", isFullSnFGroupChatSupported = " + capInfo.isFullSnFGroupChatSupported()
                + ", isRcsIpVoiceCallSupported = " + capInfo.isRcsIpVoiceCallSupported()
                + ", isRcsIpVideoCallSupported = " + capInfo.isRcsIpVideoCallSupported()
                + ", isRcsIpVideoOnlyCallSupported = " + capInfo.isRcsIpVideoOnlyCallSupported()
                + ", timestamp = " + capInfo.getCapTimestamp();
        String[] extensions = capInfo.getExts();
        for (int i = 0; i < extensions.length; ++i) {
            info += ",";
            info += extensions[i];
        }
        return info;
    }
}

