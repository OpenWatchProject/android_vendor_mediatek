package com.mediatek.settingslib.ext;

import android.content.Context;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

public class OpSettingsLibCustomizationFactoryBase {

    public IDrawerExt makeDrawer(Context context) {
        return new DefaultDrawerExt(context);
    }

    public IWifiLibExt makeWifiLib() {
        return new DefaultWifiLibExt();
    }

    private static final List<OperatorFactoryInfo> sOpFactoryInfoList
                                                = new ArrayList<OperatorFactoryInfo>();
    static {
        sOpFactoryInfoList.add(
                new OperatorFactoryInfo("OP01SettingsLib.apk",
                        "com.mediatek.settingslib.op01.Op01SettingsLibCustomizationFactory",
                        "com.mediatek.settingslib.op01",
                        "OP01"
                     ));

        sOpFactoryInfoList.add(
                new OperatorFactoryInfo("OP09AlibSettingsLib.apk",
                        "com.mediatek.settingslib.op09alib.Op09AlibSettingsLibCustomizationFactory",
                        "com.mediatek.settingslib.op09alib",
                        "OP09",
                        "SEGDEFAULT"
                     ));
    }

    static OpSettingsLibCustomizationFactoryBase sFactory = null;
    public static synchronized OpSettingsLibCustomizationFactoryBase getOpFactory(Context context) {
        if (sFactory == null) {
            sFactory = (OpSettingsLibCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                           .loadFactory(context, sOpFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpSettingsLibCustomizationFactoryBase();
            }
        }
        return sFactory;
    }
}
