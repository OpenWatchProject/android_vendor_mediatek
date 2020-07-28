package com.mediatek.plugin.builder;

import com.mediatek.plugin.element.Element;
import com.mediatek.plugin.element.ParameterDef;
import com.mediatek.plugin.res.IResource;

import org.xmlpull.v1.XmlPullParser;

/**
 * parser ParameterDefBuilder tag of XML for element.
 */
class ParameterDefBuilder extends Builder {
    private final static String SUPPORT_TAG = "parameter-def";
    private final static String VALUE_ID = "id";
    private final static String VALUE_TYPE = "type";

    @Override
    public String getSupportedTag() {
        return SUPPORT_TAG;
    }

    @Override
    public Element parser(XmlPullParser xmlPullParser, IResource attributeParser) {
        ParameterDef element = new ParameterDef();
        element.id = xmlPullParser.getAttributeValue(null, VALUE_ID);
        String type = xmlPullParser.getAttributeValue(null, VALUE_TYPE);
        if (type != null) {
            element.type = ParameterDef.ParameterType.valueOf(type.toUpperCase());
        }
        return element;
    }

}
