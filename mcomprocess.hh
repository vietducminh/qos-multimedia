// mcomprocess.hh

/*
	Element processes MCOM packets
*/

#ifndef CLICK_MCOMPROCESS_HH
#define CLICK_MCOMPROCESS_HH
#include <click/element.hh>
#include <click/hashtable.hh>
#include <clicknet/udp.h>

#include <click/bighashmap.hh>
#include<click/hashmap.hh>
#include "/home/viet/code/click-2.0.1/elements/olsr/olsr_lineariplookup.hh"
#include "/home/viet/code/click-2.0.1/elements/olsr/olsr_topology_infobase.hh"

#include "buffer.hh"
#include "MulticastProcess.hh"
#include "click_mcom.hh"
#include "flow_member_infobase.hh"


CLICK_DECLS

//#define MAX 30

//struct addr_flag{
//	IPAddress addr;
//	uint8_t flag;
//};


//struct multicast_hdr{
//	uint8_t protocolid;
//	uint8_t length;	
//	uint8_t size; // size of array
//	addr_flag array[MAX];
	
//};


class MCOMProcess : public Element  { 
	public:
		MCOMProcess();
		~MCOMProcess();
		
		const char *class_name() const	{ return "MCOMProcess"; }
		const char *port_count() const	{ return "1/4"; }
		const char *processing() const	{ return PUSH; }
		int configure(Vector<String>&, ErrorHandler*);
		void sourceProcess(WritablePacket*, Vector <addr_flag> );
		void forwarderProcess(WritablePacket*, Vector <addr_flag> );
	
		HashTable<IPAddress,Vector<IPAddress> > build_lastMPRDestMap(Vector <addr_flag>);
		Vector<addr_flag> build_MPR_Dest_List(HashTable<IPAddress,Vector<IPAddress> > , Vector<IPAddress> );
		HashTable<IPAddress,Vector<addr_flag> > build_nextHopDestMap(Vector<addr_flag> );
		void source_send_packet(WritablePacket*,HashTable<IPAddress,Vector<addr_flag> >);
		void forwarder_send_packet(WritablePacket*,HashTable<IPAddress,Vector<addr_flag> >);

		void forward(WritablePacket*,Vector <addr_flag>);
		void send_SMF(WritablePacket *,Vector <addr_flag> &);
		bool is_neighbor(IPAddress);
		
		void push(int, Packet *);
		
	private:
		IPAddress _myAddr;
		Buffer *_buffer;
		int _flowid;
  		OLSRLinearIPLookup *_linearIPlookup;
  		OLSRTopologyInfoBase *_topologyInfo;
		OLSRRoutingTable *_routingTable;
		FlowMemberInfoBase* _flowMemberInfo;
		OLSRNeighborInfoBase *_neighborInfo;
};

CLICK_ENDDECLS
#endif
