
package com.mediatek.simservs.capability;

import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import javax.xml.parsers.ParserConfigurationException;

/**
 * Barring Service Capability class.
 *
 */
public class BarringServiceCapability extends CapabilitiesType {

    public static final String NODE_NAME = "communication-barring-serv-cap";

    ConditionCapabilities mConditionCapabilities;

    /**
     * Constructor.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @throws XcapException    if XCAP error
     * @throws ParserConfigurationException if parser configuration error
     */
    public BarringServiceCapability(XcapUri xcapUri, String parentUri, String intendedId)
            throws XcapException, ParserConfigurationException {
        super(xcapUri, parentUri, intendedId);
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    public ConditionCapabilities getConditionCapabilities() {
        return mConditionCapabilities;
    }

    @Override
    public void initServiceInstance(Document domDoc) {
        NodeList conditionsNode = domDoc.getElementsByTagName(ConditionCapabilities.NODE_NAME);
        if (conditionsNode.getLength() > 0) {
            Element conditionNode = (Element) conditionsNode.item(0);
            mConditionCapabilities = new ConditionCapabilities(mXcapUri, NODE_NAME, mIntendedId,
                    conditionNode);
        }

    }
}
