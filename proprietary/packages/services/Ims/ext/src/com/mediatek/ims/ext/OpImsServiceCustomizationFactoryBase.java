package com.mediatek.ims.ext;

import android.content.Context;
import android.telephony.Rlog;
import com.mediatek.ims.internal.ImsVTProvider;
import com.mediatek.ims.internal.ImsVTUsageManager;

/** Factory class to make default plugin objects.
  */
public class OpImsServiceCustomizationFactoryBase {
    /**
    * An API to make ImsService default impl object.
    * @param context context
    * @return IImsServiceExt
    */
    public IImsServiceExt makeImsServiceExt(Context context) {
         return new ImsServiceExt(context);
    }

    public DigitsUtil makeDigitsUtil() {
        return new DigitsUtilBase();
    }

    public OpImsCallSessionProxy makeOpImsCallSessionProxy() {
        Rlog.d("OpFactory", "makeOpImsCallSessionProxy: default");
        return new OpImsCallSessionProxyBase();
    }

    /**
     * Create different ImsVTProvider for each operator.
     *
     * @return The new ImsVTProvider.
     */
    public ImsVTProvider makeImsVtProvider() {
        return new ImsVTProvider();
    }

    public ImsVTUsageManager makeImsVTUsageManager() {
        return new ImsVTUsageManager();
    }
}
