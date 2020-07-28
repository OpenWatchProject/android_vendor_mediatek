package com.mediatek.lbs.em2.utils;

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

import android.util.Log;

public class MtkAgpsXmlParser {

    public static void main(String[] args) {
        MtkAgpsXmlParser xml = null;
        try {
            xml = new MtkAgpsXmlParser("./xml/agps_profiles_conf2.xml");
        } catch (IOException e) {
            e.printStackTrace();
        }
        log(xml);
    }

    public final static String supportMajVer = "1";
    public final static String supportMinVer = "1";

    protected AgpsConfig agpsConfig;
    protected String versionString;
    protected Document doc;

    public MtkAgpsXmlParser() throws IOException {
        this("/etc/agps_profiles_conf2.xml");
    }

    public AgpsConfig getAgpsConfig() {
        return agpsConfig;
    }

    // TODO need to check range of each parameters
    public MtkAgpsXmlParser(String file) throws IOException {
        agpsConfig = new AgpsConfig();

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

        nList = doc.getElementsByTagName("supl_profile");
        for (int i = 0; i < nList.getLength(); i++) {
            Node nNode = nList.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) nNode;
                SuplProfile profile = new SuplProfile();
                profile.name = e.getAttribute("name");
                profile.addr = e.getAttribute("addr");
                profile.port = Integer.valueOf(e.getAttribute("port"));
                profile.tls = Boolean.valueOf(e.getAttribute("tls"));
                profile.mccMnc = e.getAttribute("mcc_mnc");
                profile.appId = e.getAttribute("app_id");
                profile.providerId = e.getAttribute("provider_id");
                profile.defaultApn = e.getAttribute("default_apn");
                profile.optionalApn = e.getAttribute("optional_apn");
                profile.optionalApn2 = e.getAttribute("optional_apn_2");
                profile.addressType = e.getAttribute("address_type");

                profile.correctIfInvalid();
                agpsConfig.getSuplProfiles().add(profile);
            }
        }

        nList = doc.getElementsByTagName("cur_supl_profile");
        for (int i = 0; i < nList.getLength(); i++) {
            Node nNode = nList.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) nNode;
                SuplProfile profile = agpsConfig.getCurSuplProfile();
                profile.name = e.getAttribute("name");
                profile.addr = e.getAttribute("addr");
                profile.port = Integer.valueOf(e.getAttribute("port"));
                profile.tls = Boolean.valueOf(e.getAttribute("tls"));
                profile.mccMnc = e.getAttribute("mcc_mnc");
                profile.appId = e.getAttribute("app_id");
                profile.providerId = e.getAttribute("provider_id");
                profile.defaultApn = e.getAttribute("default_apn");
                profile.optionalApn = e.getAttribute("optional_apn");
                profile.optionalApn2 = e.getAttribute("optional_apn_2");
                profile.addressType = e.getAttribute("address_type");
                profile.correctIfInvalid();
            }
        }


        nList = doc.getElementsByTagName("cdma_profile");
        for (int i = 0; i < nList.getLength(); i++) {
            Node nNode = nList.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) nNode;
                CdmaProfile p = agpsConfig.getCdmaProfile();
                p.name = e.getAttribute("name");
                p.mcpEnable = Boolean.valueOf(e.getAttribute("mcp_enable"));
                p.mcpAddr = e.getAttribute("mcp_addr");
                p.mcpPort = Integer.valueOf(e.getAttribute("mcp_port"));
                p.pdeAddrValid = Boolean.valueOf(e
                        .getAttribute("pde_addr_valid"));
                p.pdeIpType = Integer.valueOf(e.getAttribute("pde_ip_type"));
                p.pdeAddr = e.getAttribute("pde_addr");
                p.pdePort = Integer.valueOf(e.getAttribute("pde_port"));
                p.pdeUrlValid = Boolean
                        .valueOf(e.getAttribute("pde_url_valid"));
                p.pdeUrlAddr = e.getAttribute("pde_url_addr");

                p.correctIfInvalid();
            }
        }

        nList = doc.getElementsByTagName("agps_setting");
        for (int i = 0; i < nList.getLength(); i++) {
            Node nNode = nList.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) nNode;
                AgpsSetting setting = agpsConfig.getAgpsSetting();
                setting.agpsEnable = Boolean.valueOf(e
                        .getAttribute("agps_enable"));
                setting.agpsProtocol = Integer.valueOf(e
                        .getAttribute("agps_protocol"));
                setting.gpevt = Boolean.valueOf(e.getAttribute("gpevt"));

                setting.correctIfInvalid();
            }
        }

        nList = doc.getElementsByTagName("cp_setting");
        for (int i = 0; i < nList.getLength(); i++) {
            Node nNode = nList.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) nNode;
                CpSetting setting = agpsConfig.getCpSetting();
                setting.molrPosMethod = Integer.valueOf(e
                        .getAttribute("molr_pos_method"));
                setting.externalAddrEnable = Boolean.valueOf(e
                        .getAttribute("external_addr_enable"));
                setting.externalAddr = e.getAttribute("external_addr");
                setting.mlcNumberEnable = Boolean.valueOf(e
                        .getAttribute("mlc_number_enable"));
                setting.mlcNumber = e.getAttribute("mlc_number");
                setting.cpAutoReset = Boolean.valueOf(e
                        .getAttribute("cp_auto_reset"));
                setting.epcMolrLppPayloadEnable = Boolean.valueOf(e
                        .getAttribute("epc_molr_lpp_payload_enable"));
                setting.epcMolrLppPayload = hexStringToByteArray(e
                        .getAttribute("epc_molr_lpp_payload"));

                setting.correctIfInvalid();
            }
        }

        nList = doc.getElementsByTagName("up_setting");
        for (int i = 0; i < nList.getLength(); i++) {
            Node nNode = nList.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element e = (Element) nNode;
                UpSetting setting = agpsConfig.getUpSetting();
                setting.caEnable = Boolean.valueOf(e.getAttribute("ca_enable"));
                setting.niRequest = Boolean.valueOf(e
                        .getAttribute("ni_request"));
                setting.roaming = Boolean.valueOf(e.getAttribute("roaming"));
                setting.cdmaPreferred = Integer.valueOf(e
                        .getAttribute("cdma_preferred"));
                setting.prefMethod = Integer.valueOf(e
                        .getAttribute("pref_method"));
                setting.suplVersion = Integer.valueOf(e
                        .getAttribute("supl_version"));
                setting.suplLog = Boolean.valueOf(e.getAttribute("supl_log"));
                setting.msaEnable = Boolean.valueOf(e
                        .getAttribute("msa_enable"));
                setting.msbEnable = Boolean.valueOf(e
                        .getAttribute("msb_enable"));
                setting.ecidEnable = Boolean.valueOf(e
                        .getAttribute("ecid_enable"));
                setting.otdoaEnable = Boolean.valueOf(e
                        .getAttribute("otdoa_enable"));
                setting.qopHacc = Integer.valueOf(e.getAttribute("qop_hacc"));
                setting.qopVacc = Integer.valueOf(e.getAttribute("qop_vacc"));
                setting.qopLocAge = Integer.valueOf(e
                        .getAttribute("qop_loc_age"));
                setting.qopDelay = Integer.valueOf(e.getAttribute("qop_delay"));

                setting.lppEnable = Boolean.valueOf(e
                        .getAttribute("lpp_enable"));
                setting.correctIfInvalid();
            }
        }
    }

    public String getVersionString() {
        return versionString;
    }

    public String toString() {
        String ret = "";
        ret += versionString + "\n";
        ret += agpsConfig + "\n";
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

    protected static byte[] hexStringToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4) + Character
                    .digit(s.charAt(i + 1), 16));
        }
        return data;
    }

    protected static void log(Object msg) {
        //System.out.println(message);
        Log.d("LocationEM [agps]:", msg.toString());
    }

    protected static void loge(Object msg) {
        //System.out.println("ERR: " + message);
        Log.d("LocationEM [agps] ERR:", msg.toString());
    }
}
