// ecnprocess.hh

/*
	Element processes ECN congestion notifications
*/

#ifndef ECN_PROCESS_HH
#define ECN_PROCESS_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include "click_mcom.hh"
#include "flow_member_infobase.hh"


CLICK_DECLS 

#define T_meas 300; // the period (in ms) used to calculate the fraction of ECN marked packets
			// this parameter was recommended from 100ms - 500ms


class ECNProcess : public Element{
public:

	ECNProcess();
	~ECNProcess();
  
	const char *class_name() const { return "ECNProcess"; }  
	const char *processing() const { return PUSH; }
	const char *port_count() const  	{ return "1/1"; }


	int configure(Vector<String> &, ErrorHandler *);
	void push(int, Packet *);

	int initialize(ErrorHandler *);
	void uninitialize();
	int ECN_marked_count; // store the number of ECN marked packets
	int total_packet; // store the number of packets the destination receives in a period

private:
	Timer _timer;
	IPAddress _myAddr;
	Vector <ecnStatis> ecnStatisTable;
	FlowMemberInfoBase* _flowMemberInfo;

	struct Userdata {
		ECNProcess * ecnprocess;
		int flowid;
		IPAddress congested_node;			
	};
	static void callback(Timer*, void * );
	void expire(Userdata*);

};

CLICK_ENDDECLS
#endif
  
