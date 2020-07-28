package com.mediatek.simservs.capability;

import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import javax.xml.parsers.ParserConfigurationException;

/**
 * Communication Diversion Capability class.
 *
 */
public class DiversionServiceCapability extends CapabilitiesType {
    public static final String NODE_NAME = "communication-diversion-serv-cap";

    ConditionCapabilities mConditionCapabilities;
    ActionCapabilities mActionCapabilities;

    /**
     * Constructor.
     *
     * @param documentUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @throws XcapException    if XCAP error
     * @throws ParserConfigurationException if parser configuration error
     */
    public DiversionServiceCapability(XcapUri documentUri, String parentUri, String intendedId)
            throws XcapException, ParserConfigurationException {
        super(documentUri, parentUri, intendedId);
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    /**
     * Get available condition configuration items.
     *
     * @return  condition capabilities
     */
    public ConditionCapabilities getConditionCapabilities() {
        return mConditionCapabilities;
    }

    /**
     * Get available action configuration items.
     *
     * @return  action capabilities
     */
    public ActionCapabilities getActionCapabilities() {
        return mActionCapabilities;
    }

    @Override
    public void initServiceInstance(Document domDoc) {
        NodeList actionsNode = domDoc.getElementsByTagName(ActionCapabilities.NODE_NAME);
        if (actionsNode.getLength() > 0) {
            Element actionNode = (Element) actionsNode.item(0);
            mActionCapabilities = new ActionCapabilities(mXcapUri, NODE_NAME, mIntendedId,
                    actionNode);
        }

        NodeList conditionsNode = domDoc.getElementsByTagName(ConditionCapabilities.NODE_NAME);
        if (conditionsNode.getLength() > 0) {
            Element conditionNode = (Element) conditionsNode.item(0);
            mConditionCapabilities = new ConditionCapabilities(mXcapUri, NODE_NAME, mIntendedId,
                    conditionNode);
        }

    }
}
