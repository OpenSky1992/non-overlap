#ifndef SEPARATE_H
#define SEPARATE_H

#include "stdafx.h"
#include "Rule.hpp"
#include "RuleList.h"
#include "BucketTree.h"
#include <unordered_map>


class separate
{
public:
	separate(rule_list &rList);
	void printRule(string filename);
	~separate();

	//return the index of origin rule set 
	int searchOriginIndex(const addr_5tup &) const;

	//return the index of independent rule set
	int searchIndepIndex(const addr_5tup &) const;

private:
	int AddNewRule(const p_rule &rule1,const uint32_t index);
	vector<int> getOverlapFromSubset(const p_rule &rule1,const vector<int> &ruleSet);
	
	//the main separate function
	void separateRule(const p_rule rule1,const uint32_t index,vector<int> &ovelap);
	
	//initial the mapping
	void mappingPrepare();
	//map the mask to the number of 0, e.g. 0xfffffff0 -> 4
	std::unordered_map<uint32_t, int> mask2number;

	rule_list independentRuleSet;

	//record the mapping of the index of independent set and orgin rule set.
	std::vector<uint32_t> indepIndex2realIndex;

	//rule_tree indepTree;
	bucket_tree *indepTree;
};

#endif