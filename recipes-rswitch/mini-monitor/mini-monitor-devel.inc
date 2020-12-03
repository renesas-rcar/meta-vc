SRC_BRANCH = "REE_modifications"
           
SRC_URI = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/mini-monitor.git;protocol=ssh;branch=${SRC_BRANCH}"

SRCREV = "ec05111f40e432dc3dba6bfa959615857554c9d7"
# Always use latest version
#SRCREV = "${AUTOREV}"

# PV needs to be updated with ${SRCPV}, otherwise no upstream changes are detected
PV .= "+git${SRCPV}"

S = "${WORKDIR}/git/Gen3_MiniMonitor"
