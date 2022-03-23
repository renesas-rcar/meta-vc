RENESAS_RSW2DUMP_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/vc-tools/rswitch2-dump.git"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"

# PV needs to be updated with ${SRCPV}, otherwise no upstream changes are detected
PV .= "+git${SRCPV}"

SRC_URI = "${RENESAS_RSW2DUMP_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"


