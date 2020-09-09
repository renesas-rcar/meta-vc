#
# REE loccl development repository override
# Internal use only - Repository unaccessable from outside REE
# Do not distribute.
#

RENESAS_RSWITCHTOOL_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/rswitch2-utils/rswitchtool.git"
BRANCH = "master"

# Use latest version
#SRCREV = "${AUTOREV}"
SRCREV="a1a25961a36cb434ead5383b88349d0df45d54b8"
#SRCREV = "ce387eb870d73fef9d38750ab4dd39e1dcba72e8"

SRC_URI = "${RENESAS_RSWITCHTOOL_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"


