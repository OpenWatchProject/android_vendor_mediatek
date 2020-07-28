
package com.mediatek.ims.internal;

import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;

/**
 * Responsible for parsing Dialog event package.
 *
 */
public interface DialogEventPackageParser {
    /**
     * Parses Diaglog Event Package and returns as dialogInfo instance.
     * @param in inputStream of Dialog Event Package XML
     * @return return the dialogInfo instance
     * @throws XmlPullParserException if XML Pull Parser related faults occur
     * @throws IOException if an error occurred while reading
     */
    DialogInfo parse(InputStream in) throws XmlPullParserException, IOException;
}
