package com.mediatek.plugin.builder;

import com.mediatek.plugin.element.Element;
import com.mediatek.plugin.element.Parameter;
import com.mediatek.plugin.res.IResource;

import org.xmlpull.v1.XmlPullParser;

/**
 * parser ParameterBuilder tag of XML for element.
 */
class ParameterBuilder extends Builder {
    private final static String SUPPORT_TAG = "parameter";
    private final static String VALUE_ID = "id";
    private final static String VALUE_V = "value";

    @Override
    public String getSupportedTag() {
        return SUPPORT_TAG;
    }

    @Override
    public Element parser(XmlPullParser xmlPullParser, IResource attributeParser) {
        Parameter element = new Parameter();
        element.id = xmlPullParser.getAttributeValue(null, VALUE_ID);
        element.value = xmlPullParser.getAttributeValue(null, VALUE_V);
        return element;
    }

}
