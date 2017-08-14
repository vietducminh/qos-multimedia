//flow_member_infobase.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/ipaddress.hh>

#include "flow_member_infobase.hh"

CLICK_DECLS

FlowMemberInfoBase::FlowMemberInfoBase()
//  : _timer(this)
{
}

FlowMemberInfoBase::~FlowMemberInfoBase()
{
}

int
FlowMemberInfoBase::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if ( cp_va_parse(conf, this, errh, 
		   0) < 0 )
    return -1;
  return 0;
}

int
FlowMemberInfoBase::initialize(ErrorHandler *)
{
 	//_flowMemberSet = new FlowMemberSet();
	_flowMemberSet = new HashTable<int, flow_member_data*>;
	//_timer.initialize(this);
	
	return 0;
}

void 
FlowMemberInfoBase::uninitialize()
{
	delete _flowMemberSet;
}

bool
FlowMemberInfoBase::add_flow_member(int flowid, IPAddress source_addr, Vector<receiver_status> _record)
{
	struct flow_member_data *fd;
	fd = new struct flow_member_data;		

	fd->flowid = flowid;
	fd->source_addr = source_addr;
	fd->count = 0;
	fd->record.clear();
	Vector<receiver_status>::iterator iter = _record.begin();
	receiver_status temp;
	while(iter != _record.end() ){
		temp.receiver_addr = iter->receiver_addr;
		temp.status = iter->status;
		temp.time = iter->time;
		fd->record.push_back(temp);
		iter++;
	}
	
	//if(! _flowMemberSet ->empty()){
	//	for (HashMap<int, void*>::iterator iter = _flowMemberSet->begin(); iter; iter++){
	//		click_chatter("flow member add");
	//	}
	//}

	return ( _flowMemberSet->find_insert(flowid, fd) );
}


flow_member_data *
FlowMemberInfoBase::find_flow_member(int flowid)
{
  if (! _flowMemberSet->empty() ) {
    flow_member_data *data;
    data = _flowMemberSet->get(flowid);

    if (! data == 0 )
      return data;
  }
  return 0;
}


void
FlowMemberInfoBase::remove_flow_member(int flowid)
{
	flow_member_data *ptr=_flowMemberSet->get(flowid);
	_flowMemberSet->erase(flowid);
	delete ptr;
}


bool
FlowMemberInfoBase::update_flow_member(int flowid, IPAddress source_addr, Vector<receiver_status> record){
	flow_member_data *data;
	data = find_flow_member(flowid);
	if ( ! data == 0 ) {
		data->flowid = flowid;
		data->source_addr = source_addr;
		data->record.clear();
		Vector<receiver_status>::iterator iter = record.begin();
		receiver_status temp;
		while(iter != record.end() ){
			temp.receiver_addr = iter->receiver_addr;
			temp.status = iter->status;
			temp.time = iter->time;
			data->record.push_back(temp);
		}

		return true;
	}
	return false;
}


HashTable<int, flow_member_data*> *
FlowMemberInfoBase::get_flow_member_set()
{
  return _flowMemberSet;
}


//void
//FlowMemberInfoBase::run_timer(Timer *)
//{
 
//}

void 
FlowMemberInfoBase::print_flow_member()
{
}
 

CLICK_ENDDECLS

EXPORT_ELEMENT(FlowMemberInfoBase);

