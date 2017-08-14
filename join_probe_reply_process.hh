// join_probe_reply_process.hh

/*
	Element processes the received reply for a join probe packet
*/

#ifndef JOIN_PROBE_REPLY_PROCESS_HH
#define JOIN_PROBE_REPLY_PROCESS_HH
#include <click/element.hh>
#include <click/timer.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <stdio.h>
#include <clicknet/udp.h>
#include <click/timestamp.hh>
 
#include "click_mcom.hh"
#include "flow_member_infobase.hh"

CLICK_DECLS


class JoinProbeReplyProcess : public Element { 
	public:
		JoinProbeReplyProcess();
		~JoinProbeReplyProcess();
		
		const char *class_name() const	{ return "JoinProbeReplyProcess"; }
		const char *port_count() const	{ return "1/1"; }
		const char *processing() const	{ return PUSH; }
		//int initialize(ErrorHandler *);

		int configure(Vector<String>&, ErrorHandler*);
		void push(int, Packet *);
	private:		
		IPAddress _myAddr;
		FlowMemberInfoBase* _flowMemberInfo;
};


CLICK_ENDDECLS
#endif
