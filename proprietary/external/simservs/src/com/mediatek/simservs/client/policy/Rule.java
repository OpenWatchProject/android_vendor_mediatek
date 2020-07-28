
package com.mediatek.simservs.client.policy;

import android.util.Log;

import com.mediatek.simservs.xcap.ConfigureType;
import com.mediatek.simservs.xcap.XcapElement;
import com.mediatek.simservs.xcap.XcapException;

import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.net.URISyntaxException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;

/**
 * Rule class.
 */
public class Rule extends XcapElement implements ConfigureType {
    public static final String NODE_NAME = "cp:rule";
    public static final String NODE_XML_NAMESPACE =
            "?xmlns(" + COMMON_POLICY_ALIAS + "=" + COMMON_POLICY_NAMESPACE + ")";

    public String mId = "none";
    public Conditions mConditions;
    public Actions mActions;

    /**
     * Constructor.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri         XCAP root directory URI
     * @param intendedId        X-3GPP-Intended-Id
     */
    public Rule(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
    }

    /**
     * Constructor with XML element.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @param domElement    DOM element
     */
    public Rule(XcapUri xcapUri, String parentUri, String intendedId,
            Element domElement) {
        super(xcapUri, parentUri, intendedId);
        instantiateFromXmlNode(domElement);
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    @Override
    public void instantiateFromXmlNode(Node domNode) {
        Element domElement = (Element) domNode;
        mId = domElement.getAttribute("id");
        NodeList conditionsNode = domElement.getElementsByTagName("conditions");
        if (conditionsNode.getLength() > 0) {
            mConditions = new Conditions(mXcapUri, NODE_NAME, mIntendedId,
                    (Element) conditionsNode.item(0));
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(COMMON_POLICY_NAMESPACE,
                    "conditions");
            if (conditionsNode.getLength() > 0) {
                mConditions = new Conditions(mXcapUri, NODE_NAME, mIntendedId,
                        (Element) conditionsNode.item(0));
            } else {
                conditionsNode = domElement.getElementsByTagName("cp:conditions");
                if (conditionsNode.getLength() > 0) {
                    mConditions = new Conditions(mXcapUri, NODE_NAME, mIntendedId,
                            (Element) conditionsNode.item(0));
                } else {
                    mConditions = new Conditions(mXcapUri, NODE_NAME, mIntendedId);
                }
            }
        }

        NodeList actionsNode = domElement.getElementsByTagName("actions");
        if (actionsNode.getLength() > 0) {
            mActions = new Actions(mXcapUri, NODE_NAME, mIntendedId,
                    (Element) actionsNode.item(0));
        } else {
            actionsNode = domElement.getElementsByTagNameNS(COMMON_POLICY_NAMESPACE, "actions");
            if (actionsNode.getLength() > 0) {
                mActions = new Actions(mXcapUri, NODE_NAME, mIntendedId,
                        (Element) actionsNode.item(0));
            } else {
                actionsNode = domElement.getElementsByTagName("cp:actions");
                if (actionsNode.getLength() > 0) {
                    mActions = new Actions(mXcapUri, NODE_NAME, mIntendedId,
                            (Element) actionsNode.item(0));
                } else {
                    mActions = new Actions(mXcapUri, NODE_NAME, mIntendedId);
                }
            }
        }
    }

    /**
     * Convert to XML element.
     *
     * @param  document DOM document
     * @return DOM element
     * @throws TransformerException if conversion error
     */
    public Element toXmlElement(Document document) throws TransformerException {
        Element ruleElement = (Element) document.createElement(NODE_NAME);
        ruleElement.setAttribute("id", mId);

        if (mConditions != null) {
            Element conditionsElement = mConditions.toXmlElement(document);
            ruleElement.appendChild(conditionsElement);
        }

        if (mActions != null) {
            Element actionsElement = mActions.toXmlElement(document);
            ruleElement.appendChild(actionsElement);
        }
        return ruleElement;
    }

    /**
     * Create actions.
     *
     * @return actions
     */
    public Actions createActions() {
        if (mActions == null) {
            mActions = new Actions(mXcapUri, NODE_NAME, mIntendedId);
        }
        return mActions;
    }

    /**
     * Create conditions.
     *
     * @return conditions
     */
    public Conditions createConditions() {
        if (mConditions == null) {
            mConditions = new Conditions(mXcapUri, NODE_NAME, mIntendedId);
        }
        return mConditions;
    }

    public void setId(String id) {
        mId = id;
    }

    public Conditions getConditions() {
        return mConditions;
    }

    public Actions getActions() {
        return mActions;
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
            mNodeUri = getNodeUri().toString();

            if (getNodeName().equals(Rule.NODE_NAME)) {
                //add rule id selector
                mNodeUri += "%5b@id=%22" + mId.replaceAll(" ", "%20") +
                        "%22%5d" + NODE_XML_NAMESPACE;
            }
            Log.d("Rule", "setContent etag=" + mEtag);
            saveContent(xml);
        } catch (URISyntaxException e) {
            e.printStackTrace();
        }
    }

    /**
     * Sets the content of the current node.
     *
     * @param  xml XML string
     * @param  hasXcapNS has XCAP namespace
     * @throws XcapException if XCAP error
     */
    public void setContent(String xml, boolean hasXcapNS) throws XcapException {
        try {
            Log.d("Rule", "setContent hasXcapNS=" + hasXcapNS);
            mNodeUri = getNodeUri().toString();

            if (getNodeName().equals(Rule.NODE_NAME)) {
                //add rule id selector and namespace
                String xcapNS = hasXcapNS ? "xmlns(" + XCAP_ALIAS + "=" + XCAP_NAMESPACE + ")" : "";
                String cpNS = "xmlns(" + COMMON_POLICY_ALIAS + "=" + COMMON_POLICY_NAMESPACE + ")";
                String namespace = "?" + xcapNS + cpNS;
                mNodeUri += "%5b@id=%22" + mId.replaceAll(" ", "%20") + "%22%5d" + namespace;
            }
            Log.d("Rule", "setContent etag=" + mEtag);
            saveContent(xml);
        } catch (URISyntaxException e) {
            e.printStackTrace();
        }
    }

    /**
     * Convert rule to XML string.
     *
     * @return  XML string
     */
    public String toXmlString() {
        Element root = null;
        String xmlString = null;
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.newDocument();
            root = (Element) this.toXmlElement(document);
            document.appendChild(root);
            xmlString = domToXmlText(root);
        } catch (ParserConfigurationException pce) {
            // Parser with specified options can't be built
            pce.printStackTrace();
        } catch (TransformerConfigurationException e) {
            e.printStackTrace();
        } catch (TransformerException e) {
            e.printStackTrace();
        }
        return xmlString;
    }

}
