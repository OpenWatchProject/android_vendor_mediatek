package com.mediatek.apmonitor.monitor;

import com.mediatek.apmonitor.IApmKpiMonitor;
import com.mediatek.apmonitor.KpiMonitor;
import com.mediatek.apmonitor.KpiMonitor.*;
import com.mediatek.apmonitor.ApmMsgDefs;
import com.mediatek.apmonitor.kpi.FooBar;

import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.lang.StringBuilder;


public class FooBarKpiMonitor extends KpiMonitor {
    private static final String TAG = "FooBarKpiMonitor";
    private static final Short APM_MSG_FOO_BAR = ApmMsgDefs.APM_MSG_FOO_BAR;

    public FooBarKpiMonitor(Context context, Handler handler) {
        super(context, handler, TAG);
        Log.d(TAG, "FooBarKpiMonitor Ctor");
    }

    @Override
    public boolean handleMessage(Message msg) {
        if (msg.what == MSG_ID_APM_KPI_BROADCAST) {
            Log.d(TAG, "FooBarKpiMonitor handleMessage " + msg);
            Intent intent = (Intent) msg.obj;
            int kpiSource = getIntentKpiSource(intent);
            if (kpiSource == IApmKpiMonitor.KPI_SOURCE_FOO_BAR) {
                // 171 is 0xAB
                FooBar kpi = new FooBar();
                kpi.mByte = (byte)0x01;
                kpi.mBytes = new byte[] {(byte)0xAB, (byte)0xCD, (byte)0xEF};
                kpi.mBytesLen = kpi.mBytes.length;
                kpi.mBoolean = true;
                kpi.mShort = 171; //0xAB
                kpi.mInteger = 43981; //0xABCD
                kpi.mFloat = 3.14f;
                kpi.mLong = 43981L; //0xABCD
                kpi.mDouble = 1.234567;
                kpi.mString = "foobar";
                kpi.mStringLen = kpi.mString.length();

                // Prepare Innter data
                kpi.AddInner(0, 1);
                kpi.AddInner(2, 3);

                byte[] payload = kpi.build();
                int len = kpi.currentWriteBytes();
                Byte simId = 0;
                long timestampMs = System.currentTimeMillis();

                Log.d(TAG, "payload(" + len + "): " + byteArrayToHex(payload));
                // payload(70):
                // 01000000 01    03000000  01       AB00   CDAB0000
                // version  mByte mBytesLen mBoolean mShort mInteger
                //
                // C3F54840 CDAB000000000000 87889B53C9C0F33F 06000000   02000000
                // mFloat   mLong            mDouble          mStringLen mInnerListSize
                //
                // ABCDEF 666F6F62617200
                // mBytes mString
                //
                // 00000000      01000000      02000000      03000000
                // Inner[0].mFoo Inner[0].mBar Inner[1].mFoo Inner[1].mBar

                apmSubmitKpiST(APM_MSG_FOO_BAR, simId, timestampMs, len, payload);
                Log.d(TAG, "submit APM_MSG_FOO_BAR");
                // Remember to close the KpiObj
                kpi.close();
                return true;
            }
        }
        return false;
    }

    public static String byteArrayToHex(byte[] a) {
       StringBuilder sb = new StringBuilder(a.length * 2);
       for (byte b: a) {
          sb.append(String.format("%02x", b));
       }
       return sb.toString();
    }
}
