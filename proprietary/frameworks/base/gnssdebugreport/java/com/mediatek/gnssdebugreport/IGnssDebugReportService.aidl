package com.mediatek.gnssdebugreport;

import com.mediatek.gnssdebugreport.IDebugReportCallback;

interface IGnssDebugReportService {
    boolean startDebug(IDebugReportCallback callback);
    boolean stopDebug(IDebugReportCallback callback);
    void addListener(IDebugReportCallback callback);
    void removeListener(IDebugReportCallback callback);
}
