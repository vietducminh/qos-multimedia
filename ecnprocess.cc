// ecnprocess.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/udp.h>
#include <clicknet/ether.h>
#include <click/ipaddress.hh>
#include <click/router.hh>
#include <click/vector.hh>

#include "/home/viet/code/click-2.0.1/elements/local/ecnprocess.hh"

CLICK_DECLS

ECNProcess::ECNProcess()
{

}

ECNProcess::~ECNProcess()
{
}

int ECNProcess::initialize(ErrorHandler *) 
{
     	return 0;
}

void ECNProcess::uninitialize(){
	ecnStatisTable.clear();}


int
ECNProcess::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,

		cpEnd) < 0) 
	{
		click_chatter("This is error from configure");
		return -1;
	
	}
	
	return 0;
}


void
ECNProcess::push(int, Packet *p)
{
	assert(p);
	WritablePacket *p_new = p->uniqueify();
	assert(p_new);

	assert(p_new->has_network_header());
	click_ip *ip = p_new->ip_header();
	click_udp *udp = p_new->udp_header();
	congest_noti_pkt_hdr *congest_hdr = (congest_noti_pkt_hdr *) (udp + 1);
	
	// find out whether the (flowid,congested_node)is already in the table
	// if not, the couple will be added in the table
	// if it is, the total marked packet will be updated
	bool is_exist = false;
	int index; // store the index of the (flowid, congested_node)
	for(int i=0; i< ecnStatisTable.size(); i++){
		if((ecnStatisTable[i].flowid == congest_hdr->flowid)&&(ecnStatisTable[i].congested_node==congest_hdr->congested_node)){
			is_exist = true;
			index = i;
		}
	}
	if(is_exist == false){
		ecnStatis statis;
		statis.flowid = congest_hdr->flowid;
		statis.congested_node = congest_hdr->congested_node;
		statis.total_marked_packets = 1;
		for(int i=0; i>congest_hdr->size; i++){
			statis.receiver_list.push_back(congest_hdr->array[i]);
		}
		ecnStatisTable.push_back(statis);	

		Userdata* udata = new Userdata();
		assert(udata);
		udata->ecnprocess = this;
		udata->flowid = congest_hdr->flowid;
		udata->congested_node = congest_hdr->congested_node;

		Timer* timer = new Timer(&ECNProcess::callback, udata);
		assert(timer);
		timer->initialize(this);
		uint32_t num = 1000; // 1000ms = 1s
		timer->schedule_after_msec(num); // the function callback() will be called after num second (when the timer fires)		
	}
	else{
		ecnStatisTable[index].total_marked_packets++; // update the total marked packets
	}
	
     	
}

void ECNProcess::callback(Timer*, void *data)
{
	Userdata* udata = (Userdata*) data;
	assert(udata);
	udata->ecnprocess->expire(udata);
}

void ECNProcess::expire(Userdata *udata)
{
	click_chatter(" expire - ECNProcess");
	
	// find the id of the flow
	int flowid = udata->flowid;
	IPAddress congested_node = udata->congested_node;
	
	// if the total marked packets is larger than a threshold, the source will preempt the flow forward to that congested node
	// the list of receivers will be deleted at the FlowMemberInfoBase
	Vector<struct receiver_status> newVector; // list of receivers excluded the preempted receivers

	for(int i=0; i<ecnStatisTable.size(); i++){
		if((ecnStatisTable[i].flowid == flowid)&&(ecnStatisTable[i].congested_node==congested_node)){
			if(ecnStatisTable[i].total_marked_packets >= ECN_MARKED_PACKET_THRESHOLD){
				HashTable<int, flow_member_data*> * _fTable = _flowMemberInfo->get_flow_member_set();
				flow_member_data* fd = _fTable->get(flowid);
				Vector<struct receiver_status>::iterator iter = (fd->record).begin();
				while(iter != (fd->record).end() ){
					for(int j=0; j<ecnStatisTable[i].receiver_list.size(); j++){
						if(ecnStatisTable[i].receiver_list[j] == iter->receiver_addr){
							iter = (fd->record).erase(iter);
							continue;
						}	
					}								
					iter++;		
				}	

			}			
		}
	}	

	delete udata;
}




CLICK_ENDDECLS

EXPORT_ELEMENT(ECNProcess);

