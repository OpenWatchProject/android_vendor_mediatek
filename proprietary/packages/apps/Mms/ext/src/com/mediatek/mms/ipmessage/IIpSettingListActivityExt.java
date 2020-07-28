package com.mediatek.mms.ipmessage;

import android.app.ListActivity;


public interface IIpSettingListActivityExt {

    /**
     * called on onCreate
     * @param activity: this activity
     * @return boolean
     * @internal
     */
    public boolean onIpCreate(ListActivity activity);

    /**
     * called on isNeedUpdateView
     * @param needUpdate: is needUpdate
     * @return boolean: is need update
     * @internal
     */
    public boolean isIpNeedUpdateView(boolean needUpdate);

    /**
     * called on setAdapter
     * @param settingList: setting list
     * @return boolean
     * @internal
     */
    public String[] setIpAdapter(String[] settingList);

    /**
     * called on mUpdateViewStateHandler
     * @return boolean
     * @internal
     */
    public boolean handleIpMessage();

    /**
     * called on onListItemClick
     * @param position: position
     * @return boolean
     * @internal
     */
    public boolean onIpListItemClick(int position);
}
