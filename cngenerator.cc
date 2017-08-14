// cngenerator.cc

/*	Viet Thi Minh Do 
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

#include "cngenerator.hh"
#include "click_mcom.hh"
#include "MulticastProcess.hh"

CLICK_DECLS

CNGenerator::CNGenerator()
{

}

CNGenerator::~CNGenerator()
{
}

int
CNGenerator::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 

		cpEnd) < 0) 
	{
		click_chatter("This is error from configure");
		return -1;
	
	}
	
	return 0;
}
	
void 
CNGenerator::push(int, Packet *p)
{
	assert(p);
	WritablePacket *p_new = p->uniqueify();
	assert(p_new);

	assert(p_new->has_network_header());
	click_ip *ip_data = p_new->ip_header();
	click_udp *udp_data = p_new->udp_header();
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp_data +1); 
	IPAddress _sourceAddr = IPAddress(ip_data->ip_src); // this is source address of the marked packet

	int packet_size = sizeof(congest_noti_pkt_hdr);
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
	
	congest_noti_pkt_hdr *congest_hdr = (congest_noti_pkt_hdr *) (udp + 1);
	congest_hdr->flowid = mulhdr->flowid;
	congest_hdr->size = mulhdr->size;
	congest_hdr->congested_node = _myAddr;
	for(int i=0; i<mulhdr->size; i++){
		congest_hdr->array[i] = mulhdr->array[i].addr;
	}

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
	udp->uh_sport = 2014;
	udp->uh_dport = 2014;
	uint16_t len = packet->length() - sizeof(click_ip);
	udp->uh_ulen = htons(len);
	udp->uh_sum = 0;
	unsigned csum = click_in_cksum((unsigned char *)udp, len);
	udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
  
	p->kill(); // kill the data packet
	output(0).push(packet);
}

CLICK_ENDDECLS

EXPORT_ELEMENT(CNGenerator);

