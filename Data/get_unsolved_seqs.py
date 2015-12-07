import re

RESULTS_DIR = r'C:\Users\famini.HUN7\Google Drive\Visual Studio 2013\Projects\SDNP\Data\results'
FILE_NAME = '1221_100_10000hh'
RESULTS_FULL_PATH = RESULTS_DIR + '\\' + FILE_NAME + '.res'
SEQUENCES_FULL_PATH = RESULTS_DIR + '\\' + FILE_NAME + '.seq'

SEQUENCE_STRING_LENGTH = len("sequence: ")

NO_PATH_FOUND = "No path found"
NO_RELAXED_SEQUENCES = "No relaxed sequences!!!"

results_file = open(RESULTS_FULL_PATH)
sequences_file = open(SEQUENCES_FULL_PATH,'w+')

result_content = results_file.read()

for sequence_result in result_content.split("\n\n"):
	if -1 != sequence_result.find(NO_PATH_FOUND) and -1 == sequence_result.find(NO_RELAXED_SEQUENCES):
		interesting_unresolved_sequence = sequence_result.split('\n')[0][SEQUENCE_STRING_LENGTH:-2]
		sequences_file.write(interesting_unresolved_sequence + '\n')
		print(interesting_unresolved_sequence)

sequences_file.close()
results_file.close()