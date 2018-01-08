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

    double simuT;
    uint32_t TCAMcap;
    std::string traceFile;
    std::string statFile;

public:
    OFswitch(string trace,string statistics);

    void CEMtest_rt_TCAM();
    void CABtest_rt_TCAM();
    void CNORtest_rt_TCAM();  //caching non-overlap rule
};

#endif
