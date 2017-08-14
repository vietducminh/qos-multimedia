// addflow.cc

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

#include "/home/viet/code/click-2.0.1/elements/local/addflow.hh"

CLICK_DECLS

AddFlow::AddFlow()
{
}

AddFlow::~AddFlow()
{
}

int
AddFlow::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh, 
		"FLOW_ID", 0, cpInteger, &_flowid, 
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,
		cpEnd) < 0) 
	{
		click_chatter("This is an error from configure");
		return -1;	
	}	
	count = 0;	
	return 0;
}

void
AddFlow::push(int, Packet *p){
	assert(p);
	WritablePacket *packet = p->uniqueify();
	assert(packet);
	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp_hdr = reinterpret_cast<click_udp *> (ip + 1);
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp_hdr +1); 
	mulhdr->flowid = _flowid; 	
	
	HashTable<int, flow_member_data*> * _fTable= _flowMemberInfo->get_flow_member_set();
	flow_member_data* fd = _fTable->get(_flowid);

	if(fd->count == 0){ // if this is the first packet of the flow
		Userdata* udata = new Userdata();
		assert(udata);
		udata->addFlow = this;
		udata->flowid = _flowid;
		Timer* timer = new Timer(&AddFlow::callback, udata);
		assert(timer);
		timer->initialize(this);
		uint32_t num = SOURCE_WAITE_REPLY_PERIOD; // seconds
		timer->schedule_after_sec(num); // the function callback() will be called after a specific of period (num second)
		fd->count = 1;
	}	
	else{
		fd->count++;
	}
	output(0).push(packet);
}

void AddFlow::callback(Timer*, void *data)
{
	Userdata* udata = (Userdata*) data;
	assert(udata);
	udata->addFlow->expire(udata);
}

void AddFlow::expire(Userdata *udata)
{
	//click_chatter(" expire - source stop to waite AC replies");
	
	int flowid = udata->flowid;
	HashTable<int, flow_member_data*> * _fTable= _flowMemberInfo->get_flow_member_set();
	flow_member_data* fd = _fTable->get(_flowid);

	int status;
	Vector<struct receiver_status>::iterator iter = fd->record.begin();

	// delete the receiver with PROBE status from the flow
	while(iter != fd->record.end() ){				
		IPAddress receiver = iter->receiver_addr;
		status = iter->status;
		if(status == MCOM_STATUS_PROBE){
			iter = fd->record.erase(iter);
			continue;
		}
		++iter;
	}
	
	delete udata;
}


CLICK_ENDDECLS

EXPORT_ELEMENT(AddFlow);

