package com.mediatek.ims.internal.ext;

import android.content.Context;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;
import java.util.List;

/** Util class to  provide Factory object.
  */
public class OpImsCustomizationUtils {

    //list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
                                                            = new ArrayList<OperatorFactoryInfo>();

    static OpImsCustomizationFactoryBase sFactory = null;

    static {
        // TODO: check n add for other operators too
        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP18Ims.jar",
                "com.mediatek.op18.ims.Op18ImsCustomizationFactory",
                null,
                "OP18"
        ));
        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP15Ims.jar",
                "com.mediatek.op15.ims.Op15ImsCustomizationFactory",
                null,
                "OP15"
        ));
        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP236Ims.jar",
                "com.mediatek.op236.ims.Op236ImsCustomizationFactory",
                null,
                "OP236"
        ));
    }

    /**
    * An API to get plugin factory object.
    * @param context context
    * @return OpImsCustomizationFactoryBase
    */
    public static synchronized OpImsCustomizationFactoryBase getOpFactory(Context context) {
        sFactory = (OpImsCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                .loadFactory(context, sOperatorFactoryInfoList);
        if (sFactory == null) {
            sFactory = new OpImsCustomizationFactoryBase();
        }
        return sFactory;
    }
}
