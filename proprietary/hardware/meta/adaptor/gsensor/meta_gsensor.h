#ifndef __META_GPIO_H__
#define __META_GPIO_H__
#include "meta_gsensor_para.h"
#endif 

typedef void (*GS_CNF_CB)(GS_CNF *cnf);
extern void Meta_GSensor_Register(GS_CNF_CB callback);

