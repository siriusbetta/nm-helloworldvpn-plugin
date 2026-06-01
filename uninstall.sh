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
echo "🎨 Удаление UI библиотеки..."
rm -f /usr/lib/x86_64-linux-gnu/qt6/plugins/plasma/network/vpn/plasmanetworkmanagement_helloworldui.so 2>/dev/null || true
rm -f /usr/lib/qt6/plugins/plasma/network/vpn/plasmanetworkmanagement_helloworldui.so 2>/dev/null || true

# 5. Перезагрузка конфигурации D-Bus и NetworkManager
echo "🔄 Перезагрузка конфигурации D-Bus и NetworkManager..."
dbus-send --system --type=method_call --dest=org.freedesktop.DBus / org.freedesktop.DBus.ReloadConfig
systemctl restart NetworkManager
nmcli connection reload

echo "✅ Удаление успешно завершено!"
