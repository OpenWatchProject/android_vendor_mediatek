package com.mediatek.tatf.common.plugin;

import android.content.Context;
import android.os.IBinder;

/**
 * Tatf internal used plugin, implement getBinder and return plugin service
 * Binder for tatfserver.
 */
public abstract class TatfBasePlugin {

    /**
     * Return plugin service's Binder for Tatfserver.
     * @param context
     */
    public abstract IBinder getBinder(Context context);
}
