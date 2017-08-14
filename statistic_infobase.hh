// statistic_infobase.hh

/*
	Element provides statistic information 
*/

#ifndef STATISTIC_INFOBASE_HH
#define STATISTIC_INFOBASE_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include <click/bighashmap.hh>
#include <click/hashtable.hh>

#include "click_mcom.hh"
#include "flow_member_infobase.hh"

CLICK_DECLS

class StatisticInfoBase: public Element{
public:

	StatisticInfoBase();
	~StatisticInfoBase();

	const char* class_name() const { return "StatisticInfoBase"; }
	const char *port_count() const  { return "0/0"; }

	int configure(Vector<String> &conf, ErrorHandler *errh);
	int initialize(ErrorHandler *);
	void uninitialize();
	void print_out_tables();
	HashTable<int, receiver_stats_data >* get_receiver_table();
	HashTable<int, receiver_stats_data > *_receiverTable;

private:
	Timer _timer;
  	void run_timer(Timer *);
	FlowMemberInfoBase* _flowMemberInfo;

};


CLICK_ENDDECLS
#endif
