#ifndef RULE_H
#define RULE_H

#include "stdafx.h"
#include "Address.hpp"
#include <boost/functional/hash.hpp>


using std::pair;
using std::endl;

class p_rule
{
public:
    pref_addr addrs[number_prefix];
    p_rule(){};
    inline p_rule(const p_rule &);
    inline p_rule(const std::string &);

    inline bool packet_hit(const addr_5tup &packet) const;
    inline addr_5tup get_random() const;
    inline std::string get_str() const;
    inline void print() const;

    inline bool match_rule(const p_rule &) const;
    inline bool operator==(const p_rule &) const;
};

class h_rule:public p_rule
{
public:
    inline h_rule(const h_rule &);
    inline h_rule(const std::string &);
    inline h_rule(const std::string &, const std::vector<p_rule> &);
    inline addr_5tup gen_header();
    inline bool match_truncate(p_rule &);
    inline void mutate_pred(uint32_t, uint32_t);
private:
    inline uint32_t cal_rela(const std::vector<p_rule> &);
    std::vector<p_rule> rela_rule;
};

inline p_rule::p_rule(const p_rule &br)
{
    for(uint32_t i=0; i<number_prefix; i++) {
        addrs[i] = br.addrs[i];
    }
}

inline p_rule::p_rule(const std::string &rule_str)
{
    vector<string> temp;
    boost::split(temp, rule_str, boost::is_any_of("\t"));
    //take off the first char '@'
    temp[0].erase(0,1);
    for(uint32_t i=0; i<number_prefix; i++) {
        addrs[i] = pref_addr(temp[i]);
    }
}

inline bool p_rule::packet_hit(const addr_5tup &packet) const
{
    for (uint32_t i = 0; i < number_prefix; i++) {
        if (!addrs[i].hit(packet.addrs[i]))
            return false;
    }
    return true;
}

inline addr_5tup p_rule::get_random() const
{
    addr_5tup header;
    for (uint32_t i = 0; i < number_prefix; i++){
        header.addrs[i]=addrs[i].get_random();
    }
    header.addrs[2]=0;
    header.addrs[3]=0;
    header.proto=true;
    return header;
}

inline std::string p_rule::get_str() const
{
    stringstream ss;
    for(uint32_t i = 0; i < number_prefix; i++) {
        ss<<addrs[i].get_str()<<"\t";
    }
    return ss.str();
}

inline void p_rule::print() const
{
    cout<<get_str()<<endl;
}

inline bool p_rule::match_rule(const p_rule &br) const
{
    for (uint32_t i = 0; i < number_prefix; ++i) {
        if (!addrs[i].match(br.addrs[i]))
            return false;
    }
    return true;
}

inline bool p_rule::operator==(const p_rule &rhs) const
{
    for(uint32_t i=0; i<number_prefix; i++) {
        if(!(addrs[i]==rhs.addrs[i]))
            return false;
    }
    return true;
}

inline h_rule::h_rule(const string & line):p_rule(line) {};

inline h_rule::h_rule(const h_rule & hr):p_rule(hr) {
    rela_rule = hr.rela_rule;
}

inline h_rule::h_rule(const string & str, const vector<p_rule> & rL) {
    vector<string> temp;
    boost::split(temp,str,boost::is_any_of("\t"));
    for(uint32_t i=0;i<number_prefix;i++){
        addrs[i]=pref_addr(temp[i]);
    }
    cal_rela(rL);
}

inline void h_rule::mutate_pred(uint32_t shrink_scale, uint32_t expand_scale) {
    for (uint32_t i = 0; i < number_prefix; ++i){
        addrs[i].mutate(shrink_scale, expand_scale);
    }
}

inline bool h_rule::match_truncate(p_rule &rule){
    for (uint32_t i = 0; i < number_prefix; ++i){
        if(!addrs[i].truncate(rule.addrs[i]))
            return false;
    }
    return true;
}

inline uint32_t h_rule::cal_rela(const vector<p_rule> & rList) {
    for (uint32_t i = 0; i < rList.size(); i++) {
        p_rule rule=rList[i];
        if(match_truncate(rule)) {
            rela_rule.push_back(rule);
        }
    }
    return rela_rule.size();
}

inline addr_5tup h_rule::gen_header() {
    auto iter = rela_rule.begin();
    advance(iter, rand()%rela_rule.size());
    return iter->get_random();
}

#endif