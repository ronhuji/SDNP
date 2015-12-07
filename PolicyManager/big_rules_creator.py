import itertools
from netaddr import *
import pdb
import sys

"""
#zones creation
for i in xrange(256,512):
    print "ZONE_" + str(i) + " = \"192.169." + str(i-256) + ".0/24\""
"""

"""
#admins
for i in xrange(256,512):
    print "ADMIN_" + str(i) + " = \"192.169." + str(i-256) + ".100/32\""
"""

"""
#zones seperated by comma
s = ""
for i in xrange(256,512):
    s+= "ZONE_" + str(i) + ","
print s



#admins seperated by comma
s = ""
for i in xrange(256,512):
    s+= "ADMIN_" + str(i) + ","
print s
"""


#zones location
for i in xrange(0,100):
    print "IP_LOCATIONS[ZONE_" + str(i) + "] = " + str(i%25)
