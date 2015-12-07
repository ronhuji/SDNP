import itertools
import pdb
import sys
import datetime
import re
from sets import Set

DIR_PATH = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\PolicyManager\table_size' + '\\'

GRAPH_PATH = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\ISPMaps\1239.r0.graph'

GRAPH_NAME = 'integra\\'
print 'grap is ' + GRAPH_NAME
TYPE = 'edge'
print 'type is ' + TYPE

#r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\ISPMaps\1239.r0.graph'
r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\PolicyManager\table_size\1755\vertex.txt'

rules_file = open(DIR_PATH + GRAPH_NAME + TYPE + ".txt")

switch_dict = {}

while(True):
#for x in xrange(20):
    line = rules_file.readline()
    if line == '':
        break
        
    m = re.search('(?<=current_switch\=)[0-9]+',line)
    switch = int(m.group(0))
    #   print switch
    if not switch in switch_dict:
        switch_dict[switch] = 0
    switch_dict[switch] += 1
rules_file.close()

max = (0,0)
for (key,value) in switch_dict.items():
    if value > max[1]:
        max=(key,value)
        
print max