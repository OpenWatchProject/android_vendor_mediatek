package com.orangelabs.rcs.core.ims.network.registration;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.helpers.DefaultHandler;

import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Registration-Info parser
 */
public class RegistrationInfoParser extends DefaultHandler {

    private StringBuffer accumulator;
    private RegistrationInfo regInfo = null;
    private RegistrationInfo.Registration currRegistration = null;
    private RegistrationInfo.Contact currContact = null;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     *
     * @param inputSource Input source
     * @throws Exception
     */
    public RegistrationInfoParser(InputSource inputSource) throws Exception {
        SAXParserFactory factory = SAXParserFactory.newInstance();
        SAXParser parser = factory.newSAXParser();
        parser.parse(inputSource, this);
    }

    public RegistrationInfo getRegistrationInfo() {
        return regInfo;
    }

    public void startDocument() {
        if (logger.isActivated()) {
            logger.debug("Start document");
        }
        accumulator = new StringBuffer();
    }

    public void characters(char buffer[], int start, int length) {
        accumulator.append(buffer, start, length);
    }

    public void startElement(String namespaceURL, String localName, String qname, Attributes attr) {
        accumulator.setLength(0);

        if (localName.equals("reginfo")) {
            int version = Integer.parseInt(attr.getValue("version").trim());
            String state = attr.getValue("state").trim();
            regInfo = new RegistrationInfo(version, state);
        } else
        if (localName.equals("registration")) {
            String aor = attr.getValue("aor").trim();
            String id = attr.getValue("id").trim();
            String state = attr.getValue("state").trim();

            currRegistration = new RegistrationInfo.Registration(id, state, aor);
        } else
        if (localName.equals("contact")) {
            String id = attr.getValue("id").trim();
            String state = attr.getValue("state").trim();
            String event = attr.getValue("event").trim();

            currContact = new RegistrationInfo.Contact(id, state, event);

            String value = attr.getValue("duration-registered");
            if (value != null) {
                currContact.setDuration(Integer.parseInt(value.trim()));
            }
            value = attr.getValue("expires");
            if (value != null) {
                currContact.setExpires(Integer.parseInt(value.trim()));
            }
            value = attr.getValue("retry-after");
            if (value != null) {
                currContact.setRetryAfter(Integer.parseInt(value.trim()));
            }
            value = attr.getValue("q");
            if (value != null) {
                currContact.setPriority(Integer.parseInt(value.trim()));
            }
            value = attr.getValue("callid");
            if (value != null) {
                currContact.setCallId(value.trim());
            }
            value = attr.getValue("cseq");
            if (value != null) {
                currContact.setCSeq(value.trim());
            }
        }
    }

    public void endElement(String namespaceURL, String localName, String qname) {
        if (localName.equals("display-name")) {
            if (currContact != null) {
                currContact.setDisplayName(accumulator.toString().trim());
            }
        } else
        if (localName.equals("uri")) {
            if (currContact != null) {
                currContact.setUri(accumulator.toString().trim());
            }
        } else
        if (localName.equals("contact")) {
            if (currRegistration != null) {
                currRegistration.addContact(currContact);
                currContact = null;
            }
        } else
        if (localName.equals("registration")) {
            regInfo.addRegistration(currRegistration);
            currRegistration = null;
        }
    }

    public void endDocument() {
        if (logger.isActivated()) {
            logger.debug("End document");
        }
    }
}
