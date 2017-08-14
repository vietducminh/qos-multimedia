// receiver_statistic.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "receiver_statistic.hh"

using namespace std;

CLICK_DECLS

ReceiverStatistic::ReceiverStatistic()

{} 

ReceiverStatistic::~ReceiverStatistic()
{}

int ReceiverStatistic::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
		"STATISTIC_INFOBASE",0,cpElement,&_statisticInfo,
		cpEnd) < 0) 
	{
		cout << "This is error from configure";
		return -1;
	
	}

	return 0;

}

void ReceiverStatistic::push(int port, Packet *p)
{
	assert(port == 0);
	WritablePacket *packet = p->uniqueify();
	assert(packet);
	
	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp +1); 

	_receiverTable = _statisticInfo->get_receiver_table();

	int flowid = mulhdr->flowid;
	int number;
	Timestamp now = Timestamp::now();
	Timestamp time_packet_sent;
	time_packet_sent =  packet->timestamp_anno();

	if(_receiverTable->find(flowid) == NULL){
		receiver_stats_data temp;
		temp.num_packet_received = 1;
		//temp.e2e_delay(0);
	  	//click_chatter("Time of variable e2e_delay: %f", temp.e2e_delay.doubleval());
	  	//click_chatter("Time of variable jitter: %f", temp.jitter.doubleval());

		//temp.e2e_delay = temp.e2e_delay + now - time_packet_sent;
		temp.e2e_delay = now - time_packet_sent;
		temp.previous_transition_time = now - time_packet_sent;
		//temp.jitter(0);
		_receiverTable->find_insert(flowid,temp);
	}
	else{
		receiver_stats_data temp = _receiverTable->get(flowid);
		temp.e2e_delay = temp.e2e_delay + now - time_packet_sent;
		
		if((now - time_packet_sent - temp.previous_transition_time) > 0)
			temp.jitter = temp.jitter + (now - time_packet_sent - temp.previous_transition_time);
		else
			temp.jitter = temp.jitter - (now - time_packet_sent - temp.previous_transition_time);
		temp.previous_transition_time = now - time_packet_sent;
		temp.num_packet_received++;
	  	//click_chatter("Time of variable e2e_delay: %f", temp.e2e_delay.doubleval());
	  	//click_chatter("Time of variable jitter: %f", temp.jitter.doubleval());

		_receiverTable->replace(flowid,temp);			
	}

	packet->kill();
}
	
CLICK_ENDDECLS
EXPORT_ELEMENT(ReceiverStatistic)

