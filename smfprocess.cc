// smfprocess.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "smfprocess.hh"
#define MAX 100

using namespace std;

CLICK_DECLS

SMFProcess::SMFProcess()
{}
SMFProcess::~ SMFProcess()
{}

int SMFProcess::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
	"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
	"OLSR_TCGENERATOR", 0, cpElement,&_tcGenerator,
	"LINEAR_IP_LOOKUP",0,cpElement,&_linearIPlookup,	
	"BUFFER",0,cpElement,&_buffer,
	"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,

	cpEnd) < 0) return -1;
	return 0;

}

void SMFProcess::push(int port, Packet *p)
{
	//click_chatter ( "%s\n", _myAddr.unparse().c_str() );

	assert(p);
	WritablePacket *packet = p->uniqueify();
	assert(packet);

	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp = reinterpret_cast<click_udp *> (ip + 1);

	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp +1); 
	
	int flowid = mulhdr->flowid;

	if(ip->ip_src == _myAddr){ // if this node is source of SMF packet, then the packet is duplicate
		click_chatter("This node is source of SMF packet");
		output(0).push(packet); // packet is sent to Discard element
		return;
	}
	int ttl = ip->ip_ttl;
	if(ttl < 1){
		click_chatter("Packet is expired, ttl is: %d", ttl);
		output(0).push(packet); // SMF packet is expired, it is sent to Discard element
		return;
	}
	String key = String(ip->ip_id); // ip_id equals to ipseq
	key += String(ip->ip_src.s_addr);
	key += String(flowid);

	//click_chatter ( "key: %s\n", key.c_str() );

	if (_buffer->table.find(key) != NULL){   //if the SMF packet is duplicate
		click_chatter("SMF packet is duplicated");
		output(0).push(packet); // send the packet to Discard element
		//cout << "SMF 4" << endl;
		return;
	}
	int distance_to_source;
	int distance_lastMPR_source;
	const IPRoute* nexthop;	
	nexthop = _linearIPlookup->lookup_iproute(ip->ip_src);
	
	
	//click_chatter ( "Binh 62");

	int id = mulhdr->protocolid;
	int len = mulhdr->length;
	//click_chatter("Protocolid: %d length: %d \n",id, len);

	Vector <addr_flag> addrlist;
	for(int i=0; i<mulhdr->size; i++){
		addrlist.push_back(mulhdr->array[i]);
	}

	int size = mulhdr->size;
	//click_chatter("Size: %d \n",size);	
	 
	// if this is a receiver for this flow and the packet is the first time, then packet is sent to upper layer
	bool is_receiver_for_flow = false;
	HashTable<int, flow_member_data*> * _fTable =  _flowMemberInfo->get_flow_member_set();
	HashTable<int, flow_member_data*>::iterator iter = _fTable->find(flowid);
	if(iter != NULL){
		flow_member_data* fd = _fTable->get(flowid);
		for(int i=0; i< fd->record.size(); i++){
			if(_myAddr == fd->record[i].receiver_addr){
				is_receiver_for_flow = true;
			}	
		}
	}

	if((ttl >= 1)&&(_buffer->table.find(key)== NULL) && (is_receiver_for_flow == true)){ 
		Packet* pk_0 = packet->clone();
		//click_chatter("smf_proc inside: packet is sent to upper layer");
		output(1).push(pk_0); // packet is sent to upper layer
	}
	
	// if node is MPR, the packet is the first time, ttl of SMF packet is greater than 1, 
	// the packet will be rebroadcast
	
	//if((_tcGenerator->_node_is_mpr)&&(buffer.find(key)== NULL) && ttl > 1)){
	if((_tcGenerator->_node_is_mpr)&&(_buffer->table.find(key)== NULL) && (ttl > 1)){
		ip->ip_ttl--;
		Packet* pk = packet->clone();
		output(2).push(pk);// rebroadcast (send to network interface)
		//unsigned long sum = (~ntohs(ip->ip_sum) & 0xFFFF) + 0xFEFF;
		//ip->ip_sum = ~htons(sum + (sum >> 16));
	}
	
	_buffer->addKnownPacket(key); // buffer for next time


}


CLICK_ENDDECLS
EXPORT_ELEMENT(SMFProcess)

