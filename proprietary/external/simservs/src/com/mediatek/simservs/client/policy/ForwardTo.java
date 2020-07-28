package com.mediatek.simservs.client.policy;

import android.telephony.Rlog;
import android.os.SystemProperties;

import com.mediatek.simservs.xcap.ConfigureType;
import com.mediatek.simservs.xcap.XcapElement;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * ForwardTo class.
 *
 */
public class ForwardTo extends XcapElement implements ConfigureType {
    private static final boolean SDBG = SystemProperties.get("ro.build.type").equals("user")
            ? false : true;
    public static final String TAG = "ForwardTo";

    public static final String NODE_NAME = "forward-to";

    static final String TAG_TARGET = "target";
    static final String TAG_NOTIFY_CALLER = "notify-caller";
    static final String TAG_REVEAL_IDENTITY_TO_CALLER = "reveal-identity-to-caller";
    static final String TAG_REVEAL_SERVED_USER_IDENTITY_TO_CALLER =
            "reveal-served-user-identity-to-caller";
    static final String TAG_NOTIFY_SERVED_USER = "notify-served-user";
    static final String TAG_NOTIFY_SERVED_USER_ON_OUTBOUND_CALL =
            "notify-served-user-on-outbound-call";
    static final String TAG_REVEAL_IDENTITY_TO_TARGET = "reveal-identity-to-target";

    public String mTarget; //minOccurs=1 maxOccurs=1
    public boolean mNotifyCaller = true; //minOccurs=0
    public boolean mRevealIdentityToCaller = true; //minOccurs=0
    public boolean mRevealServedUserIdentityToCaller = true; //minOccurs=0
    public boolean mNotifyServedUser = false; //minOccurs=0
    public boolean mNotifyServedUserOnOutboundCall = false; //minOccurs=0
    public boolean mRevealIdentityToTarget = true; //minOccurs=0
    public boolean mIsValidTargetNumber = true;

    /**
     * Constructor.
     *
     * @param xcapUri   XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     */
    public ForwardTo(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
        // TODO Auto-generated constructor stub
    }

    /**
     * Constructor.
     *
     * @param xcapUri   XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @param domElement    DOM element
     */
    public ForwardTo(XcapUri xcapUri, String parentUri, String intendedId,
            Element domElement) {
        super(xcapUri, parentUri, intendedId);
        // TODO Auto-generated constructor stub

        instantiateFromXmlNode(domElement);
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    private boolean isValidTargetNumber(String uri) {
        if (uri != null && uri.startsWith("sip:") ||
            uri.startsWith("sips:") || uri.startsWith("tel") ||
            uri.startsWith("+") || uri.matches("^[0-9]+$")) {
            Rlog.d(TAG, "isValidTargetNumber = " + Rlog.pii(SDBG, uri) + ", result = true");
            return true;
        } else if (uri == null || uri.equals("")) {
            Rlog.d(TAG, "Number is empty, we should put CF number to server");
            return false;
        } else {
            Rlog.d(TAG, "isValidTargetNumber = " + Rlog.pii(SDBG, uri) + ", result = false");
            return false;
        }
    }

    @Override
    public void instantiateFromXmlNode(Node domNode) {
        Element domElement = (Element) domNode;
        NodeList forwardToNode = domElement.getElementsByTagName(TAG_TARGET);
        if (forwardToNode.getLength() > 0) {
            Element targetElement = (Element) forwardToNode.item(0);
            mTarget = targetElement.getTextContent();
            mIsValidTargetNumber = isValidTargetNumber(mTarget);
        } else {
            forwardToNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_TARGET);
            if (forwardToNode.getLength() > 0) {
                Element targetElement = (Element) forwardToNode.item(0);
                mTarget = targetElement.getTextContent();
                mIsValidTargetNumber = isValidTargetNumber(mTarget);
            } else {
                forwardToNode = domElement.getElementsByTagName(
                        XCAP_ALIAS + ":" + TAG_TARGET);
                if (forwardToNode.getLength() > 0) {
                    Element targetElement = (Element) forwardToNode.item(0);
                    mTarget = targetElement.getTextContent();
                    mIsValidTargetNumber = isValidTargetNumber(mTarget);
                }
            }
        }

        forwardToNode = domElement.getElementsByTagName(TAG_NOTIFY_CALLER);
        if (forwardToNode.getLength() > 0) {
            Element notifyCallerElement = (Element) forwardToNode.item(0);
            String notifyCaller = notifyCallerElement.getTextContent();
            mNotifyCaller = notifyCaller.equals("true");
        } else {
            forwardToNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_NOTIFY_CALLER);
            if (forwardToNode.getLength() > 0) {
                Element notifyCallerElement = (Element) forwardToNode.item(0);
                String notifyCaller = notifyCallerElement.getTextContent();
                mNotifyCaller = notifyCaller.equals("true");
            } else {
                forwardToNode = domElement.getElementsByTagName(
                        XCAP_ALIAS + ":" + TAG_NOTIFY_CALLER);
                if (forwardToNode.getLength() > 0) {
                    Element notifyCallerElement = (Element) forwardToNode.item(0);
                    String notifyCaller = notifyCallerElement.getTextContent();
                    mNotifyCaller = notifyCaller.equals("true");
                }
            }
        }

        forwardToNode = domElement.getElementsByTagName(TAG_REVEAL_IDENTITY_TO_CALLER);
        if (forwardToNode.getLength() > 0) {
            Element revealCallerElement = (Element) forwardToNode.item(0);
            String revealCaller = revealCallerElement.getTextContent();
            mRevealIdentityToCaller = revealCaller.equals("true");
        } else {
            forwardToNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE,
                    TAG_REVEAL_IDENTITY_TO_CALLER);
            if (forwardToNode.getLength() > 0) {
                Element revealCallerElement = (Element) forwardToNode.item(0);
                String revealCaller = revealCallerElement.getTextContent();
                mRevealIdentityToCaller = revealCaller.equals("true");
            } else {
                forwardToNode = domElement.getElementsByTagName(
                        XCAP_ALIAS + ":" + TAG_REVEAL_IDENTITY_TO_CALLER);
                if (forwardToNode.getLength() > 0) {
                    Element revealCallerElement = (Element) forwardToNode.item(0);
                    String revealCaller = revealCallerElement.getTextContent();
                    mRevealIdentityToCaller = revealCaller.equals("true");
                }
            }
        }

        forwardToNode = domElement.getElementsByTagName(TAG_REVEAL_IDENTITY_TO_TARGET);
        if (forwardToNode.getLength() > 0) {
            Element revealTargetElement = (Element) forwardToNode.item(0);
            String revealTarget = revealTargetElement.getTextContent();
            mRevealIdentityToTarget = revealTarget.equals("true");
        } else {
            forwardToNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE,
                    TAG_REVEAL_IDENTITY_TO_TARGET);
            if (forwardToNode.getLength() > 0) {
                Element revealTargetElement = (Element) forwardToNode.item(0);
                String revealTarget = revealTargetElement.getTextContent();
                mRevealIdentityToTarget = revealTarget.equals("true");
            } else {
                forwardToNode = domElement.getElementsByTagName(
                        XCAP_ALIAS + ":" + TAG_REVEAL_IDENTITY_TO_TARGET);
                if (forwardToNode.getLength() > 0) {
                    Element revealTargetElement = (Element) forwardToNode.item(0);
                    String revealTarget = revealTargetElement.getTextContent();
                    mRevealIdentityToTarget = revealTarget.equals("true");
                }
            }
        }

        forwardToNode = domElement.getElementsByTagName(TAG_REVEAL_SERVED_USER_IDENTITY_TO_CALLER);
        if (forwardToNode.getLength() > 0) {
            Element element = (Element) forwardToNode.item(0);
            String str = element.getTextContent();
            mRevealServedUserIdentityToCaller = str.equals("true");
        } else {
            forwardToNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE,
                    TAG_REVEAL_SERVED_USER_IDENTITY_TO_CALLER);
            if (forwardToNode.getLength() > 0) {
                Element element = (Element) forwardToNode.item(0);
                String str = element.getTextContent();
                mRevealServedUserIdentityToCaller = str.equals("true");
            } else {
                forwardToNode = domElement.getElementsByTagName(
                        XCAP_ALIAS + ":" + TAG_REVEAL_SERVED_USER_IDENTITY_TO_CALLER);
                if (forwardToNode.getLength() > 0) {
                    Element element = (Element) forwardToNode.item(0);
                    String str = element.getTextContent();
                    mRevealServedUserIdentityToCaller = str.equals("true");
                }
            }
        }

        forwardToNode = domElement.getElementsByTagName(TAG_NOTIFY_SERVED_USER);
        if (forwardToNode.getLength() > 0) {
            Element element = (Element) forwardToNode.item(0);
            String str = element.getTextContent();
            mNotifyServedUser = str.equals("true");
        } else {
            forwardToNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE,
                    TAG_NOTIFY_SERVED_USER);
            if (forwardToNode.getLength() > 0) {
                Element element = (Element) forwardToNode.item(0);
                String str = element.getTextContent();
                mNotifyServedUser = str.equals("true");
            } else {
                forwardToNode = domElement.getElementsByTagName(
                        XCAP_ALIAS + ":" + TAG_NOTIFY_SERVED_USER);
                if (forwardToNode.getLength() > 0) {
                    Element element = (Element) forwardToNode.item(0);
                    String str = element.getTextContent();
                    mNotifyServedUser = str.equals("true");
                }
            }
        }

        forwardToNode = domElement.getElementsByTagName(TAG_NOTIFY_SERVED_USER_ON_OUTBOUND_CALL);
        if (forwardToNode.getLength() > 0) {
            Element element = (Element) forwardToNode.item(0);
            String str = element.getTextContent();
            mNotifyServedUserOnOutboundCall = str.equals("true");
        } else {
            forwardToNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE,
                    TAG_NOTIFY_SERVED_USER_ON_OUTBOUND_CALL);
            if (forwardToNode.getLength() > 0) {
                Element element = (Element) forwardToNode.item(0);
                String str = element.getTextContent();
                mNotifyServedUserOnOutboundCall = str.equals("true");
            } else {
                forwardToNode = domElement.getElementsByTagName(
                        XCAP_ALIAS + ":" + TAG_NOTIFY_SERVED_USER_ON_OUTBOUND_CALL);
                if (forwardToNode.getLength() > 0) {
                    Element element = (Element) forwardToNode.item(0);
                    String str = element.getTextContent();
                    mNotifyServedUserOnOutboundCall = str.equals("true");
                }
            }
        }
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
            Element forwardElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                    "ss:" + NODE_NAME);

            Element allowElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                    "ss:" + TAG_TARGET);
            allowElement.setTextContent(mTarget);
            forwardElement.appendChild(allowElement);

            String completeForwardTo = System.getProperty("xcap.completeforwardto",
                    "false");

            if ("true".equals(completeForwardTo)) {
                Element notifyCallerElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_NOTIFY_CALLER);
                notifyCallerElement.setTextContent(mNotifyCaller ? "true" : "false");
                forwardElement.appendChild(notifyCallerElement);

                Element revealIdentityToCallerElement =
                        (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_REVEAL_IDENTITY_TO_CALLER);
                revealIdentityToCallerElement.setTextContent(
                            mRevealIdentityToCaller ? "true" : "false");
                forwardElement.appendChild(revealIdentityToCallerElement);

                Element revealServedUserIdentityToCallerElement =
                        (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_REVEAL_SERVED_USER_IDENTITY_TO_CALLER);
                revealServedUserIdentityToCallerElement.setTextContent(
                            mRevealServedUserIdentityToCaller ? "true" : "false");
                forwardElement.appendChild(revealServedUserIdentityToCallerElement);

                Element notifyServedUserElement =
                        (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_NOTIFY_SERVED_USER);
                notifyServedUserElement.setTextContent(mNotifyServedUser ? "true" : "false");
                forwardElement.appendChild(notifyServedUserElement);

                Element notifyServedUserOnOutboundCallElement =
                        (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_NOTIFY_SERVED_USER_ON_OUTBOUND_CALL);
                notifyServedUserOnOutboundCallElement.setTextContent(
                        mNotifyServedUserOnOutboundCall ? "true" : "false");
                forwardElement.appendChild(notifyServedUserOnOutboundCallElement);

                Element revealIdentityToTargetElement =
                        (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_REVEAL_IDENTITY_TO_TARGET);
                revealIdentityToTargetElement.setTextContent(
                        mRevealIdentityToTarget ? "true" : "false");
                forwardElement.appendChild(revealIdentityToTargetElement);
            }

            return forwardElement;
        } else {
            Element forwardElement = (Element) document.createElement(NODE_NAME);

            Element allowElement = (Element) document.createElement(TAG_TARGET);
            allowElement.setTextContent(mTarget);
            forwardElement.appendChild(allowElement);

            String completeForwardTo = System.getProperty("xcap.completeforwardto",
                    "false");

            if ("true".equals(completeForwardTo)) {
                Element notifyCallerElement = (Element) document.createElement(TAG_NOTIFY_CALLER);
                notifyCallerElement.setTextContent(mNotifyCaller ? "true" : "false");
                forwardElement.appendChild(notifyCallerElement);

                Element revealIdentityToCallerElement =
                        (Element) document.createElement(TAG_REVEAL_IDENTITY_TO_CALLER);
                revealIdentityToCallerElement.setTextContent(
                        mRevealIdentityToCaller ? "true" : "false");
                forwardElement.appendChild(revealIdentityToCallerElement);

                Element revealServedUserIdentityToCallerElement =
                        (Element) document.createElement(TAG_REVEAL_SERVED_USER_IDENTITY_TO_CALLER);
                revealServedUserIdentityToCallerElement.setTextContent(
                        mRevealServedUserIdentityToCaller ? "true" : "false");
                forwardElement.appendChild(revealServedUserIdentityToCallerElement);

                Element notifyServedUserElement =
                        (Element) document.createElement(TAG_NOTIFY_SERVED_USER);
                notifyServedUserElement.setTextContent(mNotifyServedUser ? "true" : "false");
                forwardElement.appendChild(notifyServedUserElement);

                Element notifyServedUserOnOutboundCallElement =
                        (Element) document.createElement(TAG_NOTIFY_SERVED_USER_ON_OUTBOUND_CALL);
                notifyServedUserOnOutboundCallElement.setTextContent(
                        mNotifyServedUserOnOutboundCall ? "true" : "false");
                forwardElement.appendChild(notifyServedUserOnOutboundCallElement);

                Element revealIdentityToTargetElement =
                        (Element) document.createElement(TAG_REVEAL_IDENTITY_TO_TARGET);
                revealIdentityToTargetElement.setTextContent(
                        mRevealIdentityToTarget ? "true" : "false");
                forwardElement.appendChild(revealIdentityToTargetElement);
            }

            return forwardElement;
        }


    }

    public void setTarget(String target) {
        mTarget = target;
    }

    public void setNotifyCaller(boolean notifyCaller) {
        mNotifyCaller = notifyCaller;
    }

    public void setRevealIdentityToCaller(boolean revealIdToCaller) {
        mRevealIdentityToCaller = revealIdToCaller;
    }

    public void setRevealServedUserIdentityToCaller(boolean revealIdToCaller) {
        mRevealServedUserIdentityToCaller = revealIdToCaller;
    }

    public void setNotifyServedUser(boolean notifyToServedUser) {
        mNotifyServedUser = notifyToServedUser;
    }

    public void setNotifyServedUserOnOutboundCall(boolean notifyToServedUser) {
        mNotifyServedUserOnOutboundCall = notifyToServedUser;
    }

    public void setRevealIdentityToTarget(boolean revealIdToTarget) {
        mRevealIdentityToTarget = revealIdToTarget;
    }

    public String getTarget() {
        return mTarget;
    }

    public boolean isNotifyCaller() {
        return mNotifyCaller;
    }

    public boolean isRevealIdentityToCaller() {
        return mRevealIdentityToCaller;
    }

    public boolean isRevealServedUserIdentityToCaller() {
        return mRevealServedUserIdentityToCaller;
    }

    public boolean isNotifyServedUse() {
        return mNotifyServedUser;
    }

    public boolean isNotifyServedUserOnOutboundCall() {
        return mNotifyServedUserOnOutboundCall;
    }

    public boolean isRevealIdentityToTarget() {
        return mRevealIdentityToTarget;
    }
}
