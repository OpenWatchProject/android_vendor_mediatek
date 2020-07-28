package com.mediatek.mmsappservice.ext;

import android.content.Context;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;
import java.util.List;

public class OpMmsAppServiceCustomizationUtils {

    // list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
            = new ArrayList<OperatorFactoryInfo>();

    static OpMmsAppServiceCustomizationFactoryBase sFactory = null;

    static {
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP01Mms.apk",
                         "com.mediatek.mms.plugin.Op01MmsAppServiceCustomizationFactory",
                         "com.mediatek.mms.plugin",
                         "OP01"
                        ));
/*
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP09Mms.apk",
                         "com.mediatek.mms.plugin.Op09MmsCustomizationFactory",
                         "com.mediatek.mms.plugin",
                         "OP09",
                         "SEGDEFAULT"
                        ));
                        */
    }

    public static synchronized OpMmsAppServiceCustomizationFactoryBase getOpFactory(Context context) {
        if (sFactory == null) {
            sFactory = (OpMmsAppServiceCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                        .loadFactory(context, sOperatorFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpMmsAppServiceCustomizationFactoryBase(context);
            }
        }
        return sFactory;
    }
}
