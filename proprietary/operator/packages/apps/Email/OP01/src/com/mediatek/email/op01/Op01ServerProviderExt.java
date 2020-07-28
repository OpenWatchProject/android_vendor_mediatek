package com.mediatek.email.op01;


import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.util.Log;

import com.mediatek.email.ext.DefaultServerProviderExt;

import com.mediatek.email.op01.R;


public class Op01ServerProviderExt extends DefaultServerProviderExt {

    private static final String TAG = "CMCCProviderPlugin";

    private static final boolean IS_SUPPORT_PROVIDER_LIST = true;
    private static final String DEFAULT_PROVIDER_DOMAIN = "139.com";
    private static final int DISPLAY_ESP_NUMBER = 7;

    /**
     * mContext will hold the Plugin's Context
     */
    private Context mContext;
    private static String[] sProviderNames;
    private static int sProviderxml;
    private static String[] sESPDomains;;
    private static int[] sProviderIconIds;
    private static int sDisplayESPNum;
    private static String sAccountNameDescription;
    private static String sDefaultAccountSignature;

    /**
     * M: PluginManager will instantiate this object with the context
     * of com.mediatek.op01.plugin
     * it's not an Application, so the context.getApplicationContext will
     * be null, we don't need it. we need context to access the resources
     * of com.mediatek.op01.plugin
     * @param context
     */
    public Op01ServerProviderExt(Context context) {
        Log.d(TAG, "Op01ServerProviderExt set up");
        mContext = context;
        loadProviderResources();
    }

    /**
     * get the plugin context.
     *
     * @return
     */
    @Override
    public Context getContext() {
        return mContext;
    }

    /**
     * M: check if need to support provider List function.
     *
     * @return true if support, the value is set in in plugin.
     */
    @Override
    public boolean isSupportProviderList() {
        Log.d(TAG, "isSupportProviderList " + IS_SUPPORT_PROVIDER_LIST);
        return IS_SUPPORT_PROVIDER_LIST;
    }

    /**
     * M: get the extension providers' names.
     *
     * @return extension provider names.
     */
    @Override
    public String[] getProviderNames() {
        return sProviderNames;
    }

    /**
     * M: get the extension provider xml, use this to get the provider host.
     *
     * @return
     */
    @Override
    public int getProviderXml() {
        return sProviderxml;
    }

    /**
     * M: get the extension providers domains.
     *
     * @return extension provider domains.
     */
    @Override
    public String[] getProviderDomains() {
        return sESPDomains;
    }

    /**
     * M: get the provider icons, used to show AccountSetupChooseESP listview.
     *
     * @return
     */
    @Override
    public int[] getProviderIcons() {
        return sProviderIconIds;
    }

    /**
     * M: get the acount description, used in account setting step.
     *
     * @return
     */
    @Override
    public String getAccountNameDescription() {
        return sAccountNameDescription;
    }

    /**
     * M: get the provider number to display in chooseESP activity.
     */
    @Override
    public int getDisplayESPNum() {
        return sDisplayESPNum;
    }

    /** M: get the account signature, use to display in send mail content.
     *
     * @return
     */
    @Override
    public String getAccountSignature() {
        return sDefaultAccountSignature;
    }

    /** M: get the default provider domain, use to check the account whether is default.
     *
     * @return
     */
    @Override
    public String getDefaultProviderDomain() {
        return DEFAULT_PROVIDER_DOMAIN;
    }

    /**
     * M: load the provider resource.
     */
    private void loadProviderResources() {
        final Resources res = mContext.getResources();
        TypedArray providerIconResources = res.obtainTypedArray(R.array.email_provider_icon_list);
        sProviderNames = res.getStringArray(R.array.email_provider_name_list);
        sProviderxml = R.xml.extension_email_providers;
        sESPDomains = res.getStringArray(R.array.email_provider_domain_list);
        sDisplayESPNum = DISPLAY_ESP_NUMBER;
        sAccountNameDescription =
                  res.getString(R.string.CMCC_provider_email_account_name_description);
        sDefaultAccountSignature = res.getString(R.string.CMCC_provider_email_account_signature);
        // load the icons id, email use plugin context to use these source.
        sProviderIconIds = new int[providerIconResources.length()];
        for (int i = 0; i < providerIconResources.length(); i++) {
            sProviderIconIds[i] = providerIconResources.getResourceId(i, 0);
        }
        Log.d(TAG, "loadProviderResources compelete");
    }
}
