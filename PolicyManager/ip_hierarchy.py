from netaddr import *
from sets import Set

ALL_IPS = "0.0.0.0/0"
ALL_IPS_NETWORK = IPNetwork(ALL_IPS)

(_PARENT, _SRC_RULES) = range(2)


def add_ip_to_hierarchy(ip_hierarchy_dict, ip_network_src, ip_network_dst):
    if not ip_hierarchy_dict:
        ip_hierarchy_dict[ip_network_src] = [ALL_IPS_NETWORK, Set([ip_network_dst])]
    
    if ip_network_src in ip_hierarchy_dict:
        ip_hierarchy_dict[ip_network_src][_SRC_RULES].add(ip_network_dst)
        return
    
    largest_contained_ip = None
    smallest_containing_ip = ALL_IPS_NETWORK
    for ip in ip_hierarchy_dict:
        if ip in ip_network_src and (largest_contained_ip is None or largest_contained_ip in ip):
            largest_contained_ip = ip
        if ip_network_src in ip and ip in smallest_containing_ip:
            smallest_containing_ip = ip
    
    ip_hierarchy_dict[ip_network_src] = [smallest_containing_ip, Set([ip_network_dst])]
        
    if largest_contained_ip is not None:
        ip_hierarchy_dict[largest_contained_ip][_PARENT] = ip_network_src

        
def get_parents(ip_hierarchy_dict, ip):
    parents = []
    while ip != ALL_IPS_NETWORK:
        parents.append(ip)
        ip = ip_hierarchy_dict[ip][_PARENT]
    return parents
    
    """
def create_dst_containing(containing_dict, dst_ip_hierarchy_dict):
    parent
    for dst in dst_ip_hierarchy_dict:
        containing_dict[dst] = []
    for dst in dst_ip_hierarchy_dict:
        containing_dict[dst] = []
    """