
package com.mediatek.simservs.client;

import android.net.Network;
import android.util.Log;

import com.mediatek.simservs.client.policy.Rule;
import com.mediatek.simservs.client.policy.RuleSet;
import com.mediatek.simservs.xcap.RuleType;
import com.mediatek.simservs.xcap.XcapElement;
import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.lang.NumberFormatException;
import java.net.URISyntaxException;
import java.util.LinkedList;
import javax.xml.parsers.ParserConfigurationException;

/**
 * Communication Diversion class.
 *
 */
public class CommunicationDiversion extends SimservType implements RuleType {

    public static final String NODE_NAME = "communication-diversion";
    private static final int NO_REPLY_TIMER_DEFAULT_VAULE = -2; //decode error

    NoReplyTimer mNoReplyTimer;
    RuleSet mRuleSet;

    /**
     * Constructor.
     *
     * @param documentUri XCAP document URI
     * @param parentUri   XCAP root directory URI
     * @param intendedId  X-3GPP-Intended-Id
     * @throws XcapException if XCAP error
     * @throws ParserConfigurationException if parser configuration error
     */
    public CommunicationDiversion(XcapUri documentUri, String parentUri, String intendedId)
            throws XcapException, ParserConfigurationException {
        super(documentUri, parentUri, intendedId);
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME;
    }

    /**
     * Instantiate from DOM XML.
     *
     * @param   domDoc  DOM document
     */
    @Override
    public void initServiceInstance(Document domDoc) {
        NodeList noReplyTimerNode = domDoc.getElementsByTagName("NoReplyTimer");
        int noReplyTimer = NO_REPLY_TIMER_DEFAULT_VAULE;
        String val = null;
        if (noReplyTimerNode.getLength() > 0) {
            Log.d("CommunicationDiversion", "Got NoReplyTimer");
            String prefix = noReplyTimerNode.item(0).getPrefix();
            String namespaceUri = noReplyTimerNode.item(0).getNamespaceURI();
            Element noReplyTimerElement = (Element) noReplyTimerNode.item(0);
            /// ALPS02670343, decode NoReplyTimer error due to network issue @{
            val = noReplyTimerElement.getFirstChild().getTextContent();
            try {
                noReplyTimer = Integer.parseInt(val.trim());
            } catch (NumberFormatException e) {
                e.printStackTrace();
            }
            /// @}
            mNoReplyTimer = new NoReplyTimer(mXcapUri, NODE_NAME, mIntendedId,
                    noReplyTimer, prefix, namespaceUri);
            if (mNetwork != null) {
                mNoReplyTimer.setNetwork(mNetwork);
            }

            if (mEtag != null) {
                mNoReplyTimer.setEtag(mEtag);
            }
        } else {
            noReplyTimerNode = domDoc.getElementsByTagNameNS(XCAP_NAMESPACE,
                    "NoReplyTimer");
            if (noReplyTimerNode.getLength() > 0) {
                Log.d("CommunicationDiversion", "Got NoReplyTimer with xcap namespace");
                String prefix = noReplyTimerNode.item(0).getPrefix();
                String namespaceUri = noReplyTimerNode.item(0).getNamespaceURI();
                Element noReplyTimerElement = (Element) noReplyTimerNode.item(0);
                /// ALPS02670343, decode NoReplyTimer error due to network issue @{
                val = noReplyTimerElement.getFirstChild().getTextContent();
                try {
                    noReplyTimer = Integer.parseInt(val.trim());
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
                /// @}
                mNoReplyTimer = new NoReplyTimer(mXcapUri, NODE_NAME, mIntendedId,
                        noReplyTimer, prefix, namespaceUri);
                if (mNetwork != null) {
                    mNoReplyTimer.setNetwork(mNetwork);
                }

                if (mEtag != null) {
                    mNoReplyTimer.setEtag(mEtag);
                }
            } else {
                noReplyTimerNode = domDoc.getElementsByTagName(XCAP_ALIAS + ":" + "NoReplyTimer");
                if (noReplyTimerNode.getLength() > 0) {
                    Log.d("CommunicationDiversion", "Got ss:NoReplyTimer");
                    String prefix = noReplyTimerNode.item(0).getPrefix();
                    String namespaceUri = noReplyTimerNode.item(0).getNamespaceURI();
                    Element noReplyTimerElement = (Element) noReplyTimerNode.item(0);
                    /// ALPS02670343, decode NoReplyTimer error due to network issue @{
                    val = noReplyTimerElement.getFirstChild().getTextContent();
                    try {
                        noReplyTimer = Integer.parseInt(val.trim());
                    } catch (NumberFormatException e) {
                        e.printStackTrace();
                    }
                    /// @}
                    mNoReplyTimer = new NoReplyTimer(mXcapUri, NODE_NAME, mIntendedId,
                            noReplyTimer, prefix, namespaceUri);
                    if (mNetwork != null) {
                        mNoReplyTimer.setNetwork(mNetwork);
                    }

                    if (mEtag != null) {
                        mNoReplyTimer.setEtag(mEtag);
                    }
                } else {
                    mNoReplyTimer = new NoReplyTimer(mXcapUri, NODE_NAME, mIntendedId,
                            -1);
                    if (mNetwork != null) {
                        mNoReplyTimer.setNetwork(mNetwork);
                    }

                    if (mEtag != null) {
                        mNoReplyTimer.setEtag(mEtag);
                    }
                }
            }
        }

        NodeList ruleSetNode = domDoc.getElementsByTagName("ruleset");
        String tmpNodeName = (mPrefix != null) ? (mPrefix + ":" + NODE_NAME) : NODE_NAME;
        if (ruleSetNode.getLength() > 0) {
            Log.d("CommunicationDiversion", "Got ruleset");
            Element nruleSetElement = (Element) ruleSetNode.item(0);
            mRuleSet = new RuleSet(mXcapUri, tmpNodeName, mIntendedId, nruleSetElement);
            if (mNetwork != null) {
                mRuleSet.setNetwork(mNetwork);
            }

            if (mEtag != null) {
                mRuleSet.setEtag(mEtag);
            }
        } else {
            ruleSetNode = domDoc.getElementsByTagNameNS(COMMON_POLICY_NAMESPACE, "ruleset");
            if (ruleSetNode.getLength() > 0) {
                Log.d("CommunicationDiversion", "Got ruleset with commmon policy namespace");
                Element nruleSetElement = (Element) ruleSetNode.item(0);
                mRuleSet = new RuleSet(mXcapUri, tmpNodeName, mIntendedId,
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
                    Log.d("CommunicationDiversion", "Got cp:ruleset");
                    Element nruleSetElement = (Element) ruleSetNode.item(0);
                    mRuleSet = new RuleSet(mXcapUri, tmpNodeName, mIntendedId,
                            nruleSetElement);
                    if (mNetwork != null) {
                        mRuleSet.setNetwork(mNetwork);
                    }

                    if (mEtag != null) {
                        mRuleSet.setEtag(mEtag);
                    }
                } else {
                    mRuleSet = new RuleSet(mXcapUri, tmpNodeName, mIntendedId);
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

    /**
     * Set dedicated network.
     *
     * @param network dedicated network
     */
    public void setNetwork(Network network) {
        super.setNetwork(network);
        if (network != null) {
            if (mNoReplyTimer != null) {
                Log.d(TAG, "XCAP dedicated network netid to mNoReplyTimer:" + network);
                mNoReplyTimer.setNetwork(mNetwork);
            }
            if (mRuleSet != null) {
                Log.d(TAG, "XCAP dedicated network netid to mRuleSet:" + network);
                mRuleSet.setNetwork(mNetwork);
            }
        }
    }

    public int getNoReplyTimer() {
        return mNoReplyTimer.getValue();
    }

    /**
     * Set noreply timer.
     *
     * @param   timerValue  time value in second
     * @throws  XcapException if XCAP error
     */
    public void setNoReplyTimer(int timerValue, boolean isSentToNW) throws XcapException {
        mNoReplyTimer.setValue(timerValue);
        if (isSentToNW) {
            String noReplyTimerXml = mNoReplyTimer.toXmlString();
            mNoReplyTimer.setContent(noReplyTimerXml);
            if (mNoReplyTimer.getEtag() != null) {
                this.mEtag = mNoReplyTimer.getEtag();
                Log.d("CommunicationDiversion", "setNoReplyTimer: mEtag=" + this.mEtag);
                mRuleSet.setEtag(this.mEtag);
            }
        }
    }

    /**
    * Get rule set.
    *
    * @return Ruleset
    */
    @Override
    public RuleSet getRuleSet() {
        return mRuleSet;
    }

    /**
     * Save ruleset to server.
     *
     * @throws  XcapException if XCAP error
     */
    @Override
    public void saveRuleSet() throws XcapException {
        String ruleXml = mRuleSet.toXmlString();
        if (ruleXml == null) {
            Log.e("CommunicationDiversion", "saveRuleSet: null xml string");
            return;
        }
        mRuleSet.setContent(ruleXml);
        if (mRuleSet.getEtag() != null) {
            this.mEtag = mRuleSet.getEtag();
            Log.d("CommunicationDiversion", "saveRuleSet: mEtag=" + this.mEtag);
            mRuleSet.setEtag(this.mEtag);
            mNoReplyTimer.setEtag(this.mEtag);
        }
    }

    /**
     * Create ruleset.
     *
     * @return  ruleset
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
                Log.e("CommunicationDiversion", "saveRule: null xml string: " + rule.mId);
                return;
            }
            rule.setContent(ruleXml, mHasXcapNS);
            if (rule.getEtag() != null) {
                this.mEtag = rule.getEtag();
                Log.d("CommunicationDiversion", "saveRule: mEtag=" + this.mEtag);
                mRuleSet.setEtag(this.mEtag);
                mNoReplyTimer.setEtag(this.mEtag);
            }
        } else {
            Log.d("saveRule", "rule is null");
        }
    }

    public void save(boolean active) throws XcapException {
        saveNode(active, mRuleSet);
        if (mEtag != null) {
            Log.d("CommunicationDiversion", "save: mEtag=" + mEtag);
            mRuleSet.setEtag(mEtag);
            mNoReplyTimer.setEtag(mEtag);
        }
    }

    /**
     * NoReplyTimer class.
     *
     */
    public class NoReplyTimer extends XcapElement {

        public static final String NODE_NAME = "NoReplyTimer";
        private static final String NODE_XML_NAMESPACE_FORMAT =
            "?xmlns(" + "%s" + "=" + "%s" + ")";

        public int mValue;
        // namespace
        private String mPrefix ;
        private String mNameSpace;
        private String mNodeName;

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
        }

        /**
         * Constructor with initial time value.
         *
         * @param cdUri        XCAP document URI
         * @param parentUri    XCAP root directory URI
         * @param intendedId   X-3GPP-Intended-Id
         * @param initValue    time value
         * @param prefix       namespace prefix
         * @param namespaceUri namespace uri
         */
        public NoReplyTimer(XcapUri cdUri, String parentUri, String intendedId, int initValue,
                String prefix, String namespaceUri) {
            this(cdUri, parentUri, intendedId, initValue);
            mPrefix = prefix;
            if (prefix != null && namespaceUri != null) {
                mNameSpace = String.format(NODE_XML_NAMESPACE_FORMAT, prefix, namespaceUri);
                mNodeName = String.format("%s:" + NODE_NAME, prefix);
            }
            Log.d("[NoReplyTimer]", "prefix =" + mPrefix + ", uri =" + mNameSpace + "\n"
                +"[NoReplyTimer]" + "node =" + mNodeName);
        }

        @Override
        protected String getNodeName() {
            return mPrefix != null ? mNodeName : NODE_NAME;
        }

        public int getValue() {
            return mValue;
        }

        public void setValue(int value) {
            mValue = value;
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

                //add namespace
                if (mPrefix != null) {
                    mNodeUri += mNameSpace;
                }

                Log.d("NoReplyTimer", "setContent etag=" + mEtag);
                saveContent(xml);
            } catch (URISyntaxException e) {
                e.printStackTrace();
            }
        }

        /**
         * Convert to XML string.
         *
         * @return XML string
         */
        public String toXmlString() {
            String nodeName = mPrefix != null ? mNodeName : NODE_NAME;
            return String.format("<%s>" + String.valueOf(mValue) + "</%s>", nodeName, nodeName);
        }
    }
}
