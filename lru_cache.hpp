#ifndef _LRU_CACHE_H
#define _LRU_CACHE_H

#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/function.hpp>
#include <cassert>
#include <iostream>
#include <vector>

#include "BucketTree.h"
#include <boost/unordered_map.hpp>

template <typename K>
class lru_cache
{
protected:
    typedef K record_T;
    typedef boost::bimaps::bimap<
    boost::bimaps::unordered_set_of<record_T>,
          boost::bimaps::list_of<double>
          > container_T;
    const uint32_t _capacity;
    const double simT;
    const double rtt;

    container_T cache;	// record the updated time
    boost::unordered_map<record_T, double> cache_rec; // record the first time
    
    uint32_t delay_rec[21];
    inline void insert(const record_T &, const double &);

public:
    uint32_t cache_miss;
    uint32_t reuse_count;

    inline lru_cache();
    inline lru_cache(const uint32_t &TcamCap, double time, double = 0.006);

    inline bool ins_rec(const record_T &, double, bool = false);
};

class lru_cache_cab
{
protected:
    typedef boost::bimaps::bimap<
    boost::bimaps::unordered_set_of<const bucket*>,
          boost::bimaps::list_of<double>
          > container_T;
    const uint32_t _capacity;
    const double simT;
    const double rtt;
    container_T cache;
    boost::unordered_map <uint32_t, uint32_t> flow_table;
    boost::unordered_map <const bucket*, double> buffer_check;
    uint32_t delay_rec[21];
    inline void insert(const bucket*, const double &);

public:
    uint32_t cache_miss;
    uint32_t rule_down_count;
    uint32_t reuse_count;

    inline lru_cache_cab();
    inline lru_cache_cab(const uint32_t &, const double &, const double & = 0.006);
    inline std::pair<size_t, size_t> getTcamUseInfo();

    inline bool ins_rec(const bucket *, const double &, bool = false);

};


/* -------------------------- class lru_cache<T> ------------------------------------
 *
 */
template <typename K>
inline lru_cache<K>::lru_cache():_capacity(0), simT(0), rtt(0) {}

template <typename K>
inline lru_cache<K>::lru_cache(const uint32_t & tcamCapacity, double time, double rt): _capacity(tcamCapacity), simT(time), rtt(rt)
{
    assert(_capacity != 0);
    cache_miss = 0;
    reuse_count = 0;
    for (uint32_t i = 0; i<21; ++i)
        delay_rec[i] = 0;
}

template <typename K>
inline bool lru_cache<K>::ins_rec(const record_T & rec, double curT, bool newFlow)
{
    const typename container_T::left_iterator it =
        cache.left.find(rec);

    if (it == cache.left.end())   // cache miss
    {
        insert (rec, curT);
        if (newFlow)   // delayed for rtt, insert the rec
        {
            ++delay_rec[20];
        }
        ++cache_miss;
        return true;
    }
    else     // cache hit
    {
        cache.right.relocate(cache.right.end(),
                             cache.project_right(it));
        it->second = curT;
        double delay = curT - cache_rec[rec];

        if (newFlow)
        {
            ++reuse_count;
            if (delay < rtt)   // delayed for record time
            {
                assert (delay >= 0);
                ++delay_rec[int(20*(rtt-delay)/rtt)];
            }
        }
        return false;
    }
}


template <typename K>
inline void lru_cache<K>::insert(const record_T & rec, const double & curT)
{
    cache_rec.insert(std::make_pair(rec, curT));

    assert(cache.size()<=_capacity);
    if (cache.size() == _capacity)
    {
        cache.right.erase(cache.right.begin());
        cache_rec.erase(cache.right.begin()->second);
    }

    cache.insert(
        typename container_T::value_type(rec, curT)
    );
}


/* -------------------------- class lru_cache<T> ------------------------------------
 *
 */

inline lru_cache_cab::lru_cache_cab(): _capacity(0), simT(0), rtt(0) {}

inline lru_cache_cab::lru_cache_cab(const uint32_t & cap, const double & time, const double & rt):_capacity(cap), simT(time), rtt(rt)
{
    rule_down_count = 0;
    reuse_count = 0;
    cache_miss = 0;
    for (int i = 0; i<21; ++i)
        delay_rec[i] = 0;
}

inline bool lru_cache_cab::ins_rec(const bucket * buck, const double & curT, bool newFlow)
{
    const container_T::left_iterator iter = cache.left.find(buck);
    if (iter == cache.left.end())   // cache miss
    {
        insert(buck, curT);
        if (newFlow)
        {
            ++delay_rec[20];
        }
        ++cache_miss;
        return true;
    }
    else     // cache hit
    {
        cache.right.relocate(cache.right.end(),
                             cache.project_right(iter));
        iter->second = curT;
        double delay = curT-buffer_check.find(buck)->second;

        if (newFlow)
        {
            ++reuse_count;
            if(delay < rtt)
            {
                assert (delay >= 0);
                ++delay_rec[int(20*(rtt-delay)/rtt)];
            }
        }
        return false;
    }
}

inline void lru_cache_cab::insert(const bucket* pbuck, const double & time)
{
    assert(cache.size() + flow_table.size() <=_capacity);
    buffer_check.insert(std::make_pair(pbuck, time)); // insert bucket as rec

    cache.insert(container_T::value_type(pbuck, time)); // insert bucket
    for (auto iter = pbuck->related_rules.begin(); iter != pbuck->related_rules.end(); iter++)
    {
        ++rule_down_count;  // controller does not know which rules are kept in OFswtich
        auto ins_rule_result = flow_table.insert(std::make_pair(*iter, 1));
        if (!ins_rule_result.second)
            ++ins_rule_result.first->second;
        //else
        //    ++rule_down_count; // controller knows which rules are kept in OFswitch
    }

    while(cache.size() + flow_table.size() > _capacity)   // kick out
    {
        const bucket * to_kick_buck = cache.right.begin()->second;
        cache.right.erase(cache.right.begin());
        buffer_check.erase(to_kick_buck);

        for (auto iter = to_kick_buck->related_rules.begin(); iter != to_kick_buck->related_rules.end(); ++iter)   // dec flow occupy no.
        {
            --flow_table[*iter];
            if (flow_table[*iter] == 0)
                flow_table.erase(*iter);
        }
    }
}

inline std::pair<size_t, size_t> lru_cache_cab::getTcamUseInfo()
{
    return std::make_pair(cache.size(),flow_table.size());
}

#endif

