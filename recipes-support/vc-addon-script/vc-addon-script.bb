SUMMARY = "Add on scripts for Renesas VC boxes"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

#PV = "1.0.1+git${SRCPV}"
#SRCREV = "3149bfdb4f513e2f0da0a7d0bc5d0873578696f2"
SRC_URI = "file://vc-addon-script-v1.0.0.tar.gz"

RDEPENDS_${PN} =+ "bash tcl"
#SRC_URI_append = "file://FPGA_bitfile/rsw1_rtlid_0x20032708_sysid_0x20070749_MT25QU01G_VC3.bin"


S = "${WORKDIR}/vc-addon-script"

include vc-addon-script-devel.inc

FILES_${PN} = "/home/root/vc2                                              \
                          /home/root/vc2/booting/eth0speed.sh \
                          /home/root/vc2/booting/ptp_status_server.py \
                          /home/root/vc2/booting/ipsock/TcpClient.py \
                          /home/root/vc2/booting/ipsock/TcpServer.py \
                          /home/root/vc2/booting/monitor_link.sh \
                          /home/root/vc2/booting/phyinit.sh \
                          /home/root/vc2/booting/startup_all.sh \
                          /home/root/vc2/booting/switchToEth1.sh \
                          /home/root/vc2/booting/wait_link_up.sh \
                          /home/root/vc2/misc/frer_dump4.sh \
                          /home/root/vc2/misc/frer_dump.sh \
                          /home/root/vc2/misc/hwVersion.sh \
                          /home/root/vc2/misc/Preemption_Frame_L500_PCP1.pcap \
                          /home/root/vc2/misc/Preemption_Frame_L78_PCP7.pcap \
                          /home/root/vc2/misc/Preemption_pathEth_Gen-s_configuration.png \
                          /home/root/vc2/misc/tas-converter.sh \
                          /home/root/vc2/misc/tas-example.sch \
                          /home/root/vc2/misc/tas-gnuplot.cmd \
                          /home/root/vc2/misc/tas-readme.txt \
                          /home/root/vc2/misc/tas-schedule-to-csv.tcl \
                          /home/root/vc2/misc/vc2-phydump.tcl \
                          /home/root/vc2/misc/vc3-printPhyRegs.tcl \
                          /home/root/vc2/misc/writeRTL9010.sh \
                          /home/root/vc2/configuration/ptp4l.cfg                                      \
               /home/root/vc2/configuration/eth_tas_port3.xml                             \
               /home/root/vc2/configuration/l2_gptp_vlan.xml                              \
               /home/root/vc2/configuration/VLAN-unaware_Routing.xml                      \
               /home/root/vc2/configuration/eth_cbs.xml                                   \
               /home/root/vc2/configuration/gptp_cfg.ini                                  \
               /home/root/vc2/configuration/phy_slave_12345.xml                           \
               /home/root/vc2/configuration/preemption.xml                                \
               /home/root/vc2/configuration/tsngw_static_entry.template                   \
               /home/root/vc2/configuration/QoS_OverloadManagement.xml                    \
               /home/root/vc2/configuration/Preemption_fwd_rxbox.xml                      \
               /home/root/vc2/configuration/l2_gptp_default.xml                           \
               /home/root/vc2/configuration/phy_master_12345.xml                          \
               /home/root/vc2/configuration/l2_gptp_vlan5.xml                             \
               /home/root/vc2/configuration/Preemption_fwd_txbox.xml                      \
               /home/root/vc2/configuration/preemption_TAS.xml                            \
               /home/root/vc2/configuration/Preemption_eth.xml                            \
               /home/root/vc2/configuration/ptp4l_master.cfg                              \
               /home/root/vc2/FPGA_bitfile/rsw1_rtlid_0x20032708_sysid_0x20070749_MT25QU01G_VC3.bin \
                         "


# do_compile() nothing
do_compile[noexec] = "1"


# Just install the scripte onto the target
do_install() {
    install -d ${D}/home/root/vc2/booting
    install -m 0755 booting/eth0speed.sh ${D}/home/root/vc2/booting
    install -m 0755 booting/gptp_status_server.py ${D}/home/root/vc2/booting
    install -m 0755 booting/monitor_link.sh ${D}/home/root/vc2/booting
    install -m 0755 booting/phyinit.sh ${D}/home/root/vc2/booting
    install -m 0755 booting/startup_all.sh ${D}/home/root/vc2/booting
    install -m 0755 booting/switchToEth1.sh ${D}/home/root/vc2/booting
    install -m 0755 booting/wait_link_up.sh ${D}/home/root/vc2/booting
    install -m 0755 booting/ipsock/TcpClient.py ${D}/home/root/vc2/booting
    install -m 0755 booting/ipsock/TcpServer.py ${D}/home/root/vc2/booting

    install -d ${D}/home/root/vc2/configuration
    install -m 0755 configuration/eth_cbs.xml                 ${D}/home/root/vc2/configuration
    install -m 0755 configuration/eth_tas_port3.xml           ${D}/home/root/vc2/configuration
    install -m 0755 configuration/gptp_cfg.ini                ${D}/home/root/vc2/configuration
    install -m 0755 configuration/l2_gptp_default.xml         ${D}/home/root/vc2/configuration
    install -m 0755 configuration/l2_gptp_vlan5.xml           ${D}/home/root/vc2/configuration
    install -m 0755 configuration/l2_gptp_vlan.xml            ${D}/home/root/vc2/configuration
    install -m 0755 configuration/phy_master_12345.xml        ${D}/home/root/vc2/configuration
    install -m 0755 configuration/phy_slave_12345.xml         ${D}/home/root/vc2/configuration
    install -m 0755 configuration/Preemption_eth.xml          ${D}/home/root/vc2/configuration
    install -m 0755 configuration/Preemption_fwd_rxbox.xml    ${D}/home/root/vc2/configuration
    install -m 0755 configuration/Preemption_fwd_txbox.xml    ${D}/home/root/vc2/configuration
    install -m 0755 configuration/preemption_TAS.xml          ${D}/home/root/vc2/configuration
    install -m 0755 configuration/preemption.xml              ${D}/home/root/vc2/configuration
    install -m 0755 configuration/ptp4l.cfg                   ${D}/home/root/vc2/configuration
    install -m 0755 configuration/ptp4l_master.cfg            ${D}/home/root/vc2/configuration
    install -m 0755 configuration/QoS_OverloadManagement.xml  ${D}/home/root/vc2/configuration
    install -m 0755 configuration/tsngw_static_entry.template ${D}/home/root/vc2/configuration
    install -m 0755 configuration/VLAN-unaware_Routing.xml    ${D}/home/root/vc2/configuration

    install -d ${D}/home/root/vc2/misc
    install -m 0755 misc/frer_dump4.sh                        ${D}/home/root/vc2/misc
    install -m 0755 misc/frer_dump.sh                         ${D}/home/root/vc2/misc
    install -m 0755 misc/hwVersion.sh                         ${D}/home/root/vc2/misc
    install -m 0755 misc/Preemption_Frame_L500_PCP1.pcap      ${D}/home/root/vc2/misc
    install -m 0755 misc/Preemption_Frame_L78_PCP7.pcap       ${D}/home/root/vc2/misc
    install -m 0755 misc/Preemption_pathEth_Gen-s_configuration.png ${D}/home/root/vc2/misc
    install -m 0755 misc/tas-converter.sh                     ${D}/home/root/vc2/misc
    install -m 0755 misc/tas-example.sch                      ${D}/home/root/vc2/misc
    install -m 0755 misc/tas-gnuplot.cmd                      ${D}/home/root/vc2/misc
    install -m 0755 misc/tas-readme.txt                       ${D}/home/root/vc2/misc
    install -m 0755 misc/tas-schedule-to-csv.tcl              ${D}/home/root/vc2/misc
    install -m 0755 misc/vc2-phydump.tcl                      ${D}/home/root/vc2/misc
    install -m 0755 misc/vc3-printPhyRegs.tcl                 ${D}/home/root/vc2/misc
    install -m 0755 misc/writeRTL9010.sh                      ${D}/home/root/vc2/misc

    install -d ${D}/home/root/vc2/FPGA_bitfile
    install -m 0644 FPGA_bitfile/rsw1_rtlid_0x20032708_sysid_0x20070749_MT25QU01G_VC3.bin ${D}/home/root/vc2/FPGA_bitfile


}
