#
# REE loccl development repository override
# Internal use only - Repository unaccessable from outside REE
# Do not distribute.
#

RENESAS_RSWITCH1TOOL_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/vc-tools/rswitch1tool.git"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"


SRC_URI = "${RENESAS_RSWITCH1TOOL_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"


