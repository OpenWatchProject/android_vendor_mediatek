/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.browser;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.MemoryInfo;
import android.content.Context;
import android.net.WebAddress;
import android.os.Debug;
import android.os.Handler;
import android.os.Process;
import android.os.SystemClock;
import android.util.Log;
import com.android.internal.util.MemInfoReader;

import java.io.IOException;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Performance analysis
 */
public class Performance {

    private static final String LOGTAG = "browser";

    private final static boolean LOGD_ENABLED =
            com.android.browser.Browser.DEBUG;

    private static boolean mInTrace;

    // add for optimize browser memory using
    private static ActivityManager.MemoryInfo mSysMemThreshold;
    private final static double RELEASE_THRESHOLD = 0.4f;
    private final static int VISIBLE_WEBVIEW_NUMBER_THRESHOLD = 5;
    private final static double RELEASE_THRESHOLD_GMO = 0.3f;
    private final static int VISIBLE_WEBVIEW_NUMBER_THRESHOLD_GMO = 3;
    private static long mTotalMem = 0;
    private static long mVisibleAppThreshold = 0;
    private static final Object mLock = new Object();

    // Performance probe
    private static final int[] SYSTEM_CPU_FORMAT = new int[] {
            Process.PROC_SPACE_TERM | Process.PROC_COMBINE,
            Process.PROC_SPACE_TERM | Process.PROC_OUT_LONG, // 1: user time
            Process.PROC_SPACE_TERM | Process.PROC_OUT_LONG, // 2: nice time
            Process.PROC_SPACE_TERM | Process.PROC_OUT_LONG, // 3: sys time
            Process.PROC_SPACE_TERM | Process.PROC_OUT_LONG, // 4: idle time
            Process.PROC_SPACE_TERM | Process.PROC_OUT_LONG, // 5: iowait time
            Process.PROC_SPACE_TERM | Process.PROC_OUT_LONG, // 6: irq time
            Process.PROC_SPACE_TERM | Process.PROC_OUT_LONG  // 7: softirq time
    };

    private static long mStart;
    private static long mProcessStart;
    private static long mUserStart;
    private static long mSystemStart;
    private static long mIdleStart;
    private static long mIrqStart;

    private static long mUiStart;

    static void tracePageStart(String url) {
        if (BrowserSettings.getInstance().isTracing()) {
            String host;
            try {
                WebAddress uri = new WebAddress(url);
                host = uri.getHost();
            } catch (android.net.ParseException ex) {
                host = "browser";
            }
            host = host.replace('.', '_');
            host += ".trace";
            mInTrace = true;
            Debug.startMethodTracing(host, 20 * 1024 * 1024);
        }
    }

    static void tracePageFinished() {
        if (mInTrace) {
            mInTrace = false;
            Debug.stopMethodTracing();
        }
    }

    static void onPageStarted() {
        mStart = SystemClock.uptimeMillis();
        mProcessStart = Process.getElapsedCpuTime();
        long[] sysCpu = new long[7];
        if (Process.readProcFile("/proc/stat", SYSTEM_CPU_FORMAT, null, sysCpu, null)) {
            mUserStart = sysCpu[0] + sysCpu[1];
            mSystemStart = sysCpu[2];
            mIdleStart = sysCpu[3];
            mIrqStart = sysCpu[4] + sysCpu[5] + sysCpu[6];
        }
        mUiStart = SystemClock.currentThreadTimeMillis();
    }

    static void onPageFinished(String url) {
        long[] sysCpu = new long[7];
        if (Process.readProcFile("/proc/stat", SYSTEM_CPU_FORMAT, null, sysCpu, null)) {
            String uiInfo =
                    "UI thread used " + (SystemClock.currentThreadTimeMillis() - mUiStart) + " ms";
            if (LOGD_ENABLED) {
                Log.d(LOGTAG, uiInfo);
            }
            // The string that gets written to the log
            String performanceString =
                    "It took total " + (SystemClock.uptimeMillis() - mStart)
                            + " ms clock time to load the page." + "\nbrowser process used "
                            + (Process.getElapsedCpuTime() - mProcessStart)
                            + " ms, user processes used " + (sysCpu[0] + sysCpu[1] - mUserStart)
                            * 10 + " ms, kernel used " + (sysCpu[2] - mSystemStart) * 10
                            + " ms, idle took " + (sysCpu[3] - mIdleStart) * 10
                            + " ms and irq took " + (sysCpu[4] + sysCpu[5] + sysCpu[6] - mIrqStart)
                            * 10 + " ms, " + uiInfo;
            if (LOGD_ENABLED) {
                Log.d(LOGTAG, performanceString + "\nWebpage: " + url);
            }
            if (url != null) {
                // strip the url to maintain consistency
                String newUrl = new String(url);
                if (newUrl.startsWith("http://www.")) {
                    newUrl = newUrl.substring(11);
                } else if (newUrl.startsWith("http://")) {
                    newUrl = newUrl.substring(7);
                } else if (newUrl.startsWith("https://www.")) {
                    newUrl = newUrl.substring(12);
                } else if (newUrl.startsWith("https://")) {
                    newUrl = newUrl.substring(8);
                }
                if (LOGD_ENABLED) {
                    Log.d(LOGTAG, newUrl + " loaded");
                }
            }
        }
    }

    static String encodeToJSON(Debug.MemoryInfo memoryInfo) {
        StringBuilder memoryUsage = new StringBuilder();
        memoryUsage.append("{\r\n")
                .append("    \"Browser app (MB)\": {\r\n")
                .append("        \"Browser\": {\r\n")
                .append("            \"Pss\": {\r\n")
                .append(String.format("                \"DVM\": %.2f,\r\n",
                        memoryInfo.dalvikPss / 1024.0))
                .append(String.format("                \"Native\": %.2f,\r\n",
                        memoryInfo.nativePss / 1024.0))
                .append(String.format("                \"Other\": %.2f,\r\n",
                        memoryInfo.otherPss / 1024.0))
                .append(String.format("                \"Total\": %.2f\r\n",
                        memoryInfo.getTotalPss() / 1024.0))
                .append("            },\r\n")
                .append("            \"Private\": {\r\n")
                .append(String.format("                \"DVM\": %.2f,\r\n",
                        memoryInfo.dalvikPrivateDirty / 1024.0))
                .append(String.format("                \"Native\": %.2f,\r\n",
                        memoryInfo.nativePrivateDirty / 1024.0))
                .append(String.format("                \"Other\": %.2f,\r\n",
                        memoryInfo.otherPrivateDirty / 1024.0))
                .append(String.format("                \"Total\": %.2f\r\n",
                        memoryInfo.getTotalPrivateDirty() / 1024.0))
                .append("            },\r\n")
                .append("            \"Swapped\": {\r\n")
                .append(String.format("                \"DVM\": %.2f,\r\n",
                        memoryInfo.dalvikSwappedOut / 1024.0))
                .append(String.format("                \"Native\": %.2f,\r\n",
                        memoryInfo.nativeSwappedOut / 1024.0))
                .append(String.format("                \"Other\": %.2f,\r\n",
                        memoryInfo.otherSwappedOut / 1024.0))
                .append(String.format("                \"Total\": %.2f\r\n",
                        memoryInfo.getTotalSwappedOut() / 1024.0))
                .append("            },\r\n")
                .append("            \"Shared\": {\r\n")
                .append(String.format("                \"DVM\": %.2f,\r\n",
                        memoryInfo.dalvikSharedDirty / 1024.0))
                .append(String.format("                \"Native\": %.2f,\r\n",
                        memoryInfo.nativeSharedDirty / 1024.0))
                .append(String.format("                \"Other\": %.2f,\r\n",
                        memoryInfo.otherSharedDirty / 1024.0))
                .append(String.format("                \"Total\": %.2f\r\n",
                        memoryInfo.getTotalSharedDirty() / 1024.0)).append("            }\r\n")
                .append("        },\r\n");

        for (int i = 0; i < Debug.MemoryInfo.NUM_OTHER_STATS; ++i) {
            memoryUsage.append("        \"" + memoryInfo.getOtherLabel(i) + "\": {\r\n")
                       .append("            \"Pss\": {\r\n")
                       .append(String.format("                \"Total\": %.2f\r\n",
                               memoryInfo.getOtherPss(i) / 1024.0))
                       .append("            },\r\n")
                       .append("            \"Private\": {\r\n")
                       .append(String.format("                \"Total\": %.2f\r\n",
                               memoryInfo.getOtherPrivateDirty(i) / 1024.0))
                       .append("            },\r\n")
                       .append("            \"Shared\": {\r\n")
                       .append(String.format("                \"Total\": %.2f\r\n",
                               memoryInfo.getOtherSharedDirty(i) / 1024.0))
                       .append("            }\r\n");

            if (i + 1 == Debug.MemoryInfo.NUM_OTHER_STATS) {
                memoryUsage.append("        }\r\n")
                           .append("    }\r\n")
                           .append("}\r\n");
            } else {
                memoryUsage.append("        },\r\n");
            }
        }

        return memoryUsage.toString();
    }

    /// M: Dump memory info
    /**
     * Dump memory info
     * @param log2File If true, write the log to file
     * @param flag The unique flag for dumping memory info
     * @return The output file name
     */
    static String printMemoryInfo(boolean log2File, String flag) {
        String outputFileName = "";
        Debug.MemoryInfo memoryInfo = new Debug.MemoryInfo();
        Debug.getMemoryInfo(memoryInfo);

        StringBuilder memMessage = new StringBuilder();
        memMessage
                .append(flag)
                .append(" Browser Memory usage: (Total/DVM/Native/Other) \r\n")
                .append(flag)
                .append(String.format(" Pss=%.2f/%.2f/%.2f/%.2f MB\r\n",
                        memoryInfo.getTotalPss() / 1024.0, memoryInfo.dalvikPss / 1024.0,
                        memoryInfo.nativePss / 1024.0, memoryInfo.otherPss / 1024.0))
                .append(flag)
                .append(String.format(" Private=%.2f/%.2f/%.2f/%.2f MB\r\n",
                         memoryInfo.getTotalPrivateDirty() / 1024.0,
                         memoryInfo.dalvikPrivateDirty / 1024.0,
                         memoryInfo.nativePrivateDirty / 1024.0,
                         memoryInfo.otherPrivateDirty / 1024.0))
                .append(flag)
                .append(String.format(" Shared=%.2f/%.2f/%.2f/%.2f MB\r\n",
                          memoryInfo.getTotalSharedDirty() / 1024.0,
                          memoryInfo.dalvikSharedDirty / 1024.0,
                          memoryInfo.nativeSharedDirty / 1024.0,
                          memoryInfo.otherSharedDirty / 1024.0))
                .append(flag)
                .append(String.format(" Swapped=%.2f/%.2f/%.2f/%.2f MB",
                           memoryInfo.getTotalSwappedOut() / 1024.0,
                           memoryInfo.dalvikSwappedOut / 1024.0,
                           memoryInfo.nativeSwappedOut / 1024.0,
                           memoryInfo.otherSwappedOut / 1024.0));

        String otherMemMsg = "Browser other mem statistics: \r\n";
        for (int i = 0; i < Debug.MemoryInfo.NUM_OTHER_STATS; ++i) {
            otherMemMsg += " [" + String.valueOf(i) + "] " + memoryInfo.getOtherLabel(i) + ", pss="
                        + String.format("%.2fMB", memoryInfo.getOtherPss(i) / 1024.0)
                        + ", private="
                        + String.format("%.2fMB", memoryInfo.getOtherPrivateDirty(i) / 1024.0)
                        + ", shared="
                        + String.format("%.2fMB", memoryInfo.getOtherSharedDirty(i) / 1024.0)
                        + "\r\n";
        }

        if (log2File) {
            try {
                java.text.SimpleDateFormat sdf = new java.text.SimpleDateFormat("yyyyMMdd_HHmmss");
                outputFileName = "/storage/emulated/0/memDumpLog"
                        + sdf.format(new java.util.Date()) + ".txt";
                java.io.PrintWriter printWriter = new java.io.PrintWriter(outputFileName);
                printWriter.print(encodeToJSON(memoryInfo));
                printWriter.close();
            } catch (IOException ex) {
                Log.d(LOGTAG, "Failed to save memory logs to file, " + ex.getMessage());
                outputFileName = "";
            }
        } else {
            Log.d(LOGTAG, memMessage.toString());
            Log.d(LOGTAG, otherMemMsg);
        }

        return outputFileName;
    }

    /**
     * @param context The current activity context
     */
    static void dumpSystemMemInfo(Context context) {
        if (context != null) {
            if (mSysMemThreshold == null) {
                mSysMemThreshold = new ActivityManager.MemoryInfo();
                ((ActivityManager) context.getSystemService(Activity.ACTIVITY_SERVICE))
                        .getMemoryInfo(mSysMemThreshold);
                mTotalMem = mSysMemThreshold.totalMem;
                mVisibleAppThreshold = mSysMemThreshold.visibleAppThreshold;
                // print system memory info once
                if (LOGD_ENABLED) {
                    String flag = "MemoryDumpInfo" + System.currentTimeMillis();
                    Log.d(LOGTAG, "Browser Current Memory Dump time = " + flag);
                    printSysMemInfo(mSysMemThreshold, flag);
                }
            }
        }
    }
    /**
     * Dump memory info and check should release memory
     * @param visibleWebviewNums Numbers of visible webview
     * @param tabIndex If isFreeMemory is true, it's current tab index
     *                 If isFreeMemory is false, it's closed tab index
     * @param isFreeMemory If true, it is free memory
     * @param url The current webview's url
     * @param freeTabIndexs All free tab indexs
     * @param isRemoveTab If true, it is close tab
     */
    static boolean checkShouldReleaseTabs(int visibleWebviewNums,
            ArrayList<Integer> tabIndex, boolean isFreeMemory, String url,
            CopyOnWriteArrayList<Integer> freeTabIndexs, boolean isRemoveTab) {

        synchronized (mLock) {
            boolean isReleaseTabs = false;
            String flag = "MemoryDumpInfo" + System.currentTimeMillis();
            Log.d(LOGTAG, "Browser Current Memory Dump time = " + flag);
            if (LOGD_ENABLED) {
                if (isFreeMemory) {
                    if (!isRemoveTab) {
                        // special log for onpagefinish
                        Log.d(LOGTAG, flag + " Performance#checkShouldReleaseTabs()-->" +
                                "tabPosition = " + tabIndex + ", url = " + url);
                    }
                } else {
                    if (isRemoveTab) {
                        // special log for remove tab
                        Log.d(LOGTAG, flag + " Perfromance#checkShouldReleaseTabs()--->" +
                                "removeTabIndex = " + tabIndex);
                    } else {
                        // special log for print free tab index
                        Log.d(LOGTAG, flag + " Performance#checkShouldReleaseTabs()-->" +
                                "freeTabIndex = " + freeTabIndexs);
                    }
                }
            }

            MemInfoReader sysMemInfo = new MemInfoReader();
            sysMemInfo.readMemInfo();
            if (LOGD_ENABLED) {
                printProcessMemInfo(sysMemInfo, flag);
                printMemoryInfo(false, flag);
            }

            Debug.MemoryInfo processMemoryInfo = new Debug.MemoryInfo();
            Debug.getMemoryInfo(processMemoryInfo);
            double totalPss = (processMemoryInfo.getTotalPss() +
                    processMemoryInfo.getSummaryTotalSwap()) * 1024.0;
            double useage = totalPss / mTotalMem;

            if (LOGD_ENABLED) {
                java.text.NumberFormat nf = java.text.NumberFormat.getInstance();
                nf.setMaximumFractionDigits(3);
                Log.d(LOGTAG, flag + " current porcess take up the memory percent is "
                        + nf.format(useage));
            }

            if (Math.max(sysMemInfo.getFreeSize(), sysMemInfo.getCachedSize())
                    < mVisibleAppThreshold) {
                if(LOGD_ENABLED) {
                    Log.d(LOGTAG,
                            "Browser Pss =: " + processMemoryInfo.getTotalPss()
                                    / 1024.0 + " PSwap =: "
                                    + processMemoryInfo.getTotalSwappedOut() / 1024.0f
                                    + " SwappablePss =: "
                                    + processMemoryInfo.getTotalSwappablePss() / 1024.0f);
                }
                String optimize = android.os.SystemProperties.get("ro.vendor.gmo.ram_optimize");
                if (optimize != null && optimize.equals("1")) {
                    if (useage > RELEASE_THRESHOLD_GMO
                            && visibleWebviewNums > VISIBLE_WEBVIEW_NUMBER_THRESHOLD_GMO
                            && isFreeMemory) {
                        isReleaseTabs = true;
                    }
                } else {
                    if (useage > RELEASE_THRESHOLD
                            && visibleWebviewNums > VISIBLE_WEBVIEW_NUMBER_THRESHOLD
                            && isFreeMemory) {
                        isReleaseTabs = true;
                    }
                }
            }
            return isReleaseTabs;
        }
    }

    /**
     * Print the system memory information for debug
     * @param sysMemThreshold System memory info
     * @param flag The unique flag for dumping meomory info
     */
    static void printSysMemInfo(ActivityManager.MemoryInfo sysMemThreshold, String flag){
        // memInfo.threshold;
        // It's hidden api , need to pay attention if Android upgrade
        if (sysMemThreshold != null) {
            long total = sysMemThreshold.totalMem;
            long threshold = sysMemThreshold.threshold;
            long availMem = sysMemThreshold.availMem;
            long hiddenAppThreshold = sysMemThreshold.hiddenAppThreshold;
            long secondaryServerThreshold = sysMemThreshold.secondaryServerThreshold;
            long visibleAppThreshold = sysMemThreshold.visibleAppThreshold;
            long foregroundAppThreshold = sysMemThreshold.foregroundAppThreshold;
            StringBuilder sysMemUsage = new StringBuilder();
            sysMemUsage
                    .append("{\r\n")
                    .append(flag)
                    .append("    \"System Memory Usage (MB)\": {\r\n")
                    .append(flag)
                    .append(String.format("                total=: %.2f,\r\n",
                            total / 1024.0 / 1024.0))
                    .append(flag)
                    .append(String.format("                threshold=: %.2f,\r\n",
                            threshold / 1024.0 / 1024.0))
                    .append(flag)
                    .append(String.format("                availMem=: %.2f,\r\n",
                            availMem / 1024.0 / 1024.0))
                    .append(flag)
                    .append(String.format("                hiddenAppThreshold=: %.2f,\r\n",
                            hiddenAppThreshold / 1024.0 / 1024.0))
                    .append(flag)
                    .append(String.format("                secondaryServerThreshold=: %.2f,\r\n",
                            secondaryServerThreshold / 1024.0 / 1024.0))
                    .append(flag)
                    .append(String.format("                visibleAppThreshold=: %.2f,\r\n",
                            visibleAppThreshold / 1024.0 / 1024.0))
                    .append(flag)
                    .append(String.format("                foregroundAppThreshold=: %.2f,\r\n",
                            foregroundAppThreshold / 1024.0 / 1024.0));
            Log.d(LOGTAG, sysMemUsage.toString());
        }
    }

    /**
     * Print the current process memory information for debug
     * @param processMemInfo The current process info
     * @param flag The unique flag for dumping meomory info
     */
    static void printProcessMemInfo(MemInfoReader processMemInfo, String flag){
        if (processMemInfo != null) {
            StringBuilder processMemUsage = new StringBuilder();
            processMemUsage
                    .append(flag)
                    .append("{\r\n")
                    .append(flag)
                    .append("    \"Process Memory Usage (MB)\": {\r\n")
                    .append(flag)
                    .append(String.format("                TotalSize =: %.2f,\r\n",
                            processMemInfo.getTotalSize() / 1024.0 / 1024.0))
                    .append(flag)
                    .append(String.format("                FreeSize =: %.2f,\r\n",
                            processMemInfo.getFreeSize() / 1024.0 / 1024.0))
                    //.append(flag)
                    //.append(String.format("                MappedSize =: %.2f,\r\n",
                    //        processMemInfo.getMappedSize() / 1024.0 / 1024.0))
                    //.append(flag)
                    //.append(String.format("                BuffersSize =: %.2f,\r\n",
                    //        processMemInfo.getBuffersSize() / 1024.0 / 1024.0))
                    .append(flag)
                    .append(String.format("                CachedSize =: %.2f,\r\n",
                            processMemInfo.getCachedSize() / 1024.0 / 1024.0))
                    .append(flag)
                    .append(String.format("                SwapTotalSizeKb =: %.2f,\r\n",
                            processMemInfo.getSwapTotalSizeKb() / 1024.0))
                    .append(flag)
                    .append(String.format("                SwapFreeSizeKb =: %.2f,\r\n",
                            processMemInfo.getSwapFreeSizeKb() / 1024.0))
                    .append(flag)
                    .append(String.format("                KernelUsedSize =: %.2f,\r\n",
                            processMemInfo.getKernelUsedSize() / 1024.0 / 1024.0));
            Log.d(LOGTAG, processMemUsage.toString());
        }
    }
}
