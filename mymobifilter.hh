// mymobifilter.hh

/*
	Element filter received packets based on a given matrix
*/

#ifndef CLICK_MYMOBIFILTER_HH
#define CLICK_MYMOBIFILTER_HH
#include <click/element.hh>
#include <click/hashtable.hh>
#include <click/etheraddress.hh>


CLICK_DECLS

#define MAX 20
class MyMobiFilter : public Element { 
	public:
		MyMobiFilter();
		~MyMobiFilter();
		
		const char *class_name() const	{ return "MyMobiFilter"; }
		const char *port_count() const	{ return "1/2"; }
		const char *processing() const	{ return PUSH; }
		int configure(Vector<String>&, ErrorHandler*);
		
		void push(int, Packet *);
	private:
		//IPAddress _myAddr;
		EtherAddress _myEtherAddr;

		Vector <IPAddress> _ipAddrList;
		Vector <EtherAddress> _etherAddrList;

		
		int mobiMatrix[MAX][MAX]; // this is mobility matrix with index is the index of nodes.
				//the value 1 means that two node is neighbor, if value 0 mean that two node is not neighbor 



};

CLICK_ENDDECLS
#endif
