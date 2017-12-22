#include "stdafx.h"
#include "BucketTree.h"
#include "Separate.h"
#include "OFswitch.h"
#include "OFswitch_CB.h"
#include "TraceGen.h"


using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace fs = boost::filesystem;


void logging_init() {
    fs::create_directory("./log");
    logging::add_file_log
    (
        keywords::file_name = "./log/sample_%N.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::format = "[%TimeStamp%]: %Message%"
    );
}

void testDiffBucketSize()
{
    string metaDir="../metadata/";
    string rulefile = metaDir+"rule4000";
    rule_list rList(rulefile);
    uint32_t bucketSize=20;
    
    // generate bucket tree
    bucket_tree *bTree;    

    // simulation test
    string statistFile="../metadata/statistInfo";
    ofstream out(statistFile);
    out.close();
    //Trace_Generate_5/trace-1000k-0.05-200/ref_trace.gz
    OFswitch ofswitch("../traceGen/bucket-30k-0.05-200/ref_trace.gz",statistFile);
    ofswitch.TCAMcap=2000;
    ofswitch.simuT=1;
    ofswitch.rList=&rList;

    separate sep(rList);
    ofswitch.sep=&sep;

    ofswitch.mode=0;
    ofswitch.run_test();
    ofswitch.mode=1;
    ofswitch.run_test();

    ofswitch.mode=2;
    for(uint32_t i=1;i<=bucketSize;i++){
        cout<<i<<endl;
        bTree=new bucket_tree(rList, i);
        ofswitch.bTree=bTree;
        ofswitch.run_test();
        delete bTree;
        bTree=NULL;
    }

    ofswitch.mode=3;
    ofswitch.run_test();
}

void testDiffNorm()
{
    // string metaDir="../metadata/";
    string rulefile ="../metadata/rule4000";
    rule_list rList(rulefile);
    uint32_t bucketSize=10;
    // generate bucket tree
    bucket_tree *bTree=new bucket_tree(rList,bucketSize);
    // simulation test
    string statistFile="../metadata/statistInfo";
    ofstream out(statistFile);
    out.close();
    //Trace_Generate_5/trace-1000k-0.05-200/ref_trace.gz
    OFswitch ofswitch("../traceGen/bucket-30k-0.04-2000/ref_trace.gz",statistFile);
    ofswitch.TCAMcap=2000;
    ofswitch.simuT=1;
    ofswitch.rList=&rList;
    ofswitch.bTree=bTree;

    separate sep(rList);
    ofswitch.sep=&sep;

    ofswitch.mode=0;
    cout<<"info collect:"<<endl;
    ofswitch.run_test();
    ofswitch.mode=1;
    cout<<"CEM:"<<endl;
    ofswitch.run_test();
    ofswitch.mode=2;
    cout<<"CAB:"<<endl;
    ofswitch.run_test();
    ofswitch.mode=3;
    cout<<"non-overlap:"<<endl;
    ofswitch.run_test();
    delete bTree;
}


void traceGenByBucket()
{
    string metaDir="../metadata/";
    string rulefile = metaDir+"rule4000";
    rule_list rList(rulefile);
    uint32_t bucketSize=10;
    bucket_tree *bTree=new bucket_tree(rList,bucketSize); 

    bTree->print_tree(metaDir+"tree_pr.dat");
    tracer tGen(&rList,"../metadata/TracePrepare_config.ini");
    tGen.hotspot_prepare();
    tGen.pFlow_pruning_gen(false);
    delete bTree;
}

void traceGenIRS()
{
    string metaDir="../metadata/";
    string rulefile = metaDir+"rule4000";
    rule_list rList(rulefile);
    separate sep(rList);

    sep.printRule(metaDir+"non-overlap.dat");
    tracer tGen(&rList,"../metadata/TracePrepare_config.ini");
    tGen.hotspot_prepare_IRS();
    tGen.pFlow_pruning_gen(true);
}



int main() {
    srand(time(NULL));
    logging_init();

    // traceGenByBucket();
    //testDiffBucketSize();
    testDiffNorm();
    return 0;
}

    //generate rule set
    //string tracefile="/home/opensky/cab/data/caida/parsedTrace/equinix-chicago.dirA.20160317-130000.UTC.anon.pcap.gz";
    //ruleGen(tracefile,"../ruleGen/rule",4000);

    //test bucket search
    // string str = "0.00%2952790016%2258155530%4000%8000%6";
    // addr_5tup packet(str);
    // auto res = bTree.search_bucket(packet, bTree.root);
    // cout<<res.first->get_str()<<endl;


    
    //test correct 
    /*
    ifstream infile("ref_trace100k");
    string line;
    int testCount=0;
    while(getline(infile,line)){
        int listSearch,bucketSearch,indepSearch;
        addr_5tup packet(line);
        listSearch=rList.linear_search(packet);
        bucketSearch=(bTree.search_bucket(packet)).second;
        indepSearch=sep.searchOriginIndex(packet);
        if(bucketSearch!=listSearch){
            cout<<"bucket search error:"<<packet.str_easy_RW()<<endl;
            return 0;
        }
        if(indepSearch!=listSearch){
            cout<<"non-overlap search error:"<<packet.str_easy_RW()<<endl;
            return 0;
        }
        testCount++;
        if(testCount%10000==0)
            cout<<"current:"<<testCount/10000<<endl;
        //cout<<"search is correct:"<<packet.str_easy_RW()<<" "<<listSearch<<endl;
    }
    infile.close();
    */