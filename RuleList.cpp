#include "RuleList.h"

using std::ifstream;
using std::ofstream;
using std::string;
using std::tie; // jiaren:missing the namespace
using std::cout;
using std::endl;


rule_list::rule_list() {}

rule_list::rule_list(string & filename) {
    ifstream file;
    file.open(filename.c_str());
    string sLine = "";
    getline(file, sLine);
    while (!file.eof()) {
        p_rule sRule(sLine);
        list.push_back(sRule);
        getline(file, sLine);
    }
    for(auto iter = list.begin(); iter != list.end(); ++iter) {
        for (auto iter_cp = iter+1; iter_cp != list.end(); ) {
            if (*iter == *iter_cp)
                iter_cp = list.erase(iter_cp);
            else
                ++iter_cp;
        }
    }
    file.close();
    cout<<"unique rule number:"<<list.size()<<endl;
}


int rule_list::linear_search(const addr_5tup & packet) {
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i].packet_hit(packet))
            return i;
    }
    return -1;
}

void rule_list::print(const string & filename) {
    ofstream file;
    file.open(filename.c_str());
    for (vector<p_rule>::iterator iter = list.begin();
            iter != list.end(); iter++) {
        file<<iter->get_str()<<endl;
    }
    file.close();
}
