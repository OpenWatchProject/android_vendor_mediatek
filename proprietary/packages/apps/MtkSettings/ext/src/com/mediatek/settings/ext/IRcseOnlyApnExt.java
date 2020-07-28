package com.mediatek.settings.ext;

public interface IRcseOnlyApnExt {

    /**
     * is showing the rcse type apn profile or not.
     * @return true if showing.
     * @internal
     */
    boolean isRcseOnlyApnEnabled(String type);

    /**
     * Called when APN Settings fragment onCreate
     * @param listener the call back to add
     * @internal
     */
    void onCreate(OnRcseOnlyApnStateChangedListener listener, int subId);

    /**
     * Called when APN Settings fragment onCreate
     * @internal
     */
    void onDestory();

    /**
     * to call back when the rcse state changed.
     */
    interface OnRcseOnlyApnStateChangedListener {
        /**
         * @internal
         */
        void OnRcseOnlyApnStateChanged();
    }
}
