#
# REE loccl development repository override
# Internal use only - Repository unaccessable from outside REE
# Do not distribute.
#

RENESAS_RSWITCH2TOOL_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/vc-tools/rswitch2tool.git"
SRC_URI = "${RENESAS_RSWITCH2TOOL_URL};protocol=ssh;branch=${BRANCH}"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"

# PV needs to be updated with ${SRCPV}, otherwise no upstream changes are detected
PV .= "+git${SRCPV}"

S = "${WORKDIR}/git/"


