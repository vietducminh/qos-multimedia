// probemeasurement.cc

/*	Viet Thi Minh Do 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "probemeasurement.hh"

using namespace std;

CLICK_DECLS
ProbeMeasurement::ProbeMeasurement()

{} 

ProbeMeasurement::~ ProbeMeasurement()
{}


int
ProbeMeasurement::initialize(ErrorHandler *)
{
	_measurementTable = new HashTable<int, measured_qos*>;	
	_flowSet = new HashTable<int, flow_data *>;
 
	return 0;
}

void 
ProbeMeasurement::uninitialize()
{
	delete _measurementTable;
	delete _flowSet;
}


int ProbeMeasurement::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 	
		"PACKET_TYPE", 0, cpInteger, &_packet_type, 		
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,
		"FLOW_REQUIREMENT_INFOBASE",0,cpElement,&_flowRequirementInfo,

		cpEnd) < 0) 
	{
		cout << "This is error from configure";
		return -1;
	
	}
	is_probe = false;
	is_receiver = false;

	return 0;

}

void ProbeMeasurement::push(int port, Packet *p)
{
	assert(port == 0);
	WritablePacket *packet = p->uniqueify();
	assert(packet);
	
	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp = packet->udp_header();
	multicast_hdr *mulhdr = reinterpret_cast<multicast_hdr *> (udp +1); 
	int _flowid = mulhdr->flowid;

	HashTable<int, flow_member_data*> * fdTable =  _flowMemberInfo->get_flow_member_set();
	//click_chatter("is_probe: %d",mulhdr->is_probe);

	
	// the flow is inserted in the flowMemberInfo table
	//if(mulhdr->is_probe == MCOM_STATUS_JOIN_PROBE){
	if(fdTable->size() == 0){
		flow_member_data* fdata = new flow_member_data;
		fdata->flowid = _flowid;
		fdata->source_addr = IPAddress(ip->ip_src);
		receiver_status temp;
		temp.receiver_addr = _myAddr;
		temp.status = MCOM_STATUS_PROBE;
		fdata->record.push_back(temp);
		fdTable->find_insert(_flowid,fdata);
		//click_chatter("Flowid has been inserted in flowMemberInfo");
	}
	
	flow_member_data* fd = fdTable->get(_flowid);
	//click_chatter("size of fd record: %d",fd->record.size());

	for(int i=0; i<fd->record.size(); i++){

		if(_myAddr == fd->record[i].receiver_addr){
			is_receiver = true;
		}
	}	
	// updating the measurement table if this is a receiver of this flow
	if (is_receiver == true){	
  		HashTable<int, measured_qos*>::iterator iter = _measurementTable->find(_flowid);
		Timestamp now = Timestamp::now();
		Timestamp time_packet_sent;
		time_packet_sent =  packet->timestamp_anno();

		// this flow does not exist in the measurement table, this flow will be added in this table and a Timer is created for this flow
		if(iter == NULL){
			measured_qos* temp = new measured_qos;
			temp->num_packet_received = 1;
			//temp->jitter = 0;
			temp->e2e_delay = now - time_packet_sent;
			temp->previous_transition_time = now - time_packet_sent;

			//click_chatter("now: %f",now.doubleval());
			//click_chatter("time sent: %f",time_packet_sent.doubleval());
			//click_chatter("e2e_delay: %f",temp->e2e_delay.doubleval());

			if(_packet_type == MCOM_REPLY_RECEIVER_TO_MPR_MESSAGE){
				temp->mprAddr = mulhdr->array[0].addr;
			}
			_measurementTable->find_insert(_flowid,temp);

			Userdata* udata = new Userdata();
			assert(udata);
			udata->pm = this;
			udata->flowid = _flowid;
	
			Timer* timer = new Timer(&ProbeMeasurement::callback, udata);
			assert(timer);
			timer->initialize(this);
			uint32_t num = PROBE_PERIOD; // 4000ms = 4s
			timer->schedule_after_sec(num); // the function callback() will be called after num second (when the timer fires)
			
		}
		else{
			measured_qos* temp = iter.value();
			temp->e2e_delay = temp->e2e_delay + now - time_packet_sent;
			
			if((now - time_packet_sent - temp->previous_transition_time) > 0)
				temp->jitter = temp->jitter + (now - time_packet_sent - temp->previous_transition_time);
			else
				temp->jitter = temp->jitter - (now - time_packet_sent - temp->previous_transition_time);

			temp->previous_transition_time = now - time_packet_sent;
			temp->num_packet_received++;
			_measurementTable->replace(_flowid,temp);
		}	
	
	}
	output(1).push(packet);

	//packet->kill();
	
}


void ProbeMeasurement::callback(Timer*, void *data)
{
	Userdata* udata = (Userdata*) data;
	assert(udata);
	udata->pm->expire(udata);
}

void ProbeMeasurement::expire(Userdata *udata)
{
	click_chatter(" expire - probe measurement");
	
	// find the id of the flow
	int flowid = udata->flowid;

	IPAddress sentToAddr;
	IPAddress sourceAddr;

	// find the quality requirement of the flow in the FlowRequirementInfoBase
	HashTable<int, flow_data*> *flow_set = _flowRequirementInfo->get_flow_set();
	struct flow_data * fd = flow_set->get(flowid);

	// find the quality of the flow in the measurement table
	HashTable<int, measured_qos*>::iterator iter = _measurementTable->find(flowid);
	measured_qos* mq = iter.value();
	double measured_packet_loss = 1 - mq->num_packet_received/(fd->real_datarate*PROBE_PERIOD);
	Timestamp avg_e2e_delay = mq->e2e_delay/mq->num_packet_received;
	Timestamp avg_jitter = mq->jitter/(mq->num_packet_received -1);
	double measured_avg_e2e_delay = avg_e2e_delay.doubleval() ; // 
	double measured_avg_jitter = avg_jitter.doubleval();

	click_chatter("measured average e2e delay: %f",measured_avg_e2e_delay);
	click_chatter("measured average jitter: %f",measured_avg_jitter);
	click_chatter("number of packet received: %d",mq->num_packet_received);
	
	HashTable<int, flow_member_data*> * fdTable =  _flowMemberInfo->get_flow_member_set();
	flow_member_data* fm = fdTable->get(flowid);
	sourceAddr = fm->source_addr;

	// if this flow is unicasted to this receiver, the reply will be sent to source
	// find the source address of the flow

	if(_packet_type == MCOM_REPLY_RECEIVER_TO_SOURCE_MESSAGE){
		sentToAddr = fm->source_addr;
	} 	
	// if this flow is smf to this receiver, the reply will be sent to MPR
	else if (_packet_type == MCOM_REPLY_RECEIVER_TO_MPR_MESSAGE){
		sentToAddr = mq->mprAddr;
	}		

	// compare the measured quality of flow and the flow requirement
	// if the qos of probe is satisfied
	if((measured_avg_e2e_delay <= fd->e2e_delay)&&(measured_packet_loss <= fd->packet_loss) && (measured_avg_jitter <= fd->jitter)){ 
	//if(measured_packet_loss <= fd->packet_loss){ 
		send_reply_from_receiver(MCOM_OK_REPLY,flowid,sourceAddr,sentToAddr); // send OK reply
	}
	else{
		send_reply_from_receiver(MCOM_PRUNE_REPLY,flowid,sourceAddr,sentToAddr);  // send PRUNE reply
	}
	
	delete udata;
}


//Packet*
void
ProbeMeasurement::send_reply_from_receiver(int type, int flowid, IPAddress sourceAddr, IPAddress sentToAddr)
{

	// ok reply: type = 0; PRUNE reply: type = 1
	int num_entry = 5;
		
  	int packet_size = sizeof(reply_pkt_hdr) + sizeof(reply_info) * num_entry;
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
	
	reply_pkt_hdr *reply_hdr = (reply_pkt_hdr *) (udp + 1);
	// this is reply from receiver
	if(_packet_type == MCOM_REPLY_RECEIVER_TO_SOURCE_MESSAGE)
		reply_hdr->_type = MCOM_REPLY_RECEIVER_TO_SOURCE_MESSAGE;
	else if(_packet_type == MCOM_REPLY_RECEIVER_TO_MPR_MESSAGE)
		reply_hdr->_type = MCOM_REPLY_RECEIVER_TO_MPR_MESSAGE;
	reply_hdr->num_entries = 1;

	reply_info * r_info = (reply_info *) (reply_hdr + 1);

	r_info->flowid = flowid;
	r_info->source_addr = sourceAddr;
	r_info->addr = _myAddr;
	r_info->value =  type;

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
	ip->ip_dst = sentToAddr;
	packet->set_dst_ip_anno(IPAddress(sentToAddr));
  
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
	udp->uh_sport = 0205;
	udp->uh_dport = 0205;
	uint16_t len = packet->length() - sizeof(click_ip);
	udp->uh_ulen = htons(len);
	udp->uh_sum = 0;
	unsigned csum = click_in_cksum((unsigned char *)udp, len);
	udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
  
	//return packet;
	output(0).push(packet);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(ProbeMeasurement)

