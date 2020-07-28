package com.mediatek.plugin.builder;

import com.mediatek.plugin.element.Element;
import com.mediatek.plugin.element.ExtensionPoint;
import com.mediatek.plugin.res.IResource;
import com.mediatek.plugin.utils.Log;

import org.xmlpull.v1.XmlPullParser;

/**
 * parser ExtensionPointBuilder tag of XML for element.
 */
class ExtensionPointBuilder extends Builder {
    private final static String TAG = "PluginManager/ExtensionPointBuilder";
    private final static String SUPPORT_TAG = "extension-point";
    private final static String VALUE_ID = "id";
    private final static String VALUE_CLASS = "class";

    @Override
    public String getSupportedTag() {
        return SUPPORT_TAG;
    }

    @Override
    public Element parser(XmlPullParser xmlPullParser, IResource attributeParser) {
        Log.d(TAG, "<parser> START_TAG  >>>>>>>> name = " + xmlPullParser.getName());
        ExtensionPoint extensionPoint = new ExtensionPoint();
        extensionPoint.id = xmlPullParser.getAttributeValue(null, VALUE_ID);
        extensionPoint.className = xmlPullParser.getAttributeValue(null, VALUE_CLASS);
        return extensionPoint;
    }

}
