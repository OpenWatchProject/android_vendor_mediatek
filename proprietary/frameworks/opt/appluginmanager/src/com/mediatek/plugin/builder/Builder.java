package com.mediatek.plugin.builder;

import com.mediatek.plugin.element.Element;
import com.mediatek.plugin.res.IResource;

import org.xmlpull.v1.XmlPullParser;

/**
 * Do Parse XML operation in this class.
 */
public abstract class Builder {

    /**
     * Associated parent-child.
     * @param parent
     *            The parent Element.
     * @param child
     *            The child Element.
     */
    public void bind(Element parent, Element child) {
        parent.addChild(child);
        child.setParent(parent);
    }

    /**
     * Get the builder support tag.
     * @return The XML tag.
     */
    public abstract String getSupportedTag();

    /**
     * Parser XML.
     * @param xmlPullParser
     *            Use XmlPullParser for parser XML.
     * @param attributeParser
     *            Use attributeParser for parsering attribute.
     * @return Current element.
     */
    public abstract Element parser(XmlPullParser xmlPullParser, IResource attributeParser);

}
