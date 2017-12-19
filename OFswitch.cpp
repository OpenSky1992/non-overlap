#include "OFswitch.h"

namespace fs = boost::filesystem;
namespace io = boost::iostreams;

using std::ifstream;
using std::ofstream;

/* Constructor:
 *
 * method brief:
 */

OFswitch::OFswitch(string trace,string statistics) {
    rList = NULL;
    bTree = NULL;
    sep=NULL;

    mode = 0;
    simuT = 100;
    TCAMcap = 4000;
    traceFile = trace;
    statFile = statistics;
}

void OFswitch::run_test() {
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

void OFswitch::flowInfomation() {
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
        io::filtering_istream trace_stream;
        trace_stream.push(io::gzip_decompressor());
        ifstream trace_file(traceFile);
        trace_stream.push(trace_file);

        string str;
        while(getline(trace_stream, str)) {
            addr_5tup packet(str);
            curT = packet.timestamp;
            packetCount++;
            flow_rec.insert(packet);
            if (curT > simuT)
                break;
        }
        io::close(trace_stream);
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }
    cout<<"totoal packet no:"<<packetCount<<endl;
    out<<"TCAM capacity: "<<TCAMcap<<endl;
    out<<"simulation time: "<<simuT<<" second"<<endl;
    out<<"totoal packet no: "<<packetCount<<endl;
    out<<"totoal flow no:"<<flow_rec.size()<<endl;
    out.close();
}

void OFswitch::CEMtest_rt_TCAM() {
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
        io::filtering_istream trace_stream;
        trace_stream.push(io::gzip_decompressor());
        ifstream trace_file(traceFile);
        trace_stream.push(trace_file);

        string str;
        while(getline(trace_stream, str)) {
            addr_5tup packet(str);
            curT = packet.timestamp;
            packetCount++;
            auto res = flow_rec.insert(packet);
            cam_cache.ins_rec(packet, curT, res.second);
            // if(packetCount%100==0){
            //     cout<<packetCount<<" tcam use: "<< cam_cache.cache_miss << endl;
            //     if(cam_cache.cache_miss> TCAMcap){
            //         break;
            //     }
            // }

            if (curT > simuT)
                break;
        }
        io::close(trace_stream);
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }

    out<<endl<<"the CEM effect:"<<endl;
    out<<"cache miss no: "<<cam_cache.cache_miss<<endl;
    // out<<"reuse entry no: "<<cam_cache.reuse_count<<endl;
    out.close();
}

void OFswitch::CABtest_rt_TCAM() {
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
        io::filtering_istream trace_stream;
        trace_stream.push(io::gzip_decompressor());
        ifstream infile(traceFile);
        trace_stream.push(infile);

        string str; 
        while(getline(trace_stream, str)) {
            addr_5tup packet(str);
            curT = packet.timestamp;
            packetCount++;
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

            if (curT > simuT)
                break;
        }
        io::close(trace_stream);
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }
    out<<endl<<"the CAB effect:"<<endl;
    out<<"bucket size: "<<bTree->threshold<<endl;
    out<<"cache miss no: "<<cam_cache.cache_miss<<endl;
    auto tcamUseInfo=cam_cache.getTcamUseInfo();
    out<<"current tcam bucket no: "<<tcamUseInfo.first<<endl;
    out<<"current tcam rule no: "<<tcamUseInfo.second<<endl;
    out<<"rule download count no: "<<cam_cache.rule_down_count<<endl;
    // out<<"reuse entry no: "<<cam_cache.reuse_count<<endl;
    out.close();
}

void OFswitch::CNORtest_rt_TCAM() {
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
        io::filtering_istream trace_stream;
        trace_stream.push(io::gzip_decompressor());
        ifstream trace_file(traceFile,std::ios::app);
        trace_stream.push(trace_file);

        string str;
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

            // if(packetCount%10000==0){
            //     cout<<packetCount<<" cache miss "<<cam_cache.cache_miss<<endl;
            //     // if(cam_cache.cache_miss > TCAMcap ){
            //     //     break;
            //     // }
            // }

            if (curT > simuT)
                break;
        }
        io::close(trace_stream);
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<endl;
    }

    out<<endl<<"the CNOR effect:"<<endl;
    out<<"cache miss no: "<<cam_cache.cache_miss<<endl;
    // out<<"reuse entry no: "<<cam_cache.reuse_count<<endl;
    out.close();
}



