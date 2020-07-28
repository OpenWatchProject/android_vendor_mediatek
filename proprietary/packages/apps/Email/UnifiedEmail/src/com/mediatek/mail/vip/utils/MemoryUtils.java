package com.mediatek.mail.vip.utils;

import com.android.internal.util.MemInfoReader;

public final class MemoryUtils {
    private MemoryUtils() {
    }

    private static long sTotalMemorySize = -1;

    public static long getTotalMemorySize() {
        if (sTotalMemorySize < 0) {
            MemInfoReader reader = new MemInfoReader();
            reader.readMemInfo();

            // getTotalSize() returns the "MemTotal" value from /proc/meminfo.
            // Because the linux kernel doesn't see all the RAM on the system (e.g. GPU takes some),
            // this is usually smaller than the actual RAM size.
            sTotalMemorySize = reader.getTotalSize();
        }
        return sTotalMemorySize;
    }
}
