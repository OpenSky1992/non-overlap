#include "OFswitch_CB.h"

namespace fs = boost::filesystem;
namespace io = boost::iostreams;

using std::ifstream;
using std::ofstream;

/* Constructor:
 *
 * method brief:
 */

OFswitch_CB::OFswitch_CB(string trace,string statistics) {
    rList = NULL;
    bTree = NULL;
    sep=NULL;

    mode = 0;
    simuT = 100;
    TCAMcap = 4000;
    traceFile = trace;
    statFile = statistics;
}

void OFswitch_CB::run_test() {
    switch (mode) {
    case 0:
        flowInfomation();
        break;
    case 1:
        CEMtest_rt_TCAM();
        break;
    case 2:
        CABtest_rt_TCAM();
        break;
    case 3:
        CNORtest_rt_TCAM();
        break;
    default:
        return;
    }
}

addr_5tup OFswitch_CB::formatConvert(string str)
{
    addr_5tup packet;
    vector<string> temp;
    boost::split(temp, str, boost::is_any_of("\t"));
    packet.addrs[0]=boost::lexical_cast<uint32_t>(temp[0]);
    packet.addrs[1]=boost::lexical_cast<uint32_t>(temp[1]);
    packet.addrs[2]=0;
    packet.addrs[3]=0;
    packet.proto=true;
    return packet;
}

void OFswitch_CB::flowInfomation() {
    fs::path ref_trace_path(traceFile);
    if (!(fs::exists(ref_trace_path) && fs::is_regular_file(ref_trace_path))) {
        cout<<"Missing Ref file"<<endl;
        return;
    }

    double curT = 0;
    boost::unordered_set<addr_5tup> flow_rec;
    ofstream out(statFile,std::ios::app);
    uint32_t packetCount=0;

    try {
        ifstream trace_stream(traceFile);
        string str;
        while(getline(trace_stream, str)) {
            addr_5tup packet=formatConvert(str);
            packetCount++;
            curT=curT+0.0001;
            flow_rec.insert(packet);
        }
        trace_stream.close();
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }
    cout<<"totoal packet no:"<<packetCount<<endl;
    out<<"TCAM capacity:"<<TCAMcap<<endl;
    out<<"simulation time:"<<simuT<<endl;
    out<<"totoal packet no:"<<packetCount<<endl;
    out<<"totoal flow no:"<<flow_rec.size()<<endl;
    out.close();
}

void OFswitch_CB::CEMtest_rt_TCAM() {
    fs::path ref_trace_path(traceFile);
    if (!(fs::exists(ref_trace_path) && fs::is_regular_file(ref_trace_path))) {
        cout<<"Missing Ref file"<<endl;
        return;
    }

    double curT = 0;
    lru_cache<addr_5tup> cam_cache(TCAMcap, simuT);
    boost::unordered_set<addr_5tup> flow_rec;
    ofstream out(statFile,std::ios::app);
    uint32_t packetCount=0;

    try {
        ifstream trace_stream(traceFile);
        string str;
        while(getline(trace_stream, str)) {
            addr_5tup packet=formatConvert(str);
            packetCount++;
            curT=curT+0.0001;
            auto res = flow_rec.insert(packet);
            cam_cache.ins_rec(packet, curT, res.second);
            // if(packetCount%100==0){
            //     cout<<packetCount<<" tcam use: "<< cam_cache.cache_miss << endl;
            //     if(cam_cache.cache_miss> TCAMcap){
            //         break;
            //     }
            // }

        }
        trace_stream.close();
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }

    out<<endl<<"the CEM effect:"<<endl;
    out<<"flow no: "<<flow_rec.size()<<endl;
    out<<"cache miss no: "<<cam_cache.cache_miss<<endl;
    out<<"reuse entry no: "<<cam_cache.reuse_count<<endl;
    out.close();
}

void OFswitch_CB::CABtest_rt_TCAM() {
    fs::path ref_trace_path(traceFile);
    if (!(fs::exists(ref_trace_path) && fs::is_regular_file(ref_trace_path))) {
        cout<<"Missing Ref file"<<endl;
        return;
    }

    double curT = 0;
    lru_cache_cab cam_cache(TCAMcap, simuT);
    boost::unordered_set<addr_5tup> flow_rec;
    ofstream out(statFile,std::ios::app);
    uint32_t packetCount=0;

    try {
        ifstream trace_stream(traceFile);
        string str; 
        while(getline(trace_stream, str)) {
            addr_5tup packet=formatConvert(str);
            packetCount++;
            curT=curT+0.0001;
            auto res = flow_rec.insert(packet);
            bucket* buck = bTree->search_bucket(packet).first;
            if (buck!=NULL){
                cam_cache.ins_rec(buck, curT, res.second);
            } 
            else{
                cout<<"null bucket" <<endl;
            }
            // if(packetCount % 1000==0){
            //     auto tcamUseInfo=cam_cache.getTcamUseInfo();
            //     uint32_t tcamUseCount=tcamUseInfo.first+tcamUseInfo.second ;
            //     cout<<packetCount<<" tcam use: "<< tcamUseCount << endl;
            //     if(tcamUseCount >= TCAMcap *0.92){
            //         break;
            //     }
            // }

        }
        trace_stream.close();
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }
    out<<endl<<"the CAB effect:"<<endl;
    out<<"cache miss no: "<<cam_cache.cache_miss<<endl;
    auto tcamUseInfo=cam_cache.getTcamUseInfo();
    out<<"current tcam bucket no: "<<tcamUseInfo.first<<endl;
    out<<"current tcam rule no: "<<tcamUseInfo.second<<endl;
    out<<"rule download count no: "<<cam_cache.rule_down_count<<endl;
    out<<"reuse entry no: "<<cam_cache.reuse_count<<endl;
    out.close();
}

void OFswitch_CB::CNORtest_rt_TCAM() {
    fs::path ref_trace_path(traceFile);
    if (!(fs::exists(ref_trace_path) && fs::is_regular_file(ref_trace_path))) {
        cout<<"Missing Ref file"<<endl;
        return;
    }

    double curT = 0;
    lru_cache<uint32_t> cam_cache(TCAMcap, simuT);
    boost::unordered_set<addr_5tup> flow_rec;
    ofstream out(statFile,std::ios::app);
    uint32_t packetCount=0;

    try {
        ifstream trace_stream(traceFile);
        string str;
        while(getline(trace_stream, str)) {
            addr_5tup packet=formatConvert(str);
            packetCount++;
            curT=curT+0.0001;
            auto res = flow_rec.insert(packet);
            int ruleIndex=sep->searchIndepIndex(packet);
            if(ruleIndex>=0){
                cam_cache.ins_rec(ruleIndex, curT, res.second);
            }
            else{
                cout<<"null rules"<<endl;
            }

            if(packetCount%10000==0){
                cout<<packetCount<<" cache miss "<<cam_cache.cache_miss<<endl;
                // if(cam_cache.cache_miss > TCAMcap ){
                //     break;
                // }
            }

        }
        trace_stream.close();
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }

    out<<endl<<"the CNOR effect:"<<endl;
    out<<"cache miss no: "<<cam_cache.cache_miss<<endl;
    out<<"reuse entry no: "<<cam_cache.reuse_count<<endl;
    out.close();
}



