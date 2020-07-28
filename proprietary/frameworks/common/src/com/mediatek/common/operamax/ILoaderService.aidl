package com.mediatek.common.operamax;

import com.mediatek.common.operamax.ILoaderStateListener;

interface ILoaderService {
    /**
     * Called to start saving. Saving service will be started in background. IStateListener should
     * be used to watch if saving is actually started or not.
     */
    void startSaving();

    /**
     * Called to stop saving. Saving service will be stopped in background.
     */
    void stopSaving();

    /**
     * Get current tunnel state.
     *
     * @return value could be:
     *     1: opened
     *     2: closed
     */
    int getTunnelState();

    /**
     * Get current saving state.
     *
     * @return value could be:
     *     1: started
     *     2: stopped
     *     3: paused
     *     4: service exception
     */
    int getSavingState();

    /**
     * Register listener to watch tunnel and saving state changes.
     *
     * @see ILoaderStateListener
     */
    void registerStateListener(ILoaderStateListener listener);

    /**
     * Unregister listener that watches tunnel and saving state changes.
     *
     * @see ILoaderStateListener
     */
    void unregisterStateListener(ILoaderStateListener listener);

    /**
     * Launch OperaMax main UI
     */
    void launchOperaMAX();

    /**
     * Add app into directed connection list
     */
    void addDirectedApp(String packageName);

    /**
     * Remove app from directed connection list
     */
    void removeDirectedApp(String packageName);

    /**
     * Remove all apps from directed connection list
     */
    void removeAllDirectedApps();

    /**
     * Check if app is in directed connection list
     */
    boolean isAppDirected(String packageName);

    /**
     * Get directed app list
     */
    String[] getDirectedAppList();

    /**
     * Add host into directed connection list
     */
    void addDirectedHost(String host);

    /**
     * Remove host from directed connection list
     */
    void removeDirectedHost(String host);

    /**
     * Remove all hosts from directed connection list
     */
    void removeAllDirectedHosts();

    /**
     * Check if host is in directed connection list
     */
    boolean isHostDirected(String host);

    /**
     * Get directed host list
     */
    String[] getDirectedHostList();

    /**
     * Add header field into directed connection list
     */
    void addDirectedHeaderField(String key, String value);

    /**
     * Remove header field from directed connection list
     */
    void removeDirectedHeaderField(String key, String value);

    /**
     * Remove all header fields from directed connection list
     */
    void removeAllDirectedHeaderFields();

    /**
     * Check if header field is in directed connection list
     */
    boolean isHeaderFieldDirected(String key, String value);

    /**
     * Get directed header field list. Each field is like 'key:value'
     */
    String[] getDirectedHeaderFieldList();

    /**
     * Set compress level.
     * @param level must be in [1, 3]. 1 -> low compress; 2 -> medium compress; 3 -> high compress,
     */
    void setCompressLevel(int level);

    /**
     * Get current compress level.
     */
    int getCompressLevel();
}
