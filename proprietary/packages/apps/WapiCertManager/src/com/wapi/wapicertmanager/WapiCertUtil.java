/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

package com.wapi.wapicertmanager;

import android.os.Environment;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

class WapiCertUtil {
    private static final String TAG = "WapiCertUtil";

    // shared preference name
    public static final String CERT_INFO_PREFS_NAME = "CertInfoCfg";
    // certificate type
    public static final int CERT_TYPE_WAPI_USER = 0;
    public static final int CERT_TYPE_WAPI_CA = 1;
    public static final int CERT_TYPE_NO_WAPI = 2;

    // wapi certificate tag
    private static final String CERT_BEGIN = "-----BEGIN CERTIFICATE-----";
    private static final String CERT_END = "-----END CERTIFICATE-----";
    private static final String PRIKEY_BEGIN = "-----BEGIN EC PRIVATE KEY-----";
    private static final String PRIKEY_END = "-----END EC PRIVATE KEY-----";

    private static final String WAPI_SIGN_ALG_OID = "1.2.156.11235.1.1.1";

    static byte[] getCertElement(byte[] certData) {
        if (certData == null) {
            return null;
        }
        String strCertData = new String(certData);
        int nBegin = strCertData.indexOf(CERT_BEGIN);
        int nEnd = strCertData.indexOf(CERT_END) + CERT_END.length();
        String strElement = strCertData.substring(nBegin, nEnd);
        return strElement.getBytes();
    }

    static byte[] getPriKeyElement(byte[] certData) {
        if (certData == null) {
            return null;
        }
        String strCertData = new String(certData);
        int nBegin = strCertData.indexOf(PRIKEY_BEGIN) + PRIKEY_BEGIN.length()
                + 1;
        int nEnd = strCertData.indexOf(PRIKEY_END) - 1;
        String strElement = strCertData.substring(nBegin, nEnd);
        return strElement.getBytes();
    }

    static byte[] addCertHeader(String strCertElement) {
        String strCertData = CERT_BEGIN + "\n" + strCertElement + "\n"
                + CERT_END;
        return strCertData.getBytes();
    }

    static byte[] readFile(File file) {
        try {
            byte[] data = new byte[(int) file.length()];
            FileInputStream inputStream = new FileInputStream(file);
            inputStream.read(data);
            inputStream.close();
            return data;
        } catch (Exception e) {
            Log.w(TAG, "Read file error: " + e);
            return null;
        }
    }

    static boolean isTheSuffix(String file, String suffix) {
        if (file == null || file.length() < suffix.length()) {
            return false;
        }
        for (int i = 0; i < suffix.length(); i++) {
            if (file.charAt(file.length() - suffix.length() + i) != suffix
                    .charAt(i)) {
                return false;
            }
        }
        return true;
    }

    static X509Certificate generateCertificate(byte[] certBytes) {
        try {
            CertificateFactory factory = CertificateFactory.getInstance("X.509");
            X509Certificate x509Cert = (X509Certificate)
                    factory.generateCertificate(
                            new ByteArrayInputStream(certBytes));
            return x509Cert;
        } catch (CertificateException e) {
            Log.w(TAG, "generateCertificate: " + e);
            return null;
        }
    }

    static int getCertType(byte[] certBytes) {
        int certType = CERT_TYPE_NO_WAPI;
        X509Certificate x509cert = generateCertificate(certBytes);
        try {
            String issuerCN = "";
            String issuerDC = "";
            String subjectCN = "";

            String issuer = x509cert.getIssuerX500Principal().getName();
            String subject = x509cert.getSubjectX500Principal().getName();
            String issuerItems[] = issuer.split(",");
            String subjectItems[] = subject.split(",");

            for (int i = 0; i < issuerItems.length; i ++) {
                if (issuerItems[i].startsWith("CN=")) {
                    issuerCN = issuerItems[i].substring(3);
                }
                else if (issuerItems[i].startsWith("DC=")) {
                    issuerDC = issuerItems[i].substring(3);
                }
            }

            for (int i = 0; i < subjectItems.length; i ++) {
                if (subjectItems[i].startsWith("CN=")) {
                    subjectCN = subjectItems[i].substring(3);
                    break;
                }
            }

            // Log.d(TAG, "issuerCN: " + issuerCN);
            // Log.d(TAG, "issuerDC: " + issuerDC);
            // Log.d(TAG, "subjectCN: " + subjectCN);

            if (x509cert.getSigAlgOID().equals(WAPI_SIGN_ALG_OID) &&
                    issuerDC.equals("WAPI")) {
                if (! subjectCN.equals(issuerCN)) {
                    certType = CERT_TYPE_WAPI_USER;
                }
                else {
                    certType = CERT_TYPE_WAPI_CA;
                }
            }
            else {
                certType = CERT_TYPE_NO_WAPI;
            }

            return certType;
        } catch (Exception e) {
            return CERT_TYPE_NO_WAPI;
        }
    }
}
