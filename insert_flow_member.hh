// insert_flow_member.hh

/* 
	Element inserts flow requirement into FlowRequirementInfoBase
*/

#ifndef INSERT_FLOW_MEMBER_HH
#define INSERT_FLOW_MEMBER_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include "click_mcom.hh"
#include "flow_member_infobase.hh"

CLICK_DECLS

class InsertFlowMember : public Element{
public:

	InsertFlowMember();
	~InsertFlowMember();
  
	const char *class_name() const { return "InsertFlowMember"; }  
	const char *processing() const { return PUSH; }
	const char *port_count() const  	{ return "0/0"; }
	int configure(Vector<String> &, ErrorHandler *);
	int initialize(ErrorHandler *);

private:
	Vector <IPAddress> _ipAddrList;
	int _flowid;
	IPAddress _sourceAddr;
	FlowMemberInfoBase *_flowMemberInfo;

};

CLICK_ENDDECLS
#endif
  
