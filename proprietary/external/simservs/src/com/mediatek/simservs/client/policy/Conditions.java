
package com.mediatek.simservs.client.policy;

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

/**
 * Condition class.
 *
 */
public class Conditions extends XcapElement implements ConfigureType {
    public static final String NODE_NAME = "cp:conditions";

    static final String TAG_BUSY = "busy";
    static final String TAG_NO_ANSWER = "no-answer";
    static final String TAG_NOT_REACHABLE = "not-reachable";
    static final String TAG_NOT_REGISTERED = "not-registered";
    static final String TAG_ROAMING = "roaming";
    static final String TAG_RULE_DEACTIVATED = "rule-deactivated";
    static final String TAG_INTERNATIONAL = "international";
    static final String TAG_INTERNATIONAL_EXHC = "international-exHC";
    static final String TAG_COMMUNICATION_DIVERTED = "communication-diverted";
    static final String TAG_PRESENCE_STATUS = "presence-status";
    static final String TAG_MEDIA = "media";
    static final String TAG_ANONYMOUS = "anonymous";
    static final String TAG_TIME = "time";

    public boolean mComprehendBusy = false;
    public boolean mComprehendNoAnswer = false;
    public boolean mComprehendNotReachable = false;
    public boolean mComprehendNotRegistered = false;
    public boolean mComprehendRoaming = false;
    public boolean mComprehendRuleDeactivated = false;
    public boolean mComprehendInternational = false;
    public boolean mComprehendInternationalexHc = false;
    public boolean mComprehendCommunicationDiverted = false;
    public boolean mComprehendPresenceStatus = false;
    public boolean mComprehendAnonymous = false;
    public String  mComprehendTime;

    public List<String> mMedias;

    /**
     * Constructor.
     *
     * @param xcapUri   XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     */
    public Conditions(XcapUri xcapUri, String parentUri, String intendedId) {
        super(xcapUri, parentUri, intendedId);
        mMedias = new LinkedList<String>();
        // TODO Auto-generated constructor stub
    }

    /**
     * Constructor with XML element.
     *
     * @param xcapUri   XCAP document URI
     * @param parentUri     XCAP root directory URI
     * @param intendedId    X-3GPP-Intended-Id
     * @param domElement    DOM element
     */
    public Conditions(XcapUri xcapUri, String parentUri, String intendedId,
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
        String conditionsPrefix = XCAP_ALIAS + ":";
        // TODO Auto-generated method stub
        NodeList conditionsNode = domElement.getElementsByTagName(TAG_BUSY);
        if (conditionsNode.getLength() > 0) {
            mComprehendBusy = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_BUSY);
            if (conditionsNode.getLength() > 0) {
                mComprehendBusy = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(conditionsPrefix + TAG_BUSY);
                if (conditionsNode.getLength() > 0) {
                    mComprehendBusy = true;
                }
            }
        }

        conditionsNode = domElement.getElementsByTagName(TAG_NO_ANSWER);
        if (conditionsNode.getLength() > 0) {
            mComprehendNoAnswer = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_NO_ANSWER);
            if (conditionsNode.getLength() > 0) {
                mComprehendNoAnswer = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(conditionsPrefix + TAG_NO_ANSWER);
                if (conditionsNode.getLength() > 0) {
                    mComprehendNoAnswer = true;
                }
            }
        }

        conditionsNode = domElement.getElementsByTagName(TAG_NOT_REACHABLE);
        if (conditionsNode.getLength() > 0) {
            mComprehendNotReachable = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_NOT_REACHABLE);
            if (conditionsNode.getLength() > 0) {
                mComprehendNotReachable = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(
                        conditionsPrefix + TAG_NOT_REACHABLE);
                if (conditionsNode.getLength() > 0) {
                    mComprehendNotReachable = true;
                }
            }
        }

        conditionsNode = domElement.getElementsByTagName(TAG_NOT_REGISTERED);
        if (conditionsNode.getLength() > 0) {
            mComprehendNotRegistered = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_NOT_REGISTERED);
            if (conditionsNode.getLength() > 0) {
                mComprehendNotRegistered = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(
                        conditionsPrefix + TAG_NOT_REGISTERED);
                if (conditionsNode.getLength() > 0) {
                    mComprehendNotRegistered = true;
                }
            }
        }

        conditionsNode = domElement.getElementsByTagName(TAG_ROAMING);
        if (conditionsNode.getLength() > 0) {
            mComprehendRoaming = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_ROAMING);
            if (conditionsNode.getLength() > 0) {
                mComprehendRoaming = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(conditionsPrefix + TAG_ROAMING);
                if (conditionsNode.getLength() > 0) {
                    mComprehendRoaming = true;
                }
            }
        }


        conditionsNode = domElement.getElementsByTagName(TAG_RULE_DEACTIVATED);
        if (conditionsNode.getLength() > 0) {
            mComprehendRuleDeactivated = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE,
                    TAG_RULE_DEACTIVATED);
            if (conditionsNode.getLength() > 0) {
                mComprehendRuleDeactivated = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(
                        conditionsPrefix + TAG_RULE_DEACTIVATED);
                if (conditionsNode.getLength() > 0) {
                    mComprehendRuleDeactivated = true;
                }
            }
        }


        conditionsNode = domElement.getElementsByTagName(TAG_INTERNATIONAL);
        if (conditionsNode.getLength() > 0) {
            mComprehendInternational = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_INTERNATIONAL);
            if (conditionsNode.getLength() > 0) {
                mComprehendInternational = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(
                        conditionsPrefix + TAG_INTERNATIONAL);
                if (conditionsNode.getLength() > 0) {
                    mComprehendInternational = true;
                }
            }
        }


        conditionsNode = domElement.getElementsByTagName(TAG_INTERNATIONAL_EXHC);
        if (conditionsNode.getLength() > 0) {
            mComprehendInternationalexHc = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE,
                    TAG_INTERNATIONAL_EXHC);
            if (conditionsNode.getLength() > 0) {
                mComprehendInternationalexHc = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(
                        conditionsPrefix + TAG_INTERNATIONAL_EXHC);
                if (conditionsNode.getLength() > 0) {
                    mComprehendInternationalexHc = true;
                }
            }
        }

        conditionsNode = domElement.getElementsByTagName(TAG_COMMUNICATION_DIVERTED);
        if (conditionsNode.getLength() > 0) {
            mComprehendCommunicationDiverted = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE,
                    TAG_COMMUNICATION_DIVERTED);
            if (conditionsNode.getLength() > 0) {
                mComprehendCommunicationDiverted = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(
                        conditionsPrefix + TAG_COMMUNICATION_DIVERTED);
                if (conditionsNode.getLength() > 0) {
                    mComprehendCommunicationDiverted = true;
                }
            }
        }

        conditionsNode = domElement.getElementsByTagName(TAG_PRESENCE_STATUS);
        if (conditionsNode.getLength() > 0) {
            mComprehendPresenceStatus = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_PRESENCE_STATUS);
            if (conditionsNode.getLength() > 0) {
                mComprehendPresenceStatus = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(
                        conditionsPrefix + TAG_PRESENCE_STATUS);
                if (conditionsNode.getLength() > 0) {
                    mComprehendPresenceStatus = true;
                }
            }
        }

        conditionsNode = domElement.getElementsByTagName(TAG_MEDIA);
        mMedias = new LinkedList<String>();
        if (conditionsNode.getLength() > 0) {
            for (int i = 0; i < conditionsNode.getLength(); i++) {
                Element mediaElement = (Element) conditionsNode.item(i);
                mMedias.add(mediaElement.getTextContent());
            }
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_MEDIA);
            if (conditionsNode.getLength() > 0) {
                for (int i = 0; i < conditionsNode.getLength(); i++) {
                    Element mediaElement = (Element) conditionsNode.item(i);
                    mMedias.add(mediaElement.getTextContent());
                }
            } else {
                conditionsNode = domElement.getElementsByTagName(conditionsPrefix + TAG_MEDIA);
                if (conditionsNode.getLength() > 0) {
                    for (int i = 0; i < conditionsNode.getLength(); i++) {
                        Element mediaElement = (Element) conditionsNode.item(i);
                        mMedias.add(mediaElement.getTextContent());
                    }
                }
            }
        }

        conditionsNode = domElement.getElementsByTagName(TAG_ANONYMOUS);
        if (conditionsNode.getLength() > 0) {
            mComprehendAnonymous = true;
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_ANONYMOUS);
            if (conditionsNode.getLength() > 0) {
                mComprehendAnonymous = true;
            } else {
                conditionsNode = domElement.getElementsByTagName(conditionsPrefix + TAG_ANONYMOUS);
                if (conditionsNode.getLength() > 0) {
                    mComprehendAnonymous = true;
                }
            }
        }

        conditionsNode = domElement.getElementsByTagName(TAG_TIME);
        if (conditionsNode.getLength() > 0) {
            Element timeElement = (Element) conditionsNode.item(0);
            mComprehendTime = timeElement.getTextContent();
        } else {
            conditionsNode = domElement.getElementsByTagNameNS(XCAP_NAMESPACE, TAG_TIME);
            if (conditionsNode.getLength() > 0) {
                Element timeElement = (Element) conditionsNode.item(0);
                mComprehendTime = timeElement.getTextContent();
            } else {
                conditionsNode = domElement.getElementsByTagName(conditionsPrefix + TAG_TIME);
                if (conditionsNode.getLength() > 0) {
                    Element timeElement = (Element) conditionsNode.item(0);
                    mComprehendTime = timeElement.getTextContent();
                }
            }
        }

    }

    /**
     * Convert to XML.
     *
     * @param   document DOM document
     * @return  XML element
     */
    public Element toXmlElement(Document document) {
        String useXcapNs = System.getProperty("xcap.ns.ss", "false");

        if ("true".equals(useXcapNs)) {
            Element conditionsElement = (Element) document.createElement(NODE_NAME);

            if (comprehendBusy()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_BUSY);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendNoAnswer()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_NO_ANSWER);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendNotReachable()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_NOT_REACHABLE);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendNotRegistered()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_NOT_REGISTERED);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendRoaming()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_ROAMING);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendRuleDeactivated()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_RULE_DEACTIVATED);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendInternational()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_INTERNATIONAL);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendInternationalExHc()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_INTERNATIONAL_EXHC);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendCommunicationDiverted()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_COMMUNICATION_DIVERTED);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendPresenceStatus()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_PRESENCE_STATUS);
                conditionsElement.appendChild(conditionElement);
            }

            if (mMedias != null) {
                if (mMedias.size() > 0) {
                    Iterator<String> it = mMedias.iterator();
                    while (it.hasNext()) {
                        Element ruleElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                                "ss:" + TAG_MEDIA);
                        ruleElement.setTextContent(it.next());
                        conditionsElement.appendChild(ruleElement);
                    }
                }
            }

            if (comprehendAnonymous()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_ANONYMOUS);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendTime() != null && !comprehendTime().isEmpty()) {
                Element conditionElement = (Element) document.createElementNS(XCAP_NAMESPACE,
                        "ss:" + TAG_TIME);
                conditionElement.setTextContent(mComprehendTime);
                conditionsElement.appendChild(conditionElement);
            }

            return conditionsElement;
        } else {
            Element conditionsElement = (Element) document.createElement(NODE_NAME);

            if (comprehendBusy()) {
                Element conditionElement = (Element) document.createElement(TAG_BUSY);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendNoAnswer()) {
                Element conditionElement = (Element) document.createElement(TAG_NO_ANSWER);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendNotReachable()) {
                Element conditionElement = (Element) document.createElement(TAG_NOT_REACHABLE);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendNotRegistered()) {
                Element conditionElement = (Element) document.createElement(TAG_NOT_REGISTERED);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendRoaming()) {
                Element conditionElement = (Element) document.createElement(TAG_ROAMING);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendRuleDeactivated()) {
                Element conditionElement = (Element) document.createElement(TAG_RULE_DEACTIVATED);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendInternational()) {
                Element conditionElement = (Element) document.createElement(TAG_INTERNATIONAL);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendInternationalExHc()) {
                Element conditionElement = (Element) document.createElement(TAG_INTERNATIONAL_EXHC);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendCommunicationDiverted()) {
                Element conditionElement =
                        (Element) document.createElement(TAG_COMMUNICATION_DIVERTED);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendPresenceStatus()) {
                Element conditionElement = (Element) document.createElement(TAG_PRESENCE_STATUS);
                conditionsElement.appendChild(conditionElement);
            }

            if (mMedias != null) {
                if (mMedias.size() > 0) {
                    Iterator<String> it = mMedias.iterator();
                    while (it.hasNext()) {
                        Element ruleElement = (Element) document.createElement(TAG_MEDIA);
                        ruleElement.setTextContent(it.next());
                        conditionsElement.appendChild(ruleElement);
                    }
                }
            }

            if (comprehendAnonymous()) {
                Element conditionElement = (Element) document.createElement(TAG_ANONYMOUS);
                conditionsElement.appendChild(conditionElement);
            }

            if (comprehendTime() != null && !comprehendTime().isEmpty()) {
                Element conditionElement = (Element) document.createElement(TAG_TIME);
                conditionElement.setTextContent(mComprehendTime);
                conditionsElement.appendChild(conditionElement);
            }

            return conditionsElement;
        }
    }

    /**
     * Turn on Busy.
     *
     */
    public void addBusy() {
        mComprehendBusy = true;
    }

    /**
     * Turn on NoAnswer.
     *
     */
    public void addNoAnswer() {
        mComprehendNoAnswer = true;
    }

    /**
     * Turn on NotReachable.
     *
     */
    public void addNotReachable() {
        mComprehendNotReachable = true;
    }

    /**
     * Turn on NotRegistered.
     *
     */
    public void addNotRegistered() {
        mComprehendNotRegistered = true;
    }

    /**
     * Turn on Roaming.
     *
     */
    public void addRoaming() {
        mComprehendRoaming = true;
    }

    /**
     * Turn on RuleDeactivated.
     *
     */
    public void addRuleDeactivated() {
        mComprehendRuleDeactivated = true;
    }

    /**
     * Turn on International.
     *
     */
    public void addInternational() {
        mComprehendInternational = true;
    }

    /**
     * Turn on InternationalExHc.
     *
     */
    public void addInternationalExHc() {
        mComprehendInternationalexHc = true;
    }

    /**
     * Turn on CommunicationDiverted.
     *
     */
    public void addCommunicationDiverted() {
        mComprehendCommunicationDiverted = true;
    }

    /**
     * Turn on PresenceStatus.
     *
     */
    public void addPresenceStatus() {
        mComprehendPresenceStatus = true;
    }

    /**
     * Turn on Anonymous.
     *
     */
    public void addAnonymous() {
        mComprehendAnonymous = true;
    }

    public void removeRuleDeactivated() {
        mComprehendRuleDeactivated = false;
    }

    /**
     * Get Busy value.
     *
     * @return value
     */
    public boolean comprehendBusy() {
        return mComprehendBusy;
    }

    /**
     * Get NoAnswer value.
     *
     * @return value
     */
    public boolean comprehendNoAnswer() {
        return mComprehendNoAnswer;
    }

    /**
     * Get NotReachable value.
     *
     * @return value
     */
    public boolean comprehendNotReachable() {
        return mComprehendNotReachable;
    }

    /**
     * Get NotRegistered value.
     *
     * @return value
     */
    public boolean comprehendNotRegistered() {
        return mComprehendNotRegistered;
    }

    /**
     * Get Romaing value.
     *
     * @return value
     */
    public boolean comprehendRoaming() {
        return mComprehendRoaming;
    }

    /**
     * Get RuleDeactivated value.
     *
     * @return value
     */
    public boolean comprehendRuleDeactivated() {
        return mComprehendRuleDeactivated;
    }

    /**
     * Get International value.
     *
     * @return value
     */
    public boolean comprehendInternational() {
        return mComprehendInternational;
    }

    /**
     * Get InternationalExHc value.
     *
     * @return value
     */
    public boolean comprehendInternationalExHc() {
        return mComprehendInternationalexHc;
    }

    /**
     * Get Communication Diverted value.
     *
     * @return value
     */
    public boolean comprehendCommunicationDiverted() {
        return mComprehendCommunicationDiverted;
    }

    /**
     * Get PresenceStatus value.
     *
     * @return value
     */
    public boolean comprehendPresenceStatus() {
        return mComprehendPresenceStatus;
    }

    /**
     * Get Anonymous value.
     *
     * @return value
     */
    public boolean comprehendAnonymous() {
        return mComprehendAnonymous;
    }

    /**
     * Add Time value.
     *
     * @param   time   time value
     */
    public void addTime(String time) {
        mComprehendTime = time;
    }

    /**
     * Get Time value.
     *
     * @return value
     */
    public String comprehendTime() {
        return mComprehendTime;
    }

    /**
     * Add Media value.
     *
     * @param   media   media value
     */
    public void addMedia(String media) {
        if (mMedias == null) {
            mMedias = new LinkedList<String>();
        }
        mMedias.add(media);
    }

    public List<String> getMedias() {
        return mMedias;
    }

    /**
     * Reset condition value.
     *
     */
    public void clearConditions() {
        mComprehendBusy = false;
        mComprehendNoAnswer = false;
        mComprehendNotReachable = false;
        mComprehendNotRegistered = false;
        mComprehendRoaming = false;
        mComprehendRuleDeactivated = false;
        mComprehendInternational = false;
        mComprehendCommunicationDiverted = false;
        mComprehendPresenceStatus = false;
        if (mMedias == null) {
            mMedias = new LinkedList<String>();
        }

        mMedias.clear();
        mComprehendAnonymous = false;
        mComprehendTime = null;
    }
}
