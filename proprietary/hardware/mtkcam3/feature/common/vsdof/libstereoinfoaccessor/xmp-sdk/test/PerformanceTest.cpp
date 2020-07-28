//#define LOG_NDEBUG 0
#define LOG_TAG "PerformanceTest"
#include <utils/Log.h>

#include "PerformanceTest.h"
#include <string>

#define TXMP_STRING_TYPE std::string

// Must be defined to give access to XMPFiles
//#define XMP_INCLUDE_XMPFILES 1

// Ensure XMP templates are instantiated
#include "XMP.incl_cpp"

// Provide access to the API
#include "XMP.hpp"

#include "XMP_Const.h"

#include <iostream>
#include <fstream>
#include <sys/time.h>


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

void PerformanceTest::testWriteGDepth(const GoogleStereoInfo* info) {
    ALOGD("<testWriteGDepth> >>>");

    long startTime, endTime;
    long totalStartTime, totalEndtime;

    totalStartTime = getCurrentTime();
    ALOGD("<testWriteGDepth> SXMPMeta::Initialize!");
    // initialize XMPMeta
    if(!SXMPMeta::Initialize())
    {
        ALOGD("<testWriteGDepth>Could not initialize toolkit!");
        return;
    }
    SXMPMeta meta;
    // write simple value
    ALOGD("<testWriteGDepth> write simple value");
    string registeredPrefix;
    meta.RegisterNamespace(NS_GFOCUS, PRIFIX_GFOCUS, &registeredPrefix);
    meta.SetProperty_Float(NS_GFOCUS, "BlurAtInfinity", info->focusBlurAtInfinity);
    meta.SetProperty_Float(NS_GFOCUS, "FocalDistance", info->focusFocalDistance);
    meta.SetProperty_Float(NS_GFOCUS, "FocalPointX", info->focusFocalPointX);
    meta.SetProperty_Float(NS_GFOCUS, "FocalPointY", info->focusFocalPointY);

    meta.RegisterNamespace(NS_GIMAGE, PRIFIX_GIMAGE, &registeredPrefix);
    meta.SetProperty(NS_GIMAGE, "Mime", info->imageMime);

    meta.RegisterNamespace(NS_GDEPTH, PRIFIX_GDEPTH, &registeredPrefix);
    meta.SetProperty(NS_GDEPTH, "Format", info->depthFormat);
    meta.SetProperty_Float(NS_GDEPTH, "Near", info->depthNear);
    meta.SetProperty_Float(NS_GDEPTH, "Far", info->depthFar);
    meta.SetProperty(NS_GDEPTH, "Mime", info->depthMime);

    // depth map
    ALOGD("<testWriteGDepth> write depth map");
    startTime = getCurrentTime();
    string encodedDepthMap;
    SXMPUtils::EncodeToBase64(info->depthMap, info->depthMapSize, &encodedDepthMap);
    endTime = getCurrentTime();
    ALOGD("<testWriteGDepth><Performance><depthMap><EncodeToBase64>\
buffer size = %d, spend time = %ld", info->depthMapSize, (endTime - startTime));

    startTime = getCurrentTime();
    meta.RegisterNamespace(NS_GDEPTH, PRIFIX_GDEPTH, &registeredPrefix);
    meta.SetProperty(NS_GDEPTH, "Data", encodedDepthMap);
    endTime = getCurrentTime();
    ALOGD("<testWriteGDepth><Performance><depthMap><SetProperty>\
buffer size = %d, spend time = %ld", info->depthMapSize, (endTime - startTime));

    // clear image
    ALOGD("<testWriteGDepth> write clear image");
    //dumpBufferToFile("/sdcard/sdk_writed_clearimage.raw", info->clearImage, info->clearImageSize);
    startTime = getCurrentTime();
    string encodedClearImage;
    SXMPUtils::EncodeToBase64(info->clearImage, info->clearImageSize, &encodedClearImage);
    endTime = getCurrentTime();
    ALOGD("<testWriteGDepth><Performance><clearImage><EncodeToBase64>\
buffer size = %d, spend time = %ld", info->clearImageSize, (endTime - startTime));

    startTime = getCurrentTime();
    meta.RegisterNamespace(NS_GIMAGE, PRIFIX_GIMAGE, &registeredPrefix);
    meta.SetProperty(NS_GIMAGE, "Data", encodedClearImage);
    endTime = getCurrentTime();
    ALOGD("<testWriteGDepth><Performance><clearImage><SetProperty>\
buffer size = %d, spend time = %ld", info->clearImageSize, (endTime - startTime));

    // serialize to RDF buffer
    ALOGD("<testWriteGDepth> serialize to RDF buffer");
    startTime = getCurrentTime();
    string metaBuffer;
    //meta.SerializeToBuffer(&metaBuffer, 0, 0, "", "", 0);
    meta.SerializeToBuffer(&metaBuffer, 0, 0);
    endTime = getCurrentTime();
    ALOGD("<testWriteGDepth><Performance><GDepth><SerializeToBuffer>\
spend time = %ld", (endTime - startTime));

    totalEndtime = getCurrentTime();
    ALOGD("<testReadClearImage><Performance><Total>spend time = %ld",
        (totalEndtime - totalStartTime));

    // write RDF buffer to file
    ALOGD("<testWriteGDepth> write RDF buffer to file");
    writeRDFToFile(&metaBuffer, "/sdcard/XMP_RDF.txt");
    ALOGD("<testWriteGDepth> <<<");
}


void PerformanceTest::testReadGDepth(GoogleStereoInfo* info) {
    ALOGD("<testReadGDepth> >>>");

    long startTime, endTime;
    long totalStartTime, totalEndtime;

    ALOGD("<testReadGDepth> readRDFFromFile");
    char* buffer = nullptr;
    int bufferSize = readRDFFromFile(&buffer, "/sdcard/XMP_RDF.txt");
    ALOGD("<testReadClearImage> buffer size = %d", bufferSize);
    ALOGD("<testReadClearImage> buffer = %x", buffer);
    ALOGD("<testReadGDepth> SXMPMeta::Initialize!");

    totalStartTime = getCurrentTime();
    if(!SXMPMeta::Initialize())
    {
        ALOGD("<testReadGDepth> Could not initialize toolkit!");
        return;
    }

    /*
    startTime = getCurrentTime();
    XMP_OptionBits options = 0;
    #if UNIX_ENV
        options |= kXMPFiles_ServerMode;
    #endif
    if ( ! SXMPFiles::Initialize ( options ) )
    {
        cout << "Could not initialize SXMPFiles.";
        return;
    }
    XMP_OptionBits opts = kXMPFiles_OpenForRead | kXMPFiles_OpenUseSmartHandler;
    bool ok;
    SXMPFiles myFile;
    char* filename = "/sdcard/XMP_RDF.txt";
    // First we try and open the file
    ok = myFile.OpenFile(filename, kXMP_UnknownFile, opts);
    if( ! ok )
    {
        // Now try using packet scanning
        opts = kXMPFiles_OpenForUpdate | kXMPFiles_OpenUsePacketScanning;
        ok = myFile.OpenFile(filename, kXMP_UnknownFile, opts);
    }
    if (! ok) {
        ALOGD("open file by XMPFiles failed");
        return;
    }
    SXMPMeta meta;
    myFile.GetXMP(&meta);
    endTime = getCurrentTime();
    ALOGD("<testReadGDepth><Performance><GDepth><ParseByXMPFiles>\
spend time = %ld", (endTime - startTime));
    */

    ALOGD("<testReadGDepth> ParseFromBuffer >>>");
    startTime = getCurrentTime();
    //SXMPMeta meta(buffer, bufferSize);
    SXMPMeta meta;
    meta.ParseFromBuffer(buffer, bufferSize);
    /*
    int i;
    int step = 4096;
    for (i = 0; i < bufferSize - step; i += step) {
        meta.ParseFromBuffer(&buffer[i], step, kXMP_ParseMoreBuffers);
    }
    meta.ParseFromBuffer (&buffer[i], bufferSize - i );
    */

    endTime = getCurrentTime();
    ALOGD("<testReadGDepth><Performance><GDepth><ParseFromBuffer>\
spend time = %ld", (endTime - startTime));
    ALOGD("<testReadGDepth> ParseFromBuffer <<<");

    ALOGD("<testReadGDepth> GetProperty>>>");
    meta.GetProperty_Float(NS_GFOCUS, "BlurAtInfinity", &(info->focusBlurAtInfinity), 0);
    meta.GetProperty_Float(NS_GFOCUS, "FocalDistance", &(info->focusFocalDistance), 0);
    meta.GetProperty_Float(NS_GFOCUS, "FocalPointX", &(info->focusFocalPointX), 0);
    meta.GetProperty_Float(NS_GFOCUS, "FocalPointY", &(info->focusFocalPointY), 0);

    string strImageMime;
    meta.GetProperty(NS_GIMAGE, "Mime", &strImageMime, 0);
    int imageMimeLen = strImageMime.length();
    char* imageMime = new char[imageMimeLen + 1];
    memset(imageMime, 0x00, imageMimeLen + 1);
    memcpy(imageMime, strImageMime.c_str(), imageMimeLen);
    info->imageMime = imageMime;
    //ALOGD("strImageMime size = %d", strImageMime.length());
    //ALOGD("imageMime = %s", imageMime);

    string strDepthFormat;
    meta.GetProperty(NS_GDEPTH, "Format", &strDepthFormat, 0);
    int depthFormatLen = strDepthFormat.length();
    char* depthFormat = new char[depthFormatLen + 1];
    memset(depthFormat, 0x00, depthFormatLen + 1);
    memcpy(depthFormat, strDepthFormat.c_str(), depthFormatLen);
    info->depthFormat = depthFormat;
    //ALOGD("depthFormat = %s", depthFormat);

    meta.GetProperty_Float(NS_GDEPTH, "Near", &(info->depthNear), 0);
    meta.GetProperty_Float(NS_GDEPTH, "Far", &(info->depthFar), 0);

    string strDepthMime;
    meta.GetProperty(NS_GDEPTH, "Mime", &strDepthMime, 0);
    int depthMimeLen = strDepthMime.length();
    char* depthMime = new char[depthMimeLen + 1];
    memset(depthMime, 0x00, depthMimeLen + 1);
    memcpy(depthMime, strDepthMime.c_str(), depthMimeLen);
    info->depthMime = depthMime;
    //ALOGD("depthMime = %s", depthMime);

    startTime = getCurrentTime();
    string encodedClearImage;
    meta.GetProperty(NS_GIMAGE, "Data", &encodedClearImage, 0);
    endTime = getCurrentTime();
    ALOGD("<testReadGDepth><Performance><clearImage><GetProperty>\
spend time = %ld", (endTime - startTime));

    startTime = getCurrentTime();
    string decodedClearImage;
    SXMPUtils::DecodeFromBase64(encodedClearImage, &decodedClearImage);
    int clearImageSize = decodedClearImage.length();
    char* clearImage = new char[clearImageSize];
    memset(clearImage, 0x00, clearImageSize);
    memcpy(clearImage, decodedClearImage.c_str(), clearImageSize);
    info->clearImage = clearImage;
    info->clearImageSize = clearImageSize;
    endTime = getCurrentTime();
    ALOGD("<testReadGDepth><Performance><clearImage><DecodeFromBase64>\
spend time = %ld", (endTime - startTime));

    //dumpBufferToFile("/sdcard/sdk_readed_clearimage.raw", info->clearImage, info->clearImageSize);

    startTime = getCurrentTime();
    string encodedDepthMap;
    meta.GetProperty(NS_GDEPTH, "Data", &encodedDepthMap, 0);
    endTime = getCurrentTime();
    ALOGD("<testReadGDepth><Performance><depthMap><GetProperty>\
spend time = %ld", (endTime - startTime));

    startTime = getCurrentTime();
    string decodedDepthMap;
    SXMPUtils::DecodeFromBase64(encodedDepthMap, &decodedDepthMap);
    int depthMapSize = decodedDepthMap.length();
    char* depthMap = new char[depthMapSize];
    memset(depthMap, 0x00, depthMapSize);
    memcpy(depthMap, decodedDepthMap.c_str(), depthMapSize);
    info->depthMap = depthMap;
    info->depthMapSize = depthMapSize;
    endTime = getCurrentTime();
    ALOGD("<testReadGDepth><Performance><depthMap><DecodeFromBase64>\
spend time = %ld", (endTime - startTime));

    totalEndtime = getCurrentTime();


    ALOGD("<testReadClearImage>GetProperty<<<");
    ALOGD("<testReadClearImage><Performance><Total>spend time = %ld",
        (totalEndtime - totalStartTime));

    delete buffer;
}

void PerformanceTest::testWriteClearImage(const char *buffer, int len) {
    ALOGD("<testWriteClearImage>SXMPMeta::Initialize!");
    if(!SXMPMeta::Initialize())
    {
        ALOGD("<testWriteClearImage>Could not initialize toolkit!");
        return;
    }

    SXMPMeta meta;
    string output;
    ALOGD("<testWriteClearImage>EncodeToBase64>>>");
    SXMPUtils::EncodeToBase64(buffer, len, &output);
    ALOGD("<testWriteClearImage>EncodeToBase64<<<");

    ALOGD("<testWriteClearImage>SetProperty>>>");
    meta.SetProperty(kXMP_NS_XMP, "ClearImage", output, 0);
    ALOGD("<testWriteClearImage>SetProperty<<<");

    ALOGD("<testWriteClearImage>SerializeToBuffer>>>");
    string metaBuffer;
    meta.SerializeToBuffer(&metaBuffer, 0, 0, "", "", 0);
    ALOGD("<testWriteClearImage>SerializeToBuffer<<<");

    ALOGD("<testWriteClearImage>writeRDFToFile>>>");
    writeRDFToFile(&metaBuffer, "/sdcard/XMP_RDF.txt");
    ALOGD("<testWriteClearImage>writeRDFToFile>>>");
}

int PerformanceTest::testReadClearImage(char** clearImage) {
    ALOGD("<testReadClearImage>readRDFFromFile");
    char* buffer = nullptr;
    int bufferSize = readRDFFromFile(&buffer, "/sdcard/XMP_RDF.txt");
    ALOGD("<testReadClearImage> buffer size = %d", bufferSize);
    ALOGD("<testReadClearImage> buffer = %x", buffer);
    ALOGD("<testReadClearImage>SXMPMeta::Initialize!");
    if(!SXMPMeta::Initialize())
    {
        ALOGD("<testReadClearImage>Could not initialize toolkit!");
        return -1;
    }

    SXMPMeta meta;
    ALOGD("<testReadClearImage>ParseFromBuffer>>>");
    meta.ParseFromBuffer(buffer, bufferSize);
    ALOGD("<testReadClearImage>ParseFromBuffer<<<");

    ALOGD("<testReadClearImage>GetProperty>>>");
    string encodedProp;
    meta.GetProperty(kXMP_NS_XMP, "ClearImage", &encodedProp, 0);
    ALOGD("<testReadClearImage>GetProperty<<<");

    ALOGD("<testReadClearImage>DecodeFromBase64>>>");
    string decodedVal;
    SXMPUtils::DecodeFromBase64(encodedProp, &decodedVal);
    int len = decodedVal.length();
    char* out = new char[len];
    memcpy(out, decodedVal.c_str(), len);
    *clearImage = out;
    ALOGD("<testReadClearImage>DecodeFromBase64<<< len = %d", len);

    delete buffer;
    return len;
}

void PerformanceTest::dumpBufferToFile(const char* fileName, char* buffer, int size) {
    FILE* fp;
    int index;

    ALOGD("<dumpBufferToFile>buffer address:%p, bufferSize %d, fileName:%s", buffer,
            size, fileName);

    if (buffer == nullptr) {
        ALOGD("<dumpBufferToFile><ERROR> null buffer address, dump fail!!!");
        return;
    }

    fp = fopen(fileName, "w");
    if (fp == nullptr) {
        ALOGD("<dumpBufferToFile><ERROR> Open file %s failed.", fileName);
        return;
    }

    for (index = 0 ; index < size; index++) {
        fprintf(fp, "%c", buffer[index]);
    }
    fclose(fp);
}

int main() {
    printf("XMP Performance Test >>> \n");
    clock_t start,finish;

    if(!SXMPMeta::Initialize())
    {
        cout << "Could not initialize toolkit!";
        return -1;
    }

    SXMPMeta meta;

    printf("SetProperty\n");
    meta.SetProperty(kXMP_NS_XMP, "CreatorTool", "Updated By XMP SDK", 0);

    int inputSize = 3326274;
    char* input = (char*)malloc(sizeof(char) * inputSize);

    if (input == nullptr) {
        cout << "Allocate memory fail.";
        return -1;
    }

    for (int i = 0; i < inputSize; i++) {
        input[i] = rand() % 255;
    }
    string output;

    printf("EncodeToBase64\n");
    start=clock();
    SXMPUtils::EncodeToBase64(input, inputSize, &output);
    free(input);
    finish =clock();
    // cout << "EncodeToBase64 output: " << output << std::endl;
    cout << "EncodeToBase64 spend time: " << (float)(finish - start) / CLOCKS_PER_SEC * 1000 << std::endl;

    printf("SetProperty large buffer\n");
    start=clock();
    meta.SetProperty(kXMP_NS_XMP, "LargeBuffer", output, 0);
    finish =clock();
    cout << "SetProperty spend time: " << (float)(finish - start) / CLOCKS_PER_SEC * 1000 << std::endl;

    printf("SerializeToBuffer\n");
    string metaBuffer;
    start=clock();
    meta.SerializeToBuffer(&metaBuffer, 0, 0, "", "", 0);
    finish =clock();
    cout << "SerializeToBuffer spend time: " << (float)(finish - start) / CLOCKS_PER_SEC * 1000 << std::endl;

    printf("writeRDFToFile\n");
    writeRDFToFile(&metaBuffer, "/sdcard/XMP_RDF.txt");
}

long getCurrentTime() {
    struct timeval tv;
       gettimeofday(&tv,nullptr);
       return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

