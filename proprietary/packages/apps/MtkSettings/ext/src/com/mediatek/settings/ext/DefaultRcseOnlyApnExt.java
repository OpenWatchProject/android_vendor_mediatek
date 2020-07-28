package com.mediatek.settings.ext;

public class DefaultRcseOnlyApnExt implements IRcseOnlyApnExt {

    /** is show the rcse type apn profile or not.
     * @return true show.
     * @return false hide.
     */
    public boolean isRcseOnlyApnEnabled(String type) {
        return true;
    }

    /** Called when APN Settings fragment onCreate
     * @param listener the call back to add
     */
    public void onCreate(OnRcseOnlyApnStateChangedListener listener, int subId) {
    }

    /** Called when APN Settings fragment onCreate
     */
    public void onDestory() {
    }
}
