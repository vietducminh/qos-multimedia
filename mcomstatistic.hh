// mcomstatistic.hh

/*
	Elements provides statistic information
*/

#ifndef CLICK_MCOMSTATISTIC_HH
#define CLICK_MCOMSTATISTIC_HH
#include <click/element.hh>
#include <click/timer.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <stdio.h>
#include <clicknet/udp.h>
#include <click/timestamp.hh>
#include <click/handlercall.hh>
#include <click/hashtable.hh>
#include <elements/standard/ratedsource.hh>

#include "click_mcom.hh"
#include "MulticastProcess.hh"
#include "admissionstatistic.hh"

CLICK_DECLS

class MCOMStatistic : public Element { 
	public:
		MCOMStatistic();
		~MCOMStatistic();
		
		const char *class_name() const	{ return "MCOMStatistic"; }
		const char *port_count() const	{ return "1/0"; }
		const char *processing() const	{ return PUSH; }

		int configure(Vector<String>&, ErrorHandler*);
		void push(int, Packet *);
		void print_out_tables();
		void run_timer(Timer* timer);
		int initialize(ErrorHandler *); 
		void uninitialize();

	private:		
		HashTable<int, int > *_sourceTable;
		HashTable<int, receiver_stats_data > *_receiverTable;
		HashTable<int, int > *_admissionTable;
		AdmissionStatistic * _admissionStatistic;
		Timer _timer;

		IPAddress _myAddr;		
};


CLICK_ENDDECLS
#endif
