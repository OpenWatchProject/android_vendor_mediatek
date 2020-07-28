 /* Execute
 * adb shell am instrument -w -e class com.mediatek.mediatekmail.test.MediatekMailJPETest com.mediatek.mediatekmail.test/android.test.InstrumentationTestRunner
 */

package com.mediatek.mediatekmail.test;

import android.test.AndroidTestCase;
import android.util.Log;

//Target package MediatekMail
import com.mediatek.protect.exchange.SmartPushCalculator;

public class MediatekMailJPETest extends AndroidTestCase {
    private static final String TAG = "MediatekMailJPETest";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        com.mediatek.common.jpe.a.b = false;
        Log.i(TAG, " Setup JPE Test Case ");
        Log.i(TAG, " Setup NativeCheck = " + com.mediatek.common.jpe.a.b);

    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        com.mediatek.common.jpe.a.b = false;
        Log.i(TAG, " TearDown JPE Test Case ");
        Log.i(TAG, " TearDown NativeCheck = " + com.mediatek.common.jpe.a.b);
    }

    public void testMediatekMailJPE() throws Throwable {

        Log.i(TAG, " testMediatekMailJPE ");

        SmartPushCalculator sp = SmartPushCalculator.getCalculator();

        Log.i(TAG, "MediatekMail NativeCheck = " + com.mediatek.common.jpe.a.b);
        assertTrue("MediatekMail is not JPE checked", (com.mediatek.common.jpe.a.b));

    }

}
