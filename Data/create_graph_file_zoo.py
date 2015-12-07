import re
import os
from sets import Set
DATA_PATH = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\archive'
    
def create_graph_for_name(FILE_NAME):
    FILE_INPUT_PATH = DATA_PATH + '\\' + FILE_NAME + '.graphml'
    FILE_OUTPUT_PATH = FILE_INPUT_PATH + '.graph'


    maxNodeNum = 0
    data_file = open(FILE_INPUT_PATH)

    while(True):
        line = data_file.readline()
        if line == '':
            break
        if line.strip()[0:5] != "<node":
            continue
        m = re.search('(?<=node id\=\")[0-9]+',line)
        node = int(m.group(0))
        if node >= maxNodeNum:
            maxNodeNum = node + 1

    data_file.close()





    data_file = open(FILE_INPUT_PATH)
    graph_file = open(FILE_OUTPUT_PATH,'w+')
    graph_file.write(str(maxNodeNum) + "\n\n")

    while(True):
        line = data_file.readline()
        if line == '':
            break
        #print line.strip()[0:5]
        if line.strip()[0:5] != "<edge":
            continue
        m = re.search('(?<=source\=\")[0-9]+',line)
        source = int(m.group(0))
        m = re.search('(?<=target\=\")[0-9]+',line)
        target = int(m.group(0))
        graph_file.write(str(source) + " " + str(target) + " " + "1" + "\n")
        graph_file.write(str(target) + " " + str(source) + " " + "1" + "\n")

    data_file.close()
    graph_file.close()

names = list(Set([name.split('.')[0] for name in os.listdir(DATA_PATH)]))
for name in names:
    create_graph_for_name(name)