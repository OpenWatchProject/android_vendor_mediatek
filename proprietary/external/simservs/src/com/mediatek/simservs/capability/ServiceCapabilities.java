package com.mediatek.simservs.capability;

import com.mediatek.simservs.xcap.XcapElement;
import com.mediatek.xcap.client.uri.XcapUri;

/**
 * Service Capability class.
 */
public class ServiceCapabilities extends XcapElement {

    public static final String ATT_PROVISIONED = "provisioned";

    /**
     * Construct ServiceCapabilities instance.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     */
    public ServiceCapabilities(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
    }

    @Override
    protected String getNodeName() {
        return null;
    }

}
