SUMMARY = "MiniHIL POSIX C++ JSON-RPC Server"
DESCRIPTION = "Backend daemon managing hardware relays and exposing a JSON-RPC 2.0 interface."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Search path for local files relative to the recipe directory.
# This makes BitBake find our local sw/minihil source directory.
FILESEXTRAPATHS:prepend := "${THISDIR}/../../..:"

SRC_URI = "file://minihil"

S = "${WORKDIR}/minihil"

# Dependencies
DEPENDS = "libgpiod nlohmann-json"

inherit cmake systemd

# Package the systemd unit
SYSTEMD_SERVICE:${PN} = "minihil.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install:append() {
    # Install the systemd service file
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/minihil.service ${D}${systemd_system_unitdir}/
}
