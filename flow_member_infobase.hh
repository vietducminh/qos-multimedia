  // flow_member_infobase.hh

// Element stores information about source and receivers of a flow: 
//(flowid, source_addr, Vector<struct receiver_status>)

#ifndef FLOW_MEMBER_INFOBASE_HH
#define FLOW_MEMBER_INFOBASE_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
//#include <click/bighashmap.hh>
#include<click/hashtable.hh>
#include "click_mcom.hh"

CLICK_DECLS

class FlowMemberInfoBase: public Element{
public:

	FlowMemberInfoBase();
	~FlowMemberInfoBase();

	const char* class_name() const { return "FlowMemberInfoBase"; }
	FlowMemberInfoBase *clone() const { return new FlowMemberInfoBase; } 
	const char *port_count() const  { return "0/0"; }

	int configure(Vector<String> &conf, ErrorHandler *errh);
	int initialize(ErrorHandler *);
	void uninitialize();

	bool add_flow_member(int flowid, IPAddress source_addr, Vector<receiver_status> record);
	struct flow_member_data *find_flow_member(int flowid);
	void remove_flow_member(int flowid);
	bool update_flow_member(int flowid, IPAddress source_addr, Vector<receiver_status> record);
	HashTable<int, flow_member_data*> *get_flow_member_set();
	void print_flow_member();

private:
	//typedef HashMap <int, void *> FlowMemberSet;
	//FlowMemberSet *_flowMemberSet;
	HashTable<int, flow_member_data*> *_flowMemberSet;
	//Timer _timer;  
	//void run_timer(Timer *);

};

CLICK_ENDDECLS
#endif
