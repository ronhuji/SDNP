############################ INPUT ############################
"""
bad input for graph 1221 that creates an unreachable FW. It is unreachable 
only for SomeMarketing to SomeDBs
"""

DBS_IP_CONTAINING = "192.0.0.0/16"
DBS_IP_CONTAINED = "192.0.3.0/24"
MARKETING_IP_CONTAINING = "192.168.0.0/16"
MARKETING_IP_CONTAINED = "192.168.3.0/24"


FUNCTIONS_AND_LOCATIONS = {}
LABELS = {}
IP_LOCATIONS = {}

FUNCTIONS_AND_LOCATIONS["FW"] = [0]
FUNCTIONS_AND_LOCATIONS["SensitiveLink"] = [(0,234)]
FUNCTIONS_AND_LOCATIONS["DPI"] = [13]

LABELS["AllMarketing"] = [MARKETING_IP_CONTAINING]
LABELS["SomeMarketing"] = [MARKETING_IP_CONTAINED]
LABELS["AllDBs"] = [DBS_IP_CONTAINING]
LABELS["SomeDBs"] = [DBS_IP_CONTAINED]

IP_LOCATIONS[MARKETING_IP_CONTAINING] = 1
IP_LOCATIONS[DBS_IP_CONTAINING] = 55

POLICIES = []
POLICIES.append("dst_port=80:!SomeMarketing->SensitiveLink->AllDBs")
POLICIES.append("*:AllMarketing->FW->SomeDBs")

ADDITIONAL_POLICIES = []
ADDITIONAL_POLICIES.append("src_port=25:AllMarketing->DPI->AllDBs")