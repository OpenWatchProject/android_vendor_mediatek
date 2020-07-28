package com.mediatek.simservs.xcap;

import android.util.Log;

import com.mediatek.simservs.client.SimServs;
import com.mediatek.xcap.client.XcapClient;
import com.mediatek.xcap.client.XcapDebugParam;
import com.mediatek.xcap.client.uri.XcapUri;

import com.android.okhttp.Headers;
import com.android.okhttp.Response;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.URI;
import java.net.URISyntaxException;

import javax.xml.parsers.ParserConfigurationException;

/**
 * InquireType abstract class.
 *
 */
public abstract class InquireType extends XcapElement {

    /**
     * Constructor.
     *
     * @param xcapUri           XCAP document URI
     * @param parentUri         XCAP root directory URI
     * @param intendedId        X-3GPP-Intended-Id
     * @throws XcapException    if XCAP error
     * @throws ParserConfigurationException if parser configuration error
     */
    public InquireType(XcapUri xcapUri, String parentUri, String intendedId)
            throws XcapException, ParserConfigurationException {
        super(xcapUri, parentUri, intendedId);
    }

    /**
     * Gets the content of the current node through XCAP protocol.
     *
     * @return configuration XML
     * @throws XcapException if XCAP error
     */
    public String getContent() throws XcapException {
        XcapClient xcapClient = null;
        Response response = null;
        String ret = null;
        InputStream is = null;
        Headers.Builder headers = new Headers.Builder();

        try {
            String nodeUri = getNodeUri().toString();
            XcapDebugParam debugParam = XcapDebugParam.getInstance();

            boolean simservQueryWhole = debugParam.getEnableSimservQueryWhole() ?
                    true : SimServs.sSimservQueryWhole;
            if (simservQueryWhole) {
                nodeUri = nodeUri.substring(0, nodeUri.lastIndexOf("simservs.xml") +
                        "simservs.xml".length());
            }

            URI uri = new URI(nodeUri);
            SimServs simSrv = SimServs.getInstance();

            if (mNetwork != null) {
                xcapClient = new XcapClient(simSrv.getContext(), mNetwork, simSrv.getPhoneId());

                if (xcapClient == null) {
                    throw new XcapException(500);
                }
            } else {
                xcapClient = new XcapClient(simSrv.getContext(), simSrv.getPhoneId());
            }

            if (mIntendedId != null) {
                headers.add(AUTH_XCAP_3GPP_INTENDED, "\"" + mIntendedId + "\"");
            }

            boolean disableETag = debugParam.getDisableETag() ? true : SimServs.sETagDisable;
            if (mEtag != null && !disableETag) {
                headers.add("If-None-Match", mEtag);
            }

            response = xcapClient.get(uri, headers.build());

            if (response != null) {
                if (response.code() == 200 || response.code() == 304) {
                    String etagValue = response.header("ETag");

                    if (etagValue != null) {
                        this.mIsSupportEtag = true;
                        this.mEtag = etagValue;
                    } else {
                        this.mIsSupportEtag = false;
                        this.mEtag = null;
                    }

                    if (disableETag) {
                        this.mIsSupportEtag = false;
                    }

                    if (response.code() == 200) {
                        is = null;
                        try {
                            is = response.body().byteStream();
                            // convert stream to string
                            ret = convertStreamToString(is);
                        } finally {
                            if (is != null) {
                                is.close();
                            }
                        }
                    }
                } else if (response.code() == 409) {
                    try {
                        ret = null;
                        is = response.body().byteStream();

                        if (is != null && "true".equals(System.getProperty("xcap.handl409"))) {
                            throw new XcapException(409, parse409ErrorMessage("phrase", is));
                        } else {
                            throw new XcapException(409);
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                        throw new XcapException(409);
                    } finally {
                        if (is != null) {
                            is.close();
                        }
                    }
                } else {
                    ret = null;
                    throw new XcapException(response.code());
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            throw new XcapException(e);
        } catch (URISyntaxException e) {
            e.printStackTrace();
        } finally {
            if (xcapClient != null) {
                xcapClient.shutdown();
            }
            if (response != null) {
                Log.d(TAG, "response.close()");
                try {
                    response.body().close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        Log.d(TAG, "Response XML:" + SimServs.encryptString(ret));
        return ret;
    }
}
