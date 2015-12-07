import itertools
from netaddr import *
import pdb
import sys
import datetime
import random
from ip_hierarchy import *

from policies_input import *

############################ CONSTANTS/MAGICS ############################

ARROW = "->"
AND = " and "
OR = " or "
NEGATION = '!'
COLON = ":"
EQUALS_SIGN = "="
ASTERISK = "*"
PREDICATE_SEPERATOR = "--__--"
ALL_PREDICATE = {'*':'*'}
DEFAULT_DAG_PATH_EXPRESSION = ARROW

DST_TYPE = "DST"
SRC_DST_TYPE = "SRC_DST"

PARTIAL_REP_PREDICATE_INDEX = 0
PARTIAL_REP_PRIORITY_INDEX = 1
PARTIAL_REP_PATHS_INDEX = 2
DEFAULT_DAG_PRIORITY = 1

ALL_IP = "0.0.0.0/0"
ALL_IP_NETWORK = IPNetwork(ALL_IP)

GROUPS["*"] = [ALL_IP]

OUTPUT_FILE_NAME = "policies.txt"
RANDOM_OUTPUT_FILE_NAME = "random_policies.txt"

############################ GLOBALS ############################

src_dst_policies = {}
generalized_src_dst_policies = {}
final_policies = []

src_ip_hierarchy_dict = {}
dst_ip_hierarchy_dict = {}

############################ GROUPS & IPS ############################

def localize_policy(policy, src_dst_policies_dict):
    """Localizes a policy to a list of local switch policies.

    arguments:
    policy -- the original policy
    """
    (predicate, path_expression) = policy.split(COLON)
    
    local_path_expressions = []

    optional_negation = ''
    if path_expression.count(NEGATION) > 0:
        optional_negation = '!'
        
    #get the different parts of the path expression
    groups_and_functions = path_expression.strip(NEGATION).split(ARROW)
    src_group_str = groups_and_functions[0]
    dst_group_str = groups_and_functions[-1]
    functions = groups_and_functions[1:-1]
    
    #localize each part according to it's type
    localized_srcs = localize_group(src_group_str)
    localized_dsts = localize_group(dst_group_str)
    localized_functions = [localize_function(function_str) for function_str in functions]
    
    #rebuild the localized path_expressions
    for (src,dst) in itertools.product(localized_srcs, localized_dsts):
        local_path_expression = predicate + ":"
        local_path_expression += optional_negation + ARROW
        for localized_function in localized_functions:
            local_path_expression += localized_function + ARROW
        add_policy(src_dst_policies_dict, IPNetwork(src), IPNetwork(dst), local_path_expression)
        local_path_expressions.append(local_path_expression)
    return local_path_expressions

    
def add_policy(policy_dict, src, dst, path_expression):
    if not (src, dst) in policy_dict:
        policy_dict[(src, dst)] = []
    policy_dict[(src, dst)].append(path_expression)
    

def localize_group(group_str):
    if group_str.count(AND) != 0:
        return get_ip_lists_intersection(GROUPS[group_str.split(AND)[0]], GROUPS[group_str.split(AND)[1]])
    if group_str.count(OR) != 0:
        return get_ip_lists_union(GROUPS[group_str.split(OR)[0]], GROUPS[group_str.split(OR)[1]])
    return GROUPS[group_str]
    
    
def get_ip_lists_intersection(ip_list1, ip_list2):
    intersection = []
    for ip in ip_list1:
        ip_intersection = get_ip_and_iplist_intersection(ip, ip_list2)
        if ip_intersection != None:
            intersection.append(ip_intersection)
    return intersection
    
    
def get_ip_and_iplist_intersection(ip, ip_list):
    for ip_from_list in ip_list:
        ip_intersection = get_subnet_intersection(ip, ip_from_list)
        if ip_intersection != None:
            return ip_intersection


def get_subnet_intersection(subnet_ip_1, subnet_ip_2):
    subnet1 = IPNetwork(subnet_ip_1)
    subnet2 = IPNetwork(subnet_ip_2)
    if subnet1 in subnet2:
        return str(subnet1)
    if subnet2 in subnet1:
        return str(subnet2)
    return None

    
def get_ip_lists_union(ip_list1, ip_list2):
    ip_list1_copy = ip_list1[:]
    ip_list2_copy = ip_list2[:]
    ip_union = []
    for ip1 in ip_list1:
        subnet1 = IPNetwork(ip1)
        was_ip1_added = False
        for ip2 in ip_list2:
            subnet2 = IPNetwork(ip2)
            if subnet1 in subnet2:
                ip_union.append(str(subnet2))
                ip_list1_copy.remove(ip1)
                ip_list2_copy.remove(ip2)
                break
            if subnet2 in subnet1:
                ip_union.append(str(subnet1))
                ip_list1_copy.remove(ip1)
                ip_list2_copy.remove(ip2)
                break
    ip_union = ip_union + ip_list1_copy + ip_list2_copy
    return ip_union

    
def generate_additional_policies(base_policies_dict, new_policies_dict, generated_policies_dict):
    
    #first alternative
    copy_src_dst_policies(base_policies_dict, generated_policies_dict)
    for (src1, dst1) in base_policies_dict:
        for (src2, dst2) in new_policies_dict:
            generate_and_add_policy(src1, dst1, src2, dst2, generated_policies_dict)
        src_dst_policies.pop(src1, dst1) #we don't need to use it to create any more policies
    
    remove_duplicates(generated_policies_dict)
    
    
def generate_additional_policies2(base_policies_dict, new_policies_dict, flat_old_dict, generated_policies_dict):
    #second alternative
    create_generalized_keys(flat_old_dict, new_policies_dict, generated_policies_dict)
    #print "middle of additional policies generation: ", datetime.datetime.now().time()
    add_base_dicts_values(base_policies_dict, new_policies_dict, generated_policies_dict)
    remove_duplicates(generated_policies_dict)

    
def copy_src_dst_policies(original_policies_dict, new_policies_dict, copy_values=True):
    for (src, dst) in original_policies_dict: 
        new_policies_dict[(src, dst)] = {}
        new_policies_dict[(src, dst)][(src, dst)] = []
        if copy_values:
            new_policies_dict[(src, dst)][(src, dst)] += src_dst_policies[(src, dst)]

            
def create_generalized_keys(base_policies_dict, new_policies_dict, generated_policies_dict):
    copy_src_dst_policies(new_policies_dict, generated_policies_dict, copy_values = False)
    for (src1, dst1) in base_policies_dict:
        for (src2, dst2) in new_policies_dict:
            create_key(src1, dst1, src2, dst2, generated_policies_dict) 


def create_key(src1, dst1, src2, dst2, generated_policies_dict):
    contained_src = None
    contained_dst = None
    
    if src1 in src2:
        contained_src = src1
    elif src2 in src1:
        contained_src = src2
    if contained_src == None:
        return
        
    if dst1 in dst2:
        contained_dst = dst1
    elif dst2 in dst1:
        contained_dst = dst2
    if contained_dst == None:
        return
        
    if not (contained_src, contained_dst) in generated_policies_dict:
        generated_policies_dict[(contained_src, contained_dst)] = {}
        

def add_base_dicts_values(original_policies_dict, new_policies_dict, generated_policies_dict):
    for (src, dst) in generated_policies_dict:
        add_from_base_to_generated_if_needed(src, dst, original_policies_dict, generated_policies_dict)
        if new_policies_dict != original_policies_dict:
            add_from_base_to_generated_if_needed(src, dst, new_policies_dict, generated_policies_dict)
             
                
def add_from_base_to_generated_if_needed(src, dst, base_policies_dict, generated_policies_dict):
        for (base_src, base_dst) in base_policies_dict:
            if src in base_src and dst in base_dst:
                if not (base_src,base_dst) in generated_policies_dict[(src,dst)]:
                    generated_policies_dict[(src,dst)][(base_src,base_dst)] = []
                generated_policies_dict[(src,dst)][(base_src,base_dst)] += base_policies_dict[(base_src, base_dst)]
                    
def remove_duplicates(policies_dict):
    for key in policies_dict:
        for key1 in policies_dict[key]:
           policies_dict[key][key1] = list(set(policies_dict[key][key1]))

        
def get_src_dst_priority(src,dst):#TODOOOO : change the input policies dict. see to do it smart because it is checked against the old one
    priority = 0
    for (other_src, other_dst) in generalized_src_dst_policies:
        if src in other_src and dst in other_dst:
            priority += 10
    return priority
    
    
def add_default_dags():
    for ip in IP_LOCATIONS:
        ip_network = IPNetwork(ip)
        should_add_ip_dag = True
        for policy in final_policies:
            if is_dag_of_ip(policy, ip_network):
                #print "is dag", ip, policy
                should_add_ip_dag = False
                break
        if should_add_ip_dag:
            add_ip_default_dag(ip_network)

            
def is_dag_of_ip(policy, ip):
    if not ip in policy.dst or policy.src != ALL_IP_NETWORK or policy.predicate_representation != ALL_PREDICATE:
        return False
    return True
   
   
def add_ip_default_dag(ip):
    policy = LocalPolicy(ALL_IP_NETWORK, ip, ALL_PREDICATE, DEFAULT_DAG_PRIORITY, [DEFAULT_DAG_PATH_EXPRESSION])
    final_policies.append(policy)
   

def flatten_dict(deep_dict):
    flat_dict = {}
    for key, key_dict in deep_dict.iteritems():
        flat_dict[key] = []
        for policy_list in key_dict.values():
            flat_dict[key] += policy_list
            
    return flat_dict
   
############################ PREDICATES ############################
        
def split_accroding_to_predicates(policies_dict):
    for ((src, dst), specific_policy_dict) in policies_dict.iteritems():
        base_priority = get_src_dst_priority(src,dst)
        split_policies_according_to_predicates(src, dst, specific_policy_dict, base_priority)


def split_policies_according_to_predicates(src, dst, specific_policy_dict, base_priority):
    predicate_combination_to_representation_dict = get_legal_predicate_combinations(src, dst, specific_policy_dict)
    policy_list = get_all_policies(specific_policy_dict)
    partial_policies_list = get_path_for_predicate(predicate_combination_to_representation_dict, policy_list)
    for partial_policy in partial_policies_list:
        for predicate_rep in partial_policy[PARTIAL_REP_PREDICATE_INDEX]:            
            full_policy = LocalPolicy(src, dst, predicate_rep, base_priority + partial_policy[PARTIAL_REP_PRIORITY_INDEX], partial_policy[PARTIAL_REP_PATHS_INDEX])
            final_policies.append(full_policy)

        
def get_all_policies(specific_policy_dict):
    policy_list = []
    for specific_src_dst_policy_list in specific_policy_dict.values():
        policy_list += specific_src_dst_policy_list
    return policy_list
        

def get_path_for_predicate(predicate_combination_to_representation_dict, policy_list):
    policies_list = []
    for (predicate_combination, predicate_representation) in predicate_combination_to_representation_dict.iteritems():
        path_expressions = []
        priority = len(predicate_combination)
        policy_partial_rep = [predicate_representation, priority, path_expressions]
        for policy in policy_list:
            policy_split = policy.split(COLON)
            predicate = policy_split[0]
            if predicate in predicate_combination:
                path_expressions.append(policy_split[1])
        policies_list.append(policy_partial_rep)
    return policies_list
    
    
def get_legal_predicate_combinations(specific_src, specific_dst, specific_policy_dict):
    predicates_list =[]
    for ((src,dst),policy_list) in specific_policy_dict.iteritems():
        for policy in policy_list:
            predicate = policy.split(COLON)[0]
            if predicate not in predicates_list:
                predicates_list.append((src,dst,predicate))
    predicate_combinations_list = get_predicate_combinations(specific_src, specific_dst, predicates_list)
    return get_legal_predicate_combinations_representation(predicate_combinations_list)
    
    
def get_legal_predicate_combinations_representation(predicate_combinations_list):
    combination_to_representaion_dict = {}
    for predicate_combination in predicate_combinations_list:
        combination_representation = get_good_combination_representation(predicate_combination)
        if len(combination_representation) > 0:
            for (comb, rep) in combination_to_representaion_dict.items(): #not iter because we change during iteration
                if are_representations_equal(rep, combination_representation):
                    if len(comb) > len(predicate_combination):
                        raise Exception('not supposed to happen')
                    combination_to_representaion_dict.pop(comb)
            combination_to_representaion_dict[predicate_combination] = combination_representation
    return combination_to_representaion_dict
            
      
def get_predicate_representation(predicate):
    predicate_representation = []
    if OR in predicate:
        for single_predicate in predicate.split(OR):
            pred_dict={}
            pred_dict[get_predicate_type(single_predicate)] = get_predicate_value(single_predicate)
            predicate_representation.append(pred_dict)
        return predicate_representation
    if AND in predicate:
        pred_dict={}
        for single_predicate in predicate.split(AND):
            pred_dict[get_predicate_type(single_predicate)] = get_predicate_value(single_predicate)
        predicate_representation.append(pred_dict)
        return predicate_representation
    pred_dict={}
    pred_dict[get_predicate_type(predicate)] = get_predicate_value(predicate)
    predicate_representation.append(pred_dict)
    return predicate_representation        
       

def are_representations_equal(rep1, rep2):
    rep2_copy = rep2[:]
    for and_dict in rep1:
        if and_dict not in rep2:
            return False
        rep2_copy.remove(and_dict)
    if len(rep2_copy) > 0:
        return False
    return True
       
def get_good_combination_representation(predicate_combination):
    predicate_combination_representation = [{}] #list of or's between dictionaries of and - (a=1^b=1)v(c=3^d=3) := [{a:1,b:1},{c:3,d:3}
    
    for predicate in predicate_combination:
        predicate_representation = get_predicate_representation(predicate)
        predicate_combination_representation = get_predicates_conjuction(predicate_combination_representation, predicate_representation)
    normalize_predicate_representation(predicate_combination_representation)
    return predicate_combination_representation 

def normalize_predicate_representation(predicate_representation):
    remove_asterisk_from_and_dicts(predicate_representation)
    normalize_if_trivial(predicate_representation)
    remove_duplicate_and_dicts(predicate_representation)
    
    
def remove_asterisk_from_and_dicts(predicate_representation):
    for and_dict in predicate_representation:
        if len(and_dict) > 1:
            and_dict.pop(ASTERISK, None)

def normalize_if_trivial(predicate_representation):
    for and_dict in predicate_representation:
        if and_dict == {ASTERISK:ASTERISK}:
            del predicate_representation[:]
            predicate_representation.append({ASTERISK:ASTERISK})
    
 
def remove_duplicate_and_dicts(predicate_representation):
    for (and_dict1, and_dict2) in itertools.combinations(predicate_representation, 2):
        if and_dict1 == and_dict2:
            predicate_representation.remove(and_dict1)
 
            
def get_predicates_conjuction(predicate1, predicate2):
    predicates_conjunction = []
    for and_dict1 in predicate1:
        for and_dict2 in predicate2:
            and_dict_conjunction = get_and_dict_conjunction(and_dict1,and_dict2)
            if and_dict_conjunction is not None:
                predicates_conjunction.append(and_dict_conjunction)
    return predicates_conjunction

    
def get_and_dict_conjunction(and_dict1,and_dict2):
    and_dict_conjunction = dict(and_dict1)
    for (type, value) in and_dict2.iteritems():
        if type in and_dict1 and and_dict1[type] != and_dict2[type]:
            return None
        and_dict_conjunction[type] = value
    return and_dict_conjunction
    
    
def get_predicate_type(predicate):
    if EQUALS_SIGN in predicate:
        return predicate.split(EQUALS_SIGN)[0]
    return ASTERISK
 
 
def get_predicate_value(predicate):
    if EQUALS_SIGN in predicate:
        return predicate.split(EQUALS_SIGN)[1]
    return ASTERISK

    
def get_predicate_combinations(specific_src, specific_dst, src_dst_predicate_list):
    predicate_combinations_list = []
    for predicates_number in xrange(1, len(src_dst_predicate_list)+1):
        for src_dst_predicate_combination in itertools.combinations(src_dst_predicate_list, predicates_number):
            if is_minimal_src_dst(specific_src, specific_dst, src_dst_predicate_combination):
                only_predicates_combination = tuple([predicate_triple[2] for predicate_triple in src_dst_predicate_combination])
                predicate_combinations_list.append(only_predicates_combination)
    return predicate_combinations_list
 
def is_minimal_src_dst(specific_src, specific_dst, predicate_combination):
    found_specific_src = False
    found_specific_dst = False
    for predicate_triple in predicate_combination:
        if predicate_triple[0] == specific_src:
            found_specific_src = True
            if found_specific_dst:
                return True
        if predicate_triple[1] == specific_dst:
            found_specific_dst = True
            if found_specific_src:
                return True
    return False
                
############################ ETC ############################

def get_switch_str(ip):
    if ip == IPNetwork(ALL_IP):
        return ""
    for (ip_from_dict, switch) in IP_LOCATIONS.iteritems():
        if ip in IPNetwork(ip_from_dict):
            return str(switch)
    #print ip
    raise Exception("ip doesn't exist: " + ip)
    
 
def localize_function(function_str):
    """Returns a disjunction string representation of the switch locations for this function

    arguments:
    function_str -- the name of the function
    """
    function_switch_disjunction_str = ""
    for switch_or_link in FUNCTIONS_AND_AREAS[function_str]:
        if type(switch_or_link) == tuple: #it is a link
            function_switch_disjunction_str += str(switch_or_link).replace(',','-').replace(' ','')[1:-1] + "|"
        else: #it is a switch
            function_switch_disjunction_str += str(switch_or_link) + "|"
    return function_switch_disjunction_str.strip("|") 
    
    
############################ LOCAL POLICY STRUCT ############################
class LocalPolicy:
    def __init__(self, src, dst, predicate_representation, priority, path_expressions):
        self.src = src
        self.dst = dst
        self.predicate_representation = predicate_representation
        self.priority = priority
        self.path_expressions = path_expressions
        if src == ALL_IP_NETWORK:
            self.type = DST_TYPE
        else:
            self.type = SRC_DST_TYPE
    
    def __str__(self):
        s = ""
        s += self.type + "\n"
        s += "src" + "=" + str(self.src) + PREDICATE_SEPERATOR
        s += "dst" + "=" + str(self.dst) + PREDICATE_SEPERATOR
        for (predicateKey, predicateValue) in self.predicate_representation.iteritems():
            if predicateKey != ASTERISK:
                s += predicateKey + "=" + predicateValue + PREDICATE_SEPERATOR
        s += "priority" + "=" + str(self.priority) + "\n"
        for path_expression in self.path_expressions:
            s += str(path_expression) + ","
        s = s.strip(",")
        s += '\n'
        return s
        
    def insert_src_dst_switches(self):
        src_switch = get_switch_str(self.src)
        dst_switch = get_switch_str(self.dst)
        updated_path_expressions = []
        for path_expression in self.path_expressions:
            optional_negation = ''
            if path_expression.count(NEGATION) > 0:
                optional_negation = '!'
            updated_path_expressions.append(optional_negation + src_switch + path_expression.strip(NEGATION) + dst_switch)
        self.path_expressions = updated_path_expressions
    
    
############################ FORWARDING RULE STRUCT ############################
class ForwardingRule:
    def __init__(self, src, dst, predicate_representation, priority, current_switch, former_switch, next_switches):
        self.src = src
        self.dst = dst
        self.predicate_representation = predicate_representation
        self.priority = priority
        self.path_expressions = path_expressions
    
    def __init__(self, rule_line):
        self.former_switch = None
        predicates_list = rule_line.split(PREDICATE_SEPERATOR)
        for predicate in predicates_list:
            (type, value) = predicate.split(EQUALS_SIGN)
            if type == 'src':
                self.src = IPNetwork(value)
            if type == 'dst':
                self.dst = IPNetwork(value)
            if type == 'predicate_representation':
                self.predicate_representation = value #TODO - read the real representation and not only string
            if type == 'priority':
                self.priority = int(value)
            if type == 'current_switch':
                self.current_switch = int(value)
            if type == 'former_switch':
                self.former_switch = int(value)
            if type == 'next_switches':
                self.next_switches = value #TODO - read the real list and not only string

    
    def __str__(self):
        s = ""
        s += "src" + "=" + str(self.src) + PREDICATE_SEPERATOR
        s += "dst" + "=" + str(self.dst) + PREDICATE_SEPERATOR
        s += "predicate_representation" + "=" + str(self.predicate_representation) + PREDICATE_SEPERATOR
        s += "priority" + "=" + str(self.priority) + PREDICATE_SEPERATOR
        s += "current_switch" + "=" + str(self.current_switch) + PREDICATE_SEPERATOR
        s += "former_switch" + "=" + str(self.former_switch) + PREDICATE_SEPERATOR
        s += "next_switches" + "=" + str(self.next_switches)
        s += '\n'
        return s
        
    def insert_src_dst_switches(self):
        src_switch = get_switch_str(self.src)
        dst_switch = get_switch_str(self.dst)
        updated_path_expressions = []
        for path_expression in self.path_expressions:
            optional_negation = ''
            if path_expression.count(NEGATION) > 0:
                optional_negation = '!'
            updated_path_expressions.append(optional_negation + src_switch + path_expression.strip(NEGATION) + dst_switch)
        self.path_expressions = updated_path_expressions

        
    
############################ BUILDING POLICIES MAIN ############################
def building_policies_main():
    print "begin: ", datetime.datetime.now().time()
    
    for policy in POLICIES:
        localize_policy(policy, src_dst_policies)
    
    for (src,dst) in src_dst_policies:
        add_ip_to_hierarchy(src_ip_hierarchy_dict, src, dst)
        add_ip_to_hierarchy(dst_ip_hierarchy_dict, dst, src)
        
    #print "before additional policies generation: ", datetime.datetime.now().time()
    generate_additional_policies2(src_dst_policies, src_dst_policies, src_dst_policies, generalized_src_dst_policies)

    #print "before predicates: ", datetime.datetime.now().time()
    split_accroding_to_predicates(generalized_src_dst_policies)
    
    #print "before dags: ", datetime.datetime.now().time()
    add_default_dags()
    
    print "before writing to file: ", datetime.datetime.now().time()   
    output_file = open(OUTPUT_FILE_NAME,'w+')
    for policy in final_policies:
        policy.insert_src_dst_switches()
        output_file.write(str(policy))
        #print policy
    output_file.close()

    print "after writing to file: ", datetime.datetime.now().time()

############################ READING FORWARDING RULES MAIN ############################
forwarding_rules = {}

def reading_forwarding_rules_main():
    rules_file = open(r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\Results\rules.res')
    while(True):
        rule_line = rules_file.readline()
        if rule_line == '':
            break
        rule = ForwardingRule(rule_line)
        if not rule.current_switch in forwarding_rules:
            forwarding_rules[rule.current_switch] = []
        forwarding_rules[rule.current_switch].append(rule)
    
    for (key, val) in forwarding_rules.iteritems():
        if len(val) > 4:
            print key, len(val)
            for rule in val:
                print rule
            print '______________'
        
 
############################ CREATING RANDOM RULES MAIN ############################ 
def creating_random_rules_main():
    ITERATIONS = 1000
    SWITCH_NUM = 318
    PATH_EXPRESSION_LENGTH = 2
    OPTIONAL_LOCATIONS = 1

    output_file = open(RANDOM_OUTPUT_FILE_NAME,'w+')
    for i in xrange(ITERATIONS):
        
        policy = LocalPolicy(str(i), str(i), "", 1, [create_random_path_exp(SWITCH_NUM, PATH_EXPRESSION_LENGTH, OPTIONAL_LOCATIONS)])
        output_file.write(str(policy))
        #print policy
    output_file.close()

def create_random_path_exp(switch_num, length, optional_locations):
    switches = []
    source_switch = random.randint(0, switch_num-1)
    destination_switch = source_switch
    while(destination_switch == source_switch):
        destination_switch = random.randint(0, switch_num-1)
    while(len(switches) < length):
        optional_switches = []
        while(len(optional_switches) < optional_locations):
            random_num = random.randint(0, switch_num-1)
            if not random_num in optional_switches and random_num != source_switch and random_num != destination_switch:
                optional_switches.append(random_num)
        switches.append(optional_switches)
    path_exp = ""
    for optional_switches in switches:
        for switch in optional_switches:
            path_exp += str(switch) + "|"
        path_exp = path_exp.strip("|")
        path_exp += "->"
    return str(source_switch) + "->" + path_exp + str(destination_switch)

############################ MAIN ############################
print sys.argv
if sys.argv[1] == '1':
    building_policies_main()
if sys.argv[1] == '2':
    reading_forwarding_rules_main()    
if sys.argv[1] == '3':
    creating_random_rules_main()  