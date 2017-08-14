// insert_flow_requirement.hh

// Element inserts flow requirement into FlowRequirementInfoBase

#ifndef INSERT_FLOW_REQUIREMENT_HH
#define INSERT_FLOW_REQUIREMENT_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include "click_mcom.hh"
#include "flow_requirement_infobase.hh"

CLICK_DECLS

class InsertFlowRequirement : public Element{
public:

	InsertFlowRequirement();
	~InsertFlowRequirement();
  
	const char *class_name() const { return "InsertFlowRequirement"; }  
	const char *processing() const { return PUSH; }
	const char *port_count() const  	{ return "0/0"; }

	int initialize(ErrorHandler *);

	int configure(Vector<String> &, ErrorHandler *);

private:
	IPAddress _myAddr;
	int _flowid;
	double _packet_loss;
	double _e2e_delay;
	double _jitter;
	double _real_datarate;
	FlowRequirementInfoBase *_flowRequirementInfo;

};

CLICK_ENDDECLS
#endif
  
