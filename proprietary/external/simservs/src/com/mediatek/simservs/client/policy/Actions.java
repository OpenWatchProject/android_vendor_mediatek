package com.mediatek.simservs.client.policy;

import android.util.Log;

import com.mediatek.simservs.xcap.ConfigureType;
import com.mediatek.simservs.xcap.XcapElement;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Actions class.
 *
 */
public class Actions extends XcapElement implements ConfigureType {

    public static final String NODE_NAME = "cp:actions";

    static final String TAG_ALLOW = "allow";
    static final String TAG_FORWARD_TO = "forward-to";

    public boolean mAllow;
    private boolean mAppendAllow = false;
    public ForwardTo mForwardTo;
    public NoReplyTimer mNoReplyTimer;

    /**
     * Constructor without XML element.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     */
    public Actions(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
        mForwardTo = new ForwardTo(xcapUri, parentUri, intendedId);
    }

    /**
     * Constructor with XML element.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @param domElement    DOM XML element
     */
    public Actions(XcapUri xcapUri, String parentUri, String intendedId,
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
        NodeList actionNode = domElement.getElementsByTagName(TAG_ALLOW);
        if (actionNode.getLength() > 0) {
            Element allowElement = (Element) actionNode.item(0);
            String allowed = allowElement.getTextContent();
            mAllow = allowed.equals("true");
            mAppendAllow = true;
        } else {
            actionNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_ALLOW);
            if (actionNode.getLength() > 0) {
                Element allowElement = (Element) actionNode.item(0);
                String allowed = allowElement.getTextContent();
                mAllow = allowed.equals("true");
                mAppendAllow = true;
            } else {
                actionNode = domElement.getElementsByTagName(XCAP_ALIAS + ":" + TAG_ALLOW);
                if (actionNode.getLength() > 0) {
                    Element allowElement = (Element) actionNode.item(0);
                    String allowed = allowElement.getTextContent();
                    mAllow = allowed.equals("true");
                    mAppendAllow = true;
                }
            }
        }

        actionNode = domElement.getElementsByTagName(TAG_FORWARD_TO);
        if (actionNode.getLength() > 0) {
            mForwardTo = new ForwardTo(mXcapUri, NODE_NAME, mIntendedId,
                    domElement);
        } else {
            actionNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_FORWARD_TO);
            if (actionNode.getLength() > 0) {
                mForwardTo = new ForwardTo(mXcapUri, NODE_NAME, mIntendedId,
                        domElement);
            } else {
                actionNode = domElement.getElementsByTagName(XCAP_ALIAS + ":" + TAG_FORWARD_TO);
                if (actionNode.getLength() > 0) {
                    mForwardTo = new ForwardTo(mXcapUri, NODE_NAME, mIntendedId,
                            domElement);
                }
            }
        }

        actionNode = domElement.getElementsByTagName(NoReplyTimer.NODE_NAME);
        if (actionNode.getLength() > 0) {
            Log.d("Actions", "Got NoReplyTimer");
            Element noReplyTimerElement = (Element) actionNode.item(0);
            String val = noReplyTimerElement.getTextContent();
            int noReplyTimer = Integer.parseInt(val);
            mNoReplyTimer = new NoReplyTimer(mXcapUri, NoReplyTimer.NODE_NAME, mIntendedId,
                    noReplyTimer);
        } else {
            actionNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, NoReplyTimer.NODE_NAME);
            if (actionNode.getLength() > 0) {
                Log.d("Actions", "Got NoReplyTimer with xcap namespace");
                Element noReplyTimerElement = (Element) actionNode.item(0);
                String val = noReplyTimerElement.getTextContent();
                int noReplyTimer = Integer.parseInt(val);
                mNoReplyTimer = new NoReplyTimer(mXcapUri, NoReplyTimer.NODE_NAME, mIntendedId,
                        noReplyTimer);
            } else {
                actionNode = domElement.getElementsByTagName(XCAP_ALIAS + ":" + NoReplyTimer.NODE_NAME);
                if (actionNode.getLength() > 0) {
                    Log.d("Actions", "Got ss:NoReplyTimer");
                    Element noReplyTimerElement = (Element) actionNode.item(0);
                    String val = noReplyTimerElement.getTextContent();
                    int noReplyTimer = Integer.parseInt(val);
                    mNoReplyTimer = new NoReplyTimer(mXcapUri, NoReplyTimer.NODE_NAME, mIntendedId,
                            noReplyTimer);
                }
            }
        }

    }

    /**
     * Convert to XML element.
     *
     * @param document  dom document
     * @return XML format element
     */
    public Element toXmlElement(Document document) {
        Element actionsElement = (Element) document.createElement(NODE_NAME);

        if (mForwardTo != null) {
            Element forwardToElement = mForwardTo.toXmlElement(document);
            actionsElement.appendChild(forwardToElement);
        } else {
            if (mAppendAllow) {
                String useXcapNs = System.getProperty("xcap.ns.ss", "false");

                if ("true".equals(useXcapNs)) {
                    Element allowElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                            "ss:" + TAG_ALLOW);
                    allowElement.setTextContent(mAllow ? "true" : "false");
                    actionsElement.appendChild(allowElement);
                } else {
                    Element allowElement = (Element) document.createElement(TAG_ALLOW);
                    allowElement.setTextContent(mAllow ? "true" : "false");
                    actionsElement.appendChild(allowElement);
                }
            }
        }

        if (mNoReplyTimer != null) {
            Element noReplyTimerElement = mNoReplyTimer.toXmlElement(document);
            actionsElement.appendChild(noReplyTimerElement);
        }

        return actionsElement;
    }

    public void setAllow(boolean allow) {
        mAllow = allow;
        mAppendAllow = true;
    }

    public boolean isAllow() {
        return mAllow;
    }

    /**
     * Set ForwardTo value.
     *
     * @param target        forward number
     * @param notifyCaller  whether to notify caller
     */
    public void setFowardTo(String target, boolean notifyCaller) {
        if (mForwardTo == null) {
            mForwardTo = new ForwardTo(mXcapUri, mParentUri, mIntendedId);
        }
        mForwardTo.setTarget(target);
        mForwardTo.setNotifyCaller(notifyCaller);
    }

    public ForwardTo getFowardTo() {
        return mForwardTo;
    }

    public void setNoReplyTimer(int value) {
        if (mNoReplyTimer != null) {
            mNoReplyTimer.setValue(value);
        } else if(value != -1) {
            mNoReplyTimer = new NoReplyTimer(mXcapUri, NoReplyTimer.NODE_NAME, mIntendedId,
                    value);
        }
    }

    public int getNoReplyTimer() {
        if (mNoReplyTimer != null) {
            return mNoReplyTimer.getValue();
        }
        return -1;
    }

    /**
     * NoReplyTimer class.
     *
     */
    public class NoReplyTimer extends XcapElement {

        public static final String NODE_NAME = "NoReplyTimer";
        public int mValue;

        /**
         * Constructor without initial time value.
         *
         * @param cdUri       XCAP document URI
         * @param parentUri   XCAP root directory URI
         * @param intendedId  X-3GPP-Intended-Id
         */
        public NoReplyTimer(XcapUri cdUri, String parentUri, String intendedId) {
            super(cdUri, parentUri, intendedId);
        }

        /**
         * Constructor with initial time value.
         *
         * @param cdUri       XCAP document URI
         * @param parentUri   XCAP root directory URI
         * @param intendedId  X-3GPP-Intended-Id
         * @param initValue   time value
         */
        public NoReplyTimer(XcapUri cdUri, String parentUri, String intendedId, int initValue) {
            super(cdUri, parentUri, intendedId);
            mValue = initValue;
            Log.d("Actions", "new NoReplyTimer  mValue=" + mValue);
        }

        @Override
        protected String getNodeName() {
            return NODE_NAME;
        }

        public int getValue() {
            return mValue;
        }

        public void setValue(int value) {
            mValue = value;
        }

        /**
         * Convert to XML string.
         *
         * @return XML string
         */
        public String toXmlString() {
            return "<NoReplyTimer>" + String.valueOf(mValue)
                    + "</NoReplyTimer>";
        }

        /**
         * Convert to XML element.
         *
         * @param document DOM document
         * @return XML element
         */
        public Element toXmlElement(Document document) {

            String useXcapNs = System.getProperty("xcap.ns.ss", "false");

            if ("true".equals(useXcapNs)) {
                Element noReplyTimerElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + NODE_NAME);
                noReplyTimerElement.setTextContent(String.valueOf(mValue));
                return noReplyTimerElement;
            } else {
                Element noReplyTimerElement = (Element) document.createElement(NODE_NAME);
                noReplyTimerElement.setTextContent(String.valueOf(mValue));
                return noReplyTimerElement;
            }
        }

    }

}
