#include "parsePcap.h"

typedef vector<fs::path> Path_Vec_T;

/* count_proc
 *
 * output: uint32_t: the count no. of processors, helps determine thread no.
 */
uint32_t count_proc() {
    ifstream infile ("/proc/cpuinfo");
    uint32_t counter = 0;
    for (string str; getline(infile,str); )
        if (str.find("processor\t") != string::npos)
            counter++;
    return counter;
}

void parsePcap::parse_pcap_file_mp(size_t min, size_t max) const {
    if (fs::create_directory(fs::path(parsed_pcap_dir)))
        cout << "creating" << parsed_pcap_dir <<endl;
    else
        cout << "exists: "<<parsed_pcap_dir<<endl;

    const size_t File_BLK = 3;
    size_t thread_no = count_proc();
    size_t block_no = (max-min + 1)/File_BLK;
    if (block_no * 3 < max-min + 1)
        ++block_no;

    if ( thread_no > block_no) {
        thread_no = block_no;
    }

    size_t task_no = block_no/thread_no;

    vector<string> to_proc;
    size_t thread_id = 1;
    vector< std::future<void> > results_exp;

    size_t counter = 0;
    fs::path dir(pcap_dir);
    if (fs::exists(dir) && fs::is_directory(dir)) {
        Path_Vec_T pvec;
        std::copy(fs::directory_iterator(dir), fs::directory_iterator(), std::back_inserter(pvec));
        std::sort(pvec.begin(), pvec.end());

        for (Path_Vec_T::const_iterator it (pvec.begin()); it != pvec.end(); ++it) {
            if (counter < min) {
                ++counter;
                continue;
            }
            if (counter > max)
                break;
            ++counter;

            if (to_proc.size() < task_no*File_BLK || thread_id == thread_no) {
                to_proc.push_back(it->string());
            } else {
                cout <<"thread " << thread_id << " covers : "<<endl;
                for (auto iter = to_proc.begin(); iter != to_proc.end(); ++iter) {
                    cout << *iter << endl;
                }

                results_exp.push_back(std::async(
                                          std::launch::async,
                                          &parsePcap::p_pf_st,
                                          this, to_proc,
                                          (thread_id-1)*task_no)
                                     );
                ++thread_id;
                to_proc.clear();
                to_proc.push_back(it->string());
            }
        }
    }

    cout <<"thread " << thread_id << " covers :" << endl;
    for (auto iter = to_proc.begin(); iter != to_proc.end(); ++iter) {
        cout << *iter << endl;
    }

    results_exp.push_back(std::async(
                              std::launch::async,
                              &parsePcap::p_pf_st,
                              this, to_proc,
                              (thread_no-1)*task_no)
                         );
    for (size_t i = 0; i < thread_no; ++i)
        results_exp[i].get();

    return;
}

void parsePcap::p_pf_st(vector<string> to_proc, size_t id) const {
    struct pcap_pkthdr header; // The header that pcap gives us
    const u_char *packet; // The actual packet

    pcap_t *handle;
    const struct sniff_ethernet * ethernet;
    const struct sniff_ip * ip;
    const struct sniff_tcp *tcp;
    uint32_t size_ip;
    uint32_t size_tcp;


    size_t count = 2;
    const size_t File_BLK = 3;

    stringstream ss;

    ss<<parsed_pcap_dir+"/packData";
    ss<<std::setw(3)<<std::setfill('0')<<id;
    ss<<"txt.gz";

    ofstream outfile(ss.str());
    cout << "created: "<<ss.str()<<endl;
    io::filtering_ostream out;
    out.push(io::gzip_compressor());
    out.push(outfile);

    for (size_t i = 0; i < to_proc.size(); ++i) {
        if (i > count) {
            out.pop();
            outfile.close();
            ss.str(string());
            ss.clear();
            ++id;
            ss<<parsed_pcap_dir+"/packData";
            ss<<std::setw(3)<<std::setfill('0')<<id;
            ss<<"txt.gz";
            outfile.open(ss.str());
            cout << "created: "<<ss.str()<<endl;
            out.push(outfile);
            count += File_BLK;
        }

        char errbuf[PCAP_ERRBUF_SIZE];
        handle = pcap_open_offline(to_proc[i].c_str(), errbuf);

        while (true) {
            packet = pcap_next(handle, &header);
            if (packet == NULL)
                break;

            ethernet = (struct sniff_ethernet*)(packet);
            int ether_offset = 0;
            if (ntohs(ethernet->ether_type) == ETHER_TYPE_IP) {
                ether_offset = 14;
            } else if (ntohs(ethernet->ether_type) == ETHER_TYPE_8021Q) {
                // here may have a bug
                ether_offset = 18;
            } else {
                continue;
            }

            ip = (struct sniff_ip*)(packet + ether_offset);
            size_ip = IP_HL(ip)*4;

            if (IP_V(ip) != 4 || size_ip < 20)
                continue;
            if (uint32_t(ip->ip_p) != 6)
                continue;

            tcp = (struct sniff_tcp*)(packet + ether_offset + size_ip);
            size_tcp = TH_OFF(tcp)*4;
            if (size_tcp < 20)
                continue;

            stringstream ss;
            ss<<header.ts.tv_sec<<'%'<<header.ts.tv_usec<<'%';
            ss<<ntohl(ip->ip_src.s_addr)<<'%'<<ntohl(ip->ip_dst.s_addr);
            ss<<'%'<<tcp->th_sport<<'%'<<tcp->th_dport;

            out<<ss.str()<<endl;
        }

        pcap_close(handle);
        cout << "finished_processing : "<< to_proc[i] << endl;
    }
    io::close(out);
}