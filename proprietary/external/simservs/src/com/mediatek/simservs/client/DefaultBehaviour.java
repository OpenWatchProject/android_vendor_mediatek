package com.mediatek.simservs.client;

import com.mediatek.simservs.xcap.XcapElement;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

/**
 * Default Behaviour class.
 *
 */
public class DefaultBehaviour extends XcapElement {

    public static final String NODE_NAME = "default-behaviour";

    public static final String DEFAULT_BEHAVIOUR_PRESENTATION_RESTRICTED =
            "presentation-restricted";
    public static final String DEFAULT_BEHAVIOUR_PRESENTATION_NOT_RESTRICTED =
            "presentation-not-restricted";

    public boolean mPresentationRestricted;

    /**
     * Constructor.
     *
     * @param cdUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     */
    public DefaultBehaviour(XcapUri cdUri, String parentUri, String intendedId) {
        super(cdUri, parentUri, intendedId);
    }

    /**
     * Constructor.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @param domElement    XML element
     */
    public DefaultBehaviour(XcapUri xcapUri, String parentUri, String intendedId,
            Element domElement) {
        super(xcapUri, parentUri, intendedId);
        String content = domElement.getTextContent();
        mPresentationRestricted = content.equals(DEFAULT_BEHAVIOUR_PRESENTATION_RESTRICTED);
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    /**
     * Convert to XML string.
     *
     * @return  XML string
     */
    public String toXmlString() {
        if (mPresentationRestricted) {
            return "<default-behaviour>" + DEFAULT_BEHAVIOUR_PRESENTATION_RESTRICTED
                    + "</default-behaviour>";
        } else {
            return "<default-behaviour>" + DEFAULT_BEHAVIOUR_PRESENTATION_NOT_RESTRICTED
                    + "</default-behaviour>";
        }
    }

    /**
     * Convert to XML element.
     *
     * @param   document    DOM document
     * @return  XML element
     */
    public Element toXmlElement(Document document) {
        Element defaultElement = (Element) document.createElement(NODE_NAME);
        if (mPresentationRestricted) {
            defaultElement.setTextContent(DEFAULT_BEHAVIOUR_PRESENTATION_RESTRICTED);
        } else {
            defaultElement.setTextContent(DEFAULT_BEHAVIOUR_PRESENTATION_NOT_RESTRICTED);
        }
        return defaultElement;
    }

    public boolean isPresentationRestricted() {
        return mPresentationRestricted;
    }

    public void setPresentationRestricted(boolean presentationRestricted) {
        mPresentationRestricted = presentationRestricted;
    }
}
