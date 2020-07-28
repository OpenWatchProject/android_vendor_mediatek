package com.mediatek.systemui.ext;

import android.view.View;
import android.view.ViewGroup;
import com.mediatek.systemui.statusbar.extcb.IconIdWrapper;


/**
 * M: the interface for Plug-in definition of QuickSettings.
 */
public interface IQuickSettingsPlugin {

    /**
     * Customize create the data usage tile.
     * @param isDisplay The default value.
     * @return if true create data usage; If false not create.
     * @internal
     */
    boolean customizeDisplayDataUsage(boolean isDisplay);

    /**
     * Customize the quick settings tile order.
     * @param defaultString the default String;
     * @return the tiles strings splited by comma.
     * @internal
     */
    String customizeQuickSettingsTileOrder(String defaultString);

    /**
     * Customize additional quick settings tile.
     * @param qsTile The default QSTile.
     * @return customize QSTile instance
     * @internal
     */
    Object customizeAddQSTile(Object qsTile);

    /**
     * Customize the data connection tile view.
     * @param dataState The data state.
     * @param icon The icon wrapper.
     * @param orgLabelStr The dual data connection tile label.
     * @return the tile label.
     * @internal
     */
    String customizeDataConnectionTile(int dataState, IconIdWrapper icon, String orgLabelStr);

    /**
     * Customize the dual Sim Settings.
     * @param enable true is enable.
     * @param icon The icon wrapper.
     * @param labelStr The dual sim quick settings icon label
     * @return the tile label.
     * @internal
     */
    String customizeDualSimSettingsTile(boolean enable, IconIdWrapper icon, String labelStr);

    /**
     * Customize the sim data connection tile.
     * @param state The sim data state.
     * @param icon The icon wrapper.
     * @internal
     */
    void customizeSimDataConnectionTile(int state, IconIdWrapper icon);

    /**
     * Disable other sim data when default data is enabled.
     */
    void disableDataForOtherSubscriptions();

    /**
     * Customize the apn settings tile.
     *
     * @param enable true is enable.
     * @param icon The icon wrapper.
     * @param orgLabelStr The apn settings tile label.
     * @return the tile label.
     * @internal
     */
    String customizeApnSettingsTile(boolean enable, IconIdWrapper icon, String orgLabelStr);

    /**
     * Add plugin added tile specs.
     *
     * @param defaultTileList tile spec list.
     * @return the tile spec.
     */
    String addOpTileSpecs(String defaultTileList);

    /**
     * Whether operator support passed tile.
     *
     * @param tileSpec tile spec.
     * @return boolean.
     */
    boolean doOperatorSupportTile(String tileSpec);

        /**
     * Creates new operator supported tile.
     *
     * @param host host.
     * @param tileSpec tile spec.
     * @return the tile object.
     */
    Object createTile(Object host, String tileSpec);

    /**
        * Creates new operator supported views.
        * @param vg view group in which view is to be added
        * @return
        */
    void addOpViews(ViewGroup vg);

    /**
     * Register callbacks.
     * @param
     * @return
     */
    void registerCallbacks();

    /**
     * Unregisters callbacks.
     * @param
     * @return
     */
    void unregisterCallbacks();

    /**
     * Set views visibility.
     * @param visibility visibility
     * @return
     */
    void setViewsVisibility(int visibility);

    /**
     * Calls views measure.
     * @param width width
     * @return
     */
    void measureOpViews(int width);

    /**
     * Gets view just over tile rows.
     * @param v view to be passed if no operator view present
     * @return View
     */
    View getPreviousView(View v);

    /**
     * Provides collective Height of operator views.
     * @param
     * @return total height of operator views
     */
    int getOpViewsHeight();

    /**
     * Call view's layout.
     * @param aboveViewHeight height of view just above target view
     * @return
     */
    void setOpViewsLayout(int aboveViewHeight);

    /**
     * Get the tile label string.
     * @param tile The tile name.
     * @return the tile label string.
     * @internal
     */
    String getTileLabel(String tile);

    /**
         * Pass instance of Host App to plugin.
         * @param o instance of Host App.
         */
    void setHostAppInstance(Object o);
}
