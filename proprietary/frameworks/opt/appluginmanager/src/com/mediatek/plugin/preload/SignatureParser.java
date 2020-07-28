package com.mediatek.plugin.preload;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.Signature;

import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.TraceHelper;

import java.io.IOException;
import java.io.InputStream;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.util.Enumeration;
import java.util.concurrent.atomic.AtomicReference;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * Parse signature for context/plugin.
 */
public class SignatureParser {
    private static AtomicReference<byte[]> sBuffer = new AtomicReference<byte[]>();

    public static final int SIGNATURE_PARSE_FAILED_INCONSISTENT_CERTIFICATES = -2;

    public static final int SIGNATURE_PARSE_FAILED_NO_CERTIFICATE = -1;

    public static final int SIGNATURE_PARSE_FAILED_UNEXCEPT_EXCEPTION = -3;

    private static final String TAG = "PluginManager/SignatureParser";

    private static Signature[] convertToSignatures(Certificate[] certs) {
        TraceHelper.beginSection(">>>>SignatureParser-convertToSignatures");
        final Signature[] res = new Signature[certs.length];
        int i = 0;
        try {
            for (; i < certs.length; i++) {
                res[i] = new Signature(certs[i].getEncoded());
            }
        } catch (CertificateEncodingException e) {
            Log.e(TAG, "<convertToSignatures>", e);
        }
        TraceHelper.endSection();
        return res;
    }

    private static Signature[] convertToSignatures(Certificate[][] certs) {
        final Signature[] res = new Signature[certs.length];
        int i = 0;
        try {
            for (; i < certs.length; i++) {
                res[i] = new Signature(certs[i][0].getEncoded());
            }
        } catch (CertificateEncodingException e) {
            Log.e(TAG, "<convertToSignatures>", e);
        }
        return res;
    }

    private static Certificate[] loadCertificates(JarFile jarFile, JarEntry je,
                                                  byte[] readBuffer) {
        TraceHelper.beginSection(">>>>SignatureParser-loadCertificates");
        try {
            TraceHelper.beginSection(">>>>SignatureParser-loadCertificates-getInputStream");
            InputStream is = jarFile.getInputStream(je);
            TraceHelper.endSection();
            TraceHelper.beginSection(">>>>SignatureParser-loadCertificates-read");
            while (is.read(readBuffer, 0, readBuffer.length) != -1) {
            }
            TraceHelper.endSection();
            is.close();
            TraceHelper.endSection();
            return je.getCertificates();
        } catch (IOException e) {
            Log.e(TAG, "<loadCertificates>", e);
        }
        TraceHelper.endSection();
        return null;
    }

    /**
     * Parse the signature for context package.
     * @param applicationContext
     *            Current application context
     * @return Signature
     */
    public static Signature[] parseSignature(Context applicationContext) {
        PackageManager pm = applicationContext.getPackageManager();
        try {
            PackageInfo info =
                    pm.getPackageInfo(applicationContext.getPackageName(),
                            PackageManager.GET_SIGNATURES);
            return info.signatures;
        } catch (NameNotFoundException e) {
            Log.e(TAG, "<parseSignature> Faild to get signature of package "
                    + applicationContext.getPackageName(), e);
            return null;
        }
    }

    /**
     * Parse the signature from JarFile.
     * @param jarFile
     *            JarFile
     * @return Signature
     */
    public static Signature[] parseSignature(JarFile jarFile) {
        Enumeration<JarEntry> entrys = jarFile.entries();
        while (entrys.hasMoreElements()) {
            TraceHelper.beginSection(">>>>SignatureParser-parseSignature-while");
            final JarEntry entry = entrys.nextElement();
            if (entry.isDirectory()) {
                continue;
            }
            if (entry.getName().startsWith("META-INF/")) {
                continue;
            }
            byte[] readBuffer = new byte[8192];
            Certificate[] certs = loadCertificates(jarFile, entry, readBuffer);
            if (certs == null) {
                Log.d(TAG, "<parseSignature> certs = null");
                continue;
            }
            final Signature[] entrySignatures = convertToSignatures(certs);
            if (entrySignatures != null) {
                TraceHelper.endSection();
                Log.d(TAG, "<parseSignature> entry name = " + entry.getName());
                return entrySignatures;
            }
            TraceHelper.endSection();
        }
        return null;
    }

    private static long readFullyIgnoringContents(InputStream in) throws IOException {
        byte[] buffer = sBuffer.getAndSet(null);
        if (buffer == null) {
            buffer = new byte[4096];
        }

        int n = 0;
        int count = 0;
        while ((n = in.read(buffer, 0, buffer.length)) != -1) {
            count += n;
        }

        sBuffer.set(buffer);
        return count;
    }
}
