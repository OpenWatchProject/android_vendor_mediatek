#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1
#include <cutils/properties.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <PQConfig.h>
#include "DpDriver.h"

#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>

using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;

PQConfig* PQConfig::s_pInstance = NULL;
PQMutex   PQConfig::s_ALMutex;

PQConfig* PQConfig::getInstance()
{
    AutoMutex lock(s_ALMutex);

    if(NULL == s_pInstance)
    {
        s_pInstance = new PQConfig();
        atexit(PQConfig::destroyInstance);
    }

    return s_pInstance;
}

void PQConfig::destroyInstance()
{
    AutoMutex lock(s_ALMutex);

    if (NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

PQConfig::PQConfig()
    :m_PQServiceStatus(PQSERVICE_NOT_READY)
{
    PQ_LOGD("[PQConfig] PQConfig()... ");

    int32_t value = DpDriver::getInstance()->getPQSupport();
    if (value == 0) // PQ_OFF
    {
        m_PQServiceStatus = PQSERVICE_NOT_ENABLE;
        PQ_LOGD("[PQConfig] m_PQSupport = %d ", value);
    }

    checkBootMode();
    updatePQServiceStatus();
};


PQConfig::~PQConfig()
{
    PQ_LOGD("[PQConfig] ~PQConfig()... ");
};


void PQConfig::checkBootMode(void)
{
    int32_t fd;
    ssize_t s;
    int32_t BootMode;
    char c_boot_mode[4] = {'0'};

    //MT_NORMAL_BOOT 0 , MT_META_BOOT 1, MT_RECOVERY_BOOT 2, MT_SW_REBOOT 3
    //MT_FACTORY_BOOT 4, MT_ADVMETA_BOOT 5, ALARM_BOOT 7

    fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDONLY);
    if (fd < 0)
    {
        PQ_LOGE("[PQConfig] fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_mode");
        return;
    }

    s = read(fd, (void *)&c_boot_mode, sizeof(c_boot_mode) - 1);
    close(fd);

    if(s <= 0)
    {
        PQ_LOGE("[PQConfig] could not read BootMode sys file\n");
        return;
    }

    c_boot_mode[s] = '\0';
    BootMode = atoi(c_boot_mode);

    if (BootMode != NORMAL_BOOT && BootMode != SW_REBOOT && BootMode != ALARM_BOOT)
    {
        m_PQServiceStatus = BOOT_MODE_UNSUPPORT_PQSERVICE;
        PQ_LOGD("[PQConfig] Bypass MDP PQ by Bootmode %d", BootMode);
    }
}

PQServiceStatus PQConfig::getPQServiceStatus(void)
{
    return updatePQServiceStatus();
}

PQServiceStatus PQConfig::updatePQServiceStatus(void)
{
    if (m_PQServiceStatus == PQSERVICE_NOT_READY)
    {
        sp<IPictureQuality> service = IPictureQuality::tryGetService();
        if (service == nullptr) {
            PQ_LOGI("[PQConfig] Can not get PQService\n");
        }
        else
        {
            PQAshmem* pPQAshmem = PQAshmem::getInstance();
            bool ret = pPQAshmem->initAshmem(service);
            if (ret == false)
            {
                PQ_LOGD("[PQConfig] Get HIDL memory failed.");
            }
            else
            {
                m_PQServiceStatus = PQSERVICE_READY;
                PQ_LOGD("[PQConfig] PQService is ready\n");
            }
        }
    }
    return m_PQServiceStatus;
}

int32_t PQConfig::getAshmemContext(uint32_t offset)
{
    PQAshmem* pPQAshmem = PQAshmem::getInstance();
    int32_t ashmemContext = 0;
    int32_t ret = 0;

    ret = pPQAshmem->getAshmemValueByOffset(offset, &ashmemContext);
    if (ret == 0)
    {
        return ashmemContext;
    }
    else
    {
        PQ_LOGE("[PQConfig] getAshmemContext failed\n");
        return 0;
    }
}

