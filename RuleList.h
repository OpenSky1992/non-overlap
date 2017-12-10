#ifndef RULELIST_H
#define RULELIST_H

#include "stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include <unordered_map>
#include <set>

class rule_list {
public:
    std::vector<p_rule> list;
    std::unordered_map <uint32_t, std::vector<uint32_t> > dep_map;

    rule_list();
    rule_list(std::string & filename);

    int linear_search(const addr_5tup &);

public:
    void print(const std::string &);
};
#endif
