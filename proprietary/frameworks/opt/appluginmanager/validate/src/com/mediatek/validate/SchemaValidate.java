package com.mediatek.validate;

import org.xml.sax.SAXException;

import java.io.IOException;
import java.io.InputStream;

import mf.javax.xml.transform.Source;
import mf.javax.xml.transform.stream.StreamSource;
import mf.javax.xml.validation.Schema;
import mf.javax.xml.validation.SchemaFactory;
import mf.javax.xml.validation.Validator;
import mf.org.apache.xerces.jaxp.validation.XMLSchemaFactory;

/**
 * Schema validate for plugin.xml.
 */
public class SchemaValidate {
    /**
     * validate XML File by XSD File.
     * @param xsdInputStream
     *            The inputStream of XSD file.
     * @param xmlInputStream
     *            The inputSream of XML file.
     * @return whether succeed validate the XML file or not.
     */
    public boolean validateXMLFile(InputStream xsdInputStream, InputStream xmlInputStream) {
        boolean flag = false;
        try {
            Source schemaSource = new StreamSource(xsdInputStream);
            Source xmlSource = new StreamSource(xmlInputStream);
            SchemaFactory schemaFactory = new XMLSchemaFactory();
            Schema schema = schemaFactory.newSchema(schemaSource);
            Validator validator = schema.newValidator();
            validator.validate(xmlSource);
            flag = true;
        } catch (SAXException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return flag;
    }
}
