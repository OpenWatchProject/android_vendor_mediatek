package com.mediatek.gnssdebugreport;

import android.os.Bundle;

interface IDebugReportCallback {
    void onDebugReportAvailable(in Bundle debugReport);
}
