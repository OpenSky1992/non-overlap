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
    pref_addr addrs[2];
    p_rule(){};
    inline p_rule(const p_rule &);
    inline p_rule(const std::string &);

    inline bool packet_hit(const addr_5tup &packet);
    inline std::string get_str();
    inline void print();

    inline bool match_rule(const p_rule &);
    inline bool operator==(const p_rule &);
};

inline p_rule::p_rule(const p_rule &br)
{
    for(uint32_t i=0; i<2; i++) {
        addrs[i] = br.addrs[i];
    }
}

inline p_rule::p_rule(const std::string &rule_str)
{
    vector<string> temp;
    boost::split(temp, rule_str, boost::is_any_of("\t"));
    //take off the first char '@'
    temp[0].erase(0,1);
    for(uint32_t i=0; i<2; i++) {
        addrs[i] = pref_addr(temp[i]);
    }
}

inline bool p_rule::packet_hit(const addr_5tup &packet)
{
    for (uint32_t i = 0; i < 2; i++) {
        if (!addrs[i].hit(packet.addrs[i]))
            return false;
    }
    return true;
}

inline std::string p_rule::get_str() 
{
    stringstream ss;
    for(uint32_t i = 0; i < 2; i++) {
        ss<<addrs[i].get_str()<<"\t";
    }
    return ss.str();
}

inline void p_rule::print() 
{
    cout<<get_str()<<endl;
}

inline bool p_rule::match_rule(const p_rule &br) 
{
    for (uint32_t i = 0; i < 2; ++i) {
        if (!addrs[i].match(br.addrs[i]))
            return false;
    }
    return true;
}

inline bool p_rule::operator==(const p_rule &rhs)
{
    for(uint32_t i=0; i<2; i++) {
        if(!(addrs[i]==rhs.addrs[i]))
            return false;
    }
    return true;
}

#endif