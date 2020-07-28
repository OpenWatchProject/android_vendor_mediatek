#include <vector>
#include <utils/RefBase.h>
#include <cutils/properties.h>

namespace NSCam
{
    //Interface class
    class IFileCache : virtual public android::RefBase
    {
    public:
        virtual ~IFileCache() {};

        //Implmentation of static function
        static android::sp<IFileCache> open(const char *filename);
        // filename not includes path, path will generate by FileCache module

        virtual unsigned int write(const void* data, unsigned int size) = 0;
        // write data to IFileCache, this function will continue write, (not write in the file position 0)
        // return value:
        //   how many data is wrote
    };
}
