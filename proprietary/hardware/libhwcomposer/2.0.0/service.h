#ifndef HWC_SERVICE_H_
#define HWC_SERVICE_H_

#include <utils/Singleton.h>

using namespace android;

class ServiceManager : public Singleton<ServiceManager>
{
public:
    ServiceManager();
    ~ServiceManager();

    void init();
};

#endif
