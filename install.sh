#!/bin/bash
# Скрипт установки nm-helloworldvpn-plugin
# Запускать из корневой директории репозитория с правами root: sudo ./install.sh

set -e

# 1. Проверка прав суперпользователя
if [ "$EUID" -ne 0 ]; then
    echo "❌ Пожалуйста, запустите этот скрипт от имени root (используйте sudo)."
    exit 1
fi

echo "🚀 Начало установки nm-helloworldvpn-plugin..."

# 2. Установка зависимостей (Python D-Bus и GI)
echo "📦 Проверка и установка зависимостей..."
if command -v apt-get &> /dev/null; then
    apt-get update -qq
    apt-get install -y python3-dbus python3-gi
elif command -v pacman &> /dev/null; then
    pacman -Sy --noconfirm python-dbus python-gobject
else
    echo "⚠️ Менеджер пакетов не распознан. Убедитесь, что пакеты python3-dbus и python3-gi (или их аналоги) установлены вручную."
fi

# 3. Установка файлов конфигурации и службы
echo "📁 Копирование файлов конфигурации..."
mkdir -p /etc/NetworkManager/system-connections
mkdir -p /usr/lib/NetworkManager/VPN
mkdir -p /usr/share/dbus-1/system-services
mkdir -p /etc/dbus-1/system.d          # ← добавить эту строку
mkdir -p /usr/local/libexec
cp ./config/helloworld-vpn.nmconnection /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection 2>/dev/null || \
cp ./helloworld-vpn.nmconnection /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection
chmod 600 /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection
chown root:root /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection

cp ./config/helloworld.name /usr/lib/NetworkManager/VPN/helloworld.name 2>/dev/null || \
cp ./helloworld.name /usr/lib/NetworkManager/VPN/helloworld.name

cp ./config/org.freedesktop.NetworkManager.helloworld.service /usr/share/dbus-1/system-services/org.freedesktop.NetworkManager.helloworld.service 2>/dev/null || \
cp ./org.freedesktop.NetworkManager.helloworld.service /usr/share/dbus-1/system-services/org.freedesktop.NetworkManager.helloworld.service

cp ./config/org.freedesktop.NetworkManager.helloworld.conf /etc/dbus-1/system.d/org.freedesktop.NetworkManager.helloworld.conf 2>/dev/null || \
cp ./org.freedesktop.NetworkManager.helloworld.conf /etc/dbus-1/system.d/org.freedesktop.NetworkManager.helloworld.conf

# 4. Перезагрузка конфигурации D-Bus
echo "🔄 Перезагрузка конфигурации D-Bus..."
dbus-send --system --type=method_call --dest=org.freedesktop.DBus / org.freedesktop.DBus.ReloadConfig

# 5. Установка DBus скрипта
echo "📁 Копирование DBus скрипта..."
cp ./src/helloworld-dbus.py /usr/local/libexec/helloworld-dbus.py 2>/dev/null || \
cp ./helloworld-dbus.py /usr/local/libexec/helloworld-dbus.py
chmod +x /usr/local/libexec/helloworld-dbus.py
chown root:root /usr/local/libexec/helloworld-dbus.py

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
	  UI_DIR="/usr/lib/x86_64-linux-gnu/qt6/plugins/plasma/network/vpn/"
	fi
fi

if [ "$OS_NAME"=="arch" ] && [ "$DE_NAME"=="kde-plasma" ]; then
	if [ -d "/usr/lib/qt6/plugins/plasma/network/vpn/" ]; then
	  UI_DIR="/usr/lib/qt6/plugins/plasma/network/vpn/"
	fi
fi

if [ "$OS_NAME"=="arch" ] && [ "$DE_NAME"=="gnome" ]; then
	if [ -d "/usr/lib/NetworkManager/" ]; then
	  UI_DIR="/usr/lib/NetworkManager/"
	fi
fi

UI_FILE1="./nm-plugin-hello-gtk-ui/build/libnm-gtk4-vpn-plugin-helloworld-editor.so"
UI_FILE2="./nm-plugin-hello-gtk-ui/build/libnm-vpn-plugin-helloworld.so"

if [ -f "$UI_FILE1" ] && [ -f "$UI_FILE2" ]; then
  if [ -n "$UI_DIR" ]; then
    echo "Установка UI библиотеки"
    cp "$UI_FILE1" "$UI_DIR"
    cp "$UI_FILE2" "$UI_DIR"
    chmod 755 "$UI_DIR/libnm-gtk4-vpn-plugin-helloworld-editor.so"
    chmod 755 "$UI_DIR/libnm-vpn-plugin-helloworld.so"
  else
    echo "Директория для UI библиотеки не найдена"
  fi
else
  echo "UI библиотека не найдена, установка UI пропущена"
fi


# 8. Перезапуск NetworkManager и перезагрузка соединений
echo "🔄 Перезапуск NetworkManager..."
systemctl restart NetworkManager
nmcli connection reload

echo "✅ Установка успешно завершена!"
echo "💡 Для подключения используйте команду: nmcli connection up helloworld-vpn"
