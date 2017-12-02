#ifndef BUCKET
#define BUCKET

#include "stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include "RuleList.h"
#include <set>

class bucket: public p_rule {
  private:
    static boost::log::sources::logger lg;
    static void logger_init();

  public:
    std::vector<bucket*> sonList; 		    // List of son nodes
    std::vector<uint32_t> related_rules;	// IDs of related rules in the bucket
    uint32_t cutArr[number_prefix];			// how does this node is cut.  e.g. [2,3] means 2 cuts on dim x, 3 cuts on dim y
    bucket * parent;

  public:
    bucket();
    bucket(const bucket &);
    bucket(const string &, const rule_list *);
    /*
    dim: indicate the dimension of cutting
    rList: rule list
    return (average,max) number of related_rules correspanding son node
    */
    std::pair<double, size_t> split(const std::vector<size_t> &dim, rule_list *rList);

    void cleanson();
    string get_str();
};

#endif

