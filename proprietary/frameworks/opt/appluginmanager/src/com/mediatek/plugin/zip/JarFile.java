package com.mediatek.plugin.zip;

import android.content.Context;

import com.mediatek.plugin.res.IResource;
import com.mediatek.plugin.res.JarResource;
import com.mediatek.plugin.utils.TraceHelper;

/**
 * Analysis Jar file for parser XML and copy so lib to host apk.
 */
public class JarFile extends ZipFile {
    private IResource mResource;

    public static String getSuffix() {
        return ".jar";
    }

    /**
     * Constructor.
     * @param filePath
     *            The file path of plugin
     */
    public JarFile(String filePath) {
        super(filePath);
    }

    @Override
    public String getXmlRelativePath() {
        return "res/raw/plugin.xml";
    }

    @Override
    public IResource getResource(Context context) {
        TraceHelper.beginSection(">>>>JarFile-getResource");
        if (mResource == null) {
            mResource = new JarResource(context, mFilPath);
        }
        TraceHelper.endSection();
        return mResource;
    }
}
