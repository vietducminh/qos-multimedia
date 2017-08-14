//flow_requirement_infobase.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/ipaddress.hh>

#include "flow_requirement_infobase.hh"

CLICK_DECLS

FlowRequirementInfoBase::FlowRequirementInfoBase()
//  : _timer(this)
{
}

FlowRequirementInfoBase::~FlowRequirementInfoBase()
{
}

int FlowRequirementInfoBase::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if ( cp_va_parse(conf, this, errh, 
		   0) < 0 )
    return -1;
  return 0;
}

int FlowRequirementInfoBase::initialize(ErrorHandler *)
{
 	_flowSet = new HashTable<int, flow_data*>;
//	_timer.initialize(this);
	
	return 0;
}

void FlowRequirementInfoBase::uninitialize()
{
	delete _flowSet;
}

bool FlowRequirementInfoBase::add_flow(int flowid, double packet_loss, double e2e_delay, double jitter, double real_datarate)
{
	struct flow_data *fd;
	fd = new struct flow_data;		//new memory released in remove
		
	fd->flowid = flowid;
	fd->packet_loss = packet_loss;
	fd->e2e_delay = e2e_delay;
	fd->jitter = jitter;
	fd->real_datarate = real_datarate;
	//fd->F_time = time;

	//if (_flowSet->empty()) {
	//	_timer.schedule_at(time);
  	//}

	_flowSet->find_insert(flowid, fd);

	//for (HashTable<int, flow_data*>::iterator iter = _flowSet->begin(); iter; iter++){
	//	click_chatter("key: %d", iter.key());
	//}

	return ( _flowSet->find_insert(flowid, fd) );
}

flow_data * FlowRequirementInfoBase::find_flow(int flowid)
{
  if (! _flowSet->empty() ) {
    flow_data *data;
    data = _flowSet->get(flowid);

    if (! data == 0 )
      return data;
  }
  return 0;
}

void FlowRequirementInfoBase::remove_flow(int flowid)
{
	flow_data *ptr= _flowSet->get(flowid);
	_flowSet->erase(flowid);
	delete ptr;
}

bool FlowRequirementInfoBase::update_flow(int flowid, double packet_loss, double e2e_delay, double jitter, double real_datarate){
	flow_data *data;
	data = find_flow(flowid);
	if ( ! data == 0 ) {
		data->flowid = flowid;
		data->packet_loss = packet_loss;
		data->e2e_delay = e2e_delay;
		data->jitter = jitter;
		data->real_datarate = real_datarate;
		return true;
	}
	return false;
}

HashTable<int, flow_data*> * FlowRequirementInfoBase::get_flow_set()
{
	//click_chatter("Flow requirement infobase: get flow set");
	
	//for (HashTable<int, flow_data*>::iterator iter = _flowSet->begin(); iter; iter++){
	//	click_chatter("flow requirement-get flow set");
	//}
	return _flowSet;
}

//void
//FlowRequirementInfoBase::run_timer(Timer *)
//{
 
//}

void FlowRequirementInfoBase::print_flows()
{
	//click_chatter ("Flow Infobase: %d\n",_flowSet->size());
	for (HashTable<int, flow_data*>::iterator iter = _flowSet->begin(); iter; iter++){
		//flow_data *tuple = (flow_data *) iter.value();
		//click_chatter ("\tFlowid=%d\tPacket loss=%d\tE2E delay=%d\t\n",tuple->flowid,tuple->packet_loss, tupe->packet_delay);
	}
}
 
CLICK_ENDDECLS

EXPORT_ELEMENT(FlowRequirementInfoBase);

