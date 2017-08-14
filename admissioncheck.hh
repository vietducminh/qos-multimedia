// admissioncheck.hh

/* 
	Element checks the admission of a flow after an Admisison period at source
*/

#ifndef CLICK_ADMISSION_CHECK_HH
#define CLICK_ADMISSION_CHECK_HH
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
class AdmissionCheck : public Element { 
	public:
		AdmissionCheck();
		~AdmissionCheck();
		
		const char *class_name() const	{ return "AdmissionCheck"; }
		const char *port_count() const	{ return "1/1"; }
		const char *processing() const	{ return PUSH; }

		int configure(Vector<String>&, ErrorHandler*);
		void push(int, Packet *);
		int initialize(ErrorHandler *);
		void uninitialize();
		void run_timer(Timer* timer);
		
	private:
	
		FlowMemberInfoBase* _flowMemberInfo;	
		IPAddress _myAddr;		
		Timer _timer;
		int _flowid;
		RatedSource* _ratedSourceElement;
		int count; // if count ==0, it means this is the first packet go through this element
};


CLICK_ENDDECLS
#endif
