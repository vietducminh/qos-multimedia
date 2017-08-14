// checkprobe.hh

/* 
	Element checks whether a packet is probe or not.
	Probe packets are pushed at output 0, none-probe packets are pushed at output 1
*/

#ifndef CHECK_PROBE_HH
#define CHECK_PROBE_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include "click_mcom.hh"


CLICK_DECLS

class CheckProbe : public Element{
public:

	CheckProbe();
	~CheckProbe();
  
	const char *class_name() const { return "CheckProbe"; }  
	const char *processing() const { return PUSH; }
	const char *port_count() const  	{ return "1/2"; }

	int configure(Vector<String> &, ErrorHandler *);
	void push(int, Packet *);

private:
	IPAddress _myAddr;
};

CLICK_ENDDECLS
#endif
  
