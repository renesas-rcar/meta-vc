require recipes-graphics/images/core-image-renesas-base.inc
require recipes-core/images/core-image-minimal.bb
require vc-image-base.inc

DESCRIPTION = "Renesas VehicleComputer Base image"

IMAGE_FEATURES += "empty-root-password allow-empty-password"
