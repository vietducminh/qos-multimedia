// JitterFlood.hh

/*
	Element adds jitter into a flooding packet
*/

#ifndef CLICK_JITTERFLOOD_HH
#define CLICK_JITTERFLOOD_HH
#include <click/element.hh>
#include <click/timer.hh>
#include <clicknet/ip.h>
#include <click/error.hh>
#include <stdio.h>


CLICK_DECLS

class JitterFlood;

class JitterFlood : public Element { 
	public:
		JitterFlood();
		~JitterFlood();
		
		const char *class_name() const	{ return "JitterFlood"; }
		JitterFlood *clone() const        { return new JitterFlood; }
		const char *port_count() const	{ return "1/1"; }
		const char *processing() const	{ return PUSH; }
		//int initialize(ErrorHandler *);

		int configure(Vector<String>&, ErrorHandler*);

		
		void push(int, Packet *);

	private:		
  		
  		int32_t wait; 
		
		struct Userdata {
			JitterFlood * jf;
			Packet *packet;
		};
		static void callback(Timer*, void * );
		void expire(Userdata*);
		
};


CLICK_ENDDECLS
#endif
