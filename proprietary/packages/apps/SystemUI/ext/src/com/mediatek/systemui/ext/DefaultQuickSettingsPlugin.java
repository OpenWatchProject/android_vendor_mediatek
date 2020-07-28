package com.mediatek.systemui.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.systemui.statusbar.extcb.IconIdWrapper;

/**
 * Default implementation of Plug-in definition of Quick Settings.
 */
public class DefaultQuickSettingsPlugin extends ContextWrapper implements
        IQuickSettingsPlugin {
    protected Context mContext;
    private static final String TAG = "DefaultQuickSettingsPlugin";

    /**
     * Constructor.
     * @param context The context.
     */
    public DefaultQuickSettingsPlugin(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public boolean customizeDisplayDataUsage(boolean isDisplay) {
        Log.i(TAG, "customizeDisplayDataUsage, return isDisplay = " + isDisplay);
        return isDisplay;
    }

    @Override
    public String customizeQuickSettingsTileOrder(String defaultString) {
        return defaultString;
    }

    @Override
    public Object customizeAddQSTile(Object qsTile) {
        return null;
    }

    @Override
    public String customizeDataConnectionTile(int dataState, IconIdWrapper icon,
            String orgLabelStr) {
        Log.i(TAG, "customizeDataConnectionTile, icon = " + icon + ", orgLabelStr=" + orgLabelStr);
        return orgLabelStr;
    }

    @Override
    public String customizeDualSimSettingsTile(boolean enable, IconIdWrapper icon,
            String labelStr) {
        Log.i(TAG, "customizeDualSimSettingsTile, enable = " + enable + " icon=" + icon
                + " labelStr=" + labelStr);
        return labelStr;
    }

    @Override
    public void customizeSimDataConnectionTile(int state, IconIdWrapper icon) {
        Log.i(TAG, "customizeSimDataConnectionTile, state = " + state + " icon=" + icon);
    }

    @Override
    public void disableDataForOtherSubscriptions() {
    }

    @Override
    public String customizeApnSettingsTile(boolean enable, IconIdWrapper icon, String orgLabelStr) {
        return orgLabelStr;
    }

    /**
     * Add plugin added tile specs.
     *
     * @param defaultTileList tile spec list.
     * @return the tile spec.
     */
    @Override
    public String addOpTileSpecs(String defaultTileList) {
        return defaultTileList;
    }

    /**
     * Whether operator support passed tile.
     *
     * @param tileSpec tile spec.
     * @return boolean.
     */
    @Override
    public boolean doOperatorSupportTile(String tileSpec) {
        return false;
    }

    /**
     * Creates new operator supported tile.
     *
     * @param host host.
     * @param tileSpec tile spec.
     * @return the tile object.
     */
    @Override
    public Object createTile(Object host, String tileSpec) {
        // Should never be called if operator implementation is not present
        return null;
    }

    /**
     * Creates new operator supported views.
     * @param vg view group in which view is to be added
     * @return
     */
    @Override
    public void addOpViews(ViewGroup vg) {
    }

    /**
     * Register callbacks.
     * @param
     * @return
     */
    @Override
    public void registerCallbacks() {
    }

    /**
     * Unregisters callbacks.
     * @param
     * @return
     */
    @Override
    public void unregisterCallbacks() {
    }

    /**
     * Set views visibility.
     * @param visibility visibility
     * @return
     */
    @Override
    public void setViewsVisibility(int visibility) {
    }

    /**
     * Calls views measure.
     * @param width width
     * @return
     */
    @Override
    public void measureOpViews(int width) {
    }

    /**
     * Gets view just over tile rows.
     * @param v view to be passed if no operator view present
     * @return View
     */
    @Override
    public View getPreviousView(View v) {
        return v;
    }

    /**
     * Provides collective Height of operator views.
     * @param
     * @return total height of operator views
     */
    @Override
    public int getOpViewsHeight() {
        return 0;
    }

    /**
     * Call view's layout.
     * @param aboveViewHeight height of view just above target view
     * @return
     */
    @Override
    public void setOpViewsLayout(int aboveViewHeight) {
    }

    @Override
    public String getTileLabel(String tile) {
        return "";
    }

    /**
     * Pass instance of Host App to plugin.
     * @param o Host App instance.
     */
    @Override
    public void setHostAppInstance(Object o) {
    }
}