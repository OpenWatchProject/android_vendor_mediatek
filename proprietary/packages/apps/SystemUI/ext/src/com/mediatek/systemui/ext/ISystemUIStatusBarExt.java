package com.mediatek.systemui.ext;

import android.content.Context;
import android.content.res.ColorStateList;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

/**
 * M: the interface for Plug-in definition of Status bar.
 */
public interface ISystemUIStatusBarExt {

    /**
     * disable host function, op does not need.
     * @return whether disable common function.
     */
    boolean disableHostFunction();

    /**
     * check if slot index has changed.
     * @param subId the sub id of SIM.
     * @param slotId the slot index of SIM.
     * @return whether slot id has changed or not in onSubscriptionsChanged callback flow.
     */
    @Deprecated
    boolean checkIfSlotIdChanged(int subId, int slotId);

    /**
     * check data status is disable or not.
     * @param subId the sub id of SIM.
     * @param dataDisabled the data disabled status.
     */
    void isDataDisabled(int subId, boolean dataDisabled);

    /**
     * Get the current service state for op customized view update.
     * @param subId the sub id of SIM.
     * @internal
     */
    void getServiceStateForCustomizedView(int subId);

    /**
     * Get the cs state from ss and add to mobileState. so that if cs state has changed,
     * it will call op update flow to update the network tower icon.
     * @param serviceState the current service state.
     * @param state the default cs state which will return.
     * @return the customized cs register state.
     * @internal
     */
    int getCustomizeCsState(ServiceState serviceState, int state);

    /**
     * Check current state is in cs call or not. if state changed need notify update icon.
     * @return whether in cs call or not.
     */
    boolean isInCsCall();

    /**
     * Get the customized network type icon id.
     * @param subId the sub id of SIM.
     * @param iconId the original network type icon id.
     * @param networkType the network type.
     * @param serviceState the service state.
     * @return the customized network type icon id.
     * @internal
     */
    int getNetworkTypeIcon(int subId, int iconId, int networkType,
            ServiceState serviceState);

    /**
     * Get the customized data type icon id.
     * @param subId the sub id of SIM.
     * @param iconId the original data type icon id.
     * @param dataType the data connection type.
     * @param dataState the data connection state.
     * @param serviceState the service state.
     * @return the customized data type icon id.
     * @internal
     */
    int getDataTypeIcon(int subId, int iconId, int dataType, int dataState,
            ServiceState serviceState);

    /**
     * Get the customized signal strength icon id.
     * @param subId the sub id of SIM.
     * @param iconId the original signal strength icon id.
     * @param signalStrength the signal strength.
     * @param networkType the network type.
     * @param serviceState the service state.
     * @return the customized signal strength icon id.
     * @internal
     */
    int getCustomizeSignalStrengthIcon(int subId, int iconId,
            SignalStrength signalStrength, int networkType,
            ServiceState serviceState);

    /**
     * Get the customized signal strength level.
     * @param signalLevel the original signal strength level.
     * @param signalStrength the signal strength.
     * @param serviceState the service state.
     * @return the customized signal strength level.
     * @internal
     */
    int getCustomizeSignalStrengthLevel(int signalLevel,
            SignalStrength signalStrength, ServiceState serviceState);

    /**
     * Add the customized view.
     * @param subId the sub id of SIM.
     * @param context the context.
     * @param root the root view group in which the customized view
     *             will be added.
     * @internal
     */
    void addCustomizedView(int subId, Context context, ViewGroup root);

    /**
     * Add the customized view in signal cluster view.
     * @param context the context.
     * @param root the root view group in which the customized view
     *             will be added.
     * @param index the add index.
     * @internal
     */
    @Deprecated
    void addSignalClusterCustomizedView(Context context, ViewGroup root, int index);

    /**
     * Set the customized network type view.
     * @param subId the sub id of SIM.
     * @param networkTypeId the customized network type icon id.
     * @param networkTypeView the network type view
     *                        which needs to be customized.
     * @internal
     */
    void setCustomizedNetworkTypeView(int subId,
            int networkTypeId, ImageView networkTypeView);

    /**
     * Set the customized data type view.
     * @param subId the sub id of SIM.
     * @param dataTypeId the customized data type icon id.
     * @param dataIn the data in state.
     * @param dataOut the data out state.
     * @internal
     */
    void setCustomizedDataTypeView(int subId,
            int dataTypeId, boolean dataIn, boolean dataOut);

    /**
     * Set the customized mobile type view.
     * @param subId the sub id of SIM.
     * @param mobileTypeView the mobile type view which needs to be customized.
     * @internal
     */
    void setCustomizedMobileTypeView(int subId, ImageView mobileTypeView);

    /**
     * Set the customized signal strength view.
     * @param subId the sub id of SIM.
     * @param signalStrengthId the customized signal strength icon id.
     * @param signalStrengthView the signal strength view
     *                           which needs to be customized.
     * @internal
     */
    void setCustomizedSignalStrengthView(int subId,
            int signalStrengthId, ImageView signalStrengthView);

    /**
     * get the common signal strength res id for op context.
     * @param subId the sub id of SIM.
     * @param mobileSignalIconId the op customized signal strength res id.
     * @return the common signal strength res id.
     */
    int getCommonSignalIconId(int subId, int mobileSignalIconId);

    /**
     * Set the Host ImageView to invisible which operator no need to show.
     * @param view the host image view.
     */
    void SetHostViewInvisible(ImageView view);

    /**
     * Set the Ims register info to op context.
     * @param subId the sub id of ims register SIM.
     * @param imsRegState the sim card ims register status.
     * @param isOverwfc whether ims register over wifi.
     * @param isImsOverVoice whether ims register over Voice.
     */
    void setImsRegInfo(int subId, int imsRegState, boolean isOverwfc, boolean isImsOverVoice);

    /**
     * Set the disconnect volte view.
     * @param subId the sim card which will to show view.
     * @param iconId the original volte icon id.
     * @param volteView the volte view which needs to be customized.
     */
    void setDisVolteView(int subId, int iconId, ImageView volteView);

    /**
     * Set the other customized views.
     * @param subId the sub id of SIM.
     * @internal
     */
    void setCustomizedView(int subId);

    /**
     * Set the customized no sim view.
     * @param noSimsVisible whether show nosim or not.
     * @internal
     */
    void setCustomizedNoSimView(boolean noSimsVisible);

    /**
     * Set the customized no sim view.
     * @param noSimView the no sim view which needs to be customized.
     * @internal
     */
    @Deprecated
    void setCustomizedNoSimView(ImageView noSimView);

    /**
     * Set the customized volte view.
     * @param iconId the original volte icon id.
     * @param volteView the volte view which needs to be customized.
     * @internal
     */
    void setCustomizedVolteView(int iconId, ImageView volteView);

    /**
     * Set the customized no sim and airplane mode view.
     * @param noSimView the no sim view which needs to be customized.
     * @param airplaneMode the airplane mode.
     * @internal
     */
    @Deprecated
    void setCustomizedAirplaneView(View noSimView, boolean airplaneMode);

    /**
     * Set the customized noSimsVisible.
     * @param noSimsVisible the noSims visible or not.
     * @internal
     */
    @Deprecated
    void setCustomizedNoSimsVisible(boolean noSimsVisible);

    /**
     * To remove network icons in case of wifi only mode for WFC.
     * @param serviceState the current service state.
     * @return whether in serive or not - false for iWLAN
     * @internal
     */
    boolean updateSignalStrengthWifiOnlyMode(ServiceState serviceState,
                boolean connected);

    /**
     * To register op phone state listener.
     * @internal
     */
    void registerOpStateListener();

   /**
     * Set IconTint for op added views.
     * @param tint the tint value.
     * @param darkIntensity the darkIntensity value.
     */
    @Deprecated
    void setIconTint(int tint, float darkIntensity);

    /**
     * Set IconTint for op added views.
     * @param tint the tint to apply
     */
    void setIconTint(ColorStateList tint);

    @Deprecated
    void setNoSimIconTint(int tint, ImageView nosim);
    /**
     * Set the if the sim is inserted.
     * @param slotId the slot id.
     * @param insert the insert status.
     * @internal
     */
    public void setSimInserted(int slotId, boolean insert);

    /**
     * save slot id for IMS to update wfc icon.
     * @internal
     */
    void setImsSlotId(final int slotId);

    /**
      * Set setCustomizedPlmnTextTint for op added views.
      * @param tint the tint value.
      */
     public void setCustomizedPlmnTextTint(int tint);

     /**
      * When in c2k cs call, need set nw icon as 2G because md can not ps fall back.
      * @param subId the sub id.
      * @param state the call state in this sub.
      * @param incomingNumber the call incomingNumber.
      * @param serviceState the current servicestate.
      * @return whether need to handle this call state change or not.
      */
     public boolean handleCallStateChanged(int subId, int state, String incomingNumber,
             ServiceState serviceState);

    /**
      * Set needShowRoamingIcons need to show roaming icon.
      * @param isInRoaming is in roaming.
      * @return whether need to show roaming or not.
      */
    public boolean needShowRoamingIcons(boolean isInRoaming);

    /**
     * Customize style to display 12 hr format am/pm text .
     * NORMAL = 0, SMALL = 1, GONE = 2.
     * @param defaultAmPmStyle default style being used by host app.
     * @return customized style
     */
    int getClockAmPmStyle(int defaultAmPmStyle);

    /**
     * Check if need to show WFC icon.
     * Shown by default, for specific optr shown by wfc settings app
     */
    boolean needShowWfcIcon();

    /**
     * Add StatusBarCallback to plugin
     * Can use this callback to set system icon
     * @param cb
     */
    void addCallback(StatusBarCallback cb);

    /**
     * Add new slot to the original slots array in the order you want.
     * The system icons will be shown in status bar in the order of slots array
     * The default slots please refer to:
     * alps/frameworks/base/core/res/res/values/config.xml
     * @param slots
     * @return new slots
     */
    String[] addSlot(String[] slots);

    /**
     */
    public interface StatusBarCallback {
        default void setSystemIcon(String slot, int resourceId,
                CharSequence contentDescription, boolean isVisible) {}
    }
}


