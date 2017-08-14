
elementclass OLSRnode {
 $my_ip0, $my_ether0, $hello_period, $tc_period, $mid_period, $jitter, $n_hold, $t_hold, $m_hold, $d_hold |

forward::OLSRForward($d_hold, duplicate_set, neighbor_info, interface_info, interfaces, $my_ip0)
interface_info::OLSRInterfaceInfoBase(routing_table, interfaces)
interfaces::OLSRLocalIfInfoBase($my_ip0)

joinInput::Join

//**************** WRITING DOWN Interfaces *******************
//******************* Interface 0 **************************

in0::FromDevice(eth0)
todevice0::BandwidthShaper(2 Mbps)
	->ToDevice(eth0)

//in0::FromDevice(wlan0)
//todevice0::ToDevice(wlan0)

drr::DWRRSched(7, 5, 3)

ps::PrioSched 
	-> todevice0
drr -> [1]ps

sched0:: Queue(50) // class 1 = 50*20 bytes = 1KB
	-> [0]ps

sched1:: Queue(50) // class 2 = 50*330 bytes = 16.5KB
	-> [0]drr

sched2:: Queue(100) // class 3 = 100*200bytes = 20KB
	-> [1]drr

sched3:: Queue(200) // best effort = 100*512 bytes = 51.2 KB
	-> [2]drr


out0::Classifier(12/0800,12/0806,-) 

class_classifier::IPClassifier(ip dscp 46,ip dscp 34,ip dscp 26, -)
class_join::Join
ps_join::Join

out0[0]
	-> class_classifier

class_classifier[0] // Class EF
	-> check_probe0::CheckProbe()

check_probe0[0] //  probe packet // this is probe
	-> IPPrint("This is a EF probe")
	-> RED(15, 40, 0.05)
	-> sched0
check_probe0[1] // data packet
	-> IPPrint("This is a EF data")
	-> ps_join

ps_join
	-> RED(40, 48, 0.05)
	-> sched0

class_classifier[1] // Class AF4
	-> check_probe1::CheckProbe()

check_probe1[0]
	-> RED(15, 40, 0.05)	
	-> IPPrint("This is a AF4 probe")
	-> sched1
check_probe1[1]
	-> RED(40, 48, 0.05)
	-> IPPrint("This is a AF4 data")
	-> sched1

class_classifier[2] // Class AF3
	-> check_probe2::CheckProbe()

check_probe2[0]
	-> RED(30, 80, 0.05)
	-> sched2
check_probe2[1]
	-> RED(80, 96, 0.05)
	-> sched2

class_classifier[3] // Class Best-effort
	//-> IPPrint("Best effort")
	-> class_join

out0[1]
	-> ps_join

out0[2]
	-> class_join

class_join
	//-> RED(40, 70, 0.05)
	-> sched3

joindevice0::Join
 -> out0
//************** WRITING DOWN ARP *************************
//**************** ARP For Interface 0 *************

c0::Classifier(12/0806 20/0001, 12/0806 20/0002, 12/0800, -);
in0-> SetTimestamp
 -> HostEtherFilter($my_ether0, DROP_OWN false, DROP_OTHER true)
 ->c0

ar0 :: ARPResponder($my_ip0 $my_ether0)
c0[0] -> ar0
 ->joindevice0

arp0::ARPQuerier($my_ip0, $my_ether0);
c0[1]->[1]arp0
arp0
	//-> Print("ARP Querier")
	->joindevice0
c0[2]->Paint(0)
 ->[0]joinInput;
c0[3]->Discard

//********************** Write Output for OLSR *******************
output0::Join
 -> OLSRAddPacketSeq($my_ip0)
 -> UDPIPEncap($my_ip0 , 698, 255.255.255.255, 698)
 -> SetIPDSCP(46)
 -> EtherEncap(0x0800, $my_ether0 , ff:ff:ff:ff:ff:ff)
 -> [0]joindevice0;
//********************** OLSR HELLO *******************
hello_generator0::OLSRHelloGenerator($hello_period, $n_hold, link_info, neighbor_info, interface_info, forward, $my_ip0, $my_ip0)
 ->output0

//********************** IP Classifier *******************
// the first is olsr packet, second is SANITY packet, third is data packet or(All other IP data packets)
//ip_classifier::IPClassifier(udp port 698,udp port 654, -)
ip_classifier::IPClassifier(udp port 698,udp port 2803,udp port 53764,udp port 34048,udp port 2567, udp port 56071, tcp port 5000, udp port 0205,-) 
				// OLSR packets, MCOM packets, SMF packets, REPLY packets
			
tun::KernelTun($my_ip0/24)

tun
	//->IPPrint("From Tun")
	->CheckIPHeader
	//-> Print("After check ip header")
	-> CheckIPHeader(CHECKSUM false)
	//-> IPPrint("from Tun: go to ip classifier")
	-> ip_classifier;


//********************** JoinInput *******************
//joinInput
// -> MarkEtherHeader
// -> Strip(14)
// -> CheckIPHeader(CHECKSUM false)
// -> ip_classifier

my_mobi_filter::MyMobiFilter(MY_ETHER_ADDRESS $my_ether0)

	//c0[2]	-> Paint(0)
	joinInput
		-> MarkEtherHeader
		//-> Print("Entering mobi filter")
		-> my_mobi_filter

	my_mobi_filter[0]
		//-> Print("mobi filter: discarded")
		-> Discard;

	my_mobi_filter[1]
		-> Strip(14)
		-> CheckIPHeader(CHECKSUM false)
		//-> IPPrint("mobi filter: go to ip classifier")
		-> ip_classifier;

//********************** Dublicate SET *******************
duplicate_set::OLSRDuplicateSet
//********************** OLSR Classifier *******************
olsrclassifier::OLSRClassifier(duplicate_set, interfaces, $my_ip0)
check_header::OLSRCheckPacketHeader(duplicate_set)
get_src_addr::GetIPAddress(12)
ip_classifier[0]
 -> get_src_addr
 -> Strip(28)
 -> check_header 
 -> olsrclassifier

check_header[1]
 -> Discard

join_fw::Join

//***************** olsr control message handling *****************
//neighbor_info::OLSRNeighborInfoBase(routing_table, tc_generator, hello_generator0, link_info, interface_info, $my_ip0, interfaces,ADDITIONAL_HELLO false);
neighbor_info::OLSRNeighborInfoBase(routing_table, tc_generator, hello_generator0, link_info, interface_info, $my_ip0, ADDITIONAL_HELLO false);

topology_info::OLSRTopologyInfoBase(routing_table);
link_info::OLSRLinkInfoBase(neighbor_info, interface_info, duplicate_set, routing_table,tc_generator)

association_info::OLSRAssociationInfoBase(routing_table);
routing_table::OLSRRoutingTable(neighbor_info, link_info, topology_info, interface_info, interfaces, association_info, linear_ip_lookup, $my_ip0);

process_hna::OLSRProcessHNA(association_info, neighbor_info, routing_table, $my_ip0);
process_hello::OLSRProcessHello($n_hold, link_info, neighbor_info, interface_info, routing_table, tc_generator, interfaces, $my_ip0);
process_tc::OLSRProcessTC(topology_info, neighbor_info, interface_info, routing_table, $my_ip0);
process_mid::OLSRProcessMID(interface_info, routing_table);

//print_routes::PrintRoutes(LINEAR_IP_LOOKUP linear_ip_lookup);

olsrclassifier[0]
 -> Discard

olsrclassifier[1]
// -> print_routes
 -> process_hello
 -> Discard

olsrclassifier[2]
 //-> IPPrint("A TC is received")
 -> process_tc

olsrclassifier[3]
 -> process_mid

olsrclassifier[4]
 -> process_hna

olsrclassifier[5]->Discard;

process_hna[0]->[2]join_fw
process_hna[1]-> Discard

process_tc[0] ->[0]join_fw
process_tc[1]-> Discard

process_mid[0]->[1]join_fw
process_mid[1]->Discard

join_fw -> [0]forward;
forward[0]->dup::Tee(1);
dup[0]->output0;

forward[1]-> Discard
//linear_ip_lookup::OLSRLinearIPLookup()
mid_generator::OLSRMIDGenerator($mid_period, $m_hold,interfaces)
tc_generator::OLSRTCGenerator($tc_period, $t_hold, neighbor_info, $my_ip0, ADDITIONAL_TC false)

joinforward::Join
tc_generator
 ->[0]joinforward
 ->[1]forward;

mid_generator
 -> [1]joinforward;

//************** PATHLET TO BE DONE ********************************
//pathlet::SanityPathlet(172.16.1.2)

//************** NO SANITY ********************************
//link_info->Discard;
//ip_classifier[1]->Discard
//ip_classifier[2]->Discard

linear_ip_lookup::OLSRLinearIPLookup()

//multicast_proc::MulticastProcess(MY_IP_ADDRESS $my_ip0,ROUTING_TABLE linear_ip_lookup);
	
flow_requirement_info::FlowRequirementInfoBase();
flow_member_info::FlowMemberInfoBase();


InsertFlowRequirement(MY_IP_ADDRESS $my_ip0, FLOW_ID 9, PACKET_LOSS 0.01, 
				E2E_DELAY 150, JITTER 30, REAL_DATARATE 45.0, FLOW_REQUIREMENT_INFOBASE flow_requirement_info);

InsertFlowMember(FLOW_ID 9, SOURCE_ID 192.168.1.74, 
				RECEIVER_IP_ADDRESS_LIST 192.168.1.58, FLOW_MEMBER_INFOBASE flow_member_info);


InsertFlowRequirement(MY_IP_ADDRESS $my_ip0, FLOW_ID 8, PACKET_LOSS 0.01, 
				E2E_DELAY 150, JITTER 30, REAL_DATARATE 40.0, FLOW_REQUIREMENT_INFOBASE flow_requirement_info);

InsertFlowMember(FLOW_ID 8, SOURCE_ID 192.168.1.80, 
				RECEIVER_IP_ADDRESS_LIST 192.168.1.73, FLOW_MEMBER_INFOBASE flow_member_info);


InsertFlowRequirement(MY_IP_ADDRESS $my_ip0, FLOW_ID 7, PACKET_LOSS 0.01, 
				E2E_DELAY 100, JITTER 30, REAL_DATARATE 50.0, FLOW_REQUIREMENT_INFOBASE flow_requirement_info);

InsertFlowMember(FLOW_ID 7, SOURCE_ID 192.168.1.88, 
				RECEIVER_IP_ADDRESS_LIST 192.168.1.73 192.168.1.72, FLOW_MEMBER_INFOBASE flow_member_info);

InsertFlowRequirement(MY_IP_ADDRESS $my_ip0, FLOW_ID 6, PACKET_LOSS 0.01, 
				E2E_DELAY 100, JITTER 30, REAL_DATARATE 50.0, FLOW_REQUIREMENT_INFOBASE flow_requirement_info);

InsertFlowMember(FLOW_ID 6, SOURCE_ID 192.168.1.75, 
				RECEIVER_IP_ADDRESS_LIST 192.168.1.56 192.168.1.80, FLOW_MEMBER_INFOBASE flow_member_info);

InsertFlowRequirement(MY_IP_ADDRESS $my_ip0, FLOW_ID 5, PACKET_LOSS 0.01, 
			E2E_DELAY 100, JITTER 30, REAL_DATARATE 50.0, FLOW_REQUIREMENT_INFOBASE flow_requirement_info);

InsertFlowMember(FLOW_ID 5, SOURCE_ID 192.168.1.58, 
				RECEIVER_IP_ADDRESS_LIST 192.168.1.72 192.168.1.73, FLOW_MEMBER_INFOBASE flow_member_info);

InsertFlowRequirement(MY_IP_ADDRESS $my_ip0, FLOW_ID 4, PACKET_LOSS 0.01, 
				E2E_DELAY 150, JITTER 30, REAL_DATARATE 40.0, FLOW_REQUIREMENT_INFOBASE flow_requirement_info);

InsertFlowMember(FLOW_ID 4, SOURCE_ID 192.168.1.80, 
				RECEIVER_IP_ADDRESS_LIST 192.168.1.72 192.168.1.73, FLOW_MEMBER_INFOBASE flow_member_info);


InsertFlowRequirement(MY_IP_ADDRESS $my_ip0, FLOW_ID 3, PACKET_LOSS 0.01, 
				E2E_DELAY 150, JITTER 30, REAL_DATARATE 40.0, FLOW_REQUIREMENT_INFOBASE flow_requirement_info);

InsertFlowMember(FLOW_ID 3, SOURCE_ID 192.168.1.74, 
				RECEIVER_IP_ADDRESS_LIST 192.168.1.80 192.168.1.58, FLOW_MEMBER_INFOBASE flow_member_info);

InsertFlowRequirement(MY_IP_ADDRESS $my_ip0, FLOW_ID 2, PACKET_LOSS 0.01, 
				E2E_DELAY 150, JITTER 30, REAL_DATARATE 45.0, FLOW_REQUIREMENT_INFOBASE flow_requirement_info);

InsertFlowMember(FLOW_ID 2, SOURCE_ID 192.168.1.73, 
				RECEIVER_IP_ADDRESS_LIST 192.168.1.56 192.168.1.58, FLOW_MEMBER_INFOBASE flow_member_info);
InsertFlowRequirement(MY_IP_ADDRESS $my_ip0, FLOW_ID 1, PACKET_LOSS 0.01, 
				E2E_DELAY 150, JITTER 30, REAL_DATARATE 45.0, FLOW_REQUIREMENT_INFOBASE flow_requirement_info);

InsertFlowMember(FLOW_ID 1, SOURCE_ID 192.168.1.72, 
				RECEIVER_IP_ADDRESS_LIST 192.168.1.56 192.168.1.58 192.168.1.80, FLOW_MEMBER_INFOBASE flow_member_info);


buffer::Buffer()
mcom_proc::MCOMProcess(MY_IP_ADDRESS $my_ip0,LINEAR_IP_LOOKUP linear_ip_lookup, TOPOLOGY_INFOBASE topology_info, 
				ROUTING_TABLE routing_table, BUFFER buffer,NEIGHBOR_INFOBASE neighbor_info, FLOW_MEMBER_INFOBASE flow_member_info);	
smf_proc::SMFProcess(MY_IP_ADDRESS $my_ip0,OLSR_TCGENERATOR tc_generator,LINEAR_IP_LOOKUP linear_ip_lookup, BUFFER buffer,FLOW_MEMBER_INFOBASE flow_member_info);
reply_proc::ReplyProcess(MY_IP_ADDRESS $my_ip0,FLOW_MEMBER_INFOBASE flow_member_info)

//admission_statistic::AdmissionStatistic(MY_IP_ADDRESS $my_ip0,FLOW_MEMBER_INFOBASE flow_member_info)
//mcom_statistic::MCOMStatistic(MY_IP_ADDRESS $my_ip0,ADMISSION_STATISTIC admission_statistic)
//mcom_statistic::MCOMStatistic(MY_IP_ADDRESS $my_ip0)

statistic_info::StatisticInfoBase(FLOW_MEMBER_INFOBASE flow_member_info)

receiver_statistic::ReceiverStatistic(STATISTIC_INFOBASE statistic_info)

dst_classifier::IPClassifier(dst $my_ip0, -);
get_dst_addr::GetIPAddress(16)
	
ip_classifier[1]
	//-> Print("Entering multicast_proc")
	-> mcom_proc
	
ip_classifier[2]
	-> MarkIPHeader
	-> smf_proc

// the packet with port 34048 is sent to ip_classifier[3]
ip_classifier[3]
	-> dst_classifier

//dst_classifier[0]
	//->SetPacketType(HOST)
	//-> IPPrint("To Tun")
	//->tun

dst_classifier[0]
	-> IPPrint("Entering reply process")
	-> reply_proc

reply_proc[0]
	-> Discard;
reply_proc[1]
	-> GetIPAddress(16)
	-> SetIPDSCP(46)
	-> [0]linear_ip_lookup

// the node is forwarder for the reply
dst_classifier[1]
	-> MarkIPHeader
	-> get_dst_addr
	-> [0]linear_ip_lookup 

linear_ip_lookup[0]
	-> [0]arp0

dst_classifier_tcp::IPClassifier(dst $my_ip0, -);

ip_classifier[7]
	-> udp_classifier::IPClassifier(dst $my_ip0, -)
udp_classifier[0]
	-> udpGroundCount::Counter
	-> Discard;
udp_classifier[1]
	-> MarkIPHeader
	-> GetIPAddress(16)
	-> linear_ip_lookup

ip_classifier[8]
	-> Discard

ip_classifier[6]
	->dst_classifier_tcp
dst_classifier_tcp[0]
	//-> IPPrint("To Tun")
	-> tun
dst_classifier_tcp[1]
	-> [0]linear_ip_lookup

mcom_proc[0]
	//-> IPPrint("packet is expired")
	->Discard // packet is expired
mcom_proc[1]
	//->IPPrint("Node0: Packet is sent to application layer")	
	-> tee::Tee(3)
tee[0]
	//-> ToIPSummaryDump(results/mcom/mcomAppDump.txt, CONTENTS  timestamp ip_src ip_dst ip_len ip_proto ip_id ip_ttl
	//	sport dport count first_timestamp)
	//-> EtherEncap(0x0800, 1:1:1:1:1:1, 0:1:2:3:4:5)
	//->SetPacketType(HOST)
	//-> IPPrint("To Tun")
	//->tun
	-> Discard

tee[1]
	-> probe_measurement_mcom::ProbeMeasurement(MY_IP_ADDRESS $my_ip0, PACKET_TYPE 1, 
				FLOW_MEMBER_INFOBASE flow_member_info,FLOW_REQUIREMENT_INFOBASE flow_requirement_info) 
	
probe_measurement_mcom[0] // send a reply to the source
	-> MarkIPHeader
	-> GetIPAddress(16)
	-> SetIPDSCP(46)
	-> linear_ip_lookup	

probe_measurement_mcom[1]
	-> Discard
	
tee[2]
	//-> ecn_process
	//-> mcom_statistic
	-> receiver_statistic

mcom_proc[2]	
	->IPPrint("before: Packet is sent to network device")
	-> StoreIPAddress(dst)
	//->IPPrint("after: Packet is sent to network device")
	-> [0]arp0 // copies of packets are sent to other next hops

mcom_proc[3]
	-> Print("Packet is broadcast as SMF")		
	-> StoreIPAddress(dst)
	//-> UDPIPEncap($my_ip0 , 1234, 255.255.255.255, 1234)
	-> EtherEncap(0x0800, $my_ether0 , ff:ff:ff:ff:ff:ff)
	-> JitterFlood(WAIT 20)
	-> [0]joindevice0;

smf_proc[0]		
	//-> Print("SMF packet is discarded")
	-> Discard
smf_proc[1]
	//->IPPrint("SMF packet is sent to application layer")
	-> smfAppCount::Counter
	-> tee_smf::Tee(3)

tee_smf[0]
	//-> ToIPSummaryDump(results/mcom/smfAppDump.txt, CONTENTS  timestamp ip_src ip_dst ip_len ip_proto ip_id ip_ttl
	//	sport dport count first_timestamp)
	-> EtherEncap(0x0800, 1:1:1:1:1:1, 0:1:2:3:4:5)
	-> Discard
tee_smf[1]
	-> probe_measurement_smf::ProbeMeasurement(MY_IP_ADDRESS $my_ip0, PACKET_TYPE 2, 
			FLOW_MEMBER_INFOBASE flow_member_info,FLOW_REQUIREMENT_INFOBASE flow_requirement_info)
probe_measurement_smf[0]	
	-> MarkIPHeader
	-> GetIPAddress(16)
	-> SetIPDSCP(46)
	-> linear_ip_lookup

probe_measurement_smf[1]
	-> Discard

tee_smf[2]
	//-> mcom_statistic
	//-> Discard
	-> receiver_statistic

smf_proc[2]
	//->Print("SMF packet is rebroadcast")
	-> StoreIPAddress(dst)
	//->UDPIPEncap($my_ip0 , 1234, 255.255.255.255, 1234)
	-> EtherEncap(0x0800, $my_ether0 , ff:ff:ff:ff:ff:ff)
	-> JitterFlood(WAIT 20)
	-> [0]joindevice0;

join_request_proc::JoinRequestProcess(MY_IP_ADDRESS $my_ip0, FLOW_MEMBER_INFOBASE flow_member_info, 
			NEIGHBOR_INFOBASE neighbor_info, TOPOLOGY_INFOBASE topology_info, LINEAR_IP_LOOKUP linear_ip_lookup)
join_probe_reply_proc::JoinProbeReplyProcess(MY_IP_ADDRESS $my_ip0, FLOW_MEMBER_INFOBASE flow_member_info)


ip_classifier[4]
	-> dst_classifier_join::IPClassifier(dst $my_ip0, -)

dst_classifier_join[0]
	-> IPPrint("Entering join request process")
	-> join_request_proc

join_request_proc[0]
	-> GetIPAddress(16)
	-> [0]linear_ip_lookup 

join_request_proc[1]
	-> Discard;

// the node is forwarder for the join request
dst_classifier_join[1]
	-> MarkIPHeader
	-> get_dst_addr
	-> [0]linear_ip_lookup 

// the packet is probe reply sent from source
ip_classifier[5]
	-> dst_classifier_join_reply::IPClassifier(dst $my_ip0, -)

dst_classifier_join_reply[0]
	-> IPPrint("Entering join probe reply process")
	-> join_probe_reply_proc
	-> Discard
	//-> probe_measurement_smf

dst_classifier_join_reply[1]
	-> MarkIPHeader
	-> get_dst_addr
	-> [0]linear_ip_lookup 

r1::RatedSource(DATA \<0800>, RATE 45, LENGTH 330, ACTIVE false)
	//-> Print("rated source 1")
	-> ratedCounter1::Counter	
	-> AddReceiver(MY_IP_ADDRESS $my_ip0, IP_ADDRESS_LIST 192.168.1.56 192.168.1.58 192.168.1.80)
	-> UDPIPEncap($my_ip0, 2803, 255.255.255.255, 2803)
	-> SetTimestamp
	-> AddFlow(FLOW_ID 1,FLOW_MEMBER_INFOBASE flow_member_info)
	-> AdmissionCheck(MY_IP_ADDRESS $my_ip0, FLOW_MEMBER_INFOBASE flow_member_info, RATED_SOURCE r1)
	-> tee_source_1::Tee(2)

tee_source_1[0]
	-> SetIPECN(1)
	-> SetIPDSCP(34)
	-> mcom_proc	

tee_source_1[1]
	-> Discard



r10::RatedSource(DATA \<0800>, RATE 1000, LENGTH 512, ACTIVE false)
	-> ratedCounter10::Counter
	//-> AddReceiver(MY_IP_ADDRESS $my_ip0, IP_ADDRESS_LIST 192.168.1.58)
	-> UDPIPEncap($my_ip0, 0205, 192.168.1.58, 0205)
	-> SetTimestamp
	//-> AddFlow(FLOW_ID 0,FLOW_MEMBER_INFOBASE flow_member_info)
	-> SetIPDSCP(0)
	-> MarkIPHeader
	-> GetIPAddress(16)
	//-> IPPrint("UDP background")
	-> linear_ip_lookup
	//-> mcom_proc



//DriverManager(wait 10s, save c.count -, stop)

// wait sometime before sending data from RatedSource, then after a specific time this click script stops

	Script(wait 15,
		write r10.active true,

		wait 2,
		//write r6.active true,
		//write r8.active true,
		wait 1,
		//write r7.active true,
		wait 1,
		//write r5.active true,
		wait 1,
		//write r3.active true,
		wait 2,
		//write r4.active true,
		wait 1,
		write r1.active true,
		//write r9.active true,
		wait 1,
		//write r2.active true,



		wait 70,
		write r1.active false,
		write r10.active false,

		wait 60, //... = 152. In 150s the system print the statistic
		print > results/statCount.txt ratedCounter10.count,
		print >> results/statCount.txt ratedCounter1.count,
			
		stop);

} // END OLSR COMPONENT ELEMENT


AddressInfo(my_ether0 eth0:eth);
AddressInfo(my_ip0 192.168.1.72);
//AddressInfo(my_ip0 129.241.205.72);


//AddressInfo(my_ether0 wlan0);
//AddressInfo(my_ip0 192.168.1.101);


u::OLSRnode(my_ip0, my_ether0, 1500, 5000, 5000, -1, 4500, 15000, 15000, 30);
//u::OLSRnode(my_ip0, my_ether0, 1500, 5000, 5000, 0.375, 4500,15000, 15000, 30); 


	
