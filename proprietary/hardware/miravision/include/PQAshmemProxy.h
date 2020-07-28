#ifndef __PQ_ASHMEMPROXY_H__
#define __PQ_ASHMEMPROXY_H__

#include <string.h>
#include <log/log.h>
#include <android/hidl/memory/1.0/IMemory.h>

#define MODULE_MAX_OFFSET (512)
#define MODULE_CUST_MAX_OFFSET (4096)

enum ProxyTuning {
    PROXY_TUNING_FLAG = 0,
    PROXY_SYNC_FLAG,
    PROXY_SYNC_OK_FLAG,
    PROXY_PREPARE_FLAG,
    PROXY_READY_FLAG,
    PROXY_FLAG_MAX
};

enum ProxyTuningBuffer {
    PROXY_DEBUG = 0,
    PROXY_DS_INPUT,
    PROXY_DS_OUTPUT,
    PROXY_DS_SWREG,
    PROXY_DC_INPUT,
    PROXY_DC_OUTPUT,
    PROXY_DC_SWREG,
    PROXY_HFG_INPUT,
    PROXY_HFG_OUTPUT,
    PROXY_HFG_SWREG,
    PROXY_RSZ_INPUT,
    PROXY_RSZ_OUTPUT,
    PROXY_RSZ_SWREG,
    PROXY_HDR_INPUT,
    PROXY_HDR_OUTPUT,
    PROXY_HDR_SWREG,
    PROXY_COLOR_INPUT,
    PROXY_COLOR_OUTPUT,
    PROXY_COLOR_SWREG,
    PROXY_CCORR_INPUT,
    PROXY_CCORR_OUTPUT,
    PROXY_CCORR_SWREG,
    PROXY_DRE_INPUT,
    PROXY_DRE_OUTPUT,
    PROXY_DRE_SWREG,
    PROXY_TDSHP_REG,
    PROXY_ULTRARESOLUTION,
    PROXY_DYNAMIC_CONTRAST,
    PROXY_MAX
};

enum ProxyCustBuffer {
    PROXY_RSZ_CUST = 0,
    PROXY_DS_CUST,
    PROXY_DC_CUST,
    PROXY_HDR_CUST,
    PROXY_DRE_CUST,
    PROXY_TDSHP_CUST,
    PROXY_COLOR_CUST,
    PROXY_CUST_MAX
};

enum ProxyRSZBuffer {
    PROXY_RSZ_CUST_SWREG = 0,
    PROXY_RSZ_CUST_HWREG,
    PROXY_RSZ_CUST_CZSWREG,
    PROXY_RSZ_CUST_CZHWREG,
    PROXY_RSZ_CUST_CZLEVEL,
    PROXY_RSZ_CUST_MAX
};

enum ProxyDSBuffer {
    PROXY_DS_CUST_REG = 0,
    PROXY_DS_CUST_CZSWREG,
    PROXY_DS_CUST_CZHWREG,
    PROXY_DS_CUST_CZLEVEL,
    PROXY_DS_CUST_MAX
};

enum ProxyDCBuffer {
    PROXY_DC_CUST_ADLREG = 0,
    PROXY_DC_CUST_HDRREG,
    PROXY_DC_CUST_MAX
};

enum ProxyHDRBuffer {
    PROXY_HDR_CUST_REG = 0,
    PROXY_HDR_CUST_PANELSPEC,
    PROXY_HDR_PQ_ID,
    PROXY_HDR_OUTPUT_BUFFER,
    PROXY_HDR_CUST_MAX
};

enum ProxyDREBuffer {
    PROXY_DRE_CUST_REG = 0,
    PROXY_DRE_OUTPUT_BUFFER,
    PROXY_DRE_CUST_MAX
};

enum ProxyTuningMode {
    PROXY_TUNING_NORMAL,
    PROXY_TUNING_READING,
    PROXY_TUNING_OVERWRITTEN,
    PROXY_TUNING_BYPASSHWACCESS,
    PROXY_TUNING_END
};

using namespace android;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::sp;

class PQAshmemProxy
{
public:
    PQAshmemProxy();
    ~PQAshmemProxy();

    static unsigned int getAshmemSize(void) { return s_ashmem_size; }
    static unsigned int getModuleSize(void) { return s_module_size; }
    static int getModuleMaxOffset(void) { return s_module_max_offset; }
    static int getAshmemMaxOffset(void) { return s_ashmem_max_offset; }

    void initMapMemory(sp<IMemory> mapMemory);
    int32_t setTuningField(int32_t offset, int32_t value);
    int32_t getTuningField(int32_t offset, int32_t *value);
    template <typename T>
    int32_t setTuningArray(ProxyCustBuffer proxyNum, int32_t offset, T* value, int32_t size)
    {
        if (m_ashmem_base == NULL || m_ashmem_base == (unsigned int*)-1) {
            ALOGD("[PQ_Proxy] PQAshmemProxy : Ashmem is not ready\n");
            return -1;
        }

        if (offset < 0 || offset >= MODULE_CUST_MAX_OFFSET) {
            ALOGD("[PQ_Proxy] PQAshmemProxy : offset (0x%08x) is invalid\n", (int)(offset*sizeof(unsigned int)));
            return -1;
        }

        if (size < 0 || (offset + size) >= MODULE_CUST_MAX_OFFSET){
            ALOGD("[PQ_Proxy] PQAshmemProxy : size (0x%08x) is invalid\n", (int)(size*sizeof(unsigned int)));
            return -1;
        }

        offset += (s_ashmem_max_offset + proxyNum * MODULE_CUST_MAX_OFFSET);
        memcpy(m_ashmem_base + offset, value, (int)(size * sizeof(unsigned int)));
        ALOGD("[PQ_Proxy] PQAshmemProxy : set array proxyNum:%d offset:0x%08x size:%d\n",
            proxyNum, (int)(offset*sizeof(unsigned int)), size);

        return 0;
    }
    bool setPQValueToAshmem(ProxyTuningBuffer proxyNum, int32_t field, int32_t value, int32_t scenario);
    bool getPQValueFromAshmem(ProxyTuningBuffer proxyNum, int32_t field, int32_t *value);

private:
    bool isValid(int32_t offset);
    bool isOverwritten(int32_t offset);
    bool isReading(int32_t offset);
    int32_t pushToMDP(int32_t offset);
    int32_t pullFromMDP(int32_t offset, int32_t scenario);

private:
    static const int s_module_max_offset = MODULE_MAX_OFFSET;
    static const int s_ashmem_max_offset = PROXY_MAX * s_module_max_offset;
    static const int s_ashmem_max_offset_total = s_ashmem_max_offset + PROXY_CUST_MAX * MODULE_CUST_MAX_OFFSET;
    static const unsigned int s_module_size = s_module_max_offset * sizeof(unsigned int);
    static const unsigned int s_ashmem_size = s_ashmem_max_offset_total * sizeof(unsigned int);

    const int m_tunning_magic_num = s_module_max_offset - (PROXY_TUNING_FLAG+1);
    const int m_sync_magic_num = s_module_max_offset - (PROXY_SYNC_FLAG+1);
    const int m_sync_ok_magic_num = s_module_max_offset - (PROXY_SYNC_OK_FLAG+1);
    const int m_prepare_magic_num = s_module_max_offset - (PROXY_PREPARE_FLAG+1);
    const int m_ready_magic_num = s_module_max_offset - (PROXY_READY_FLAG+1);

    bool m_is_ashmem_init;
    unsigned int *m_ashmem_base;
    sp<IMemory> m_mapMemory;
};
#endif
