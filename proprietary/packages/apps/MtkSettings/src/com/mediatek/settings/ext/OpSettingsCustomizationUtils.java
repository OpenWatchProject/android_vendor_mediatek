package com.mediatek.settings.ext;

import android.content.Context;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;
import com.mediatek.settings.ext.OpSettingsCustomizationFactoryBase;

import java.util.ArrayList;
import java.util.List;

public class OpSettingsCustomizationUtils {
    // list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
            = new ArrayList<OperatorFactoryInfo>();

    static OpSettingsCustomizationFactoryBase sFactory = null;

    static {
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP01Settings.apk",
                         "com.mediatek.settings.op01.Op01SettingsCustomizationFactory",
                         "com.mediatek.settings.op01",
                         "OP01"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP02Settings.apk",
                         "com.mediatek.settings.op02.Op02SettingsCustomizationFactory",
                         "com.mediatek.settings.op02",
                         "OP02"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP06Settings.apk",
                         "com.mediatek.op06.settings.Op06SettingsCustomizationFactory",
                         "com.mediatek.op06.settings",
                         "OP06",
                         "SEGDEFAULT"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP07Settings.apk",
                         "com.mediatek.op07.settings.OP07SettingsCustomizationFactory",
                         "com.mediatek.op07.settings",
                         "OP07",
                         "SEGDEFAULT"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP08Settings.apk",
                         "com.mediatek.op08.settings.Op08SettingsCustomizationFactory",
                         "com.mediatek.op08.settings",
                         "OP08",
                         "SEGDEFAULT"
                        ));
       sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP09ClibSettings.apk",
                         "com.mediatek.settings.op09clib.Op09ClibSettingsCustomizationFactory",
                         "com.mediatek.settings.op09clib",
                         "OP09",
                         "SEGC"
                        ));
       sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP12Settings.apk",
                         "com.mediatek.op12.settings.Op12SettingsCustomizationFactory",
                         "com.mediatek.op12.settings",
                         "OP12"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP18Settings.apk",
                         "com.mediatek.op18.settings.OP18SettingsCustomizationFactory",
                         "com.mediatek.op18.settings",
                         "OP18",
                         "SEGDEFAULT"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP20Settings.apk",
                         "com.mediatek.op20.settings.OP20SettingsCustomizationFactory",
                         "com.mediatek.op20.settings",
                         "OP20",
                         "SEGDEFAULT"
                        ));
    }

    public static synchronized OpSettingsCustomizationFactoryBase getOpFactory(Context context) {
        if (sFactory == null) {
            sFactory = (OpSettingsCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                        .loadFactory(context, sOperatorFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpSettingsCustomizationFactoryBase(context);
            }
        }
        return sFactory;
    }

    public static synchronized void setOPSettings() {
        sFactory = null;
    }
}
