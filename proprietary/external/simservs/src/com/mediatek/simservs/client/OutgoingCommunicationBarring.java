package com.mediatek.simservs.client;

import android.net.Network;
import android.util.Log;

import com.mediatek.simservs.client.policy.Rule;
import com.mediatek.simservs.client.policy.RuleSet;
import com.mediatek.simservs.xcap.RuleType;
import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.LinkedList;
import javax.xml.parsers.ParserConfigurationException;

/**
 * Outgoing Communication Barring class.
 */
public class OutgoingCommunicationBarring extends SimservType implements RuleType {

    public static final String NODE_NAME = "outgoing-communication-barring";

    RuleSet mRuleSet;


    /**
     * Constructor.
     *
     * @param documentUri       XCAP document URI
     * @param parentUri         XCAP root directory URI
     * @param intendedId        X-3GPP-Intended-Id
     * @throws XcapException    if XCAP error
     * @throws ParserConfigurationException if parser configuration error
     */
    public OutgoingCommunicationBarring(XcapUri documentUri, String parentUri, String intendedId)
            throws XcapException, ParserConfigurationException {
        super(documentUri, parentUri, intendedId);
    }

    @Override
    public void initServiceInstance(Document domDoc) {
        NodeList ruleSetNode = domDoc.getElementsByTagName("ruleset");
        if (ruleSetNode.getLength() > 0) {
            Log.d("OutgoingCommunicationBarring", "Got ruleset");
            Element nruleSetElement = (Element) ruleSetNode.item(0);
            mRuleSet = new RuleSet(mXcapUri, NODE_NAME, mIntendedId, nruleSetElement);
            if (mNetwork != null) {
                mRuleSet.setNetwork(mNetwork);
            }

            if (mEtag != null) {
                mRuleSet.setEtag(mEtag);
            }
        } else {
            ruleSetNode = domDoc.getElementsByTagNameNS(COMMON_POLICY_NAMESPACE, "ruleset");
            if (ruleSetNode.getLength() > 0) {
                Log.d("OutgoingCommunicationBarring", "Got ruleset");
                Element nruleSetElement = (Element) ruleSetNode.item(0);
                mRuleSet = new RuleSet(mXcapUri, NODE_NAME, mIntendedId,
                        nruleSetElement);
                if (mNetwork != null) {
                    mRuleSet.setNetwork(mNetwork);
                }

                if (mEtag != null) {
                    mRuleSet.setEtag(mEtag);
                }
            } else {
                ruleSetNode = domDoc.getElementsByTagName("cp:ruleset");
                if (ruleSetNode.getLength() > 0) {
                    Log.d("OutgoingCommunicationBarring", "Got cp:ruleset");
                    Element nruleSetElement = (Element) ruleSetNode.item(0);
                    mRuleSet = new RuleSet(mXcapUri, NODE_NAME, mIntendedId,
                            nruleSetElement);
                    if (mNetwork != null) {
                        mRuleSet.setNetwork(mNetwork);
                    }

                    if (mEtag != null) {
                        mRuleSet.setEtag(mEtag);
                    }
                } else {
                    mRuleSet = new RuleSet(mXcapUri, NODE_NAME, mIntendedId);
                    if (mNetwork != null) {
                        mRuleSet.setNetwork(mNetwork);
                    }

                    if (mEtag != null) {
                        mRuleSet.setEtag(mEtag);
                    }
                }
            }
        }
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    /**
     * Get ruleset.
     *
     * @return ruleset
     */
    @Override
    public RuleSet getRuleSet() {
        return mRuleSet;
    }

    /**
     * Save ruleset to configuration on server.
     *
     * @throws XcapException if error
     */
    @Override
    public void saveRuleSet() throws XcapException {
        String ruleXml = mRuleSet.toXmlString();
        if (ruleXml == null) {
            Log.e("OutgoingCommunicationBarring", "saveRuleSet: null xml");
            return;
        }
        mRuleSet.setContent(ruleXml);
        if (mRuleSet.getEtag() != null) {
            this.mEtag = mRuleSet.getEtag();
            Log.d("OutgoingCommunicationBarring", "saveRuleSet: mEtag=" + this.mEtag);
            mRuleSet.setEtag(this.mEtag);
        }
    }

    /**
     * Save ruleset to configuration on server.
     *
     * @return ruleset
     */
    @Override
    public RuleSet createNewRuleSet() {
        mRuleSet = new RuleSet(mXcapUri, NODE_NAME, mIntendedId);
        if (mNetwork != null) {
            mRuleSet.setNetwork(mNetwork);
        }
        if (mEtag != null) {
            mRuleSet.setEtag(mEtag);
        }
        return mRuleSet;
    }

    /**
     * Save rule to server.
     *
     * @param ruleId rule to be saved by the id
     * @throws  XcapException if XCAP error
     */
    @Override
    public void saveRule(String ruleId) throws XcapException {
        if (ruleId != null && !ruleId.isEmpty()) {
            LinkedList<Rule> rules =  (LinkedList<Rule>) mRuleSet.getRules();
            for (Rule rule : rules) {
                if (ruleId.equals(rule.mId)) {
                    saveRule(rule);
                    break;
                }
            }
        } else {
            Log.d("saveRule", "ruleId is null");
        }
    }

   /**
     * Save rule to server.
     *
     * @param rule rule to be saved by the rule object
     * @throws  XcapException if XCAP error
     */
    @Override
    public void saveRule(Rule rule) throws XcapException {
        if (rule != null) {
            String ruleXml = rule.toXmlString();
            if (ruleXml == null) {
                Log.e("OutgoingCommunicationBarring", "saveRule: null xml: " + rule.mId);
                return;
            }
            rule.setContent(ruleXml);
            if (rule.getEtag() != null) {
                this.mEtag = rule.getEtag();
                Log.d("OutgoingCommunicationBarring", "saveRule: mEtag=" + this.mEtag);
                mRuleSet.setEtag(this.mEtag);
            }
        } else {
            Log.d("saveRule", "rule is null");
        }
    }

    public void save(boolean active) throws XcapException {
        saveNode(active, mRuleSet);
        if (mEtag != null) {
            Log.d("OutgoingCommunicationBarring", "save: mEtag=" + mEtag);
            mRuleSet.setEtag(mEtag);
        }
    }

    @Override
    public void setNetwork(Network network) {
        super.setNetwork(network);
        if (network != null) {
            if (mRuleSet != null) {
                Log.d(TAG, "XCAP dedicated network netid to mRuleSet:" + network);
                mRuleSet.setNetwork(network);
            }
        }
    }
}
