#include "DataItem.h"
#include "XmpMeta.h"
#include <iostream>
#include <fstream>

#include <string>
#include <stdlib.h>

using namespace std;
using namespace stereo;

/**
 * Writes an XMP packet in XML format to a text file
 *
 * rdf - a pointer to the serialized XMP
 * filename - the name of the file to write to
 */
void writeRDFToFile(string * rdf, string filename) {
    ofstream outFile;
    outFile.open(filename.c_str(), ios::out);
    outFile << *rdf;
    outFile.close();
}

int readRDFFromFile(char** outBuffer, string filename) {
    ifstream inFile;
    inFile.open(filename.c_str(), ios::binary|ios::ate);
    streamsize size = inFile.tellg();
    inFile.seekg(0, ios::beg);
    char* buffer = new char[size];
    inFile.read(buffer, size);
    inFile.close();
    *outBuffer = buffer;
    return size;
}

void mockBuffer(char** outBuffer, int bufferSize) {
    char *buffer = new char[bufferSize];
    for (int i = 0; i < bufferSize; i++)
    {
        buffer[i] = rand() % 255;
    }
    *outBuffer = buffer;
}

bool compareBuffer(char* firstBuffer, int firstBufferSize, char* secondBuffer, int secondBufferSize) {
    if (firstBufferSize != secondBufferSize) {
        return false;
    }
    for(int i = 0; i < firstBufferSize; i++) {
        if(firstBuffer[i] != secondBuffer[i]) {
            return false;
        }
    }
    return true;
}

void testNameSpaceItem() {
    NameSpaceItem nameSpaceItem;
    nameSpaceItem.dest = DEST_TYPE_EXTENDED_XMP;
    nameSpaceItem.nameSpace = "http://ns.google.com/photos/1.0/image/";
    nameSpaceItem.nameSpacePrefix = "Gdepth";
    cout << "testNameSpaceItem ..." << endl;
    cout << "dest = " << nameSpaceItem.dest << endl;
    cout << "nameSpace = " << nameSpaceItem.nameSpace << endl;
    cout << "nameSpacePrefix = " << nameSpaceItem.nameSpacePrefix << endl;
    cout << endl;
}

void testArrayItem() {
    ArrayItem arrayItem;
    arrayItem.dest = DEST_TYPE_EXTENDED_XMP;
    arrayItem.index = 0;
    arrayItem.name = "Array Name";
    arrayItem.value = "Array Value";
    arrayItem.nameSpaceItem.dest = DEST_TYPE_EXTENDED_XMP;
    arrayItem.nameSpaceItem.nameSpace = "http://ns.google.com/photos/1.0/image/";
    arrayItem.nameSpaceItem.nameSpacePrefix = "Gdepth";
    cout << "testArrayItem ..." << endl;
    cout << "index = " << arrayItem.index << endl;
    cout << "dest = " << arrayItem.dest << endl;
    cout << "name = " << arrayItem.name << endl;
    cout << "value = " << arrayItem.value << endl;
    cout << "nameSpace = " << arrayItem.nameSpaceItem.nameSpace << endl;
    cout << "nameSpacePrefix = " << arrayItem.nameSpaceItem.nameSpacePrefix << endl;
    cout << endl;
}

void testDataCollections() {
    DataCollections collections;

    ArrayItem arrayItem1;
    arrayItem1.dest = DEST_TYPE_EXTENDED_XMP;
    arrayItem1.index = 1;
    arrayItem1.name = "Array Name1";
    arrayItem1.value = "Array Value1";
    arrayItem1.nameSpaceItem.dest = DEST_TYPE_EXTENDED_XMP;
    arrayItem1.nameSpaceItem.nameSpace = "http://ns.google.com/photos/1.0/image/";
    arrayItem1.nameSpaceItem.nameSpacePrefix = "Gdepth";
    collections.listOfArrayItem.push_back(arrayItem1);

    ArrayItem arrayItem2;
    arrayItem2.dest = DEST_TYPE_EXTENDED_XMP;
    arrayItem2.index = 2;
    arrayItem2.name = "Array Name2";
    arrayItem2.value = "Array Value2";
    arrayItem2.nameSpaceItem.dest = DEST_TYPE_EXTENDED_XMP;
    arrayItem2.nameSpaceItem.nameSpace = "http://ns.google.com/photos/1.0/image/";
    arrayItem2.nameSpaceItem.nameSpacePrefix = "Gdepth";
    collections.listOfArrayItem.push_back(arrayItem2);

    ArrayItem arrayItem3;
    arrayItem3.dest = DEST_TYPE_EXTENDED_XMP;
    arrayItem3.index = 3;
    arrayItem3.name = "Array Name3";
    arrayItem3.value = "Array Value3";
    arrayItem3.nameSpaceItem.dest = DEST_TYPE_EXTENDED_XMP;
    arrayItem3.nameSpaceItem.nameSpace = "http://ns.google.com/photos/1.0/image/";
    arrayItem3.nameSpaceItem.nameSpacePrefix = "Gdepth";
    collections.listOfArrayItem.push_back(arrayItem3);

    cout << "testDataCollections ..." << endl;
    for (auto it = collections.listOfArrayItem.begin();
            it != collections.listOfArrayItem.end(); ++it) {
        ArrayItem item = *it;
        cout << "index = " << item.index << endl;
        cout << "dest = " << item.dest << endl;
        cout << "name = " << item.name << endl;
        cout << "value = " << item.value << endl;
        cout << "nameSpace = " << item.nameSpaceItem.nameSpace << endl;
        cout << "nameSpacePrefix = " << item.nameSpaceItem.nameSpacePrefix << endl;
        cout << endl;
    }
}

void testXmpMeta() {
    cout << "testXmpMeta" << endl;

    string rdfPath = "/sdcard/XMP_RDF.txt";
    const string NS_GFOCUS = "http://ns.google.com/photos/1.0/focus/";
    const string PRIFIX_GFOCUS = "GFocus";
    const string NS_GIMAGE = "http://ns.google.com/photos/1.0/image/";
    const string PRIFIX_GIMAGE = "GImage";
    const string NS_GDEPTH = "http://ns.google.com/photos/1.0/depthmap/";
    const string PRIFIX_GDEPTH = "GDepth";

    string blurAtInfinityName = "BlurAtInfinity";
    string blurAtInfinityValue = "0.055234075";
    string imageMimeName = "Mime";
    string imageMimeValue = "image/jpeg";
    string depthMimeName = "Mime";
    string depthMimeValue = "image/jpeg";

    string clearImageName = "Data";
    int clearImageSize = 100;
    char *clearImageValue = nullptr;
    mockBuffer(&clearImageValue, clearImageSize);

    string depthMapName = "Data";
    int depthMapSize = 50;
    char *depthMapValue = nullptr;
    mockBuffer(&depthMapValue, depthMapSize);

    XmpMeta meta(nullptr, 0);

    // write simple value
    cout << "write simple value... " << endl;
    meta.registerNamespace(NS_GFOCUS, PRIFIX_GFOCUS);
    meta.setPropertyString(NS_GFOCUS, blurAtInfinityName, blurAtInfinityValue);

    meta.registerNamespace(NS_GIMAGE, PRIFIX_GIMAGE);
    meta.setPropertyString(NS_GIMAGE, imageMimeName, imageMimeValue);

    meta.registerNamespace(NS_GDEPTH, PRIFIX_GDEPTH);
    meta.setPropertyString(NS_GDEPTH, depthMimeName, depthMimeValue);

    // write buffer
     cout << "write buffer... " << endl;
    meta.registerNamespace(NS_GIMAGE, PRIFIX_GIMAGE);
    meta.setPropertyBase64(NS_GIMAGE, clearImageName, clearImageValue, clearImageSize);

    meta.registerNamespace(NS_GDEPTH, PRIFIX_GDEPTH);
    meta.setPropertyBase64(NS_GDEPTH, depthMapName, depthMapValue, depthMapSize);

    // serialize
    cout << "serialize... " << endl;
    string rdfString;
    meta.serialize(&rdfString);

    // write to file
    cout << "writeRDFToFile " << rdfPath << endl;
    writeRDFToFile(&rdfString, rdfPath);

    // read from file
    cout << "readRDFFromFile " << rdfPath << endl;
    char* readedRdfString = nullptr;
    int readedRdfSize = readRDFFromFile(&readedRdfString, rdfPath);
    cout << "initialize meta " << endl;
    XmpMeta readMeta(readedRdfString, readedRdfSize);

    // read simple value
    cout << "read simple value... " << endl;
    string readedBlurAtInfinityValue;
    readMeta.getPropertyString(NS_GFOCUS, blurAtInfinityName, &readedBlurAtInfinityValue);
    cout << "readed BlurAtInfinityValue: " << readedBlurAtInfinityValue << endl;

    string readedImageMimeValue;
    readMeta.getPropertyString(NS_GIMAGE, imageMimeName, &readedImageMimeValue);
    cout << "readed ImageMimeValue: " << readedImageMimeValue << endl;

    string readedDepthMimeValue;
    readMeta.getPropertyString(NS_GDEPTH, depthMimeName, &readedDepthMimeValue);
    cout << "readed DepthMimeValue: " << readedDepthMimeValue << endl;

    cout << "read buffer... " << endl;
    char* readedClearImageValue = nullptr;
    int readedClearImageSize;
    readMeta.getPropertyBase64(NS_GIMAGE, clearImageName, &readedClearImageValue, &readedClearImageSize);
    int compareResult = compareBuffer(clearImageValue, clearImageSize, readedClearImageValue, readedClearImageSize);
    cout << "compare clearimage, result = " << compareResult << endl;

    char* readedDepthMapValue = nullptr;
    int readedDepthMapSize;
    readMeta.getPropertyBase64(NS_GDEPTH, depthMapName, &readedDepthMapValue, &readedDepthMapSize);
    compareResult = compareBuffer(depthMapValue, depthMapSize, readedDepthMapValue, readedDepthMapSize);
    cout << "compare depthMap, result = " << compareResult << endl;

    delete clearImageValue;
    delete readedClearImageValue;
    delete depthMapValue;
    delete readedDepthMapValue;
}

int main() {
    testNameSpaceItem();
    testArrayItem();
    testDataCollections();
    testXmpMeta();
    return 0;
}
