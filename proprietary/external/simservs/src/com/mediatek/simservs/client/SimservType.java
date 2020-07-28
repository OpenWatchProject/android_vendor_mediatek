package com.mediatek.simservs.client;

import android.util.Log;

import com.mediatek.simservs.client.policy.RuleSet;
import com.mediatek.simservs.xcap.InquireType;
import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.XcapDebugParam;
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
import java.net.URISyntaxException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

/**
 * SimservType (SS type) abstract class.
 */
public abstract class SimservType extends InquireType {
    //[MMTelSS] Add by mtk01411 for testing/debuging
    public static final String TAG = "SimservType";
    static final String ATT_ACTIVE = "active";

    public boolean mActived = false;
    protected String mPrefix = null;
    protected boolean mHasXcapNS = false;
    String mSsTc;

    /**
     * Constructor.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @throws XcapException if XCAP error
     * @throws ParserConfigurationException if parser configuration error
     */
    public SimservType(XcapUri xcapUri, String parentUri, String intendedId)
            throws XcapException, ParserConfigurationException {
        super(xcapUri, parentUri, intendedId);
        Log.d(TAG, "Xcap debug params: \n" + mDebugParams.toString());
    }

    /**
     * Sync latest version with NAF server.
     *
     * @throws Exception if error
     */
    public void refresh() throws Exception {
        this.loadConfiguration();
    }

    /**
     * Is E-Tag supported.
     *
     * @return true/false
     */
    public boolean isSupportEtag() {
        return mIsSupportEtag;
    }

    /**
     * Retrieve configuration.
     *
     * @throws XcapException    if XCAP error
     * @throws ParserConfigurationException if parser configuration error
     */
    //[Revise this API for UT by mtk01411]
    protected void loadConfiguration() throws XcapException,
            ParserConfigurationException {
        String xmlContent = "";
        String nodeName = getNodeName();
        Log.d(TAG, "loadConfiguration():nodeName=" + nodeName);

        if (XcapDebugParam.getInstance().getEnablePredefinedSimservQueryResult()) {
            //Use different XML strings for UT purpose
            xmlContent = readXmlFromFile("/data/ss.xml");

            //Compare if the nodeName is same with tested xml node name
            if (xmlContent.contains(nodeName) == false) {
                //Can't query anything - not same query target
                Log.d(TAG, "loadConfiguration():fail to get tested xml for nodeName=" + nodeName);
                return;
            } else {
                Log.d(TAG, "loadConfiguration():get tested xml for nodeName=" + nodeName);
            }

        } else {
            //Get string from remote XCAP server
            xmlContent = getContent();
        }

        Log.v(TAG, "xmlContent=" + SimServs.encryptString(xmlContent));

        if (xmlContent != null) {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setNamespaceAware(true);
            DocumentBuilder db = factory.newDocumentBuilder();
            InputSource is = new InputSource();
            is.setCharacterStream(new StringReader(xmlContent));
            Document doc;
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
                    throw new XcapException(500);
                } catch (IOException err) {
                    err.printStackTrace();
                    // Throws a server error
                    throw new XcapException(500);
                }
            } catch (IOException e) {
                e.printStackTrace();
                // Throws a server error
                throw new XcapException(500);
            }
            NodeList currentNode = doc.getElementsByTagName(getNodeName());
            Log.v(TAG, "getNodeName()=" + SimServs.encryptString(getNodeName()));

            if (currentNode.getLength() > 0) {
                Element activeElement = (Element) currentNode.item(0);
                NamedNodeMap map = activeElement.getAttributes();
                mPrefix = currentNode.item(0).getPrefix();
                mHasXcapNS = false;
                Log.d("loadConfiguration", "Got " + getNodeName());
                if (map.getLength() > 0) {
                    for (int i = 0; i < map.getLength(); i++) {
                        Node node = map.item(i);
                        if (node.getNodeName().equals(ATT_ACTIVE)) {
                            mActived = node.getNodeValue().endsWith(TRUE);
                            break;
                        }
                    }
                }
            } else {
                currentNode = doc.getElementsByTagNameNS(XCAP_NAMESPACE, getNodeName());
                if (currentNode.getLength() > 0) {
                    Element activeElement = (Element) currentNode.item(0);
                    NamedNodeMap map = activeElement.getAttributes();
                    mPrefix = currentNode.item(0).getPrefix();
                    mHasXcapNS = true;
                    Log.i("loadConfiguration", "Got " + getNodeName() + ":" + XCAP_NAMESPACE
                            + ", prefix=" + mPrefix + ", NS=" + mHasXcapNS);
                    if (map.getLength() > 0) {
                        for (int i = 0; i < map.getLength(); i++) {
                            Node node = map.item(i);
                            if (node.getNodeName().equals(ATT_ACTIVE)) {
                                mActived = node.getNodeValue().endsWith(TRUE);
                                break;
                            }
                        }
                    }
                } else {
                    currentNode = doc.getElementsByTagName(XCAP_ALIAS + ":" + getNodeName());
                    if (currentNode.getLength() > 0) {
                        Element activeElement = (Element) currentNode.item(0);
                        NamedNodeMap map = activeElement.getAttributes();
                        mPrefix = currentNode.item(0).getPrefix();
                        mHasXcapNS = false;
                        Log.i("loadConfiguration", "Got " + XCAP_ALIAS + ":" + getNodeName());
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
                }
            }

            Log.v(TAG, "xmldoc=" + SimServs.encryptString(doc.toString()));

            initServiceInstance(doc);
        }
    }

    public boolean isActive() {
        return mActived;
    }

    /**
     * Set active value.
     *
     * @param active            active value
     * @throws XcapException    if XCAP error
     */
    public void setActive(boolean active) throws XcapException {
        mActived = active;
        String xml = null;
        String useXcapNs = System.getProperty("xcap.ns.ss", "false");
        boolean quotationMarkNeeded = "true".equals(
                System.getProperty("xcap.attr.active.quote", "false"));

        if ("true".equals(useXcapNs)) {
            if (mActived) {
                xml = "<ss:" + this.getNodeName() + " active=\"true\"/>";
            } else {
                xml = "<ss:" + this.getNodeName() + " active=\"false\"/>";
            }
            this.setContent(xml);
        } else {
            if (mActived) {
                setByAttrName(ATT_ACTIVE, quotationMarkNeeded ? TRUE_WITH_QUOTE : TRUE);
            } else {
                setByAttrName(ATT_ACTIVE, quotationMarkNeeded ? FALSE_WITH_QUOTE : FALSE);
            }
        }
    }

    public void saveNode(boolean active, RuleSet ruleSet) throws XcapException {
        String useXcapNs = System.getProperty("xcap.ns.ss", "false");
        useXcapNs = ("true".equals(useXcapNs) ? "ss:" : "");
        String thisXml = "<" + useXcapNs + getNodeName() + " "
            + (useXcapNs.equals("ss:") ?
                "xmlns:ss=\"http://uri.etsi.org/ngn/params/xml/simservs/xcap\" " : "")
            + ATT_ACTIVE + "=\"" + active + "\">"
            + (ruleSet != null && ruleSet.getRules().size() != 0 ? ruleSet.toXmlString() : "")
               + "</" + useXcapNs + getNodeName() + ">";
        Log.d(TAG, "saveNode: thisXml=" + SimServs.encryptString(thisXml));
        setContent(thisXml);
    }

    /**
     * Sets the content of the current node.
     *
     * @param  xml XML string
     * @throws XcapException if XCAP error
     */
    @Override
    public void setContent(String xml) throws XcapException {
        try {
            String useXcapNs = System.getProperty("xcap.ns.ss", "false");
            useXcapNs = ("true".equals(useXcapNs) ? "ss:" : "");
            mNodeUri = getNodeUri().toString();
            mNodeUri = mNodeUri.replace(getNodeName(), useXcapNs + getNodeName());
            mNodeUri = mNodeUri + (useXcapNs.equals("ss:") ?
                "?xmlns(ss=http://uri.etsi.org/ngn/params/xml/simservs/xcap)" : "");
            Log.d(TAG, "setContent: mNodeUri=" + SimServs.encryptString(mNodeUri));
            saveContent(xml);
        } catch (URISyntaxException e) {
            e.printStackTrace();
        }
    }

    /**
     * Instantiate from XML text.
     *
     * @param domDoc XML document
     */
    public abstract void initServiceInstance(Document domDoc);
}
