#include "Separate.h"

using std::vector;
using std::cout;
using std::endl;
using std::ofstream;

separate::separate(rule_list *sourceList)
{
	int uselessRule_count=0;
	rList=sourceList;
	mappingPrepare();
	for(uint32_t originIndex=0;originIndex<(rList->list.size());originIndex++)
	{
		if(AddNewRule(originIndex)==0)
			uselessRule_count++;
	}
	cout<<"the number of useless rule:"<<uselessRule_count<<endl;
	cout<<"the number of independent rule:"<<independentRuleSet.list.size()<<endl;
	rList=NULL;
	indepTree=new bucket_tree(independentRuleSet,50);
	//cout<<"the indep tree depth: "<<indepTree->getTreeDepth()<<endl;
}

separate::~separate()
{
	delete indepTree;
}

void separate::printRule(string filename)
{
	ofstream out(filename);
	for(p_rule indep:independentRuleSet.list)
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

//the main function
int separate::AddNewRule(const uint32_t originIndex)
{
	int beforeSize=independentRuleSet.list.size();

	p_rule newRule=rList->list[originIndex];
	const vector<uint32_t> &depRule=rList->dep_map[originIndex];
	remainWildcard.clear();
	remainWildcard.insert(remainWildcard.begin(),newRule);
	for(uint32_t depRuleIndex:depRule)
	{
		p_rule target=rList->list[depRuleIndex];
		auto depIt=remainWildcard.begin();
		while(depIt!=remainWildcard.end())
		{
			if(depIt->match_rule(target))
			{
				p_rule new_rule=*depIt;
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
				    	//strictly speaking, this will incur error. 
						//In the current, a rule from two prefix. this will not incur error.
						new_rule.addrs[i].pref=target.addrs[i].pref ^ compare_bit;
						new_rule.addrs[i].mask=(new_rule.addrs[i].mask>>1)+(1<<(ipv4_standard_length-1));
						new_rule.addrs[i].pref=new_rule.addrs[i].pref & new_rule.addrs[i].mask;
						depIt=remainWildcard.insert(depIt,new_rule);
						depIt++;
						new_rule.addrs[i].pref=target.addrs[i].pref & new_rule.addrs[i].mask;
						compare_bit=compare_bit>>1;
					}
				}
				depIt=remainWildcard.erase(depIt);
			}
			else
				depIt++;
		}
	}

	for(auto it=remainWildcard.begin();it!=remainWildcard.end();it++)
	{
		independentRuleSet.list.push_back(*it);
	}

	return independentRuleSet.list.size()-beforeSize;
}

int separate::searchIndepIndex(const addr_5tup &packet) const
{
	return indepTree->search_bucket(packet).second; 
}

int separate::searchOriginIndex(const addr_5tup &packet) const
{
	int result=searchIndepIndex(packet);
	if(result==-1)
		return -1;
	else
		return indepIndex2realIndex[result];
}