#include "OFswitch.h"

namespace fs = boost::filesystem;
namespace io = boost::iostreams;

using std::ifstream;
/* Constructor:
 *
 * method brief:
 */

OFswitch::OFswitch(double time,uint32_t capacity,string trace) {
    rList = NULL;
    bTree = NULL;
    sep=NULL;

    mode = 0;
    simuT = time;
    TCAMcap = capacity;
    tracefile_str = trace;
}

/* Constructor:
 *
 *
 * method brief:
 * same as default
 */
void OFswitch::set_para(rule_list * rL, bucket_tree * bT,separate *separate) {
    rList = rL;
    bTree = bT;
    sep=separate;
}

/* run_test
 *
 * method brief:
 * do test according mode id
 * mode 0: CAB, mode 1: Exact Match, mode 2: Cache Dependent Rules, mode 3: Cache non-overlap rules
 */

void OFswitch::run_test() {
    switch (mode) {
    case 0:
        CABtest_rt_TCAM();
        break;
    case 1:
        CEMtest_rt_TCAM();
        break;
    case 2:
        CNORtest_rt_TCAM();
        break;
    default:
        return;
    }
}

/* CABtest_rt_TCAM
 *
 * method brief:
 * test caching in bucket
 * trace format <addr5tup(easyRW)>
 * test CAB performance
 */
void OFswitch::CABtest_rt_TCAM() {
    double curT = 0;

    lru_cache_cab cam_cache(TCAMcap, simuT);
    boost::unordered_set<addr_5tup> flow_rec;

    try {
        io::filtering_istream trace_stream;
        trace_stream.push(io::gzip_decompressor());
        ifstream infile(tracefile_str);
        trace_stream.push(infile);

        string str; 
        uint32_t packetCount=0;
        while(getline(trace_stream, str)) {
            addr_5tup packet(str);
            curT = packet.timestamp;
            packetCount++;

            auto res = flow_rec.insert(packet);
            bucket* buck = bTree->search_bucket(packet).first;
            //++total_packet;
            if (buck!=NULL){
                cam_cache.ins_rec(buck, curT, res.second);
            } 
            else{
                cout<<"null bucket" <<endl;
            }

            if (curT > simuT)
                break;
        }
        io::close(trace_stream);
        cout<<endl<<"flow info:"<<endl;
        cout<<"totoal packet no:"<<packetCount<<endl;
        cout<<"totoal flow no:"<<flow_rec.size()<<endl;

    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }

    cout<<endl<<"the CAB effect:"<<endl;
    cam_cache.fetch_data();
}

/* CEMtest_rt_id
 *
 * method brief:
 * testing cache exact match entries
 * trace format <time \t ID>
 * others same as CABtest_rt_TCAM
 */
void OFswitch::CEMtest_rt_TCAM() {
    fs::path ref_trace_path(tracefile_str);
    if (!(fs::exists(ref_trace_path) && fs::is_regular_file(ref_trace_path))) {
        cout<<"Missing Ref file"<<endl;
        return;
    }

    double curT = 0;
    lru_cache<addr_5tup> cam_cache(TCAMcap, simuT);
    boost::unordered_set<addr_5tup> flow_rec;

    try {
        io::filtering_istream trace_stream;
        trace_stream.push(io::gzip_decompressor());
        ifstream trace_file(tracefile_str);
        trace_stream.push(trace_file);

        string str;
        while(getline(trace_stream, str)) {
            addr_5tup packet(str);
            curT = packet.timestamp;
            
            auto res = flow_rec.insert(packet);
            cam_cache.ins_rec(packet, curT, res.second);

            if (curT > simuT)
                break;
        }
        io::close(trace_stream);
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }

    cout<<endl<<"the CEM effect:"<<endl;
    cam_cache.fetch_data();

}

/* CNOtest_rt_TCAM
 *
 * method brief:
 * testing cache non-overlap rules
 * trace format <time \t ID>
 * others same as CABtest_rt_TCAM
 */
void OFswitch::CNORtest_rt_TCAM() {
    fs::path ref_trace_path(tracefile_str);
    if (!(fs::exists(ref_trace_path) && fs::is_regular_file(ref_trace_path))) {
        cout<<"Missing Ref file"<<endl;
        return;
    }

    double curT = 0;
    lru_cache<uint32_t> cam_cache(TCAMcap, simuT);
    boost::unordered_set<addr_5tup> flow_rec;

    try {
        io::filtering_istream trace_stream;
        trace_stream.push(io::gzip_decompressor());
        ifstream trace_file(tracefile_str);
        trace_stream.push(trace_file);

        string str;
        uint32_t packetCount=0;
        while(getline(trace_stream, str)) {
            addr_5tup packet(str);
            curT = packet.timestamp;
            packetCount++;
            
            auto res = flow_rec.insert(packet);
            int ruleIndex=sep->searchIndepIndex(packet);
            if(ruleIndex>=0){
                cam_cache.ins_rec(ruleIndex, curT, res.second);
            }
            else{
                cout<<"null rules"<<endl;
            }

            if(packetCount%10000==0){
                cout<<packetCount/10000<<endl;
            }

            if (curT > simuT)
                break;
        }
        io::close(trace_stream);
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }

    cout<<endl<<"the CNOR effect:"<<endl;
    cam_cache.fetch_data();

}



