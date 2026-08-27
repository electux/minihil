SUMMARY = "Wireless configuration for systemd-networkd on MiniHIL"
DESCRIPTION = "Enables DHCP on wlan0 and autostarts wpa_supplicant service under systemd."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://25-wlan0.network"

S = "${WORKDIR}"

do_install() {
    # 1. Install systemd-networkd configuration for wlan0
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/25-wlan0.network ${D}${sysconfdir}/systemd/network/

    # 2. Symlink wpa_supplicant-wlan0.conf to wpa_supplicant.conf
    # wpa_supplicant@wlan0.service looks for /etc/wpa_supplicant/wpa_supplicant-wlan0.conf
    install -d ${D}${sysconfdir}/wpa_supplicant
    ln -sf ../wpa_supplicant.conf ${D}${sysconfdir}/wpa_supplicant/wpa_supplicant-wlan0.conf

    # 3. Enable wpa_supplicant@wlan0.service on boot
    install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants
    ln -sf ${systemd_system_unitdir}/wpa_supplicant@.service ${D}${sysconfdir}/systemd/system/multi-user.target.wants/wpa_supplicant@wlan0.service
}

FILES:${PN} += " \
    ${sysconfdir}/systemd/network/25-wlan0.network \
    ${sysconfdir}/wpa_supplicant/wpa_supplicant-wlan0.conf \
    ${sysconfdir}/systemd/system/multi-user.target.wants/wpa_supplicant@wlan0.service \
"

RDEPENDS:${PN} += "wpa-supplicant"
