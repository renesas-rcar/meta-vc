RENESAS_BSP_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/renesas-linux.git"
BRANCH = "master"
SRC_URI = "${RENESAS_BSP_URL};protocol=ssh;nocheckout=1;branch=${BRANCH}"

# Unfortunately the latest rswitch2 driver in master is broken, so
# let's use tag "1.4.0" until someone decides to fix it ;-P
SRCREV = "2faa9d4254394f7a118940be87f5e5e79f961a84"

# Uncomment and customize the following variables if you need a kernel
# configuration different from the one in linux-renesas_4.14.bbappend
#PATCHES = ""
#HYPERFLASH_CONFIG = ""
#CONFIG = ""
