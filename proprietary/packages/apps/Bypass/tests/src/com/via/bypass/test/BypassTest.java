package com.via.bypass.test;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;

import com.via.bypass.BypassSettings;

/**
 * Bypass test.
 */
public class BypassTest extends ActivityInstrumentationTestCase2<BypassSettings> {
    private static final String TAG = "BypassTest";
    private Instrumentation mInst;
    private Context mContext;
    private Activity mActivity = null;
//    private Intent mIntent;
    public int mode = -1;
    public int action = -1;
    public int result = -1;

    /**
     * Constructor.
     */
    public BypassTest() {
        super(BypassSettings.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        mInst = this.getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
//        mIntent = new Intent(Intent.ACTION_MAIN);
//        mIntent.setComponent(new ComponentName(mContext, BypassSettings.class.getName()));
    }

    /**
     * Test case 01.
     */
    public void test01() {
        Log.d(TAG, "test01: " + action + " " + mode);
        assertNotNull(mInst);
        assertNotNull(mContext);
        BypassSettings activity = (BypassSettings) mActivity; // startActivity(mIntent, null, null);
        mInst.waitForIdleSync();
        assertNotNull(activity);
        if (action < 0 || mode < 0) {
            result = -1;
            return;
        } else {
//            BypassTestSettings bypassTestSettings = new BypassTestSettings(activity);
            if (action == 0) {
                result = activity.enableBypassModeWait(mode);
            } else if (action == 1) {
                result = activity.disableBypassModeWait(mode);
            } else if (action == 2) {
                result = activity.queryBypassModeWait(mode);
            }
//            bypassTestSettings.stop();
        }
        Log.d(TAG, "test01 result: " + result);
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }
}
