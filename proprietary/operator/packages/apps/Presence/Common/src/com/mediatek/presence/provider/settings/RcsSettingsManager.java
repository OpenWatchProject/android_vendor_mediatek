package com.mediatek.presence.provider.settings;

import android.content.Context;
import android.telephony.TelephonyManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.utils.logger.Logger;

public class RcsSettingsManager {
    private final static String TAG = "RcsSettingsManager";
    private final static Logger logger = Logger.getLogger(TAG);
    private static RcsSettings[] mRcsSettings = null;

    public static synchronized void createRcsSettings() {
        logger.debug("createRcsSettings");
        if (mRcsSettings == null) {
            Context ctx = AndroidFactory.getApplicationContext();
            if (ctx == null) {
                logger.error("Create RcsSettings Fail. Context is null !!");
                return;
            }
            TelephonyManager tm = (TelephonyManager) ctx.getSystemService(
                            Context.TELEPHONY_SERVICE);
            int simCount = tm.getSimCount();
            mRcsSettings = new RcsSettings[simCount];
            for (int slotId = 0; slotId < simCount; ++slotId) {
                mRcsSettings[slotId] = new RcsSettings(ctx, slotId);
            }
        }
    }

    public static RcsSettings getRcsSettingsInstance(int slotId) {
        if (mRcsSettings == null) {
            logger.debug("mRcsSettings is null so create it");
            createRcsSettings();
        }
        return mRcsSettings[slotId];
    }
}
