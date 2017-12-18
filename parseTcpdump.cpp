#include "stdafx.h"
#include "Address.hpp"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;

namespace io = boost::iostreams;


int main(int argc, char **argv)
{
	string sLine;
	getline(cin,sLine);

	string fileName(argv[1]);
	fileName=fileName+".gz";
	ofstream outfile(fileName);
	io::filtering_ostream out;
	out.push(io::gzip_compressor());
	out.push(outfile);

	while(!cin.eof())
	{
		vector<string> LineArr;
		string sec,usec;
		uint32_t IpSrcValue=0,IpDstValue=0,PortSrcValue=0,PortDstValue=0;
		boost::split(LineArr, sLine, boost::is_any_of(" "));
		if(LineArr[1]=="IP"){
			vector<string> timeArr;
			boost::split(timeArr, LineArr[0], boost::is_any_of("."));
			sec=timeArr[0];
			usec=timeArr[1];

			vector<string> ipSplit;
			uint32_t ValueTemp=0;
			boost::split(ipSplit, LineArr[2], boost::is_any_of("."));
			if(ipSplit.size()==5)	//make sure L4 exist
			{
				for(int i=0;i<4;i++){
					ValueTemp=(ValueTemp<<8)+boost::lexical_cast<uint32_t>(ipSplit[i]);
				}
				IpSrcValue=ValueTemp;
				PortSrcValue=boost::lexical_cast<uint32_t>(ipSplit[4]);		

				ipSplit.clear();
				ValueTemp=0;
				vector<string> cutColon;
				boost::split(cutColon, LineArr[4], boost::is_any_of(":"));
				boost::split(ipSplit, cutColon[0], boost::is_any_of("."));
				for(int i=0;i<4;i++){
					ValueTemp=(ValueTemp<<8)+boost::lexical_cast<uint32_t>(ipSplit[i]);
				}
				IpDstValue=ValueTemp;
				PortDstValue=boost::lexical_cast<uint32_t>(ipSplit[4]);

				out<<sec<<"%"<<usec<<"%"<<IpSrcValue<<"%"<<IpDstValue<<"%"<<PortSrcValue<<"%"<<PortDstValue<<endl;
			}
			//ICMP only have L3
		}
		getline(cin,sLine);
	}

	io::close(out);
	return 0;
}