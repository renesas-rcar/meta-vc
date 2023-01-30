SUMMARY = "A small image just capable of allowing a device to boot."

require recipes-core/images/core-image-minimal.bb
require rcar-image-minimal.inc

COMPATIBLE_MACHINE = "(vc4)"

# Enable package manager
EXTRA_IMAGE_FEATURES += "package-management"

PREFERRED_VERSION_ethtool ?= "5.12"

PREFERRED_VERSION_iproute2 ?= "6.1.0"

# Basic packages
IMAGE_INSTALL_append = " \
    bash \
"
IMAGE_INSTALL_append = " \
    optee-client \
"
