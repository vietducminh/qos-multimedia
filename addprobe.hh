// addprobe.hh

/*	Element adds the status as PROBE in a packet. 
	After going thourgh this element, the packet becomes a probe packet.	 
*/

#ifndef ADD_PROBE_HH
#define ADD_PROBE_HH

#include <click/element.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>
#include "click_mcom.hh"


CLICK_DECLS

class AddProbe : public Element{
public:

	AddProbe();
	~AddProbe();
  
	const char *class_name() const { return "AddProbe"; }  
	const char *processing() const { return PUSH; }
	const char *port_count() const  	{ return "1/1"; }

	int configure(Vector<String> &, ErrorHandler *);
	void push(int, Packet *);

private:
	IPAddress _myAddr;
};

CLICK_ENDDECLS
#endif
  
