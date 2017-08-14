// SMFProcess.hh

/*
	Element processes SMF packets 
*/

#ifndef CLICK_SMFPROCESS_HH
#define CLICK_SMFPROCESS_HH
#include <click/element.hh>
#include <clicknet/ip.h>
#include <click/timer.hh>
#include <click/hashtable.hh>
#include <click/error.hh>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "mcomprocess.hh"
//#include "destinationlist.hh"

using namespace std;

#define EXPIRE_INTERVAL 5.0
#define NETWORK_DIAMETER 16

CLICK_DECLS

class SMFProcess : public Element { 
	public:
		SMFProcess();
		~SMFProcess();
		
		const char *class_name() const	{ return "SMFProcess"; }
		const char *port_count() const	{ return "1/3"; }
		const char *processing() const	{ return PUSH; }
		//int initialize(ErrorHandler *) ;
		int configure(Vector<String>&, ErrorHandler*);
		//void run_timer(Timer* timer);
		void push(int, Packet *);
		//void addKnownSMF(String &);	
		
	private:
		Buffer* _buffer;
		OLSRTCGenerator * _tcGenerator;
		OLSRLinearIPLookup *_linearIPlookup;
		FlowMemberInfoBase* _flowMemberInfo;
  	
		Timer _timer;
		IPAddress _myAddr;
		
};

CLICK_ENDDECLS
#endif
