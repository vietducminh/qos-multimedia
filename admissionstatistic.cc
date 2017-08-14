// admissionstatistic.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "admissionstatistic.hh"

using namespace std;

CLICK_DECLS

AdmissionStatistic::AdmissionStatistic()
{} 

AdmissionStatistic::~AdmissionStatistic()
{}

int AdmissionStatistic::initialize(ErrorHandler *) 
{
	_admissionTable = new HashTable<int, int >; // flowid, ADMITTED_FLOW/REJECTED_FLOW 

     	return 0;
}

void AdmissionStatistic::uninitialize(){
	delete _admissionTable;
}

int AdmissionStatistic::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,

		cpEnd) < 0) 
	{
		cout << "This is error from configure";
		return -1;
	
	}

	return 0;

}

void AdmissionStatistic::push(int port, Packet *p)
{
	assert(port == 0);
	WritablePacket *packet = p->uniqueify();
	assert(packet);
	
	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp +1); 
	int flowid = mulhdr->flowid;
	if(_admissionTable->find(flowid) == NULL){
		_admissionTable->find_insert(flowid,TEMP_FLOW);

		Userdata* udata = new Userdata();
		assert(udata);
		udata->admissionStatistic = this;
		udata->flowid = flowid;

		Timer* timer = new Timer(&AdmissionStatistic::callback, udata);
		assert(timer);
		timer->initialize(this);
		uint32_t num = ADMISSION_CHECK_PERIOD; // 7 seconds
		timer->schedule_after_sec(num); // the function callback() will be called after num second (when the timer fires)		
	}
	packet->kill();
}

void AdmissionStatistic::callback(Timer*, void *data)
{
	Userdata* udata = (Userdata*) data;
	assert(udata);
	udata->admissionStatistic->expire(udata);
}

void AdmissionStatistic::expire(Userdata *udata)
{
	// find the admission of the flow
	int flowid = udata->flowid;
	HashTable<int, flow_member_data*> * _fTable= _flowMemberInfo->get_flow_member_set();
	flow_member_data* fd = _fTable->get(flowid);
	if(fd->record.size() == 0){ // there is no accepted receiver for this flow
		_admissionTable->replace(flowid,REJECTED_FLOW);
		click_chatter("admission statistic: Flow is rejected");
	}	
	else{
		_admissionTable->replace(flowid,ADMITTED_FLOW);
		click_chatter("admission statistic: Flow is accepted");
	}

	delete udata;
}

HashTable<int, int > *
AdmissionStatistic::get_admission_table(){
	return _admissionTable;
}


CLICK_ENDDECLS
EXPORT_ELEMENT(AdmissionStatistic)

