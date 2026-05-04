# 📦 NetworkManager Custom VPN Plugin (HelloWorld)

Минимальная архитектура кастомного VPN-плагина для NetworkManager. Состоит из C-библиотеки, Python D-Bus демона и конфигурационных файлов. Реализует полный жизненный цикл подключения через асинхронную шину D-Bus и парсинг `stdout` внешнего клиента.

```
NetworkManager → C-плагин (.so) → D-Bus → Python-демон → VPN-клиент (stdout/stderr)
```

---

## 📂 Структура файлов и пути размещения

| Файл | Куда копировать | Права | Владелец | Назначение |
|------|----------------|-------|----------|------------|
| `libnm-vpn-plugin-helloworld.so` | `/usr/lib/NetworkManager/VPN/` | `644` | `root:root` | Плагин NM (загружается автоматически) |
| `helloworldvpn-daemon.py` | `/usr/local/bin/helloworldvpn-daemon` | `755` | `root:root` | D-Bus демон (Python 3.12+) |
| `org.freedesktop.NetworkManager.HelloWorldVPN.service` | `/usr/share/dbus-1/system-services/` | `644` | `root:root` | Автозапуск демона через D-Bus |
| `org.freedesktop.NetworkManager.HelloWorldVPN.conf` | `/etc/dbus-1/system.d/` | `644` | `root:root` | Политика доступа к шине |
| `helloworldvpn-daemon.service` | `/etc/systemd/system/` | `644` | `root:root` | Unit для systemd (опционально) |
| `helloworldVPN.nmconnection` | `/etc/NetworkManager/system-connections/` | **`600`** | **`root:root`** | Профиль подключения ⚠️ |

---

## 🛠️ Установка

### 1. Сборка плагина
```bash
gcc -shared -fPIC -o libnm-vpn-plugin-helloworld.so nm-vpn-plugin-helloworld.c \
  $(pkg-config --cflags --libs glib-2.0 libnm)
```

### 2. Развёртывание файлов
```bash
# Плагин
sudo cp libnm-vpn-plugin-helloworld.so /usr/lib/NetworkManager/VPN/
sudo chmod 644 /usr/lib/NetworkManager/VPN/libnm-vpn-plugin-helloworld.so

# Python-демон
sudo cp helloworldvpn-daemon.py /usr/local/bin/helloworldvpn-daemon
sudo chmod 755 /usr/local/bin/helloworldvpn-daemon

# D-Bus сервис и политика
sudo cp org.freedesktop.NetworkManager.HelloWorldVPN.service /usr/share/dbus-1/system-services/
sudo chmod 644 /usr/share/dbus-1/system-services/org.freedesktop.NetworkManager.HelloWorldVPN.service

sudo cp org.freedesktop.NetworkManager.HelloWorldVPN.conf /etc/dbus-1/system.d/
sudo chmod 644 /etc/dbus-1/system.d/org.freedesktop.NetworkManager.HelloWorldVPN.conf

# systemd unit (опционально)
sudo cp helloworldvpn-daemon.service /etc/systemd/system/
sudo chmod 644 /etc/systemd/system/helloworldvpn-daemon.service

# Профиль NM
sudo cp helloworldVPN.nmconnection /etc/NetworkManager/system-connections/
sudo chown root:root /etc/NetworkManager/system-connections/helloworldVPN.nmconnection
sudo chmod 600 /etc/NetworkManager/system-connections/helloworldVPN.nmconnection
```

### 3. Перезапуск служб
```bash
sudo systemctl daemon-reload
sudo systemctl reload dbus
sudo systemctl restart NetworkManager
```

---

## 🧪 Тестирование

### Ручной запуск демона (рекомендуется на этапе отладки)
```bash
sudo python3 helloworldvpn-daemon.py
# Вывод: [Daemon] Listening on org.freedesktop.NetworkManager.HelloWorldVPN
```

### Вызов подключения (в другом терминале)
```bash
# Вариант A: через D-Bus (быстрая проверка архитектуры)
sudo gdbus call --system \
  --dest org.freedesktop.NetworkManager.HelloWorldVPN \
  --object-path /org/freedesktop/NetworkManager/HelloWorldVPN \
  --method org.freedesktop.NetworkManager.HelloWorldVPN.Service.Connect \
  '@a{sa{sv}} {}' '@a{sv} {}' '30'

# Вариант B: через NetworkManager (требует активного физического интерфейса)
sudo nmcli connection reload
sudo nmcli connection up helloworldVPN
```

✅ **Успешный результат:**
- В терминале демона: `[Daemon] Connect() received` → `[Daemon] Activated`
- В логах NM: `state changed to: CONNECTING (1)` → `state changed to: ACTIVATED (2)`

---

## 🐛 Отладка и мониторинг

| Задача | Команда |
|--------|---------|
| Логи демона | `journalctl -f -u helloworldvpn-daemon` (или вывод в терминале) |
| Логи NetworkManager | `journalctl -f -u NetworkManager \| grep -i vpn` |
| Мониторинг D-Bus | `sudo dbus-monitor --system "interface='org.freedesktop.NetworkManager.HelloWorldVPN.Service'"` |
| Проверка профиля NM | `nmcli -t -f NAME,TYPE,DEVICE connection show \| grep helloworld` |

---

## ⚠️ Важные нюансы
1. **Права `.nmconnection`**: NetworkManager **молча игнорирует** профиль, если права ≠ `600` или владелец ≠ `root`.
2. **WSL2**: Виртуальный `eth0` часто помечен как `unmanaged`. Для тестов достаточно прямого вызова `gdbus`. Для продакшена настройте `wsl.conf` с `systemd=true`.
3. **Потокобезопасность**: Сигналы `StateChanged` отправляются через `GLib.idle_add()`. Это гарантирует доставку в `GMainLoop` NetworkManager без крашей и race conditions.
4. **Парсинг stdout**: Ключевые слова успеха/ошибки находятся в методе `_read_stdout()` демона. Адаптируйте их под формат логов вашего VPN-клиента.
