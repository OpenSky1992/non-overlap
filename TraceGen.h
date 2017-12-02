#ifndef TRACEGEN_H
#define TRACEGEN_H

#include "stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include "RuleList.h"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#include <cassert>
#include <thread>
#include <future>
#include <mutex>
#include <atomic>
#include <chrono>
#include <set>
#include <map>

#include <pcap.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

using std::vector;
using std::string;
using std::atomic_uint;
using std::atomic_bool;
using std::mutex;
using boost::unordered_map;
using boost::unordered_set;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

class tgen_para {
public:
    // basic parameter
    double flow_rate;
    double simuT;

    // locality traffic parameter
    double cold_prob;
    uint32_t hotspot_no;
    uint32_t scope[number_prefix];		    // hotspot probing scope
    uint32_t mut_scalar[2];     // mutate scale parameter
    bool prep_mutate;
    uint32_t hot_rule_thres;	// lower bound for indentify a hot rule
    uint32_t hot_candi_no;	    // number of hot candidate to generate
    
    string trace_root_dir;      // root directory of generated traces	
    string hotcandi_str;	    // hotspot candi file
    string hotspot_ref;         // hotspot reference file
    string flowInfoFile_str;    // first arr time of each flow
    
    // dynamics parameter
    bool evolving;
    double hotvtime;            // arrival interval of hot spot
    double evolving_time;       // ?? 
    size_t evolving_no;         // ??

    // trace source parameter 
    string pcap_dir;		    // original pcap trace direcotry
    string parsed_pcap_dir;	    // directory of parsed pcap file

    // bulk generate
    int bulk_no;
    int flow_rate_step;
    double cold_prob_step;
    int hotspot_no_step;

public:
    tgen_para();
    tgen_para(const tgen_para & another_para);
    tgen_para(string config_file);
};

/*
 * Usage:
 *   tracer tGen(rulelist pointer);
 *   tGen.hotspot_*(reference file)
 *   pFlow_pruning_gen(objective synthetic trace directory)
 */
class tracer {
public:
    tracer();
    tracer(rule_list * rList, string para_file);
    //generate the hotspot buckets by bucketTree
    void hotspot_prepare();
    void pFlow_pruning_gen(bool);

private:
    rule_list * rList;
    EpochT jesusBorn;      // double simuT;

    vector<string> to_proc_files;
    string gen_trace_dir;     // the directory for generating one specific trace

    tgen_para para; // TODO: this will replace all the parameters
    
    boost::log::sources::logger tracer_log;    
    
private:
    void flow_pruneGen_mp(unordered_set<addr_5tup> &) const;
    void flow_pruneGen_mp_ev(unordered_set<addr_5tup> &) const;
    void f_pg_st (string, uint32_t, boost::unordered_map<addr_5tup, std::pair<uint32_t, addr_5tup> > *) const;
    void merge_files(string) const;
    void get_proc_files();
    boost::unordered_set<addr_5tup> flow_arr_mp() const;
    boost::unordered_set<addr_5tup> f_arr_st (string) const;

    void p_pf_st(vector<string>, size_t) const;
    void p_count_st(fs::path, atomic_uint*, mutex *, boost::unordered_map<addr_5tup, uint32_t>*, atomic_bool *);
    void print_setup() const;
};

#endif
