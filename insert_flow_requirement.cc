// insert_flow_requirement.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
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

#include "/home/viet/code/click-2.0.1/elements/local/insert_flow_requirement.hh"

CLICK_DECLS

InsertFlowRequirement::InsertFlowRequirement()
{
}

InsertFlowRequirement::~InsertFlowRequirement()
{
}

int
InsertFlowRequirement::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"FLOW_ID", 0, cpInteger, &_flowid, 
		"PACKET_LOSS", 0, cpDouble, &_packet_loss, 
		"E2E_DELAY", 0, cpDouble, &_e2e_delay, 
		"JITTER", 0, cpDouble, &_jitter, 		
		"REAL_DATARATE", 0, cpDouble, &_real_datarate, 
		"FLOW_REQUIREMENT_INFOBASE",0,cpElement,&_flowRequirementInfo,

		cpEnd) < 0) 
	{
		click_chatter("This is error from configure");
		return -1;
	
	}
	
	return 0;
}

int 
InsertFlowRequirement::initialize(ErrorHandler *){

	//click_chatter("flowid: %d", _flowid);
	//click_chatter("packet loss: %f", _packet_loss);
	//click_chatter("delay: %f", _e2e_delay);
	//click_chatter("jitter: %f", _jitter);	
	//click_chatter("data rate: %f", _real_datarate);
	_flowRequirementInfo->add_flow(_flowid,_packet_loss,_e2e_delay,_jitter,_real_datarate);

     	return 0;


}


CLICK_ENDDECLS

EXPORT_ELEMENT(InsertFlowRequirement);

