#ifndef OF_SWITCH_CB_H
#define OF_SWITCH_CB_H

#include "stdafx.h"
#include "Address.hpp"
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



class OFswitch_CB
{
public:
    rule_list * rList;
    bucket_tree * bTree;
    separate * sep;

public:
    uint32_t mode; // mode 0: flow information collection: 
                   // mode 1: CEM, mode 2: CAB, mode 3: CNOR
    double simuT;
    uint32_t TCAMcap;
    std::string traceFile;
    std::string statFile;

public:
    OFswitch_CB(string trace,string statistics);

    void run_test();

private:

    //convert the classbench trace to addr_5tup
    addr_5tup formatConvert(string str);


    void flowInfomation();
    void CABtest_rt_TCAM();
    void CEMtest_rt_TCAM();
    void CNORtest_rt_TCAM();  //caching non-overlap rule
};

#endif
