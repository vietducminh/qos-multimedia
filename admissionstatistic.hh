// admissionstatistic.hh

/*
	Element checks the admission of a flow 
*/

#ifndef CLICK_ADMISSION_STATISTIC_HH
#define CLICK_ADMISSION_STATISTIC_HH
#include <click/element.hh>
#include <click/timer.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <stdio.h>
#include <clicknet/udp.h>
#include <click/timestamp.hh>
#include <click/handlercall.hh>
#include <elements/standard/ratedsource.hh>

#include "click_mcom.hh"
#include "flow_member_infobase.hh"
#include "MulticastProcess.hh"

CLICK_DECLS
class AdmissionStatistic : public Element { 
	public:
		AdmissionStatistic();
		~AdmissionStatistic();
		
		const char *class_name() const	{ return "AdmissionStatistic"; }
		const char *port_count() const	{ return "1/0"; }
		const char *processing() const	{ return PUSH; }

		int configure(Vector<String>&, ErrorHandler*);
		void push(int, Packet *);
		int initialize(ErrorHandler *);
		void uninitialize();
		HashTable<int, int > * get_admission_table();
		
	private:
		HashTable<int, int > *_admissionTable;
	
		FlowMemberInfoBase* _flowMemberInfo;	
		IPAddress _myAddr;		
		struct Userdata {
			AdmissionStatistic * admissionStatistic;
			int flowid;	
		};
		static void callback(Timer*, void * );
		void expire(Userdata*);
};


CLICK_ENDDECLS
#endif
