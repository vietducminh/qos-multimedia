// click_mcom.hh
// this file contains the declaration of structs

#ifndef CLICK_MCOM_HH
#define CLICK_MCOM_HH

#include <click/ipaddress.hh>
#include <click/glue.hh>
#include <click/vector.hh>
#include <click/string.hh>
#include <click/ipaddress.hh>

CLICK_DECLS
#define MAX_RECEIVERS 3
#define EXPERIMENT_PERIOD 150 // seconds
#define PROBE_PERIOD 4 // seconds
#define MPR_WAITE_REPLY_PERIOD 1 // seconds
#define SOURCE_WAITE_REPLY_PERIOD 6 // seconds
#define ADMISSION_CHECK_PERIOD 7 // second, the time source wait to check admission of a flow

// value for admissionTable (at AdmissionCheck element)
#define REJECTED_FLOW 0
#define ADMITTED_FLOW 1
#define TEMP_FLOW 2

//REPLY Types
#define MCOM_REPLY_RECEIVER_TO_SOURCE_MESSAGE 1
#define MCOM_REPLY_RECEIVER_TO_MPR_MESSAGE    2
#define MCOM_REPLY_MPR_TO_SOURCE_MESSAGE   3

// Status value of flowMemberInfo
// and the value of field is_probe in multicast_hdr of data packets
#define MCOM_STATUS_PROBE 1
#define MCOM_STATUS_DATA 2
#define MCOM_STATUS_JOIN_PROBE 3
#define MCOM_STATUS_REJECTED 4

// Value of flag field for receiver in the data packet
#define MCOM_UNICASTED_RECEIVER_FLAG 0
#define MCOM_MPR_RECEIVER_FLAG 2
#define MCOM_JOIN_PROBE_RECEIVER_FLAG 3

// REPLY value
#define MCOM_PRUNE_REPLY 0
#define MCOM_OK_REPLY 1

//#define RECEIVER_JOINING 1
//#define RECEIVER_JOINED 2

// if the number of marked packet is larger than this threshold, the source 
// will preempt the part of flow forward the congested node
#define ECN_MARKED_PACKET_THRESHOLD 10 

// Statistic information at receivers
struct receiver_stats_data{
	int num_packet_received;
	Timestamp e2e_delay;
	Timestamp jitter;
	Timestamp previous_transition_time; // the transition time of the perivious packet. Used to calculate the jitter
					// jitter += new_transition_time - previous_transition_time
};

// Statistic information for ECN
struct ecnStatis{
	int flowid;
	IPAddress congested_node;
	int total_marked_packets;	
	Vector <IPAddress > receiver_list;
};

// header of the congestion notification packet
struct congest_noti_pkt_hdr{
	uint16_t flowid;
	IPAddress congested_node;
	uint8_t size; // size of array
	IPAddress array[MAX_RECEIVERS];
	
};

// qos parameters measured at receivers 
struct measured_qos{
	Timestamp e2e_delay;
	Timestamp jitter;
	Timestamp previous_transition_time; // the transition time of the perivious packet. Used to calculate the jitter
						// jitter += new_transition_time - previous_transition_time
	int num_packet_received;
	IPAddress mprAddr;	
	struct timeval time;
};

// decision notification sent from receiver
struct receiver_decision{
	IPAddress receiver_addr;
	int decision; // MCOM_PRUNE_REPLY, MCOM_OK_REPLY
}; 

struct flowid_source{
	int flowid;
	IPAddress source_addr;
	
};

struct decision_table_data{
	//int flow;
	IPAddress source_addr;
	Vector<struct receiver_decision> rd;
};

struct receiver_status{
	IPAddress receiver_addr;
	int status; // three values: PROBE, DATA, JOIN_PROBE
	struct timeval time;
};

struct flow_member_data{
	int flowid;
	IPAddress source_addr;
	int count; // the number of packets has been sent from source
	Vector<struct receiver_status> record;
};

struct member_data{
	IPAddress M_addr;
	int M_status;
	struct timeval M_time;
};	

// structure of a flow
struct flow_data{
	int flowid;
	double packet_loss;
	double e2e_delay;
	double jitter;
	double real_datarate;
	struct timeval F_time;
};

// structure of a group
struct group_data{
	IPAddress G_source_addr;
	int G_group_id;
	struct timeval G_time;
};	

// header of a request
struct request_pkt_hdr{
	int flowid;
	IPAddress source_addr;
	IPAddress receiver_addr;
};

// header of a reply for a join probe
struct join_probe_reply_pkt_hdr{
	int flowid;
	IPAddress receiver_addr;
	int data_status;
};

// header of a normal reply
struct reply_pkt_hdr{
	uint16_t pkt_length;
	uint16_t pkt_seq;			
	uint8_t _type; // to determine the type of reply: from receiver to source, from receiver to MPR, from MPR to source
	uint8_t num_entries; //number of entries with reply_info, or number of receivers stored in the reply
};

struct reply_info{
	int flowid;
	IPAddress source_addr;
	IPAddress addr; // address of a receiver
	int value; // this is to store OK or PRUNE message
			// Note: If the reply is from receiver to MPR or source, then there is one entry for addr and value.
			// If the reply is from MPR to source, then there are several entries of addr and value.
};

/*struct multicast_hdr{
	uint8_t protocolid; // id of the protocol (default 1)
	uint8_t length; // length of header
	uint8_t size; // size of array
	uint16_t flowid;
	uint8_t is_probe;
	addr_flag array[MAX_RECEIVERS]; // list of receiver addresses
};
*/

CLICK_ENDDECLS

#endif
