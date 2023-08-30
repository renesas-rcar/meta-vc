# Uncomment definitions in this file to use kernel sources from a
# different repository

#VC4_KERNEL_REPO = "git://partnergitlab.renesas.solutions/vehicle-computer/renesas-linux.git"
#BRANCH = "vc4-on-3.13"

# Always use the latest version
#SRCREV = "${AUTOREV}"

# Override SRC_URI
#SRC_URI = "${VC4_KERNEL_REPO};branch=${BRANCH};protocol=https"

# Can add patches against that repo
#SRC_URI += "file:///my-kernel-patch.patch"

# WARNING: KBUILD_DEFCONFIG takes defconfig from UNPATCHED kernel.
# If the patches above try to change vc4_defconfig, those changes won't get
# into the build.
#KBUILD_DEFCONFIG = "renesas-vc4_defconfig"

# Alternatively, can use explicitly provided defconfig
#SRC_URI += "file://defconfig"
