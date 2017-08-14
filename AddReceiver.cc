// AddReceiver.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "AddReceiver.hh"

using namespace std;

CLICK_DECLS
AddReceiver::AddReceiver()
{} 

AddReceiver::~ AddReceiver()
{}

int AddReceiver::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
		"IP_ADDRESS_LIST", cpkP, cpIPAddressList, &_ipAddrList, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 		
		cpEnd) < 0) 
	{
		cout << "This is error from configure";
		return -1;
	
	}
	cout << "In configuration";
	return 0;

}

void AddReceiver::push(int port, Packet *p)
{
	//access the packet p to get receiver addresses
	
	//click_chatter("This is first line");
 
	assert(p);
	
	int num_bytes = 2 + 4 * _ipAddrList.size(); 
	int packet_size = p-> length() + sizeof(multicast_hdr);
	int headroom = sizeof( click_ether ) + sizeof( click_ip ) + sizeof( click_udp );
	int tailroom = 0;
	WritablePacket *packet = Packet::make( headroom, 0, packet_size, tailroom );
	if ( packet == 0 )
	{
		click_chatter( "in %s: cannot make packet!", name().c_str() );
	}
	memset( packet->data(), 0, packet->length() );

	
	multicast_hdr *mc = reinterpret_cast<multicast_hdr *> (packet->data());
	mc->protocolid = 1;
	mc->length = num_bytes;
	mc->size = _ipAddrList.size();

	addr_flag af;
	for(int i=0; i<_ipAddrList.size(); i++){
		af.addr = _ipAddrList[i];
		af.flag = 0;
		mc->array[i] = af;	
	}


	addr_flag tmp; 
	tmp.flag = 0;
	int pos = 0;
	//click_chatter("_ipAddrList vector size: %d", _ipAddrList.size());
	for(int i = 0; i < _ipAddrList.size(); i++)
	{

		//tmp.addr = *iter_addr;
		tmp.addr = _ipAddrList.at(i);
		tmp.flag = 0;
		mc->array[i]=tmp;	

	}
	
	//int size = mc->addr_vector.size();
	//click_chatter("size in AddReceiver: %d",size);
	output(0).push(packet);	
}

CLICK_ENDDECLS
EXPORT_ELEMENT(AddReceiver)

