############################ INPUT ############################
"""
bad input for graph "test_sequence.graph" that creates a longer path through a FW without creating loops
"""

DST_IP = "192.0.3.0/26"
SRC_IP= "192.168.0.0/16"


FUNCTIONS_AND_LOCATIONS = {}
LABELS = {}
IP_LOCATIONS = {}

FUNCTIONS_AND_LOCATIONS["FW"] = [0,5]
FUNCTIONS_AND_LOCATIONS["DPI"] = [7]
FUNCTIONS_AND_LOCATIONS["NAT"] = [4]


LABELS["Src"] = [SRC_IP]
LABELS["Dst"] = [DST_IP]

IP_LOCATIONS[SRC_IP] = 1
IP_LOCATIONS[DST_IP] = 6

POLICIES = []
POLICIES.append("*:Src->FW->DPI->NAT->Dst")