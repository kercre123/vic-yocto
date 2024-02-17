#!/usr/bin/env bash

# Delete files older than 1 day.  Will see some errors for files owned
# by root
find /tmp -type f -mtime +1 -print0 | xargs -r0 rm --

# Delete empty directories.  Will see some errors for files owned
# by root
find /tmp -type d -empty -print0 | xargs -r0 rmdir --

# Return exit code 0
true
