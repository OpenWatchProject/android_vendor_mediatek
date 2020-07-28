/* Execute
 * adb shell am instrument -w -e class com.mediatek.tatfcommon.test.TatfCommonTest com.mediatek.tatfcommon.test/android.test.InstrumentationTestRunner
 */
package com.mediatek.tatfcommon.test;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.test.AndroidTestCase;
import android.text.TextUtils;

import com.mediatek.tatf.common.utils.Logging;
import com.mediatek.tatf.common.utils.TelephonyEnvLoader;

public class TatfCommonTest extends AndroidTestCase {

    private static final String SIM1_NUMBER = "123213544546";
    private static final String SIM1_TYPE = "CMCC";
    private static final String SIM1_CAPABILITY = "3G";
    private static final String SIM1_SLOT = "1";

    private static final String SIM2_NUMBER = "13122387652";
    private static final String SIM2_TYPE = "CU";
    private static final String SIM2_CAPABILITY = "4G";
    private static final String SIM2_SLOT = "2";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        createJsonFile();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testTelEnvLoader() throws Throwable {

        Logging.i("testTelEnvLoader ");
        JSONArray result = TelephonyEnvLoader.getSims();
        assertTrue("loadTelEnv Failed, null", result != null);
        assertTrue("loadTelEnv Failed", result.length() == 2);

        int length = result.length();
        for (int i = 0; i < length; i++) {
            JSONObject sim = TelephonyEnvLoader.getSim(i);
            assertTrue("get sim " + i + " Failed", sim != null);

            String number = TelephonyEnvLoader.getSimNumber(i);
            String expectNumber = (i == 0 ? SIM1_NUMBER : SIM2_NUMBER);
            assertTrue("get sim number " + i + " Failed", expectNumber.equals(number));

            String type = TelephonyEnvLoader.getSimType(i);
            String expectType = (i == 0 ? SIM1_TYPE : SIM2_TYPE);
            assertTrue("get sim type " + i + " Failed", expectType.equals(type));

            String capability = TelephonyEnvLoader.getSimCapability(i);
            String expectCapability = (i == 0 ? SIM1_CAPABILITY : SIM2_CAPABILITY);
            assertTrue("get sim capability " + i + " Failed", expectCapability.equals(capability));

            String slot = TelephonyEnvLoader.getSimSlot(i);
            String expectSlot = (i == 0 ? SIM1_SLOT : SIM2_SLOT);
            assertTrue("get sim slot " + i + " Failed", expectSlot.equals(slot));

            /*
             * JSONArray spn = TelephonyEnvLoader.getSimSpn(i);
             * assertTrue("get sim spn " + i + " Failed", spn != null);
             */
        }
    }

    private JSONObject createDemoJson() throws JSONException {
        JSONObject sim1 = new JSONObject();
        sim1.put(TelephonyEnvLoader.SIM_NUMBER, SIM1_NUMBER);
        sim1.put(TelephonyEnvLoader.SIM_TYPE, SIM1_TYPE);
        sim1.put(TelephonyEnvLoader.SIM_CAPABILITY, SIM1_CAPABILITY);
        sim1.put(TelephonyEnvLoader.SIM_SLOT, SIM1_SLOT);

        JSONObject sim2 = new JSONObject();
        sim2.put(TelephonyEnvLoader.SIM_NUMBER, SIM2_NUMBER);
        sim2.put(TelephonyEnvLoader.SIM_TYPE, SIM2_TYPE);
        sim2.put(TelephonyEnvLoader.SIM_CAPABILITY, SIM2_CAPABILITY);
        sim2.put(TelephonyEnvLoader.SIM_SLOT, SIM2_SLOT);

        JSONArray sims = new JSONArray();
        sims.put(sim1).put(sim2);

        JSONObject config = new JSONObject();
        config.put(TelephonyEnvLoader.SIM_ALL, sims);
        return config;
    }

    private void createJsonFile() {
        File configFile = new File(TelephonyEnvLoader.TEL_ENV_FILE_NAME);
        Logging.i("configFile path: " + configFile);
        try {
            BufferedWriter br = new BufferedWriter(new FileWriter(configFile, false));
            br.write(createDemoJson().toString());
            br.flush();
        } catch (IOException | JSONException e) {
            Logging.e("loaderConfig e: " + e.getMessage());
        }
    }

}
