package com.mediatek.engineermode.rsc;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.rsc.ConfigXMLData.ProjectData;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;



/**
 *This class deal with the xml file.
*/
public class ContentHandler extends DefaultHandler {
    private static final String TAG = "rcs/ContentHandler";
    private static final String VER_TAG = "runtime_switchable_config";
    private static final String VER_ATTR = "version";
    private static final String MAGIC_TAG = "magic";
    private static final String PROJ_TAG = "proj_item";
    private static final String INDEX_ATTR = "index";
    private static final String TAR_PARTITON_TAG = "part_info";
    private static final String NAME_TAG = "name";
    private static final String OPTR_TAG = "operator";
    private static final String OFFSET_TAG = "offset";

    private String mNodeName;
    private StringBuilder mTempStr;
    private ConfigXMLData mConfigXmlData;

    private boolean mIsTarPart = false;
    private ProjectData mProjData;


    /**
     * Construct function.
     * @param xmlData XML data holder.
    */
    public ContentHandler(ConfigXMLData xmlData) {
        mConfigXmlData = xmlData;
        mTempStr = new StringBuilder();
    }

    @Override
    public void startElement(String uri, String localName, String qName,
            Attributes attributes) throws SAXException {
        Elog.d(TAG, "startElement qName:" + qName);
        mNodeName = qName;
        mTempStr.setLength(0);
        if (TAR_PARTITON_TAG.equals(qName)) {
            mIsTarPart = true;
        } else if (VER_TAG.equals(qName)) {
            int version = Integer.valueOf(attributes.getValue(VER_ATTR));
            mConfigXmlData.setVersion(version);
        } else if (PROJ_TAG.equals(qName)) {
            mProjData = mConfigXmlData.new ProjectData();
            Elog.d(TAG, "new mProjData");
            String strIndex = attributes.getValue(INDEX_ATTR);
            mProjData.setIndex(Integer.valueOf(strIndex));
        }
    }

    @Override
    public void characters(char[] ch, int start, int length) throws SAXException {
        mTempStr.append(ch, start, length);
    }

    @Override
    public void endElement(String uri, String localName, String qName)
            throws SAXException {
        Elog.d(TAG, "endElement qName:" + qName);
        if (TAR_PARTITON_TAG.equals(qName)) {
            mIsTarPart = false;
        } else if (PROJ_TAG.equals(qName)) {
            mConfigXmlData.addProjectName(mProjData);
            Elog.d(TAG, "add mProjData");
        } else if (MAGIC_TAG.equals(mNodeName)) {
            mConfigXmlData.setMagic(mTempStr.toString());
        } else if (NAME_TAG.equals(mNodeName)) {
            if (mIsTarPart) {
                mConfigXmlData.setTarPartName(mTempStr.toString());
                Elog.d(TAG, "setTarPartName:" + mTempStr);
            } else {
                mProjData.setName(mTempStr.toString());
                Elog.d(TAG, "addProjectName:" + mTempStr);
            }
        } else if (OPTR_TAG.equals(mNodeName)) {
            mProjData.setOptr(mTempStr.toString());
        } else if (OFFSET_TAG.equals(mNodeName)) {
            mConfigXmlData.setTarPartOffset(mTempStr.toString());

        }
    }
}
