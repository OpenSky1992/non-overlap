#include "stdafx.h"
#include "BucketTree.h"
#include "TraceGen.h"
#include "parsePcap.h"
#include "Separate.h"


using std::cout;
using std::endl;
using std::ofstream;

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


int main() {
    srand (time(NULL));
    logging_init();
    string metaDir="../metadata/";
    string rulefile = metaDir+"rule4000";
    rule_list rList(rulefile);
    
    // generate bucket tree
    bucket_tree bTree(rList, 8);
    bTree.tree_depth = 0;
    bTree.cal_tree_depth(bTree.root);
    cout <<"bucket Tree depth:"<< bTree.tree_depth << endl;
    bTree.print_tree(metaDir+"tree_pr.dat");


    //trace generation
    // tracer tGen(&rList,"../metadata/TracePrepare_config.ini");
    // tGen.hotspot_prepare();
    // tGen.pFlow_pruning_gen(false);

    //test bucket search
    // string str = "0.00%2952790016%2258155530%4000%8000%6";
    // addr_5tup packet(str);
    // auto res = bTree.search_bucket(packet, bTree.root);
    // cout<<res.first->get_str()<<endl;

    //test parsing pcap file
    // parsePcap parser("/home/part2/cab/data","/home/part2/cab/parsedTrace2");
    // parser.parse_pcap_file_mp(0,19);

    //test separate rule
    separate sep(rList);
    sep.printRule(metaDir+"non-overlap");
    

    return 0;
}





