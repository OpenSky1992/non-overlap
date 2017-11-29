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
    std::vector<bucket*> sonList; 		// List of son nodes
    std::vector<uint32_t> related_rules;	// IDs of related rules in the bucket
    uint32_t cutArr[2];			// how does this node is cut.  e.g. [2,3] means 2 cuts on dim x, 3 cuts on dim y
    bool hit;
    bucket * parent;
    size_t max_gain;
    size_t repart_level;

  public:
    bucket();
    bucket(const bucket &);
    bucket(const string &, const rule_list *);
    std::pair<double, size_t> split(const std::vector<size_t> &, rule_list *);

    void cleanson();
    string get_str();
};

#endif

