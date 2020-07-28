#ifndef __ROIINFOCONFIG_H__
#define __ROIINFOCONFIG_H__

#include "Base64.h"

#include <utils/Vector.h>
#include <utils/KeyedVector.h>

#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <assert.h>

//#define DEBUG_ROIINFO
#ifdef DEBUG_ROIINFO
#include <android/log.h>
#define LOG_TAG "RoiInfoConfig"
#define ROIINFOCONFIG_ALOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,"[INFOCONFIG]" __VA_ARGS__)
#define ROIINFOCONFIG_IN() //ROIINFOCONFIG_ALOG("[%p] + %s", this, __func__)
#define ROIINFOCONFIG_OUT() //ROIINFOCONFIG_ALOG("[%p] - %s", this, __func__)
#define ROIINFOCONFIG_PROP() //ROIINFOCONFIG_ALOG("[%p] --> %s: %d", this, __func__, __LINE__)
#else
#define ROIINFOCONFIG_ALOG(...)
#define ROIINFOCONFIG_IN()
#define ROIINFOCONFIG_OUT()
#define ROIINFOCONFIG_PROP()
#endif

namespace MtkVenc {

struct RoiInfo;
class MtkRoiConfigEntry;
class Job;
class MtkRoiConfigWorker;
typedef void (*GenMap) (android::Vector<struct RoiInfo*>&, int, int, int, unsigned char*);

struct RoiInfo {
    union {
        unsigned short top;
        unsigned short t;
    };
    union {
        unsigned short left;
        unsigned short l;
    };
    union {
        unsigned short bottom;
        unsigned short b;
    };
    union {
        unsigned short right;
        unsigned short r;
    };
    union {
        unsigned short level;
        unsigned short v;
    };
};

class MtkRoiConfigEntry {
protected:
    unsigned int width,height;
    GenMap func;

    int defaultBase;

    bool keepRaw;
    unsigned short* rawInfo;

    bool convertOnGen;
    unsigned int rawInfoB64len;
    char* rawInfoB64;

    bool infoReady;
    unsigned short size;
    android::Vector<struct RoiInfo*> infoList;

    pthread_mutex_t infoLock;

public:
    MtkRoiConfigEntry(unsigned int w, unsigned int h, GenMap f, bool keep=false)
    : width(w), height(h), func(f)
    , defaultBase(51)
    , keepRaw(keep), rawInfo(NULL)
    , convertOnGen(false), rawInfoB64len(0), rawInfoB64(NULL)
    , infoReady(false), size(0)
    {
        ROIINFOCONFIG_IN();
        pthread_mutex_init(&this->infoLock, 0);
        ROIINFOCONFIG_OUT();
    }
    virtual ~MtkRoiConfigEntry()
    {
        ROIINFOCONFIG_IN();
        if(this->rawInfo != NULL) free(this->rawInfo);
        clearRoiInfo(false);
        pthread_mutex_destroy(&this->infoLock);
        ROIINFOCONFIG_OUT();
    }
    inline void putRoiInfo(unsigned short s, char* _info, bool lock=true)
    {
        if(_info == NULL) return;
        if(this->infoReady) return;

        ROIINFOCONFIG_IN();
        this->size = s;

        if(lock) pthread_mutex_lock(&infoLock);

        this->rawInfoB64len = strlen(_info);
        this->rawInfoB64 = (char*)malloc(sizeof(char)*(this->rawInfoB64len+1));
        assert(this->rawInfoB64 != NULL);
        strncpy(this->rawInfoB64, _info, this->rawInfoB64len);
        this->rawInfoB64[this->rawInfoB64len]=0;

        if(this->rawInfo != NULL) delete this->rawInfo;
        this->rawInfo = (unsigned short*) malloc(sizeof(unsigned short)*s*5);
        assert(this->rawInfo != NULL);

        if(lock) pthread_mutex_unlock(&infoLock);

        ROIINFOCONFIG_ALOG("[DUMP] putRoiInfo %s", _info);

        this->convertOnGen = true;
        this->infoReady = true;
        this->keepRaw = true;
        ROIINFOCONFIG_OUT();
    }
    inline void putRoiInfo(unsigned short s, unsigned short* _info, bool lock=true)
    {
        if(_info==NULL) return;

        ROIINFOCONFIG_IN();
        this->size = s;
        if(this->keepRaw && this->rawInfo == NULL)
        {
            int rawMemSize = sizeof(unsigned short)*s*5;
            this->rawInfo = (unsigned short*)malloc(rawMemSize);
            assert(this->rawInfo != NULL);
            memcpy(this->rawInfo, _info, rawMemSize);
        }

        unsigned short* p=_info;
        for(unsigned short i=0; i<this->size; i++)
        {
            struct RoiInfo* info = (struct RoiInfo*) malloc(sizeof(struct RoiInfo));
            assert(info != NULL);
            info->t = *(p++);
            info->l = *(p++);
            info->b = *(p++);
            info->r = *(p++);
            info->v = *(p++);

            if(lock) pthread_mutex_lock(&infoLock);
            infoList.push(info);
            if(lock) pthread_mutex_unlock(&infoLock);

            ROIINFOCONFIG_ALOG("[DUMP] putRoiInfo {t %u, l %u, b %u, r %u, v %u}",
                info->t, info->l, info->b, info->r, info->v);
        }
        this->convertOnGen = false;
        this->infoReady = true;
        ROIINFOCONFIG_OUT();
    }
    inline void updateRoiSize(unsigned int w, unsigned int h, bool lock=true)
    {
        if(lock) pthread_mutex_lock(&infoLock);

        this->width=w;
        this->height=h;

        if(lock) pthread_mutex_unlock(&infoLock);

        ROIINFOCONFIG_ALOG("[DUMP] updateRoiSize w x h= %d x %d",
                this->width, this->height);
    }
    inline void clearRoiInfo(bool lock=true)
    {
        ROIINFOCONFIG_IN();
        if(lock) pthread_mutex_lock(&infoLock);
        while(infoList.size() > 0)
        {
            struct RoiInfo* info = infoList[0];
            infoList.removeAt(0);
            if(info) free(info);
        }
        this->infoReady = false;
        if(lock) pthread_mutex_unlock(&infoLock);
        ROIINFOCONFIG_ALOG("Entry infolist of %p clear" , this);
        ROIINFOCONFIG_OUT();
    }
    inline void tryDoBase64Decode(bool lock=true)
    {
        ROIINFOCONFIG_ALOG("[%s][DUMP] keepRaw %d rawInfoB64 %p rawInfoB64len %d convertOnGen %d",
            __func__, this->keepRaw, this->rawInfoB64, this->rawInfoB64len, this->convertOnGen);

        if(!this->keepRaw)
        {
            ROIINFOCONFIG_ALOG("[%s] Keep raw info is not raise for base64 decode", __func__);
            return;
        }
        if(this->rawInfoB64 == NULL || this->rawInfoB64len <= 0)
        {
            ROIINFOCONFIG_ALOG("[%s] raw info buffer not available base64 decode", __func__);
            return;
        }
        if(!this->convertOnGen)
        {
            ROIINFOCONFIG_ALOG("[%s] Convert base64 on gen map flag not raise", __func__);
            return;
        }

        ROIINFOCONFIG_IN();
        if(lock) pthread_mutex_lock(&infoLock);
        android::Vector<unsigned char> decodedBytes = base64Decode(this->rawInfoB64len, this->rawInfoB64);
        if(lock) pthread_mutex_unlock(&infoLock);

        int __count = 0;

        if(this->rawInfoB64len > 0 && decodedBytes.size() > 0)
        {
            for(int i=0; i<decodedBytes.size(); i+=2)
            {
                if(i+1 < decodedBytes.size())
                {
                    unsigned char low = decodedBytes[i];
                    unsigned char hig = decodedBytes[i+1];
                    int index = i>>1;
                    this->rawInfo[index] = low | hig << 8;
                    __count ++;
                }
            }
        }

        __count /= 5;

        if(this->size > __count)
        {
            ROIINFOCONFIG_ALOG("[WARNING] roi-size %d > roi-rect len: %d ", this->size, __count);
            this->size = __count;
        }

        this->keepRaw = false;
        this->putRoiInfo(this->size, this->rawInfo, lock);
        ROIINFOCONFIG_OUT();
    }
    void doGenRoiMap(unsigned char* va=NULL, int qp=0)
    {
        ROIINFOCONFIG_IN();

        if(func != NULL) func(infoList, width, height, qp, va);

        ROIINFOCONFIG_OUT();
    }
    // blocking until roi map generated
    inline void getRoiMap(unsigned char* va, int qp, bool lock=true)
    {
        if(va==NULL) return;
        if(this->width == 0 || this->height == 0) return;

        ROIINFOCONFIG_IN();
        if(lock) pthread_mutex_lock(&infoLock);
        bool infoReady = this->infoReady;
        if(lock) pthread_mutex_unlock(&infoLock);

        if(!infoReady)
        {
            ROIINFOCONFIG_ALOG("[%s] Info not ready", __func__);
            return;
        }

        if(this->width == 0 || this->height == 0) {
            ROIINFOCONFIG_ALOG("width x height = 0");
            return;
        }

        if(lock) pthread_mutex_lock(&infoLock);
        tryDoBase64Decode(false);
        doGenRoiMap(va, qp);
        if(lock) pthread_mutex_unlock(&infoLock);

        ROIINFOCONFIG_ALOG("[%s] Get RoiMap: %p, base %d", __func__, va, qp);
        ROIINFOCONFIG_OUT();
    }
};

static void doGenMapFromQpSimple(android::Vector<struct RoiInfo*>& infoList, int width, int height, int qp, unsigned char* va)
{
    ROIINFOCONFIG_ALOG("gen roi map: wxh = %dx%d using va %p, infolist count %d",
            width, height, va, infoList.size());

    //android::Vector<struct RoiInfo*>::iterator it = NULL;
    //for(it = infoList.begin(); it != infoList.end(); it++)
    memset(va, 0, width*height);
    for(int idx=0; idx<infoList.size(); idx++)
    {
        struct RoiInfo* info = infoList[idx];
        ROIINFOCONFIG_ALOG("[DUMP] doGenRoiMap {t %d, l %d, b %d, r %d, v %d}",
            info->t, info->l, info->b, info->r, info->v);

        unsigned short mapValue = 0;
        if(qp > info->v)
        {
            mapValue = qp - info->v;
            mapValue = (mapValue > 51)?51:mapValue;
        }

        // naive method
        for(int i=0; i<height; i++)
        {
            for(int j=0; j<width; j++)
            {
                if(j >= info->l && j <= info->r && i >= info->t && i <= info->b) {
                    if((j==info->l || j==info->r) && (i==info->t || i==info->b)) {
                        ROIINFOCONFIG_ALOG("Write %d (base %d v %d) -> [%d][%d] (%d)",
                            mapValue, qp, info->v, i, j, i*width + j);
                    }

                    va[i*width + j] = (unsigned char)mapValue;
                }
            }
        }
    }
}

static void doGenMapFromQpJump(android::Vector<struct RoiInfo*>& infoList, int width, int height, int qp, unsigned char* va)
{
    memset(va, 0, width*height);
    for(int idx=0; idx<infoList.size(); idx++)
    {
        struct RoiInfo* info = infoList[idx];
        unsigned short infoW = info->r - info->l;
        unsigned short infoH = info->b - info->t;

        if(infoW == 0 || infoH == 0 || info->r >= width || info->b >= height)
        {
            continue;
        }

        unsigned short mapValue = 0;
        if(qp > info->v)
        {
            mapValue = qp - info->v;
            mapValue = (mapValue > 15)?15:mapValue;
        }

        unsigned char* rowStart = va + (width*info->t) + info->l;
        for(int i=0; i<infoH; i++)
        {
            for(int j=0; j<infoW; j++)
            {
                *(rowStart+j) = (char)mapValue;
            }
            rowStart += width;
        }

        ROIINFOCONFIG_ALOG("[%s][DUMP] t %d l %d w %d h %d mapValue %d (base %d v %d)",
            __func__, info->t, info->l, infoW, infoH, mapValue, qp, info->v);
    }
}

static void doGenMapFromQualJump(android::Vector<struct RoiInfo*>& infoList, int width, int height, int qp, unsigned char* va)
{
    memset(va, 0, width*height);
    for(int idx=0; idx<infoList.size(); idx++)
    {
        struct RoiInfo* info = infoList[idx];
        unsigned short infoW = info->r - info->l;
        unsigned short infoH = info->b - info->t;

        if(infoW == 0 || infoH == 0 || info->r >= width || info->b >= height)
        {
            continue;
        }

        unsigned short mapValue = 0;
        if(info->v > qp) mapValue = qp;
        else mapValue = info->v;

        unsigned char* rowStart = va + (width*info->t) + info->l;
        for(int i=0; i<infoH; i++)
        {
            for(int j=0; j<infoW; j++)
            {
                *(rowStart+j) = (char)mapValue;
            }
            rowStart += width;
        }

        ROIINFOCONFIG_ALOG("[%s][DUMP] t %d l %d w %d h %d mapValue %d (base %d v %d)",
            __func__, info->t, info->l, infoW, infoH, mapValue, qp, info->v);
    }
}

template <class KEY, class TYPE>
class MtkRoiConfig {
private:
    android::DefaultKeyedVector<KEY, TYPE*> EntryTable;
    android::DefaultKeyedVector<TYPE*, int> EntryCount;

    pthread_mutex_t entryLock;

    TYPE* workingEntry;

    unsigned int width, height;
    GenMap genMapFunc;

public:
    MtkRoiConfig(GenMap f)
    : EntryTable(NULL), EntryCount(0), workingEntry(NULL)
    , width(0), height(0), genMapFunc(f)
    {
        ROIINFOCONFIG_IN();
        pthread_mutex_init(&this->entryLock, 0);
        (void)static_cast<MtkRoiConfigEntry*>((TYPE*)0);
        ROIINFOCONFIG_OUT();
    }
    ~MtkRoiConfig()
    {
        ROIINFOCONFIG_IN();
        while(EntryTable.size() > 0)
        {
            TYPE* entry = EntryTable[0];
            if(entry != NULL) delete entry;
            EntryTable.removeItemsAt(0);
        }
        pthread_mutex_destroy(&this->entryLock);
        ROIINFOCONFIG_OUT();
    }
    inline int entryInsert(KEY k, TYPE* e, bool lock=true)
    {
        if(e == NULL) return -1;

        int refCount = -1;

        if(lock) pthread_mutex_lock(&this->entryLock);

        if(EntryTable.indexOfKey(k) < 0)
        {
            EntryTable.add(k, e);
            refCount = entryRefAdd(e, false);

            ROIINFOCONFIG_ALOG("[%s] Put entry-buffer pair %p-%p, ref count %d", __func__, k, e, refCount);
        }
        else
        {
            ROIINFOCONFIG_ALOG("[WARNING] Put entry-buffer pair %p - %p repeated", k, e);
        }

        if(lock) pthread_mutex_unlock(&this->entryLock);

        return refCount;
    }
    inline int entryRemove(KEY k, bool lock=true)
    {
        int refCount = -1;

        if(lock) pthread_mutex_lock(&this->entryLock);

        if(EntryTable.indexOfKey(k) >= 0)
        {
            TYPE* entry = EntryTable.valueFor(k);

            EntryTable.removeItem(k);
            refCount = entryRefDec(entry, false);

            ROIINFOCONFIG_ALOG("[%s] Removed entry-buffer pair %p-%p", __func__, k, entry);
        }

        if(lock) pthread_mutex_unlock(&this->entryLock);

        return refCount;
    }
    inline int entryRefAdd(TYPE* e, bool lock=true)
    {
        if(e == NULL) return -1;

        int refCount = 0;

        if(lock) pthread_mutex_lock(&this->entryLock);
        if(EntryCount.indexOfKey(e) >= 0)
        {
            refCount = EntryCount.valueFor(e);
            refCount++;
            EntryCount.replaceValueFor(e, refCount);
        }
        else
        {
            refCount = 1;
            EntryCount.add(e, refCount);
        }
        if(lock) pthread_mutex_unlock(&this->entryLock);

        ROIINFOCONFIG_ALOG("[%s] ref of %p = %d", __func__, e, refCount);

        return refCount;
    }
    inline int entryRefDec(TYPE* e, bool lock=true)
    {
        if(e == NULL) return -1;

        int refCount = 0;

        if(lock) pthread_mutex_lock(&this->entryLock);

        refCount = EntryCount.valueFor(e);
        refCount--;

        if(refCount == 0)
        {
            EntryCount.removeItem(e);
            delete e;
        }
        else
        {
            EntryCount.add(e, refCount);
        }

        if(lock) pthread_mutex_unlock(&this->entryLock);

        ROIINFOCONFIG_ALOG("[%s] ref of %p = %d", __func__, e, refCount);

        return refCount;
    }
    // 0. set w,h
    void adjustSize(unsigned int w, unsigned int h)
    {
        this->width = w;
        this->height = h;

        if(workingEntry != NULL) workingEntry->updateRoiSize(this->width, this->height);
    }
    // 1. reset session
    void resetInfoSession()
    {
        ROIINFOCONFIG_IN();
        pthread_mutex_lock(&this->entryLock);

        if(workingEntry != NULL) {
            entryRefDec(workingEntry, false);
        }

        workingEntry = new TYPE(this->width, this->height, genMapFunc);
        entryRefAdd(workingEntry, false);

        pthread_mutex_unlock(&this->entryLock);
        ROIINFOCONFIG_OUT();
    }
    // 2. give Info
    void giveRoiInfo(unsigned short s, unsigned short* _info)
    {
        ROIINFOCONFIG_IN();

        assert(workingEntry!=NULL);

        pthread_mutex_lock(&this->entryLock);
        workingEntry->putRoiInfo(s, _info);
        pthread_mutex_unlock(&this->entryLock);

        ROIINFOCONFIG_ALOG("[DUMP] giveRoiInfo %p {%d,..", _info, *_info);

        ROIINFOCONFIG_OUT();
    }
    // 2. give Info
    void giveRoiInfo(unsigned int s, char* _info)
    {
        ROIINFOCONFIG_IN();

        assert(workingEntry!=NULL);

        pthread_mutex_lock(&this->entryLock);
        workingEntry->putRoiInfo(s, _info);
        pthread_mutex_unlock(&this->entryLock);

        ROIINFOCONFIG_ALOG("[DUMP] giveRoiInfo str %s", _info);

        ROIINFOCONFIG_OUT();
    }
    // 3. give key (buffer hdr ptr): repeat call this if reuse previous info
    int giveBuffer(KEY buffer)
    {
        ROIINFOCONFIG_IN();

        int refCount = entryInsert(buffer, workingEntry);

        ROIINFOCONFIG_OUT();
        return refCount;
    }

    // 4. get va by buffer
    void getBufferRoiMap(KEY buffer, unsigned char* va, int qp)
    {
        ROIINFOCONFIG_IN();
        pthread_mutex_lock(&this->entryLock);

        TYPE* entry = EntryTable.valueFor(buffer);

        pthread_mutex_unlock(&this->entryLock);

        if(entry == NULL) {
            ROIINFOCONFIG_OUT();
            return;
        }

        ROIINFOCONFIG_ALOG("[%s] Get entry %p from key buffer %p", __func__, entry, buffer);

        entry->updateRoiSize(this->width, this->height);
        entry->getRoiMap(va, qp);

        ROIINFOCONFIG_OUT();
    }

    // 5. remove key (buffer hdr ptr): reduce ref count until 0, delete entry
    int ungiveBuffer(KEY buffer)
    {
        ROIINFOCONFIG_IN();

        int refCount = entryRemove(buffer);

        ROIINFOCONFIG_OUT();
        return refCount;
    }
};

}; // NS MtkVenc

#endif //__ROIINFOCONFIG_H__