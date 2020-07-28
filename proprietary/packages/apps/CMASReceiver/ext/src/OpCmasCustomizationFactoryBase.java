package com.mediatek.cmas.ext;

import android.content.Context;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;
import java.util.List;


public class OpCmasCustomizationFactoryBase {

    public ICmasDuplicateMessageExt makeCmasDuplicateMessage(Context context) {
        return new DefaultCmasDuplicateMessageExt(context);
    }

    public ICmasMainSettingsExt makeCmasMainSettings(Context context) {
        return new DefaultCmasMainSettingsExt(context);
    }

    public ICmasMessageInitiationExt makeCmasMessageInitiation(Context context) {
        return new DefaultCmasMessageInitiationExt(context);
    }

    public ICmasSimSwapExt makeCmasSimSwap(Context context) {
        return new DefaultCmasSimSwapExt(context);
    }


    private static final List<OperatorFactoryInfo> sOpFactoryInfoList
                                                = new ArrayList<OperatorFactoryInfo>();
    static {
        sOpFactoryInfoList.add(
            new OperatorFactoryInfo("OP07cellbroadcastreceiver.apk",
         "com.mediatek.op07.cellbroadcastreceiver.Op07cellbroadcastreceiverCustomizationFactory",
         "com.mediatek.op07.cellbroadcastreceiver",
         "OP07"
        ));

        sOpFactoryInfoList.add(
            new OperatorFactoryInfo("OP08cellbroadcastreceiver.apk",
         "com.mediatek.op08.cellbroadcastreceiver.Op08cellbroadcastreceiverCustomizationFactory",
         "com.mediatek.op08.cellbroadcastreceiver",
         "OP08"
        ));

        sOpFactoryInfoList.add(
            new OperatorFactoryInfo("OP12CellbroadcastReceiver.apk",
         "com.mediatek.op12.cellbroadcastreceiver.Op12CellbroadcastReceiverCustomizationFactory",
         "com.mediatek.op12.cellbroadcastreceiver",
         "OP12"
        ));

        sOpFactoryInfoList.add(
            new OperatorFactoryInfo("OP20CellbroadcastReceiver.apk",
         "com.mediatek.op20.cellbroadcastreceiver.Op20CellbroadcastReceiverCustomizationFactory",
         "com.mediatek.op20.cellbroadcastreceiver",
         "OP20"
        ));

        sOpFactoryInfoList.add(
            new OperatorFactoryInfo("OP236cellbroadcastreceiver.apk",
         "com.mediatek.op236.cellbroadcastreceiver.Op236cellbroadcastreceiverCustomizationFactory",
         "com.mediatek.op236.cellbroadcastreceiver",
         "OP236"
        ));
    }

    static OpCmasCustomizationFactoryBase sFactory = null;
    public static synchronized OpCmasCustomizationFactoryBase getOpFactory(Context context) {
        if (sFactory == null) {
            sFactory = (OpCmasCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                           .loadFactory(context, sOpFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpCmasCustomizationFactoryBase();
            }
        }
        return sFactory;
    }

    public static synchronized void resetOpFactory() {
        sFactory = null;
    }
}
