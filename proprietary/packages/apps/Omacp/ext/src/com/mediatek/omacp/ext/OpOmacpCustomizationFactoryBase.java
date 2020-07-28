package com.mediatek.omacp.ext;

import android.content.Context;

import android.util.Log;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;

public class OpOmacpCustomizationFactoryBase {

    private static final String TAG = "OpOmacpCustomizationFactoryBase";
    private static final ArrayList<OperatorFactoryInfo> sOpFactoryInfos =
            new ArrayList<OperatorFactoryInfo>();

    /// Operator owner register operator info @{
    static {
      if (isOpFactoryLoaderAvailable()) {
        sOpFactoryInfos.add(
                new OperatorFactoryInfo("OP18Omacp.apk",
                         "com.mediatek.op18.omacp.Op18OmacpCustomizationFactory",
                         "com.mediatek.op18.omacp",
                         "OP18")
                );

      }
    };
    /// @}

    public static synchronized OpOmacpCustomizationFactoryBase getOpFactory(Context context) {
        OpOmacpCustomizationFactoryBase sFactory = null;
        if (isOpFactoryLoaderAvailable()) {
            sFactory = (OpOmacpCustomizationFactoryBase)
                    OperatorCustomizationFactoryLoader.loadFactory(context, sOpFactoryInfos);
        }
        if (sFactory == null) {
            sFactory = new OpOmacpCustomizationFactoryBase();
        }
        return sFactory;
    }

    /**
     * For portable.
     * @return
     */
    private static boolean isOpFactoryLoaderAvailable() {
        try {
            Class.forName("com.mediatek.common.util.OperatorCustomizationFactoryLoader");
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    /// Operator owner should override these methods @{
    public IOmacpExt getOmacpExt() {
        Log.d(TAG, "return DefaultOmacpExt");
        return new DefaultOmacpExt();
    }
}
