# OpenEmbedded sitecustomize.py (C) 2002-2008 Michael 'Mickey' Lauer <mlauer@vanille-media.de>
# GPLv2 or later
# Version: 20081123
# Features:
# * set proper default encoding
# Features removed for SELinux:
# * enable readline completion in the interactive interpreter
# * load command line history on startup
# * save command line history on exit 

import os

def __enableDefaultEncoding():
    import sys
    try:
        sys.setdefaultencoding( "utf8" )
    except LookupError:
        pass

import sys
try:
    import rlcompleter, readline
except ImportError:
    pass
else:
    __enableDefaultEncoding()
