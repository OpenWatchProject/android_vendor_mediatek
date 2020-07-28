package com.mediatek.plugin.zip;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;

import com.mediatek.plugin.res.ApkResource;
import com.mediatek.plugin.res.IResource;
import com.mediatek.plugin.utils.TraceHelper;

/**
 * Analysis Apk file for parser XML and copy so Lib.
 */
public class ApkFile extends ZipFile {
    protected PackageInfo mInfo;
    private IResource mResource;

    public static String getSuffix() {
        return ".apk";
    }

    /**
     * Constructor.
     * @param filePath
     *            The file path of plugin
     */
    public ApkFile(String filePath) {
        super(filePath);
    }

    @Override
    public String getXmlRelativePath() {
        return "res/raw/plugin.xml";
    }

    @Override
    public IResource getResource(Context context) {
        if (mResource == null) {
            initRes(context);
        }
        return mResource;
    }

    public PackageInfo getPackageInfo(Context context) {
        if (mInfo == null) {
            initRes(context);
        }
        return mInfo;
    }

    private void initRes(Context context) {
        TraceHelper.beginSection(">>>>ApkFile-initResource");
        mInfo =
                context.getPackageManager().getPackageArchiveInfo(mFilPath,
                        PackageManager.GET_CONFIGURATIONS);
        mResource = new ApkResource(context, mFilPath, mInfo == null ? null : mInfo.packageName);
        TraceHelper.endSection();
    }
}
