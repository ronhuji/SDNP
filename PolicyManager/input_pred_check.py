############################ INPUT ############################
"""
bad input for graph 1221 that creates an unreachable FW. It is unreachable only for dst_port=80 
from Marketing1
"""

DBS_IP = "192.0.3.0/26"
MARKETING_IP_CONTAINING = "192.168.0.0/16"
MARKETING_IP_CONTAINED = "192.168.3.0/24"


FUNCTIONS_AND_LOCATIONS = {}
LABELS = {}
IP_LOCATIONS = {}

FUNCTIONS_AND_LOCATIONS["FW"] = [0]
FUNCTIONS_AND_LOCATIONS["SensitiveLink"] = [(0,234)]

#LABELS["Marketing"] = [MARKETING_IP_CONTAINING, MARKETING_IP_CONTAINED]
LABELS["Marketing1"] = [MARKETING_IP_CONTAINING]
LABELS["Marketing2"] = [MARKETING_IP_CONTAINED]
LABELS["DBs"] = [DBS_IP]

IP_LOCATIONS[MARKETING_IP_CONTAINING] = 1
#IP_LOCATIONS[MARKETING_IP_CONTAINED] = 16
IP_LOCATIONS[DBS_IP] = 55

POLICIES = []
POLICIES.append("dst_port=80:!Marketing2->SensitiveLink->DBs")
POLICIES.append("*:Marketing1->FW->DBs")

ADDITIONAL_POLICIES = []