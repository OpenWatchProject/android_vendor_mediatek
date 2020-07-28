package com.mediatek.settingslib.ext;

/**
 *  Interface for customize operator request.
 */
public interface IDrawerExt {

    /**
     * Customize strings which contains 'SIM', replace 'SIM' by
     * 'UIM/SIM','UIM','card' etc.
     * @param simString: the strings which contains SIM
     * @param soltId: 1 , slot1 0, slot0 , -1 means always.
     * @internal
     */
    public String customizeSimDisplayString(String simString, int slotId);

    /**
     * Customize the title of factory reset settings.
     * @param obj: header or activity
     * @internal
     */
    public void setFactoryResetTitle(Object obj);
}
