#include "BucketTree.h"

typedef vector<uint32_t>::iterator Iter_id;
typedef vector<bucket*>::iterator Iter_son;

namespace fs = boost::filesystem;
namespace io = boost::iostreams;

using std::set;
using std::list;
using std::ifstream;
using std::ofstream;

// ---------- bucket_tree ------------
bucket_tree::bucket_tree() {
    root = NULL;
    threshold=0;
}

//constructor
bucket_tree::bucket_tree(rule_list & rL, uint32_t thr) {
    threshold=thr;
    rList = &rL;
    root = new bucket(); // full address space
    for (uint32_t i = 0; i < rL.list.size(); i++)
        root->related_rules.insert(root->related_rules.end(), i);

    gen_candi_split();
    splitNode_fix(root);
    tree_depth = 0;
}

bucket_tree::~bucket_tree() {
    delNode(root);
}

std::pair<bucket *, int> bucket_tree::search_bucket(const addr_5tup &packet) const
{
    return search_bucket_R(packet,root);
}

pair<bucket *, int> bucket_tree::search_bucket_R(const addr_5tup &packet, bucket * buck) const {
    if (!buck->sonList.empty()) {
        size_t idx = 0;
        for (int i = number_prefix-1; i >= 0; --i) {
            if (buck->cutArr[i] != 0) {
                idx = (idx << buck->cutArr[i]);
                size_t offset = (packet.addrs[i] - buck->addrs[i].pref);
                offset = offset/((~(buck->addrs[i].mask) >> buck->cutArr[i]) + 1);
                idx += offset;
            }
        }
        assert (idx < buck->sonList.size());
        return search_bucket_R(packet, buck->sonList[idx]);
    } else {
        int rule_id = -1;
        for (auto iter = buck->related_rules.begin(); iter != buck->related_rules.end(); ++iter) {
            if (rList->list[*iter].packet_hit(packet)) {
                rule_id = *iter;
                break;
            }
        }
        return std::make_pair(buck, rule_id);
    }
}

void bucket_tree::gen_candi_split(size_t cut_no) {
    vector<size_t> base(number_prefix,0);
    for (size_t i = 0; i <= cut_no; ++i) {
        base[0] = i;
        base[1] = cut_no - i;
        candi_split.push_back(base);
    }
}

void bucket_tree::splitNode_fix(bucket * ptr) {
    double cost = ptr->related_rules.size();
    if (cost < threshold)
        return;

    pair<double, size_t> opt_cost = std::make_pair(ptr->related_rules.size(), ptr->related_rules.size());
    vector<size_t> opt_cut;

    for (auto iter = candi_split.begin(); iter != candi_split.end(); ++iter) {
        auto cost = ptr->split(*iter, rList);

        if (cost.first < 0)
            continue;

        if (cost.first < opt_cost.first || ((cost.first == opt_cost.first) && (cost.second < opt_cost.second))) {
            opt_cut = *iter;
            opt_cost = cost;
        }
    }

    if (opt_cut.empty()) {
        ptr->cleanson();
        return;
    } else {
        ptr->split(opt_cut, rList);
        for (size_t i = 0; i < number_prefix; ++i)
            ptr->cutArr[i] = opt_cut[i];

        for (auto iter = ptr->sonList.begin(); iter != ptr->sonList.end(); ++iter)
            splitNode_fix(*iter);
    }
}

void bucket_tree::delNode(bucket * ptr) {
    for (Iter_son iter = ptr->sonList.begin(); iter!= ptr->sonList.end(); iter++) {
        delNode(*iter);
    }
    delete ptr;
}

void bucket_tree::cal_tree_depth(bucket * ptr, int count) {
    for (Iter_son iter = ptr->sonList.begin(); iter != ptr->sonList.end(); iter++) {
        cal_tree_depth(*iter, count+1);
    }
    if (count > tree_depth)
        tree_depth = count;
}

void bucket_tree::print_bucket(ofstream & in, bucket * bk, bool detail) { // const
    if (bk->sonList.empty()) {
        in << bk->get_str() << endl;
        if (detail) {
            in << "re: ";
            for (Iter_id iter = bk->related_rules.begin(); iter != bk->related_rules.end(); iter++) {
                in << *iter << " ";
            }
            in <<endl;
        }

    } else {
        for (Iter_son iter = bk->sonList.begin(); iter != bk->sonList.end(); iter++)
            print_bucket(in, *iter, detail);
    }
    return;
}

void bucket_tree::print_tree(const string & filename, bool det) { // const
    ofstream out(filename);
    print_bucket(out, root, det);
    out.close();
}

