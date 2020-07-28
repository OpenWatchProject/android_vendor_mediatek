package com.mediatek.tatf.common.utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import android.util.Log;

/**
 * Utility methods used only by tests.
 */
public class TestUtils {
    public interface Condition {
        public boolean isMet();
    }

    /**
     * Wait until a {@code Condition} is met.
     */
    public static boolean waitUntil(Condition condition, int timeoutSeconds) {
        return waitUntil("", condition, timeoutSeconds);
    }

    /**
     * Wait until a {@code Condition} is met.
     */
    public static boolean waitUntil(String message, Condition condition, int timeoutSeconds) {
        return waitUntil(message, condition, timeoutSeconds, 500);
    }

    /**
     * M: Wait until a {@code Condition} is met. Looping and check the condition
     * in @sleeptime.
     */
    public static boolean waitUntil(String message, Condition condition, int timeoutSeconds,
                                    int sleeptime) {
        Log.d(Logging.TAG, message + ": Waiting...");
        final long timeout = System.currentTimeMillis() + timeoutSeconds * 1000;
        while (System.currentTimeMillis() < timeout) {
            if (condition.isMet()) {
                return true;
            }
            try {
                Thread.sleep(sleeptime);
            } catch (InterruptedException ignore) {
                //
            }
        }
        return false;
    }

    public static void sleep(int i) {
        try {
            Thread.sleep(i * 1000);
        } catch (InterruptedException e) {
            // ignore
        }
    }

    public static String loadMTCallCertification(String filePath) {
        File configFile = new File(filePath);
        Logging.i("getEnv, path: " + configFile);
        try {
            BufferedReader br = new BufferedReader(new FileReader(configFile));
            String line = null;
            StringBuilder objString = new StringBuilder();
            while ((line = br.readLine()) != null) {
                objString.append(line);
            }
            return objString.toString();
        } catch (IOException e) {
            Logging.e("loaderConfig e: " + e.getMessage());
        }
        return "";
    }
}
