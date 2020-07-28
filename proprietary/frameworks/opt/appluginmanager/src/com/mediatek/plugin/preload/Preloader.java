package com.mediatek.plugin.preload;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.Signature;

import com.mediatek.plugin.element.PluginDescriptor;
import com.mediatek.plugin.res.ApkResource;
import com.mediatek.plugin.res.IResource;
import com.mediatek.plugin.utils.ArrayUtils;
import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.TraceHelper;
import com.mediatek.plugin.zip.ApkFile;
import com.mediatek.plugin.zip.JarFile;
import com.mediatek.plugin.zip.ZipCenter;
import com.mediatek.plugin.zip.ZipFile;

import java.io.IOException;
import java.io.InputStream;

/**
 * Load the plugin XML from lib file and return the element.
 */
public class Preloader {
    private final static String TAG = "PluginManager/Preloader";
    private static final String NAME_XSD = "plugin.xsd";
    private static Preloader sPreloader;
    private boolean mHasInitEnvironment = false;
    private Signature[] mHostSignature;
    private int mHostVersion = -1;

    /**
     * Get singleton Preloader object.
     * @return return Preloader object.
     */
    public static Preloader getInstance() {
        if (sPreloader == null) {
            sPreloader = new Preloader();
        }
        return sPreloader;
    }

    /**
     * Check the Lib Signature with host package, and load plugin XML.
     * @param context
     *            The package context.
     * @param archivePath
     *            The Lib filePath.
     * @param nativeLibDir
     *            The native lib file Path.
     * @param signatureCheckEnabled
     *            True, check if signature of plugin matches with host before preload, if not match,
     *            not preload this plugin. False, not check.
     * @param xmlValidateEnabled
     *            True, check if plugin.xml matches the rule before preload, if not match, not
     *            preload this plugin. False, not check.
     * @return The element parsed from the Lib plugin XML.
     */
    public PluginDescriptor preloadPlugin(Context context, String archivePath,
                                          String nativeLibDir, boolean signatureCheckEnabled,
                                          boolean xmlValidateEnabled) {
        TraceHelper.beginSection(">>>>Preloader-preloadPlugin");

        // Do some initial operation here, these operations only do when preload the first plugin
        initPreloadEnviorment(context, signatureCheckEnabled);

        // File the ZipFile to process this plugin
        ZipFile zipFile = ZipCenter.createZipFile(archivePath);
        if (zipFile == null) {
            Log.d(TAG, "<preloadPlugin> Cannot find the ZipFile to process, return null");
            TraceHelper.endSection();
            return null;
        }

        // Get and check signature of plugin
        if (signatureCheckEnabled) {
            Signature[] targetFileSig = zipFile.getSignature();
            if (mHostSignature == null || targetFileSig == null
                    || !ArrayUtils.areExactMatch(mHostSignature, targetFileSig)) {
                Log.d(TAG, "<preloadPlugin> Signature not match, return null");
                zipFile.recycle();
                TraceHelper.endSection();
                return null;
            }
        }

        // Schema validate
        if (xmlValidateEnabled && !zipFile.validateXML(getXsdInputStream(context))) {
            Log.e(TAG, "<preloadPlugin> Schema validate fail, return null");
            zipFile.recycle();
            TraceHelper.endSection();
            return null;
        }

        // Parse plugin.xml and get PluginDescriptor
        IResource resource = zipFile.getResource(context);
        XMLParser xmlfile = new XMLParser(zipFile.getXmlInputStream(), resource);
        PluginDescriptor descriptor = (PluginDescriptor) xmlfile.parserXML();
        if (descriptor == null) {
            Log.e(TAG, "<preloadPlugin> parserXML return null, return null");
            zipFile.recycle();
            TraceHelper.endSection();
            return null;
        }
        if (!isMatchHostVersion(descriptor)) {
            Log.e(TAG, "<preloadPlugin> Version is not match with host, return null");
            zipFile.recycle();
            TraceHelper.endSection();
            return null;
        }
        descriptor.setArchivePath(archivePath);

        // Init Element for apk
        if (zipFile instanceof ApkFile) {
            descriptor.setAssetManager(((ApkResource) resource).getAssetManager());
            descriptor.setResource(((ApkResource) resource).getResources());
            descriptor.setPackageInfo(((ApkFile) zipFile).getPackageInfo(context));
        }

        // Copy so lib to native lib dir
        zipFile.copySoLib(context, nativeLibDir);

        // Print all elements for debug
        // descriptor.printf();

        zipFile.recycle();
        TraceHelper.endSection();
        return descriptor;
    }

    private synchronized void initPreloadEnviorment(Context context,
                                                    boolean signatureCheckEnabled) {
        if (!mHasInitEnvironment) {
            TraceHelper.beginSection(">>>>initPreloadEnviorment");
            ZipCenter.registerZipFile(ApkFile.getSuffix(), ApkFile.class);
            ZipCenter.registerZipFile(JarFile.getSuffix(), JarFile.class);
            if (signatureCheckEnabled) {
                initHostSignature(context);
            }
            initHostVersion(context);
            mHasInitEnvironment = true;
            TraceHelper.endSection();
        }
    }

    private boolean isMatchHostVersion(PluginDescriptor pluginDes) {
        if (mHostVersion == -1 || mHostVersion > pluginDes.requireMaxHostVersion
                || mHostVersion < pluginDes.requireMinHostVersion) {
            Log.d(TAG, "<isMatchHostVersion> version unvalidate! hostVersion " + mHostVersion
                    + ", plugin requireMaxHostVersion " + pluginDes.requireMaxHostVersion
                    + ", plugin requireMinHostVersion " + pluginDes.requireMinHostVersion);
            return false;
        }
        return true;
    }

    private void initHostVersion(Context context) {
        try {
            TraceHelper.beginSection(">>>>Preloader-initHostVersion");
            PackageInfo info =
                    context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
            mHostVersion = info.versionCode;
            Log.d(TAG, "<initHostVersion> mHostVersion = " + mHostVersion);
        } catch (NameNotFoundException e) {
            // Cannot find the package name, so don't add in the version parameter
            // This shouldn't happen.
            Log.e(TAG, "Invalid package name for context", e);
        } finally {
            TraceHelper.endSection();
        }
    }

    private void initHostSignature(Context context) {
        TraceHelper.beginSection(">>>>Preloader-initHostSignature");
        mHostSignature = SignatureParser.parseSignature(context);
        Log.d(TAG, "<initHostSignature> mHostSignature = " + mHostSignature);
        TraceHelper.endSection();
    }

    private InputStream getXsdInputStream(Context context) {
        try {
            TraceHelper.beginSection(">>>>Preloader-getXsdInputStream");
            return context.getAssets().open(NAME_XSD);
        } catch (IOException e1) {
            Log.e(TAG, "<initXSDInputStream> IOException", e1);
            return null;
        } finally {
            TraceHelper.endSection();
        }
    }
}
