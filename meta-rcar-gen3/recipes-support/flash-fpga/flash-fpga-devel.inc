SRC_BRANCH = "master"
           
SRC_URI = "git://git@gitlab.eur.eng.renesas.com:2222/automotive-network-section-ans/vehicle-computer-vc/vc-tools/flash-fpga.git;protocol=ssh;branch=${SRC_BRANCH}"

# Always use latest version
SRCREV = "${AUTOREV}"

# PV needs to be updated with ${SRCPV}, otherwise no upstream changes are detected
PV .= "+git${SRCPV}"

S = "${WORKDIR}/git"
