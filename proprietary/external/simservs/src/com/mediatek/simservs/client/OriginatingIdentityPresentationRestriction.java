
package com.mediatek.simservs.client;

import android.net.Network;
import android.util.Log;

import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;

/**
 * Originating Identity Presentation Restriction class.
 */
public class OriginatingIdentityPresentationRestriction extends SimservType {

    public static final String NODE_NAME = "originating-identity-presentation-restriction";
    public static int NODE_ROOT_FULL_CHILD = 0;
    public static int NODE_ROOT_NO_CHILD = 1;
    public static int NODE_DEFAULT_BEHAVIOUR = 2;
    public DefaultBehaviour mDefaultBehaviour;
    public boolean mContainDefaultBehaviour = false;
    public boolean mShowActivePara = false;
    public int mNodeSelector = NODE_ROOT_FULL_CHILD;

    /**
     * Constructor.
     *
     * @param documentUri       XCAP document URI
     * @param parentUri         XCAP root directory URI
     * @param xui               X-3GPP-Intended-Id
     * @throws  Exception       if error
     */
    public OriginatingIdentityPresentationRestriction(XcapUri documentUri, String parentUri,
            String xui) throws Exception {
        super(documentUri, parentUri, xui);
    }

    @Override
    public void initServiceInstance(Document domDoc) {
        NodeList defaultBehaviour = domDoc.getElementsByTagName(DefaultBehaviour.NODE_NAME);
        if (defaultBehaviour.getLength() > 0) {
            mContainDefaultBehaviour = true;
            Element defaultBehaviourElement = (Element) defaultBehaviour.item(0);
            mDefaultBehaviour = new DefaultBehaviour(mXcapUri, NODE_NAME, mIntendedId,
                    defaultBehaviourElement);

            if (mNetwork != null) {
                mDefaultBehaviour.setNetwork(mNetwork);
            }
        } else {
            defaultBehaviour = domDoc.getElementsByTagNameNS(XCAP_NAMESPACE,
                    DefaultBehaviour.NODE_NAME);
            if (defaultBehaviour.getLength() > 0) {
                mContainDefaultBehaviour = true;
                Element defaultBehaviourElement = (Element) defaultBehaviour.item(0);
                mDefaultBehaviour = new DefaultBehaviour(mXcapUri, NODE_NAME, mIntendedId,
                        defaultBehaviourElement);

                if (mNetwork != null) {
                    mDefaultBehaviour.setNetwork(mNetwork);
                }
            } else {
                mDefaultBehaviour = new DefaultBehaviour(mXcapUri, NODE_NAME, mIntendedId);

                if (mNetwork != null) {
                    mDefaultBehaviour.setNetwork(mNetwork);
                }
            }
        }
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    /**
     * Save configuration to server.
     *
     * @throws XcapException    if XCAP error
     */
    public void saveConfiguration() throws XcapException {
        String serviceXml = toXmlString();
        if (serviceXml == null) {
            Log.e(TAG, "saveConfiguration: null xml string");
            return;
        }
        setContent(serviceXml);
        mContainDefaultBehaviour = true;
    }

    /**
     * Convert DOM to XML string.
     *
     * @return XML string
     */
    public String toXmlString() {
        Element root = null;
        String xmlString = null;
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.newDocument();
            root = (Element) document.createElement(NODE_NAME);
            Log.d(TAG, "toXmlString: mShowActivePara=" + mShowActivePara
                + ", mActived=" + mActived
                + ", mNodeSelector=" + mNodeSelector);
            if (mShowActivePara) {
                root.setAttribute(ATT_ACTIVE, String.valueOf(mActived));
            }
            document.appendChild(root);
            if (mNodeSelector != NODE_ROOT_NO_CHILD) {
                Element defaultElement = mDefaultBehaviour.toXmlElement(document);
                root.appendChild(defaultElement);
            }
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

    public boolean isContainDefaultBehaviour() {
        return mContainDefaultBehaviour;
    }

    public boolean isDefaultPresentationRestricted() {
        return mDefaultBehaviour.isPresentationRestricted();
    }

    /**
     * set Default Presentation Restricted value.
     *
     * @param presentationRestricted default restricted value
     * @throws XcapException if XCAP error
     */
    public void setDefaultPresentationRestricted(boolean presentationRestricted) throws
            XcapException {
        mDefaultBehaviour.setPresentationRestricted(presentationRestricted);

        if (isDefaultPresentationRestricted()) {
            String defaultBehaviourXml = mDefaultBehaviour.toXmlString();
            mDefaultBehaviour.setContent(defaultBehaviourXml);
        } else {
            saveConfiguration();
        }
    }

    public void setDefaultPresentationRestricted(boolean presentationRestricted,
            boolean nodeActive, int nodeSelector, boolean showActivePara) throws XcapException {
        mDefaultBehaviour.setPresentationRestricted(presentationRestricted);
        mActived = nodeActive;
        mShowActivePara = showActivePara;
        mNodeSelector = nodeSelector;

        if (mNodeSelector == NODE_DEFAULT_BEHAVIOUR) {
            String defaultBehaviourXml = mDefaultBehaviour.toXmlString();
            mDefaultBehaviour.setContent(defaultBehaviourXml);
        } else {
            saveConfiguration();
        }
    }

    /**
     * Set dedicated network.
     *
     * @param network dedicated network
     */
    @Override
    public void setNetwork(Network network) {
        super.setNetwork(network);
        if (network != null && mDefaultBehaviour != null) {
            Log.i(TAG, "XCAP dedicated network netid to mDefaultBehaviour: " + network);
            mDefaultBehaviour.setNetwork(network);
        }
    }
}
