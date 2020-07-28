package com.mediatek.email.ext;


import android.content.Context;

public class DefaultServerProviderExt implements IServerProviderExt {

    /**
     * M: check if need to support provider List function.
     *
     * @return true if support, the value is set in in plugin.
     */
    @Override
    public boolean isSupportProviderList() {
        return false;
    }

    /**
     * M: get the extension providers domains.
     *
     * @return extension provider domains.
     */
    @Override
    public String[] getProviderDomains() {
        return null;
    }

    /**
     * M: get the extension providers' names.
     *
     * @return extension provider names.
     */
    @Override
    public String[] getProviderNames() {
        return null;
    }

    /**
     * M: get the plugin context.
     *
     * @return
     */
    @Override
    public Context getContext() {
        return null;
    }

    /**
     * M: get the acount description, used in account setting step.
     *
     * @return
     */
    @Override
    public String getAccountNameDescription() {
        return null;
    }

    /**
     * M: get the extension provider xml, use this to get the provider host.
     *
     * @return
     */
    @Override
    public int getProviderXml() {
        return -1;
    }

    /**
     * M: get the provider icons, used to show AccountSetupChooseESP listview.
     *
     * @return
     */
    @Override
    public int[] getProviderIcons() {
        return null;
    }

    /**
     * M: get the provider number to display in chooseESP activity.
     */
    @Override
    public int getDisplayESPNum() {
        return 0;
    }

    /**
     * M: get the account signature, use to display in send mail content.
     *
     * @return
     */
    @Override
    public String getAccountSignature() {
        return null;
    }

    /** M: get the default provider domain, use to check the account whether is default.
     *
     * @return
     */
    public String getDefaultProviderDomain() {
        return null;
    }
}
