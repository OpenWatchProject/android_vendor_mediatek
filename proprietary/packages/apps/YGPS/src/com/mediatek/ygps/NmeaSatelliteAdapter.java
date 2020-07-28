package com.mediatek.ygps;

import java.util.Iterator;

/**
 * Adapter to hold a serial of satellites information.
 *
 */
public class NmeaSatelliteAdapter implements Iterable<SatelliteInfo> {

    private Iterator<NmeaParser.SatelliteInfo> mNmeaIterator = null;

    /**
     * Constructor function.
     * @param nmeaItr Iterable for satellites
     */
    public NmeaSatelliteAdapter(Iterable<NmeaParser.SatelliteInfo> nmeaItr) {
        if (nmeaItr != null) {
            mNmeaIterator = nmeaItr.iterator();
        }
    }

    @Override
    public Iterator<SatelliteInfo> iterator() {

        return new Iterator<SatelliteInfo>() {

            public boolean hasNext() {
                if (mNmeaIterator == null) {
                    return false;
                }
                return mNmeaIterator.hasNext();
            }

            public SatelliteInfo next() {
                if (mNmeaIterator == null) {
                    return null;
                }
                NmeaParser.SatelliteInfo nmeaSatel = mNmeaIterator.next();
                return toSatelliteInfo(nmeaSatel);
            }

            public void remove() {
                if (mNmeaIterator != null) {
                    mNmeaIterator.remove();
                }
            }

        };
    }

    private SatelliteInfo toSatelliteInfo(NmeaParser.SatelliteInfo nmeaSatel) {
        if (nmeaSatel == null) {
            return null;
        }
        SatelliteInfo satInfo = new SatelliteInfo();
        satInfo.mPrn = nmeaSatel.mPrn;
        satInfo.mSnr = nmeaSatel.mSnr;
        satInfo.mElevation = nmeaSatel.mElevation;
        satInfo.mAzimuth = nmeaSatel.mAzimuth;
        satInfo.mUsedInFix = nmeaSatel.mUsedInFix;
        satInfo.mColor = nmeaSatel.mColor;
        return satInfo;
    }

}
