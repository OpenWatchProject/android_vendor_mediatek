package com.mediatek.plugin.zip;

import android.content.Context;
import android.content.pm.Signature;

import com.mediatek.plugin.preload.SchemaValidate;
import com.mediatek.plugin.preload.SignatureParser;
import com.mediatek.plugin.preload.SoOperater;
import com.mediatek.plugin.res.IResource;
import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.TraceHelper;

import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.zip.ZipEntry;

/**
 * Analysis Zip file for parser XML and copy so Lib.
 */
public abstract class ZipFile {
    private final static String TAG = "PluginManager/ZipFile";
    protected String mFilPath;
    private java.util.jar.JarFile mJarFile;
    protected SchemaValidate mSchemaValidate;

    /**
     * ZipEntryConsumer is use for call back operation.
     */
    public static interface ZipEntryConsumer {
        /**
         * Check current ZipEntry.
         * @param entry
         *            The current ZipEntry.
         * @return whether should continue to traverser or not.
         */
        boolean consume(ZipEntry entry);
    }

    /**
     * Constructor.
     * @param filePath
     *            The file path of plugin
     */
    public ZipFile(String filePath) {
        mFilPath = filePath;
        TraceHelper.beginSection(">>>>ZipFile-new JarFile");
        try {
            mJarFile = new java.util.jar.JarFile(filePath);
        } catch (IOException e) {
            Log.e(TAG, "<ZipFile>", e);
        }
        TraceHelper.endSection();
    }

    /**
     * Get the signature of current plugin file.
     * @return The signature
     */
    public Signature[] getSignature() {
        TraceHelper.beginSection(">>>>ZipFile-getSignature");
        Signature[] res = SignatureParser.parseSignature(mJarFile);
        TraceHelper.endSection();
        return res;
    }

    /**
     * Copy so Lib to host Apk.
     * @param context
     *            use the context to get SharedPreferences object.
     * @param nativeLibDir
     *            The destination of so Lib copy to.
     */
    public void copySoLib(final Context context, final String nativeLibDir) {
        TraceHelper.beginSection(">>>>ZipFile-copySoLib");
        final SoOperater operater = new SoOperater();
        ZipEntry entry = enumerateZipEntry(new ZipEntryConsumer() {
            @Override
            public boolean consume(ZipEntry entry) {
                String entryName = entry.getName();
                boolean isSoFile = entryName.endsWith(".so");
                if (isSoFile) {
                    Log.d(TAG, "<copySoLib> So Lib Name: " + entryName + " || " + nativeLibDir);
                    return operater.isNewSo(context, entryName, entry.getTime());
                } else {
                    return false;
                }
            }
        });
        if (entry != null) {
            Log.d(TAG, "<copySoLib> Copying so lib: " + entry.getName() + " to :"
                    + nativeLibDir);
            operater.copy(context, mJarFile, entry, nativeLibDir);
        }
        TraceHelper.endSection();
    }

    /**
     * Get XML InputStream.
     * @return XML inputStream.
     */
    public InputStream getXmlInputStream() {
        return getInputStream(mFilPath, getXmlRelativePath());
    }

    /**
     * Get XML InputStream.
     * @param archivePath
     *            The absolute path.
     * @param relativePathOfFile
     *            The relative path.
     * @return XML inputStream.
     */
    public InputStream getInputStream(String archivePath, final String relativePathOfFile) {
        TraceHelper.beginSection(">>>>ZipFile-getInputStream");
        ZipEntry entry = enumerateZipEntry(new ZipEntryConsumer() {
            @Override
            public boolean consume(ZipEntry entry) {
                return relativePathOfFile.equalsIgnoreCase(entry.getName());
            }
        });
        InputStream inputStream = null;
        if (entry != null) {
            Log.d(TAG, "<getInputStream> entryName = " + entry.getName());
            try {
                inputStream = mJarFile.getInputStream(entry);
            } catch (IOException e) {
                Log.e(TAG, "<getInputStream>", e);
            }
        }
        TraceHelper.endSection();
        return inputStream;
    }

    /**
     * Verify plugin XML file whether conform to the rules by XML Schema.
     * @param xsdInputStream
     *            The xsd InputStream.
     * @return Whether available or not.
     */
    public boolean validateXML(InputStream xsdInputStream) {
        TraceHelper.beginSection(">>>>ZipFile-validateXML");
        if (mSchemaValidate == null) {
            mSchemaValidate = new SchemaValidate();
        }
        InputStream xmlInputStream = getXmlInputStream();
        boolean res = mSchemaValidate.validateXMLFile(xsdInputStream, xmlInputStream);
        TraceHelper.endSection();
        return res;
    }

    /**
     * Recycle holded resource, opened stream
     */
    public void recycle() {
        try {
            mJarFile.close();
        } catch (IOException e) {
            Log.e(TAG, "<recycle>", e);
        }
    }

    private ZipEntry enumerateZipEntry(ZipEntryConsumer consumer) {
        if (consumer == null) {
            return null;
        }
        Enumeration<JarEntry> entrys = mJarFile.entries();
        while (entrys.hasMoreElements()) {
            final ZipEntry entry = entrys.nextElement();
            if (consumer.consume(entry)) {
                return entry;
            }
        }
        return null;
    }

    /**
     * Get the file Resource for parser XML attribute.
     * @param context
     *            Use the context for get AssetManager.
     * @return The resource of the zip file.
     */
    public abstract IResource getResource(Context context);

    /**
     * Get XML position of the zip file.
     * @return The zip file path
     */
    protected abstract String getXmlRelativePath();
}
