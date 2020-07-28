package com.mediatek.systemui.op02;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.systemui.op02.R;
import com.mediatek.systemui.ext.DefaultQuickSettingsPlugin;

/**
 * Customize carrier text.
 */
public class OP02QuickSettingsPlugin extends DefaultQuickSettingsPlugin {
    public static final String MobileDataTile = "dataconnection";
    public static final String TAG = "OP02QuickSettingsPlugin";

    /**
     * Constructs a new OP02QuickSettingsPlugin instance with Context.
     * @param context A Context object
     */
    public OP02QuickSettingsPlugin(Context context) {
        super(context);
    }

    @Override
    public boolean customizeDisplayDataUsage(boolean isDisplay) {
        Log.i(TAG, "customizeDisplayDataUsage, " + " return true");
        return true;
    }

    @Override
    public String customizeQuickSettingsTileOrder(String defaultString) {
        String[] tiles = defaultString.split(",");
        String tempTiles = tiles[0];
        for (int i = 1; i < tiles.length; i++) {
            if (i == 4) {
                // Op MobileDataTile order index is 4
                tempTiles = tempTiles + "," + MobileDataTile + "," + tiles[i];
            } else {
                tempTiles = tempTiles + "," + tiles[i];
            }
        }
        return tempTiles;
    }

    @Override
    public String getTileLabel(String tile) {
        String tileString = "";
        if (tile != null && tile.equalsIgnoreCase("mobiledata")) {
            tileString = mContext.getString(R.string.mobile);
        }
        return tileString;
    }
}
