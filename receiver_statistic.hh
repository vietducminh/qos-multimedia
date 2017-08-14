// receiver_statistic.hh

/*
	Element provides statistic information at receivers
	(delay, jitter, packet loss, ...) 
*/

#ifndef CLICK_RECEIVER_STATISTIC_HH
#define CLICK_RECEIVER_STATISTIC_HH
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
#include "statistic_infobase.hh"

CLICK_DECLS

class ReceiverStatistic : public Element { 
	public:
		ReceiverStatistic();
		~ReceiverStatistic();
		
		const char *class_name() const	{ return "ReceiverStatistic"; }
		const char *port_count() const	{ return "1/0"; }
		const char *processing() const	{ return PUSH; }

		int configure(Vector<String>&, ErrorHandler*);
		void push(int, Packet *);
	private:		
		HashTable<int, receiver_stats_data > *_receiverTable;
		StatisticInfoBase* _statisticInfo;
};


CLICK_ENDDECLS
#endif
