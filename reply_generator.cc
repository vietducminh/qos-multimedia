// reply_generator.cc

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

#include "/home/viet/code/click-2.0.1/elements/local/reply_generator.hh"

CLICK_DECLS

MCOMReplyGenerator::MCOMReplyGenerator()
  : _timer(this)
{
}

MCOMReplyGenerator::~MCOMReplyGenerator()
{
}

int
MCOMReplyGenerator::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"SOURCE_GROUP_ADDRESS", 0, cpIPAddress, &_sourceGroupAddr, 	
	
		"REPLY_TYPE", 0, cpInteger, &_type, 	
		cpEnd) < 0) 
	{
		click_chatter("This is error from configure");
		return -1;
	
	}
	num_entry = 1;
	_id = 0;

	return 0;
}

int
MCOMReplyGenerator::initialize(ErrorHandler *)
{
}

void
MCOMReplyGenerator::run_timer(Timer *)
{
}

Packet *
MCOMReplyGenerator::generate_reply()
{
	Vector<reply_info> entry_list; 

	if((_type == MCOM_REPLY_RECEIVER_TO_SOURCE_MESSAGE)||(_type == MCOM_REPLY_RECEIVER_TO_MPR_MESSAGE))
		num_entry = 1;
	else if(_type == MCOM_REPLY_MPR_TO_SOURCE_MESSAGE)
		num_entry = num_entry;
	else
		click_chatter("There is an error with type of reply");

  	int packet_size = sizeof(click_ip) + sizeof(click_udp) + sizeof(reply_pkt_hdr) + sizeof(reply_info) * num_entry;
	int headroom = sizeof(click_ether);
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
	
	reply_pkt_hdr *reply_hdr = (reply_pkt_hdr *) (udp + 1);
	reply_hdr->_type = _type;
	reply_hdr->num_entries = entry_list.size();

	reply_info * r_info = (reply_info *) (reply_hdr + 1);

	Vector<reply_info>::iterator iter = entry_list.begin();

	while(iter != entry_list.end() ){
		r_info->addr = iter->addr;
		r_info->value = iter->value;
		r_info++;
		++iter;
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
	ip->ip_dst = _sourceGroupAddr;
	packet->set_dst_ip_anno(IPAddress(_sourceGroupAddr));
  
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
	udp->uh_sport = 2803;
	udp->uh_dport = 2803;
	uint16_t len = packet->length() - sizeof(click_ip);
	udp->uh_ulen = htons(len);
	udp->uh_sum = 0;
	unsigned csum = click_in_cksum((unsigned char *)udp, len);
	udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
  
	return packet;

}

CLICK_ENDDECLS

EXPORT_ELEMENT(MCOMReplyGenerator);

