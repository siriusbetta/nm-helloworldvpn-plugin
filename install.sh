#!/usr/bin/env bash
set -euo pipefail

if [[ ${EUID} -ne 0 ]]; then
    echo "Запустите install.sh от имени root (sudo)." >&2
    exit 1
fi

ROOT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
CONFIG_DIR="$ROOT_DIR/config"
GTK_BUILD_DIR="$ROOT_DIR/nm-plugin-hello-gtk-ui/build"
QT_BUILD_DIR="$ROOT_DIR/nm-plugin-hello-qt6-ui/build"

source /etc/os-release
OS_ID=${ID:-unknown}
DESKTOP=${XDG_CURRENT_DESKTOP:-${XDG_SESSION_DESKTOP:-}}
DESKTOP=${DESKTOP,,}

nm_libdir=$(pkg-config --variable=libdir libnm 2>/dev/null || true)
if [[ -z "$nm_libdir" ]]; then
    nm_libdir=/usr/lib
fi
NM_UI_DIR="$nm_libdir/NetworkManager"

qt_plugin_dir=$(qtpaths6 --plugin-dir 2>/dev/null || true)
if [[ -z "$qt_plugin_dir" ]]; then
    case "$OS_ID" in
        debian|ubuntu) qt_plugin_dir="/usr/lib/$(dpkg-architecture -qDEB_HOST_MULTIARCH 2>/dev/null || echo x86_64-linux-gnu)/qt6/plugins" ;;
        *) qt_plugin_dir=/usr/lib/qt6/plugins ;;
    esac
fi
QT_UI_DIR="$qt_plugin_dir/plasma/network/vpn"

install -d -m 755 /etc/NetworkManager/system-connections \
    /usr/lib/NetworkManager/VPN /usr/share/dbus-1/system-services \
    /etc/dbus-1/system.d /usr/local/libexec
install -m 600 -o root -g root "$CONFIG_DIR/helloworld-vpn.nmconnection" \
    /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection
install -m 644 "$CONFIG_DIR/helloworld.name" /usr/lib/NetworkManager/VPN/helloworld.name
install -m 644 "$CONFIG_DIR/org.freedesktop.NetworkManager.helloworld.service" \
    /usr/share/dbus-1/system-services/org.freedesktop.NetworkManager.helloworld.service
install -m 644 "$CONFIG_DIR/org.freedesktop.NetworkManager.helloworld.conf" \
    /etc/dbus-1/system.d/org.freedesktop.NetworkManager.helloworld.conf
install -m 755 "$ROOT_DIR/src/helloworld-dbus.py" /usr/local/libexec/helloworld-dbus.py
install -m 755 "$CONFIG_DIR/helloworld-auth-dialog.py" /usr/local/libexec/nm-helloworld-auth-dialog

case "$DESKTOP" in
    *gnome*)
        [[ -d "$NM_UI_DIR" ]] || { echo "Не найден каталог GNOME UI: $NM_UI_DIR" >&2; exit 1; }
        install -m 755 "$GTK_BUILD_DIR/libnm-vpn-plugin-helloworld.so" "$NM_UI_DIR/"
        install -m 755 "$GTK_BUILD_DIR/libnm-vpn-plugin-helloworld-editor.so" "$NM_UI_DIR/"
        install -m 755 "$GTK_BUILD_DIR/libnm-gtk4-vpn-plugin-helloworld-editor.so" "$NM_UI_DIR/"
        ;;
    *kde*|*plasma*)
        install -d -m 755 "$QT_UI_DIR"
        install -m 755 "$QT_BUILD_DIR/libplasmanetworkmanagement_helloworldui.so" "$QT_UI_DIR/"
        ;;
    *)
        echo "Окружение не определено; UI-библиотеки не устанавливаются." >&2
        ;;
esac

dbus-send --system --type=method_call --dest=org.freedesktop.DBus \
    / org.freedesktop.DBus.ReloadConfig
systemctl restart NetworkManager
nmcli connection reload
echo "Установка завершена для ОС $OS_ID, окружения ${DESKTOP:-unknown}."
