package com.mediatek.engineermode.audio;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import com.mediatek.engineermode.Elog;

/**
 *This class deal with the xml file.
*/
public class ContentHandler extends DefaultHandler {
    private static final String TAG = "Audio/ContentHandler";

    private String mNodeName;

    private StringBuilder mOption;

    AudioLoggerXMLData mAudioLoggerXMLData;

    DumpOptions mDumpOptions = null;

    /**
     * This class get the AudioLoggerXMLData get from main.
     * @param xmlData
     *            xmlData
     * */
    public ContentHandler(AudioLoggerXMLData xmlData) {
        mAudioLoggerXMLData = xmlData;
    }

    @Override
    public void startDocument() throws SAXException {

        mOption = new StringBuilder();

        Elog.d(TAG, "startDocument");
    }

    @Override
    public void startElement(String uri, String localName, String qName,
            Attributes attributes) throws SAXException {
       Elog.d(TAG, "uri:" + uri + " localName:" + localName + " qName:" + qName);

        mNodeName = localName;

        if ("Category".equals(mNodeName)) {

            String myTitle = attributes.getValue("title");

            if (!myTitle.isEmpty()) {
                Elog.d(TAG, "myTitle:" + myTitle);

                mDumpOptions = new DumpOptions();

                mDumpOptions.mCategoryTitle = myTitle;

            }

        } else if ("Option".equals(mNodeName)) {

            String type = attributes.getValue("type");
            String mCmd = attributes.getValue("cmd");
            String check = attributes.getValue("check");
            String uncheck = attributes.getValue("uncheck");

            Elog.d(TAG, "attributes.getValue(type):" + type);
            Elog.d(TAG, "attributes.getValue(cmd):" + mCmd);
            Elog.d(TAG, "attributes.getValue(check):" + check);
            Elog.d(TAG, "attributes.getValue(uncheck):" + uncheck);

            // if(!str.isEmpty())
            mDumpOptions.mType.add(type);
            mDumpOptions.mCmd.add(mCmd);
            mDumpOptions.mCheck.add(check);
            mDumpOptions.mUncheck.add(uncheck);
        }

    }

    @Override
    public void characters(char[] ch, int start, int length) throws SAXException {

        if ("Option".equals(mNodeName)) {
            mOption.append(ch, start, length);
        }
    }

    @Override
    public void endElement(String uri, String localName, String qName)
            throws SAXException {
        if ("Category".equals(localName)) {

            Elog.d(TAG, "endElement,Category->mOption:" + mOption.toString());

            if (!mOption.toString().isEmpty()) {
                String[] str = mOption.toString().trim().replaceAll("\r|\n", ",")
                        .replaceAll("\\s*", "").split(",");

                for (int i = 0; i < str.length; i++) {
                    mDumpOptions.mCmdName.add(str[i]);
                }

                mAudioLoggerXMLData.mAudioDumpOperation.add(mDumpOptions);
            }
            mOption.setLength(0);
        } else if ("SetAudioCommand".equals(localName)) {
            String[] str = mOption.toString().trim().replaceAll("\r|\n", "@")
                    .replaceAll("\\s*", "").split("@");
            for (int i = 0; i < str.length; i++) {
                mAudioLoggerXMLData.setAudioCommandSetOperation(str[i]);
            }
            mOption.setLength(0);
        } else if ("GetAudioCommand".equals(localName)) {
            String[] str = mOption.toString().trim().replaceAll("\r|\n", ",")
                    .replaceAll("\\s*", "").split(",");
            for (int i = 0; i < str.length; i++) {
                mAudioLoggerXMLData.setAudioCommandGetOperation(str[i]);
            }
            mOption.setLength(0);
        } else if ("SetParameters".equals(localName)) {
            String[] str = mOption.toString().trim().replaceAll("\r|\n", ",")
                    .replaceAll("\\s*", "").split(",");
            for (int i = 0; i < str.length; i++) {
                mAudioLoggerXMLData.setParametersSetOperation(str[i]);
            }
            mOption.setLength(0);
        } else if ("GetParameters".equals(localName)) {
            String[] str = mOption.toString().trim().replaceAll("\r|\n", ",")
                    .replaceAll("\\s*", "").split(",");
            for (int i = 0; i < str.length; i++) {
                mAudioLoggerXMLData.setParametersGetOperation(str[i]);
            }
            mOption.setLength(0);
        }
    }

    @Override
    public void endDocument() throws SAXException {

        Elog.d(TAG, "endDocument");

        for (int i = 0; i < mAudioLoggerXMLData.mAudioCommandSetOperation.size(); i++) {
            Elog.d(TAG, "mAudioLoggerXMLData.mAudioCommandSetOperation:" + i + " : "
                    + mAudioLoggerXMLData.mAudioCommandSetOperation.get(i));
        }
        for (int i = 0; i < mAudioLoggerXMLData.mAudioCommandGetOperation.size(); i++) {
            Elog.d(TAG, "mAudioLoggerXMLData.mAudioCommandGetOperation:" + i + " : "
                    + mAudioLoggerXMLData.mAudioCommandGetOperation.get(i));
        }
        for (int i = 0; i < mAudioLoggerXMLData.mParametersSetOperationItems.size(); i++) {
            Elog.d(TAG, "mAudioLoggerXMLData.mParametersSetOperationItems:" + i
                    + " : " + mAudioLoggerXMLData.mParametersSetOperationItems.get(i));
        }

        for (int i = 0; i < mAudioLoggerXMLData.mParametersGetOperationItems.size(); i++) {
            Elog.d(TAG, "mAudioLoggerXMLData.mParametersGetOperationItems:" + i
                    + " : " + mAudioLoggerXMLData.mParametersGetOperationItems.get(i));
        }

        for (int i = 0; i < mAudioLoggerXMLData.mAudioDumpOperation.size(); i++) {
            Elog.d(TAG, "mAudioLoggerXMLData.mAudioDumpOperation,title:" + i + " : "
                    + mAudioLoggerXMLData.mAudioDumpOperation.get(i).mCategoryTitle);
            for (int j = 0; j < mAudioLoggerXMLData.mAudioDumpOperation.get(i).mCmdName
                    .size(); j++) {
                Elog.d(TAG, "mAudioLoggerXMLData.mAudioDumpOperation,mCmd:"
                        + mAudioLoggerXMLData.mAudioDumpOperation.get(i).mCmd.get(j));
                Elog.d(TAG, "mAudioLoggerXMLData.mAudioDumpOperation,mCmd name:"
                        + mAudioLoggerXMLData.mAudioDumpOperation.get(i).mCmdName.get(j));
            }

        }

    }

}
