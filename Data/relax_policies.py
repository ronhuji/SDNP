DIRECTORY_PATH = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\Results'
INPUT_FILE_NAME = '1221.policies'
INPUT_FILE_PATH = DIRECTORY_PATH + '\\' + INPUT_FILE_NAME
OUTPUT_FILE_PATH = INPUT_FILE_PATH + '_relaxed'
AC_STRING = "AC:"
OS_STRING = "OS:"
PATH_DELIMITER = "->"
COMMA_DELIMITER = ","

if len(AC_STRING) != len(OS_STRING):
	raise Error("cannot work with policy strings of different lengths")

input_file = open(INPUT_FILE_PATH)
output_file = open(OUTPUT_FILE_PATH, 'w+')
vertexNum = 0
vertexDict = {}
while(True):
	line = input_file.readline()
	if line == '':
		break
	line = line.replace(" ", "")
	if line.count(PATH_DELIMITER) < 2:
		output_file.write(line)
		continue
	line_parts = line.split(sep = PATH_DELIMITER, maxsplit=1)
	policy_type = line_parts[0][0:len(AC_STRING)]
	src_group = line_parts[0][len(AC_STRING):]
	rest_of_policy = line_parts[1]
	for src in src_group.split(COMMA_DELIMITER):
		output_file.write(policy_type + src + PATH_DELIMITER + rest_of_policy)
	

input_file.close()
output_file.close()
