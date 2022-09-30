Yocto layer for Renesas Vehicle Computer boards
===============================================

This layer provides the recipes needed to build a Yocto image for
Vehicle Computer boards.

Please copy `bblayers.conf` and one of the `local*.conf` from
`docs/sample/conf/h3vc/poky-gcc/rswitch/` to your `build/conf/`
folder.

There are two images available (`vc-image-base` and vc-image-weston`)
and can be build with:

```
$ bitbake vc-image-base
```
or:
```
$ bitbake vc-image-weston
```

`vc-image-base` is minimal system that includes only the necessary
driver to use Renesas Vehicle Computer boards. To build this image,
please copy `docs/sample/conf/h3vc/poky-gcc/rswitch/local.conf` to
your `build/conf/` directory.

`vc-image-weston` will include also the gfx drivers and Weston window
system.  To build this image, please copy
`docs/sample/conf/h3vc/poky-gcc/rswitch/local-wayland.conf` to your
`build/conf/` directory and rename it to `local.conf`.  
It is also possible to build an `vc-image-weston` image and include
additional Multimedia Packages. To achieve that, please copy
`docs/sample/conf/h3vc/poky-gcc/rswitch/local-wayland-mmp.conf` to
your `build/conf/` directory and rename it to `local.conf`. This
configuration file can be tuned to include the desired multimedia
packages.

Note: `vc-image-weston` depends on proprietary packages that are
provided by the `meta-renesas` layer.
