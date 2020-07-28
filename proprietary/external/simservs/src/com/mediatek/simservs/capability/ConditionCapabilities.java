
package com.mediatek.simservs.capability;

import com.mediatek.simservs.xcap.ConfigureType;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Condition Capabilities class.
 *
 */
public class ConditionCapabilities extends ServiceCapabilities implements ConfigureType {
    public static final String NODE_NAME = "serv-cap-conditions";

    static final String TAG_ANONYMOUS = "serv-cap-anonymous";
    static final String TAG_REQUEST_NAME = "serv-cap-request-name";
    static final String TAG_COMMUNICATION_DIVERTED = "serv-cap-communication-diverted";
    static final String TAG_EXTERNAL_LIST = "serv-cap-external-list";
    static final String TAG_IDENTITY = "serv-cap-identity";
    static final String TAG_INTERNATIONAL = "serv-cap-international";
    static final String TAG_INTERNATIONAL_EXHC = "serv-cap-international-exHC";
    static final String TAG_MEDIA = "serv-cap-media";
    static final String TAG_OTHER_IDENTITY = "serv-cap-other-identity";
    static final String TAG_PRESENCE_STATUS = "serv-cap-presence-status";
    static final String TAG_ROAMING = "serv-cap-roaming";
    static final String TAG_RULE_DEACTIVATED = "serv-cap-rule-deactivated";
    static final String TAG_VALIDITY = "serv-cap-validity";
    static final String TAG_BUSY = "serv-cap-busy";
    static final String TAG_NOT_REGISTERED = "serv-cap-not-registered";
    static final String TAG_NO_ANSWER = "serv-cap-no-answer";
    static final String TAG_NOT_REACHABLE = "serv-cap-not-reachable";

    public boolean mAnonymousProvisioned = false;
    public boolean mRequestNameProvisioned = false;
    public boolean mCommunicationDivertedProvisioned = false;
    public boolean mExternalListProvisioned = false;
    public boolean mIdentityProvisioned = false;
    public boolean mInternationalProvisioned = false;
    public boolean mInternationalexHCProvisioned = false;
    public boolean mOtherIdentityProvisioned = false;
    public boolean mPresenceStatusProvisioned = false;
    public boolean mRoamingProvisioned = false;
    public boolean mRuleDeactivatedProvisioned = false;
    public boolean mValidityProvisioned = false;

    MediaConditions mMediaConditions;

    /**
     * Constructor.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     */
    public ConditionCapabilities(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
    }

    /**
     * Constructor with XML nodes.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @param nodes         XML nodes
     */
    public ConditionCapabilities(XcapUri xcapUri, String parentUri, String intendedId,
            Node nodes) {
        super(xcapUri, parentUri, intendedId);
        instantiateFromXmlNode(nodes);
    }

    @Override
    public void instantiateFromXmlNode(Node domNode) {
        Element domElement = (Element) domNode;

        NodeList conditionNode = domElement.getElementsByTagName(TAG_ANONYMOUS);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mAnonymousProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_REQUEST_NAME);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mRequestNameProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_COMMUNICATION_DIVERTED);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mCommunicationDivertedProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_EXTERNAL_LIST);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mExternalListProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_IDENTITY);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mIdentityProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_INTERNATIONAL);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mInternationalProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_INTERNATIONAL_EXHC);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mInternationalexHCProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_OTHER_IDENTITY);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mOtherIdentityProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_PRESENCE_STATUS);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mPresenceStatusProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_ROAMING);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mRoamingProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_RULE_DEACTIVATED);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mRuleDeactivatedProvisioned = provisioned.equals(TRUE);
        }

        conditionNode = domElement.getElementsByTagName(TAG_VALIDITY);
        if (conditionNode.getLength() > 0) {
            Element conditionElement = (Element) conditionNode.item(0);
            String provisioned = conditionElement.getAttribute(ATT_PROVISIONED);
            mValidityProvisioned = provisioned.equals(TRUE);
        }

        NodeList mediassNode = domElement.getElementsByTagName(TAG_MEDIA);
        if (mediassNode.getLength() > 0) {
            mMediaConditions = new MediaConditions(mXcapUri, NODE_NAME, mIntendedId,
                    (Element) mediassNode.item(0));
        }
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    public boolean isAnonymousProvisioned() {
        return mAnonymousProvisioned;
    }

    public boolean isRequestNameProvisioned() {
        return mRequestNameProvisioned;
    }

    public boolean isCommunicationDivertedProvisioned() {
        return mCommunicationDivertedProvisioned;
    }

    public boolean isExternalListProvisioned() {
        return mExternalListProvisioned;
    }

    public boolean isIdentityProvisioned() {
        return mIdentityProvisioned;
    }

    public boolean isInternationalProvisioned() {
        return mInternationalProvisioned;
    }

    public boolean isInternationalexHCProvisioned() {
        return mInternationalexHCProvisioned;
    }

    public boolean isOtherIdentityProvisioned() {
        return mOtherIdentityProvisioned;
    }

    public boolean isPresenceStatusProvisioned() {
        return mPresenceStatusProvisioned;
    }

    public boolean isRoamingProvisioned() {
        return mRoamingProvisioned;
    }

    public boolean isRuleDeactivatedProvisioned() {
        return mRuleDeactivatedProvisioned;
    }

    public boolean isValidityProvisioned() {
        return mValidityProvisioned;
    }

    public MediaConditions getMediaConditions() {
        return mMediaConditions;
    }
}
