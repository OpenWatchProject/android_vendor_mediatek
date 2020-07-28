
package com.mediatek.ims.internal;

import android.util.Xml;

import com.mediatek.ims.internal.DialogInfo.Dialog;
import com.mediatek.ims.internal.DialogInfo.Local;
import com.mediatek.ims.internal.DialogInfo.MediaAttribute;
import com.mediatek.ims.internal.DialogInfo.Param;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;

import android.telephony.Rlog;

/**
 * Implement for the {@link DialogEventPackageParser}, which is responsible for parsing Dialog event
 * package.
 */
public class DepXmlPullParser implements DialogEventPackageParser {
    private static final String namespace = null;

    /**
     * Parses Diaglog Event Package and returns as dialogInfo instance.
     * @param in inputStream of Dialog Event Package XML
     * @return return the dialogInfo instance
     * @throws XmlPullParserException if XML Pull Parser related faults occur
     * @throws IOException if an error occurred while reading
     */
    public DialogInfo parse(InputStream in) throws XmlPullParserException,
            IOException {
        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false);
            parser.setInput(in, null);
            parser.nextTag();
            return readDialogInfo(parser);
        } finally {
            in.close();
        }
    }

    private DialogInfo readDialogInfo(XmlPullParser parser)
            throws XmlPullParserException, IOException {
        DialogInfo dialogInfo = new DialogInfo();

        parser.require(XmlPullParser.START_TAG, namespace, "dialog-info");
        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            String name = parser.getName();
            if (name.equals("dialog")) {
                Dialog dialog = readDialog(parser);
                dialogInfo.addDialog(dialog);
            } else {
                skip(parser);
            }
        }
        return dialogInfo;
    }

    private Dialog readDialog(XmlPullParser parser) throws XmlPullParserException,
            IOException {
        Local local = null;
        parser.require(XmlPullParser.START_TAG, namespace, "dialog");
        int dialogId = Integer.parseInt(parser.getAttributeValue(null, "id"));
        // TODO
        boolean exclusive = true;
        String state = "";

        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            String name = parser.getName();
            if (name.equals("sa:exclusive")) {
                String str = readText(parser);
                exclusive = Boolean.valueOf(str);
            } else if (name.equals("state")) {
                state = readText(parser);
            } else if (name.equals("local")) {
                local = readLocal(parser);
            } else {
                skip(parser);
            }
        }
        return new Dialog(dialogId, exclusive, state, local);
    }

    private Local readLocal(XmlPullParser parser) throws XmlPullParserException, IOException {
        Local local = new Local();

        parser.require(XmlPullParser.START_TAG, namespace, "local");
        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            String name = parser.getName();
            if (name.equals("identity")) {
                String identity = readText(parser);
                local.setIdentity(identity);
            } else if (name.equals("target")) {
                String targetUri = parser.getAttributeValue(null, "uri");
                local.setTargetUri(targetUri);
                // This is an empty-element tag. We should skip it.
                readTargetParamAttributesToLocal(parser, local);
                //skip(parser);
            } else if (name.equals("mediaAttributes")) {
                MediaAttribute mediaAttribute = readMediaAttributes(parser);
                local.addMediaAttribute(mediaAttribute);
            } else if (name.equals("param")) {
                Param param = readParam(parser);
                local.setParam(param);
                Rlog.d("DEP Parser", "read param from Local()");
            } else {
                skip(parser);
            }
        }
        return local;
    }

    private void readTargetParamAttributesToLocal(XmlPullParser parser, Local local)
            throws XmlPullParserException, IOException {
        parser.require(XmlPullParser.START_TAG, namespace, "target");
        Rlog.d("DEP Parser", "readTargetParamAttributesToLocal()");
        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            String name = parser.getName();
            if (name.equals("param")) {
                Param param = readParam(parser);
                local.setParam(param);
            } else {
                skip(parser);
            }
        }
    }

    private MediaAttribute readMediaAttributes(XmlPullParser parser)
            throws XmlPullParserException, IOException {
        String mediaType = "";
        String mediaDirection = "";
        boolean port0 = false;
        parser.require(XmlPullParser.START_TAG, namespace, "mediaAttributes");
        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            String name = parser.getName();
            if (name.equals("mediaType")) {
                mediaType = readText(parser);
            } else if (name.equals("mediaDirection")) {
                mediaDirection = readText(parser);
            } else if (name.equals("port0")) {
                port0 = true;
                // This is a complete empty-element tag. We should skip it.
                skip(parser);
            } else {
                skip(parser);
            }
        }
        return new MediaAttribute(mediaType, mediaDirection, port0);
    }

    private Param readParam(XmlPullParser parser) throws IOException, XmlPullParserException {
        parser.require(XmlPullParser.START_TAG, namespace, "param");
        String pname = parser.getAttributeValue(null, "pname");
        String pval = parser.getAttributeValue(null, "pval");
        // This is an empty-element tag. We should skip it.
        skip(parser);
        return new Param(pname, pval);
    }

    private String readText(XmlPullParser parser) throws IOException, XmlPullParserException {
        String result = "";
        if (parser.next() == XmlPullParser.TEXT) {
            result = parser.getText();
            parser.nextTag();
        }
        return result;
    }

    private void skip(XmlPullParser parser) throws XmlPullParserException, IOException {
        if (parser.getEventType() != XmlPullParser.START_TAG) {
            throw new IllegalStateException();
        }
        int depth = 1;
        while (depth != 0) {
            switch (parser.next()) {
                case XmlPullParser.END_TAG:
                    depth--;
                    break;
                case XmlPullParser.START_TAG:
                    depth++;
                    break;
                default:
                    break;
            }
        }
    }
}
