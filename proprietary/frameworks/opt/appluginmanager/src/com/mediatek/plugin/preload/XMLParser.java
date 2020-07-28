package com.mediatek.plugin.preload;

import com.mediatek.plugin.builder.Builder;
import com.mediatek.plugin.builder.BuilderFactory;
import com.mediatek.plugin.element.Element;
import com.mediatek.plugin.res.IResource;
import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.TraceHelper;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.IOException;
import java.io.InputStream;
import java.util.Stack;

/**
 * Parser XML file.
 */
class XMLParser {
    private final static String TAG = "PluginManager/XMLParser";
    private InputStream mInputStream;
    private IResource mResParser = null;

    /**
     * Constructor.
     * @param inputStream
     *            The XML stream
     * @param res
     *            The resource operator
     */
    public XMLParser(InputStream inputStream, IResource res) {
        mInputStream = inputStream;
        mResParser = res;
    }

    /**
     * Do parse XML operation.
     * @return the Element parsed from XML.
     */
    public Element parserXML() {
        if (mInputStream == null) {
            return null;
        }
        TraceHelper.beginSection(">>>>XMLParser-parserXML");
        Element root = new Element();
        root.id = "Root";
        Stack<Element> stack = new Stack<Element>();
        Builder builder = null;
        Element top = null;
        try {
            TraceHelper.beginSection(">>>>XMLParser-parserXML-new XmlPullParserFactory");
            XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
            TraceHelper.endSection();
            TraceHelper.beginSection(">>>>XMLParser-parserXML-new XmlPullParser");
            XmlPullParser xmlPullParser = factory.newPullParser();
            TraceHelper.endSection();
            TraceHelper.beginSection(">>>>XMLParser-parserXML-setInput");
            xmlPullParser.setInput(mInputStream, "UTF-8");
            TraceHelper.endSection();
            int eventType = -1;
            try {
                while (eventType != XmlPullParser.END_DOCUMENT) {
                    TraceHelper.beginSection(">>>>XMLParser-parserXML-while");
                    TraceHelper.beginSection(">>>>XMLParser-parserXML-while-getEventType");
                    eventType = xmlPullParser.getEventType();
                    TraceHelper.endSection();
                    Log.d(TAG, "<parserXML> " + xmlPullParser.getName() + " || eventType = "
                            + eventType + " stack = " + stack.size());
                    switch (eventType) {
                        case XmlPullParser.START_DOCUMENT:
                            stack.push(root);
                            break;
                        case XmlPullParser.START_TAG:
                            builder = BuilderFactory.getBuilder(xmlPullParser.getName());
                            Log.d(TAG, "<parserXML> builder = " + builder);
                            if (builder != null) {
                                top = builder.parser(xmlPullParser, mResParser);
                                if (top != null) {
                                    stack.push(top);
                                }
                            }
                            break;
                        case XmlPullParser.END_TAG:
                            top = stack.pop();
                            Element parent = stack.peek();
                            if (builder != null && parent != null && top != null) {
                                builder.bind(parent, top);
                            }
                            break;
                        case XmlPullParser.END_DOCUMENT:
                            assert (stack.size() == 1);
                            stack.clear();
                            break;
                        case XmlPullParser.TEXT:
                            break;
                    }
                    TraceHelper.beginSection(">>>>XMLParser-parserXML-while-next");
                    eventType = xmlPullParser.next();
                    TraceHelper.endSection();
                    TraceHelper.endSection();
                }
            } catch (IOException e) {
                Log.d(TAG, "<parserXML>", e);
            }
        } catch (XmlPullParserException e) {
            Log.d(TAG, "<parserXML>", e);
        }
        TraceHelper.endSection();
        return top;
    }
}
