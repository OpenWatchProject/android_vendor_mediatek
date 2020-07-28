#ifndef __META_GYROSCOPE_H__
#define __META_GYROSCOPE_H__
#include "meta_gyroscope_para.h"


typedef void (*GYRO_CNF_CB)(GYRO_CNF *cnf);
extern void Meta_Gyroscope_Register(GYRO_CNF_CB callback);

#endif 

