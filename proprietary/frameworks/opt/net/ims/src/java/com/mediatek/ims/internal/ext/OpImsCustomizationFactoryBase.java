package com.mediatek.ims.internal.ext;

import android.content.Context;

/** Factory class to make default plugin objects.
  */
public class OpImsCustomizationFactoryBase {
    /**
    * An API to make ImsManger plugin object.
    * @param context context
    * @return IImsManagerExt
    */
    public IImsManagerExt makeImsManagerExt(Context context) {
         return new ImsManagerExt();
    }
}
