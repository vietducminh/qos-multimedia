// join_request_generator.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/udp.h>
#include <clicknet/ether.h>
#include <click/ipaddress.hh>
#include <click/router.hh>
#include <click/vector.hh>

#include "join_request_generator.hh"

CLICK_DECLS

JoinRequestGenerator::JoinRequestGenerator()
:_timer(this)
{

}

JoinRequestGenerator::~JoinRequestGenerator()
{
}

int 
JoinRequestGenerator::initialize(ErrorHandler *) 
{
	_timer.initialize(this);   // Initialize timer object (mandatory).
	//_timer.schedule_now(); 
	_timer.schedule_after_sec(_tJoin);
	return 0;
}

int
JoinRequestGenerator::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"FLOW_ID", 0, cpInteger, &_flowid, 
		"SOURCE_ADDR", 0, cpIPAddress, &_sourceAddr, 
		"JOIN_AFTER_MSEC", 0, cpInteger, &_tJoin, 

		cpEnd) < 0) 
	{
		click_chatter("This is error from configure");
		return -1;
	
	}
	
	return 0;
}

void 
JoinRequestGenerator::run_timer(Timer* timer)
{
	output(0).push(generate_join_request());

}
	
Packet* 
JoinRequestGenerator::generate_join_request()
{
	_receiverAddr = _myAddr;
	int packet_size = sizeof(request_pkt_hdr)*3;
	int headroom = sizeof(click_ip) + sizeof(click_udp) + sizeof(click_ether);
	int tailroom = 0; 
	WritablePacket *packet = Packet::make(headroom,0,packet_size, tailroom);
	if ( packet == 0 ){
		click_chatter( "in %s: cannot make packet!", name().c_str());
	}
	memset(packet->data(), 0, packet->length());
	struct timeval tv;
	click_gettimeofday(&tv);
	packet->set_timestamp_anno(tv);

	click_ip *ip = reinterpret_cast<click_ip *>(packet->data());
	click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);
	
	request_pkt_hdr *request_hdr = (request_pkt_hdr *) (udp + 1);
	request_hdr->flowid = _flowid;
	request_hdr->source_addr = _sourceAddr;
	request_hdr->receiver_addr = _myAddr;
	//click_chatter("\n request generator - receiverAddr: %s",request_hdr->receiver_addr.unparse().c_str());


#if !HAVE_INDIFFERENT_ALIGNMENT
	assert((uintptr_t)ip % 4 == 0);
#endif
	// set up IP header
	ip->ip_v = 4;
	ip->ip_hl = sizeof(click_ip) >> 2;
	ip->ip_len = htons(packet->length());
	ip->ip_id = htons(_id.fetch_and_add(1));
	ip->ip_p = IP_PROTO_UDP;
	ip->ip_src = _myAddr;
	ip->ip_dst = _sourceAddr;
	packet->set_dst_ip_anno(IPAddress(_sourceAddr));
  
	ip->ip_tos = 0;
	ip->ip_off = 0;
	ip->ip_ttl = 250;

	ip->ip_sum = 0;
#if HAVE_FAST_CHECKSUM && FAST_CHECKSUM_ALIGNED
	if (_aligned)
		ip->ip_sum = ip_fast_csum((unsigned char *)ip, sizeof(click_ip) >> 2);
	else
		ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));
#elif HAVE_FAST_CHECKSUM
	ip->ip_sum = ip_fast_csum((unsigned char *)ip, sizeof(click_ip) >> 2);
#else
	ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));
#endif

	packet->set_ip_header(ip, sizeof(click_ip));

	// set up UDP header
	udp->uh_sport = 1802;
	udp->uh_dport = 1802;
	uint16_t len = packet->length() - sizeof(click_ip);
	udp->uh_ulen = htons(len);
	udp->uh_sum = 0;
	unsigned csum = click_in_cksum((unsigned char *)udp, len);
	udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
  
	return packet;
}

CLICK_ENDDECLS

EXPORT_ELEMENT(JoinRequestGenerator);

