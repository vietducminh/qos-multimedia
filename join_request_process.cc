// join_request_process.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "join_request_process.hh"

using namespace std;

CLICK_DECLS

JoinRequestProcess::JoinRequestProcess()
{} 

JoinRequestProcess::~JoinRequestProcess()
{}

int JoinRequestProcess::initialize(ErrorHandler *) 
{
     	return 0;
}

void JoinRequestProcess::uninitialize(){
}

int JoinRequestProcess::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,
		"NEIGHBOR_INFOBASE",0,cpElement,&_neighborInfo,
		"TOPOLOGY_INFOBASE",0,cpElement,&_topologyInfo,
		"LINEAR_IP_LOOKUP",0,cpElement,&_linearIPlookup,

		cpEnd) < 0) 
	{
		cout << "This is error from configure";
		return -1;
	
	}

	return 0;

}

void JoinRequestProcess::push(int port, Packet *p)
{
	assert(port == 0);
	WritablePacket *packet = p->uniqueify();
	assert(packet);

	assert(packet->has_network_header());
	//click_ip *ip = packet->ip_header();
	click_ip *ip = reinterpret_cast<click_ip *> (packet->data());
	click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);
	request_pkt_hdr *request_hdr = reinterpret_cast<request_pkt_hdr *> (udp + 1);
	int flowid = request_hdr->flowid;
	IPAddress _receiverAddr = IPAddress(ip->ip_src); 
	//click_chatter("\n receiver_addr: %s",_receiverAddr.unparse().c_str());
	//click_chatter("source_addr: %s\n",request_hdr->source_addr.unparse().c_str());


	HashTable<int, flow_member_data*> * _fTable = _flowMemberInfo->get_flow_member_set();

	// add the joiner as a receiver (with status JOIN_PROBE) in FlowMemberInfoBase
	if(_fTable->size() == 0){
		//click_chatter("_fTable is NULL");
		flow_member_data* temp = new flow_member_data;
		temp->flowid = flowid;
		temp->source_addr = _myAddr;
		receiver_status rs;
		rs.receiver_addr = _receiverAddr;
		rs.status = MCOM_STATUS_JOIN_PROBE;
		temp->record.push_back(rs);
		_fTable->find_insert(flowid, temp);
		//click_chatter("flowid and receiver have been inserted");
	}
	else {
		//click_chatter("_fTable is not NULL");
		HashTable<int, flow_member_data*>::iterator it = _fTable->find(flowid);
		if(it == NULL){
			click_chatter("there is no flow with flowid");
			flow_member_data* temp = new flow_member_data;
			temp->flowid = flowid;
			temp->source_addr = _myAddr;
			receiver_status rs;
			rs.receiver_addr = _receiverAddr;
			rs.status = MCOM_STATUS_JOIN_PROBE;
			temp->record.push_back(rs);
			_fTable->find_insert(flowid, temp);
		}
		else{
			flow_member_data* fd = _fTable->get(flowid);
			receiver_status rs;
			rs.receiver_addr = _receiverAddr;
			rs.status = MCOM_STATUS_JOIN_PROBE;
			fd->record.push_back(rs);
			//Vector<struct receiver_status>* rt = &(fd->record);
			//int size = (fd->record).size();
			//for(int i=0; i<size; i++){
			//	if(_receiverAddr == fd->record[i].receiver_addr)	
			//		fd->record[i].status = MCOM_STATUS_JOIN_PROBE;
			//}
		}
	}
	

	Vector<IPAddress> addrlist; 
	addrlist.push_back(_receiverAddr);

	//click_chatter("fTable size: %d",_fTable->size());

  	for (HashTable<int, flow_member_data*> ::iterator it = _fTable->begin(); it; ++it){
		//click_chatter("record size: %d",it.value()->record.size());
		//click_chatter("flowid: %d",it.key());
		//click_chatter ( "%s\n", it.value()->source_addr.unparse().c_str() );
	}

	// check whether the joiner connected to a last-MPR with more than a threshold receivers or not

	if(_fTable->size() != 0){
		//click_chatter("fTable is not NULL");
		HashTable<int, flow_member_data*>::iterator it = _fTable->find(flowid);
		if(it != NULL){	
			flow_member_data* fd = _fTable->get(flowid);
			for(int i=0; i<fd->record.size(); i++){
				addrlist.push_back(fd->record[i].receiver_addr);
				//click_chatter("Join request process 3");
			}
		}
	}
	
	HashTable<IPAddress,Vector<IPAddress> > lastMPRDestMap;
	Vector<IPAddress>::iterator iter = addrlist.begin();

	while(iter != addrlist.end() ){	// if node is source, the receiver which is 1-hop of source is excluded from addrlist
		IPAddress dest = *iter;		
		if(is_neighbor(dest) == true){
			iter = addrlist.erase(iter);
			continue;
		}
		++iter;
	}

	lastMPRDestMap = build_lastMPRDestMap(addrlist);
	bool is_unicast = is_unicasted_receiver(lastMPRDestMap, _receiverAddr);

	if(is_unicast == false){ // if this is not unicasted receiver (it means this node receives packet by SMF), 
				//source will send a probe reply to the receiver
				// the joiner will be added to the FlowMemberInfoBase
		send_probe_reply(flowid,_receiverAddr);

	}
			// if this receiver is unicasted receiver
	// the JOIN_PROBE flag will be set in the packet
	// on reception, the graft node that unicasts this packet to the receiver will set the packet as shadow class 
		
	output(1).push(packet);
	
}

void
JoinRequestProcess::send_probe_reply(int flowid, IPAddress _receiverAddr)
{
	int packet_size = sizeof(click_ip) + sizeof(click_udp) + sizeof(join_probe_reply_pkt_hdr);
	int headroom = sizeof(click_ether);
	int tailroom = 0; 
	WritablePacket *packet = Packet::make(headroom,0,packet_size, tailroom);
	if ( packet == 0 ){
		click_chatter( "in %s: cannot make packet!", name().c_str());
	}
	memset(packet->data(), 0, packet->length());
	struct timeval tv;
	click_gettimeofday(&tv);
	packet->set_timestamp_anno(tv);
	click_ip *ip = reinterpret_cast<click_ip *>(packet->data());
	click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);

	join_probe_reply_pkt_hdr *jp_reply_hdr = (join_probe_reply_pkt_hdr *) (udp + 1);
	jp_reply_hdr->flowid = flowid;
	jp_reply_hdr->receiver_addr = _receiverAddr;
	jp_reply_hdr->data_status = MCOM_STATUS_PROBE;	

#if !HAVE_INDIFFERENT_ALIGNMENT
	assert((uintptr_t)ip % 4 == 0);
#endif
	// set up IP header
	ip->ip_v = 4;
	ip->ip_hl = sizeof(click_ip) >> 2;
	ip->ip_len = htons(packet->length());
	ip->ip_id = htons(_id.fetch_and_add(1));
	ip->ip_p = IP_PROTO_UDP;
	ip->ip_src = _myAddr;
	ip->ip_dst = _receiverAddr;
	packet->set_dst_ip_anno(IPAddress(_receiverAddr));
 
	ip->ip_tos = 0;
	ip->ip_off = 0;
	ip->ip_ttl = 250;

	ip->ip_sum = 0;
#if HAVE_FAST_CHECKSUM && FAST_CHECKSUM_ALIGNED
	if (_aligned)
		ip->ip_sum = ip_fast_csum((unsigned char *)ip, sizeof(click_ip) >> 2);
	else
		ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));
#elif HAVE_FAST_CHECKSUM
		ip->ip_sum = ip_fast_csum((unsigned char *)ip, sizeof(click_ip) >> 2);
	#else
		ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));
	#endif
		packet->set_ip_header(ip, sizeof(click_ip));
		// set up UDP header
	udp->uh_sport = 2011;
	udp->uh_dport = 2011;
	uint16_t len = packet->length() - sizeof(click_ip);
	udp->uh_ulen = htons(len);
	udp->uh_sum = 0;
	unsigned csum = click_in_cksum((unsigned char *)udp, len);
	udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
	
	output(0).push(packet);
}

// Building lastMPRDestMap: contain records (last MPR,receiver list)

HashTable<IPAddress,Vector<IPAddress> >
JoinRequestProcess::build_lastMPRDestMap(Vector <IPAddress> addrlist)
{
	int new_list_size = addrlist.size();	
	Vector<IPAddress> separatedReceivers; // if there is no MPR connected to a receiver, then it is included in this set

	HashTable<IPAddress,Vector<IPAddress> > lastMPRDestMap; // (last MPR, list of destination)
	HashMap<IPPair, void*> *topology_set = _topologyInfo->get_topology_set();

	Vector<IPAddress> temp;

	for(int i=0; i < new_list_size; i++){
		IPAddress dst;
		IPAddress key;
		dst = addrlist[i];
		bool bdst = false; // this variable is used to check whether there is one MPR connected to dst. 
				//If not (bdst == false), this dst will be directly included in separatedReceivers 

		//click_chatter ( "Dst: %s\n", dst.unparse().c_str() ); 
		//cout << "LastMPR 3" << endl;

		for ( HashMap<IPPair, void*>::iterator iter = topology_set->begin(); iter != topology_set->end(); iter++ ) {
			topology_data *topology = ( topology_data * ) iter.value();
			const IPRoute * dest_temp;
			const IPRoute * last_temp;

			dest_temp = _linearIPlookup->lookup_iproute( topology->T_dest_addr );
			last_temp = _linearIPlookup->lookup_iproute( topology->T_last_addr );
			if(dest_temp == NULL){
				cout << "Dest is null "  << endl;
				continue;
			}
			if(last_temp == NULL){
				cout << "Last is null "  << endl;
				continue;
			}
			if(topology->T_dest_addr == dst){
				bdst = true;
				key = topology->T_last_addr;
				temp.clear();	

				if (lastMPRDestMap.find(key) == NULL){
					temp.push_back(dst);
					lastMPRDestMap.find_insert(key,temp);
				}
				else{
					temp = lastMPRDestMap[key];
					temp.push_back(dst);
					lastMPRDestMap.replace(key,temp);
				}
				break; 
			}			
		}
		if(bdst == false){
			separatedReceivers.clear();	
			key = NULL;
			if (lastMPRDestMap.find(key) == NULL){
				separatedReceivers.push_back(dst);
				lastMPRDestMap.find_insert(key,separatedReceivers);
			}
			else{
				separatedReceivers = lastMPRDestMap[key];
				separatedReceivers.push_back(dst);
				lastMPRDestMap.replace(key,separatedReceivers);
			}
		}
	}
	return lastMPRDestMap;

}

// this function checks whether the joiner is receiver that does not connect to a last-MPR which has more than a threshold receiver
// 1 = packet will be unicasted to this receiver, 0 = packet will be broadcasted to this receiver by SMF

//Vector<IPAddress> 
//JoinRequestProcess::build_MPR_Dest_List(HashTable<IPAddress,Vector<IPAddress> > lastMPRDestMap, Vector<IPAddress> neighborReceivers)
bool
JoinRequestProcess::is_unicasted_receiver(HashTable<IPAddress,Vector<IPAddress> > lastMPRDestMap, IPAddress _receiverAddr)
{
	bool is_unicast = true;
	IPAddress temp;
	for(HashTable<IPAddress,Vector<IPAddress> >::iterator it = lastMPRDestMap.begin(); it; ++it){
		IPAddress hkey = it.key();
		Vector<IPAddress> temp_list = it.value();	
		for(int i=0; i < temp_list.size(); i++){
			if (_receiverAddr == temp_list[i]){
				if (temp_list.size() >= 2){
					is_unicast = false;
				}
			}
		}		
	}

	return is_unicast;
}

bool JoinRequestProcess::is_neighbor(IPAddress node)
{
	bool isNeighbor = false;
	neighbor_data * nd = _neighborInfo->find_neighbor(node);
	if(nd != 0){
		isNeighbor = true;	
	}
	return isNeighbor;

}


CLICK_ENDDECLS
EXPORT_ELEMENT(JoinRequestProcess)

