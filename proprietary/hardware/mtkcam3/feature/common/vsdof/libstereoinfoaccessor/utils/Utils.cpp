//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/Utils"

#include "Utils.h"
#include "StereoLog.h"
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fstream>
#include "BufferManager.h"
#include <utils/Trace.h>
#include <png.h>
#include <stdio.h>
#include <limits>


#define ATRACE_TAG ATRACE_TAG_ALWAYS

using namespace stereo;
using namespace std;

const bool Utils::ENABLE_BUFFER_DUMP = (Utils::isFileExist(DUMP_FILE_FOLDER) == 0);
const bool Utils::ENABLE_GDEPTH = (Utils::isFileExist(ENABLE_GDEPTH_CFG) == 0);

int Utils::isFileExist(const char *filePath) {
    if (filePath == nullptr) {
        return -1;
    }
    if (access(filePath, F_OK) == 0) {
        StereoLogD("<isFileExist> %s existed", filePath);
        return 0;
    }
    return -1;
}

int Utils::isDirExist(const char *dirPath) {
    StereoLogD("<isDirExist> dirPath: %s", dirPath);
    if (dirPath == nullptr) {
        StereoLogD("<isDirExist> error: dir path is null");
        return -1;
    }
    if (opendir(dirPath) == nullptr) {
        StereoLogD("<isDirExist> error: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int Utils::checkOrCreateDir(const char *dirPath) {
    if (isDirExist(dirPath) != 0) {
        StereoLogD("folder not exist, create it, %s", dirPath);
        return mkdir(dirPath, S_IRWXU);
    }
    return 0;
}

void Utils::int2str(const int &intVal, StereoString &strVal) {
    stringstream ss;
    ss << intVal;
    strVal = ss.str();
}

void Utils::d2str(const double &dVal, StereoString &strVal) {
    int prec = std::numeric_limits<double>::digits10;
    stringstream ss;
    ss.precision(prec);
    ss << dVal;
    strVal = ss.str();
    StereoLogD("d2str, prec =  %d, value = %s", prec, strVal.c_str());
}

StereoString Utils::buffer2Str(const StereoBuffer_t &buffer) {
    StereoString str((char*)buffer.data, buffer.size);
    return str;
}

StereoString Utils::buffer2Str(const StereoBuffer_t &buffer, S_UINT32 size) {
    StereoString str((char*)buffer.data, size);
    return str;
}

StereoString Utils::buffer2Str(
        const StereoBuffer_t &buffer, S_UINT32  offset, S_UINT32 size) {
    StereoString str((char*)buffer.data + offset, size);
    return str;
}


StereoString Utils::intToHexString(int val) {
    char hex[16] = { 0 };
    sprintf(hex, "0x%X", val);
    StereoString hexStr = hex;
    return hexStr;
}

void Utils::writeBufferToFile(const StereoString &destFile, const StereoBuffer_t &buffer) {
    StereoLogD("<writeBufferToFile> file path, %s", destFile.c_str());
    if (!buffer.isValid()) {
        StereoLogD("<writeBufferToFile> buffer is null");
        return;
    }
    ATRACE_NAME(">>>>Utils-writeBufferToFile");
    // check file path
    // get parent folder
    std::size_t found = destFile.find_last_of("/");
    StereoString parentFolder = destFile.substr(0, found);
    // check or create parent folder
    if (checkOrCreateDir(parentFolder.c_str()) != 0) {
        StereoLogD(
            "<writeBufferToFile> checkOrCreateDir fail, parent dir:%s, error: %s ",
            parentFolder.c_str(), strerror(errno));
    }

    // write buffer to file
    FILE *fp = fopen(destFile.c_str(), "wb");
    if (fp) {
        fwrite(buffer.data, buffer.size * sizeof( S_UINT8 ), 1, fp );
        fclose(fp);
    } else {
        StereoLogE(
            "<writeBufferToFile> write fail, error: %s ", strerror(errno));
    }
}


void Utils::writeStringToFile(const StereoString &destFile, const StereoString &value) {
    StereoLogD("<writeStringToFile> file path, %s", destFile.c_str());
    if (value.empty()) {
        StereoLogD("<writeStringToFile> value is null");
        return;
    }

    // check file path
    // get parent folder
    std::size_t found = destFile.find_last_of("/");
    StereoString parentFolder = destFile.substr(0, found);
    // check or create parent folder
    if (checkOrCreateDir(parentFolder.c_str()) != 0) {
        StereoLogD(
            "<writeStringToFile> checkOrCreateDir fail, parent dir:%s, error: %s ",
            parentFolder.c_str(), strerror(errno));
    }

    // write buffer to file
    ofstream outFile;
    outFile.open(destFile.c_str(), ios::out);
    outFile << value;
    outFile.close();
}

void Utils::readFileToBuffer(const StereoString &filePath, StereoBuffer_t &outBuffer) {
    StereoLogD("<readFileToBuffer> file path, %s", filePath.c_str());
    if (isFileExist(filePath.c_str()) != 0) {
        StereoLogD("<readFileToBuffer> file not exist, %s", filePath.c_str());
        return;
    }
    ATRACE_NAME(">>>>Utils-readFileToBuffer");
    FILE *fp = fopen(filePath.c_str(), "rb");
    if (fp) {
        struct stat statbuff;
        int size = -1;
        if(stat(filePath.c_str(), &statbuff) < 0){
            StereoLogE(
                "<readFileToBuffer> cannot stat file, error: %s ", strerror(errno));
            return;
        }else{
            size = statbuff.st_size;
        }
        BufferManager::createBuffer(size, outBuffer);
        int realRead = fread(outBuffer.data, size * sizeof( S_UINT8 ), 1, fp );
        if((size * sizeof( S_UINT8 ) == 0) || (realRead != 1))
            StereoLogE("<readFileToBuffer> nothing to read or read fail, error: %s ", strerror(errno));

        fclose(fp);
    } else {
        StereoLogE(
            "<readFileToBuffer> read fail, error: %s ", strerror(errno));
    }
}


StereoString Utils::getFileNameFromPath(const StereoString &filePath) {
    std::size_t found = filePath.find_last_of("/");
    int pathLen = filePath.length();
    if (found < 0 || found > pathLen) {
        return filePath;
    }
    StereoString path = filePath.substr(found + 1, pathLen);
    return path;
}

/*************** png encode ***************/
typedef unsigned char ui8;

struct TPngDestructor {
    png_struct *p;
    TPngDestructor(png_struct *p) : p(p)  {}
    ~TPngDestructor() {
        if (p) {
            png_destroy_write_struct(&p, NULL);
        }
    }
};

static void PngWriteCallback(png_structp  png_ptr, png_bytep data, png_size_t length) {
    std::vector<char> *p = (std::vector<char>*)png_get_io_ptr(png_ptr);
    p->insert(p->end(), data, data + length);
}

void Utils::encodePng(const StereoBuffer_t &inputBuffer,
        int width, int height, StereoBuffer_t &outBuffer) {
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        StereoLogE("png_create_write_struct fail");
        return;
    }
    TPngDestructor destroyPng(png_ptr);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        StereoLogE("png_create_info_struct fail");
        return;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        StereoLogE("setjmp(png_jmpbuf(png_ptr) failed");
        return;
    }
    png_set_IHDR(png_ptr, info_ptr, width, height, 8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);
    //png_set_compression_level(p, 1);

    std::vector<char> tmpOut;

    // convert gray to rgba
    int dataRGBALen = inputBuffer.size * PNG_BYTES_PER_PIXEL;
    char *dataRGBA = (char*)malloc(dataRGBALen);

    if (dataRGBA == nullptr) {
        StereoLogE("Allocate memory fail.");
        return;
    }

    memset(dataRGBA, 0xFF, dataRGBALen);

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            dataRGBA[(h * width + w) * PNG_BYTES_PER_PIXEL + PNG_NUM_3]
                    = (inputBuffer.data)[h * width + w];
        }
    }

    // write buffer
    std::vector<ui8*> rows(height);
    for (int y = 0; y < height; ++y) {
        rows[y] = (ui8*)dataRGBA + y * width * 4;
    }
    png_set_rows(png_ptr, info_ptr, &rows[0]);
    png_set_write_fn(png_ptr, &tmpOut, PngWriteCallback, NULL);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    BufferManager::createBuffer(tmpOut.size(), outBuffer);
    memcpy(outBuffer.data, &tmpOut[0], tmpOut.size());

    free(dataRGBA);
}
/*************** png encode ending ***************/


