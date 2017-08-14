// replyprocess.hh

/*
	Element processes MCOM replies 
*/

#ifndef CLICK_REPLYPROCESS_HH
#define CLICK_REPLYPROCESS_HH
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

CLICK_DECLS

class FlowMemberInfoBase;
class ReplyProcess;

class ReplyProcess : public Element { 
	public:
		ReplyProcess();
		~ReplyProcess();
		
		const char *class_name() const	{ return "ReplyProcess"; }
		ReplyProcess *clone() const        { return new ReplyProcess; }
		const char *port_count() const	{ return "1/2"; }
		const char *processing() const	{ return PUSH; }
		//int initialize(ErrorHandler *);

		int configure(Vector<String>&, ErrorHandler*);
		void push(int, Packet *);
		int initialize(ErrorHandler *);
		void uninitialize();

		void send_reply_from_MPR(int flowid, IPAddress sourceAddr);
		HashTable<int, decision_table_data* > *_decisionTable;
		FlowMemberInfoBase* _flowMemberInfo;
		//RatedSource* _ratedSourceElement;
	private:		
		IPAddress _myAddr;		
		struct Userdata {
			ReplyProcess * rp;
			int flowid;
			IPAddress sourceAddr;			
		};
		static void callback(Timer*, void * );
		void expire(Userdata*);
		//bool _flow_admission;
		atomic_uint32_t _id;
};


CLICK_ENDDECLS
#endif
