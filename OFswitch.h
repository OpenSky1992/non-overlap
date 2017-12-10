#ifndef OF_SWITCH_H
#define OF_SWITCH_H

#include "stdafx.h"
#include "Rule.hpp"
#include "lru_cache.hpp"
#include "RuleList.h"
#include "BucketTree.h"
#include "Separate.h"
#include <unordered_map>
#include <boost/unordered_set.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <set>

class OFswitch
{
public:
    rule_list * rList;
    bucket_tree * bTree;
    separate * sep;

public:
    uint32_t mode; // mode 0: CAB, mode 1: CEM, mode 2: CDR, mode 3: CNOR
    double simuT;
    uint32_t TCAMcap;
    std::string tracefile_str;

public:
    OFswitch(double ,uint32_t,string);

    void set_para(rule_list *, bucket_tree *,separate *);
    void run_test();

private:
    void CABtest_rt_TCAM();
    void CEMtest_rt_TCAM();
    void CNORtest_rt_TCAM();  //caching non-overlap rule
};

#endif
