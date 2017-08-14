# qos-multimedia
The software develop an efficient framework to provide quality of service for multicast real time applications in wireless networks.
It includes mechanisms to increase the quality of real time traffic received at recipients in mobile ad hoc networks. With multicast real time applications, source(s) sends a flow traffic to multiple recipients at the same time (e.g., video conference, multiplayer game).
The software is built on the platform Click Modular Router and run in Linux.  

Main functionalities of the software: 
+ Including a light signaling protocol (admission control) to establish sessions from source(s) to multiple recipients
+ Detecting congestion and quickly deal with it to avoid overload in the network
+ Finding effective routes from source(s) to multiple recipients. Recovering broken links automatically
+ Managing the join or leave of a new recipient without affecting the quality of existing sessions.

Important files (components):
+	mcomprocess{.h,.cc}
+	smfprocess{.h,.cc}
+	ecnprocess{.h,.cc}
+	replyprocess{.h,.cc}
+	cngenerator{.h,.cc}
+	join_request_process{.h,.cc}
+	mcomstatistic{.h,.cc}
+	probemeasurement{.h,.cc}

Viet Thi Minh Do
Norwegian University of Science and Technology
Email: vietdtm83@gmail.com
