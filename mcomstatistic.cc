// mcomstatistic.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "mcomstatistic.hh"

using namespace std;

CLICK_DECLS

MCOMStatistic::MCOMStatistic()
:_timer(this)

{} 

MCOMStatistic::~MCOMStatistic()
{}

int MCOMStatistic::initialize(ErrorHandler *) 
{
	_sourceTable = new HashTable<int, int >;
	_receiverTable = new HashTable<int, receiver_stats_data >;

	_timer.initialize(this);   // Initialize timer object (mandatory).
	_timer.schedule_after_sec(EXPERIMENT_PERIOD);

     	return 0;
}

void MCOMStatistic::uninitialize(){
	delete _sourceTable;
	delete _receiverTable;
}

int MCOMStatistic::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	_admissionStatistic = NULL;
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"ADMISSION_STATISTIC",0,cpElement,&_admissionStatistic,
		cpEnd) < 0) 
	{
		cout << "This is error from configure";
		return -1;
	
	}

	return 0;

}

void MCOMStatistic::push(int port, Packet *p)
{
	assert(port == 0);
	WritablePacket *packet = p->uniqueify();
	assert(packet);
	
	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp +1); 
	int flowid = mulhdr->flowid;
	int number;
	Timestamp now = Timestamp::now();
	Timestamp time_packet_sent;
	time_packet_sent =  packet->timestamp_anno();

	if(_myAddr == ip->ip_src){ // if this node is source
		if(_sourceTable->find(flowid) == NULL){
			_sourceTable->find_insert(flowid,1);
		}
		else{
			number = _sourceTable->get(flowid); // number of received packets
			number++;
			_sourceTable->replace(flowid,number);
		}
	}
	else{ // if this node is receiver
		if(_receiverTable->find(flowid) == NULL){
			receiver_stats_data temp;
			temp.num_packet_received = 1;
			temp.e2e_delay = temp.e2e_delay + now - time_packet_sent;
			temp.previous_transition_time = now - time_packet_sent;
			//temp.jitter = 0
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
			_receiverTable->replace(flowid,temp);			
		}
	}
	packet->kill();
	//output(0).push(packet); // packet will be Discarded
}

void 
MCOMStatistic::run_timer(Timer* timer)
{
	print_out_tables();
}


void 
MCOMStatistic::print_out_tables()
{
	click_chatter("this node: %s",_myAddr.unparse().c_str());
	click_chatter("Souce statistic table: \n");
	for (HashTable<int, int>::iterator it = _sourceTable->begin(); it; ++it){
		click_chatter("flowid: %d", it.key());
		click_chatter("total number of sent packet sent: %d",it.value());
	}
	click_chatter("Receiver statistic table: \n");
	for (HashTable<int, receiver_stats_data>::iterator it = _receiverTable->begin(); it; ++it){
		click_chatter("flowid: %d",it.key());
		double average_delay = it.value().e2e_delay.doubleval()/it.value().num_packet_received;
		double average_jitter = it.value().jitter.doubleval()/(it.value().num_packet_received - 1);
		double received_packet = it.value().num_packet_received;
		click_chatter("average delay: %f", average_delay);
		click_chatter("average jitter: %f", average_jitter);
		click_chatter("number of received packet: %f", received_packet);
	}
	click_chatter("Admission table: \n");
	if(_admissionStatistic != NULL){
		_admissionTable = _admissionStatistic->get_admission_table();
		for (HashTable<int, int>::iterator it = _admissionTable->begin(); it; ++it){
			click_chatter("flowid: %d",it.key());
			click_chatter("admission status: %d", it.value());
		}
	}
	click_chatter("after admissionStatistic");
}
	
CLICK_ENDDECLS
EXPORT_ELEMENT(MCOMStatistic)

