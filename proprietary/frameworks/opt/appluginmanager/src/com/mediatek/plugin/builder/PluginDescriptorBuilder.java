package com.mediatek.plugin.builder;

import com.mediatek.plugin.element.Element;
import com.mediatek.plugin.element.PluginDescriptor;
import com.mediatek.plugin.res.IResource;

import org.xmlpull.v1.XmlPullParser;

/**
 * parser PluginDescriptorBuilder tag of XML for element.
 */
class PluginDescriptorBuilder extends Builder {
    private final static String SUPPORT_TAG = "plugin";
    public static final String VALUE_ID = "id";
    public static final String VALUE_NAME = "name";
    public static final String VALUE_VERSION = "version";
    public static final String VALUE_CLASS = "class";
    public static final String VALUE_REQUEST_ID = "require-plugin-id";
    public static final String REQUIRE_MAX_HOST_VERSION = "require-max-host-version";
    public static final String REQUIRE_MIN_HOST_VERSION = "require-min-host-version";

    @Override
    public String getSupportedTag() {
        return SUPPORT_TAG;
    }

    @Override
    public Element parser(XmlPullParser xmlPullParser, IResource attributeParser) {
        PluginDescriptor element = new PluginDescriptor();
        element.id = xmlPullParser.getAttributeValue(null, VALUE_ID);
        element.name = xmlPullParser.getAttributeValue(null, VALUE_NAME);
        String version = xmlPullParser.getAttributeValue(null, VALUE_VERSION);
        if (version != null) {
            element.version = Integer.parseInt(version);
        }
        element.className = xmlPullParser.getAttributeValue(null, VALUE_CLASS);
        if (element.className == null) {
            element.className = "com.mediatek.plugin.Plugin";
        }
        String requestId = xmlPullParser.getAttributeValue(null, VALUE_REQUEST_ID);
        if (attributeParser != null) {
            element.requiredPluginIds = attributeParser.getString(requestId, "\\|");
        }
        String requireMaxHostVersion =
                xmlPullParser.getAttributeValue(null, REQUIRE_MAX_HOST_VERSION);
        if (requireMaxHostVersion != null) {
            element.requireMaxHostVersion = Integer.parseInt(requireMaxHostVersion);
        }
        String requireMinHostVersion =
                xmlPullParser.getAttributeValue(null, REQUIRE_MIN_HOST_VERSION);
        if (requireMinHostVersion != null) {
            element.requireMinHostVersion = Integer.parseInt(requireMinHostVersion);
        }
        return element;
    }

}
