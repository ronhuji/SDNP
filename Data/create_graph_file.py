import re

DATA_PATH = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\ISPMaps'
FILE_NUMBER = '7018'
FILE_NAME = '\\' + FILE_NUMBER + '.r0'

data_file = open(DATA_PATH + FILE_NAME + ".cch")
vertexNum = 0
vertexDict = {}
while(True):
	line = data_file.readline()
	if line == '':
		break
	if re.search('<([0-9]+)>',line) == None:
		continue
	vertexDict[line.split()[0]] = vertexNum
	vertexNum = vertexNum+1

data_file.close()

index_file = open(DATA_PATH + FILE_NAME + ".index", 'w+')
index_file.write(str(vertexDict))
index_file.close()

data_file = open(DATA_PATH + FILE_NAME + ".cch")
graph_file = open(DATA_PATH + FILE_NAME + ".graph",'w+')
graph_file.write(str(vertexNum) + "\n\n")

while(True):
    line = data_file.readline()
    if line == '':
        break
    original_src = (line.split()[0])
    if not original_src in vertexDict:
        continue
    src = vertexDict[original_src]
    for enclosed_number_string in re.findall('<([0-9]+)>',line):
        dst = vertexDict[enclosed_number_string]
        graph_file.write(str(src) + " " + str(dst) + " " + "1" + "\n")

graph_file.close()
data_file.close()