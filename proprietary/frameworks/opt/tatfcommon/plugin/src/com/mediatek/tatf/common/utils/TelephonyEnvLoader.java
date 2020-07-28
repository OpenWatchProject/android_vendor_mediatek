package com.mediatek.tatf.common.utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.os.Environment;

/**
 * Load telephony framework test environment from configuration file.
 * All the interface return the original json class, e.g  JSONArray/JSONObject
 * Make sure it be much extensionable for the futurn change.
 *
 * {
 * "sims": [
 *   {
 *   "number": "15928746301",
 *   "type": "CMCC",
 *   "capability": "4G",
 *   "slot":"1",
 *   "spn": ["CMCC"]
 *   },
 *   {
 *   "number": "15928746301",
 *   "type": "CU",
 *   "capability": "4G",
 *   "slot":"2",
 *   "spn": ["CU"]
 *   }
 * ]
 *}
 */
public class TelephonyEnvLoader {
    public static String TEL_ENV_FILE_NAME = Environment.getExternalStorageDirectory()
            + "/telenv.json";
    private static JSONObject sEnvInfo = null;
    public static final String SIM_ALL = "sims";
    public static final String SIM_NUMBER = "number";
    public static final String SIM_TYPE = "type";
    public static final String SIM_CAPABILITY = "capability";
    public static final String SIM_SLOT = "slot";
    public static final String SIM_SPN = "spn";

    /**
     * Get all this SIMs for the current test environments.
     * @return, sims of JSONArray
     */
    public static JSONArray getSims() {
        reloadIfNeed();
        if (sEnvInfo == null) {
            return null;
        }
        try {
            return sEnvInfo.getJSONArray("sims");
        } catch (JSONException e) {
            Logging.e("getSims exception " + e.getMessage());
            return null;
        }
    }

    private static String getSimSubItem(int index, String key) {
        JSONObject sim = getSim(index);
        if (sim == null) {
            return null;
        }
        try {
            return sim.getString(key);
        } catch (JSONException e) {
            Logging.e("getSimSubItem [" + index + "/" + key + "] exception " + e.getMessage());
            return null;
        }
    }

    private static JSONArray getSimSubItems(int index, String key) {
        JSONObject sim = getSim(index);
        if (sim == null) {
            return null;
        }
        try {
            return (JSONArray) sim.getJSONArray(key);
        } catch (JSONException e) {
            Logging.e("getSimSubItems [" + index + "/" + key + "] exception " + e.getMessage());
            return null;
        }
    }

    /**
     * Get sim number for the given slot
     * @param index
     * @return, sim number
     */
    public static String getSimNumber(int index) {
        return getSimSubItem(index, SIM_NUMBER);
    }

    /**
     * Get sim type for the given slot
     * @param index
     * @return CMCC/CU/CT
     */
    public static String getSimType(int index) {
        return getSimSubItem(index, SIM_TYPE);
    }

    /**
     * Get sim type for the given slot
     * @param index, sim index of json, start from 0.
     * @return slot id of Json file.
     */
    public static String getSimSlot(int index) {
        return getSimSubItem(index, SIM_SLOT);
    }

    /**
     * Get sim type for the given slot
     * @param index, sim index of json, start from 0.
     * @return 4G/3G/2G
     */
    public static String getSimCapability(int index) {
        return getSimSubItem(index, SIM_CAPABILITY);
    }

    /**
     * Get sim type for the given slot
     * @param index, sim index of json, start from 0.
     * @return spn of sim index.
     */
    public static JSONArray getSimSpn(int index) {
        return getSimSubItems(index, SIM_SPN);
    }

    /**
     * Get SIM of given slot id.
     * @param index, sim1: 0, sim2 1.
     * @return JSONObject of sim
     */
    public static JSONObject getSim(int index) {
        JSONArray sims = getSims();
        if (sims == null) {
            return null;
        }
        if (sims.length() < index) {
            Logging.e("sim index out of bound, input index [" + index + "], sim length ["
                    + sims.length() + "]");
            return null;
        }
        try {
            return sims.getJSONObject(index);
        } catch (JSONException e) {
            Logging.e("getSim [" + index + "] exception " + e.getMessage());
            return null;
        }
    }

    private static void reloadIfNeed() {
        if (sEnvInfo == null) {
            sEnvInfo = getEnv();
        }
    }

    /**
     * Load TelephonyEnvironment from json file in sdcard.
     *
     * @return the original json object of file.
     */
    private static JSONObject getEnv() {
        File configFile = new File(TEL_ENV_FILE_NAME);
        Logging.i("getEnv, path: " + configFile);
        try {
            BufferedReader br = new BufferedReader(new FileReader(configFile));
            String line = null;
            StringBuilder objString = new StringBuilder();
            while ((line = br.readLine()) != null) {
                objString.append(line);
            }
            JSONObject jsonObjs = new JSONObject(objString.toString());
            Logging.d("getEnv, load jsonObjs :" + jsonObjs);
            return jsonObjs;
        } catch (IOException | JSONException e) {
            Logging.e("loaderConfig e: " + e.getMessage());
        }
        return null;
    }

}
