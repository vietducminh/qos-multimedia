// join_request_generator.hh

/*
	Element generates a join request
*/

#ifndef JOIN_REQUEST_GENERATOR_HH
#define JOIN_REQUEST_GENERATOR_HH

#include <click/element.hh>
#include <click/ipaddress.hh>
#include "click_mcom.hh"

CLICK_DECLS 

class JoinRequestGenerator : public Element{
public:

	JoinRequestGenerator();
	~JoinRequestGenerator();
  
	const char *class_name() const { return "JoinRequestGenerator"; }  
	const char *processing() const { return PUSH; }
	const char *port_count() const  	{ return "0/1"; }

	int configure(Vector<String> &, ErrorHandler *);
	//void push(int, Packet *);
	void run_timer(Timer* timer);
	int initialize(ErrorHandler *);
	Packet* generate_join_request();

private:
	IPAddress _myAddr;
	int _flowid;
	IPAddress _sourceAddr;
	IPAddress _receiverAddr;
	int _tJoin;
	Timer _timer;
	atomic_uint32_t _id;

};

CLICK_ENDDECLS
#endif
  
