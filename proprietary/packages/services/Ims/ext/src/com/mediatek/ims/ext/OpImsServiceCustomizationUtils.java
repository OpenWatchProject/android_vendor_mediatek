package com.mediatek.ims.ext;

import android.content.Context;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;
import java.util.List;

/** Util class to  provide Factory object.
  */
public class OpImsServiceCustomizationUtils {

    //list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
                                                            = new ArrayList<OperatorFactoryInfo>();

    static OpImsServiceCustomizationFactoryBase sFactory = null;

    static {
        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP08Ims.apk",
                "com.mediatek.op08.ims.Op08ImsServiceCustomizationFactory",
                "com.mediatek.op08.ims",
                "OP08"
        ));
        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP18Ims.jar",
                "com.mediatek.op18.ims.Op18ImsServiceCustomizationFactory",
                null,
                "OP18"
        ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP12Ims.apk",
                "com.mediatek.op12.ims.Op12ImsServiceCustomizationFactory",
                "com.mediatek.op12.ims",
                "OP12"
        ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP112Ims.apk",
                "com.mediatek.op112.ims.Op112ImsServiceCustomizationFactory",
                "com.mediatek.op112.ims",
                "OP112"
        ));
    }

    /**
    * An API to get plugin factory object.
    * @param context context
    * @return OpImsServiceCustomizationFactoryBase
    */
    public static synchronized OpImsServiceCustomizationFactoryBase getOpFactory(Context context)
    {
        sFactory = (OpImsServiceCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                .loadFactory(context, sOperatorFactoryInfoList);
        if (sFactory == null) {
            sFactory = new OpImsServiceCustomizationFactoryBase();
        }
        return sFactory;
    }
}
