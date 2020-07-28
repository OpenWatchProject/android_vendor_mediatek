
package com.mediatek.simservs.capability;

import com.mediatek.simservs.xcap.InquireType;
import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.IOException;
import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

/**
 * Capability type abstract class.
 *
 */
public abstract class CapabilitiesType extends InquireType {

    static final String ATT_ACTIVE = "active";

    public boolean mActived = false;

    /**
     * Constructor.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @throws XcapException    if XCAP error
     * @throws ParserConfigurationException if parser configuration error
     */
    public CapabilitiesType(XcapUri xcapUri, String parentUri, String intendedId)
            throws XcapException, ParserConfigurationException {
        super(xcapUri, parentUri, intendedId);
        loadConfiguration();
    }

    /**
     * Instantiate from server XML text.
     *
     * @throws XcapException    if XCAP error
     * @throws ParserConfigurationException if parser configuration error
     */
    private void loadConfiguration() throws XcapException, ParserConfigurationException {
        String xmlContent = getContent();
        if (xmlContent == null) {
            return;
        }
        DocumentBuilder db = DocumentBuilderFactory.newInstance().newDocumentBuilder();
        InputSource is = new InputSource();
        is.setCharacterStream(new StringReader(xmlContent));
        Document doc;
        try {
            doc = db.parse(is);
        } catch (SAXException e) {
            e.printStackTrace();
            // Throws a server error
            throw new XcapException(500);
        } catch (IOException e) {
            e.printStackTrace();
            // Throws a server error
            throw new XcapException(500);
        }
        NodeList currentNode = doc.getElementsByTagName(getNodeName());

        if (currentNode.getLength() > 0) {
            Element activeElement = (Element) currentNode.item(0);
            NamedNodeMap map = activeElement.getAttributes();
            if (map.getLength() > 0) {
                for (int i = 0; i < map.getLength(); i++) {
                    Node node = map.item(i);
                    if (node.getNodeName().equals(ATT_ACTIVE)) {
                        mActived = node.getNodeValue().endsWith(TRUE);
                        break;
                    }
                }
            }
        }
        initServiceInstance(doc);
    }

    /**
     * Decide Active by attribute.
     *
     * @return active value
     * @throws XcapException    if XCAP error
     */
    public boolean isActive() throws XcapException {
        String value = getByAttrName("active");
        if (value == null) {
            return true;
        } else {
            return getByAttrName("active").equals("true");
        }
    }

    /**
     * Set Active.
     *
     * @param active           active value
     * @throws XcapException    if XCAP error
     */
    public void setActive(boolean active) throws XcapException {
        if (active) {
            setByAttrName("active", "true");
        } else {
            setByAttrName("active", "false");
        }
    }

    /**
     * Instantiate from XML text.
     *
     * @param domDoc  DOM document source
     */
    public abstract void initServiceInstance(Document domDoc);

}
