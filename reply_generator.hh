// reply_generator.hh

/*
	Element generateS MCOM replies 
*/

#ifndef MCOM_REPLY_GENERATOR_HH
#define MCOM_REPLY_GENERATOR_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include "click_mcom.hh"


CLICK_DECLS

class MCOMReplyGenerator : public Element{
public:

	MCOMReplyGenerator();
	~MCOMReplyGenerator();
  
	const char *class_name() const { return "MCOMReplyGenerator"; }  
	const char *processing() const { return PUSH; }
	MCOMReplyGenerator *clone() const {return new MCOMReplyGenerator(); }
	const char *port_count() const  	{ return "1/1"; }


	int configure(Vector<String> &, ErrorHandler *);
	int initialize(ErrorHandler *);

	Packet *generate_reply();
	void run_timer(Timer *);

private:
	Timer _timer;
	IPAddress _myAddr;
	IPAddress _sourceGroupAddr;

	int _type;	
	atomic_uint32_t _id;
	int num_entry; // store the number of entry of reply_info	
};

CLICK_ENDDECLS
#endif
  
