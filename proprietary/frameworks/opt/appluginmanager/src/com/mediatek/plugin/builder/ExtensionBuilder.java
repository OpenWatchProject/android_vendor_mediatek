package com.mediatek.plugin.builder;

import com.mediatek.plugin.element.Element;
import com.mediatek.plugin.element.Extension;
import com.mediatek.plugin.res.IResource;
import com.mediatek.plugin.utils.Log;

import org.xmlpull.v1.XmlPullParser;

/**
 * Parser ExtensionBuilder tag of XML for element.
 */
class ExtensionBuilder extends Builder {
    private final static String TAG = "PluginManager/ExtensionBuilder";
    private final static String SUPPORT_TAG = "extension";
    private final static String VALUE_ID = "id";
    private final static String VALUE_PLUGIN_ID = "plugin-id";
    private final static String VALUE_POINT_ID = "point-id";
    private final static String VALUE_NAME = "name";
    private final static String VALUE_ICON = "icon";
    private final static String VALUE_CLASS = "class";

    @Override
    public String getSupportedTag() {
        return SUPPORT_TAG;
    }

    @Override
    public Element parser(XmlPullParser xmlPullParser, IResource attributeParser) {
        Log.d(TAG, "<parser> START_TAG  >>>>>>>> name = " + xmlPullParser.getName());
        Extension extension = new Extension();
        extension.id = xmlPullParser.getAttributeValue(null, VALUE_ID);
        String name = xmlPullParser.getAttributeValue(null, VALUE_NAME);
        extension.name = attributeParser.getString(name);
        extension.pluginId = xmlPullParser.getAttributeValue(null, VALUE_PLUGIN_ID);
        extension.extensionPointId = xmlPullParser.getAttributeValue(null, VALUE_POINT_ID);
        String icon = xmlPullParser.getAttributeValue(null, VALUE_ICON);
        if (icon != null) {
            extension.drawable = attributeParser.getDrawable(icon);
        }
        extension.className = xmlPullParser.getAttributeValue(null, VALUE_CLASS);
        Log.d(TAG, "<parser> START_TAG  <<<<<<<<<< name = " + xmlPullParser.getName());
        return extension;
    }

}
