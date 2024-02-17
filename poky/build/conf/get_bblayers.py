# Generate the bblayers.conf file for the current build workspace
# emitted to stdout for simplicity.

import os, sys, fnmatch, re
from operator import itemgetter

# Trawl the OEROOT as passed to us and find all the layer files that meet our
# metadata directory criteria...
def getLayerPriority (layerConfPath) :
    # Open layer.conf file and find the priority for it...
    confFile = open(layerConfPath, "r")
    if (confFile != None) :
        for line in confFile :
            fields = line.split()
            if (len(fields) > 0 and re.match("BBFILE_PRIORITY",  fields[0])) :
                #return priority
                return int(fields[2].strip("\""))
    confFile.close()

def getLayerPaths(target,  fnexpr) :
    retList = []
    for file in os.listdir(target) :
        if (fnmatch.fnmatch(file, fnexpr) and not
             (fnmatch.fnmatch(file, "meta-skeleton") or
             fnmatch.fnmatch(file, "meta-selftest") or
             fnmatch.fnmatch(file, "meta-yocto") or
             fnmatch.fnmatch(file, "meta-yocto-bsp"))):
            # Found what might be a metadata layer...
            layerPath = target + "/" + file
            layerConfPath = layerPath + "/conf/layer.conf"
            if os.path.exists(layerConfPath) :
                # Found a layer.  Find the priority for it...
                retList += [( layerPath,  getLayerPriority (layerConfPath) )]
            # Add cv layers manually since these are sublayers
            if (fnmatch.fnmatch(file, "meta-qti-cv-prop")):
                scve = layerPath + "/scve"
                fastcv = layerPath + "/fastcv"
                retList += [( scve, getLayerPriority(scve + "/conf/layer.conf") )]
                retList += [( fastcv, getLayerPriority(fastcv + "/conf/layer.conf") )]
    # In order to avoid potential namespace conflicts, between recipes on layers
    # we sort the list in descending order of priority.
    return sorted(retList,  key=itemgetter(1), reverse=True)


# Just spool the tuple list's paths out in order to a string...
def generatePathString ( pathList ):
    retString = ""
    for path, priority in pathList:
        retString = retString + path + " "
    return retString.strip()


# Emit our config file...
print "# This configuration file is dynamically generated every time" 
print "# set_bb_env.sh is sourced to set up a workspace.  DO NOT EDIT."
print "#--------------------------------------------------------------"
print "LCONF_VERSION = \"6\""
print
print "export WORKSPACE := \"${@os.path.abspath(os.path.join(os.path.dirname(d.getVar('FILE', d)),'../../..'))}\""
#print "# Make sure WORKSPACE isn't exported"
#print "WORKSPACE[unexport] = \"1\""
print 
print "BBPATH = \"${TOPDIR}\""
print "BBFILES ?= \"\""
print "BBLAYERS = \"" + generatePathString(getLayerPaths(sys.argv[1].strip("\""), sys.argv[2].strip("\""))) + "\""



