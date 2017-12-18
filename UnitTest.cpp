#include "stdafx.h"
#include "BucketTree.h"
#include "TraceGen.h"
#include "Separate.h"
#include "OFswitch.h"
#include "OFswitch_CB.h"


using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;


namespace fs = boost::filesystem;


int main() {
    srand (time(NULL));
    string metaDir="../metadata/";
    string rulefile = metaDir+"rule4000";
    rule_list rList(rulefile);
    // uint32_t bucketSize=1;
    
    // generate bucket tree
    // bucket_tree bTree(rList, bucketSize);
    // bTree.print_tree(metaDir+"tree_pr.dat");


    //trace generation
    tracer tGen(&rList,"../metadata/TracePrepare_config.ini");
    tGen.hotspot_prepare();
    tGen.pFlow_pruning_gen(false);

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

    // simulation test
    // string statistFile="../metadata/statistInfo";
    // ofstream out(statistFile);
    // out<<"bucket size :"<<bucketSize<<endl;
    // out.close();
    // //Trace_Generate_5/trace-1000k-0.05-200/ref_trace.gz
    // OFswitch_CB ofswitch("../metadata/ ",statistFile);
    // ofswitch.TCAMcap=4000;
    // ofswitch.simuT=50;
    // ofswitch.rList=&rList;
    // ofswitch.bTree=&bTree;

    // separate sep(rList);
    // ofswitch.sep=&sep;

    // ofswitch.mode=0;
    // ofswitch.run_test();
    // ofswitch.mode=1;
    // ofswitch.run_test();
    // ofswitch.mode=2;
    // ofswitch.run_test();
    // ofswitch.mode=3;
    // ofswitch.run_test();

    return 0;
}