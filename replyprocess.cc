// replyprocess.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "replyprocess.hh"

using namespace std;

CLICK_DECLS

ReplyProcess::ReplyProcess()
//:_timer(this)
{} 

ReplyProcess::~ReplyProcess()
{}

int ReplyProcess::initialize(ErrorHandler *) 
{
     	//_timer.initialize(this);   // Initialize timer object (mandatory).
     	//_timer.schedule_now();     // Set the timer to fire as soon as the
                                // router runs.
	_decisionTable = new HashTable<int, decision_table_data* >;
     	return 0;
}

void ReplyProcess::uninitialize(){
	delete _decisionTable;
}

int ReplyProcess::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,
		//"RATED_SOURCE",0,cpElement,&_ratedSourceElement,		

		cpEnd) < 0) 
	{
		cout << "This is error from configure";
		return -1;
	
	}

	return 0;

}

void ReplyProcess::push(int port, Packet *p)
{
	assert(port == 0);
	WritablePacket *packet = p->uniqueify();
	assert(packet);
	
	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);
	click_chatter("reply process 0a");

	reply_pkt_hdr *reply_hdr = reinterpret_cast<reply_pkt_hdr *> (udp + 1);
	reply_info *ri = reinterpret_cast<reply_info *> (reply_hdr + 1);
	int type;

	type =  reply_hdr->_type;
	int ok_prune; // 1 = OK, 0 = PRUNE
	IPAddress rec_addr; // address of receiver
	int _flowid;
	IPAddress sourceAddr;

	_flowid = ri->flowid;
	sourceAddr = ri->source_addr;
	//struct flowid_source _flowidSource;
	//_flowidSource = ri->flowidSource;

	if(type == MCOM_REPLY_RECEIVER_TO_SOURCE_MESSAGE){ // this is a reply from receiver to source
						// source changes the status of receiver in _flowMemberInfo
		//click_chatter("This is a reply from a receiver to source");
		ok_prune = ri->value;
		rec_addr = ri->addr;

		click_chatter("flowid: %d",_flowid);		
		click_chatter("receiver addr: %s",rec_addr.unparse().c_str());		
		click_chatter("ok_prune: %d",ok_prune);		

		HashTable<int, flow_member_data*> * _fTable = _flowMemberInfo->get_flow_member_set();

		flow_member_data* fd = _fTable->get(_flowid);
		Vector<struct receiver_status>::iterator iter = (fd->record).begin();
		while(iter != (fd->record).end() ){
			if(rec_addr == iter->receiver_addr){
				//iter->status = ok_prune;
				if(ok_prune == MCOM_PRUNE_REPLY){
					iter->status = MCOM_STATUS_REJECTED;
					//click_chatter("replyprocess: mcom status rejected");
				}
				else if(ok_prune == MCOM_OK_REPLY){
					iter->status = MCOM_STATUS_DATA;
					//click_chatter("replyprocess: mcom status data");
				}
			}
			iter++;		
		}

		// delete receivers that has status PRUNE in the Vector fd->record
		Vector<receiver_status>::iterator iter1 = fd->record.begin();

		while(iter1 != fd->record.end() ){	
			int status = iter1->status;
			if(status == MCOM_STATUS_REJECTED){
				click_chatter("Delete receiver from the table");
				iter1 = fd->record.erase(iter1);
				continue;
			}
			++iter1;
		}
	
	}

	else if(type == MCOM_REPLY_RECEIVER_TO_MPR_MESSAGE){ // this is a reply from receiver to last-MPR
						// last-MPR combines replies from connected receivers in a predefied time	
						// after the predefied time, last-MPR will send combied reply to the source
		//click_chatter("This is a reply from receiver to MPR");


		//HashTable<flowid_source, Vector<receiver_decision>* >::iterator iter = _decisionTable->find(_flowid);
		HashTable<int, decision_table_data* >::iterator iter = _decisionTable->find(_flowid);

		if(iter == NULL){
			decision_table_data* dtd = new decision_table_data;	
			struct receiver_decision temp; 
			dtd->source_addr = sourceAddr;	
			temp.receiver_addr = ri->addr;
			temp.decision = ri->value;	

			dtd->rd.push_back(temp);

			_decisionTable->find_insert(_flowid, dtd);
				
			Userdata* udata = new Userdata();
			assert(udata);
			udata->rp = this;
			udata->flowid = _flowid;
			udata->sourceAddr = sourceAddr;

			Timer* timer = new Timer(&ReplyProcess::callback, udata);
			assert(timer);
			timer->initialize(this);
			uint32_t num = MPR_WAITE_REPLY_PERIOD; // second
			timer->schedule_after_sec(num); // the function callback() will be called after num second (when the timer fires)		
		}
		else{
			receiver_decision rd;
			rd.receiver_addr = ri->addr;
			rd.decision = ri->value;	
			iter.value()->rd.push_back(rd);
		}
		
	}
	else if(type == MCOM_REPLY_MPR_TO_SOURCE_MESSAGE){ // reply from last-MPR to source
		click_chatter("This is a reply from MPR to source");
 		int num_entries = reply_hdr->num_entries; // source will check the entry list and update or remove information of receivers
		HashTable<int, flow_member_data*> * _fTable= _flowMemberInfo->get_flow_member_set();

		flow_member_data* fd = _fTable->get(_flowid);

		if(fd == NULL){
			click_chatter("fd is NULL");
		}
		click_chatter("num_entries: %d",num_entries);
		for(int i=0; i<num_entries; i++){
			_flowid = ri->flowid;
			ok_prune = ri->value;
			rec_addr = ri->addr; 

			click_chatter("flowid: %d",_flowid);		
			click_chatter("receiver addr: %s",rec_addr.unparse().c_str());		
			click_chatter("ok_prune: %d",ok_prune);
			int k=0;
		
			click_chatter("fd->record size: %d",fd->record.size());	
			for(k; k<fd->record.size(); k++){
				if(rec_addr == fd->record[k].receiver_addr){
					if(ok_prune == MCOM_PRUNE_REPLY){
						fd->record[k].status = MCOM_STATUS_REJECTED;
						//click_chatter("replyprocess: mcom status rejected");
					}
					else if(ok_prune == MCOM_OK_REPLY){
						fd->record[k].status = MCOM_STATUS_DATA;
						//click_chatter("replyprocess: mcom status data");
					}
				}		
			}
			
			ri++;
		}
		
		// delete receivers that has status PRUNE in the Vector fd->record
		Vector<receiver_status>::iterator iter = fd->record.begin();

		while(iter != fd->record.end() ){	
			int status = iter->status;
			if(status == MCOM_STATUS_REJECTED){
				click_chatter("Delete receiver from the table");
				iter = fd->record.erase(iter);
				continue;
			}
			++iter;
		}
		/*
		if(_flow_admission == false){
			click_chatter("flow admission is false");
			int success = HandlerCall::call_write(_ratedSourceElement, "active", "FALSE");
		}
		*/
	}

	output(0).push(packet);
}

void ReplyProcess::callback(Timer*, void *data)
{
	Userdata* udata = (Userdata*) data;
	assert(udata);
	udata->rp->expire(udata);
}

void ReplyProcess::expire(Userdata *udata)
{
	click_chatter(" expire - send from MPR to source");
	
	// find the id of the flow

	int flowid = udata->flowid;
	IPAddress sourceAddr = udata->sourceAddr;
	
	send_reply_from_MPR(flowid, sourceAddr);
	
	delete udata;
}

void
ReplyProcess::send_reply_from_MPR(int flowid,IPAddress sourceAddr)
{
	int num_entries;
	IPAddress _sourceGroupAddr;
	//HashTable<int, flow_member_data*> * fdTable =  _flowMemberInfo->get_flow_member_set();	
	//flow_member_data* fd = fdTable->get(_flowid);
	//_sourceGroupAddr = fd->source_addr;

	_sourceGroupAddr = sourceAddr;
	HashTable<int, decision_table_data* >::iterator iter = _decisionTable->find(flowid);
	decision_table_data* dtd = new decision_table_data;
	//Vector<receiver_decision>* rec_dec = iter.value();
	//num_entries = rec_dec->size();
	dtd = iter.value();
	num_entries = dtd->rd.size();

  	int packet_size = sizeof(click_ip) + sizeof(click_udp) + sizeof(reply_pkt_hdr) + sizeof(reply_info) * num_entries;
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
	
	reply_pkt_hdr *reply_hdr = (reply_pkt_hdr *) (udp + 1);
	reply_hdr->_type = MCOM_REPLY_MPR_TO_SOURCE_MESSAGE;
	reply_hdr->num_entries = num_entries;

	reply_info * r_info = (reply_info *) (reply_hdr + 1);
	for(int i=0; i<dtd->rd.size(); i++){
		r_info->flowid = flowid;
		r_info->source_addr = _sourceGroupAddr;
		r_info->addr = (dtd->rd)[i].receiver_addr;
		r_info->value = (dtd->rd)[i].decision;

		r_info++;
	}	

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
	ip->ip_dst = _sourceGroupAddr;
	packet->set_dst_ip_anno(IPAddress(_sourceGroupAddr));
 
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
	udp->uh_sport = 0205;
	udp->uh_dport = 0205;
	uint16_t len = packet->length() - sizeof(click_ip);
	udp->uh_ulen = htons(len);
	udp->uh_sum = 0;
	unsigned csum = click_in_cksum((unsigned char *)udp, len);
	udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
	
	output(1).push(packet);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(ReplyProcess)

