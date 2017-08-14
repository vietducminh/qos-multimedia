// JitterFlood.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <time.h>
#include "JitterFlood.hh"
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;


CLICK_DECLS
JitterFlood::JitterFlood()
{
	srand(time(NULL));
}

JitterFlood::~ JitterFlood()
{}

int JitterFlood::configure(Vector<String> &conf, ErrorHandler *errh)
{
        	if (cp_va_kparse(conf, this, errh,"WAIT", cpkP, cpInteger, &wait,cpEnd) < 0) 
	{	
		return -1;	
	}	
	return 0;
}

void JitterFlood::callback(Timer*, void *data)
{
	Userdata* udata = (Userdata*) data;
	assert(udata);
	Packet *p = udata->packet;
	
	udata->jf->expire(udata);
	//delete udata;
}

void JitterFlood::expire(Userdata *udata)
{
	Packet *p = udata->packet;
	output(0).push(p);
  	//click_chatter("\n Expire2");
	delete udata;
}
void JitterFlood::push(int port, Packet *p)
{
  	assert(port == 0);
	//click_chatter("\n Packet is pushed");
	WritablePacket *_packet = p->uniqueify();
	assert(_packet);

	Userdata* udata = new Userdata();
	assert(udata);
	udata->jf = this;
	udata->packet = _packet;
	
	Timer* timer = new Timer(&JitterFlood::callback, udata);
	assert(timer);
	timer->initialize(this);
	uint32_t num = (uint32_t)((double) wait * rand()/(RAND_MAX+1.0));
  	//click_chatter("\n Num: %d", num);
	timer->schedule_after_msec(num); // the function callback() will be called after num second (when the timer fires)

}

CLICK_ENDDECLS
EXPORT_ELEMENT(JitterFlood)
