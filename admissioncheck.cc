// admissioncheck.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "admissioncheck.hh"

using namespace std;

CLICK_DECLS

AdmissionCheck::AdmissionCheck()
:_timer(this)

{} 

AdmissionCheck::~AdmissionCheck()
{}

int AdmissionCheck::initialize(ErrorHandler *) 
{
    	return 0;
}

void AdmissionCheck::uninitialize(){
}

int AdmissionCheck::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,
		"RATED_SOURCE",0,cpElement,&_ratedSourceElement,		

		cpEnd) < 0) 
	{
		cout << "This is error from configure";
		return -1;
	
	}
	count = 0;


	return 0;

}

void AdmissionCheck::push(int port, Packet *p)
{
	assert(port == 0);
	WritablePacket *packet = p->uniqueify();
	assert(packet);
	
	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp_hdr = reinterpret_cast<click_udp *> (ip + 1);
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp_hdr +1); 
	_flowid = mulhdr->flowid;

	HashTable<int, flow_member_data*> * _fTable= _flowMemberInfo->get_flow_member_set();
	flow_member_data* fd = _fTable->get(_flowid);
	//click_chatter("admission check: %d",fd->record.size());

	//if(fd->count == 1){ // this is the first packet of the flow
	if(count == 0){
		_timer.initialize(this);   // Initialize timer object (mandatory).
		_timer.schedule_after_sec(ADMISSION_CHECK_PERIOD); 
		double current_time = Timestamp::now().doubleval();
		click_chatter("Admission check: Time for first packet: %llu", current_time);
		click_chatter("First Time:");
		count = 1;
	}
	else{
		count++;
	}
	
	output(0).push(packet);	
}

void 
AdmissionCheck::run_timer(Timer* timer)
{
	HashTable<int, flow_member_data*> * _fTable= _flowMemberInfo->get_flow_member_set();
	flow_member_data* fd = _fTable->get(_flowid);
	if(fd != NULL){
		if(fd->record.size() == 0){ // there is no accepted receiver for this flow
			click_chatter("There is no accepted receiver for this flow");
			int success = HandlerCall::call_write(_ratedSourceElement, "active", "false");

			click_chatter("success value: %d",success);
			double current_time = Timestamp::now().doubleval();
			click_chatter("Admission check: Time after expire: %llu", current_time);
		}
	}	
}

CLICK_ENDDECLS
EXPORT_ELEMENT(AdmissionCheck)

