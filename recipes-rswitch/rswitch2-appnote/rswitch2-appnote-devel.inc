SRC_BRANCH = "master"

SRC_URI = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/vc-tools/rswitch2-appnote.git;protocol=ssh;branch=${SRC_BRANCH}"

# Always use latest version
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

PV_append = "+git${SRCPV}"



