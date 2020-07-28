package com.mediatek.ims.rcsua;

import android.util.Log;

/**
 * Callback indicate ACS status changed
 */
public abstract class AcsEventCallback extends AppCallback {

    /**
     * Used to notify Configuration status changed.
     *
     * @param valid Configuration is valid or not.
     * @param version Configuration version, only positive value is meaningful.
     */
    public void onConfigurationStatusChanged(boolean valid, int version) {}

    /**
     * Indicate ACS connection exist. Used for UI indicator.
     */
    public void onAcsConnected() {}

    /**
     * Indicate ACS connection not exist. Used fo UI indicator.
     */
    public void onAcsDisconnected() {}

    class Runner extends BaseRunner<Integer> {

        Runner(Integer... params) {
            super(params);
        }

        @Override
        void exec(Integer... params) {
            int type = params[0];
            boolean valid = params[1] == 1;
            int version = params[2];

            Log.d("AcsEventCallback",
                    "type[" + type + "],valid[" + valid + "],version[" + version + "]");

            switch (type) {
                case 0:
                    onConfigurationStatusChanged(valid, version);
                    break;

                case 1:
                    onAcsConnected();
                    break;

                case 2:
                    onAcsDisconnected();
                    break;

                default:
                    /* Nothing to do */
                    break;
            }
        }
    }

}

