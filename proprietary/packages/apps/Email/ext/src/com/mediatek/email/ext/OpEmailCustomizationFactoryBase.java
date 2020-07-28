package com.mediatek.email.ext;

import android.content.Context;
import java.util.ArrayList;
import java.util.List;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import com.mediatek.email.ext.DefaultSendNotification;
import com.mediatek.email.ext.ISendNotification;
import com.mediatek.email.ext.DefaultServerProviderExt;
import com.mediatek.email.ext.IServerProviderExt;

public class OpEmailCustomizationFactoryBase {

    public ISendNotification makeSendNotification(Context context) {
        return new DefaultSendNotification();
    }

    public IServerProviderExt makeServerProvider(Context context) {
        return new DefaultServerProviderExt();
    }

    private static final List<OperatorFactoryInfo> sOpFactoryInfoList
                                                = new ArrayList<OperatorFactoryInfo>();
    static {
        sOpFactoryInfoList.add(
                new OperatorFactoryInfo("OP01Email.apk",
                         "com.mediatek.email.op01.Op01EmailCustomizationFactory",
                         "com.mediatek.email.op01",
                         "OP01",
                         "SEGC"
                        ));
    }

    static OpEmailCustomizationFactoryBase sFactory = null;
    public static synchronized OpEmailCustomizationFactoryBase getOpFactory(Context context) {
            sFactory = (OpEmailCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                           .loadFactory(context, sOpFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpEmailCustomizationFactoryBase();
            }
        return sFactory;
    }
}