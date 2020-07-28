package com.android.providers.downloads;

import android.content.Context;

//import com.mediatek.common.MPlugin;
import com.mediatek.mtkdownloadmanager.ext.DefaultDownloadProviderFeatureExt;
import com.mediatek.mtkdownloadmanager.ext.IDownloadProviderFeatureExt;
import com.mediatek.mtkdownloadmanager.ext.OpDownloadProviderCustomizationFactoryBase;

/**
 * M: Plug-in helper class as the facade for accessing related add-ons.
 */
public class PluginFactory {
    private static IDownloadProviderFeatureExt sPlugin = null;

    /**
     * Get a IDownloadProviderFeatureExt object with Context.
     *
     * @param context A Context object.
     * @return IDownloadProviderFeatureExt object.
     */
    public static IDownloadProviderFeatureExt getDefault(Context context) {
        if (sPlugin == null) {
            //sPlugin = (IDownloadProviderFeatureExt) MPlugin.createInstance(
              //      IDownloadProviderFeatureExt.class.getName(), context);
              sPlugin = (OpDownloadProviderCustomizationFactoryBase.getOpFactory(context)).
              makeOpDownloadProvider(context);
            if (sPlugin == null) {
                sPlugin = new DefaultDownloadProviderFeatureExt(context);
            }
        }
        return sPlugin;
    }
}