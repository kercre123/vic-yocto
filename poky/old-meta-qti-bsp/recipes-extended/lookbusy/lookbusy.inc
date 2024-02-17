UMMARY = "lookbusy -- a synthetic load generator"
DESCRIPTION = "lookbusy generates synthetic CPU, memory and disk access \
loads on a host.  CPU load is induced by simple arithmetic looping (with \
as little memory bandwidth consumption as feasible) alternating with \
periods of sleeping in an attempt to generate the degree of utilization \
selected.  Memory load is induced by allocating a buffer of \
a controllable size, then steadily stirring it to keep the pages active \
from the VM standpoint.  Disk load is induced through creation of one or \
more files, and copying blocks of it between two moving positions."
HOMEPAGE = "http://devin.com/lookbusy"
SECTION = "utils"

SRC_URI = "http://devin.com/lookbusy/download/lookbusy-${PV}.tar.gz"

inherit autotools gettext pkgconfig texinfo

