import itertools
import pdb
import sys
import datetime
import re
from sets import Set

DIR_PATH = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\PolicyManager\link_fail\Policies\garr' + '\\'

GRAPH_PATH = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\archive\Garr200908.graphml.graph'

NUMBER = '1000'
print 'number is ' + NUMBER
SELECTION = 68
print 'selection is ' + str(SELECTION)

#r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\ISPMaps\1239.r0.graph'

rules_file = open(DIR_PATH + "rules" + NUMBER + ".txt")


def get_random_links(graph_path, number_of_links):
    random_links = []
    s = get_all_links(graph_path)
    print "num of links"
    print len(s)
    for i in xrange(number_of_links):
        random_links.append(s.pop())
    #print random_links
    return random_links
    
    
def get_all_links(graph_path):
    s = Set()
    graph_file = open(graph_path)
    graph_file.readline()
    graph_file.readline()
    while(True):
        #for x in xrange(20):
        line = graph_file.readline()
        if line == '':
            break
        node1 = int(line.split(' ')[0])
        node2 = int(line.split(' ')[1])        
        link_list = [node1,node2]
        link_list.sort()
        link = tuple(link_list)
        s.add(link)    
    graph_file.close()
    return s



link_dict = {}

while(True):
#for x in xrange(20):
    line = rules_file.readline()
    if line == '':
        break
    m = re.search('(?<=priority\=)[0-9]+',line)
    if m.group(0) == '1':
        continue
        
    m = re.search('(?<=current_switch\=)[0-9]+',line)
    node1 = int(m.group(0))
    #print node1
    m = re.search('(?<=next_switches\=\[)[0-9,]+', line)
    if m.group(0).count(',') > 1:
        continue
    
    meta_data = line.split('--__--cu')[0]
    
    for node_str in m.group(0).split(','):
        if node_str is not "":
            node2 = int(node_str)
            #print node2
            link_list = [node1,node2]
            link_list.sort()
            link = tuple(link_list)
            if link not in link_dict:
                link_dict[link] = []
            link_dict[link].append(meta_data)
rules_file.close()

random_link_list = get_random_links(GRAPH_PATH, SELECTION)

all_meta_data = []
for link in random_link_list:
    if link in link_dict:
        all_meta_data += link_dict[link]
print "paths"
print len(all_meta_data)
#print all_meta_data
#all_meta_data = list(Set(all_meta_data))
#print len(all_meta_data)

#for key in link_dict:
#    print key, link_dict[key]
print "participating links"    
print len(link_dict)


policies_file = open(DIR_PATH + NUMBER + ".txt")
new_policies_file = open(DIR_PATH + "new_policies" + NUMBER + ".txt", 'w')

while(True):
#for x in xrange(20):
    type = policies_file.readline()
    #print type
    if type == '':
        break
    meta_data = policies_file.readline()
    constraints = policies_file.readline().strip()
    #print meta_data
    for i in xrange(all_meta_data.count(meta_data.strip())):
        new_policies_file.write(type)
        new_policies_file.write(meta_data)
        new_policies_file.write(constraints +'\n')

policies_file.close()
new_policies_file.close()


