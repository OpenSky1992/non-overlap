#include "TraceGen.h"

using std::string;
using std::vector;
using std::pair;
using std::list;
using std::set;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::thread;
using std::atomic_uint;
using std::atomic_bool;
using std::mutex;



typedef boost::unordered_map<addr_5tup, uint32_t>::iterator Map_iter;
typedef vector<fs::path> Path_Vec_T;

/* ---------------------------- tgen_para ----------------------- */
tgen_para::tgen_para() {
    /* default para settings */
    flow_rate = 100;
    simuT = 1800;

    cold_prob = 0;
    hot_rule_thres = 6;
    hot_candi_no = 40;
    hotspot_no = 10;
    hotvtime = 10;

    mut_scalar[0] = 4;
    mut_scalar[1] = 1;

    evolving = false;
    evolving_time = 20;
    evolving_no = 10;

    prep_mutate = false;

    flow_rate_step = 10; 
    cold_prob_step = 0;
    hotspot_no_step = 0;
}

tgen_para::tgen_para(const tgen_para & another_para) {
    this->simuT = another_para.simuT;
    this->flow_rate = another_para.flow_rate;
    this->cold_prob = another_para.cold_prob;
    this->hotcandi_str = another_para.hotcandi_str;
    this->hotspot_ref = another_para.hotspot_ref;

    for (int i = 0; i < 4; ++i)
        this->scope[i] = another_para.scope[i];

    this->mut_scalar[0] = another_para.mut_scalar[0];
    this->mut_scalar[1] = another_para.mut_scalar[1];

    this->prep_mutate = another_para.prep_mutate;

    this->hot_rule_thres = another_para.hot_rule_thres;
    this->hot_candi_no = another_para.hot_candi_no;

    this->hotvtime = another_para.hotvtime;
    this->evolving_time = another_para.evolving_time;
    this->evolving_no = another_para.evolving_no;

    this->trace_root_dir = another_para.trace_root_dir;
    this->flowInfoFile_str = another_para.flowInfoFile_str;
    this->pcap_dir = another_para.pcap_dir;
    this->parsed_pcap_dir = another_para.parsed_pcap_dir;
}

tgen_para::tgen_para(string config_file):tgen_para() {
    /* read config file */
    ifstream config_stream(config_file);
    string config_line;

    while (getline(config_stream, config_line)) {
        vector<string> tmp_arr;
        boost::split(tmp_arr, config_line, boost::is_any_of(" \t"),
                     boost::token_compress_on);

        if (tmp_arr.size() >= 2) {
            /* this is a comment */
            if (tmp_arr[0][0] == '#')
                continue;

            if (tmp_arr[0] == "gen_root_dir") {
                trace_root_dir = tmp_arr[1];
                continue;
            }

            if (tmp_arr[0] == "meta_dir") {
                flowInfoFile_str = tmp_arr[1] + "/flow_info";
                hotcandi_str = tmp_arr[1] + "/hotspot.dat";
                hotspot_ref = tmp_arr[1] + "/tree_pr.dat";
                continue;
            }

            if (tmp_arr[0] == "flow_arrival_rate") {
                flow_rate = boost::lexical_cast<double>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "trace_len") {
                simuT = boost::lexical_cast<double>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "cold_probability") {
                cold_prob = boost::lexical_cast<double>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "hotspot_number") {
                hotspot_no = boost::lexical_cast<uint32_t>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "scope" && tmp_arr.size() >= (number_prefix+1)) {
                for(unsigned int i = 0; i < number_prefix; ++i) {
                    scope[i] = boost::lexical_cast<uint32_t>(tmp_arr[i+1]);
                }
                continue;
            }

            if (tmp_arr[0] == "mutate_scale" && tmp_arr.size() >= 3) {
                for (int i = 0; i < 2; ++i) {
                    mut_scalar[i] = boost::lexical_cast<uint32_t>(tmp_arr[i+1]);
                }
                continue;
            }

            if (tmp_arr[0] == "mutate_at_hot_prep") {
                if (tmp_arr[1] == "true")
                    prep_mutate = true;
                continue;
            }

            if (tmp_arr[0] == "hot_rule_size_thres") {
                hot_rule_thres = boost::lexical_cast<uint32_t>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "hotspot_candidate_no") {
                hot_candi_no = boost::lexical_cast<uint32_t>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "hotspot_arrival_time") {
                hotvtime = boost::lexical_cast<double>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "evolving") {
                if (tmp_arr[1] == "true")
                    evolving = true;
            }
            if (tmp_arr[0] == "evolving_time") {
                evolving_time = boost::lexical_cast<double>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "evolving_no") {
                evolving_no = boost::lexical_cast<size_t>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "origin_trace_dir") {
                pcap_dir = tmp_arr[1];
                continue;
            }

            if (tmp_arr[0] == "parsed_origin_trace_dir") {
                parsed_pcap_dir = tmp_arr[1];
                continue;
            }

            if (tmp_arr[0] == "bulk_no") {
                bulk_no = boost::lexical_cast<int>(tmp_arr[1]);
                continue;
            }

            if (tmp_arr[0] == "flow_rate_step") {
                flow_rate_step = boost::lexical_cast<int>(tmp_arr[1]);
                continue;  
            }

            if (tmp_arr[0] == "cold_prob_step") {
                cold_prob_step = boost::lexical_cast<double>(tmp_arr[1]);
                continue;  
            }

            if (tmp_arr[0] == "hotspot_no_step") {
                hotspot_no_step = boost::lexical_cast<int>(tmp_arr[1]);
                continue;  
            }
        }
    }
}

/* tracer
 *
 * function brief:
 * constructor: set rule list and simulation time offset
 */
tracer::tracer() {
    rList = NULL;
    jesusBorn = EpochT(-1,0);
    para = tgen_para();
}

tracer::tracer(rule_list * rL, string para_file) {
    para = tgen_para(para_file);
    rList = rL;
    jesusBorn = EpochT(-1,0);

    this->para = para;
}

void tracer::print_setup() const {
    cout <<" ======= SETUP BRIEF: ======="<< endl;
    cout <<"(basic)"<<endl;
    cout <<"Avg flow arrival rate:\t\t"<<para.flow_rate<<endl;
    cout <<"Trace length:\t\t\t"<<para.simuT<<" sec"<<endl;
    cout <<"(locality)"<<endl;
    cout <<"Cold trace probability: \t"<<para.cold_prob <<endl;
    cout <<"Hot spot no: \t\t\t"<<para.hotspot_no <<endl;
    cout <<"Scope: \t\t\t\t"<<para.scope[0]<<":"<<para.scope[1];
    cout <<":"<<para.scope[2] << ":"<<para.scope[3]<<endl;
    cout <<"Mutate scaling parameter:\t"<<para.mut_scalar[0]<<":"<<para.mut_scalar[1]<<endl;
    cout <<"Mutate preparation: \t\t"<<para.prep_mutate<<endl;
    cout <<"Thres size for a hot rule: \t"<<para.hot_rule_thres<<endl;
    cout <<"No. of hot rule candidates: \t"<<para.hot_candi_no<<endl;
    cout <<"(dirs)"<<endl;
    cout <<"flow arrival info: \t\t" << para.flowInfoFile_str << endl;
    cout <<"hotspot candidates are in\t" << para.hotcandi_str<<endl;
    cout <<"parsed real trace: \t\t" << para.parsed_pcap_dir<<endl;
    cout <<"original real trace: \t\t" << para.pcap_dir<<endl;
    cout <<" =========== END ==========="<<endl;
}


/* get_proc_file
 *
 * input: string ref_trace_dir: real trace directory
 * output:vector <fs::ps> :     paths of the real trace files in need for process
 *
 * function_brief:
 * get the paths of real traces within the range of the simulation Time
 */
void tracer::get_proc_files () {
    // find out how many files to process
    fs::path dir(para.parsed_pcap_dir);

    if (fs::exists(dir) && fs::is_directory(dir)) {
        Path_Vec_T pvec;
        std::copy(fs::directory_iterator(dir), fs::directory_iterator(), std::back_inserter(pvec));
        std::sort(pvec.begin(), pvec.end());

        for (Path_Vec_T::const_iterator it (pvec.begin()); it != pvec.end(); ++it) {
            try {
                io::filtering_istream in;
                in.push(io::gzip_decompressor());
                ifstream infile(it->c_str());
                in.push(infile);
                string str;
                getline(in,str);

                if (jesusBorn < 0) { // init jesusBorn
                    EpochT time (str);
                    jesusBorn = time;
                }

                addr_5tup packet(str, jesusBorn); // readable

                if (packet.timestamp > para.simuT) {
                    io::close(in);
                    break;
                }

                to_proc_files.push_back(it->string());
                io::close(in);
            } catch(const io::gzip_error & e) {
                cout<<e.what()<<endl;
            }
        }
    }
}


/* merge_files
 *
 * input: string gen_trace_dir: the targetting dir
 *
 * function_brief:
 * Collects the gz traces with prefix "ptrace-";
 * Merge into one gz trace named "ref_trace.gz"
 */
void tracer::merge_files(string proc_dir) const {
    fs::path file (proc_dir + "/ref_trace.gz");
    if (fs::exists(file))
        fs::remove(file);

    for (uint32_t i = 0; ; ++i) {
        stringstream ss;
        ss<<proc_dir<<"/ptrace-";
        ss<<std::setw(3)<<std::setfill('0')<<i;
        ss<<".gz";
        fs::path to_merge(ss.str());
        if (fs::exists(to_merge)) {
            io::filtering_ostream out;
            out.push(io::gzip_compressor());
            ofstream out_file(proc_dir+"/ref_trace.gz", std::ios_base::app);
            out.push(out_file);
            cout << "Merging:" << ss.str()<<endl;
            io::filtering_istream in;
            in.push(io::gzip_decompressor());
            ifstream in_file(ss.str().c_str());
            in.push(in_file);
            io::copy(in, out);
            in.pop();
            fs::remove(to_merge);
            out.pop();
        } else
            break;
    }
}

/* hotspot_prob
 *
 * input: string sav_str: output file path
 *
 * function brief:
 * generate the hotspot candidate with some reference file and put them into a candidate file for later header mapping
 */
void tracer::hotspot_prepare() {
    uint32_t hs_count = 0;

    ofstream ff (para.hotcandi_str);
    vector <string> file;
    ifstream in (para.hotspot_ref);

    for (string str; getline(in, str); ) {
        vector<string> temp;
        boost::split(temp, str, boost::is_any_of("\t"));
        if (boost::lexical_cast<uint32_t>(temp.back()) > para.hot_rule_thres) {
            file.push_back(str);
        }
    }

    random_shuffle(file.begin(), file.end());
    vector<string>::iterator iter = file.begin();

    while (hs_count < para.hot_candi_no && iter < file.end()) {
        ff <<*iter<<endl;
        iter++;
        ++hs_count;
    }

    ff.close();
}

// ===================================== Trace Generation and Evaluation =========================

/* pFlow_pruning_gen
 *
 * input: string trace_root_dir: target directory
 *
 * function_brief:
 * wrapper function for generate localized traces
 */
void tracer::pFlow_pruning_gen(bool evolving) {
    // init processing file
    if (to_proc_files.size() == 0) {
        get_proc_files();
    }

    // create root dir
    fs::path dir(para.trace_root_dir);
    if (fs::create_directory(dir)) {
        cout<<"creating: " << dir.string()<<endl;
    } else {
        cout<<"exitst: "<<dir.string()<<endl;
    }

    /* get the arrival time of each flow. */
    cout << "Generating flow arrival file ... ..."<<endl;
    unordered_set<addr_5tup> flowInfo;
    if (fs::exists(fs::path(para.flowInfoFile_str))) {
        ifstream infile(para.flowInfoFile_str.c_str());
        for (string str; getline(infile, str);) {
            addr_5tup packet(str);
            if (packet.timestamp > para.simuT)
                continue;
            flowInfo.insert(packet);
        }
        infile.close();
        cout << "Warning: using old flowInfo file" <<endl;
    } else {
        flowInfo = flow_arr_mp();
        cout << "flowInfo file generated" <<endl;
    }

    // trace generated in format of  "trace-200k-0.05-20"
    stringstream ss;
    ss<<dir.string()<<"/trace-"<<para.flow_rate<<"k-"<<para.cold_prob<<"-"<<para.hotspot_no;
    gen_trace_dir = ss.str();

    if (evolving)
        flow_pruneGen_mp_ev(flowInfo);
    else
        flow_pruneGen_mp(flowInfo);
}


/* flow_pruneGen_mp
 * input: unordered_set<addr_5tup> & flowInfo : first packet count
 * 	  string ref_trace_dir : real trace directory
 * 	  string hotspot_candi : candidate hotspot no. generated
 *
 * function_brief:
 * prune the headers according arrival, and map the headers
 */
void tracer::flow_pruneGen_mp( unordered_set<addr_5tup> & flowInfo) const {
    if (fs::create_directory(fs::path(gen_trace_dir)))
        cout<<"creating: "<<gen_trace_dir<<endl;
    else
        cout<<"exists:   "<<gen_trace_dir<<endl;

    std::multimap<double, addr_5tup> ts_prune_map;
    for (unordered_set<addr_5tup>::iterator iter=flowInfo.begin();
            iter != flowInfo.end(); ++iter) {
        ts_prune_map.insert(std::make_pair(iter->timestamp, *iter));
    }
    cout << "total flow no. : " << ts_prune_map.size() <<endl;

    /* prepare hot spots */
    list<h_rule> hotspot_queue;
    ifstream in (para.hotcandi_str);

    for (uint32_t i = 0; i < para.hotspot_no; i++) {
        string line;
        if (!getline(in, line)) {
            in.clear();
            in.seekg(0, std::ios::beg);
        }
        h_rule hr(line, rList->list);
        hotspot_queue.push_back(hr);
    }

    /* every ten second tube pruning eccessive flows */
    boost::unordered_map<addr_5tup, pair<uint32_t, addr_5tup> > pruned_map;

    /* pruned_map   old_header-> (header_id, new_header) */

    const double smoothing_interval = 10.0;
    double next_checkpoint = smoothing_interval;
    double flow_thres = 10 * para.flow_rate;

    vector< addr_5tup > header_buf;
    header_buf.reserve(3000);
    uint32_t id = 0;
    uint32_t total_header = 0;

    double nextKickOut = para.hotvtime;

    for (auto iter = ts_prune_map.begin(); iter != ts_prune_map.end(); ++iter) {
        if (iter->first > next_checkpoint) {
            random_shuffle(header_buf.begin(), header_buf.end());
            uint32_t i = 0;

            for (i = 0; i < flow_thres && i < header_buf.size(); ++i) {
                addr_5tup header;

                if ((double) rand()/RAND_MAX < (1-para.cold_prob)) {
                    /* hot packets */
                    auto q_iter = hotspot_queue.begin();
                    advance(q_iter, rand() % para.hotspot_no);
                    header = q_iter->gen_header();
                } else {
                    /* cold packets */
                    header = rList->list[(rand()%(rList->list.size()))].get_random();
                }

                pruned_map.insert(std::make_pair(header_buf[i], std::make_pair(id, header)));

                ++id;
            }

            total_header += i;
            header_buf.clear();
            next_checkpoint += smoothing_interval;
        }

        header_buf.push_back(iter->second);

        if (iter->first > nextKickOut) {
            hotspot_queue.pop_front();
            string line;
            if (!getline(in, line)) {
                in.clear();
                in.seekg(0, std::ios::beg);
                getline(in, line);
            }
            h_rule hr (line, rList->list);
            hotspot_queue.push_back(hr);
            nextKickOut += para.hotvtime;
        }
    }

    cout << "after smoothing, average: " << double(total_header)/para.simuT <<endl;

    /* process using multi-thread; */

    vector< std::future<void> > results_exp;

    for(uint32_t file_id = 0; file_id < to_proc_files.size(); ++file_id) {
        results_exp.push_back(std::async(std::launch::async,
                                         &tracer::f_pg_st, this,
                                         to_proc_files[file_id],
                                         file_id, &pruned_map));
    }

    for (uint32_t file_id = 0; file_id < to_proc_files.size(); ++file_id) {
        results_exp[file_id].get();
    }

    cout<< "Merging Files... "<<endl;
    merge_files(gen_trace_dir);

    cout<<"Generation Finished. Enjoy :)" << endl;
    return;
}

void tracer::flow_pruneGen_mp_ev( unordered_set<addr_5tup> & flowInfo) const {
    if (fs::create_directory(fs::path(gen_trace_dir)))
        cout<<"creating: "<<gen_trace_dir<<endl;
    else
        cout<<"exists:   "<<gen_trace_dir<<endl;

    std::multimap<double, addr_5tup> ts_prune_map;
    for (unordered_set<addr_5tup>::iterator iter=flowInfo.begin(); iter != flowInfo.end(); ++iter) {
        ts_prune_map.insert(std::make_pair(iter->timestamp, *iter));
    }
    cout << "total flow no. : " << ts_prune_map.size() <<endl;

    // prepair hot spots
    vector<h_rule> hotspot_seed;
    ifstream in (para.hotcandi_str);

    for (string str; getline(in, str); ) {
        vector<string> temp;
        boost::split(temp, str, boost::is_any_of("\t"));

        if (boost::lexical_cast<uint32_t>(temp.back()) > para.hot_rule_thres) {
            h_rule hr(str, rList->list);
            hotspot_seed.push_back(hr);
        }
    }

    random_shuffle(hotspot_seed.begin(), hotspot_seed.end());
    if (para.hot_candi_no > hotspot_seed.size()) {
        cout<<"revert to: " << hotspot_seed.size() << " hotspots"<<endl;
    } else {
        hotspot_seed = vector<h_rule>(hotspot_seed.begin(), hotspot_seed.begin()+ para.hot_candi_no);
    }
    vector<h_rule> hotspot_vec;

    for (auto iter = hotspot_seed.begin(); iter != hotspot_seed.end(); ++iter) {
        h_rule hr = *iter;
        hr.mutate_pred(para.mut_scalar[0], para.mut_scalar[1]);
        hotspot_vec.push_back(hr);
    }

    list<h_rule> hotspot_queue;
    auto cur_hot_iter = hotspot_vec.begin() + para.hotspot_no;
    for (size_t i = 0; i < para.hotspot_no; i++) {
        h_rule hr = hotspot_vec[i];
        hotspot_queue.push_back(hr);
    }

    // smoothing every 10 sec, map the headers
    boost::unordered_map<addr_5tup, pair<uint32_t, addr_5tup> > pruned_map;
    const double smoothing_interval = 10.0;
    double next_checkpoint = smoothing_interval;
    double flow_thres = 10 * para.flow_rate;
    vector< addr_5tup > header_buf;
    header_buf.reserve(3000);
    uint32_t id = 0;
    uint32_t total_header = 0;
    double nextKickOut = para.hotvtime;
    double nextEvolving = para.evolving_time;

    for (auto iter = ts_prune_map.begin(); iter != ts_prune_map.end(); ++iter) {
        if (iter->first > next_checkpoint) {
            random_shuffle(header_buf.begin(), header_buf.end());
            uint32_t i = 0 ;
            for (i = 0; i < flow_thres && i < header_buf.size(); ++i) {
                addr_5tup header;
                if ((double) rand() /RAND_MAX < (1-para.cold_prob)) { // no noise
                    auto q_iter = hotspot_queue.begin();
                    advance(q_iter, rand()%para.hotspot_no);
                    header = q_iter->gen_header();
                } else {
                    header = rList->list[(rand()%(rList->list.size()))].get_random();
                }
                pruned_map.insert( std::make_pair(header_buf[i], std::make_pair(id, header)));
                ++id;
            }
            total_header += i;
            header_buf.clear();
            next_checkpoint += smoothing_interval;
        }
        header_buf.push_back(iter->second);

        if (iter->first > nextKickOut) {
            hotspot_queue.pop_front();
            if (cur_hot_iter == hotspot_vec.end())
                cur_hot_iter = hotspot_vec.begin();
            hotspot_queue.push_back(*cur_hot_iter);
            ++cur_hot_iter;
            nextKickOut += para.hotvtime;
        }

        if (iter->first > nextEvolving) {
            vector<int> choice;
            for (int i = 0; i < int(hotspot_vec.size()); ++i)
                choice.push_back(i);
            random_shuffle(choice.begin(), choice.end());
            for (int i = 0; i < int(para.evolving_no); ++i) {
                h_rule hr = hotspot_seed[choice[i]];
                hr.mutate_pred(para.mut_scalar[0], para.mut_scalar[1]);
                hotspot_vec[choice[i]] = hr;
            }
            nextEvolving += para.evolving_time;
        }
    }
    cout << "after smoothing, average: " << double(total_header)/para.simuT <<endl;

    // process using multi-thread;

    vector< std::future<void> > results_exp;

    for(uint32_t file_id = 0; file_id < to_proc_files.size(); ++file_id) {
        results_exp.push_back(std::async(std::launch::async, &tracer::f_pg_st, this, to_proc_files[file_id], file_id, &pruned_map));
    }

    for (uint32_t file_id = 0; file_id < to_proc_files.size(); ++file_id) {
        results_exp[file_id].get();
    }

    cout<< "Merging Files... "<<endl;
    merge_files(gen_trace_dir);

    cout<<"Generation Finished. Enjoy :)" << endl;
    return;
}

void tracer::f_pg_st(string ref_file, uint32_t id, boost::unordered_map<addr_5tup, pair<uint32_t, addr_5tup> > * map_ptr) const {
    cout << "Processing " << ref_file << endl;
    io::filtering_istream in;
    in.push(io::gzip_decompressor());
    ifstream infile(ref_file);
    in.push(infile);

    stringstream ss1;
    ss1 << gen_trace_dir<< "/ptrace-";
    ss1 << std::setw(3) << std::setfill('0')<<id;
    ss1 <<".gz";
    io::filtering_ostream out_loc;
    out_loc.push(io::gzip_compressor());
    ofstream outfile_gen (ss1.str().c_str());
    out_loc.push(outfile_gen);
    out_loc.precision(15);

    for (string str; getline(in, str); ) {
        addr_5tup packet (str, jesusBorn); // readable;
        if (packet.timestamp > para.simuT)
            break;
        auto iter = map_ptr->find(packet);
        if (iter != map_ptr->end()) {
            packet.copy_header(iter->second.second);
            out_loc << packet.str_easy_RW() << endl;
        }
    }
    cout << " Finished Processing " << ref_file << endl;
    in.pop();
    out_loc.pop();
}

/* flow_arr_mp
 * input: string ref_trace_dir: pcap reference trace
 * 	  string flow_info_str: output trace flow first packet infol
 * output:unordered_set<addr_5tup> : the set of first packets of all flows
 *
 * function_brief:
 * obtain first packet of each flow for later flow based pruning
 */
boost::unordered_set<addr_5tup> tracer::flow_arr_mp() const {
    cout << "Processing ... To process trace files " << to_proc_files.size() << endl;
    // process using multi-thread;
    vector< std::future<boost::unordered_set<addr_5tup> > > results_exp;
    for (uint32_t file_id = 0; file_id < to_proc_files.size(); file_id++) {
        results_exp.push_back(std::async(std::launch::async, &tracer::f_arr_st, this, to_proc_files[file_id]));
    }
    vector< boost::unordered_set<addr_5tup> >results;
    for (uint32_t file_id = 0; file_id < to_proc_files.size(); file_id++) {
        boost::unordered_set<addr_5tup> res = results_exp[file_id].get();
        results.push_back(res);
    }

    // merge the results;
    boost::unordered_set<addr_5tup> flowInfo_set;
    for (uint32_t file_id = 0; file_id < to_proc_files.size(); file_id++) {
        boost::unordered_set<addr_5tup> res = results[file_id];
        for ( boost::unordered_set<addr_5tup>::iterator iter = res.begin(); iter != res.end(); iter++) {
            auto ist_res = flowInfo_set.insert(*iter);
            if (!ist_res.second) { // update timestamp;
                if (iter->timestamp < ist_res.first->timestamp) {
                    addr_5tup rec = *ist_res.first;
                    rec.timestamp = iter->timestamp;
                    flowInfo_set.insert(rec);
                }
            }
        }
    }

    // print the results;
    ofstream outfile(para.flowInfoFile_str);
    outfile.precision(15);
    for (boost::unordered_set<addr_5tup>::iterator iter = flowInfo_set.begin(); iter != flowInfo_set.end(); ++iter) {
        outfile<< iter->str_easy_RW() <<endl;
    }
    outfile.close();
    return flowInfo_set;
}

/* f_arr_st
 * input: string ref_file: trace file path
 * output:unordered_set<addr_5tup> : pairtial set of all arrival packet;
 *
 * function_brief:
 * single thread process of flow_arr_mp
 */
boost::unordered_set<addr_5tup> tracer::f_arr_st(string ref_file) const {
    cout<<"Procssing " << ref_file<< endl;
    boost::unordered_set<addr_5tup> partial_flow_rec;
    io::filtering_istream in;
    in.push(io::gzip_decompressor());
    ifstream infile(ref_file);
    in.push(infile);
    for (string str; getline(in, str); ) {
        addr_5tup packet(str, jesusBorn);
        if (packet.timestamp > para.simuT)
            break;
        partial_flow_rec.insert(packet);
    }
    io::close(in);
    cout<<"Finished procssing " << ref_file << endl;
    return partial_flow_rec;
}



