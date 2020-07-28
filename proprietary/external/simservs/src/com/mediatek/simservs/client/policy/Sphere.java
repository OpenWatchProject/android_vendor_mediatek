
package com.mediatek.simservs.client.policy;

import com.mediatek.simservs.xcap.XcapElement;
import com.mediatek.xcap.client.uri.XcapUri;

/**
 * Spheret class.
 */
public class Sphere extends XcapElement {

    public static final String NODE_NAME = "sphere";

    /**
     * Constructor.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     */
    public Sphere(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }
}
