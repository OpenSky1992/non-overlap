#ifndef BUCKET_TREE
#define BUCKET_TREE

#include "stdafx.h"
#include "Address.hpp"
#include "Rule.hpp"
#include "RuleList.h"
#include "Bucket.h"
#include <cmath>
#include <set>
#include <deque>
#include <list>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>

class bucket_tree {
  private:
    boost::log::sources::logger bTree_log;
  public:
    bucket * root;
    rule_list * rList;
    uint32_t threshold;

    int tree_depth;
    std::vector<std::vector<size_t> > candi_split;

  public:
    bucket_tree();
    
    //rule_list, threshold that must be split
    bucket_tree(rule_list &, uint32_t thre);
    ~bucket_tree();

    //return bucket and the index of rule
    std::pair<bucket *, int> search_bucket(const addr_5tup &) const;
    //get the tree depth
    int getTreeDepth();
    

  private:
    // static related
    void gen_candi_split(size_t = 2);
    void splitNode_fix(bucket * = NULL);
    void delNode(bucket *);
    void print_bucket(std::ofstream &, bucket *, bool); // const
    std::pair<bucket *, int> search_bucket_R(const addr_5tup &, bucket* ) const;
    void cal_tree_depth(bucket *, int = 0);

  public:

    void print_tree(const string & filename, bool details = false); // const

};

#endif


