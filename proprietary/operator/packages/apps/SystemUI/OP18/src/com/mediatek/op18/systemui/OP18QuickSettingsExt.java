package com.mediatek.op18.systemui;

import android.content.Context;
import android.content.res.Resources;
import android.os.SystemProperties;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import com.android.ims.ImsManager;
import com.android.systemui.qs.QSHost;

import com.mediatek.op18.systemui.R;
import com.mediatek.systemui.ext.DefaultQuickSettingsPlugin;

import java.util.HashMap;

/**
 * Customize carrier text.
 */
public class OP18QuickSettingsExt extends DefaultQuickSettingsPlugin {
    public static final String TAG = "OP18QuickSettingsExt";

    public static final int WIFI_CALLING_TILE_INDEX = 0;
    private static final String MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
    private Context mPluginContext;
    private final HashMap<Integer, String> mOp18TileSpecs = new HashMap<Integer, String>();
    private Resources mRes;

    private SimSelectionView mSimSelectionView;

    /**
     * Constructs a new OP18QuickSettingsExt instance with Context.
     * @param context A Context object
     */
    public OP18QuickSettingsExt(Context context) {
        super(context);
        mPluginContext = context;
        mRes = mPluginContext.getResources();
        loadTilesSupported();
    }

    /**
     * Add plugin added tile specs.
     *
     * @param defaultTileList tile spec list.
     * @return the tile spec.
     * @internal
     */
    @Override
    public String addOpTileSpecs(String defaultTileList) {
        Log.d(TAG, "addOpTileSpecs, defaultTileList:" + defaultTileList);
        if (ImsManager.isWfcEnabledByPlatform(mPluginContext)) {
            return defaultTileList + "," + mRes.getString(R.string.quick_settings_tiles_op18);
        } else {
            return defaultTileList;
        }
    }

    /**
     * Whether operator support passed tile.
     *
     * @param tileSpec tile spec.
     * @return boolean.
     * @internal
     */
    @Override
    public boolean doOperatorSupportTile(String tileSpec) {
        Log.d(TAG, "doOperatorSupportTile, op18 spec:" + mOp18TileSpecs + ", tile:" + tileSpec);
        Log.d(TAG, "WFC supported by platform:" + ImsManager.isWfcEnabledByPlatform(mPluginContext));
        Log.d(TAG, " multiims support = "+SystemProperties.getInt(MULTI_IMS_SUPPORT, 1));
        if (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) != 1) {
            return false;
        }
        return ImsManager.isWfcEnabledByPlatform(mPluginContext)
                && mOp18TileSpecs.containsValue(tileSpec);
    }

    /**
     * Creates new operator supported tile.
     *
     * @param host host.
     * @param tileSpec tile spec.
     * @return the tile object.
     * @internal
     */
    @Override
    public Object createTile(Object host, String tileSpec) {
        // No need to check whether operator supports this tile or not,
        // as it is already checked via doOperatorSupportTile
        int matchedTileIndex = -1;
        for (HashMap.Entry<Integer, String> entry : mOp18TileSpecs.entrySet()) {
            if (tileSpec.equals(entry.getValue())) {
                matchedTileIndex = entry.getKey();
                Log.d(TAG, "matched Key = " + matchedTileIndex + ", Value = " + tileSpec);
                break;
            }
        }
        switch(matchedTileIndex) {
            case WIFI_CALLING_TILE_INDEX:
                Log.d(TAG, tileSpec + " tile returned");
                return WifiCallingTile.getInstance(mPluginContext, (QSHost) host);
            default:
                Log.d(TAG, tileSpec + "not supported. Should never happen");
                return null;
        }
    }

    @Override
    public void addOpViews(ViewGroup vg) {
        // Add all views here
        mSimSelectionView = SimSelectionView.getInstance(mPluginContext, vg);
        mSimSelectionView.addView();
    }

    @Override
    public void registerCallbacks() {
        // register Views callbacks, receiver here
        mSimSelectionView.registerCallbacks();
    }

    @Override
    public void unregisterCallbacks() {
        // unregister Views callbacks, receiver here
        mSimSelectionView.unregisterCallbacks();
    }

    @Override
    public void setViewsVisibility(int visibility) {
        // Set Views visibility here
        mSimSelectionView.setVisibility(visibility);
    }

    @Override
    public void measureOpViews(int width) {
        // call all views measure method
        mSimSelectionView.measure(width);
    }

    @Override
    public View getPreviousView(View v) {
        // Keep cascading views returned by previous view in next views parameter
        View prevView = mSimSelectionView.getPreviousView(v);
        // Ex: prevView = xxxxView.getPreviousView(prevView);
        // prevView = yyyView.getPreviousView(prevView);
        return prevView;
    }

    @Override
    public int getOpViewsHeight() {
        // Add all views height here
        int height = mSimSelectionView.getViewHeight();
        // EX: height += xxxView.getViewHeight();
        return height;
    }

    @Override
    public void setOpViewsLayout(int aboveViewHeight) {
        // Set all views layout here
        int totalAboveViewsHeight = mSimSelectionView.setLayout(aboveViewHeight);
        // Ex: totalAboveViewsHeight += xxxView.setLayout(totalAboveViewsHeight);
        // Ex: totalAboveViewsHeight += yyyView.setLayout(totalAboveViewsHeight);
    }

    @Override
    public String customizeQuickSettingsTileOrder(String defaultString) {
        Log.d(TAG, "customizeQuickSettingsTileOrder, defaultString: " + defaultString);
        Log.d(TAG, "WFC enabled: " + ImsManager.isWfcEnabledByPlatform(mPluginContext));
        String tiles = mRes.getString(R.string.quick_settings_tiles_op18);
        if (ImsManager.isWfcEnabledByPlatform(mPluginContext)
                && !(defaultString.contains(tiles))) {
            return defaultString + "," + tiles;
        } else {
            return defaultString;
        }
    }

    private void loadTilesSupported() {
        String tileSpecs = mRes.getString(R.string.quick_settings_tiles_op18);
        int tileIndex = 0;
        for (String tile : tileSpecs.split(",")) {
            tile = tile.trim();
            if (tile.isEmpty()) {
                continue;
            }
            mOp18TileSpecs.put(tileIndex++, tile);
        }
        Log.d(TAG, "loadTilesSupported, op18 spec:" + mOp18TileSpecs);
    }

    /**
     * Pass instance of Host App to plugin.
     * @param o instance of Host App.
     */
    @Override
    public void setHostAppInstance(Object o) {
        WifiCallingTile.addQSTileHostInstance(mPluginContext, o);
    }
}
