#include "Separate.h"

using std::vector;
using std::cout;
using std::endl;
using std::ofstream;

separate::separate(rule_list &rList)
{
	int uselessRule_count=0;
	uint32_t originIndex=0;
	mappingPrepare();
	for(p_rule original_rule:rList.list)
	{
		if(AddNewRule(original_rule,originIndex)==0)
			uselessRule_count++;
		originIndex++;
	}
	cout<<"the number of useless rule:"<<uselessRule_count<<endl;
	cout<<"the number of independent rule:"<<independentRuleSet.size()<<endl;
}

void separate::printRule(string filename)
{
	ofstream out(filename);
	for(p_rule indep:independentRuleSet)
		out<<indep.get_str()<<endl;
	out.close();
}

// initial the mapping relatioin
void separate::mappingPrepare()
{
	unsigned int temp=0;
	temp=~temp;
	for(unsigned int i=0;i<=ipv4_standard_length;i++)
	{
		mask2number[temp]=i;
		temp=temp<<1;
	}
}

// return rules overlap with parameter:rule1 from parameter:ruleSet 
vector<int> separate::getOverlapFromSubset(const p_rule &rule1,const vector<int> &ruleSet)
{
	vector<int> result;
	for(int index:ruleSet){
		if(rule1.match_rule(independentRuleSet[index]))
			result.push_back(index);
	}
	return result;
}

//the main function
void separate::separateRule(const p_rule rule1,const uint32_t originIndex,vector<int> &overlap)
{
	if(overlap.empty()){
		independentRuleSet.push_back(rule1);
		indepIndex2realIndex.push_back(originIndex);
		return ;
	}
	auto it=overlap.begin();
	int maxItValue=*it;
	it=overlap.erase(it);

	p_rule new_rule=rule1;
	p_rule target=independentRuleSet[maxItValue];
	for(unsigned int i=0;i<number_prefix;i++){
		int rule1_mask,target_mask,diff_mask;
		uint32_t compare_bit;
		rule1_mask=mask2number[new_rule.addrs[i].mask];
		target_mask=mask2number[target.addrs[i].mask];
		if(rule1_mask<=target_mask)
			continue;
		compare_bit=1<<(rule1_mask-1);
		diff_mask=rule1_mask-target_mask;
		for(int j=0;j<diff_mask;j++){
			new_rule.addrs[i].pref=target.addrs[i].pref ^ compare_bit;
			new_rule.addrs[i].mask=(new_rule.addrs[i].mask>>1)+(1<<(ipv4_standard_length-1));
			new_rule.addrs[i].pref=new_rule.addrs[i].pref & new_rule.addrs[i].mask;
			vector<int> subOverlap=getOverlapFromSubset(new_rule,overlap);
			separateRule(new_rule,originIndex,subOverlap);
			new_rule.addrs[i].pref=target.addrs[i].pref & new_rule.addrs[i].mask;
			compare_bit=compare_bit>>1;
		}
	}
}

int separate::AddNewRule(const p_rule &rule1,const uint32_t originIndex)
{
	int beforeSize=independentRuleSet.size();
	vector<int> overlap;
	for(unsigned int i=0;i<independentRuleSet.size();i++){
		if(rule1.match_rule(independentRuleSet[i]))
			overlap.push_back(i);
	}
	separateRule(rule1,originIndex,overlap);
	return independentRuleSet.size()-beforeSize;
}

int separate::searchIndepIndex(const addr_5tup &packet) const
{
	int result=-1;
	for(int i=0;i<int(independentRuleSet.size());i++){
		if(independentRuleSet[i].packet_hit(packet)){
			result=i;
			break;
		}
	}
	return result;
}

int separate::searchOriginIndex(const addr_5tup &packet) const
{
	int result=searchIndepIndex(packet);
	if(result==-1)
		return -1;
	else
		return indepIndex2realIndex[result];
}