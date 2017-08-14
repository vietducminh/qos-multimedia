// join_probe_reply_process.cc

/*	Viet Thi Minh Do, Lars Landmark, Øivind Kure 
*	Norwegian University of Science and Technology 
*/

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include <fstream>

#include "join_probe_reply_process.hh"

using namespace std;

CLICK_DECLS

JoinProbeReplyProcess::JoinProbeReplyProcess()
{} 

JoinProbeReplyProcess::~JoinProbeReplyProcess()
{}

int JoinProbeReplyProcess::configure(Vector<String> &conf, ErrorHandler *errh) 
{
	if (cp_va_kparse(conf, this, errh, 
		"MY_IP_ADDRESS", 0, cpIPAddress, &_myAddr, 
		"FLOW_MEMBER_INFOBASE",0,cpElement,&_flowMemberInfo,
		
		cpEnd) < 0) 
	{
		cout << "This is error from configure";
		return -1;
	
	}

	return 0;
}

void JoinProbeReplyProcess::push(int port, Packet *p)
{
	assert(port == 0);
	WritablePacket *packet = p->uniqueify();
	assert(packet);

	assert(packet->has_network_header());
	click_ip *ip = packet->ip_header();
	click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);
	join_probe_reply_pkt_hdr *reply_hdr = reinterpret_cast<join_probe_reply_pkt_hdr *> (udp + 1);
	int flowid = reply_hdr->flowid;
	HashTable<int, flow_member_data*> * _fTable = _flowMemberInfo->get_flow_member_set();
	//HashTable<int, flow_member_data*>::iterator iter = _fTable->find(flowid);

	// this node become a PROBE receiver
	// if this flow is not exist in the flowMemberInfo, this flow will be added
	if(_fTable->size() == 0){
		flow_member_data* fdata = new flow_member_data;
		fdata->flowid = flowid;
		//fdata->source_addr = IPAddress(ip.ip_src);
		receiver_status temp;
		temp.receiver_addr = _myAddr;
		temp.status = reply_hdr->data_status;
		fdata->record.push_back(temp);
		_fTable->find_insert(flowid, fdata);
	}
	else{ // else the flow will be updated, this node is added in the record
		flow_member_data* fd = _fTable->get(flowid);
		if (fd == NULL){
			fd = new flow_member_data;
		}
		receiver_status temp;
		temp.receiver_addr = _myAddr;
		temp.status = reply_hdr->data_status;
		fd->record.push_back(temp);
	}
	output(0).push(packet);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(JoinProbeReplyProcess)

