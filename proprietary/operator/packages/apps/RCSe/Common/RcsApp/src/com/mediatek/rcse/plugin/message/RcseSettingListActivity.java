package com.mediatek.rcse.plugin.message;

import android.app.ListActivity;
import android.content.Intent;
import android.content.res.Resources;
import android.text.TextUtils;
import android.view.View;

import com.mediatek.mms.ipmessage.DefaultIpSettingListActivityExt;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcs.R;

public class RcseSettingListActivity extends DefaultIpSettingListActivityExt {
    private static String TAG = "RcseSettingListActivity";
    
    private boolean mIsSupportIpMsg = false;
    private int mIpMsgState = -1;
    private boolean mIsWithIpMsg = false;
    private ListActivity mContext;
    
    @Override
    public boolean onIpCreate(ListActivity activity) {
        mContext = activity;
        mIsSupportIpMsg = IpMessageServiceMananger.getInstance(mContext).isFeatureSupported(
                IpMessageConsts.FeatureId.APP_SETTINGS);
        Logger.d(TAG, "onCreate mIsSupportIpMsg " + mIsSupportIpMsg);
        return true;
    }

    @Override
    public boolean isIpNeedUpdateView(boolean needUpdate) {
        int ipMsgState = -1;
        if (mIsSupportIpMsg) {
            ipMsgState = IpMessageServiceMananger.getInstance(mContext).getDisableServiceStatus();
        }
        if (ipMsgState != mIpMsgState) {
            mIpMsgState = ipMsgState;
            return true;
        }
        Logger.d(TAG, "isNeedUpdateView needUpdate: " + needUpdate + "  ipMsgState " + ipMsgState);
        return needUpdate;
    }

    @Override
    public String[] setIpAdapter(String[] settingList) {
        boolean isIpDisablePermanent = (mIpMsgState == IpMessageConsts.DisableServiceStatus.DISABLE_PERMANENTLY);
        if (mIsSupportIpMsg && !isIpDisablePermanent) {
            String strIpMsg = IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_ip_message);
            if (TextUtils.isEmpty(strIpMsg)) {
                strIpMsg = " ";
            }
            mIsWithIpMsg = true;
            String[] ipSettingList = new String[5];
            for (int i = 0; i < settingList.length; i++) {
                ipSettingList[i] = settingList[i];
            }
            ipSettingList[4] = strIpMsg;
            return ipSettingList;
        } else {
            mIsWithIpMsg = false;
        }
        return settingList;
    }

    @Override
    public boolean handleIpMessage() {
        if (mIsWithIpMsg && mIpMsgState == IpMessageConsts.DisableServiceStatus.DISABLE_TEMPORARY) {
            View ipMsgView = (View) mContext.getListView().getChildAt(1);
            ipMsgView.setEnabled(false);
            Logger.d(TAG, " mUpdateViewStateHandler set ipMsgView disabled");
            return true;
        }
        return false;
    }

    @Override
    public boolean onIpListItemClick(int position) {
        if (mIsWithIpMsg) {
            switch (position) {
                case 5:
                    Intent systemSettingsIntent = new Intent(
                            IpMessageConsts.RemoteActivities.SYSTEM_SETTINGS);
                    IpMessageUtils.startRemoteActivity(mContext, systemSettingsIntent);
                    return true;
            }
        }
        return false;
    }
}
