// mymobifilter.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "mymobifilter.hh"
#include <click/etheraddress.hh>
#include <clicknet/ether.h>
#include <click/string.hh>
#include <click/vector.hh>


#include<iostream>
#include<fstream>
#include <string>

using namespace std;
 
CLICK_DECLS
MyMobiFilter::MyMobiFilter()
{}

MyMobiFilter::~ MyMobiFilter()
{}

int MyMobiFilter::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (cp_va_kparse(conf, this, errh, 
		"MY_ETHER_ADDRESS", 0, cpEtherAddress, &_myEtherAddr, 
		//"NODE_IP_ADDRESS_LIST", cpkP, cpIPAddressList, &_ipAddrList, 
		cpEnd) < 0) return -1;
	
	// Read the list of ether address of nodes from file
	std::ifstream ether_file;
          
	//ether_file.open ("/home/viet/testbed/mcom/ether_file.txt", std::ifstream::in);
	ether_file.open ("/home/viet/testbed/qos/ether_file.txt", std::ifstream::in);

	if(!ether_file)
		click_chatter("While opening a file an error is encountered\n");
	
	int num_node = 0; // the number of network nodes
	//ether_file >> num_node;	
	//string STRING;
	string STRING;

	//getline(ether_file,STRING);
	//num_node = (int)(STRING);
	//click_chatter("Number of nodes: %d", num_node);
	//const char *data;
	//const char data;

	EtherAddress tmp;
	bool convert;
	const EtherAddress context;	
	while(!ether_file.eof())
	{
		getline(ether_file,STRING);
		const char* cstr = STRING.c_str(); 	
		String click_string(cstr);

		//click_chatter("String of click_chatter: %s", STRING.c_str());
		//std::cin.getline(data,256);		
		//strcpy((char*) data, STRING);
		//data = STRING.c_str();	
		//EtherAddress tmp((const unsigned char)(data));
		//click_chatter("ether address: %s",data);
		EtherAddress result;	
		convert = cp_ethernet_address(click_string,&result);
		tmp = result; 	
		_etherAddrList.push_back(tmp);
	} 
	

	ether_file.close();

	return 0;
}

void MyMobiFilter::push(int, Packet *p)
{
	assert(p);
	WritablePacket *packet = p->uniqueify();
	assert(packet);

	HashTable<EtherAddress,int > node_index; // this variable is used to present the index of each node
						   // then nodes have index is 0,1,2,3...
						   // then the index is used in mobility matrix
								
	for(int i = 0; i < _etherAddrList.size(); i++)
	{
		EtherAddress tmp = _etherAddrList.at(i);
		node_index.set(tmp,i);
	} 

	// mobiMatrix is read from file "mobi_file.txt"
	
	//int mobiMatrix[20][20];
	std::ifstream mobi_file;
          
	//mobi_file.open ("/home/viet/testbed/mcom/mobi_file.txt", std::ifstream::in);
	mobi_file.open ("/home/viet/testbed/qos/mobi_file.txt", std::ifstream::in);

	if(!mobi_file)
		click_chatter("While opening a file an error is encountered\n");
	
	int num_node = 0; // the number of network nodes
	mobi_file >> num_node;
	while(!mobi_file.eof())
	{
		for( int i=0; i<num_node; i++)
			for( int j=0; j<num_node; j++)
			{
				mobi_file >> mobiMatrix[i][j];
				//cout << mobiMatrix[i][j] << endl;
			}
	} 

	mobi_file.close();

	const click_ether *e = (const click_ether *) (p->data());
	EtherAddress src(e->ether_shost); //source address of the packet p

	int _myEtherAddr_index = node_index[_myEtherAddr];
	int src_index = node_index[src];

	//click_chatter("myEtherAddr index: %d", _myEtherAddr_index);
	//click_chatter("source index: %d", src_index);
	//click_chatter("ether address: %s",src.unparse().c_str());
	if (mobiMatrix[_myEtherAddr_index][src_index] == 0){ //the node src is not neighbor of this node
		output(0).push(packet); // packet will be pushed to Discard element
	}
	else if (mobiMatrix[_myEtherAddr_index][src_index] == 1)
	{
		output(1).push(packet); // packet to be pushed for further processing
	}
	else 
		click_chatter("There is an error from MyMobiFilter");
	
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MyMobiFilter)
