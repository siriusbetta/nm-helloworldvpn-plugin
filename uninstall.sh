#!/usr/bin/env bash
set -euo pipefail

if [[ ${EUID} -ne 0 ]]; then
    echo "Запустите uninstall.sh от имени root (sudo)." >&2
    exit 1
fi

source /etc/os-release
nm_libdir=$(pkg-config --variable=libdir libnm 2>/dev/null || true)
[[ -n "$nm_libdir" ]] || nm_libdir=/usr/lib
NM_UI_DIR="$nm_libdir/NetworkManager"
qt_plugin_dir=$(qtpaths6 --plugin-dir 2>/dev/null || true)
if [[ -z "$qt_plugin_dir" ]]; then
    case ${ID:-unknown} in
        debian|ubuntu) qt_plugin_dir="/usr/lib/$(dpkg-architecture -qDEB_HOST_MULTIARCH 2>/dev/null || echo x86_64-linux-gnu)/qt6/plugins" ;;
        *) qt_plugin_dir=/usr/lib/qt6/plugins ;;
    esac
fi
QT_UI_DIR="$qt_plugin_dir/plasma/network/vpn"

rm -f /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection
rm -f /usr/lib/NetworkManager/VPN/helloworld.name
rm -f /usr/share/dbus-1/system-services/org.freedesktop.NetworkManager.helloworld.service
rm -f /etc/dbus-1/system.d/org.freedesktop.NetworkManager.helloworld.conf
rm -f /usr/local/libexec/helloworld-dbus.py /usr/local/libexec/nm-helloworld-auth-dialog
rm -f "$NM_UI_DIR/libnm-vpn-plugin-helloworld.so"
rm -f "$NM_UI_DIR/libnm-vpn-plugin-helloworld-editor.so"
rm -f "$NM_UI_DIR/libnm-gtk4-vpn-plugin-helloworld-editor.so"
rm -f "$QT_UI_DIR/libplasmanetworkmanagement_helloworldui.so"

dbus-send --system --type=method_call --dest=org.freedesktop.DBus \
    / org.freedesktop.DBus.ReloadConfig
systemctl restart NetworkManager
nmcli connection reload
echo "Удаление завершено."
