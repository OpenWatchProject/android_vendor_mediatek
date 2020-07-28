package com.mediatek.presence.core.ims.rcsua;

import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.platform.AndroidFactory;

import android.telephony.TelephonyManager;
import android.content.Context;
import java.util.ArrayList; 
import java.net.InetAddress;


public class RcsUaAdapterManager {
    private final static String TAG = "RcsUaAdapterManager";
    private final static Logger logger = Logger.getLogger(TAG);
    private static RcsUaAdapter[] mRcsUaAdapters = null;

    public static void createRcsUaAdapters() {
        logger.debug("createRcsUaAdapter");
        if (mRcsUaAdapters == null) {
            Context ctx = AndroidFactory.getApplicationContext();
            if (ctx == null) {
                logger.error("Create RcsSettings Fail. Context is null !!");
                return;
            }
            TelephonyManager tm = (TelephonyManager) ctx.getSystemService(
                            Context.TELEPHONY_SERVICE);
            int simCount = tm.getSimCount();
            mRcsUaAdapters = new RcsUaAdapter[simCount];
            for (int slotId = 0; slotId < simCount; ++slotId) {
                mRcsUaAdapters[slotId] = new RcsUaAdapter(slotId, ctx);
            }
        }
    }

    public static RcsUaAdapter getRcsUaAdapter(int slotId) {
        if (mRcsUaAdapters == null) {
            logger.debug("mRcsUaAdapters is null so create it");
            createRcsUaAdapters();
        }

        return mRcsUaAdapters[slotId];
    }
}
