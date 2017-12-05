#ifndef SEPARATE_H
#define SEPARATE_H

#include "stdafx.h"
#include "Rule.hpp"
#include "RuleList.h"
#include <unordered_map>


class separate
{
public:
	separate(rule_list &rList);
	void printRule(string filename);

private:
	void AddNewRule(const p_rule &rule1);
	vector<int> getOverlapFromSubset(const p_rule &rule1,const vector<int> &ruleSet);
	//the main separate function
	void separateRule(const p_rule rule1,vector<int> &ovelap);
	//initial the mapping
	void mappingPrepare();


	std::unordered_map<uint32_t, int> mask2number;
	std::vector<p_rule> independentRuleSet;
};

#endif