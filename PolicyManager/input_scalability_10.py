############################ INPUT ############################
"""
runs with "1221.r0.reachable.graph"
calculates paths from "Broadcast" through 1 MB to "zones" and through 2 MBs to "admins"
additionally calculates dags to all admins

170 zones and admins:
took 7 secs for policy manager
took 9 secs for paths solver on graph 1755                (without writing to files!!)
took 19 secs for paths solver on graph 1221 (reachable)    (without writing to files!!)
took 60 secs for paths solver on graph 1239                (without writing to files!!)

311 zones and admins:
took 25 secs for policy manager
took 35 secs for paths solver on graph 1221 (reachable)      (without writing to files!!)
took 107 secs for paths solver on graph 1239                  (without writing to files!!)

512 zones and admins:
took 62 secs for policy manager
took 183 secs for paths solver on graph 1239

"""

BROADCAST_IP = "1.1.1.0/24"


ZONE_0 = "192.168.0.0/24"
ZONE_1 = "192.168.1.0/24"
ZONE_2 = "192.168.2.0/24"
ZONE_3 = "192.168.3.0/24"
ZONE_4 = "192.168.4.0/24"
ZONE_5 = "192.168.5.0/24"
ZONE_6 = "192.168.6.0/24"
ZONE_7 = "192.168.7.0/24"
ZONE_8 = "192.168.8.0/24"
ZONE_9 = "192.168.9.0/24"

ADMIN_0 = "192.168.0.100/32"
ADMIN_1 = "192.168.1.100/32"
ADMIN_2 = "192.168.2.100/32"
ADMIN_3 = "192.168.3.100/32"
ADMIN_4 = "192.168.4.100/32"
ADMIN_5 = "192.168.5.100/32"
ADMIN_6 = "192.168.6.100/32"
ADMIN_7 = "192.168.7.100/32"
ADMIN_8 = "192.168.8.100/32"
ADMIN_9 = "192.168.9.100/32"

FUNCTIONS_AND_LOCATIONS = {}
LABELS = {}
IP_LOCATIONS = {}

FUNCTIONS_AND_LOCATIONS["FW"] = [0,1,2]
FUNCTIONS_AND_LOCATIONS["COUNTER"] = [3,7,12]
FUNCTIONS_AND_LOCATIONS["SPECIAL_COUNTER"] = [7]

LABELS["Zones"] = [ZONE_0,ZONE_1,ZONE_2,ZONE_3,ZONE_4,ZONE_5,ZONE_6,ZONE_7,ZONE_8,ZONE_9]
LABELS["Admins"] = [ADMIN_0,ADMIN_1,ADMIN_2,ADMIN_3,ADMIN_4,ADMIN_5,ADMIN_6,ADMIN_7,ADMIN_8,ADMIN_9]
LABELS["SpecialAdmins"] = [ADMIN_0,ADMIN_1]
LABELS["Broadcast"] = [BROADCAST_IP] 

IP_LOCATIONS[ZONE_0] = 0
IP_LOCATIONS[ZONE_1] = 1
IP_LOCATIONS[ZONE_2] = 2
IP_LOCATIONS[ZONE_3] = 3
IP_LOCATIONS[ZONE_4] = 4
IP_LOCATIONS[ZONE_5] = 5
IP_LOCATIONS[ZONE_6] = 6
IP_LOCATIONS[ZONE_7] = 7
IP_LOCATIONS[ZONE_8] = 8
IP_LOCATIONS[ZONE_9] = 9

IP_LOCATIONS[BROADCAST_IP] = 10


POLICIES = []
POLICIES.append("*:Broadcast->FW->Zones")
POLICIES.append("dst_port=80:Broadcast->COUNTER->Admins")

ADDITIONAL_POLICIES = []
ADDITIONAL_POLICIES.append("dst_port=80:!Broadcast->SPECIAL_COUNTER->SpecialAdmins")