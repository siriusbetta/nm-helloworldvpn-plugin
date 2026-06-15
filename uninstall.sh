#!/bin/bash
# Скрипт полного удаления nm-helloworldvpn-plugin
# Запускать с правами root: sudo ./uninstall.sh

set -e

# 1. Проверка прав суперпользователя
if [ "$EUID" -ne 0 ]; then
    echo "❌ Пожалуйста, запустите этот скрипт от имени root (используйте sudo)."
    exit 1
fi

echo "🗑️ Начало удаления nm-helloworldvpn-plugin..."

# 2. Остановка соединения и очистка процессов
echo "🛑 Остановка активных процессов и соединений..."
nmcli connection down helloworld-vpn || true
pkill -f helloworld-dbus.py || true
rm -f /tmp/helloworld-vpn.log || true
ip tuntap del dev hello-vpn0 mode tun 2>/dev/null || true

# 3. Удаление установленных системных файлов
echo "📁 Удаление системных файлов..."
rm -f /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection
rm -f /usr/lib/NetworkManager/VPN/helloworld.name
rm -f /usr/share/dbus-1/system-services/org.freedesktop.NetworkManager.helloworld.service
rm -f /etc/dbus-1/system.d/org.freedesktop.NetworkManager.helloworld.conf
rm -f /usr/local/libexec/helloworld-dbus.py

# 4. Удаление UI библиотеки из правильного места

UI_DIR=""

if [ -f /etc/os-release ]; then
	source /etc/os-release
	echo "Дистрибутив: $NAME"
else
	echo "Файл /ect/os-release не найден"
fi

OS_NAME="$ID"

if [ "$ID" = "arch" ] || [ "$ID" = "manjaro" ]; then
	OS_NAME="arch"
fi

DE_NAME=""
DE=$(echo "$XDG_CURRENT_DESKTOP" | tr '[:upper:]' '[:lower:]')

case "$DE" in
    *gnome*)
        echo "Запущен GNOME"
	DE_NAME="gnome"
        ;;
    *kde*|*plasma*)
        echo "Запущен KDE Plasma"
	DE_NAME="kde-plasma"
        ;;
    *xfce*)
        echo "Запущен XFCE"
	DE_NAME="xfce"
        ;;
    *mate*)
        echo "Запущен MATE"
	DE_NAME="mate"
        ;;
    *cinnamon*)
        echo "Запущен Cinnamon"
	DE_NAME="cinnamon"
        ;;
    *)
        echo "Окружение не определено или используется консоль: $XDG_CURRENT_DESKTOP"
        ;;
esac

if [ "$OS_NAME"=="debian" ] && [ "$DE_NAME"=="kde-plasma" ]; then
	if [ -d "/usr/lib/x86_64-linux-gnu/qt6/plugins/plasma/network/vpn/" ]; then
	  UI_DIR="/usr/lib/x86_64-linux-gnu/qt6/plugins/plasma/network/vpn"
	fi
fi

if [ "$OS_NAME"=="arch" ] && [ "$DE_NAME"=="kde-plasma" ]; then
	if [ -d "/usr/lib/qt6/plugins/plasma/network/vpn/" ]; then
	  UI_DIR="/usr/lib/qt6/plugins/plasma/network/vpn"
	fi
fi

if [ "$OS_NAME"=="debian" ] && [ "$DE_NAME"=="kde-plasma" ]; then
	if [ -d "/usr/lib/x86_64-linux-gnu/NetworkManager/" ]; then
	  UI_DIR="/usr/lib/x86_64-linux-gnu/NetworkManager"
	fi
fi

UI_FILE=""
UI_FILE_LOC=""

if [ "$DE_NAME"=="kde-plasma" ]; then
	UI_FILE_LOC="./nm-plugin-hello-qt6-ui/build/bin"
	UI_FILE="plasmanetworkmanagement_helloworld-vpnui.so"
fi

if [ "$DE_NAME"=="gnome" ]; then
	UI_FILE_LOC="./nm-plugin-hello-gtk-ui/build/"
	UI_FILE="libnm-helloworldvpn-plugin.so"
fi

echo "🎨 Удаление UI библиотеки..."
rm -f "$UI_DIR/$UI_FILE" 2>/dev/null || true

# 5. Перезагрузка конфигурации D-Bus и NetworkManager
echo "🔄 Перезагрузка конфигурации D-Bus и NetworkManager..."
dbus-send --system --type=method_call --dest=org.freedesktop.DBus / org.freedesktop.DBus.ReloadConfig
systemctl restart NetworkManager
nmcli connection reload

echo "✅ Удаление успешно завершено!"
