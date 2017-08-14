// AddReceiver.hh

/* 
	Element adds a list of receivers into a packet
*/

#ifndef CLICK_ADDRECEIVER_HH
#define CLICK_ADDRECEIVER_HH
#include <click/element.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <clicknet/udp.h>

#include <click/timer.hh>
#include <click/hashtable.hh>
#include <click/error.hh>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "/home/viet/code/click-2.0.1/elements/local/MulticastProcess.hh"
using namespace std;

CLICK_DECLS

//struct addr_flag{
//	IPAddress addr; // address of nodes
//	uint8_t flag; // flag for node to determine the type of node (MPRs, receivers, ...)
//};


//struct multicast_hdr{
//	uint8_t protocolid; // id of the protocol (default 1)
//	uint8_t length; // length of header
//	Vector<addr_flag> addr_vector; // list of receiver addresses
	
//};


class AddReceiver : public Element { 
	public:
		AddReceiver();
		~AddReceiver();
		
		const char *class_name() const	{ return "AddReceiver"; }
		const char *port_count() const	{ return "1/1"; }
		const char *processing() const	{ return PUSH; }
		int configure(Vector<String>&, ErrorHandler*);
		
		void push(int, Packet *);
		
	private:
		Vector <IPAddress> _ipAddrList;
		IPAddress _myAddr;

};

CLICK_ENDDECLS
#endif
