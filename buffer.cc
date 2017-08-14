// buffer.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "buffer.hh"
#define MAX 100

using namespace std;

CLICK_DECLS

Buffer::Buffer(): _timer(this)
{}
Buffer::~ Buffer()
{}

int Buffer::initialize(ErrorHandler *) 
{
     	_timer.initialize(this);   // Initialize timer object (mandatory).
     	_timer.schedule_now();     // Set the timer to fire as soon as the
                                // router runs.
     	return 0;
 }


int Buffer::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
	//"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
	//"OLSR_TCGENERATOR", 0, cpElement,&_tcGenerator,
	//"LINEAR_IP_LOOKUP",0,cpElement,&_linearIPlookup,	
	cpEnd) < 0) return -1;
	return 0;

}


void Buffer::addKnownPacket(String & key)
{
	Timestamp now = Timestamp::now();
	Timestamp interval(0.0);
	interval.set_sec(5.0);
	interval.set_subsec(0.0);

	now += interval; // the entry is expired after 5s
	bool b = table.set(key,now);
}

void Buffer::run_timer(Timer* timer)
{
	assert(timer == &_timer);
	Timestamp now = Timestamp::now();	
	for (HashTable<String, Timestamp>::iterator it = table.begin(); it; ){
		Timestamp expire = it.value();
		if (now >= expire){		// value of the hashtable is expired time
			it = table.erase(it);
		}
		else {
			it++;
		}

	}
	
	_timer.reschedule_after_sec(5); // Fire again EXPIRE_INTERVAL seconds later
}


CLICK_ENDDECLS
EXPORT_ELEMENT(Buffer)

