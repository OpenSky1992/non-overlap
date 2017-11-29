#include "Bucket.h"

// --------- bucket ------------

using std::vector;
using std::list;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::set;


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace attrs = boost::log::attributes;

typedef vector<uint32_t>::iterator Iter_id;
typedef vector<bucket*>::iterator Iter_son;


src::logger bucket::lg = src::logger();

void bucket::logger_init() {
    bucket::lg.add_attribute("Class", attrs::constant< string > ("BuckObj "));
}

bucket::bucket():hit(false), parent(NULL), max_gain(0) {}

bucket::bucket(const bucket & bk) : p_rule(bk) {
    sonList = vector<bucket*>();
    related_rules = vector<uint32_t>();
    hit = false;
    parent = NULL;
    max_gain = 0;
}

bucket::bucket(const string & b_str, const rule_list * rL) : p_rule(b_str) {
    for (size_t idx = 0; idx != rL->list.size(); ++idx)
        if (match_rule(rL->list[idx]))
            related_rules.push_back(idx);
    hit = false;
    parent = NULL;
    max_gain = 0;
}

pair<double, size_t> bucket::split(const vector<size_t> & dim , rule_list *rList) {
    if (!sonList.empty())
        cleanson();

    uint32_t new_masks[4];
    size_t total_son_no = 1;

    for (size_t i = 0; i < 4; ++i) { // new mask
        new_masks[i] = addrs[i].mask;

        for (size_t j = 0; j < dim[i]; ++j) {
            if (~(new_masks[i]) == 0)
                return std::make_pair(-1, 0);

            new_masks[i] = (new_masks[i] >> 1) + (1 << 31);
            total_son_no *= 2;
        }
    }

    sonList.reserve(total_son_no);

    size_t total_rule_no = 0;
    size_t largest_rule_no = 0;

    for (size_t i = 0; i < total_son_no; ++i) {
        bucket * son_ptr = new bucket(*this);
        son_ptr->parent = this;

        uint32_t id = i;
        for (size_t j = 0; j < 4; ++j) { // new pref
            son_ptr->addrs[j].mask = new_masks[j];
            size_t incre = (~(new_masks[j]) + 1);
            son_ptr->addrs[j].pref += (id % (1 << dim[j]))*incre;
            id = (id >> dim[j]);
        }

        for (Iter_id iter = related_rules.begin(); iter != related_rules.end(); ++iter) { // rela rule
            if (son_ptr->match_rule(rList->list[*iter]))
                son_ptr->related_rules.push_back(*iter);
        }

        total_rule_no += son_ptr->related_rules.size();
        largest_rule_no = std::max(largest_rule_no, son_ptr->related_rules.size());

        sonList.push_back(son_ptr);
    }
    return std::make_pair(double(total_rule_no)/total_son_no, largest_rule_no);
}

string bucket::get_str() {
    stringstream ss;
    ss << p_rule::get_str() << "\t" << related_rules.size();
    return ss.str();
}

void bucket::cleanson() {
    for (auto iter = sonList.begin(); iter != sonList.end(); ++iter)
        delete (*iter);
    sonList.clear();
}

