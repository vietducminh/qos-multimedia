// join_request_process.hh

#ifndef JOIN_REQUEST_PROCESS_HH
#define JOIN_REQUEST_PROCESS_HH
#include <click/element.hh>
#include <click/timer.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <click/error.hh>
#include <stdio.h>
#include <clicknet/udp.h>
#include <click/timestamp.hh>
 
#include "/home/viet/code/click-2.0.1/elements/olsr/olsr_neighbor_infobase.hh"
#include "/home/viet/code/click-2.0.1/elements/olsr/olsr_lineariplookup.hh"
#include "/home/viet/code/click-2.0.1/elements/olsr/olsr_topology_infobase.hh"

#include "click_mcom.hh"
#include "flow_member_infobase.hh"
#include "MulticastProcess.hh"

CLICK_DECLS


class JoinRequestProcess : public Element { 
	public:
		JoinRequestProcess();
		~JoinRequestProcess();
		
		const char *class_name() const	{ return "JoinRequestProcess"; }
		const char *port_count() const	{ return "1/2"; }
		const char *processing() const	{ return PUSH; }
		//int initialize(ErrorHandler *);

		int configure(Vector<String>&, ErrorHandler*);
		void push(int, Packet *);
		int initialize(ErrorHandler *);
		void uninitialize();
		bool is_neighbor(IPAddress node);
		bool is_unicasted_receiver(HashTable<IPAddress,Vector<IPAddress> > lastMPRDestMap, IPAddress);
		HashTable<IPAddress,Vector<IPAddress> > build_lastMPRDestMap(Vector <IPAddress> addrlist);
		void send_probe_reply(int flowid, IPAddress _receiverAddr);
	private:		
		IPAddress _myAddr;
		FlowMemberInfoBase* _flowMemberInfo;
		OLSRNeighborInfoBase *_neighborInfo;
  		OLSRTopologyInfoBase *_topologyInfo;
  		OLSRLinearIPLookup *_linearIPlookup;

		atomic_uint32_t _id;
};


CLICK_ENDDECLS
#endif
