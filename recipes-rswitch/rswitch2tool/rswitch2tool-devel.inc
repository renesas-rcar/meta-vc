#
# REE loccl development repository override
# Internal use only - Repository unaccessable from outside REE
# Do not distribute.
#

RENESAS_RSWITCHTOOL_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/rswitch2-utils/rswitchtool.git"
BRANCH = "master"

# Use latest version
#SRCREV = "${AUTOREV}"
SRCREV = "d805691b5b1b8b6b28dff4a9a30f320f22dc57c4"


SRC_URI = "${RENESAS_RSWITCHTOOL_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"


