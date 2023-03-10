SRC_BRANCH = "REE_modifications"
           
SRC_URI = "git://git@gitlab.eur.eng.renesas.com:2222/automotive-network-section-ans/vehicle-computer-vc/bootstrap/mini-monitor.git;protocol=ssh;branch=${SRC_BRANCH}"

SRCREV = "ec05111f40e432dc3dba6bfa959615857554c9d7"
# Always use latest version
#SRCREV = "${AUTOREV}"

# PV needs to be updated with ${SRCPV}, otherwise no upstream changes are detected
PV .= "+git${SRCPV}"

S = "${WORKDIR}/git/Gen3_MiniMonitor"
