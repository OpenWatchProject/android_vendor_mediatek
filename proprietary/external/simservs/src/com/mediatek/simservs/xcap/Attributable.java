package com.mediatek.simservs.xcap;

/**
 * Attributable interface.
 *
 */
public interface Attributable {
    /**
     * Get specific attribute by name.
     *
     * @param attribute attribute name
     * @return  attribute value
     * @throws  XcapException   if XCAP error
     */
    public String getByAttrName(String attribute) throws XcapException;

    /**
     * Set specific attribute by name.
     *
     * @param attrName     attribute name
     * @param attrValue     attribute value
     * @throws  XcapException   if XCAP error
     */
    public void setByAttrName(String attrName, String attrValue) throws XcapException;

    /**
     * Delete specific attribute by name.
     *
     * @param attribute     attribute name
     * @throws  XcapException   if XCAP error
     */
    public void deleteByAttrName(String attribute) throws XcapException;
}
