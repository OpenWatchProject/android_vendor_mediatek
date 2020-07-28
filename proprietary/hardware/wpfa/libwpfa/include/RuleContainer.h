#ifndef RULECONTAINER_H
#define RULECONTAINER_H

#include <iostream>
#include <string>
#include <map>
#include "FilterStructure.h"

using namespace std;

#define RC_LOG_TAG "WpfaRuleContainer"


class RuleContainer {
public:
    RuleContainer();
    virtual ~RuleContainer();

    int addFilter(int fid, WPFA_filter_reg_t filter);
    int removeFilter(int fid);
    WPFA_filter_reg_t* getFilterFromMap(int fid);
    int getFilterMapSize();
    void dumpAllFilterId();
    int findFilterById(int fid);


protected:
    void copyFilter(int fid, WPFA_filter_reg_t filter);

private:
    //declaration container and iterator
    map<int, WPFA_filter_reg_t> filterMap;
    map<int, WPFA_filter_reg_t>::iterator iter;
    map<int, WPFA_filter_reg_t>::reverse_iterator iter_r;
};

#endif /* end of RULECONTAINER_H */

