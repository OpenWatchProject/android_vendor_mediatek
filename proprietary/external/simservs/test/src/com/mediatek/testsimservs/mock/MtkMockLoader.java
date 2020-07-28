package com.mediatek.testsimservs.mock;

import android.util.Log;
import org.w3c.dom.*;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.StringReader;

/**
 * Created by mtk11301 on 11/2/16.
 */
public class MtkMockLoader {
    protected static final String XCAP_NAMESPACE =
            "http://uri.etsi.org/ngn/params/xml/simservs/xcap";
    // this load is same as SimservType, from this solution we should must make sure
    // these method is same.
    public Document loadConfiguration(String xmlContent, String nodeName, boolean active)
            throws com.mediatek.simservs.xcap.XcapException,
            ParserConfigurationException {
        Log.d("MtkMockLoader", "xmlContent:" + xmlContent + " nodeName:" + nodeName + " active:" + active);
        Document doc =null;
        if (xmlContent != null) {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setNamespaceAware(true);
            DocumentBuilder db = factory.newDocumentBuilder();
            InputSource is = new InputSource();
            is.setCharacterStream(new StringReader(xmlContent));

            try {
                doc = db.parse(is);
            } catch (SAXException e) {
                factory.setNamespaceAware(false);
                db = factory.newDocumentBuilder();
                is = new InputSource();
                is.setCharacterStream(new StringReader(xmlContent));
                try {
                    doc = db.parse(is);
                } catch (SAXException err) {
                    err.printStackTrace();
                    // Throws a server error
                    throw new com.mediatek.simservs.xcap.XcapException(500);
                } catch (IOException err) {
                    err.printStackTrace();
                    // Throws a server error
                    throw new com.mediatek.simservs.xcap.XcapException(500);
                }
            } catch (IOException e) {
                e.printStackTrace();
                // Throws a server error
                throw new com.mediatek.simservs.xcap.XcapException(500);
            }

            NodeList currentNode = doc.getElementsByTagName(nodeName);

            if (currentNode.getLength() > 0) {
                Element activeElement = (Element) currentNode.item(0);
                NamedNodeMap map = activeElement.getAttributes();
                if (map.getLength() > 0) {
                    for (int i = 0; i < map.getLength(); i++) {
                        Node node = map.item(i);
                        if (node.getNodeName().equals("active")) {
                            active = node.getNodeValue().endsWith("true");
                            break;
                        }
                    }
                }
            } else {
                currentNode = doc.getElementsByTagNameNS(XCAP_NAMESPACE, nodeName);
                if (currentNode.getLength() > 0) {
                    Element activeElement = (Element) currentNode.item(0);
                    NamedNodeMap map = activeElement.getAttributes();
                    if (map.getLength() > 0) {
                        for (int i = 0; i < map.getLength(); i++) {
                            Node node = map.item(i);
                            if (node.getNodeName().equals("active")) {
                                active = node.getNodeValue().endsWith("true");
                                break;
                            }
                        }
                    }
                }
            }

        }
        return doc;
    }
}