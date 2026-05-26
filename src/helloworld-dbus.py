#!/usr/bin/env python3

import time
import socket
import struct
import subprocess
import traceback

import dbus
import dbus.service
import dbus.mainloop.glib

from gi.repository import GLib


BUS_NAME = "org.freedesktop.NetworkManager.helloworld"
OBJECT_PATH = "/org/freedesktop/NetworkManager/VPN/Plugin"
IFACE = "org.freedesktop.NetworkManager.VPN.Plugin"

LOG_FILE = "/tmp/helloworld-vpn.log"

IFNAME = "hello-vpn0"
VPN_IP = "10.255.0.2"
VPN_GW = "10.255.0.1"
VPN_PREFIX = 24
VPN_EXTERNAL_GATEWAY = "192.168.0.1"

NM_VPN_SERVICE_STATE_INIT = 1
NM_VPN_SERVICE_STATE_SHUTDOWN = 2
NM_VPN_SERVICE_STATE_STARTING = 3
NM_VPN_SERVICE_STATE_STARTED = 4
NM_VPN_SERVICE_STATE_STOPPING = 5
NM_VPN_SERVICE_STATE_STOPPED = 6

NM_VPN_PLUGIN_FAILURE_LOGIN_FAILED = 0
NM_VPN_PLUGIN_FAILURE_CONNECT_FAILED = 1
NM_VPN_PLUGIN_FAILURE_BAD_IP_CONFIG = 2


def log(msg):
    line = f"{time.strftime('%Y-%m-%d %H:%M:%S')} {msg}\n"
    with open(LOG_FILE, "a", encoding="utf-8") as f:
        f.write(line)
    print(line, end="", flush=True)


def run(cmd, check=True):
    log(f"RUN: {' '.join(cmd)}")

    p = subprocess.run(
        cmd,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    if p.stdout:
        log(f"STDOUT: {p.stdout.strip()}")

    if p.stderr:
        log(f"STDERR: {p.stderr.strip()}")

    if check and p.returncode != 0:
        raise RuntimeError(
            f"command failed with code {p.returncode}: {' '.join(cmd)}"
        )

    return p


def ip4_to_uint32(addr):
    """
    NetworkManager VPN D-Bus API ожидает IPv4 как uint32.
    """
    return dbus.UInt32(struct.unpack("!I", socket.inet_aton(addr))[0])


class HelloWorldVpnPlugin(dbus.service.Object):
    def __init__(self, bus):
        self.loop = None
        self.bus_name = dbus.service.BusName(BUS_NAME, bus=bus)

        super().__init__(
            conn=bus,
            object_path=OBJECT_PATH,
            bus_name=self.bus_name,
        )

        log("helloworld vpn dbus service started")

    # ---------------------------------------------------------------------
    # Signals expected by NetworkManager
    # ---------------------------------------------------------------------

    @dbus.service.signal(dbus_interface=IFACE, signature="u")
    def StateChanged(self, state):
        pass

    @dbus.service.signal(dbus_interface=IFACE, signature="u")
    def Failure(self, reason):
        pass

    @dbus.service.signal(dbus_interface=IFACE, signature="s")
    def LoginBanner(self, banner):
        pass

    @dbus.service.signal(dbus_interface=IFACE, signature="a{sv}")
    def Config(self, config):
        pass

    @dbus.service.signal(dbus_interface=IFACE, signature="a{sv}")
    def Ip4Config(self, config):
        pass

    @dbus.service.signal(dbus_interface=IFACE, signature="a{sv}")
    def Ip6Config(self, config):
        pass

    @dbus.service.signal(dbus_interface=IFACE, signature="sas")
    def SecretsRequired(self, message, secrets):
        pass

    # ---------------------------------------------------------------------
    # Methods called by NetworkManager
    # ---------------------------------------------------------------------

    @dbus.service.method(
        dbus_interface=IFACE,
        in_signature="a{sa{sv}}",
        out_signature="s",
    )
    def NeedSecrets(self, connection):
        log("NeedSecrets() called")
        log(f"NeedSecrets connection: {repr(connection)}")

        # Секреты не нужны.
        return dbus.String("")

    @dbus.service.method(
        dbus_interface=IFACE,
        in_signature="a{sa{sv}}",
        out_signature="",
    )
    def Connect(self, connection):
        log("Connect() called")
        log(f"Connect connection: {repr(connection)}")

        self.StateChanged(dbus.UInt32(NM_VPN_SERVICE_STATE_STARTING))

        # Асинхронно завершаем connect, чтобы D-Bus method быстро вернулся.
        GLib.timeout_add(100, self.finish_connect)

    @dbus.service.method(
        dbus_interface=IFACE,
        in_signature="a{sa{sv}}a{sv}",
        out_signature="",
    )
    def ConnectInteractive(self, connection, details):
        log("ConnectInteractive() called")
        log(f"ConnectInteractive details: {repr(details)}")

        self.Connect(connection)

    @dbus.service.method(
        dbus_interface=IFACE,
        in_signature="",
        out_signature="",
    )
    def Disconnect(self):
        log("Disconnect() called")

        self.StateChanged(dbus.UInt32(NM_VPN_SERVICE_STATE_STOPPING))

        try:
            self.delete_tun()
        except Exception:
            log("delete_tun() failed:")
            log(traceback.format_exc())

        self.StateChanged(dbus.UInt32(NM_VPN_SERVICE_STATE_STOPPED))

        GLib.timeout_add(300, self.quit_loop)

    @dbus.service.method(
        dbus_interface=IFACE,
        in_signature="a{sa{sv}}",
        out_signature="",
    )
    def NewSecrets(self, connection):
        log("NewSecrets() called")
        log(f"NewSecrets connection: {repr(connection)}")

    # ---------------------------------------------------------------------
    # Internal
    # ---------------------------------------------------------------------

    def finish_connect(self):
        try:
            log("finish_connect()")

            self.ensure_tun()

            config = dbus.Dictionary(
                {
                    # Имя tun-интерфейса, который создал backend.
                    "tundev": dbus.String(IFNAME),

                    # Внешний VPN gateway.
                    # Для реального VPN тут будет адрес сервера.
                    # Для hello-world ставим loopback, чтобы NM не ругался на отсутствие gateway.
                    "gateway": ip4_to_uint32(VPN_EXTERNAL_GATEWAY),

                    "has-ip4": dbus.Boolean(True),
                    "has-ip6": dbus.Boolean(False),
                },
                signature="sv",
            )

            log(f"emit Config: {repr(config)}")
            self.Config(config)

            ip4_config = dbus.Dictionary(
                {
                    "address": ip4_to_uint32(VPN_IP),
                    "gateway": ip4_to_uint32(VPN_GW),
                    "ptp": ip4_to_uint32(VPN_GW),
                    "prefix": dbus.UInt32(VPN_PREFIX),

                    # Необязательно, но полезно для валидного config.
                    "dns": dbus.Array(
                        [
                            ip4_to_uint32("1.1.1.1"),
                            ip4_to_uint32("8.8.8.8"),
                        ],
                        signature="u",
                    ),

                    "domains": dbus.Array(
                        [
                            dbus.String("~helloworld"),
                        ],
                        signature="s",
                    ),
                },
                signature="sv",
            )

            log(f"emit Ip4Config: {repr(ip4_config)}")
            self.Ip4Config(ip4_config)

            self.LoginBanner(dbus.String("Hello from helloworld VPN plugin"))

            self.StateChanged(dbus.UInt32(NM_VPN_SERVICE_STATE_STARTED))

            log("VPN marked as STARTED")

        except Exception:
            log("finish_connect() failed:")
            log(traceback.format_exc())

            self.Failure(dbus.UInt32(NM_VPN_PLUGIN_FAILURE_CONNECT_FAILED))
            self.StateChanged(dbus.UInt32(NM_VPN_SERVICE_STATE_STOPPED))

        return False

    def ensure_tun(self):
        log(f"ensure_tun({IFNAME})")

        # Удаляем старый интерфейс, если остался после прошлого запуска.
        run(["ip", "link", "show", IFNAME], check=False)
        run(["ip", "tuntap", "del", "dev", IFNAME, "mode", "tun"], check=False)

        # Создаём persistent tun.
        run(["ip", "tuntap", "add", "dev", IFNAME, "mode", "tun"])

        # Поднимаем link. IP-адрес должен назначить NetworkManager после Ip4Config.
        run(["ip", "link", "set", IFNAME, "up"])

        log(f"TUN interface {IFNAME} created")

    def delete_tun(self):
        log(f"delete_tun({IFNAME})")

        run(["ip", "link", "set", IFNAME, "down"], check=False)
        run(["ip", "tuntap", "del", "dev", IFNAME, "mode", "tun"], check=False)

        log(f"TUN interface {IFNAME} deleted")

    def quit_loop(self):
        log("quit_loop()")

        if self.loop is not None:
            self.loop.quit()

        return False


def main():
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = dbus.SystemBus()
    plugin = HelloWorldVpnPlugin(bus)

    loop = GLib.MainLoop()
    plugin.loop = loop

    loop.run()


if __name__ == "__main__":
    main()
