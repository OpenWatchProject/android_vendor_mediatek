package com.mediatek.op07.browser;

import android.content.Context;

import com.mediatek.browser.ext.INetworkStateHandlerExt;
import com.mediatek.browser.ext.OpBrowserCustomizationFactoryBase;

public class Op07BrowserCustomizationFactory extends OpBrowserCustomizationFactoryBase {
    private Context mContext;

    public Op07BrowserCustomizationFactory(Context context) {
        mContext = context;
	}

    @Override
    public INetworkStateHandlerExt makeNetworkStateHandlerExt() {
        android.util.Log.d("Op07BrowserCustomizationFactory", "new plugin framework op07!");
        return new OP07NetworkStateHandlerExt(mContext);
    }

}