package com.mediatek.settings.ext;

import android.content.Context;
import android.content.IntentFilter;
import android.view.View;
import android.view.View.OnClickListener;

import androidx.preference.Preference;
import androidx.preference.SwitchPreference;

public interface IDataUsageSummaryExt {

    /**
     * Called when user trying to disabling data
     * @param subId  the sub id been disabling
     * @return true if need handled disabling data by host, false if plug-in handled
     * @internal
     */
    boolean onDisablingData(int subId);

    /**
     * Called when DataUsageSummary need set data switch state such as clickable.
     * @param subId current SIM subId
     * @return true if allow data switch.
     * @internal
     */
    public boolean isAllowDataEnable(int subId);

    /**
     * Called when host bind the view, plug-in should set customized onClickListener and call
     * the passed listener.onClick if necessary
     * @param context context of the host app
     * @param view the view need to set onClickListener
     * @param listener view on click listener
     * @internal
     */
    public void onBindViewHolder(Context context, View view, OnClickListener listener);


    /**
     * Customize when OP07
     * Set summary for mobile data switch.
     * @param p cellDataPreference of mobile data item.
     */
    void setPreferenceSummary(Preference p);

    /**
     * Customize when OPxx
     * Customize intent filter for mobile data switch.
     * @param intentFilter The intentfilter that need custom for OP07 when mobile data switch.
     */
    void customReceiver(IntentFilter intentFilter);

    /**
     * Customize when OPxx
     * Customize intent action for mobile data switch.
     * @param action The intent action that need custom for OP07 when mobile data switch.
     */
    boolean customDualReceiver(String action);

    /**
     * Customize when OPxx
     * Allow to disable data for other subscriptions.
     */
    boolean isAllowDataDisableForOtherSubscription();

    /**
     * custom Tempdata disable or not.
     * @param phoneId phone id.
     */
    boolean customTempdata(int phoneId);

    /**
     * custom Tempdata disable or not.
     */
    boolean customTempdata();

    /**
     * custom Tempdata show or not.
     * @param SwitchPreference tempDataService
     */
    void customTempdataHide(Object preference);

    /**
     * custom Tempdata summary.
     * @param summary String summary
     * @param type summary or alert message
     */
    String customTempDataSummary(String summary, int type);

    /**
     * custom Tempdata dual with broadcast
     * @return true if china mobile feature.
     */
    boolean customizeBroadcastReceiveIntent();

}

