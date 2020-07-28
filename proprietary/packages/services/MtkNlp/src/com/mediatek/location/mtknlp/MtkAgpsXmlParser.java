package com.mediatek.location.mtknlp;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

import android.util.Log;

public class MtkAgpsXmlParser {

    public final static String supportMajVer = "1";
    public final static String supportMinVer = "1";

    protected ArrayList<NlpProfile> nlpProfiles;
    protected String versionString;
    protected Document doc;

    public MtkAgpsXmlParser() throws IOException {
        this("/etc/agps_profiles_conf2.xml");
    }

    // TODO need to check range of each parameters
    public MtkAgpsXmlParser(String file) throws IOException {
        nlpProfiles = new ArrayList<NlpProfile>();

        doc = getXmlDocument(file);

        // check version
        String majVer = doc.getDocumentElement().getAttribute("maj_ver");
        String minVer = doc.getDocumentElement().getAttribute("min_ver");
        if (!supportMajVer.equals(majVer)) {
            throw new IOException("Version Incompatiable, Parser="
                    + supportMajVer + "." + supportMinVer + " XML_file="
                    + majVer + "." + minVer);
        }
        if (!supportMinVer.equals(minVer)) {
            log("Version Warning, Parser=" + supportMajVer + "."
                    + supportMinVer + " XML_file=" + majVer + "." + minVer);
        }
        versionString = "parser=[" + supportMajVer + "." + supportMinVer
                + "] xml=[" + majVer + "." + minVer + "]";

        NodeList nList;

        nList = doc.getElementsByTagName("nlp_profile");
        for (int i = 0; i < nList.getLength(); i++) {
            Node nNode = nList.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) nNode;
                NlpProfile profile = new NlpProfile();
                profile.setNlpName(e.getAttribute("name"));
                profile.setPackageName(e.getAttribute("package_name"));
                nlpProfiles.add(profile);
            }
        }
    }

    public ArrayList<NlpProfile> getNlpProfiles() {
        return nlpProfiles;
    }

    public String getVersionString() {
        return versionString;
    }

    public String toString() {
        String ret = "";
        ret += versionString + "\n";
        for (NlpProfile p : nlpProfiles) {
            ret += p + "\n";
        }
        return ret;
    }

    protected static Document getXmlDocument(String file) throws IOException {
        File fXmlFile = new File(file);
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder dBuilder;
        Document doc = null;
        try {
            dBuilder = dbFactory.newDocumentBuilder();
            doc = dBuilder.parse(fXmlFile);
        } catch (ParserConfigurationException e) {
            throw new IOException(e);
        } catch (SAXException e) {
            throw new IOException(e);
        }
        return doc;
    }

    protected static void log(Object msg) {
        //System.out.println(message);
        Log.d("MtkNlp [parser]:", msg.toString());
    }

    protected static void loge(Object msg) {
        //System.out.println("ERR: " + message);
        Log.d("MtkNlp [parser] ERR:", msg.toString());
    }
}
