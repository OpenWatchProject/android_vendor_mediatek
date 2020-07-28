package com.mediatek.mms.ext;

import android.content.Context;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;
import java.util.List;

public class OpMmsCustomizationUtils {

    // list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
            = new ArrayList<OperatorFactoryInfo>();

    static OpMmsCustomizationFactoryBase sFactory = null;

    static {
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP01Mms.apk",
                         "com.mediatek.mms.plugin.Op01MmsCustomizationFactory",
                         "com.mediatek.mms.plugin",
                         "OP01"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP02Mms.apk",
                         "com.mediatek.mms.plugin.Op02MmsCustomizationFactory",
                         "com.mediatek.mms.plugin",
                         "OP02"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP09MmsClib.apk",
                         "com.mediatek.mms.plugin.Op09MmsCustomizationFactory",
                         "com.mediatek.mms.plugin",
                         "OP09",
                         "SEGC"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("Op18Mms.apk",
                         "com.mediatek.op18.mms.Op18MmsCustomizationFactory",
                         "com.mediatek.op18.mms",
                         "OP18",
                         "SEGDEFAULT"
                        ));
    }

    public static synchronized OpMmsCustomizationFactoryBase getOpFactory(Context context) {
        if (sFactory == null) {
            sFactory = (OpMmsCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                        .loadFactory(context, sOperatorFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpMmsCustomizationFactoryBase(context);
            }
        }
        return sFactory;
    }
}
