package com.android.camera.v2.app.location;

import android.content.Context;
import android.location.Location;

public class LocationManager {

    ILocationProvider mLocationProvider;
    private boolean mRecordLocation;

    public LocationManager(
            Context context) {
        LocationProviderImpl llp = new LocationProviderImpl(context);
        mLocationProvider = llp;
    }

    /**
     * Start/stop location recording.
     */
    public void recordLocation(boolean recordLocation) {
        mRecordLocation = recordLocation;
        mLocationProvider.recordLocation(mRecordLocation);
    }

    /**
     * Returns the current location from the location provider or null, if
     * location could not be determined or is switched off.
     */
    public Location getCurrentLocation() {
        return mLocationProvider.getCurrentLocation();
    }

    /**
     * Disconnects the location provider.
     */
    public void disconnect() {
        mLocationProvider.disconnect();
    }
}
