//statistic_infobase.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/ipaddress.hh>

#include "statistic_infobase.hh"

CLICK_DECLS

StatisticInfoBase::StatisticInfoBase()
  : _timer(this)
{
}

StatisticInfoBase::~StatisticInfoBase()
{
}

int
StatisticInfoBase::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh, 
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,
		cpEnd) < 0) 
	{
		//cout << "This is error from configure";
		return -1;
	}
	return 0;
}

int
StatisticInfoBase::initialize(ErrorHandler *)
{
	_receiverTable = new HashTable<int, receiver_stats_data >;

	_timer.initialize(this);   // Initialize timer object (mandatory).
	_timer.schedule_after_sec(EXPERIMENT_PERIOD);

	return 0;
}

void 
StatisticInfoBase::uninitialize()
{
	delete _receiverTable;
}

HashTable<int, receiver_stats_data >*
StatisticInfoBase::get_receiver_table()
{
	return _receiverTable;
}

void 
StatisticInfoBase::run_timer(Timer* timer)
{
	print_out_tables();
}

void 
StatisticInfoBase::print_out_tables()
{
	click_chatter("Receiver statistic table: \n");
	for (HashTable<int, receiver_stats_data>::iterator it = _receiverTable->begin(); it; ++it){
		click_chatter("flowid: %d",it.key());
		double average_delay = it.value().e2e_delay.doubleval()/it.value().num_packet_received;
		double average_jitter = it.value().jitter.doubleval()/(it.value().num_packet_received - 1);
		double received_packet = it.value().num_packet_received;
		click_chatter("average delay: %f", average_delay);
		click_chatter("average jitter: %f", average_jitter);
		click_chatter("number of received packet: %f", received_packet);
	}
	click_chatter("Admission infomation: \n");

	HashTable<int, flow_member_data*> * _fTable= _flowMemberInfo->get_flow_member_set();

	int status;
	for (HashTable<int, flow_member_data*>::iterator it = _fTable->begin(); it; ++it){
		click_chatter("flowid: %d",it.key());
		flow_member_data* fd = it.value();
		//click_chatter("The number of packet sent from source of this flow:%d",fd->count);

		status = 0; // rejected = 0, admitted = 1, probing=2
	
		if(fd->record.size() == 0){
			status = 0; // rejected
			click_chatter("This flow is rejected");
		} 
		else {
			//click_chatter("record size: %d",fd->record.size());
			for(int i=0; i < fd->record.size(); i++){
				if(fd->record[i].status == MCOM_STATUS_PROBE){
					status = 2; // probing
					//click_chatter("This flow is probing");
				}
			}
			if(status != 2){
				status = 1 ;// admitted
				click_chatter("This flow is admitted");
				for(int j=0; j < fd->record.size(); j++){
					click_chatter("receiver address: %s",fd->record[j].receiver_addr.unparse().c_str());
					click_chatter("status of receiver: %d",fd->record[j].status);
				}	

			}
		}
	}
	
	click_chatter("ending of statistic");
}


CLICK_ENDDECLS

EXPORT_ELEMENT(StatisticInfoBase);

