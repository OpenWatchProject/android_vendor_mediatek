package com.mediatek.dm;

import android.net.Network;

/**
 * @hide
 */
interface IDmService {
    int getDmSupported();
    /**
     * Use to get/set Imc Provision state.
     */
    boolean getImcProvision(int phoneId, int feature);
    boolean setImcProvision(int phoneId, int feature, int pvs_en);
}
