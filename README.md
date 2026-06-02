
## Description

## 1 - nmconnection

```bash
cp config/helloworld-vpn.nmconnection /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection

sudo chmod 600 /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection
sudo chown root:root /etc/NetworkManager/system-connections/helloworld-vpn.nmconnection
```

## 2 - name

```bash
cp config/helloworld.name  /usr/lib/NetworkManager/VPN/helloworld.name
```

## 3 - service

```bash
cp config/org.freedesktop.NetworkManager.helloworld.service /usr/share/dbus-1/system-services/org.freedesktop.NetworkManager.helloworld.service
```

## 4 - conf

```bash
cp config/org.freedesktop.NetworkManager.helloworld.conf /etc/dbus-1/system.d/org.freedesktop.NetworkManager.helloworld.conf
```

## 5 - reset dbus

```bash
sudo dbus-send \
  --system \
  --type=method_call \
  --dest=org.freedesktop.DBus \
  / \
  org.freedesktop.DBus.ReloadConfig
```


## 6 - dbus dispatcher

```bash
cp src/helloworld-dbus.py /usr/local/libexec/helloworld-dbus.py
sudo chmod +x /usr/local/libexec/helloworld-dbus.py
sudo chown root:root /usr/local/libexec/helloworld-dbus.py
```
### for Debian/Ubuntu
```bash
sudo apt update
sudo apt install python3-dbus python3-gi
```
### for Arch/Manjaro
```bash
sudo pacman -S python-dbus python-gobject
```

## 7 - install UI
### Qt6 based 

```bash
cd nm-plugin-hello-qt6-ui
sudo cp build/bin/plasmanetworkmanagement_helloworldui.so /usr/lib/qt6/plugins/plasma/network/vpn/
sudo chmod 755 /usr/lib/qt6/plugins/plasma/network/vpn/plasmanetworkmanagement_helloworldui.so
```

## 8 - restart NetworkManager

```bash
sudo systemctl restart NetworkManager
sudo nmcli connection reload
```


## 9 - up connection

```bash
sudo nmcli connection up helloworld-vpn
```

## debug

```bash
sudo pkill -f helloworld-dbus.py || true
sudo rm -f /tmp/helloworld-vpn.log
sudo ip tuntap del dev hello-vpn0 mode tun 2>/dev/null || true
```

## build container for KDE

```bash
mkdir ~/tmp-build
cd nm-plugin-hello-qt6-ui 
TMPDIR=~/tmp-build podman build -t kde-arch-dev .
```

## build lib for plugin

```bash  
TMPDIR=~/tmp-build podman run --rm -v "$PWD:/src:Z" kde-arch-dev 
sh -c "rm -rf build && cmake -B build && cmake --build build"
```

## Install

```bash
chmod +x install.sh
sudo ./install.sh
```

## Uninstall

```bash
chmod +x uninstall.sh
sudo ./uninstall.sh
```
