#!/usr/bin/env python3
import sys
import subprocess
import threading
from gi.repository import Gio, GLib

BUS_NAME = "org.freedesktop.NetworkManager.HelloWorldVPN"
OBJECT_PATH = "/org/freedesktop/NetworkManager/HelloWorldVPN"
IFACE_NAME = f"{BUS_NAME}.Service"

# GLib Log Levels: 4=Info, 5=Message, 6=Debug, 3=Critical, 2=Error
LOG_INFO = 4

class HelloWorldDaemon:
    def __init__(self):
        self.connection = None
        self.process = None

        ######
    def _start_vpn_client(self):
        if self.process and self.process.poll() is None:
            return False 
        
        cmd = ["/usr/local/bin/mock-vpn-client"]
        self._emit_log(f"Starting: {' '.join(cmd)}")
        self._emit_state(1)  # CONNECTING

        self.process = subprocess.Popen(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1
        )
        threading.Thread(target=self._read_stdout, daemon=True).start()
        return False

    def _method_call(self, connection, sender, path, iface, method, params, invocation):
        print(f"[DBG] D-Bus call: {method}", file=sys.stderr, flush=True)
        try:
            if method == "Connect":
                # Мгновенный ответ, чтобы gdbus/nmcli не ждали
                invocation.return_value(GLib.Variant("()", ()))
                GLib.idle_add(self._start_vpn_client)
                
            elif method == "Disconnect":
                invocation.return_value(GLib.Variant("()", ()))
                # Запускаем в отдельном потоке, чтобы НЕ блокировать GMainLoop
                threading.Thread(target=self._stop_vpn_client_bg, daemon=True).start()
                
            else:
                invocation.return_dbus_error(
                    "org.freedesktop.DBus.Error.UnknownMethod",
                    f"Method '{method}' not implemented"
                )
        except Exception as e:
            print(f"[ERR] Method call failed: {e}", file=sys.stderr, flush=True)
            try: invocation.return_dbus_error("org.freedesktop.DBus.Error.Failed", str(e))
            except: pass
    
    def _stop_vpn_client_bg(self):
        """Фоновая остановка клиента (не блокирует D-Bus)"""
        if self.process:
            self.process.terminate()
            try:
                self.process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.process.kill()
            self.process = None
        # Возвращаем сигнал в главный поток
        GLib.idle_add(self._emit_state, 4)  # DISCONNECTED

        def _read_stdout(self):
        if not self.process or not self.process.stdout: return
        try:
            for line in self.process.stdout:
                line = line.strip()
                if not line: continue
                
                print(f"[VPN] {line}", file=sys.stderr, flush=True)
                self._emit_log(line)
                
                # ИСПРАВЛЕНО: явное сравнение вместо always-True строки
                if "connected" in line.lower():
                    self._emit_state(2)  # ACTIVATED
                elif "disconnected" in line.lower() or "error" in line.lower() or "fail" in line.lower():
                    self._emit_state(3)  # FAILED
                    break
        except Exception as e:
            print(f"[ERR] stdout reader: {e}", file=sys.stderr, flush=True)
            self._emit_state(3)

    def _emit_state(self, state):
        if self.connection:
            GLib.idle_add(self.connection.emit_signal, None, OBJECT_PATH, IFACE_NAME,
                          "StateChanged", GLib.Variant("(uu)", (state, 0)))

    def _emit_log(self, message):
        if self.connection:
            GLib.idle_add(self.connection.emit_signal, None, OBJECT_PATH, IFACE_NAME,
                          "LogMessage", GLib.Variant("(us)", (LOG_INFO, message)))

    def run(self):
        self.connection = Gio.bus_get_sync(Gio.BusType.SYSTEM, None)
        node_info = Gio.DBusNodeInfo.new_for_xml(f"""
            <node name="/org/freedesktop/NetworkManager/HelloWorldVPN">
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
                <signal name='LogMessage'>
                  <arg name='priority' type='u'/>
                  <arg name='message' type='s'/>
                </signal>
              </interface>
            </node>
        """)
        self.connection.register_object(OBJECT_PATH, node_info.interfaces[0], self._method_call, None, None)
        Gio.bus_own_name_on_connection(self.connection, BUS_NAME, Gio.BusNameOwnerFlags.NONE, None, None)
        print(f"[Daemon] Listening on {BUS_NAME}", file=sys.stderr)
        GLib.MainLoop().run()

if __name__ == "__main__":
    HelloWorldDaemon().run()
