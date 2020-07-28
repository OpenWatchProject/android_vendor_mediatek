package com.mediatek.mms.ipmessage;

import android.content.Context;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;
import java.util.List;

public class IpMmsCustomizationUtils {

    // list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
            = new ArrayList<OperatorFactoryInfo>();

    static IpMmsCustomizationFactoryBase sFactory = null;

    static {
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("Rcse.apk",
                         "com.mediatek.rcse.plugin.message.RcsIpMmsCustomizationFactory",
                         "com.mediatek.rcs",
                         "OP06",
                         "SEGDEFAULT"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("Rcse.apk",
                         "com.mediatek.rcse.plugin.message.RcsIpMmsCustomizationFactory",
                         "com.mediatek.rcs",
                         "OP07",
                         "SEGDEFAULT"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("Rcse.apk",
                         "com.mediatek.rcse.plugin.message.RcsIpMmsCustomizationFactory",
                         "com.mediatek.rcs",
                         "OP08",
                         "SEGDEFAULT"
                        ));
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("Rcse.apk",
                         "com.mediatek.rcse.plugin.message.RcsIpMmsCustomizationFactory",
                         "com.mediatek.rcs",
                         "OP18",
                         "SEGDEFAULT"
                        ));
    }

    public static synchronized IpMmsCustomizationFactoryBase getOpFactory(Context context) {
        if (sFactory == null) {
            sFactory = (IpMmsCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                        .loadFactory(context, sOperatorFactoryInfoList);
            if (sFactory == null) {
                sFactory = new IpMmsCustomizationFactoryBase(context);
            }
        }
        return sFactory;
    }
}
