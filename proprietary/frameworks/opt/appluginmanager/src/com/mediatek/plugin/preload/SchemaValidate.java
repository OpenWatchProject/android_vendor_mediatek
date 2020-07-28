package com.mediatek.plugin.preload;

import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.TraceHelper;

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
 * Schema validate fo plugin.xml.
 */
public class SchemaValidate {
    private final static String TAG = "PluginManager/SchemaValidate";

    /**
     * validate XML File by XSD File.
     * @param xsdInputStream
     *            The inputStream of XSD file.
     * @param xmlInputStream
     *            The inputSream of XML file.
     * @return whether succeed validate the XML file or not.
     */
    public boolean validateXMLFile(InputStream xsdInputStream, InputStream xmlInputStream) {
        TraceHelper.beginSection(">>>>SchemaValidate-validateXMLFile");
        boolean flag = false;
        try {
            TraceHelper.beginSection(">>>>SchemaValidate-validateXMLFile-new-StreamSource-xsd");
            Source schemaSource = new StreamSource(xsdInputStream);
            TraceHelper.endSection();
            TraceHelper.beginSection(">>>>SchemaValidate-validateXMLFile-new-StreamSource-xml");
            Source xmlSource = new StreamSource(xmlInputStream);
            TraceHelper.endSection();

            TraceHelper.beginSection(">>>>SchemaValidate-validateXMLFile-new-XMLSchemaFactory");
            SchemaFactory schemaFactory = new XMLSchemaFactory();
            TraceHelper.endSection();
            TraceHelper.beginSection(">>>>SchemaValidate-validateXMLFile-newSchema");
            Schema schema = schemaFactory.newSchema(schemaSource);
            TraceHelper.endSection();
            TraceHelper.beginSection(">>>>SchemaValidate-validateXMLFile-new-newValidator");
            Validator validator = schema.newValidator();
            TraceHelper.endSection();
            TraceHelper.beginSection(">>>>SchemaValidate-validateXMLFile-validate");
            validator.validate(xmlSource);
            TraceHelper.endSection();
            flag = true;
        } catch (SAXException e) {
            Log.e(TAG, "<validateXMLFile> SAXException: ", e);
        } catch (IOException e) {
            Log.e(TAG, "<validateXMLFile> IOException: " + e);
        }
        TraceHelper.endSection();
        return flag;
    }
}
