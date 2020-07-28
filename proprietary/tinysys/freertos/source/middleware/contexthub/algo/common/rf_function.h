#ifndef __RF_FUNCTION_H
#define __RF_FUNCTION_H

#include "feature_struct.h"
#include "math_method.h"

#ifndef uint8_t
#include <stdint.h>
#endif

typedef struct {
    uint16_t dn_num;
    uint16_t ln_num;
    uint8_t max_depth;
    uint8_t tree_size;
    uint8_t class_num;
    uint16_t feature_num;
} RandomForestSetting;

typedef struct {
    uint8_t *leaf_node_class_distribution;
    uint16_t *decision_node_feature;
    int32_t *decision_node_theshold;
} Tree;

typedef struct {
    unsigned *decimal_indicator;
    uint16_t *feature;
    int32_t *theshold;
    uint8_t *distribution;
} Tree_None_Perfect;

void predict(Feature_t *input, Tree *tree, RandomForestSetting *setting, int32_t *prob);
void predict_non(Feature_t *input, Tree_None_Perfect *tree, RandomForestSetting *setting, int32_t *prob);
void normalize(uint8_t *distr, int32_t *freq, int32_t class_num);

#endif /*__RF_FUNCTION_H*/