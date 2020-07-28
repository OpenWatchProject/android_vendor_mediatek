package com.mediatek.mtkdownloadmanager.ext;

import android.content.Context;
import java.util.ArrayList;
import java.util.List;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

public class OpDownloadProviderCustomizationFactoryBase {

    public IDownloadProviderFeatureExt makeOpDownloadProvider(Context context) {
        return new DefaultDownloadProviderFeatureExt(context);
    }

    private static final List<OperatorFactoryInfo> sOpFactoryInfoList
                                                = new ArrayList<OperatorFactoryInfo>();
    static {

    sOpFactoryInfoList.add(
            new OperatorFactoryInfo("OP01DownloadProvider.apk",
                     "com.mediatek.downloadmanager.op01.Op01DownloadProviderCustomizationFactory",
                     "com.mediatek.downloadmanager.op01",
                     "OP01"
                 ));

    sOpFactoryInfoList.add(
            new OperatorFactoryInfo("OP02DownloadProvider.apk",
                     "com.mediatek.downloadmanager.op02.Op02DownloadProviderCustomizationFactory",
                     "com.mediatek.downloadmanager.op02",
                     "OP02"
                 ));

    sOpFactoryInfoList.add(
            new OperatorFactoryInfo("OP09DownloadProvider.apk",
                     "com.mediatek.downloadmanager.op09.Op09DownloadProviderCustomizationFactory",
                     "com.mediatek.downloadmanager.op09",
                     "OP09",
                     "SEGDEFAULT"
                 ));


    }

    static OpDownloadProviderCustomizationFactoryBase sFactory = null;
    public static synchronized OpDownloadProviderCustomizationFactoryBase
    getOpFactory(Context context) {
            sFactory = (OpDownloadProviderCustomizationFactoryBase)
            OperatorCustomizationFactoryLoader
                           .loadFactory(context, sOpFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpDownloadProviderCustomizationFactoryBase();
            }
        return sFactory;
    }
}
