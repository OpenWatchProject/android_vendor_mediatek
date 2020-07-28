#define LOG_TAG "file_cache"

#include <iostream>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>

#include <utils/std/IFileCache.h>
#include <utils/std/JobQueue.h>
#include <utils/std/Log.h>
#include <utils/Mutex.h>

using namespace std;
using namespace NSCam;
using namespace android;

static unsigned int g_u4LogEn = 0;
static unsigned long long int g_storageSize = 1024*1024*1024;
static unsigned long long int g_totalSize=0;
static Mutex& g_mutex = *new Mutex();

static JobQueue<void()> g_jobQueue("UT");

void writeToFile(char *filename, vector<pair<void *, unsigned int>> *dataList);

//Generalization class
class FileCache : public IFileCache
{
private:
    char *m_filename;
    vector<pair<void *, unsigned int>>* m_memoryCacheList;
    Mutex m_mutex;
public:
    FileCache(const char *filename);
    virtual ~FileCache();
    virtual void onFirstRef() {};
    virtual unsigned int write(const void* data, unsigned int size);
    // write data to IFileCache, this function will continue write, (not write in the file position 0)
    // return value:
    //   how many data is wrote
};

sp<IFileCache>
IFileCache::
open(const char *filename)
{
        if(filename==NULL)
        {
                CAM_LOGE("File name is NULL\n");
                return NULL;
        }
        return new FileCache(filename);
}

FileCache::
FileCache(const char * filename) :
    m_memoryCacheList(NULL)
{
    g_u4LogEn     = property_get_int32("vendor.debug.file_cache.log", 0);
    g_storageSize = property_get_int64("vendor.debug.file_cache.storage_size", 1024*1024*1024);

    CAM_LOGI_IF(g_u4LogEn, "Initialize FileCache <%s>", filename);

    m_filename=(char *)calloc(1, 256);
    if(m_filename!=NULL) {
        strncpy(m_filename, filename, 255);
        m_memoryCacheList = new vector<pair<void *, unsigned int>>();
    } else {
        CAM_LOGE("<%s> calloc fail", filename);
    }
}

FileCache::
~FileCache()
{
    writeToFile(m_filename, m_memoryCacheList);
    CAM_LOGI_IF(g_u4LogEn, "<%s> Dealloc of FileCache", m_filename);
}

unsigned int
FileCache::
write(const void* data, unsigned int size)
{
    Mutex::Autolock _l(m_mutex);

    g_mutex.lock();
    if ((g_totalSize + size) > g_storageSize)
    {
        g_mutex.unlock();
        CAM_LOGE("<%s> Storage size exceed %uMB/%uMB", m_filename, g_totalSize/1024/1024, g_storageSize/1024/1024);
        return -1;
    }
    g_totalSize+=size;
    g_mutex.unlock();

    //allocate the buffer we need
    void *_allocBuf = (void *)malloc(sizeof(char)*size);

    if (_allocBuf == NULL)
    {
        CAM_LOGE("<%s> Fail to allocate memory", m_filename);
        return -1;
    }

    //copy data from source
    memcpy(_allocBuf, data, size);

    CAM_LOGD_IF(g_u4LogEn, "<%s> Write %d bytes to memory", m_filename, size);

    //put data into our list
    m_memoryCacheList->push_back(make_pair(_allocBuf, size));

    return size;
}

void
writeToFile(char *filename, vector<pair<void *, unsigned int>> *dataList)
{
    function<void()>pfunc =
        //bind a lambda function with no input parameters and return values
        bind
        (
            [=]()->void
            {
                FILE * pFile;
                pFile = fopen(filename, "wb");
                if (pFile != NULL)
                {
                    for (int i = 0; i < (int)dataList->size(); i++)
                    {
                        CAM_LOGD_IF(g_u4LogEn,"<%s> write %d bytes to file (%d/%d)", (unsigned char *)dataList->at(i).first, dataList->at(i).second, i, (int)dataList->size());
                        int n = fwrite(dataList->at(i).first, sizeof(char), dataList->at(i).second, pFile);
                        if (n != (int)dataList->at(i).second) {
                            CAM_LOGE("<%s> write error!!!", filename);
                        }
                        free(dataList->at(i).first);
                        g_mutex.lock();
                        g_totalSize -= dataList->at(i).second;
                        g_mutex.unlock();
                    }

                    fclose(pFile);
                } else {
                        CAM_LOGE("Fail to open physical file <%s>", filename);
                }
                dataList->clear();
                delete(dataList);
                free(filename);

                CAM_LOGI_IF(g_u4LogEn, "<%s> write done, (remain = %d KB)", filename, g_totalSize/1024);
            }
        );

    // unsigned int fgLogEnOuter = (g_u4LogEn & EN_FILE_CACHE_LOG_FILE) ? 1 : 0;
    // CAM_LOGD_IF(fgLogEnOuter,"Add Job to JobQueue <%s>", filename);

    g_jobQueue.addJob(pfunc);

    // CAM_LOGD_IF(fgLogEnOuter,"Success to add JobQueue, and leave this function");
}

