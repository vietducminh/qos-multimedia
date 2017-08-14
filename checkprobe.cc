// checkprobe.cc

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

#include "checkprobe.hh"
#include "MulticastProcess.hh"

CLICK_DECLS

CheckProbe::CheckProbe()
{
}

CheckProbe::~CheckProbe()
{
}

int
CheckProbe::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh, 
		cpEnd) < 0) 
	{
		click_chatter("This is error from configure");
		return -1;
	
	}
	
	return 0;
}


void
CheckProbe::push(int, Packet *p){
	assert(p);
	WritablePacket *packet = p->uniqueify();
	assert(packet);

	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp_hdr = reinterpret_cast<click_udp *> (ip + 1);
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp_hdr +1); 
	if((mulhdr->is_probe == MCOM_STATUS_PROBE)|| (mulhdr->is_probe == MCOM_STATUS_JOIN_PROBE)){
		output(0).push(packet); // this is probe
	}
	else if(mulhdr->is_probe == MCOM_STATUS_DATA){
		output(1).push(packet); // this is data
	}

}

CLICK_ENDDECLS

EXPORT_ELEMENT(CheckProbe);

