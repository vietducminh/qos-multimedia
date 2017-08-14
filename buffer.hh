// buffer.hh
/*
	Element works as a buffer, it stores a packet to for duplicate checking
	and deletes the packet in a specific of time.
*/

#ifndef CLICK_BUFFER_HH
#define CLICK_BUFFER_HH
#include <click/element.hh>
#include <clicknet/ip.h>
#include <click/timer.hh>
#include <click/hashtable.hh>
#include <click/error.hh>
#include <stdio.h>
#include <iostream>
#include <fstream>
//#include "multicastprocess.hh"

using namespace std;

CLICK_DECLS

class Buffer : public Element { 
	public:
		Buffer();
		~Buffer();
		
		const char *class_name() const	{ return "Buffer"; }
		const char *port_count() const	{ return "0/0"; }
		const char *processing() const	{ return PUSH; }
		int initialize(ErrorHandler *) ;
		int configure(Vector<String>&, ErrorHandler*);
		void run_timer(Timer* timer);
		//void push(int, Packet *);
		void addKnownPacket(String &);	
		
		HashTable<String, Timestamp> table;

	private:
		//HashTable<String, Timestamp> buffer;
		Timer _timer;
		
		
};

CLICK_ENDDECLS
#endif
