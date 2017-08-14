// probemeasurement.hh
/*
	Element measures quality of received probes 
	(in delay, jitter, packet loss, number of packet received)
*/

#ifndef CLICK_PROBEMEASUREMENT_HH
#define CLICK_PROBEMEASUREMENT_HH
#include <click/element.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <clicknet/udp.h>
#include <click/timestamp.hh>
#include <click/timer.hh>
#include <click/hashtable.hh>
#include <click/error.hh>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <click/hashtable.hh>
#include <click/timestamp.hh>
#include <sys/time.h>

#include "click_mcom.hh"
#include "flow_requirement_infobase.hh"
#include "MulticastProcess.hh"
#include "flow_member_infobase.hh"

#define PACKET_TYPE_UNICAST 1
#define PACKET_TYPE_SMF 2

using namespace std;

CLICK_DECLS

struct mcom_reply_pkt_hdr{
	int _type; // 0 = OK-reply; 1 = PRUNE-reply
};


class ProbeMeasurement : public Element { 
	public:
		ProbeMeasurement();
		~ProbeMeasurement();
		
		const char *class_name() const	{ return "ProbeMeasurement"; }
		const char *port_count() const	{ return "1/2"; }
		const char *processing() const	{ return PUSH; }
		int configure(Vector<String>&, ErrorHandler*);
		void send_reply_from_receiver(int type, int flowid, IPAddress sourceAddr, IPAddress sentToAddr);		
		void push(int, Packet *);
		int initialize(ErrorHandler *);
		void uninitialize();
		
	private:
		IPAddress _myAddr;
		int _packet_type;
		FlowMemberInfoBase *_flowMemberInfo;
		FlowRequirementInfoBase *_flowRequirementInfo;
		HashTable <int, measured_qos*> *_measurementTable; 
		HashTable<int, flow_data*> *_flowSet;
		
		bool is_probe;
		bool first_probe;
		bool is_receiver;
		int num_packet;
		Timestamp total_delay; 
		int32_t pdr;
		int total_sent_packet;
		IPAddress _sourceAddr;

		struct Userdata {
			ProbeMeasurement * pm;
			int flowid;
		};
		static void callback(Timer*, void * );
		void expire(Userdata*);
		atomic_uint32_t _id;
};

CLICK_ENDDECLS
#endif
