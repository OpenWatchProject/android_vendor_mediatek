
package com.mediatek.simservs.client.policy;

import android.net.Network;
import android.util.Log;

import com.mediatek.simservs.client.SimServs;
import com.mediatek.simservs.xcap.ConfigureType;
import com.mediatek.simservs.xcap.XcapElement;
import com.mediatek.xcap.client.uri.XcapUri;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;

/**
 * Rule class.
 */
public class RuleSet extends XcapElement implements ConfigureType {

    public static final String NODE_NAME = COMMON_POLICY_ALIAS + ":ruleset";
    public static final String NODE_NAME_WITH_NAMESPACE = NODE_NAME +
            "?xmlns(" + COMMON_POLICY_ALIAS + "=" + COMMON_POLICY_NAMESPACE + ")";
    public List<Rule> mRules;

    /**
     * Constructor.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     */
    public RuleSet(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
        mRules = new LinkedList<Rule>();
    }

    /**
     * Constructor with XML element.
     *
     * @param xcapUri       XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @param domElement    DOM element
     */
    public RuleSet(XcapUri xcapUri, String parentUri, String intendedId, Element domElement) {
        super(xcapUri, parentUri, intendedId);
        instantiateFromXmlNode(domElement);
    }

    @Override
    protected String getNodeName() {
        return NODE_NAME_WITH_NAMESPACE;
    }

    @Override
    public void instantiateFromXmlNode(Node domNode) {
        if (SimServs.LIB_CONFIG_MULTIPLE_RULE_CONDITIONS) {
            Element domElement = (Element) domNode;
            NodeList domNodes = domElement.getElementsByTagName("rule");
            mRules = new LinkedList<Rule>();
            if (domNodes.getLength() > 0) {
                Log.d("RuleSet", "Got rule");
                for (int i = 0; i < domNodes.getLength(); i++) {
                    Element element = (Element) domNodes.item(i);
                    Rule aRule = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId,
                            element);
                    if (mNetwork != null) {
                        aRule.setNetwork(mNetwork);
                    }

                    if (mEtag != null) {
                        aRule.setEtag(mEtag);
                    }

                    mRules.add(aRule);
                }
            }

            domNodes = domElement.getElementsByTagNameNS(COMMON_POLICY_NAMESPACE, "rule");
            if (domNodes.getLength() > 0) {
                Log.d("RuleSet", "Got rule with common policy namespace");
                for (int i = 0; i < domNodes.getLength(); i++) {
                    Element element = (Element) domNodes.item(i);
                    Rule aRule = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId,
                            element);
                    if (mNetwork != null) {
                        aRule.setNetwork(mNetwork);
                    }

                    if (mEtag != null) {
                        aRule.setEtag(mEtag);
                    }

                    mRules.add(aRule);
                }
            } else {
                domNodes = domElement.getElementsByTagName("cp:rule");
                if (domNodes.getLength() > 0) {
                    Log.d("RuleSet", "Got cp:rule");
                    for (int i = 0; i < domNodes.getLength(); i++) {
                        Element element = (Element) domNodes.item(i);
                        Rule aRule = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME,
                                mIntendedId, element);
                        if (mNetwork != null) {
                            aRule.setNetwork(mNetwork);
                        }

                        if (mEtag != null) {
                            aRule.setEtag(mEtag);
                        }

                        mRules.add(aRule);
                    }
                }
            }

            Log.d("RuleSet", "rules size:" + mRules.size());
        } else {
            // Unfold the conditions to more rules if multiple conditions exists
            Element domElement = (Element) domNode;
            NodeList domNodes = domElement.getElementsByTagName("rule");
            mRules = new LinkedList<Rule>();
            if (domNodes.getLength() > 0) {
                Log.d("RuleSet", "Got rule");
                for (int i = 0; i < domNodes.getLength(); i++) {
                    Element element = (Element) domNodes.item(i);
                    Rule aRule = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId,
                            element);
                    if (mNetwork != null) {
                        aRule.setNetwork(mNetwork);
                    }

                    if (mEtag != null) {
                        aRule.setEtag(mEtag);
                    }

                    Conditions conditions = aRule.getConditions();
                    if (conditions != null) {
                        List<String> medias = aRule.getConditions().getMedias();
                        // Unfolding multiple medias
                        if (medias != null && medias.size() > 0) {
                            Iterator<String> it = medias.iterator();
                            while (it.hasNext()) {
                                unfoldRules(aRule, element, it.next());
                            }
                        } else {
                            unfoldRules(aRule, element, null);
                        }
                    } else {
                        mRules.add(aRule);
                    }
                }
            } else {
                domNodes = domElement.getElementsByTagNameNS(COMMON_POLICY_NAMESPACE, "rule");
                if (domNodes.getLength() > 0) {
                    Log.d("RuleSet", "Got rule");
                    for (int i = 0; i < domNodes.getLength(); i++) {
                        Element element = (Element) domNodes.item(i);
                        Rule aRule = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId,
                                element);
                        if (mNetwork != null) {
                            aRule.setNetwork(mNetwork);
                        }

                        if (mEtag != null) {
                            aRule.setEtag(mEtag);
                        }
                        Conditions conditions = aRule.getConditions();
                        if (conditions != null) {
                            List<String> medias = aRule.getConditions().getMedias();
                            // Unfolding multiple medias
                            if (medias != null && medias.size() > 0) {
                                Iterator<String> it = medias.iterator();
                                while (it.hasNext()) {
                                    unfoldRules(aRule, element, it.next());
                                }
                            } else {
                                unfoldRules(aRule, element, null);
                            }
                        } else {
                            mRules.add(aRule);
                        }
                    }
                }
            }
        }
    }

    private void unfoldRules(Rule aRule, Element element, String media) {
        if (aRule.getConditions().comprehendBusy()) {
            Rule ruleBusy = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId,
                    element);
            if (mNetwork != null) {
                ruleBusy.setNetwork(mNetwork);
            }
            ruleBusy.getConditions().clearConditions();
            ruleBusy.getConditions().addBusy();
            if (media != null) {
                ruleBusy.getConditions().addMedia(media);
            }

            mRules.add(ruleBusy);
        }
        if (aRule.getConditions().comprehendNotReachable()) {
            Rule ruleNotReachable = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId,
                    element);
            if (mNetwork != null) {
                ruleNotReachable.setNetwork(mNetwork);
            }
            ruleNotReachable.getConditions().clearConditions();
            ruleNotReachable.getConditions().addNotReachable();
            if (media != null) {
                ruleNotReachable.getConditions().addMedia(media);
            }

            mRules.add(ruleNotReachable);
        }
        if (aRule.getConditions().comprehendInternational()) {
            Rule ruleInternational = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId,
                    element);
            if (mNetwork != null) {
                ruleInternational.setNetwork(mNetwork);
            }
            ruleInternational.getConditions().clearConditions();
            ruleInternational.getConditions().addInternational();
            if (media != null) {
                ruleInternational.getConditions().addMedia(media);
            }

            mRules.add(ruleInternational);
        }
        if (aRule.getConditions().comprehendInternationalExHc()) {
            Rule ruleInternationalExHc = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME,
                    mIntendedId, element);
            if (mNetwork != null) {
                ruleInternationalExHc.setNetwork(mNetwork);
            }
            ruleInternationalExHc.getConditions().clearConditions();
            ruleInternationalExHc.getConditions().addInternational();
            if (media != null) {
                ruleInternationalExHc.getConditions().addMedia(media);
            }

            mRules.add(ruleInternationalExHc);
        }

        if (aRule.getConditions().comprehendNoAnswer()) {
            Rule ruleNoAnswer = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId,
                    element);
            if (mNetwork != null) {
                ruleNoAnswer.setNetwork(mNetwork);
            }
            ruleNoAnswer.getConditions().clearConditions();
            ruleNoAnswer.getConditions().addNoAnswer();
            if (media != null) {
                ruleNoAnswer.getConditions().addMedia(media);
            }

            mRules.add(ruleNoAnswer);
        }
        if (aRule.getConditions().comprehendRoaming()) {
            Rule ruleRoaming = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId,
                    element);
            if (mNetwork != null) {
                ruleRoaming.setNetwork(mNetwork);
            }
            ruleRoaming.getConditions().clearConditions();
            ruleRoaming.getConditions().addRoaming();
            if (media != null) {
                ruleRoaming.getConditions().addMedia(media);
            }

            mRules.add(ruleRoaming);
        }
    }

    public List<Rule> getRules() {
        return mRules;
    }

    /**
     * Create rule.
     *
     * @param   id rule ID
     * @return  new rule
     */
    public Rule createNewRule(String id) {
        if (mRules == null) {
            mRules = new LinkedList<Rule>();
        }
        Rule aRule = new Rule(mXcapUri, mParentUri + "/" + NODE_NAME, mIntendedId);
        if (mNetwork != null) {
            aRule.setNetwork(mNetwork);
        }
        if (mEtag != null) {
            aRule.setEtag(mEtag);
        }
        aRule.setId(id);
        mRules.add(aRule);
        return aRule;
    }

    /**
     * Empty rules.
     */
    public void clearRules() {
        if (mRules == null) {
            mRules = new LinkedList<Rule>();
        }
        mRules.clear();
    }

    /**
     * Convert ruleset to XML string.
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
            root = (Element) document.createElement(NODE_NAME);
            document.appendChild(root);
            //Add by mtk01411: 2014-0128 to avoid Null Pointer Exception
            if (mRules != null) {
                Iterator<Rule> it = mRules.iterator();
                while (it.hasNext()) {
                    Rule rule = (Rule) it.next();
                    Element ruleElement = rule.toXmlElement(document);
                    root.appendChild(ruleElement);
                }
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

    /**
     * Set ETag value.
     *
     * @param etag value
     */
    @Override
    public void setEtag(String etag) {
        mEtag = etag;

        for (Rule rule : mRules) {
            Log.d("RuleSet", "rule:" + rule.mId + ", set etag:" + etag);
            rule.setEtag(etag);
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
            for (Rule rule : mRules) {
                Log.d("RuleSet", "rule:" + rule.mId + ", netid:" + network);
                rule.setNetwork(network);
            }
        }
    }

}
