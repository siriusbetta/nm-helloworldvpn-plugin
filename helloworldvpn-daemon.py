#!/usr/bin/env python3
import sys
from gi.repository import Gio, GLib

BUS_NAME = "org.freedesktop.NetworkManager.HelloWorldVPN"
OBJECT_PATH = "/org/freedesktop/NetworkManager/HelloWorldVPN"
IFACE_NAME = f"{BUS_NAME}.Service"

class HelloWorldDaemon:
    def __init__(self):
        self.connection = None

    def _method_call(self, connection, sender, path, iface, method, params, invocation):
        """Обработка входящих D-Bus вызовов от C-плагина"""
        if method == "Connect":
            settings, secrets, timeout = params.unpack()
            print("[Daemon] Connect() received", file=sys.stderr)
            self._emit_signal("StateChanged", GLib.Variant("(uu)", (1, 0)))  # CONNECTING
            # Имитация задержки подключения (2 сек)
            GLib.timeout_add(2000, self._activate)
            invocation.return_value(None)
            
        elif method == "Disconnect":
            print("[Daemon] Disconnect() received", file=sys.stderr)
            self._emit_signal("StateChanged", GLib.Variant("(uu)", (4, 0)))  # DISCONNECTED
            invocation.return_value(None)

    def _activate(self):
        print("[Daemon] Activated", file=sys.stderr)
        self._emit_signal("StateChanged", GLib.Variant("(uu)", (2, 0)))  # ACTIVATED
        return False  # Выполнить один раз

    def _emit_signal(self, name, body):
        """Корректная отправка сигнала через системную шину"""
        if self.connection:
            self.connection.emit_signal(None, OBJECT_PATH, IFACE_NAME, name, body)

    def run(self):
        # 1. Подключаемся к системной шине
        self.connection = Gio.bus_get_sync(Gio.BusType.SYSTEM, None)
        
        # 2. Парсим XML интерфейса
        node_info = Gio.DBusNodeInfo.new_for_xml(f"""
            <node>
              <interface name='{IFACE_NAME}'>
                <method name='Connect'>
                  <arg name='settings' type='a{{sa{{sv}}}}' direction='in'/>
                  <arg name='secrets' type='a{{sv}}' direction='in'/>
                  <arg name='timeout' type='u' direction='in'/>
                </method>
                <method name='Disconnect'/>
                <signal name='StateChanged'>
                  <arg name='state' type='u'/>
                  <arg name='reason' type='u'/>
                </signal>
              </interface>
            </node>
        """)
        
        # 3. Регистрируем объект и обработчик методов
        self.connection.register_object(
            OBJECT_PATH, node_info.interfaces[0], self._method_call, None, None
        )
        
        # 4. Захватываем имя на шине
        Gio.bus_own_name_on_connection(
            self.connection, BUS_NAME, Gio.BusNameOwnerFlags.NONE, None, None
        )
        
        print(f"[Daemon] Listening on {BUS_NAME}", file=sys.stderr)
        GLib.MainLoop().run()

if __name__ == "__main__":
    HelloWorldDaemon().run()
