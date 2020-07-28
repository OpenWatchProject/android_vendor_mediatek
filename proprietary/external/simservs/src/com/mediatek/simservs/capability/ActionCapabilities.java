package com.mediatek.simservs.capability;

import com.mediatek.simservs.xcap.ConfigureType;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Action capability class.
 */
public class ActionCapabilities extends ServiceCapabilities implements ConfigureType {

    public static final String NODE_NAME = "serv-cap-actions";

    static final String TAG_TARGET = "serv-cap-target";
    static final String TAG_NOTIFY_CALLER = "serv-cap-notify-caller";
    static final String TAG_NOTIFY_SERVED_USER = "serv-cap-notify-served-user";
    static final String TAG_NOTIFY_SERVED_USER_ON_OUTBOUND_CALL =
            "serv-cap-notify-served-user-on-outbound-call";
    static final String TAG_REVEAL_IDENTITY_TO_CALLER = "serv-cap-reveal-identity-to-caller";
    static final String TAG_REVEAL_SERVED_USER_IDENTITY_TO_CALLER =
            "serv-cap-reveal-served-user-identity-to-caller";
    static final String TAG_REVEAL_IDENTITY_TO_TARGET = "serv-cap-reveal-identity-to-target";

    public boolean mNotifyCallerProvisioned = false;
    public boolean mNotifyServedUserProvisioned = false;
    public boolean mNotifyServedUserOnOutboundCallProvisioned = false;
    public boolean mRevealIdentityToCallerProvisioned = false;
    public boolean mRevealServedUserIdentityToCallerProvisioned = false;
    public boolean mRevealIdentityToTargetProvisioned = false;

    /**
     * Constructor without XML Node.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     */
    public ActionCapabilities(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
    }

    /**
     * Constructor without XML Node.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @param nodes         pre-define action capability node
     */
    public ActionCapabilities(XcapUri xcapUri, String parentUri, String intendedId,
            Node nodes) {
        super(xcapUri, parentUri, intendedId);
        instantiateFromXmlNode(nodes);
    }

    @Override
    public void instantiateFromXmlNode(Node domNode) {
        Element domElement = (Element) domNode;

        NodeList conditionNode = domElement.getElementsByTagName(TAG_NOTIFY_CALLER);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mNotifyCallerProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_NOTIFY_SERVED_USER);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mNotifyServedUserProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_NOTIFY_SERVED_USER_ON_OUTBOUND_CALL);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mNotifyServedUserOnOutboundCallProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_REVEAL_IDENTITY_TO_CALLER);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mRevealIdentityToCallerProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_REVEAL_SERVED_USER_IDENTITY_TO_CALLER);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mRevealServedUserIdentityToCallerProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_REVEAL_IDENTITY_TO_TARGET);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mRevealIdentityToTargetProvisioned = provisioned.equals(TRUE);
        }
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    public boolean isNotifyCallerProvisioned() {
        return mNotifyCallerProvisioned;
    }

    public boolean isNotifyServedUserProvisioned() {
        return mNotifyServedUserProvisioned;
    }

    public boolean isNotifyServedUserOnOutboundCallProvisioned() {
        return mNotifyServedUserOnOutboundCallProvisioned;
    }

    public boolean isRevealIdentityToCallerProvisioned() {
        return mRevealIdentityToCallerProvisioned;
    }

    public boolean isRevealServedUserIdentityToCallerProvisioned() {
        return mRevealServedUserIdentityToCallerProvisioned;
    }

    public boolean isRevealIdentityToTargetProvisioned() {
        return mRevealIdentityToTargetProvisioned;
    }
}
