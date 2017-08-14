// cngenerator.hh

/*
	Element generates ECN congestion notification towards source
*/

#ifndef CN_GENERATOR_HH
#define CN_GENERATOR_HH

#include <click/element.hh>
#include <click/ipaddress.hh>
#include "click_mcom.hh"


CLICK_DECLS 


class CNGenerator : public Element{
public:

	CNGenerator();
	~CNGenerator();
  
	const char *class_name() const { return "CNGenerator"; }  
	const char *processing() const { return PUSH; }
	const char *port_count() const  	{ return "1/1"; }

	int configure(Vector<String> &, ErrorHandler *);
	void push(int, Packet *);
private:
	IPAddress _myAddr;
	int _flowid;
	IPAddress _sourceAddr;
	atomic_uint32_t _id;

};

CLICK_ENDDECLS
#endif
  
