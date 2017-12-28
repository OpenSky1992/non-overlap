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
using std::stringstream;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace fs = boost::filesystem;

string rulefile ="../metadata/ruleset/rule4000";
string tracefile="../traceGen/bucket-30k-0.04-20/ref_trace.gz";
//Trace_Generate_5/trace-1000k-0.05-200/ref_trace.gz

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
    rule_list rList(rulefile);
    const uint32_t bucketNum=6;
    uint32_t bucketSizeArr[]={1,5,10,15,20,25};
    
    // generate bucket tree
    bucket_tree *bTree;    

    // simulation test
    string statistFile="../metadata/statistInfo";
    ofstream out(statistFile);
    out.close();
    separate sep(rList);
    OFswitch ofswitch(tracefile,statistFile);
    ofswitch.TCAMcap=400;
    ofswitch.simuT=60;
    ofswitch.sep=&sep;
    ofswitch.rList=&rList;
    ofswitch.flowInfomation();
    
    
        
    
    for(uint32_t i=0;i<=16;i++){
        stringstream ss;
        ofswitch.TCAMcap=400+i*100;
        cout<<"TCAM:"<<ofswitch.TCAMcap<<endl;
        ss<<statistFile<<"_"<<ofswitch.TCAMcap;
        ofswitch.statFile=ss.str();
        ofswitch.CEMtest_rt_TCAM();

        for(uint32_t i=0;i<bucketNum;i++){
            cout<<"bucket size: "<<bucketSizeArr[i]<<endl;
            bTree=new bucket_tree(rList, bucketSizeArr[i]);
            ofswitch.bTree=bTree;
            ofswitch.CABtest_rt_TCAM();
            delete bTree;
            bTree=NULL;
        }
        ofswitch.CNORtest_rt_TCAM();
    }
}

void testDiffNorm()
{    
    rule_list rList(rulefile);
    uint32_t bucketSize=10;
    // generate bucket tree
    bucket_tree *bTree=new bucket_tree(rList,bucketSize);
    // simulation test
    string statistFile="../metadata/statistInfo";
    ofstream out(statistFile);
    out.close();
    
    OFswitch ofswitch(tracefile,statistFile);
    ofswitch.TCAMcap=1000;
    ofswitch.simuT=1;
    ofswitch.rList=&rList;
    ofswitch.bTree=bTree;
    separate sep(rList);
    ofswitch.sep=&sep;

    cout<<"info collect:"<<endl;
    ofswitch.flowInfomation();
    cout<<"CEM:"<<endl;
    ofswitch.CEMtest_rt_TCAM();
    cout<<"CAB:"<<endl;
    ofswitch.CABtest_rt_TCAM();
    cout<<"non-overlap:"<<endl;
    ofswitch.CNORtest_rt_TCAM();
    delete bTree;
}


void traceGen()
{
    rule_list rList(rulefile);
    uint32_t bucketSize=10;
    bucket_tree *bTree=new bucket_tree(rList,bucketSize); 

    bTree->print_tree("../metadata/tree_pr.dat");
    tracer tGen(&rList,"../metadata/TracePrepare_config.ini");
    tGen.hotspot_prepare();
    tGen.pFlow_pruning_gen(false);
    delete bTree;
}

int main() {
    srand(time(NULL));
    logging_init();

    // traceGen();
    testDiffBucketSize();
    // testDiffNorm();
    return 0;
}

/*
void traceGenIRS()
{
    rule_list rList(rulefile);
    separate sep(rList);

    sep.printRule("../metadata/non-overlap.dat");
    tracer tGen(&rList,"../metadata/TracePrepare_config.ini");
    tGen.hotspot_prepare_IRS();
    tGen.pFlow_pruning_gen(true);
}
*/
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