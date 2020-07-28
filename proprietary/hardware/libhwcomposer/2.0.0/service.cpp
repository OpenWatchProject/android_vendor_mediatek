#define DEBUG_LOG_TAG "SERVICE"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "service.h"

#include <binder/IServiceManager.h>

ANDROID_SINGLETON_STATIC_INSTANCE(ServiceManager);

ServiceManager::ServiceManager()
{
}

ServiceManager::~ServiceManager()
{
}

void ServiceManager::init()
{
}
