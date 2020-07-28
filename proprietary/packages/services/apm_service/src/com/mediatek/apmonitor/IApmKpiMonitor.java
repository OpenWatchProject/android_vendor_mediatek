package com.mediatek.apmonitor;

/**
 * APM interface, KPI providers send vendor proprietary broadcast ACTION_APM_KPI_EVENT
 * with permission KPI_RECEIVE_PERMISSION to APM service, including a extra key EXTRA_KPI_SOURCE
 * to identify where the KPI come.
 */
public class IApmKpiMonitor {
    public static final String ACTION_APM_KPI_EVENT = "mediatek.intent.action.APM_KPI_EVENT";
    // In order to avoid DMC runs during boot-up, use this broadcast to notify APM
    // to create HIDL session to DMC.
    public static final String ACTION_APM_INIT_EVENT = "mediatek.intent.action.APM_INIT_EVENT";
    public static final String APM_SERVICE_PACKAGE = "com.mediatek.apmonitor";
    public static final String APM_SERVICE_COMPONENT = "com.mediatek.apmonitor.ApmKpiReceiver";

    public static final String RECEIVE_APM_KPI_PERMISSION =
        "com.mediatek.apmonitor.RECEIVE_APM_KPI_PERMISSION";

    public static final String EXTRA_KPI_SOURCE = "kpi_source";
    public static final String EXTRA_KPI_TIMESTAMP = "kpi_timestamp";
    public static final String EXTRA_KPI_SIMID = "kpi_simid";

    public static final int KPI_SIMID_UNKNOWN = -1;
    public static final int SIM_ID_DEFAULT = Byte.MAX_VALUE;
    public static final long KPI_INVALID_TIMESTAMP = 0;

    public static final int KPI_SOURCE_UNKNOWN = -1;
    public static final int KPI_SOURCE_FOO_BAR = 0;
}
