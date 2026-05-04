#include <gio/gio.h>
#include <NetworkManager.h>
#include <nm-vpn-service-plugin.h>

#define BUS_NAME  "org.freedesktop.NetworkManager.HelloWorldVPN"
#define OBJ_PATH  "/org/freedesktop/NetworkManager/HelloWorldVPN"
#define IFACE     "org.freedesktop.NetworkManager.HelloWorldVPN.Service"

/* --- Явный GObject бойлерплейт (избегаем проблем с авто-генерацией макросов) --- */
typedef struct _HelloWorldVpnPlugin HelloWorldVpnPlugin;
typedef struct _HelloWorldVpnPluginClass HelloWorldVpnPluginClass;

struct _HelloWorldVpnPlugin {
    NMVpnServicePlugin parent_instance;
    GDBusProxy *proxy;
};

struct _HelloWorldVpnPluginClass {
    NMVpnServicePluginClass parent_class;
};

static GType hw_vpn_plugin_get_type(void);

#define HW_TYPE_VPN_PLUGIN (hw_vpn_plugin_get_type())
#define HW_VPN_PLUGIN(obj)         G_TYPE_CHECK_INSTANCE_CAST((obj), HW_TYPE_VPN_PLUGIN, HelloWorldVpnPlugin)
#define HW_VPN_PLUGIN_CLASS(klass) G_TYPE_CHECK_CLASS_CAST((klass), HW_TYPE_VPN_PLUGIN, HelloWorldVpnPluginClass)
#define HW_IS_VPN_PLUGIN(obj)      G_TYPE_CHECK_INSTANCE_TYPE((obj), HW_TYPE_VPN_PLUGIN)

G_DEFINE_TYPE(HelloWorldVpnPlugin, hw_vpn_plugin, NM_TYPE_VPN_SERVICE_PLUGIN)

/* --- Обработка D-Bus сигналов от Python-демона --- */
static void
on_dbus_signal(GDBusProxy *proxy, const gchar *sender_name, const gchar *signal_name,
               GVariant *parameters, gpointer user_data)
{
    HelloWorldVpnPlugin *self = HW_VPN_PLUGIN(user_data);
    if (g_str_equal(signal_name, "StateChanged")) {
        guint state, reason;
        g_variant_get(parameters, "(uu)", &state, &reason);
        nm_vpn_service_plugin_set_state(NM_VPN_SERVICE_PLUGIN(self), (NMVpnConnectionState)state);
    }
}

/* --- Инициализация D-Bus прокси --- */
static void
hw_vpn_plugin_constructed(GObject *object)
{
    G_OBJECT_CLASS(hw_vpn_plugin_parent_class)->constructed(object);
    HelloWorldVpnPlugin *self = HW_VPN_PLUGIN(object);

    /* NM требует указания типа VPN при регистрации плагина */
    g_object_set(G_OBJECT(self), "vpn-type", "helloworld", NULL);

    GError *err = NULL;
    self->proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, NULL,
        BUS_NAME, OBJ_PATH, IFACE, NULL, &err);
    if (self->proxy) {
        g_signal_connect(self->proxy, "g-signal", G_CALLBACK(on_dbus_signal), self);
    } else {
        g_warning("HW-VPN: D-Bus proxy creation failed: %s", err->message);
        g_error_free(err);
    }
}

/* --- NM вызывает при подключении --- */
static gboolean
hw_connect(NMVpnServicePlugin *plugin, NMConnection *conn, GError **err)
{
    HelloWorldVpnPlugin *self = HW_VPN_PLUGIN(plugin);
    if (!self->proxy) return FALSE;

    GVariant *settings = nm_connection_to_dbus(conn, NM_CONNECTION_SERIALIZE_ALL);
    g_dbus_proxy_call(self->proxy, "Connect",
                      g_variant_new("(a{sa{sv}}a{sv}u)", settings, NULL, 45),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
    return TRUE;
}

/* --- NM вызывает при отключении --- */
static gboolean
hw_disconnect(NMVpnServicePlugin *plugin, GError **err)
{
    HelloWorldVpnPlugin *self = HW_VPN_PLUGIN(plugin);
    if (!self->proxy) return FALSE;

    g_dbus_proxy_call(self->proxy, "Disconnect", g_variant_new("()"),
                      G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
    return TRUE;
}

static void hw_vpn_plugin_init(HelloWorldVpnPlugin *self) {}

static void
hw_vpn_plugin_class_init(HelloWorldVpnPluginClass *klass)
{
    GObjectClass *gobj = G_OBJECT_CLASS(klass);
    NMVpnServicePluginClass *vpn = NM_VPN_SERVICE_PLUGIN_CLASS(klass);

    gobj->constructed = hw_vpn_plugin_constructed;
    vpn->connect = hw_connect;
    vpn->disconnect = hw_disconnect;
}

/* --- Точка входа, которую вызывает NetworkManager --- */
NMVpnServicePlugin *
nm_vpn_service_plugin_factory(GError **error)
{
    return g_object_new(HW_TYPE_VPN_PLUGIN, NULL);
}
