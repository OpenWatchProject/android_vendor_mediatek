package com.mediatek.common.operamax;

oneway interface ILoaderStateListener {
    /**
     * Called when the tunnel state is changed.
     *
     * @param state, could be:
     *     1: opened
     *     2: closed
     */
    void onTunnelState(int state);

    /**
     * Called when the saving state is changed. Saving will be paused when no available network or using WiFi network.
     *
     * @param state, could be:
     *     1: started
     *     2: stopped
     *     3: paused
     */
    void onSavingState(int state);
}
