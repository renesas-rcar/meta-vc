Accessing the Hyperfash from Linux OS
=====================================

The R-Car Gen3 SiP includes an internal Hyperflash that can be used to
store the IPL (Initial Program Loader) and the payloads to execute
after initialization (e.g. OP-TEE Trusted OS, u-boot).

It is possible to enable read/write access to the internal Hyperflash
from Linux appending `hyperflash-linux` to `DISTRO_FEATURES` in the
yocto `local.conf`.

*Note that for security reasons, the `hyperflash-linux` feature should
be enabled only on development systems.*


How to access the hyperflash
----------------------------

Linux exposes the hyperflash as an MTD device and will create a
`/dev/mtdblock*` file for each partition in the flash. The number of
partitions and their mapping to the hyperflash memory is configurable
via device-tree:

```
&rpc0 {
        status = "okay";

        flash@0 {
                compatible = "cfi-flash";
                reg = <0>;

                partitions {
                        compatible = "fixed-partitions";
                        #address-cells = <1>;
                        #size-cells = <1>;

                        bootparam@0 {
                                reg = <0x00000000 0x040000>;
                                read-only;
                        };
                        bl2@00040000 {
                                reg = <0x00040000 0x140000>;
                                read-only;
                        };
                        cert_header_sa6@00180000 {
                                reg = <0x00180000 0x040000>;
                                read-only;
                        };
                        bl31@001C0000 {
                                reg = <0x001C0000 0x480000>;
                                read-only;
                        };
                        uboot@00640000 {
                                reg = <0x00640000 0x0C0000>;
                                read-only;
                        };
                };
        };
```

Note that by default all partitions are read-only.

A summary of the available partitions is shown by `/proc/mtd`:

```
root@h3vc2:~# cat /proc/mtd 
dev:    size   erasesize  name
mtd0: 00040000 00040000 "bootparam"
mtd1: 00140000 00040000 "bl2"
mtd2: 00040000 00040000 "cert_header_sa6"
mtd3: 00480000 00040000 "bl31"
mtd4: 000c0000 00040000 "uboot"
root@h3vc2:~# 
```

It is possible to access the hyperflash partitions simply by
reading/writing the `/dev/mtdblock*` files.


Example: How to flash a new version of u-boot from Linux
--------------------------------------------------------

***Warning: Be sure to understand the following steps and double-check
all commands parameters before re-flashing any hyperflash
partition. Any mistake could result in an un-bootable system!***

Suppose that we want to store a new version of u-boot into the
hyperflash and the file obtained from the build process is named
`u-boot-h3vc2-v2018.09.srec`.

Remember that by default the device-tree sets all hyperflash
partitions as read-only, so the first step is to recompile the
devicetree removing the "read-only" property from the `uboot@00640000`
node as below:

```
&rpc0 {
        status = "okay";

        flash@0 {
                compatible = "cfi-flash";
                reg = <0>;

                partitions {
                        compatible = "fixed-partitions";
                        #address-cells = <1>;
                        #size-cells = <1>;

                        bootparam@0 {
                                reg = <0x00000000 0x040000>;
                                read-only;
                        };
                        bl2@00040000 {
                                reg = <0x00040000 0x140000>;
                                read-only;
                        };
                        cert_header_sa6@00180000 {
                                reg = <0x00180000 0x040000>;
                                read-only;
                        };
                        bl31@001C0000 {
                                reg = <0x001C0000 0x480000>;
                                read-only;
                        };
                        uboot@00640000 {
                                reg = <0x00640000 0x0C0000>;
                        };
                };
        };
```

Now it's time to convert the srec file to a raw binary file. This can
be done using any Linux PC that has installed the `srec_cat` utility
(part of the `srecord` package on Debian based systems).

```
srec_cat u-boot-h3vc2-v2018.09.srec \
         -offset - -minimum-addr u-boot-h3vc2-v2018.09.srec \
         -o u-boot-h3vc2-v2018.09.bin -binary
```

Copy the binary file to the target board and check which MTD partition
correspond to u-boot:

```
root@h3vc2:~# cat /proc/mtd 
dev:    size   erasesize  name
mtd0: 00040000 00040000 "bootparam"
mtd1: 00140000 00040000 "bl2"
mtd2: 00040000 00040000 "cert_header_sa6"
mtd3: 00480000 00040000 "bl31"
mtd4: 000c0000 00040000 "uboot"
root@h3vc2:~#  
```

`/proc/mtd` shows that "uboot" corresponds to mtd4 so we need to write
our binary file to `/dev/mtdblock4`:

```
root@h3vc2:~# dd if=/home/root/u-boot-elf-h3vc2-v2018.09.bin of=/dev/mtdblock4
```

After the `dd` command returns, the target can be rebooted and the new
version of u-boot will be loaded from the hyperflash.



