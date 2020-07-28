package com.mediatek.providers.settings.ext;

import android.content.Context;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

public class OpSettingsProviderCustomizationFactoryBase {

    public IDatabaseHelperExt makeDatabaseHelp(Context context) {
        return new DefaultDatabaseHelperExt(context);
    }

    private static final List<OperatorFactoryInfo> sOpFactoryInfoList
                                                = new ArrayList<OperatorFactoryInfo>();
    static {
        sOpFactoryInfoList.add(
                new OperatorFactoryInfo("OP01SettingsProvider.apk",
                    "com.mediatek.providers.settings.op01.Op01SettingsProviderCustomizationFactory",
                    "com.mediatek.providers.settings.op01",
                    "OP01"
                 ));



        sOpFactoryInfoList.add(
                new OperatorFactoryInfo("OP18SettingsProvider.jar",
                    "com.mediatek.op18.settingsProvider.OP18SettingsProviderCustomizationFactory",
                    null,
                    "OP18",
                    "SEGDEFAULT"
                 ));
    }

    static OpSettingsProviderCustomizationFactoryBase sFactory = null;
    public static synchronized OpSettingsProviderCustomizationFactoryBase getOpFactory(
            Context context) {
      if (sFactory == null) {
            sFactory =(OpSettingsProviderCustomizationFactoryBase)
                      OperatorCustomizationFactoryLoader.loadFactory(context, sOpFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpSettingsProviderCustomizationFactoryBase();
            }
        }
        return sFactory;
    }
}
