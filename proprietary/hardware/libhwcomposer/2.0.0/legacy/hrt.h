#ifndef HWC_HRT_H
#define HWC_HRT_H

#include <sstream>
#include <linux/disp_session.h>

#include "hrt_common.h"
#include "display.h"

struct HrtLayerConfig;

class Hrt : public HrtCommon
{
public:
    Hrt()
    {
        memset(m_layer_config_list, 0, sizeof(m_layer_config_list));
        memset(m_layer_config_len, 0, sizeof(m_layer_config_len));
        memset(&m_disp_layer, 0, sizeof(m_disp_layer));
        memset(m_hrt_config_list, 0, sizeof(m_hrt_config_list));
        memset(m_hrt_config_len, 0, sizeof(m_hrt_config_len));
    }
    ~Hrt()
    {
        for (int i = 0; i < DisplayManager::MAX_DISPLAYS; ++i)
        {
            if (m_layer_config_list[i])
                free(m_layer_config_list[i]);
        }
    }

    bool isEnabled() const;

    bool isRPOEnabled() const;

    void printQueryValidLayerResult();

    void fillLayerConfigList(const std::vector<sp<HWCDisplay> >& displays);

    void fillDispLayer(const std::vector<sp<HWCDisplay> >& displays);

    void fillLayerInfoOfDispatcherJob(const std::vector<sp<HWCDisplay> >& displays);

    bool queryValidLayer();
private:
    layer_config* m_layer_config_list[DisplayManager::MAX_DISPLAYS];
    int m_layer_config_len[DisplayManager::MAX_DISPLAYS];

    disp_layer_info m_disp_layer;

    HrtLayerConfig* m_hrt_config_list[DisplayManager::MAX_DISPLAYS];
    int m_hrt_config_len[DisplayManager::MAX_DISPLAYS];
};

#endif
