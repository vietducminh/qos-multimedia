// mcomprocess.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "mcomprocess.hh"

#include <iostream>
#include <fstream>
using namespace std;


//#define MAX 50

CLICK_DECLS
MCOMProcess::MCOMProcess()
{}

MCOMProcess::~ MCOMProcess()
{}

int MCOMProcess::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"LINEAR_IP_LOOKUP",0,cpElement,&_linearIPlookup,
		"TOPOLOGY_INFOBASE",0,cpElement,&_topologyInfo,
		"ROUTING_TABLE",0,cpElement,&_routingTable,
		"BUFFER",0,cpElement,&_buffer,
		"NEIGHBOR_INFOBASE",0,cpElement,&_neighborInfo,
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,

		cpEnd) < 0) return -1;
	return 0;
}

void MCOMProcess::push(int, Packet *p){
	//access the packet p to get receiver addresses

	//click_chatter ( "%s\n", _myAddr.unparse().c_str() );
	//_routingTable->print_routing_table();


	assert(p);
	WritablePacket *packet = p->uniqueify();
	assert(packet);

	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp = packet->udp_header();

	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp +1); 

	Vector <addr_flag> addrlist;
	int size = mulhdr->size;
	int protocolid = mulhdr->protocolid;
	_flowid = mulhdr->flowid;

	String key = String(ip->ip_id); // ip_id equals to ipseq
	key += String(ip->ip_src.s_addr);
	key+=String(_flowid);

	//_buffer->addKnownPacket(key); // buffer for next time

	bool node_is_receiver = false;

	//if((ip->ip_ttl >= 1)&&(_buffer->table.find(key)== NULL) && (node_is_receiver)){ 
	if((ip->ip_ttl >= 1) && (node_is_receiver)){ 

		Packet* pk_0 = packet->clone();
		output(1).push(pk_0); // packet is sent to upper layer
		double current_time = Timestamp::now().doubleval();
		//click_chatter("Packet is sent to upper layer, get time now: %llu", current_time);
		//click_chatter("\n");

		//cout << "SMF 3" << endl;
	}


	if(ip->ip_ttl < 1){ // TTL of the packet is less or equal 1, the packet is expired
		output(0).push(packet); // send packet to element Discard 
		return;
	}

	
	addrlist.clear();
	for(int i=0; i<size; i++){
		addr_flag af;
		af = mulhdr->array[i];
		addrlist.push_back(af);
	}
	//click_chatter("addrlist size: %d",addrlist.size());
	//click_chatter ( "Dest: %d\n", ip->ip_dst.s_addr );
	
	if(mulhdr->protocolid == 1){		
		// Extract receiver addresses of the packet
		if(ip->ip_src == _myAddr){ // if this node is source
			//click_chatter("This is source");
			sourceProcess(packet,addrlist);
		}
		else{   // this node is a forwarder/receiver
			forwarderProcess(packet,addrlist);	
		}	
	}	 
	else
		click_chatter("An error");	
	//_buffer->addKnownPacket(key); // buffer for next time	
}

bool MCOMProcess::is_neighbor(IPAddress node)
{
	bool isNeighbor = false;
	//HashMap<IPAddress, void *> * _neighborSet = _neighborInfo->get_neighbor_set();
	//HashMap<IPPair, void*> * _twohopSet = _neighborInfo->get_twohop_set();
	neighbor_data * nd = _neighborInfo->find_neighbor(node);

	//if(_neighborInfo->find_neighbor(node) != NULL){

	if(nd != 0){
		isNeighbor = true;	
		//click_chatter("is_neighbor");
	}
	return isNeighbor;

}

// process MCOM packets at source
void MCOMProcess::sourceProcess(WritablePacket* packet, Vector <addr_flag> addrlist)
{
	addrlist.clear();

	HashTable<int, flow_member_data*> * _fTable = _flowMemberInfo->get_flow_member_set();
	if(_fTable->size() != 0){
		//click_chatter("_fTable is not NULL");
		for (HashTable<int, flow_member_data*>::iterator it = _fTable->begin(); it; ++it){
			//click_chatter("flowid: %d",it.key());
		}
		
		flow_member_data* fd = _fTable->get(_flowid);
		if(fd != NULL){
			for(int i=0; i< fd->record.size(); i++){
				addr_flag temp;
				//click_chatter("\n temp addrlist: %s",temp.addr.unparse().c_str());

				temp.addr = fd->record[i].receiver_addr;
				addrlist.push_back(temp);
			}
		}
	}
	
	int size = addrlist.size();
	Vector<IPAddress> neighborReceivers;
	HashTable<IPAddress,Vector<IPAddress> > lastMPRDestMap;
	Vector<addr_flag> mpr_dest_vector;
	HashTable<IPAddress,Vector<addr_flag> > nextHopDestMap;

	/*
	click_chatter("Dump routes");
	click_chatter("%s\n", ((LinearIPLookup*)_linearIPlookup)->dump_routes().c_str());
	_linearIPlookup->dump_routes();
	*/

	//neighborReceivers.clear();
	Vector<addr_flag>::iterator iter = addrlist.begin();
	while(iter != addrlist.end() ){	// if node is source, the receiver which is 1-hop of source is excluded from addrlist
		//click_chatter("\n addrlist: %s",iter->addr.unparse().c_str());
		IPAddress dest = iter->addr;
		if(is_neighbor(dest) == true){
			neighborReceivers.push_back(dest);
			iter = addrlist.erase(iter);
			continue;
		}
		++iter;
	}
	
	lastMPRDestMap = build_lastMPRDestMap(addrlist);
	
	mpr_dest_vector = build_MPR_Dest_List(lastMPRDestMap,neighborReceivers);
	
	nextHopDestMap = build_nextHopDestMap(mpr_dest_vector);	
		
	source_send_packet(packet,nextHopDestMap);
}

// process MCOM packet at forwarders
void MCOMProcess::forwarderProcess(WritablePacket* packet, Vector <addr_flag> addrlist)
{
	click_ip *ip = packet->ip_header();
	click_udp *udp = packet->udp_header();
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp +1); 
	
	int list_size = addrlist.size();
	//cout << "List size: " << list_size << endl;
	Vector<addr_flag>::iterator iter = addrlist.begin();
	bool is_receiver = false;
	int index_node; // store index of this node (if it is a receiver) in addrlist
	
	int is_probe = mulhdr->is_probe;
	//click_chatter("is_probe is: %d",is_probe);

	for(int i=0; i < list_size; i++){	
	     if ((_myAddr == addrlist[i].addr)&& (addrlist[i].flag == MCOM_UNICASTED_RECEIVER_FLAG)){ // if this node is one of receiver and flag = 0 = MCOM_UNICASTED_RECEIVER_FLAG, 
						//a copy of packet (or packet) will be sent to higher layer of this node (push to ouput1). 
						// Also, this receiver address is removed from destination list
			
			if(list_size == 1){  // this node is only one receiver
				output(1).push(packet); // send packet to upper layer
			  	//click_chatter("Time sent to upper layer1: %f", Timestamp::now().doubleval());
				return;
				//Timestamp::now().doubleval();
			}
			else {					
				Packet *pk = packet->clone();
				output(1).push(pk); // send packet to upper layer
			  	//click_chatter("Time sent to upper layer2: %f", Timestamp::now().doubleval());
				//Timestamp::now().doubleval();
			}
			index_node = i;
			is_receiver = true;
		
			//click_chatter("mcomprocess - addr 1");

			//iter = addrlist.erase(iter);
			//click_chatter("mcomprocess - addr 2");
			//cout << "Test 5" << endl;
			break;					
		}
		
	}
	//iter = addrlist.begin();
	if(is_receiver == true){
		addrlist[index_node] = addrlist[list_size-1]; // the end element is copied in the place of inde_node
		addrlist.pop_back();
		list_size = addrlist.size();
	}
	//click_chatter("mcomprocess -  addr 3 - list_size: %d",list_size);

	is_receiver = false;
	if(list_size == 0){
		output(0).push(packet); // delete the packet
		return;
	}	

	for(int i=0; i < list_size; i++){
	
	//click_chatter("mcomprocess - addr 4");

	//	if ((_myAddr == iter->addr)&& (iter->flag == 2)){ //if this node is one of receiver and flag = 2, 
						// packet will be SMF: packet will be broadcast with TTL = 2
						// Also, this receiver address is removed from destination list
		
		if ((_myAddr == addrlist[i].addr)&& (addrlist[i].flag == MCOM_MPR_RECEIVER_FLAG)){ //if this node is one of receiver and flag = 2, 
						// packet will be SMF: packet will be broadcast with TTL = 2
						// Also, this receiver address is removed from destination list
	
			//click_chatter("mcomprocess - addr 5");

			//cout << "Test 7" << endl;
			ip->ip_ttl = 2;
			
			IPAddress addr_temp;
			
			ip->ip_dst == INADDR_BROADCAST;
			
			udp->uh_sport = (uint16_t)(1234);
			udp->uh_dport = (uint16_t)(1234);
	

			//cout <<"ip_dest: " << ip->ip_dst.s_addr<< endl;
				// delete all old addresses in multicast header and insert addresses of this node to the multicast header
				
			addr_flag af;
			const IPRoute* nexthop;
			nexthop = _linearIPlookup->lookup_iproute(ip->ip_src);
			
			//cout << "Forwarder 3" << endl;
			if(nexthop == NULL){
				cout << "Next hop is NULL" << endl;
				af.flag = 100;
			} 
			else{
				af.flag = nexthop->extra; // distance from last-MPR to source is recorded at the field "flag" for this packet
			}
			af.addr = _myAddr;

			//cout << "Forwarder 32" << endl;
			mulhdr->array[0]= af;
			mulhdr->size = 1;
		
			//cout << "Forwarder 4" << endl;
			mulhdr->protocolid == 1;						
			mulhdr->length = 6; // = 2+4
			mulhdr->size = 1;
			packet->set_dst_ip_anno(0xFFFFFFFFU);
			//ip->ip_src.s_addr = _myAddr;

			if(list_size == 1){  // this node is only one last-MPR
				//cout << "Forwarder 5" << endl;
				output(3).push(packet); // broadcast to others (send packet to the network interface)
				return;
			}
			else { // this node is both a last-MPR and a forwarder to others last-MPR/receivers
				//cout << "Forwarder 6" << endl;
				Packet *pk = packet->clone();
				output(3).push(pk); // broadcast to others
			}
			index_node = i;
			is_receiver = true;
			break;
			//iter = addrlist.erase(iter);	
		}
		
	}
	//click_chatter("mcomprocess - addr 6");

	if(is_receiver == true){
		addrlist[index_node] = addrlist[list_size-1];
		addrlist.pop_back();
	}

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
			
	//click_chatter("mcomprocess - addr 7");

	if(!addrlist.empty()){	// if addrlist is not empty, copies of the packet will be forwarded to next hops
		//forward(packet,addrlist);
		//click_chatter("mcomprocess - addr 8");

		HashTable<IPAddress,Vector<addr_flag> > nextHopDestMap;
	
		//cout << "Forwarder 9" << endl;
		nextHopDestMap = build_nextHopDestMap(addrlist);	
		//cout << "ForwarderProcess 1" << endl;
	
		forwarder_send_packet(packet,nextHopDestMap);
		//cout << "ForwarderProcess 2" << endl;	

	}

}

// Building lastMPRDestMap: contain records (last MPR,receiver list)

HashTable<IPAddress,Vector<IPAddress> >
MCOMProcess::build_lastMPRDestMap(Vector <addr_flag> addrlist)
{
	//cout << "LastMPR 1" << endl;
	int new_list_size = addrlist.size();	
	//cout << "LastMPR 11" << endl;

	Vector<IPAddress> separatedReceivers; // if there is no MPR connected to a receiver, then it is included in this set

	HashTable<IPAddress,Vector<IPAddress> > lastMPRDestMap; // (last MPR, list of destination)
	HashMap<IPPair, void*> *topology_set = _topologyInfo->get_topology_set();

	//HashMap<IPPair, topology_data*> *topology_set = _topologyInfo->get_topology_set();

	//cout << "LastMPR 2" << endl;

	Vector<IPAddress> temp;
	//click_chatter("build_lastMPRDestMap - new list size:%d",new_list_size);

	for(int i=0; i < new_list_size; i++){
		IPAddress dst;
		IPAddress key;
		dst = addrlist[i].addr;
		bool bdst = false; // this variable is used to check whether there is one MPR connected to dst. 
				//If not (bdst == false), this dst will be directly included in separatedReceivers 

		//click_chatter ( "Dst: %s\n", dst.unparse().c_str() ); 
		//cout << "LastMPR 3" << endl;

		for ( HashMap<IPPair, void*>::iterator iter = topology_set->begin(); iter != topology_set->end(); iter++ ) {
			topology_data *topology = ( topology_data * ) iter.value();
			const IPRoute * dest_temp;
			const IPRoute * last_temp;

			//cout << "LastMPR 4" << endl;

			dest_temp = _linearIPlookup->lookup_iproute( topology->T_dest_addr );
			last_temp = _linearIPlookup->lookup_iproute( topology->T_last_addr );
			if(dest_temp == NULL){
				//cout << "Dest is null "  << endl;
				continue;
			}
			if(last_temp == NULL){
				//cout << "Last is null "  << endl;
				continue;
			}
			//click_chatter ( "dest_temp: %s\n", dest_temp->gw.unparse().c_str() );
			//click_chatter ( "last_temp: %s\n", last_temp->gw.unparse().c_str() );

			//click_chatter ( "topology->T_dest_addr: %s\n", topology->T_dest_addr.unparse().c_str() );
			//click_chatter ( "topology->T_last_addr: %s\n", topology->T_last_addr.unparse().c_str() );
			//int t1 = last_temp->extra;
			//int t2 = dest_temp->extra;
			//cout << "last_temp->extra: " << t1 << endl;
			//cout << "dest_temp->extra: " << t2 << endl;

			//if((topology->T_dest_addr == dst)&& (last_temp->extra <= dest_temp->extra)){
			if(topology->T_dest_addr == dst){
				bdst = true;
				key = topology->T_last_addr;
				//cout << "LastMPR 5" << endl;
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
		if(bdst == false){ // if there is no MPR node connected to this node
			separatedReceivers.clear();	
			key = NULL;
			if (lastMPRDestMap.find(key) == NULL){
				separatedReceivers.push_back(dst);
				//click_chatter("Separated 1 \n");
				lastMPRDestMap.find_insert(key,separatedReceivers);
			}
			else{
				separatedReceivers = lastMPRDestMap[key];
				separatedReceivers.push_back(dst);
				lastMPRDestMap.replace(key,separatedReceivers);
			}
			//separatedReceivers.push_back(dst);
		}
	}
	//cout << "LastMPR 6" << endl;

	return lastMPRDestMap;

	
}


// Building MPR/destination list

Vector<addr_flag> 
MCOMProcess::build_MPR_Dest_List(HashTable<IPAddress,Vector<IPAddress> > lastMPRDestMap, Vector<IPAddress> neighborReceivers)
{
	Vector<addr_flag> mpr_dest_vector;
	addr_flag temp;
	for(HashTable<IPAddress,Vector<IPAddress> >::iterator it = lastMPRDestMap.begin(); it; ++it){
		IPAddress hkey = it.key();
		Vector<IPAddress> temp_list = it.value();		
		int size = temp_list.size();	
		//click_chatter("Size of temp_list: %d \n", size);

		if (hkey == NULL){
			//click_chatter("MPR dest list 3");
			for(int i=0; i<size; i++){
				temp.addr = temp_list[i];
				temp.flag = MCOM_UNICASTED_RECEIVER_FLAG; // = 0
				mpr_dest_vector.push_back(temp);
			}
			continue;			
		}

		if (temp_list.size() >= 2){ //if the number of receivers of a lastMPR is greater than or equal to 2
				       // then addresses of receivers are replaced by MPR address

			//click_chatter("List of receivers is greater than or equals to 2");

			temp.addr = it.key();
			temp.flag = MCOM_MPR_RECEIVER_FLAG; // = 2
			mpr_dest_vector.push_back(temp);	
		}
		else if(temp_list.size() == 1){
			temp.addr = temp_list[0];
			temp.flag = MCOM_UNICASTED_RECEIVER_FLAG;
			mpr_dest_vector.push_back(temp);
		}		
	}
	for(int i=0; i < neighborReceivers.size(); i++){
		temp.addr = neighborReceivers[i];
		temp.flag = MCOM_UNICASTED_RECEIVER_FLAG;
		mpr_dest_vector.push_back(temp);
	}
	return mpr_dest_vector;	
}


// Building nextHopDestMap: contain records (next hop,MPR/destination list)

HashTable<IPAddress,Vector<addr_flag> > 
MCOMProcess::build_nextHopDestMap(Vector<addr_flag> mpr_dest_vector)
{
	HashTable<IPAddress,Vector<addr_flag> > nextHopDestMap; // (next hop, list of MPR/destination)
	Vector<addr_flag> temp_vector;
	const IPRoute* nexthop;

	for(int i=0; i < mpr_dest_vector.size(); i++){
		temp_vector.clear();
		addr_flag temp_mpr_dest;
		temp_mpr_dest = mpr_dest_vector[i];

		//click_chatter("\ntemp_mpr_dest: %s",temp_mpr_dest.addr.unparse().c_str() );
		nexthop = _linearIPlookup->lookup_iproute(temp_mpr_dest.addr);
		//click_chatter("\n next hop: %s",nexthop->gw.unparse().c_str() );

		if(!nexthop){ // get next hop for a destination
			//cout << "Route is not existed for addrlist - " << i << endl;
			continue;	
		} 
		
		IPAddress key = nexthop->gw;
		//click_chatter ( "Next hop is: %s\n", key.unparse().c_str() );


		//cout << "Size of MPR/destination list: " << mpr_dest_vector.size() << endl;

		if (nextHopDestMap.find(key) == NULL){ // the next hop is not existed in the hash table
			temp_vector.push_back(temp_mpr_dest);
			nextHopDestMap.find_insert(key,temp_vector);
			//cout << "next hop is not existed in the hash table" << endl;
		}
		else { // the next hop is existed in the hash table. Another destination will be inserted for the next hop
			temp_vector = nextHopDestMap[key];
			temp_vector.push_back(temp_mpr_dest);
			nextHopDestMap.replace(key,temp_vector);
			//cout << "next hop is existed in the hash table. Another destination will be inserted" << endl;		
		}
	}
	return nextHopDestMap;	
}

// sending packets at source
void MCOMProcess::source_send_packet(WritablePacket* packet,HashTable<IPAddress,Vector<addr_flag> > nextHopDestMap)
{
	// There are three situations in source (3 types): only these three situations are considered, other situations 
	// are not considered here.
	// 1. All receiver has status PROBE, then the probe flag of the sending packet is set
	// 2. All receiver has status DATA, then the probe flag	is not set
	// 3. One reciever has status JOIN_PROBE, the rest of receivers has status DATA. Then in the packet, 
	//	the addr_flag of that receiver has flag of JOIN_PROBE
	
	// There 	

	int type = 2; // type 1, 2,3 as described above
	IPAddress join_receiver;
	//click_chatter("Entering send_packet");
	HashTable<int, flow_member_data*> * _fTable = _flowMemberInfo->get_flow_member_set();
	if(_fTable->size() != 0){
		//click_chatter("_fTable is not NULL in mcomprocess");
		flow_member_data* fd = _fTable->get(_flowid);
		if(fd != NULL){
			for(int i=0; i<fd->record.size(); i++){
				if(fd->record[i].status == MCOM_STATUS_JOIN_PROBE){
					type = 3;
					join_receiver = fd->record[i].receiver_addr;
				}
				else if(fd->record[i].status == MCOM_STATUS_PROBE)
					type = 1;
			}
		}
	}
	//click_chatter("MCOM process - send_packet 1");

	click_ip *ip = packet->ip_header();
	int num = 0;
	int next_hop_size = nextHopDestMap.size();
	for(HashTable<IPAddress,Vector<addr_flag> >::iterator it = nextHopDestMap.begin(); it; ++it) {
		//create each packet for each hop

		//click_chatter ( "Next hop in send_packet is: %s\n", (it.key()).unparse().c_str() );

		num ++;
		//cout << "Number of next hops: " << num << endl;
		
		click_udp *udp_temp = packet->udp_header();
		multicast_hdr *md = reinterpret_cast<multicast_hdr *> (udp_temp +1); 
		assert(md);
		Vector<addr_flag> tmp;
		tmp = it.value();				
		
		// insert addresses of destinations to the multicast header
		
		if((type == 3)&&(tmp.size() == 1)&&(tmp[0].addr == join_receiver)){ // this is the graft node that unicast packet to join_receiver
										// set this packet as shadow class
			md->is_probe = MCOM_STATUS_PROBE;

		} else	if(type == 1){ // if all receivers have status PROBE, the packet will be marked as PROBE
			md->is_probe = MCOM_STATUS_PROBE;
		} 
		else if(type = 2){ //if all receivers have status DATA
			md->is_probe = MCOM_STATUS_DATA;	
		}

		for(int i=0; i<tmp.size(); i++){
				addr_flag af;
				af.addr = tmp[i].addr;
				if(join_receiver == tmp[i].addr){
					af.flag = MCOM_JOIN_PROBE_RECEIVER_FLAG;
				} else {
					af.flag = tmp[i].flag; 
				}
				md->array[i].addr = tmp[i].addr;;
				md->array[i].flag = tmp[i].flag;
				//cout << "tmp size: " << tmp.size << endl;
		}

		md->protocolid = 1;
		md->length = 2 + 4 * tmp.size(); 
		md->size = tmp.size();
		packet->set_dst_ip_anno(it.key());
		
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
		

		if(next_hop_size == num){
			output(2).push(packet); // send packet to network interface
		}
		else {
			//Packet *pk1 = packet->clone();
			output(2).push(packet->clone());	
		}
	}
}

// sending packets at forwarders
void MCOMProcess::forwarder_send_packet(WritablePacket* packet,HashTable<IPAddress,Vector<addr_flag> > nextHopDestMap)
{
	IPAddress join_receiver;
	//click_chatter("Entering send_packet");
	
	click_udp *udp_temp = packet->udp_header();
	multicast_hdr *md = reinterpret_cast<multicast_hdr *> (udp_temp +1); 
	assert(md);
	Vector<addr_flag> tmp;
	bool is_graft = false;

	click_ip *ip = packet->ip_header();
	int num = 0;
	int next_hop_size = nextHopDestMap.size();
	for(HashTable<IPAddress,Vector<addr_flag> >::iterator it = nextHopDestMap.begin(); it; ++it) {
		//create each packet for each hop
		//click_chatter ( "Next hop in send_packet is: %s\n", (it.key()).unparse().c_str() );
		tmp = it.value();				
		if((tmp[0].flag == MCOM_JOIN_PROBE_RECEIVER_FLAG)&&(tmp.size() == 1)){
			md->is_probe = MCOM_STATUS_JOIN_PROBE;
		}
		num ++;
	
		// insert addresses of destinations to the multicast header
		for(int i=0; i<tmp.size(); i++){
			md->array[i].addr = tmp[i].addr;;
			md->array[i].flag = tmp[i].flag;
			//cout << "tmp size: " << tmp.size << endl;
		}
		
		
		// send the packet to the next hop
		
		md->protocolid = 1;
		md->length = 2 + 4 * tmp.size(); 
		md->size = tmp.size();

		packet->set_dst_ip_anno(it.key());

		
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
		

		if(next_hop_size == num){
			output(2).push(packet); // send packet to network interface
		}
		else {
			//Packet *pk1 = packet->clone();
			output(2).push(packet->clone());	
		}
	}
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MCOMProcess)
