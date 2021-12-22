require recipes-graphics/images/core-image-renesas-base.inc
require recipes-graphics/images/core-image-weston.inc
require recipes-graphics/images/core-image-weston.bb
require vc-image-base.inc

DESCRIPTION = "Renesas VehicleComputer Weston image"

IMAGE_FEATURES += "empty-root-password allow-empty-password"
