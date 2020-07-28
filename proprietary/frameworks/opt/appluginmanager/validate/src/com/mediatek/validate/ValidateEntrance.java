package com.mediatek.validate;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
/**
 * Entrance class for Schema validate.
 */
public class ValidateEntrance {
    private final static int LENGHT_ARGS = 3;
    /**
     * @param args
     *            args[xml, xsd, validateResult]
     */
    public static void main(String[] args) {
        boolean result = false;
        if (args == null) {
            System.out.println("Please put xml, xsd and, validate result path!");
        } else if (args.length != LENGHT_ARGS) {
            System.out.println("Please put xml, xsd and, validate result path!");
        } else if (args.length == LENGHT_ARGS) {
            SchemaValidate schemaValidate = new SchemaValidate();
            InputStream xmlInputStream = getInputStream(args[0]);
            InputStream xsdInputStream = getInputStream(args[1]);
            result = schemaValidate.validateXMLFile(xsdInputStream, xmlInputStream);
        }
        saveResult(args[2], result);
    }

    private static InputStream getInputStream(String filePath) {
        File file = new File(filePath);
        InputStream inputStream = null;
        try {
            inputStream = new FileInputStream(file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return inputStream;
    }

    private static void saveResult(String filePath, boolean result) {
        FileOutputStream outputStream = getOutputStream(filePath);
        try {
            outputStream.write(String.valueOf(result).getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static FileOutputStream getOutputStream(String filePath) {
        FileOutputStream outputStream = null;
        try {
            outputStream = new FileOutputStream(new File(filePath));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return outputStream;
    }
}
