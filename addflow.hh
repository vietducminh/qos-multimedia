// addflow.hh

/* 
	Element adds a flow into the Flow Member InfoBase at source
	After a specific of time, the receivers with the PROBE status are removed from the flow
 */

#ifndef ADD_FLOW_HH
#define ADD_FLOW_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>

#include "click_mcom.hh"
#include "MulticastProcess.hh"
#include"flow_member_infobase.hh"

CLICK_DECLS

class AddFlow : public Element{
public:

	AddFlow();
	~AddFlow();
  
	const char *class_name() const { return "AddFlow"; }  
	const char *processing() const { return PUSH; }
	const char *port_count() const  	{ return "1/1"; }

	int configure(Vector<String> &, ErrorHandler *);
	void push(int, Packet *);

private:
	int _flowid;
	int count; // if count ==0, this packet is the first one of the flow
	FlowMemberInfoBase* _flowMemberInfo;

	struct Userdata {
		AddFlow * addFlow;
		int flowid;
	};
	static void callback(Timer*, void * );
	void expire(Userdata*);
};

CLICK_ENDDECLS
#endif
  
