// insert_flow_member.cc

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

#include "/home/viet/code/click-2.0.1/elements/local/insert_flow_member.hh"

CLICK_DECLS

InsertFlowMember::InsertFlowMember()
{
}

InsertFlowMember::~InsertFlowMember()
{
}

int
InsertFlowMember::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh, 
		"FLOW_ID", 0, cpInteger, &_flowid, 
		"SOURCE_ID", 0, cpIPAddress, &_sourceAddr, 
		"RECEIVER_IP_ADDRESS_LIST", 0, cpIPAddressList, &_ipAddrList, 
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,
		cpEnd) < 0) 
	{
		click_chatter("This is an error from configure");
		return -1;
	
	}

	return 0;
}


int InsertFlowMember::initialize(ErrorHandler *) 
{
     	Vector<receiver_status> record;
	Vector<IPAddress>::iterator iter = _ipAddrList.begin();
	receiver_status temp;
	temp.status = MCOM_STATUS_PROBE; 
	//temp.time = 29838;
	while(iter != _ipAddrList.end()){
		temp.receiver_addr = *iter;
		record.push_back(temp);
		iter++;
	}

	if (!_flowMemberInfo->add_flow_member(_flowid,_sourceAddr,record))
		click_chatter("The adding of flow member fails");
     	return 0;
 }

CLICK_ENDDECLS

EXPORT_ELEMENT(InsertFlowMember);

