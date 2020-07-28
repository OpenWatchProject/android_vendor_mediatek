package com.mediatek.simservs.xcap;

import org.w3c.dom.Node;

/**
 * Configure Type interface.
 *
 */
public interface ConfigureType {

    /**
     * Instanciate from XML.
     *
     * @param domNode   DOM node
     */
    public void instantiateFromXmlNode(Node domNode);
}
