#ifndef ADDRESS_H
#define ADDRESS_H

#include "stdafx.h"
#include <boost/functional/hash.hpp>
using std::vector;

class range_addr;

class EpochT {
    long int sec;
    long int msec;

public:
    inline EpochT():sec(0),msec(0) {}

    inline EpochT(int isec, int imsec):sec(isec),msec(imsec) {}

    inline EpochT(const std::string & str) {
        std::vector<std::string> temp;
        boost::split(temp, str, boost::is_any_of("%"));
        sec = boost::lexical_cast<uint32_t> (temp[0]);
        msec = boost::lexical_cast<uint32_t> (temp[1]);
    }

    inline EpochT(const double & dtime) {
        sec = int(dtime);
        msec = int((dtime-sec)*1000000);
    }

    inline EpochT(const int & itime) {
        sec = int(itime);
        msec = 0;
    }

    inline EpochT(const EpochT & rhs) {
        sec = rhs.sec;
        msec = rhs.msec;
    }

    inline EpochT operator+(const double & dtime) const {
        long sec = this->sec + int(dtime);
        long msec = this->msec + int((dtime-int(dtime))*1000000);
        if (msec > 1000000) {
            msec -= 1000000;
            sec += 1;
        }
        EpochT res(sec, msec);
        return res;
    }

    inline EpochT operator+(const int & rhs) const {
        EpochT res(this->sec+rhs, this->msec);
        return res;
    }

    inline EpochT operator+(const EpochT & rhs) const {
        long sec = this->sec+rhs.sec;
        long msec = this->msec+rhs.sec;
        if (msec > 1000000) {
            msec -= 1000000;
            sec += 1;
        }
        EpochT res(sec, msec);
        return res;
    }

    inline EpochT operator-(const EpochT & rhs) const {
        long sec = this->sec-rhs.sec;
        long msec = this->msec-rhs.sec;
        if (msec < 0) {
            msec += 1000000;
            sec -= 1;
        }
        EpochT res(sec, msec);
        return res;
    }

    bool operator<(const EpochT & rhs) const {
        if (this->sec < rhs.sec) {
            return true;
        } else if(this->sec == rhs.sec) {
            if (this->msec < rhs.sec)
                return true;
        }
        return false;
    }

    double toDouble(const EpochT & offset) const {
        double res = this->sec - offset.sec;
        res += double(this->msec - offset.msec)/1000000;
        return res;
    }

};

class addr_5tup {
public:
    uint32_t addrs[4];
    bool proto;
    double timestamp;

public:
    inline addr_5tup();
    inline addr_5tup(const addr_5tup &);
    inline addr_5tup(const std::string &); // processing gen
    inline addr_5tup(const std::string &, const EpochT &); // processing raw
    inline addr_5tup(const std::string &, double);

    inline void copy_header(const addr_5tup &);
    inline bool operator==(const addr_5tup &) const;
    inline friend uint32_t hash_value(addr_5tup const &);

    inline std::string str_readable() const;
    inline std::string str_easy_RW() const;
};


class pref_addr {
public:
    uint32_t pref;
    uint32_t mask;

public:
    inline pref_addr();
    inline pref_addr(const pref_addr &);
    inline pref_addr(const std::string &);

    inline bool operator==(const pref_addr &) const;

    inline bool match (const pref_addr &) const;
    inline bool hit (const uint32_t &) const;
    inline uint32_t get_extreme(bool) const;
    inline uint32_t get_random() const;


    inline void print() const;
    inline std::string get_str() const;
};


// ---------------------- Addr_5tup ---------------------
using std::vector;
using std::string;
using std::stringstream;
using std::cout;
using std::endl;

inline addr_5tup::addr_5tup() {
    for (uint32_t i = 0; i < 4; i++)
        addrs[i] = 0;
    proto = true;
    timestamp = 0;
}

inline addr_5tup::addr_5tup(const addr_5tup & ad) {
    for (uint32_t i = 0; i < 4; i++)
        addrs[i] = ad.addrs[i];
    proto = ad.proto;
    timestamp = ad.timestamp;
}

inline addr_5tup::addr_5tup(const string & str) {
    vector<string> temp;
    boost::split(temp, str, boost::is_any_of("%"));
    proto = true;
    timestamp = boost::lexical_cast<double>(temp[0]);
    addrs[0] = boost::lexical_cast<uint32_t>(temp[1]);
    addrs[1] = boost::lexical_cast<uint32_t>(temp[2]);
    addrs[2] = boost::lexical_cast<uint32_t>(temp[3]);
    addrs[3] = boost::lexical_cast<uint32_t>(temp[4]);
}

inline addr_5tup::addr_5tup(const string & str, const EpochT & offset) {
    vector<string> temp;
    boost::split(temp, str, boost::is_any_of("%"));
    proto = true;
    EpochT ts_ep(boost::lexical_cast<uint32_t>(temp[0]), boost::lexical_cast<uint32_t>(temp[1]));
    timestamp = ts_ep.toDouble(offset);
    addrs[0] = boost::lexical_cast<uint32_t>(temp[2]);
    addrs[1] = boost::lexical_cast<uint32_t>(temp[3]);
    addrs[2] = boost::lexical_cast<uint32_t>(temp[4]);
    addrs[3] = boost::lexical_cast<uint32_t>(temp[5]);
}


inline addr_5tup::addr_5tup(const string & str, double ts) {
    vector<string> temp;
    boost::split(temp, str, boost::is_any_of("\t"));
    timestamp = ts;
    proto = true;
    for (uint32_t i = 0; i < 4; i++) {
        addrs[i] = boost::lexical_cast<uint32_t>(temp[i]);
    }
}

inline void addr_5tup::copy_header(const addr_5tup & ad) {
    for (uint32_t i = 0; i < 4; i++)
        addrs[i] = ad.addrs[i];
    proto = ad.proto;
}

inline bool addr_5tup::operator==(const addr_5tup & rhs) const {
    for (uint32_t i = 0; i < 4; i++) {
        if (addrs[i] != rhs.addrs[i])
            return false;
    }
    return (proto == rhs.proto);
}

inline uint32_t hash_value(addr_5tup const & packet) {
    size_t seed = 0;
    boost::hash_combine(seed, packet.addrs[0]);
    boost::hash_combine(seed, packet.addrs[1]);
    boost::hash_combine(seed, packet.addrs[2]);
    boost::hash_combine(seed, packet.addrs[3]);
    return seed;
}

inline string addr_5tup::str_readable() const {
    stringstream ss;
    ss.precision(15);
    ss<<timestamp<<"%";
    for (uint32_t i = 0; i < 2; i++) {
        for (uint32_t j = 0; j < 4; j++) {
            ss << ((addrs[i] >> (24-8*j)) & ((1<<8)-1));
            if (j!=3)
                ss<<".";
        }
        ss<<"%";
    }
    for (uint32_t i = 2; i < 4; i++)
        ss<<addrs[i]<<"%";

    if (proto)
        ss<<"6";
    else
        ss<<"13";
    return ss.str();
}

inline string addr_5tup::str_easy_RW() const {
    stringstream ss;
    ss.precision(15);
    ss<<timestamp<<"%";
    for (uint32_t i = 0; i < 4; i++) {
        ss<<addrs[i]<<"%";
    }
    if (proto)
        ss<<"1";
    else
        ss<<"0";
    return ss.str();
}

// ---------------------- pref_addr ---------------------

inline pref_addr::pref_addr() {
    pref = 0;
    mask = 0;
}

inline pref_addr::pref_addr(const string & prefstr) {
    vector<string> temp1;
    boost::split(temp1, prefstr, boost::is_any_of("/"));

    uint32_t maskInt = boost::lexical_cast<uint32_t>(temp1[1]);
    mask = 0;
    if (maskInt != 0)
        mask = ((~uint32_t(0)) << (32-maskInt));

    vector<string> temp2;
    boost::split(temp2, temp1[0], boost::is_any_of("."));

    pref = 0;
    for(uint32_t i=0; i<4; i++) {
        pref = (pref<<8) + boost::lexical_cast<uint32_t>(temp2[i]);
    }
    pref=(pref & mask);
}

inline pref_addr::pref_addr(const pref_addr & pa) {
    pref = pa.pref;
    mask = pa.mask;
}

inline bool pref_addr::operator==(const pref_addr & rhs) const {
    if (mask != rhs.mask)
        return false;
    if((pref & mask) != (rhs.pref & mask))
        return false;
    return true;
}

inline bool pref_addr::hit(const uint32_t & ad) const {
    return (pref == (ad & mask));
}

inline bool pref_addr::match(const pref_addr & ad) const {
    uint32_t mask_short;
    if (mask > ad.mask)
        mask_short = ad.mask;
    else
        mask_short = mask;

    return ((pref & mask_short) == (ad.pref & mask_short));
}

inline uint32_t pref_addr::get_extreme(bool hi) const {
    if (hi)
        return (pref+(~mask));
    else
        return pref;
}

inline uint32_t pref_addr::get_random() const {
    if (!(~mask+1))
        return pref;
    return (pref + rand()%(~mask+1));
}

inline void pref_addr::print() const {
    cout<<get_str()<<endl;
}

inline string pref_addr::get_str() const {
    stringstream ss;
    for (uint32_t i = 0; i<4; i++) {
        ss<<((pref>>(24-(i*8))&((1<<8)-1)));
        if (i != 3)
            ss<<".";
    }
    ss<<"/";

    uint32_t m = 0;
    uint32_t mask_cp = mask;

    if ((~mask_cp) == 0) {
        ss<<32;
        return ss.str();
    }
    for (uint32_t i=0; mask_cp; i++) {
        m++;
        mask_cp = (mask_cp << 1);
    }
    ss<<m;
    return ss.str();
}

#endif