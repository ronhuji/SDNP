import itertools
from netaddr import *
import pdb
import sys
import datetime
import copy
import re

from input_pred_check import *





############################ CONSTANTS/MAGICS ############################
WILDCARD = 2
d = 2 #number of fields
DECISION_INDEX = d

"""ARROW = "->"
AND = " and "
OR = " or "
NEGATION = '!'
COLON = ":"
EQUALS_SIGN = "="
ASTERISK = "*"
PREDICATE_SEPERATOR = "--__--"
ALL_PREDICATE = [{'*':'*'}]
DEFAULT_DAG_PATH_EXPRESSION = ARROW

DST_TYPE = "DST"
SRC_DST_TYPE = "SRC_DST"/Users/ronfamini/Google Drive/Visual Studio 2013/Projects/SDNP/PolicyManager/stats.py

PARTIAL_REP_PREDICATE_INDEX = 0
PARTIAL_REP_PRIORITY_INDEX = 1
PARTIAL_REP_PATHS_INDEX = 2
DEFAULT_DAG_PRIORITY = 1

ALL_IP = "0.0.0.0/0"
ALL_IP_NETWORK = IPNetwork(ALL_IP)

LABELS["*"] = [ALL_IP]

OUTPUT_FILE_NAME = "op.txt"


############################ GLOBALS ############################

src_dst_policies = {}
generalized_src_dst_policies = {}
final_policies = []
"""

############################ READING RULES ############################

#DIR_PATH = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\PolicyManager\link_fail\Policies\garr' + '\\'
DIR_PATH = r'link_fail/Policies/garr' + '/'

#GRAPH_PATH = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\archive\Garr200908.graphml.graph'

NUMBER = '100'
print 'number is ' + NUMBER

def create_classifeier_from_rules_file():
    switches_tables = {}
    
    rules_file = open(DIR_PATH + "rules" + NUMBER + ".txt")
    while(True):
    #for x in xrange(20):
        line = rules_file.readline()
        if line == '':
            break
        m = re.search('(?<=priority\=)[0-9]+',line)
        priority = int(m.group(0))
    
        m = re.search('(?<=current_switch\=)[0-9]+',line)
        current_switch = int(m.group(0))
        #if current_switch != 0:
            #continue
        #print node1

        m = re.search('(?<=next_switches\=\[)[0-9,]+', line)
        decision = m.group(0)
        #if m.group(0).count(',') > 1:
            #continue
        
        m = re.search('(?<=src\=)[0-9\./]+', line)
        src_network = IPNetwork(m.group(0))
        src = get_ip_range(src_network)
        src_tbit = get_tbit_representation(src_network)
        #print src
        
        m = re.search('(?<=dst\=)[0-9\./]+', line)
        dst_network = IPNetwork(m.group(0))
        dst = get_ip_range(dst_network)
        dst_tbit = get_tbit_representation(dst_network)
        #print "dst: " + dst

        if not current_switch in switches_tables:
            switches_tables[current_switch] = {}

        if not priority in switches_tables[current_switch]:
            switches_tables[current_switch][priority] = []

        rule = (src_network, dst_network, decision)
        switches_tables[current_switch][priority].append(rule)
        
        #print switches_tables

        #meta_data = line.split('--__--cu')[0]

    #print switches_tables
    for (switch,switch_table) in switches_tables.iteritems():
        print "switch: " + str(switch)
        ordered_priorities = []
        network_classifier = []
        for priority in switch_table:
            ordered_priorities.append(priority)
        ordered_priorities.sort()
        ordered_priorities.reverse()
        for priority in ordered_priorities:
            #print switch_table[priority]
            network_classifier += switch_table[priority]
        range_classifier = []
        tbit_classifier = []
        for network_rule in network_classifier:
            range_classifier.append((get_ip_range(network_rule[0]), get_ip_range(network_rule[1]), network_rule[2]))
            l_tbit_rule = get_tbit_representation(network_rule[0]) + get_tbit_representation(network_rule[1]) + [network_rule[2]]
            tbit_classifier.append(tuple(l_tbit_rule))
        print "before redundancy removal: "+ str(len(tbit_classifier))
#print classifier
        all_match_tree = all_match_tree_construction(range_classifier)
        delete_redundant_rules(tbit_classifier, all_match_tree)
        print "after redundancy removal: "+ str(len(tbit_classifier))
        print tbit_classifier
        minimal_partition = find_minimal_partition(tbit_classifier)
        print minimal_partition
        after_BMA = []
        for partition in minimal_partition:
            swapped_partition = get_prefix_bit_swap(partition)
            print swapped_partition
            after_BMA_partition = BMA(swapped_partition)
            after_BMA += after_BMA_partition
        print after_BMA
        print "after BMA: " + str(len(after_BMA))
        #break
        
        
#break


def get_ip_range(ip_network):
    ip_range = []
    for ip in ip_network:
        ip_range.append(ip.value)
    return tuple(ip_range)


def get_tbit_representation(ip_network):
    bin_string = ip_network.ip.bin.lstrip('0b')
    #print bin_string
    bit_rep = []
    for s_bit in reversed(bin_string):
        bit_rep.append(int(s_bit))
    del bit_rep[16:len(bit_rep)]
    for i in xrange(32 - ip_network.prefixlen):
        bit_rep[i] = 2
    bit_rep.reverse()
    return bit_rep
#print bit_rep


############################ REDUNDANCY REMOVAL ############################
def all_match_tree_construction(classifier):
    all_match_tree = {}
    
    #build path from first rule
    first_rule = classifier[0]
    #print first_rule
    current_root = all_match_tree
    for i in xrange(len(first_rule)-1):
        t_field = first_rule[i]
        current_root[t_field] = {}
        current_root = current_root[t_field]
    current_root[0] = []
    current_root[0].append(0)

    #add the other rules
    for i in xrange(1,len(classifier)):  #skip the first rule
        #print classifier[i]
        add_rule_to_all_match_tree(all_match_tree, classifier[i], 0, i)

#print all_match_tree
    return all_match_tree

def add_rule_to_all_match_tree(v, rule, m, i):
    if m == d:
        if not 0 in v:
            v[0] = []
        v[0].append(i)
        return

    t_rest_of_rule_field = get_rest_of_rule_field(rule[m], v)
    if t_rest_of_rule_field != ():
        current_root = v
        v[t_rest_of_rule_field] = {}
        for field_index in xrange(i+1, len(rule)-1):
            current_root[rule[field_index]] = {}
            current_root = current_root[rule[field_index]]

    keys_to_replace = []
    keys_to_add = []

#print "m:" + str(m)
#   print "v:" + str(v)

    for t_range_key in v:
        if range_is_contained(t_range_key, rule[m]):
            #print "1111: " + str(v[t_range_key])
            add_rule_to_all_match_tree(v[t_range_key], rule, m+1, i)
            continue
        t_intersection = get_intersection(rule[m], t_range_key)
        if t_intersection != ():
            #copy
            subgraph_copy = copy.deepcopy(v[t_range_key])
            keys_to_add.append((t_intersection, subgraph_copy))
            ####v[t_intersection] = subgraph_copy
            #remove intersection from old edge
            t_new_range_key = substract_from_field(t_range_key, t_intersection)
            #####v[t_new_range_key] = v[t_range_key]
            keys_to_replace.append((t_range_key,t_new_range_key))

    for key in keys_to_replace:
        v[key[1]] = v.pop(key[0])

    for key in keys_to_add:
        v[key[0]] = key[1]
        #print "22222: " + str(v[key[0]])
        add_rule_to_all_match_tree(v[key[0]], rule, m+1, i)


def delete_redundant_rules(classifier, all_match_tree):
    containment_list = create_containment_list(all_match_tree)
    #print containment_list
    residency_list = create_residency_list(containment_list, len(classifier))
    #print residency_list

    redundancy_list = []
    for i in reversed(xrange(len(residency_list))):
        redundant = True
        for j in residency_list[i]:
            if not i in containment_list[j]:
                print "weiiiiierererd"
                return
            if len(containment_list[j]) == 1:
                redundant = False
                break
            """print containment_list[j]
            print classifier[i][DECISION_INDEX]
            print containment_list[j][1]
            print len(classifier)
            print classifier[containment_list[j][1]]#[DECISION_INDEX]"""
            if (i == containment_list[j][0] and classifier[containment_list[j][1]][DECISION_INDEX] != classifier[i][DECISION_INDEX]):
                redundant = False
                break
        if redundant == True:
            #print "redundant " + str(i)
            redundancy_list.append(i)
            for j in residency_list[i]:
                containment_list[j].remove(i)
    for redundant_index in redundancy_list:
        del classifier[redundant_index]
#print classifier


def get_rest_of_rule_field(t_field, all_match_tree_vertex):
    for t_range_key in all_match_tree_vertex:
        t_field = substract_from_field(t_field, t_range_key)
    return t_field


def substract_from_field(t_original_field, t_field_part_to_substract):
    l_updated_field = list(t_original_field)
    for point in t_field_part_to_substract:
        try:
            l_updated_field.remove(point)
        except:
            pass
    return tuple(l_updated_field)

def get_intersection(t_range1, t_range2):
    l_intersection_range = []
    for item in t_range1:
        if item in t_range2:
            l_intersection_range.append(item)
    return tuple(l_intersection_range)

def range_is_contained(t_range_contained, t_range_containing):
    for point in t_range_contained:
        if not point in t_range_containing:
            return False
    return True


def create_containment_list(all_match_tree):
    containment_list = []
    create_containment_list_rec(all_match_tree, containment_list)
    return containment_list

def create_containment_list_rec(vertex, containment_list):
    for vertex_son in vertex.itervalues():
        if type(vertex_son) == list:
            containment_list.append(vertex_son)
            return
        create_containment_list_rec(vertex_son, containment_list)

def create_residency_list(containment_list, number_of_rules):
    residency_list = []
    for residency_index in xrange(number_of_rules):
        residency_index_list = []
        for containment_index in xrange(len(containment_list)):
            if residency_index in containment_list[containment_index]:
                residency_index_list.append(containment_index)
        residency_list.append(residency_index_list)
    #print residency_list
    return residency_list

"""
def substract_from_field(original_field, field_part_to_substract):
    remove_list = []
    for original_range in original_field:
        for substraction_range in field_part_to_substract:
            if substraction_range[RANGE_START] > original_range[RANGE_END] or substraction_range[RANGE_END] < original_range[RANGE_START]:
                continue
            if substraction_range[RANGE_START] <= original_range[RANGE_START]:
                if substraction_range[RANGE_END] >= original_range[RANGE_END]:
                    remove_list.append(original_range)
                    continue
                original_range[RANGE_START] = substraction_range[RANGE_END]
                continue
            if substraction_range[RANGE_END] >= original_range:
                original_range[RANGE_END] = substraction_range[RANGE_START]
                continue
            original_range[RANGE_END] = substraction_range[RANGE_START]
            original_field.append([substraction_range[RANGE_END], original_range[RANGE_END]])
    for range in remove_list:
        original_field.remove(range)
"""
############################ PREFIX BIT SWAP ############################
def get_prefix_bit_swap(original_rules_matrix):
    COUNT_INDEX = 0
    INDEX_INDEX = 1

    count_wildcard = []
    for i in xrange(len(original_rules_matrix[0])-1): #ignore decision
        count_wildcard.append([0,i])

    swapped_rules_matrix = []

    for i in xrange(len(original_rules_matrix)):
        for j in xrange(len(original_rules_matrix[i])-1): #ignore decision
            if original_rules_matrix[i][j] == WILDCARD:
                count_wildcard[j][COUNT_INDEX] += 1

    #print count_wildcard
    count_wildcard.sort()
    #print count_wildcard

    for i in xrange(len(original_rules_matrix)):
        l_swapped_rule = []
        for j in xrange(len(original_rules_matrix[i])-1): #ignore decision
            #print count_wildcard[j][INDEX_INDEX]
            l_swapped_rule.append(original_rules_matrix[i][count_wildcard[j][INDEX_INDEX]])
        l_swapped_rule.append(original_rules_matrix[i][len(original_rules_matrix[i])-1]) #add decision
        swapped_rules_matrix.append(tuple(l_swapped_rule))
    return swapped_rules_matrix

"""
    print sys.argv
    if sys.argv[1] == '1':
    building_policies_main()
    if sys.argv[1] == '2':
    reading_forwarding_rules_main()
    """

############################ MINIMAL PARTITION ############################
def find_minimal_partition(rules_matrix):
    partition_list = []
    current_partition = []
    for rule in rules_matrix:
        if introduces_cross_pattern(rule, current_partition):
            partition_list.append(current_partition)
            current_partition = [rule]
        else:
            current_partition.append(rule)
        #print current_partition
    partition_list.append(current_partition)
    return partition_list

def introduces_cross_pattern(rule, partition):
    for second_rule in partition:
        if are_crossed(rule, second_rule):
            return True
    return False

def are_crossed(rule1, rule2):
    b_rule1_contains = False
    b_rule2_contains = False
    for i in xrange(len(rule1)-1): #ignore the decision
        #print i
        ter_bit1 = rule1[i]
        ter_bit2 = rule2[i]
        #print ter_bit1 == WILDCARD
        #print ter_bit2 == WILDCARD
        diff = ((ter_bit1 == WILDCARD) - (ter_bit2 == WILDCARD))
        #print diff
        if diff > 0:
            b_rule1_contains = True
        if diff < 0:
            b_rule2_contains = True
        if b_rule1_contains and b_rule2_contains:
            return True
    return False

############################ BMA ############################
def BMA(rules_matrix):
    C = {}
    S = list(set(rules_matrix))
    for rule in S:
        add_rule_to_partition(rule, C)
    OS = []
    #print "C: "
    #print C
    for c in C.itervalues():
        not_covered_indeces = list(xrange(len(c)))
        
        for i in xrange(len(c)-1):
            for j in xrange(i+1, len(c)):
                ternary_cover = get_ternary_cover(c[i],c[j])
                if ternary_cover != None:
                    OS.append(ternary_cover)
                    if i in not_covered_indeces:
                        not_covered_indeces.remove(i)
                    if j in not_covered_indeces:
                        not_covered_indeces.remove(j)
        for i in not_covered_indeces:
            OS.append(c[i])
    #print "OS: "
    #print OS
    O = get_in_decreasing_order_prefix_length(OS)
    if are_same_sets(O, S):
        #print "O: "
        #print O
        #print "S: "
        #print S
        return O
    else:
        #print "dO: "
        #print O
        #print "dS: "
        #print S
        return BMA(O)

def add_rule_to_partition(rule, C):
    t_rule_bitmask = get_rule_bitmask(rule)
    key = (t_rule_bitmask)
    if key not in C:
        C[key] = []
    C[key].append(rule)

def get_rule_bitmask(rule):
    l_rule_bitmask = list(rule)
    for i in xrange(len(l_rule_bitmask)):
        if l_rule_bitmask[i] == 1:
            l_rule_bitmask[i] = 0
    return tuple(l_rule_bitmask)

def get_ternary_cover(rule1, rule2):
    different_index = -1
    for i in xrange(len(rule1)-1): #-1 to avoid decision
        if rule1[i] != rule2[i]:
            if rule1[i] == WILDCARD or rule2[i] == WILDCARD:
                print "AAAAAAAA - unexpected different bitmask"
                raise Exception()
            if different_index != -1:
                return None
            different_index = i
    l_ternary_cover = list(rule1)
    l_ternary_cover[different_index] = WILDCARD
    return tuple(l_ternary_cover)

def get_in_decreasing_order_prefix_length(OS):
    O = []
    prefix_length_to_index_list = []
    for rule_index in xrange(len(OS)):
        prefix_length_to_index_list.append((get_prefix_length_negated(OS[rule_index]), rule_index))
    prefix_length_to_index_list.sort()
    for (length, index) in prefix_length_to_index_list:
        O.append(OS[index])
    return O

def get_prefix_length_negated(rule):
    prefix_length = 0
    for i in reversed(xrange(len(rule)-1)):   #avoid decision
        if rule[i] != WILDCARD:
            break
        prefix_length -= 1
    return prefix_length

def are_same_sets(O, S):
    for rule in O:
        if rule not in S:
            return False
    return True

############################ MAIN ############################
"""rule1 = (tuple(range(5)),tuple(range(10)),1)
rule2 = (tuple(range(5)),tuple(range(5,10)),1)
rule3 = (tuple(range(5,10)),tuple(range(3)),0)
rule4 = (tuple(range(10)),tuple(range(10)),0)

classifier = []
classifier.append(rule1)
classifier.append(rule2)
classifier.append(rule3)
classifier.append(rule4)

all_match_tree = all_match_tree_construction(classifier)

delete_redundant_rules(classifier, all_match_tree)
"""

create_classifeier_from_rules_file()
"""
rule1 = (1,0,0,0,0,0,2,2,'d')
rule2 = (1,0,0,0,1,0,2,2,'d')
rule3 = (1,0,0,1,0,0,2,2,'d')
rule4 = (1,0,0,1,1,0,2,2,'d')
rule5 = (1,0,1,0,0,0,2,2,'d')
rule6 = (1,1,0,0,0,0,2,2,'d')
rule7 = (1,1,1,0,0,0,2,2,'d')

classifier = []
classifier.append(rule1)
classifier.append(rule2)
classifier.append(rule3)
classifier.append(rule4)
classifier.append(rule5)
classifier.append(rule6)
classifier.append(rule7)

#print get_ternary_cover(rule1,rule2)

O = BMA(classifier)
print classifier
print O
print are_same_sets(classifier, O)
"""