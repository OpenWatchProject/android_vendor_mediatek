
package com.mediatek.simservs.capability;

import com.mediatek.simservs.xcap.ConfigureType;
import com.mediatek.simservs.xcap.XcapElement;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.util.LinkedList;
import java.util.List;

/**
 * Media Conditions class.
 */
public class MediaConditions extends XcapElement implements ConfigureType {
    public static final String NODE_NAME = "serv-cap-media";

    static final String TAG_MEDIA = "media";

    List<String> mMedias;

    /**
     * Construct without XML.
     *
     * @param xcapUri           XCAP document URI
     * @param parentUri         XCAP root directory URI
     * @param intendedId        X-3GPP-Intended-Id
     */
    public MediaConditions(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
    }

    /**
     * Construct with XML.
     *
     * @param xcapUri           XCAP document URI
     * @param parentUri         XCAP root directory URI
     * @param intendedId        X-3GPP-Intended-Id
     * @param domElement        XML element
     */
    public MediaConditions(XcapUri xcapUri, String parentUri, String intendedId,
            Element domElement) {
        super(xcapUri, parentUri, intendedId);
        instantiateFromXmlNode(domElement);
    }

    @Override
    public void instantiateFromXmlNode(Node domNode) {
        Element domElement = (Element) domNode;
        NodeList mediasNode = domElement.getElementsByTagName(TAG_MEDIA);
        mMedias = new LinkedList<String>();
        for (int i = 0; i < mediasNode.getLength(); i++) {
            Element mediaElement = (Element) mediasNode.item(i);
            mMedias.add(mediaElement.getTextContent());
        }

    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    public List<String> getMedias() {
        return mMedias;
    }
}
