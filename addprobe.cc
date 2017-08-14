// addprobe.cc

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

#include "addprobe.hh"
#include "MulticastProcess.hh"

CLICK_DECLS

AddProbe::AddProbe()
{
}

AddProbe::~AddProbe()
{
}

int
AddProbe::configure(Vector<String> &conf, ErrorHandler *errh)
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
AddProbe::push(int, Packet *p){	
	assert(p);
	WritablePacket *packet = p->uniqueify();
	assert(packet);

	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp_hdr = reinterpret_cast<click_udp *> (ip + 1);
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp_hdr +1); 
	mulhdr->is_probe = true; 
	output(0).push(packet);
}

CLICK_ENDDECLS

EXPORT_ELEMENT(AddProbe);

