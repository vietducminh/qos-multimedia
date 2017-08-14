  // mcom_memberlist_infobase.hh

#ifndef MCOM_MEMBERLIST_INFOBASE_HH
#define MCOM_MEMBERLIST_INFOBASE_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include <click/bighashmap.hh>
#include "click_mcom.hh"

CLICK_DECLS

class MCOMMemberlistInfoBase: public Element{
public:

	MCOMMemberlistInfoBase();
	~MCOMMemberlistInfoBase();

	const char* class_name() const { return "MCOMMemberlistInfoBase"; }
	MCOMMemberlistInfoBase *clone() const { return new MCOMMemberlistInfoBase(); } 
	const char *port_count() const  { return "0/0"; }

	int configure(Vector<String> &conf, ErrorHandler *errh);
	int initialize(ErrorHandler *);
	void uninitialize();

	bool add_member(IPAddress receiver_addr, int status, struct timeval time);
	struct member_data *find_member(IPAddress receiver_addr);
	void remove_member(IPAddress receiver_addr);
	bool update_member(IPAddress receiver_addr, int status);
	HashMap<IPAddress, void*> *get_member_set();
	void print_members();

private:
	typedef HashMap <IPAddress, void *> MemberSet;

	MemberSet *_memberSet;
	Timer _timer;
  
	void run_timer(Timer *);

};


CLICK_ENDDECLS
#endif
