// flow_requirement_infobase.hh

/* 
	Element stores the requirement of a flow: (flowid, packet_loss, e2e_delay)
*/

#ifndef FLOW_REQUIREMENT_INFOBASE_HH
#define FLOW_REQUIREMENT_INFOBASE_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include <click/bighashmap.hh>
#include <click/hashtable.hh>

#include "/home/viet/code/click-2.0.1/elements/local/click_mcom.hh"

CLICK_DECLS

class FlowRequirementInfoBase: public Element{
public:

	FlowRequirementInfoBase();
	~FlowRequirementInfoBase();

	const char* class_name() const { return "FlowRequirementInfoBase"; }
	FlowRequirementInfoBase *clone() const { return new FlowRequirementInfoBase; } 
	const char *port_count() const  { return "0/0"; }

	int configure(Vector<String> &conf, ErrorHandler *errh);
	int initialize(ErrorHandler *);
	void uninitialize();

	bool add_flow(int flowid, double packet_loss, double e2e_delay, double jitter,double real_datarate);
	struct flow_data *find_flow(int flowid);
	void remove_flow(int flowid);
	bool update_flow(int flowid, double packet_loss, double e2e_delay,double jitter,double real_datarate);
	HashTable<int, flow_data*> *get_flow_set();
	void print_flows();
	HashTable <int, flow_data *> *_flowSet;

private:
	//typedef HashMap <int, flow_data*> FlowSet;
	//FlowSet *_flowSet;

	//Timer _timer;
  	//void run_timer(Timer *);

};


CLICK_ENDDECLS
#endif
