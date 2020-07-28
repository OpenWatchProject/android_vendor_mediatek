#include "Nvram.h"
#include <android-base/logging.h>
#include <log/log.h>
#include "libnvram.h"
#include "libnvram_log.h"
using std::string;


#define NVRAM_LOG(...) \
        do { \
            ALOGD(__VA_ARGS__); \
        } while (0)


namespace vendor {
namespace mediatek {
namespace hardware {
namespace nvram {
namespace V1_0 {
namespace implementation {
    void covertVector2Array(std::vector<uint8_t> in, char* out) {
        int size = in.size();
        for(int i = 0; i < size; i++) {
            out[i] = in.at(i);
        }
    }

    void covertArray2Vector(const char* in, int len, std::vector<uint8_t>& out) {
        out.clear();
        for(int i = 0; i < len; i++) {
            out.push_back(in[i]);
        }
    }

// Methods from ::vendor::mediatek::hardware::nvram::V1_0::INvram follow.
Return<void> Nvram::readFileByName(const hidl_string& filename, uint32_t size, readFileByName_cb _hidl_cb) {
    // TODO implement
    int pRecSize=0,pRecNum=0;
    bool IsRead=1;
    char *buff=NULL;
    int file_lid = -1;
    string result;
    int i = 0;
    char *nvramstr = (char*)malloc(2*size+1);
    char *nvramptr = nvramstr;
    char *cstr=new char[filename.size()+1];
    if (nvramstr==NULL || size==0 || cstr==NULL) {
    NVRAM_LOG("nvramstr==NULL\n");
        if(nvramstr!=NULL)
            free(nvramstr);
        if(cstr!=NULL)
            delete[] cstr;
    return Void();
    }

    snprintf(cstr, filename.size()+1,"%s", filename.c_str());

    file_lid = NVM_GetLIDByName(cstr);
    if(file_lid < 0)
    {
        NVRAM_LOG("Get LID by name fail! %s\n",cstr);
        free(nvramstr);
        delete[] cstr;
        return Void();
    }

    F_ID fd=NVM_GetFileDesc(file_lid,&pRecSize,&pRecNum,IsRead);
    if (fd.iFileDesc==-1)
    {
        LOG(ERROR) << "open file Error!";
        free(nvramstr);
        delete[] cstr;
        return Void();
    }
    LOG(ERROR) << "RecNum is "<<pRecNum;
    //size=pRecSize*pRecNum;
    buff=(char *)malloc(size);
    if(buff == NULL)
    {
        NVRAM_LOG("Malloc Error!\n");
        if(!NVM_CloseFileDesc(fd))
            NVRAM_LOG("close File error!\n");
        free(nvramstr);
        delete[] cstr;
        return Void();
    }
    if((ssize_t)size == read(fd.iFileDesc,buff,(ssize_t)size))
    {
        if(NVM_CloseFileDesc(fd))
        {
            NVRAM_LOG("Read Done!Size is %d\n",size);
            //return buff;
        }
        else
        {
            NVRAM_LOG("Close file error!\n");
            free(buff);
            free(nvramstr);
            delete[] cstr;
            return Void();
        }
    }
    else
    {
        NVRAM_LOG("read File error!\n");
        if(!NVM_CloseFileDesc(fd))
            NVRAM_LOG("close File error!\n");
        free(buff);
        free(nvramstr);
        delete[] cstr;
        return Void();
    }

    NVRAM_LOG("nvramstr buff[0]%x, buff[1]%x, buff[2]%x, buff[3]%x, buff[4]%x, buff[5]%x, buff[6]%x, buff[7]%x, buff[8]%x \n",
        buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],buff[8]);


    for(i=0; i<(int)size; i++)
    {
       nvramptr += sprintf(nvramptr, "%02X",buff[i]);
    }
    sprintf(nvramptr,"\n");
    *(nvramptr+1)='\0';


    NVRAM_LOG("nvramstr %s\n",nvramstr);
    _hidl_cb(nvramstr);
    free(buff);
    delete[] cstr;
    return Void();
}

Return<int8_t> Nvram::writeFileByNamevec(const hidl_string& filename, uint32_t size, const hidl_vec<uint8_t>& data) {
    // TODO implement
    char *cstr_filename=new char[filename.size()+1];
    char *cstr_data=new char[data.size()+1];
    if (cstr_data==NULL || size==0 || cstr_filename==NULL) {
    NVRAM_LOG("cstr_data==NULL\n");
        if(cstr_data!=NULL)
            delete[] cstr_data;
        if(cstr_filename!=NULL)
            delete[] cstr_filename;
    return int8_t {};
    }
    snprintf(cstr_filename, filename.size()+1,"%s", filename.c_str());
    covertVector2Array(data, cstr_data);

    int pRecSize=0,pRecNum=0,looptimes=0;
    bool IsRead=0;
    int file_lid = -1;

    file_lid = NVM_GetLIDByName(cstr_filename);
    if(file_lid < 0)
    {
        NVRAM_LOG("Get LID by name fail!\n");
        delete[] cstr_data;
        delete[] cstr_filename;
        return int8_t {};
    }


    F_ID fd=NVM_GetFileDesc(file_lid,&pRecSize,&pRecNum,IsRead);
    if (fd.iFileDesc==-1)
    {
        NVRAM_LOG("open file Error!\n");
        delete[] cstr_data;
        delete[] cstr_filename;
        return int8_t {};
    }
    #if 0
    if(size != pRecSize)
    {
        NVRAM_LOG("Input size (%d) and RecSize (%d) not match!\n",size,pRecSize);
        if(!NVM_CloseFileDesc(fd))
            NVRAM_LOG("close File error!\n");
        //return 0;
        return int8_t {};
    }
    #endif

    // GetFileDesc should return right pos and this would cause pro_info multi lids issue.
    #if 0
    if(0 != lseek(fd.iFileDesc,0,SEEK_SET)){
        NVRAM_LOG("lseek error!\n");
        if(!NVM_CloseFileDesc(fd))
            NVRAM_LOG("close File error!\n");
        return 0;
        }
    #endif
    looptimes = pRecNum;
    NVRAM_LOG("RecNum is :%d\n",pRecNum);
    while(looptimes--)
    {
        if((ssize_t)size != write(fd.iFileDesc,cstr_data,(ssize_t)size))
        {
            NVRAM_LOG("write file error!\n");
            if(!NVM_CloseFileDesc(fd))
                NVRAM_LOG("close File error!\n");
            delete[] cstr_data;
            delete[] cstr_filename;
            return int8_t {};
        }
    }
    if(NVM_CloseFileDesc(fd))
    {
        NVRAM_LOG("Write file Done!\n");
        delete[] cstr_data;
        delete[] cstr_filename;
        return int8_t {};
    }
    else
    {
        NVRAM_LOG("close File error!\n");
        delete[] cstr_data;
        delete[] cstr_filename;
        return int8_t {};
    }
    //return int8_t {};
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

INvram* HIDL_FETCH_INvram(const char* /* name */) {
    return new Nvram();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace nvram
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
