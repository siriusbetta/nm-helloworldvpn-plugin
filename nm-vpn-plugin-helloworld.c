#include <gio/gio.h>
#include <NetworkManager.h>
#include <nm-vpn-service-plugin.h>

#define BUS_NAME  "org.freedesktop.NetworkManager.HelloWorldVPN"
#define OBJ_PATH  "/org/freedesktop/NetworkManager/HelloWorldVPN"
#define IFACE     "org.freedesktop.NetworkManager.HelloWorldVPN.Service"

typedef struct {
    NMVpnServicePlugin parent;
    GDBusProxy *proxy;
} HelloWorldVpnPlugin;

typedef struct {
    NMVpnServicePluginClass parent;
} HelloWorldVpnPluginClass;

/* G_DEFINE_TYPE создаёт hw_vpn_plugin_get_type() */
G_DEFINE_TYPE(HelloWorldVpnPlugin, hw_vpn_plugin, NM_TYPE_VPN_SERVICE_PLUGIN)

/* Явные макросы для кастов (чтобы не зависеть от авто-генерации GLib) */
#define HW_TYPE_VPN_PLUGIN (hw_vpn_plugin_get_type())
#define HW_VPN_PLUGIN(obj) G_TYPE_CHECK_INSTANCE_CAST((obj), HW_TYPE_VPN_PLUGIN, HelloWorldVpnPlugin)

static void on_dbus_signal(GDBusProxy *proxy, const gchar *sender, const gchar *signal_name,
                           GVariant *params, gpointer user_data)
{
    HelloWorldVpnPlugin *self = HW_VPN_PLUGIN(user_data);
    
    if (g_str_equal(signal_name, "StateChanged")) {
        guint state, reason;
        g_variant_get(params, "(uu)", &state, &reason);
	g_signal_emit_by_name(NM_VPN_SERVICE_PLUGIN(self), "state-changed",
	       	(NMVpnConnectionState)state, 0);
    } 
    else if (g_str_equal(signal_name, "LogMessage")) {
        guint priority; gchar *msg;
        g_variant_get(params, "(us)", &priority, &msg);
        g_info("[CustomVPN] %s", msg);
        g_free(msg);
    }
}

static void hw_vpn_plugin_constructed(GObject *object) {
    G_OBJECT_CLASS(hw_vpn_plugin_parent_class)->constructed(object);
    HelloWorldVpnPlugin *self = HW_VPN_PLUGIN(object);
    
    g_object_set(G_OBJECT(self), "vpn-type", "helloworld", NULL);
    
    GError *err = NULL;
    self->proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, 0, NULL,
        BUS_NAME, OBJ_PATH, IFACE, NULL, &err);
    if (self->proxy) {
        g_signal_connect(self->proxy, "g-signal", G_CALLBACK(on_dbus_signal), self);
    } else {
        g_warning("HW-VPN: D-Bus proxy failed: %s", err->message);
        g_clear_error(&err);
    }
}

static gboolean hw_connect(NMVpnServicePlugin *plugin, NMConnection *conn, GError **err) {
    HelloWorldVpnPlugin *self = HW_VPN_PLUGIN(plugin);
    if (!self->proxy) return FALSE;
    
    GVariant *settings = nm_connection_to_dbus(conn, NM_CONNECTION_SERIALIZE_ALL);
    g_dbus_proxy_call(self->proxy, "Connect",
                      g_variant_new("(a{sa{sv}}a{sv}u)", settings, NULL, 45),
                      0, -1, NULL, NULL, NULL);
    return TRUE;
}

static gboolean hw_disconnect(NMVpnServicePlugin *plugin, GError **err) {
    HelloWorldVpnPlugin *self = HW_VPN_PLUGIN(plugin);
    if (!self->proxy) return FALSE;
    
    g_dbus_proxy_call(self->proxy, "Disconnect", g_variant_new("()"), 0, -1, NULL, NULL, NULL);
    return TRUE;
}

static void hw_vpn_plugin_init(HelloWorldVpnPlugin *self) {}

static void hw_vpn_plugin_class_init(HelloWorldVpnPluginClass *klass) {
    GObjectClass *gobj = G_OBJECT_CLASS(klass);
    NMVpnServicePluginClass *vpn = NM_VPN_SERVICE_PLUGIN_CLASS(klass);
    gobj->constructed = hw_vpn_plugin_constructed;
    vpn->connect = hw_connect;
    vpn->disconnect = hw_disconnect;
}

NMVpnServicePlugin *nm_vpn_service_plugin_factory(GError **error) {
    return g_object_new(hw_vpn_plugin_get_type(), NULL);
}
